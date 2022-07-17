#include "pistates.h"
#include "pimachine.h"
#include <QDateTime>

// PIEntryNestedState ////////////////////////////////////////////////////////////
PIEntryNestedState::PIEntryNestedState(QState *_parent): QState(_parent) {}
void PIEntryNestedState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  emit externalSignal(evtype);
}

// PIPriceOnEntryState ////////////////////////////////////////////////////////////
PIPriceOnEntryState::PIPriceOnEntryState(QState *_parent): QState(_parent) {}
void PIPriceOnEntryState::onEntry(QEvent *ev) {
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  // QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  // QString evtype = se->arguments().at(0).toString();
  QJsonObject price = m->product("price").toArray()[0].toObject();
  QString evtypeex = price["productStatus"] == PIMachine::statusMap[ACTIVE_TRIAL] ? "trial_activation_completed" : "activation_completed";
  emit externalSignal(evtypeex);
}
//------------------------ usage ----------------------------------------
// PISuspendingState ////////////////////////////////////////////////////////////
PISuspendingState::PISuspendingState(QState *_parent): QState(_parent) {}

void PISuspendingState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  // creating task SUSPEND to OE
  int tseconds=QDateTime::currentDateTime().toTime_t();
  QJsonObject jo, extra, extra1;
  QJsonValue res;
  jo["wakeAtFunction"] = "SUSPEND";
  jo["wakeAt"] = tseconds;
  QJsonArray prices = QJsonArray() + m->currentPrice();
  extra["price"] = prices;
  jo["extraParams"] = extra;
  m->createTask(jo);
  jo.remove("wakeAt");
  jo["wakeAtFunction"] = "send_event";
  extra1["eventType"] = "trial_ended";
  jo["extraParams"] = extra1;
  m->deleteTask(jo);
  jo["wakeAtFunction"] = "send_event";
  extra1["eventType"] = "commercial_ended";
  jo["extraParams"] = extra1;
  m->deleteTask(jo);
}

// PIProlonationState ////////////////////////////////////////////////////////////
PIProlongationState::PIProlongationState(QState *_parent): QState(_parent) {}
void PIProlongationState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  int tarificationPeriod = m->product("tarificationPeriod").toInt();
  if(tarificationPeriod == 1) {
    emit externalSignal("activate_completed");
    return;
  }
  // creating task to send ACTIVATE or PROLONG request to OE
  int tseconds=QDateTime::currentDateTime().toTime_t();
  QJsonObject jo, extra;
  jo["wakeAtFunction"] = "ACTIVATE";  // TODO: Уточнить имя сценария
  jo["wakeAt"] = tseconds;
  extra["price"] = QJsonArray() + m->currentPrice();
  jo["extraParams"] = extra;
  m->createTask(jo);
  emit m->transitionCompleted();
}

// PIResumingState ////////////////////////////////////////////////////////////
PIResumingState::PIResumingState(QState *_parent): QState(_parent) {}
void PIResumingState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QJsonObject jo;
  QJsonValue res;
  jo["wakeAtFunction"] = "RESUME";  // TODO: Уточнить имя сценария
  jo["wakeAt"] = static_cast<int>(QDateTime::currentDateTime().toTime_t());
  m->createTask(jo);
}

// PISuspendedState ////////////////////////////////////////////////////////////
PISuspendedState::PISuspendedState(QState *_parent): QState(_parent) {}
void PISuspendedState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QJsonObject jo, extra;
  QJsonValue res;
  int tseconds = m->suspendDuration();
  jo["wakeAtFunction"] = "send_event";
  jo["wakeAt"] = tseconds;
  extra["eventType"] = "suspend_ended";
  jo["extraParams"] = extra;
  m->createTask(jo);
  emit m->transitionCompleted();
}

void PISuspendedState::onExit(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  QJsonObject jo, extra;
  QJsonValue res;
  jo["wakeAtFunction"] = "send_event";
  extra["eventType"] = "suspend_ended";
  jo["extraParams"] = extra;
  m->deleteTask(jo);
  emit externalSignal("resume_tarification");
}

// PIPendingDisconnectState ////////////////////////////////////////////////////////////
PIPendingDisconnectState::PIPendingDisconnectState(QState *_parent): QState(_parent) {}
void PIPendingDisconnectState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  QString reason = se->arguments().at(1).toString();
  QJsonValue res;
  QJsonObject jo, extra;
  int tseconds = m->termDate(reason);

  // Ставим все task-и send_event (trial_ended и commercial_ended) на удаление
  QList<QString> to_delete({"trial_ended", "commercial_ended"});
  jo["wakeAtFunction"] = "send_event";
  for(QList<QString>::iterator it=to_delete.begin(); it != to_delete.end(); it++) {    
    extra["eventType"] = *it;
    jo["extraParams"] = extra;
    m->deleteTask(jo);
  }
  // Задаем новые таски
  jo["wakeAtFunction"] = "send_event";
  jo["wakeAt"] = tseconds;
  extra["eventType"] = "disconnect";
  jo["extraParams"] = extra;
  m->createTask(jo);
  m->setProduct("activeEndDate", tseconds);
  emit m->transitionCompleted();
}

