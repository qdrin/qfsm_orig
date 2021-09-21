#ifndef PIMACHINE_H
#define PIMACHINE_H
#include <QStateMachine>
#include "stringtransition.h"

class PiMachine: public QObject
{
    Q_OBJECT
private:
    QObject *m_parent;
    QStateMachine *m_machine;
    void buildMachine();
public:
    PiMachine(QObject *_parent = nullptr);
    QStateMachine *machine();
signals:
    void externalSignal(QString eventType);
public slots:
    void externalEventProcess(QString eventType);
};

#endif // PIMACHINE_H
