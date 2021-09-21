#ifndef PIMACHINE_H
#define PIMACHINE_H
#include <QStateMachine>

class PIMachine: public QStateMachine
{
    Q_OBJECT
private:
    QObject *m_parent;
    void buildMachine();
public:
    PIMachine(QObject *_parent = nullptr);
signals:
    void externalSignal(QString eventType);
public slots:
    void externalEventProcess(QString eventType);
};
#endif // PIMACHINE_H
