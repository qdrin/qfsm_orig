#ifndef PIMACHINE_H
#define PIMACHINE_H
#include <QStateMachine>

class PIMachine: public QStateMachine
{
    Q_OBJECT
private:
    QHash<QString, QState*> states;
    const int m_id;
    QObject *m_parent;
    void buildMachine();
public:
    PIMachine(const int _id, QObject *_parent = nullptr);
    int id() { return m_id; }
signals:
    void externalSignal(const QString &eventType);
public slots:
    QString externalEventProcess(const QString &eventType);
    QString init(const QString &stateName);
};
#endif // PIMACHINE_H
