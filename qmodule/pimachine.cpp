#include <QFinalState>
#include <QThread>
#include <QTimer>
#include <QEventLoop>
#include "pimachine.h"
#include "stringtransition.h"

const int PIMachine::msTimeout = 10;

PIMachine::PIMachine(const int _id, QObject *_parent):
  m_parent(_parent),
  m_id (_id)
{
  buildMachine();
  connect(this, &PIMachine::stopped, this, &PIMachine::onStop);
  connect(this, &PIMachine::started, this, &PIMachine::onStart);
}

void PIMachine::buildMachine()
{
  QState *pendingActivate = new QState(this);
  QState *activeTrial = new QState(this);
  QState *aborted = new QState(this);
  QState *active = new QState(this);
  QState *pendingDisconnect = new QState(this);
  QState *disconnect = new QState(this);
  QState *suspended = new QState(this);
  pendingActivate->assignProperty(this, "state", "PENDING_ACTIVATE");
  aborted->assignProperty(this, "state", "ABORTED");
  activeTrial->assignProperty(this, "state", "ACTIVE_TRIAL");
  active->assignProperty(this, "state", "ACTIVE");
  pendingDisconnect->assignProperty(this, "state", "PENDING_DISCONNECT");
  disconnect->assignProperty(this, "state", "DISCONNECT");
  suspended->assignProperty(this, "state", "SUSPENDED");
  // QFinalState *sFinal = new QFinalState(m_machine);
  setProperty("state", "UNDEFINED");
  states["PENDING_ACTIVATE"] = pendingActivate;
  states["ABORTED"] = aborted;
  states["ACTIVE_TRIAL"] = activeTrial;
  states["ACTIVE"] = active;
  states["SUSPENDED"] = suspended;
  states["PENDING_DISCONNECT"] = pendingDisconnect;
  states["DISCONNECT"] = disconnect;

  StringTransition *t1 = new StringTransition(this, m_id, "trial_activation_completed");
  StringTransition *t2 = new StringTransition(this, m_id, "activation_aborted");
  StringTransition *t3 = new StringTransition(this, m_id, "deactivation_started");
  StringTransition *t4 = new StringTransition(this, m_id, "deactivation_completed");
  StringTransition *t5 = new StringTransition(this, m_id, "payment_failed");
  StringTransition *t5_1 = new StringTransition(this, m_id, "payment_failed");
  StringTransition *t6 = new StringTransition(this, m_id, "payment_processed");
  StringTransition *t7 = new StringTransition(this, m_id, "activation_completed");

  t1->setTargetState(activeTrial);
  pendingActivate->addTransition(t1);

  t2->setTargetState(aborted);
  pendingActivate->addTransition(t2);

  t3->setTargetState(pendingDisconnect);
  activeTrial->addTransition(t3);

  t4->setTargetState(disconnect);
  pendingDisconnect->addTransition(t4);

  t5->setTargetState(suspended);
  active->addTransition(t5);

  t5_1->setTargetState(suspended);
  activeTrial->addTransition(t5_1);

  t6->setTargetState(active);
  suspended->addTransition(t6);

  t7->setTargetState(active);
  pendingActivate->addTransition(t7);

  setInitialState(pendingActivate);
  qDebug() << "[" << id() << "]PIMachine::buildMachine completed";
}

QString PIMachine::externalEventProcess(const QString &eventType)
{
  qDebug() << "[" << id() << "]PIMachine::externalEventProcess start\n";
  QTimer timer;
  timer.setSingleShot(true);
  QEventLoop loop;
  connect(this, &QState::entered, &loop, &QEventLoop::quit);
  connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
  emit externalSignal(eventType);
  timer.start(msTimeout);
  loop.exec();
  if(timer.isActive()) {
    qDebug("Wait for state change failed");
    return QString("");
  }
  else
    qDebug("state changed");
  return property("state").toString();
}

QString PIMachine::init(const QString &stateName)
{
  qDebug() << "[" << id() << "]PIMachine::init start: " << stateName << "\n";
  QTimer timer;
  timer.setSingleShot(true);
  QEventLoop loop;
  setInitialState(states[stateName]);
  connect(this, &PIMachine::onStop, &loop, &QEventLoop::quit);
  connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
  connect(this, &PIMachine::onStart, &loop, &QEventLoop::quit);
  timer.start(msTimeout);
  stop();
  loop.exec();
  if(timer.isActive()) {
    qDebug() << "[" << id() << "]PIMachine stop failed. isRunning()=" << isRunning() << Qt::endl;
    return QString("");
  }
  timer.start(msTimeout);
  start();
  loop.exec();
  if(timer.isActive()) {
    qDebug() << "[" << id() << "]PIMachine start failed. isRunning()=" << isRunning() << Qt::endl;
    return QString("");
  }
  qDebug() << "[" << id() << "]PIMachine::init completed";
  return property("state").toString();
}

void PIMachine::onStart()
{
  qDebug() << "[" << id() << "] PIMashine started" << Qt::endl;
  emit hasStarted();
}

void PIMachine::onStop()
{
  qDebug() << "[" << id() << "] PIMashine stopped" << Qt::endl;
  emit hasStopped();
}