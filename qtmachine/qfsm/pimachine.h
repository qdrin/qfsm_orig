#ifndef PIMACHINE_H
#define PIMACHINE_H
#include <QStateMachine>
#include <QString>
#include <QAbstractAnimation>
#include "stringtransition.h"

class PIMachine: public QObject
{
private:
    QStateMachine *m_machine;
    void buildMachine();
public:
    PIMachine();
    QStateMachine *machine() { return m_machine; }
    void postEvent(const QString eventString);
};

#endif // PIMACHINE_H
