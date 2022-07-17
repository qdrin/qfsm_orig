#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>
#include <QMetaObject>
#include <QFinalState>
#include "pimachine.h"
#include "stringtransition.h"
#include "pistates.h"

// const int PIMachine::msTimeout = 10;
const QList<QString> PIMachine::m_processedProperties = {
  "trialEndDate", "activeEndDate", "tarificationPeriod"
};

const QMap<ProductStates, QString> PIMachine::statusMap = QMap<ProductStates, QString> {
  { ACTIVE, "ACTIVE" },
  { ACTIVE_TRIAL, "ACTIVE_TRIAL" },
  { SUSPENDED, "SUSPENDED" },
  { PENDING_DISCONNECT, "PENDING_DISCONNECT" },
  { PENDING_ACTIVATE, "PENDING_ACTIVATE" },
  { DISCONNECT, "DISCONNECT" },
  { ABORTED, "ABORTED"},
  { UNDEFINED, "UNDEFINED"}
};

// TODO: Remove L(?) and rid from args
PIMachine::PIMachine(const int _id, QObject *_parent, const int _timeout):
  m_parent(_parent),
  m_id (_id),
  msTimeout(_timeout)
{
  resetStatus();
  buildMachine();
  connect(this, &PIMachine::machineError, this, &PIMachine::onMachineError);
}

void PIMachine::resetInitialStates()
{
  //setting initial states for childs
  //usage
  findChild<QState*>("usage")->setInitialState(findChild<QState*>("usageOn"));
  findChild<QState*>("usageOn")->setInitialState(findChild<QState*>("activated"));
  findChild<QState*>("activated")->setInitialState(findChild<QState*>("activatedEntry"));
  findChild<QState*>("usageOff")->setInitialState(findChild<QState*>("pendingDisconnect"));
  //payment
  findChild<QState*>("payment")->setInitialState(findChild<QState*>("paymentOn"));
  findChild<QState*>("paymentOn")->setInitialState(findChild<QState*>("paymentOnEntry"));
  findChild<QState*>("paymentOff")->setInitialState(findChild<QState*>("paymentStopping"));
  //price
  findChild<QState*>("price")->setInitialState(findChild<QState*>("priceOn"));
  findChild<QState*>("priceOn")->setInitialState(findChild<QState*>("priceOnEntry"));
}

