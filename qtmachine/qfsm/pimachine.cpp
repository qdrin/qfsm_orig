#include <QFinalState>
#include "pimachine.h"

PIMachine::PIMachine(QObject *_parent):
    m_parent(_parent)
{
    m_machine = new QStateMachine(_parent);
    buildMachine();
    m_machine->start();
}

void PIMachine::buildMachine()
{
    QState *pendingActivate = new QState(m_machine);
    QState *activeTrial = new QState(m_machine);
    QState *aborted = new QState(m_machine);
    QState *pendingDisconnect = new QState(m_machine);
    QState *disconnect = new QState(m_machine);
    pendingActivate->assignProperty(m_machine, "state", "PENDING_ACTIVATE");
    aborted->assignProperty(m_machine, "state", "ABORTED");
    activeTrial->assignProperty(m_machine, "state", "ACTIVE_TRIAL");
    pendingDisconnect->assignProperty(m_machine, "state", "PENDING_DISCONNECT");
    disconnect->assignProperty(m_machine, "state", "DISCONNECT");
    QFinalState *sFinal = new QFinalState(m_machine);
    m_machine->setProperty("state", "UNDEFINED");

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

    m_machine->setInitialState(pendingActivate);
}

void PIMachine::externalEventProcess(QString eventType)
{
    emit externalSignal(eventType);
}