// PIDisconnectionState ////////////////////////////////////////////////////////////
PIDisconnectionState::PIDisconnectionState(QState *_parent): QState(_parent) {}
void PIDisconnectionState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  // creating task to send ACTIVATE or PROLONG request to OE
  int tseconds=QDateTime::currentDateTime().toTime_t();
  QJsonObject jo;
  jo["wakeAtFunction"] = "DISCONNECT";
  jo["wakeAt"] = tseconds;
  m->createTask(jo);
  emit m->transitionCompleted();
}

//------------------------ price -----------------------------------------
// PIPriceChangingState ////////////////////////////////////////////////////////////
PIPriceChangingState::PIPriceChangingState(QState *_parent): QState(_parent) {}
void PIPriceChangingState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QJsonObject price = m->currentPrice().toObject();
  QJsonArray alt_array = price["priceAlterations"].toArray();
  QJsonObject alt;
  int period = price["period"].toInt();
  int duration = price["duration"].toInt();
  if(! alt_array.isEmpty()) {
    alt = alt_array[0].toObject();
    period = alt["period"].toInt();
    duration = alt["duration"].toInt();
  }

  if(duration == 0 || period < duration) {
    qDebug() << "PIPriceChangingState::onEntry: price shouldn't be changed. period" << period << "of" << duration;
    emit externalSignal("not_change_price");
    return;
  }
  qDebug() << "PIPriceChangingState::onEntry: price should be changed. period" << period << "of" << duration;
  int tseconds=QDateTime::currentDateTime().toTime_t();
  QJsonObject jo, extra;
  jo["wakeAtFunction"] = "send_event";
  jo["wakeAt"] = tseconds;
  extra["eventType"] = "change_price";
  jo["extraParams"] = extra;

  m->createTask(jo);  

  emit m->transitionCompleted();
}

// PIPriceChangedState ////////////////////////////////////////////////////////////
PIPriceChangedState::PIPriceChangedState(QState *_parent): QState(_parent) {}
void PIPriceChangedState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;

  QJsonObject jo;
  QJsonObject extra;
  QJsonValue res;
  jo["wakeAtFunction"] = "CHANGE_PRICE";
  jo["wakeAt"] = static_cast<int>(QDateTime::currentDateTime().toTime_t());
  extra["nextPrice"] = m->prop("nextPrice");
  jo["extraParams"] = extra;
  m->createTask(jo);
  emit m->transitionCompleted();
}

void PIPriceChangedState::onExit(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  if(!(evtype == "change_price_completed")) {
    return;
  }
  QJsonArray np = m->prop("nextPrice").toArray();
  if(np.isEmpty()) {
    m->machineError("PIPriceChangedState::onExit nextPrice property (array) is required", -2);
    return;
  }
  m->setProduct("price", np);
}

// PIPriceNotChangedState ////////////////////////////////////////////////////////////
PIPriceNotChangedState::PIPriceNotChangedState(QState *_parent): QState(_parent) {}
void PIPriceNotChangedState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QString signal = "change_price_completed";
  emit externalSignal(signal);
}

void PIPriceNotChangedState::onExit(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  m->setProperty("priceStartDate", m->product("activeEndDate").toInt());
}

// PICommercialState ////////////////////////////////////////////////////////////
PICommercialState::PICommercialState(QState *_parent): QState(_parent) {}
void PICommercialState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  QJsonObject jo;
  int tseconds = m->targetStateEol("commercial");
  if(tseconds < 0) return;
  m->increasePeriods();
  m->setProduct("activeEndDate", tseconds);
  // Prepairing tasks
  //sets the time by substracting 1hour from activate end date
  tseconds = tseconds - m->config("priceEndedBeforeTime").toInt();
  // creating task to send commercial_ended event
  jo["wakeAtFunction"] = "send_event";
  jo["wakeAt"] = tseconds;
  QJsonObject extra;
  extra["eventType"] = "commercial_ended";
  jo["extraParams"] = extra;
  m->createTask(jo);
  QString cascadeEvtype = evtype == "activation_completed" ? "activation_completed" : "wait_payment";
  // Если мы получили сигнал от notPaid, это - выход из suspend-а и конечная точка транзакции
  if(se->sender()->objectName() == "suspended") {
    emit m->transitionCompleted();
    return;
  }
  emit externalSignal(cascadeEvtype);
}