void PIMachine::buildMachine()
{
  setObjectName("machine");
  setProperty("status", statusMap[UNDEFINED]);
  QState *pendingActivate = new QState(this);
  pendingActivate->setObjectName("pendingActivate");
  pendingActivate->assignProperty(this, "status", statusMap[PENDING_ACTIVATE]);
  connect(pendingActivate, &QState::entered, this, &PIMachine::onTransitionCompleted);
  QState *aborted = new QState(this);
  aborted->setObjectName("aborted");
  aborted->assignProperty(this, "status", statusMap[ABORTED]);
  connect(aborted, &QState::entered, this, &PIMachine::onTransitionCompleted);
  QState *disconnect = new QState(this);
  disconnect->setObjectName("disconnect");
  disconnect->assignProperty(this, "status", statusMap[DISCONNECT]);
  connect(disconnect, &QState::entered, this, &PIMachine::onTransitionCompleted);
  QState *productProvision = new QState(QState::ParallelStates, this);
  productProvision->setObjectName("productProvision");
  QState *usage = new QState(productProvision);
  usage->setObjectName("usage");
  QState *payment = new QState(productProvision);
  payment->setObjectName("payment");
  QState *price = new QState(productProvision);
  price->setObjectName("price");
  QState *usageOn = new QState(usage);
  usageOn->setObjectName("usageOn");
  QState *usageOff = new QState(usage);
  usageOff->setObjectName("usageOff");
  QState *activated = new QState(usageOn);
  activated->setObjectName("activated");
  PISuspendedState *suspended = new PISuspendedState(usageOn);
  suspended->setObjectName("suspended");
  suspended->assignProperty(this, "status", statusMap[SUSPENDED]);
  PISuspendingState *suspending = new PISuspendingState(usageOn);
  suspending->setObjectName("suspending");
  connect(suspending, &QState::entered, this, &PIMachine::onTransitionCompleted);
  PIResumingState *resuming = new PIResumingState(usageOn);
  resuming->setObjectName("resuming");
  PIProlongationState *prolongation = new PIProlongationState(usageOn);
  prolongation->setObjectName("prolongation");
  PIEntryNestedState *activatedEntry = new PIEntryNestedState(activated);
  activatedEntry->setObjectName("activatedEntry");
  QState *activeTrial = new QState(activated);
  activeTrial->setObjectName("activeTrial");
  activeTrial->assignProperty(this, "status", statusMap[ACTIVE_TRIAL]);
  connect(activeTrial, &QState::entered, this, &PIMachine::onTransitionCompleted);
  QState *active = new QState(activated);
  active->setObjectName("active");
  active->assignProperty(this, "status", "ACTIVE");
  connect(active, &QState::entered, this, &PIMachine::onTransitionCompleted);
  PIPendingDisconnectState *pendingDisconnect = new PIPendingDisconnectState(usageOff);
  pendingDisconnect->setObjectName("pendingDisconnect");
  pendingDisconnect->assignProperty(this, "status", statusMap[PENDING_DISCONNECT]);
  PIDisconnectionState *disconnection = new PIDisconnectionState(usageOff);
  disconnection->setObjectName("disconnection");
  QState *disconnected = new QState(usageOff);
  disconnected->setObjectName("disconnected");
  PIPaymentOnState *paymentOn = new PIPaymentOnState(payment);
  paymentOn->setObjectName("paymentOn");
  QState *paymentOff = new QState(payment);
  paymentOff->setObjectName("paymentOff");
  PIEntryNestedState *paymentOnEntry = new PIEntryNestedState(paymentOn);
  paymentOnEntry->setObjectName("paymentOnEntry");
  PIPaidState *paid = new PIPaidState(paymentOn);
  paid->setObjectName("paid");
  PINotPaidState *notPaid = new PINotPaidState(paymentOn);
  notPaid->setObjectName("notPaid");
  PIWaitingPaymentState *waitingPayment = new PIWaitingPaymentState(paymentOn);
  waitingPayment->setObjectName("waitingPayment");
  PIPaymentStoppingState *paymentStopping = new PIPaymentStoppingState(paymentOff);
  paymentStopping->setObjectName("paymentStopping");
  QState *paymentStopped = new PIPaymentStoppedState(paymentOff);
  paymentStopped->setObjectName("paymentStopped");
  // price
  QState *priceOn = new QState(price);
  priceOn->setObjectName("priceOn");
  PIPriceOffState *priceOff = new PIPriceOffState(price);
  priceOff->setObjectName("priceOff");
  PICommercialState *commercial = new PICommercialState(priceOn);
  commercial->setObjectName("commercial");
  PITrialState *trial = new PITrialState(priceOn);
  trial->setObjectName("trial");
  PIPriceOnEntryState *priceOnEntry = new PIPriceOnEntryState(priceOn);
  priceOnEntry->setObjectName("priceOnEntry");
  PIPriceChangingState *priceChanging = new PIPriceChangingState(priceOn);
  priceChanging->setObjectName("priceChanging");
  PIPriceChangedState *priceChanged = new PIPriceChangedState(priceOn);
  priceChanged->setObjectName("priceChanged");
  PIPriceNotChangedState *priceNotChanged =  new PIPriceNotChangedState(priceOn);
  priceNotChanged->setObjectName("priceNotChanged");

  //defining final states for parallel group productProvision
  QFinalState *usageFinal = new QFinalState(usage);
  usageFinal->setObjectName("usageFinal");
  QFinalState *paymentFinal = new QFinalState(payment);
  paymentFinal->setObjectName("paymentFinal");
  QFinalState *priceFinal = new QFinalState(price);
  priceFinal->setObjectName("priceFinal");

  resetInitialStates();
  //----------------------- transitions -----------------------------------------------
  StringTransition *t;
  productProvision->addTransition(productProvision, &QState::finished, disconnect);
  //price---------------------
  t = new StringTransition(paymentStopped, "off_price");
  t->setTargetState(priceOff);
  priceOn->addTransition(t);
  priceOff->addTransition(priceOff, &QState::entered, priceFinal);
  t = new StringTransition(priceOnEntry, "activation_completed");
  t->setTargetState(commercial);
  priceOnEntry->addTransition(t);
  t = new StringTransition(priceOnEntry, "trial_activation_completed");
  t->setTargetState(trial);
  priceOnEntry->addTransition(t);
  t = new StringTransition(this, "trial_ended",
    [this]() -> bool { return configuration().contains(findChild<QState*>("activeTrial")); });
  t->setTargetState(priceChanging);
  trial->addTransition(t);
  t = new StringTransition(this, "commercial_ended",
    [this]() -> bool { return configuration().contains(findChild<QState*>("active")); });
  t->setTargetState(priceChanging);
  commercial->addTransition(t);
  t = new StringTransition(this, "change_price",
    [this]() -> bool {return ! pricesEquals();});
  t->setTargetState(priceChanged);
  priceChanging->addTransition(t);
  t = new StringTransition(priceChanging, "not_change_price");
  t->setTargetState(priceNotChanged);
  priceChanging->addTransition(t);
  t = new StringTransition(this, "change_price_completed",
    [this]() -> bool {
      QJsonObject np = prop("nextPrice").toArray()[0].toObject();
      return np["productStatus"].toString() == statusMap[ACTIVE_TRIAL]; 
    });
  t->setTargetState(trial);
  priceChanged->addTransition(t);
  t = new StringTransition(this, "change_price_completed",
    [this]() -> bool {
      QJsonObject np = prop("nextPrice").toArray()[0].toObject();
      return np["productStatus"].toString() == statusMap[ACTIVE]; 
    });
  t->setTargetState(commercial);
  priceChanged->addTransition(t);
  t = new StringTransition(priceNotChanged, "change_price_completed",
    [this]() -> bool { return currentPrice("productStatus").toString() == statusMap[ACTIVE]; });
  t->setTargetState(commercial);
  priceNotChanged->addTransition(t);
  t = new StringTransition(priceNotChanged, "change_price_completed",
    [this]() -> bool { return currentPrice("productStatus").toString() == statusMap[ACTIVE_TRIAL]; });
  t->setTargetState(trial);
  priceNotChanged->addTransition(t);
  t = new StringTransition(suspended, "resume_tarification");
  t->setTargetState(commercial);
  commercial->addTransition(t);

  //usage transitions
  t = new StringTransition(this, "trial_activation_completed");
  t->setTargetState(productProvision);
  pendingActivate->addTransition(t);
  t = new StringTransition(this, "activation_completed");
  t->setTargetState(productProvision);
  pendingActivate->addTransition(t);
  t = new StringTransition(waitingPayment, "activation_completed");
  t->setTargetState(active);
  activatedEntry->addTransition(t);
  t = new StringTransition(paid, "trial_activation_completed");
  t->setTargetState(activeTrial);
  activatedEntry->addTransition(t);
  t = new StringTransition(trial, "change_price_completed");
  t->setTargetState(activeTrial);
  active->addTransition(t);
  t = new StringTransition(trial, "change_price_completed");
  t->setTargetState(activeTrial);
  activeTrial->addTransition(t);
  t = new StringTransition(this, "activation_aborted");
  t->setTargetState(aborted);
  pendingActivate->addTransition(t);
  t = new StringTransition(priceOff, "off_usage");
  t->setTargetState(usageOff);
  usageOn->addTransition(t);
  t = new StringTransition(this, "disconnect");
  t->setTargetState(disconnection);
  pendingDisconnect->addTransition(t);
  t = new StringTransition(this, "deactivation_completed");
  t->setTargetState(disconnected);
  disconnection->addTransition(t);
  disconnected->addTransition(disconnected, &QState::entered, usageFinal);
  t = new StringTransition(this, "suspend_completed");
  t->setTargetState(suspended);
  suspending->addTransition(t);
  t = new StringTransition(this, "suspend_ended");
  t->setTargetState(paymentOff);
  notPaid->addTransition(t); 
  t = new StringTransition(waitingPayment, "prolong");
  t->setTargetState(prolongation);
  activated->addTransition(t);
  t = new StringTransition(prolongation, "activate_completed");
  t->setTargetState(active);
  prolongation->addTransition(t);
  t = new StringTransition(this, "prolong_completed",
    [this]() -> bool {return currentPrice("productStatus") == "ACTIVE";});
  t->setTargetState(active);
  prolongation->addTransition(t);
  t = new StringTransition(this, "prolong_completed",
    [this]() -> bool {return currentPrice("productStatus") == "ACTIVE_TRIAL";});
  t->setTargetState(activeTrial);
  prolongation->addTransition(t);
  t = new StringTransition(notPaid, "resume");
  t->setTargetState(resuming);
  suspended->addTransition(t);
  t = new StringTransition(this, "resume_completed");
  t->setTargetState(active);
  resuming->addTransition(t);
  t = new StringTransition(waitingPayment, "suspend");
  t->setTargetState(suspending);
  activeTrial->addTransition(t);
  t = new StringTransition(waitingPayment, "suspend");
  t->setTargetState(suspending);
  active->addTransition(t);

  //-------------- payment transitions -------------------------
  t = new StringTransition(commercial, "activation_completed");
  t->setTargetState(waitingPayment);
  paymentOnEntry->addTransition(t);
  t = new StringTransition(trial, "trial_activation_completed");
  t->setTargetState(paid);
  paymentOnEntry->addTransition(t);
  t = new StringTransition(this, "payment_failed");
  t->setTargetState(notPaid);
  waitingPayment->addTransition(t);
  t = new StringTransition(this, "waiting_pay_ended");
  t->setTargetState(notPaid);
  waitingPayment->addTransition(t);
  t = new StringTransition(this, "payment_processed");
  t->setTargetState(paid);
  notPaid->addTransition(t);
  t = new StringTransition(this, "payment_processed");
  t->setTargetState(paid);
  waitingPayment->addTransition(t);
  t = new StringTransition(this, "close_subscriber");
  t->setTargetState(paymentOff);
  paymentOn->addTransition(t);
  t = new StringTransition(this, "deactivation_external");
  t->setTargetState(paymentOff);
  paymentOn->addTransition(t);  
  t = new StringTransition(this, "deactivation_started");
  t->setTargetState(paymentStopped);
  paymentOn->addTransition(t);
  t = new StringTransition(this, "end_of_suspend");
  t->setTargetState(paymentOff);
  notPaid->addTransition(t);
  t = new StringTransition(this, "deactivation_started");
  t->setTargetState(paymentStopped);
  paymentStopping->addTransition(t);
  t = new StringTransition(paymentStopped, "stop_pay");
  t->setTargetState(paymentFinal);
  paymentStopped->addTransition(t);  
  // paymentStopped->addTransition(paymentStopped, &QState::entered, paymentFinal);
  t = new StringTransition(commercial, "wait_payment");
  t->setTargetState(waitingPayment);
  paid->addTransition(t);
  t = new StringTransition(this, "wait_payment");
  t->setTargetState(waitingPayment);
  paid->addTransition(t);
  t = new StringTransition(priceChanged, "prolong");
  t->setTargetState(paid);
  waitingPayment->addTransition(t);
  t = new StringTransition(priceNotChanged, "prolong");
  t->setTargetState(paid);
  waitingPayment->addTransition(t);


  //set the very first initial state
  setInitialState(pendingActivate);
}

 QJsonObject PIMachine::getMachineState(const QObject *from)
{
  QString pName;
  QState *sptr;
  QJsonObject jo;
  QJsonObject res;
  for(QList<QObject *>::const_iterator i = from->children().begin(); i != from->children().end(); i++)
  {
    pName = (*i)->objectName();
    if(pName == "") continue;
    sptr = static_cast<QState *>((*i));
    if(! sptr->active()) continue;
    if( ! (*i)->children().isEmpty() )
    {
      jo = getMachineState((*i));
      res[pName] = jo.count() == 0 ? 1 : (QJsonValue)jo;
    }
    else
      res[pName] = 1;
  }
  return res;
}

