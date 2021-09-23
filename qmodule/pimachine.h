#ifndef PIMACHINE_H
#define PIMACHINE_H
#include <QDebug>
#include <QStateMachine>

class PIMachine: public QStateMachine
{
    Q_OBJECT
private:
  static const int msTimeout;
  QHash<QString, QState*> states;
  const int m_id;
  QObject *m_parent;
  void buildMachine();
public:
  PIMachine(const int _id, QObject *_parent = nullptr);
  int id() { return m_id; }
signals:
  void externalSignal(const QString &eventType);
  void hasStopped();
  void hasStarted();
public slots:
  QString externalEventProcess(const QString &eventType);
  QString init(const QString &stateName);
  void onStart();
  void onStop();
};
#endif // PIMACHINE_H
