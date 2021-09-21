#include <QFinalState>
#include "pimachine.h"
#include "stringtransition.h"

PIMachine::PIMachine(QObject *_parent):
    m_parent(_parent)
{
    buildMachine();
    start();
}

void PIMachine::buildMachine()
{
    QState *pendingActivate = new QState(this);
    QState *activeTrial = new QState(this);
    QState *aborted = new QState(this);
    QState *pendingDisconnect = new QState(this);
    QState *disconnect = new QState(this);
    pendingActivate->assignProperty(this, "state", "PENDING_ACTIVATE");
    aborted->assignProperty(this, "state", "ABORTED");
    activeTrial->assignProperty(this, "state", "ACTIVE_TRIAL");
    pendingDisconnect->assignProperty(this, "state", "PENDING_DISCONNECT");
    disconnect->assignProperty(this, "state", "DISCONNECT");
    // QFinalState *sFinal = new QFinalState(m_machine);
    setProperty("state", "UNDEFINED");

    StringTransition *t1 = new StringTransition(this, "trial_activation_completed");
    StringTransition *t2 = new StringTransition(this, "activation_aborted");
    StringTransition *t3 = new StringTransition(this, "deactivation_started");
    StringTransition *t4 = new StringTransition(this, "deactivation_completed");

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

void PIMachine::externalEventProcess(QString eventType)
{
    emit externalSignal(eventType);
}

