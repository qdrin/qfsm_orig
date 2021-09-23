#ifndef STRINGTRANSITION_H
#define STRINGTRANSITION_H

#include <QStateMachine>
#include <QSignalTransition>
#include <QEvent>
#include <QString>

class StringTransition: public QSignalTransition
{
public:
    StringTransition(QObject *sender, const int _machineId, const QString& _value);
protected:
    virtual bool eventTest(QEvent *e) override;

    virtual void onTransition(QEvent *e)  override;
private:
    QString m_value;
    int machineId;
};

#endif // STRINGTRANSITION_H
