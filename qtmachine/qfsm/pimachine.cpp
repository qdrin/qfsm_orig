#include "pimachine.h"
#include <QTextStream>

PIMachine::PIMachine()
{
    m_machine = new QStateMachine;
    buildMachine();
    m_machine->start();
}

void PIMachine::buildMachine()
{
    QState *s1 = new QState(m_machine);
    QState *s2 = new QState(m_machine);
    QState *s3 = new QState(m_machine);
    s1->assignProperty(m_machine, "state", "1");
    s2->assignProperty(m_machine, "state", "2");
    s3->assignProperty(m_machine, "state", "3");
    // QFinalState *sFinal = new QFinalState();
    m_machine->setProperty("state", "UNDEFINED");

    StringTransition *t1 = new StringTransition("2");
    StringTransition *t2 = new StringTransition("3");
    StringTransition *t3 = new StringTransition("1");

    t1->setTargetState(s2);
    t2->setTargetState(s3);
    t3->setTargetState(s1);

    s1->addTransition(t1);
    s2->addTransition(t2);
    s3->addTransition(t3);

    m_machine->setInitialState(s1);
    m_machine->setProperty("state", "UNDEFINED");
}

void PIMachine::postEvent(const QString eventString)
{
    QTextStream cout(stdout);
    cout << "PIMachine.postEvent(\"" << eventString << "\") started\n";
    m_machine->postEvent(new StringEvent(eventString));
}
