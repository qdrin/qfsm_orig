#include "stringtransition.h"

StringTransition::StringTransition(QObject *sender, const int _machineId, const QString& _value):
    QSignalTransition(sender, SIGNAL(externalSignal(QString))),
    m_value(_value),
    machineId(_machineId)
{
}

bool StringTransition::eventTest(QEvent *e)
{
  if (!QSignalTransition::eventTest(e))
      return false;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
  // qDebug() << "StringTransition::eventTest called\n";
  return (m_value == se->arguments().at(0));
}

void StringTransition::onTransition(QEvent *e) {
  qDebug() << "StringTransition::onTransition called, machine state\n";
}
