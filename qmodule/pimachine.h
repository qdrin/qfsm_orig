#ifndef PIMACHINE_H
#define PIMACHINE_H
#include <QStateMachine>

class PIMachine: public QStateMachine
{
    Q_OBJECT
private:
    const int m_id;
    QObject *m_parent;
    void buildMachine();
public:
    PIMachine(const int _id, QObject *_parent = nullptr);
    int id() { return m_id; }
signals:
    void externalSignal(const int id, const QString &eventType);
public slots:
    void externalEventProcess(const int id, const QString &eventType);
};
#endif // PIMACHINE_H
