#include "stringtransition.h"

StringTransition::StringTransition(QObject *sender, const QString& _value):
    QSignalTransition(sender, SIGNAL(externalSignal(QString))),
    m_value(_value)
{
}

bool StringTransition::eventTest(QEvent *e)
{
    if (!QSignalTransition::eventTest(e))
        return false;
    QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
    return (m_value == se->arguments().at(0));
}

void StringTransition::onTransition(QEvent *e) {
}
