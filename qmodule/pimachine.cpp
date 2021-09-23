#include <QFinalState>
#include "pimachine.h"
#include "stringtransition.h"
#include <QTextStream>  // TODO: Remove after debug

PIMachine::PIMachine(const int _id, QObject *_parent):
  m_parent(_parent),
  m_id (_id)
{
  buildMachine();
}

void PIMachine::buildMachine()
{
  QState *pendingActivate = new QState(this);
  QState *activeTrial = new QState(this);
  QState *aborted = new QState(this);
  QState *active = new QState(this);
  QState *pendingDisconnect = new QState(this);
  QState *disconnect = new QState(this);
  pendingActivate->assignProperty(this, "state", "PENDING_ACTIVATE");
  aborted->assignProperty(this, "state", "ABORTED");
  activeTrial->assignProperty(this, "state", "ACTIVE_TRIAL");
  active->assignProperty(this, "state", "ACTIVE");
  pendingDisconnect->assignProperty(this, "state", "PENDING_DISCONNECT");
  disconnect->assignProperty(this, "state", "DISCONNECT");
  // QFinalState *sFinal = new QFinalState(m_machine);
  setProperty("state", "UNDEFINED");
  states["PENDING_ACTIVATE"] = pendingActivate;
  states["ABORTED"] = aborted;
  states["ACTIVE_TRIAL"] = activeTrial;
  states["ACTIVE"] = active;
  states["PENDING_DISCONNECT"] = pendingDisconnect;
  states["DISCONNECT"] = disconnect;

  StringTransition *t1 = new StringTransition(this, m_id, "trial_activation_completed");
  StringTransition *t2 = new StringTransition(this, m_id, "activation_aborted");
  StringTransition *t3 = new StringTransition(this, m_id, "deactivation_started");
  StringTransition *t4 = new StringTransition(this, m_id, "deactivation_completed");

  t1->setTargetState(activeTrial);
  pendingActivate->addTransition(t1);

  t2->setTargetState(aborted);
  pendingActivate->addTransition(t2);

  t3->setTargetState(pendingDisconnect);
  activeTrial->addTransition(t3);

  t4->setTargetState(disconnect);
  pendingDisconnect->addTransition(t4);

  setInitialState(pendingActivate);
}

QString PIMachine::externalEventProcess(const QString &eventType)
{
  QTextStream cout(stdout);
  cout << "[" << id() << "]PIMachine::externalEventProcess start\n";
  emit externalSignal(eventType);
  return property("state").toString();
}

QString PIMachine::init(const QString &stateName)
{
  QTextStream cout(stdout);
  cout << "[" << id() << "]PIMachine::init start: " << stateName << "\n";
  setRunning(false);
  stop();
  setInitialState(states[stateName]);
  start();
  return property("state").toString();
}

