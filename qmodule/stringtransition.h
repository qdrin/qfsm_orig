#ifndef STRINGTRANSITION_H
#define STRINGTRANSITION_H

#include <QStateMachine>
#include <QSignalTransition>
#include <QEvent>
#include <QString>

class StringTransition: public QSignalTransition
{
public:
    StringTransition(QObject *sender, const QString& _value);
protected:
    virtual bool eventTest(QEvent *e) override;

    virtual void onTransition(QEvent *e)  override;
private:
    QString m_value;
};

#endif // STRINGTRANSITION_H