bool PIMachine::loadStateOnly(QState* from, const QJsonObject &data)
{
  bool isSuccess = true;
  QJsonObject::const_iterator i;
  QAbstractState *sptr;
  for(i = data.begin(); i != data.end(); i++)
  {
    sptr = findChild<QAbstractState *>(i.key());
    if(sptr == nullptr) {
      status.opState = ERROR;
      QString err = QString("loadState failed! Unknown state ") + i.key();
      qInfo() << err;
      emit machineError(err);
      return false;
    }
    if(from->childMode() == QState::ExclusiveStates) {
      from->setInitialState(sptr);
    }
    if(i->isObject()) {
      if(! loadStateOnly(static_cast<QState *>(sptr), i->toObject())) {
        isSuccess = false;
        break;
      }
    }
  }
  return isSuccess;
}

//TODO: replace label, characteristic and price from config to properties
bool PIMachine::loadState(QState* from, const QJsonValue &data)
{
  m_product = QJsonObject();;
  m_config = QJsonObject();;

  QJsonObject d = data.toObject();
  QJsonObject conf = d["config"].toObject();
  QJsonObject state = d["machine"].toObject();
  m_product = d["product"].toObject();
  setProperty("status", m_product["status"].toString());
  m_product["state"] =  state;
  m_properties = d["properties"].toObject();

  m_config["suspendDuration"] = conf["suspend_duration"].toInt();
  m_config["paymentWaitingTime"] = conf["payment_waiting_time"].toInt();
  m_config["priceEndedBeforeTime"] = conf["price_ended_before_time"].toInt();
  m_config["immediateTaskTime"] = conf["immediate_task_time"].toInt();

  if(d["tasks"].isArray()) m_tasks = d["tasks"].toArray();
  if(d["delete_tasks"].isArray()) m_deleteTasks = d["delete_tasks"].toArray();
  if(! loadStateOnly(from, state)) {
    return false;
  }
  return true;
}

