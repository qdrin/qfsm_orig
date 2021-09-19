#ifndef STRINGTRANSITION_H
#define STRINGTRANSITION_H

#include <QAbstractTransition>
#include <QEvent>
#include <QString>
#include "stringevent.h"

class StringTransition: public QAbstractTransition
{
    Q_OBJECT
public:
    StringTransition(const QString& _value);

protected:
    virtual bool eventTest(QEvent *e) override;

    virtual void onTransition(QEvent *e)  override;
private:
    QString m_value;
};

#endif // STRINGTRANSITION_H
