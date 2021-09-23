#include "stringtransition.h"
#include <QTextStream>  // TODO: Remove after debug

StringTransition::StringTransition(QObject *sender, const int _machineId, const QString& _value):
    QSignalTransition(sender, SIGNAL(externalSignal(QString))),
    m_value(_value),
    machineId(_machineId)
{
}

bool StringTransition::eventTest(QEvent *e)
{
  QTextStream cout(stdout);
  cout << "StringTransition::eventTest called\n";
  if (!QSignalTransition::eventTest(e))
      return false;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
  return (m_value == se->arguments().at(0));
}

void StringTransition::onTransition(QEvent *e) {
}