const QJsonValue PIMachine::result()
{
  QJsonObject jo;
  if(status.opState == ERROR) {
    jo["status"] = status.reason;
    jo["error"] = status.error;
    status.result = QJsonValue::Null;
    return QJsonValue(jo);
  }
  else {
    status.error = QJsonValue::Null;
    QJsonObject res = getState().toObject();
    return res;

  }
}

const QJsonValue PIMachine::externalEventProcess(const QJsonValue event)
{
  resetStatus();
  status.opState = PENDING;
  QTimer timer;
  timer.setSingleShot(true);
  QEventLoop loop;
  QJsonObject jo = event.toObject();
  QString eventType = jo["eventType"].toString();
  m_config["eventDate"] = static_cast<int>(jo["eventDate"].toDouble());
  setProperty("priceStartDate", static_cast<int>(m_config["eventDate"].toDouble()));
  connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
  connect(this, &PIMachine::machineError, &loop, &QEventLoop::quit);
  connect(this, &PIMachine::transitionCompleted, &loop, &QEventLoop::quit);
  emit externalSignal(eventType);
  timer.start(msTimeout);
  loop.exec();
  if(! timer.isActive()) {
    status.opState = ERROR;
    QString machineState = QJsonDocument(getMachineState(this)).toJson(QJsonDocument::Compact);
    if(! status.hasTransition) {
      qInfo() << "PIMachine::externalEventProcess transition unknown:" << machineState;
      status.reason = -2;
      status.error = QString("unknown event ").append(eventType).append(" or it's not allowed for status ").
        append(property("status").toString()).append(", machineState ").append(machineState);
    }
    else {
      status.error = QString("PIMachine::externalEventProcess failed: timeout exceeded. event: ").append(eventType).
                    append(", machineState: ").append(machineState);
      status.reason = -1;
    }
  }
  status.opState = status.opState != ERROR ? SUCCESS : ERROR;
  QJsonValue res = result();
  resetInitialStates();
  return res;
}