// PITrialState ////////////////////////////////////////////////////////////
PITrialState::PITrialState(QState *_parent): QState(_parent) {}
void PITrialState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  QJsonObject jo, extra;
  // First setting the properties
  int tseconds = m->targetStateEol("trial");
  if(tseconds < 0) return;
  m->setProduct("trialEndDate", tseconds);
  m->setProduct("activeEndDate", tseconds);
  m->increasePeriods();

  // Now working with tasks
  tseconds = tseconds - m->config("priceEndedBeforeTime").toInt();
  jo["wakeAtFunction"] = "send_event";
  jo["wakeAt"] = tseconds;
  extra["eventType"] = "trial_ended";
  jo["extraParams"] = extra;
  m->createTask(jo);
  // trial_activation_completed мы транслируем в машинку payment
  if(evtype == "trial_activation_completed" ||
      evtype == "change_price_completed") {
    emit externalSignal(evtype);
    return;
  }
  emit m->transitionCompleted();
}

//--------------------- payment ---------------------------------
// PIPaidState ////////////////////////////////////////////////////////////
PIPaidState::PIPaidState(QState *_parent): QState(_parent) {}
void PIPaidState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
    // trial_activation_completed мы транслируем в машинку usage
  if(evtype == "trial_activation_completed") emit externalSignal(evtype);
}

// PIWaitingPaymentState ////////////////////////////////////////////////////////////
PIWaitingPaymentState::PIWaitingPaymentState(QState *_parent): QState(_parent) {}
void PIWaitingPaymentState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  QJsonObject jo;
  int tseconds = QDateTime::currentDateTime().toTime_t() + m->config("paymentWaitingTime").toInt();
  jo["wakeAtFunction"] = "send_event";
  jo["wakeAt"] = tseconds;
  QJsonObject extra;
  extra["eventType"] = "waiting_pay_ended";
  jo["extraParams"] = extra;
  m->createTask(jo);
  if(evtype == "activation_completed") {
    emit externalSignal(evtype);
    return;
  }
  emit m->transitionCompleted();
}

void PIWaitingPaymentState::onExit(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  // В зависимости от типа события, посылаем самим себе либо prolong, либо suspend
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QList<QString> exitEvents = {"deactivation_started", "deactivation_external", "close_subscriber"};
  QJsonObject jo, extra;
  QJsonValue res;
  // Ставим task waiting_pay_ended на удаление
  jo["wakeAtFunction"] = "send_event";
  extra["eventType"] = "waiting_pay_ended";
  jo["extraParams"] = extra;
  m->deleteTask(jo);  
  if(exitEvents.contains(se->arguments().at(0).toString())) return;
  QString signal = se->arguments().at(0) == "payment_processed" ? "prolong" : "suspend";
  emit externalSignal(signal);
}

// PINotPaidState ////////////////////////////////////////////////////////////
PINotPaidState::PINotPaidState(QState *_parent): QState(_parent) {}
void PINotPaidState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  int tPeriod = m->product("tarificationPeriod").toInt();
  m->setProduct("tarificationPeriod", --tPeriod);
  m->setProduct("activeEndDate", m->config("eventDate").toInt());
}
void PINotPaidState::onExit(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  if(evtype == "payment_processed") {
    emit externalSignal("resume");
  }
}


// PIPaymentOnState ////////////////////////////////////////////////////////////
PIPaymentOnState::PIPaymentOnState(QState *_parent): QState(_parent) {}
void PIPaymentOnState::onExit(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString reason = se->arguments().at(0).toString();
}

// PIPaymentStoppingState ////////////////////////////////////////////////////////////
PIPaymentStoppingState::PIPaymentStoppingState(QState *_parent): QState(_parent) {}
void PIPaymentStoppingState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  int tseconds = m->termDate(evtype);
  if(evtype != "deactivation_external") m->setProduct("activeEndDate", tseconds);
  QJsonObject jo;
  // Ставим task waiting_pay_ended на удаление
  jo["wakeAtFunction"] = "DISCONNECT_EXTERNAL";
  jo["wakeAt"] = static_cast<int>(QDateTime::currentDateTime().toTime_t());
  m->createTask(jo);    
  emit m->transitionCompleted();
}


// TODO: Remove???
void PIPaymentStoppingState::onExit(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  // emit externalSignal("payment_stopped");
}

// PIPaymentStoppedState ////////////////////////////////////////////////////////////
PIPaymentStoppedState::PIPaymentStoppedState(QState *_parent): QState(_parent) {}
void PIPaymentStoppedState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString evtype = se->arguments().at(0).toString();
  emit externalSignal("off_price", evtype);
  emit externalSignal("stop_pay");  // TODO: remove???
}

//-------------------------- price ----------------------------------------
// PIPriceOffState /////////////////////////////////////////////////////////
PIPriceOffState::PIPriceOffState(QState *_parent): QState(_parent) {}
void PIPriceOffState::onEntry(QEvent *ev)
{
  PIMachine *m = static_cast<PIMachine*>(machine());
  if(m->getOperationStatus().opState == INIT) return;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(ev);
  QString reason = se->arguments().at(1).toString();
  emit externalSignal("off_usage", reason);
}

