#include "stringtransition.h"

StringTransition::StringTransition(QObject *sender, const QString& _value, std::function<bool()>_condition):
    QSignalTransition(sender, SIGNAL(externalSignal(QString))),
    m_value(_value),
    m_condition(_condition)
{
}

bool StringTransition::eventTest(QEvent *e)
{
  if (!QSignalTransition::eventTest(e))
      return false;
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
  QString evType = se->arguments().at(0).toString();
  bool res = (m_value == se->arguments().at(0)) && m_condition();
  return res;
}

void StringTransition::onTransition(QEvent *e) {
  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
  QString evType = se->arguments().at(0).toString();
  PIMachine *m = static_cast<PIMachine*>(machine());
  // qInfo() << "StringTransition::onTransition" << evType << "from" << se->sender()->objectName()
  //    << "target" << targetState()->objectName();
  m->setHasTransition(true);
}