QJsonValue PIMachine::getState()
{
  status.opState = PENDING;
  QJsonObject res;
  QMetaProperty p;
  QJsonValue var;
  m_product["status"] = property("status").toString();
  res["machine"] = getMachineState(this);
  res["product"] = m_product;
  res["properties"] = m_properties;
  res["tasks"] = m_tasks;
  res["delete_tasks"] = m_deleteTasks;
  status.opState = SUCCESS;
  status.result = QJsonValue(res);
  status.reason = 1;
  QJsonObject result;
  result["status"] = 1;
  result["result"] = res;
  return QJsonValue(result);
}

QJsonValue PIMachine::init(const QJsonValue &state)
{
  status.busy = true;
  status.opState = INIT;
  emit occupied();
  QJsonObject st = state.toObject();
  m_tasks = st["tasks"].toArray();
  m_deleteTasks = st["delete_tasks"].toArray();
  m_config = QJsonObject();
  QTimer timer;
  timer.setSingleShot(true);
  QEventLoop loop;
  if(! loadState(this, state)) {
    emit machineError("PIMachine::init loadState failed");
    return result();
  }
  connect(this, &PIMachine::started, &loop, &QEventLoop::quit);
  connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
  connect(this, &PIMachine::stopped, &loop, &QEventLoop::quit);
  timer.start(msTimeout);
  stop();
  loop.exec();
  if(! timer.isActive()) {
    status.opState = ERROR;
    status.error = QString("PIMachine stop failed. Timeout exceded");
    status.reason = -1;
    return result();
  }
  timer.start(msTimeout);
  start();
  loop.exec();
  if(! timer.isActive()) {
    status.opState = ERROR;
    status.reason = -1;
    status.error = QString("PIMachine start failed. Timeout exceded");
    return result();
  }
  QJsonValue curState = getState();
  return curState;
}

