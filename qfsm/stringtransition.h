#ifndef STRINGTRANSITION_H
#define STRINGTRANSITION_H
#include <QDebug>
#include <QStateMachine>
#include <QSignalTransition>
#include <QEvent>
#include <QString>
#include "pimachine.h"

class StringTransition: public QSignalTransition
{
public:
  StringTransition(QObject *sender, const QString& _value, std::function<bool()>_condition = []() -> bool {return true;});
protected:
  virtual bool eventTest(QEvent *e) override;
  const QString &value() { return m_value; }
  virtual void onTransition(QEvent *e)  override;
private:
  QString m_value;
  std::function<bool()>m_condition;
  int machineId;
};

#endif // STRINGTRANSITION_H
