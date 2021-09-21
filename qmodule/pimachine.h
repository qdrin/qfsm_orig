#ifndef PIMACHINE_H
#define PIMACHINE_H
#include <QStateMachine>
#include <QString>
#include "stringtransition.h"

class PIMachine: public QObject
{
    Q_OBJECT
private:
    QObject *m_parent;
    QStateMachine *m_machine;
    void buildMachine();
public:
    PIMachine(QObject *_parent = nullptr);
    QStateMachine *machine() { return m_machine; }
signals:
    void externalSignal(QString eventType);
public slots:
    void externalEventProcess(QString eventType);
};

#endif // PIMACHINE_H