void PIMachine::createTask(QJsonValue task)
{
  QJsonObject jo = task.toObject();
  jo["status"] = "PENDING";
  m_tasks.append(jo);
}

void PIMachine::deleteTask(QJsonValue task)
{
  m_deleteTasks.append(task);
}

void PIMachine::onMachineError(const QString err, const int reason)
{
  qInfo() << "PIMachine::onMachineError start. reason" << reason << "error" << err;
  QString machineState = QJsonDocument(getMachineState(this)).toJson(QJsonDocument::Compact);
  status.opState = ERROR;
  status.reason = reason;
  status.error = err;
  status.error.append(". machineState: ").append(machineState);
}

// #include <QThread>
void PIMachine::onTransitionCompleted()
{
  emit transitionCompleted();
}

const QJsonValue PIMachine::currentPrice(QString key) {
  QJsonValue price = m_product["price"].toArray()[0];
  if(price.toObject().isEmpty()) {
    QString err = "PIMachine::currentPrice: no price found in product";
    qInfo() << err.toStdString().c_str();
    emit machineError(err);
    return QJsonValue();
  }
  if(key.isEmpty()) {
    return price;
  }
  return price.toObject()[key];
}

int PIMachine::suspendDuration()
{
  return (m_config["suspendDuration"].toInt()
    + static_cast<int>(QDateTime::currentDateTime().toTime_t()));
}

QJsonValue PIMachine::config(QString key)
{
  return m_config[key];
}

QJsonValue PIMachine::product(QString key)
{
  return m_product[key];
}

QJsonValue PIMachine::setProduct(QString key, QJsonValue val)
{
  return m_product[key] = val;
}


int PIMachine::targetStateEol(QString curState)
{
  QJsonObject price = currentPrice().toObject();
  // qDebug() << "PIMachine::targetStateEol price" << price;
  int plength = price["recurringChargePeriodLength"].toInt();
  QString durationType = price["recurringChargePeriodType"].toString();
  QDateTime now;
  now.setTime_t(property("priceStartDate").toInt());
  if(durationType == "day") {
    return now.addDays(plength).toTime_t();
  }
  if(durationType == "month") {
    return now.addMonths(plength).toTime_t();
  }
  emit machineError(QString("PIMachine::targetStateEol: Unknown durationType ").append(durationType));
  return -1;
}

int PIMachine::termDate(QString reason)
{
  QJsonArray::iterator it;

  int termDeferred = m_product["activeEndDate"].toInt();
  int termNow = QDateTime::currentDateTime().toTime_t() + m_config["immediateTaskTime"].toInt();
  // Если мы были в suspend-е, то activeEndDate - в прошлом и мы ее меняем на сейчас
  termDeferred = QDateTime::currentDateTime().toTime_t() > termDeferred ? termNow : termDeferred;
  QJsonArray characteristic = m_product["characteristic"].toArray();
  QJsonArray labels = m_product["label"].toArray();
  QString currentStatus = m_product["status"].toString();
  QJsonObject currentState = m_product["state"].toObject();
  if(reason != "deactivation_started") {
    return termNow;
  }

  for(it=labels.begin(); it != labels.end(); it++) {
    QJsonObject label = it->toObject();
    if(label["name"].toString() == "Double Billing" and label["value"].toBool())
      return termNow;
  }
  for(it=characteristic.begin(); it != characteristic.end(); it++) {
    QJsonObject ch = it->toObject();
    if(ch["refName"] == "ActiveDeactivationMode" && ch["value"] == "Immediate"
      && currentStatus == "ACTIVE") {
        return termNow;
    }
    if(ch["refName"] == "TrialDeactivationMode" && ch["value"] == "Immediate"
      && currentStatus == "ACTIVE_TRIAL") {
        return termNow;
    }
  }
  return termDeferred;
}

bool PIMachine::pricesEquals() {
  QJsonObject price = currentPrice().toObject();
  QJsonObject nextPrice = m_properties["nextPrice"].toArray()[0].toObject();
  if(nextPrice.isEmpty()) {
    emit machineError("PIMachine::pricesEquals: nextPrice not found", -2);
  }
  // qDebug() << "PIMachine::pricesEquals: price" << price << ", nextPrice" << nextPrice;
  bool res = nextPrice["id"] == price["id"];
  QJsonObject alt = price["priceAlterations"].toArray()[0].toObject();
  QJsonObject nextAlt = nextPrice["priceAlterations"].toArray()[0].toObject();
  if(! alt.isEmpty() || ! nextAlt.isEmpty()) {
    res = res && (alt["id"].toString() == nextAlt["id"].toString());
  }
  if(res) {
    emit machineError("PIMachine::pricesEquals: next price is the same as current!", -2);
    return res;
  }
  return res;
}

void PIMachine::increasePeriods(int increment) {
  QJsonObject price = currentPrice().toObject();
  QJsonObject alt = price["priceAlterations"].toArray()[0].toObject();
  int p;
  p = price["period"].toInt() + increment;
  price["period"] = p;
  p = m_product["tarificationPeriod"].toInt() + increment;
  m_product["tarificationPeriod"] = p;
  if(! alt.isEmpty()) {
    p = alt["period"].toInt() + increment;
    alt["period"] = p;
    QJsonArray alt_array;
    alt_array.append(alt);
    price["priceAlterations"] = alt_array;
  }
  QJsonArray arr;
  arr.append(price);
  setProduct("price", arr);
}