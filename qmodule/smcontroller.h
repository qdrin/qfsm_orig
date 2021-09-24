#include <QDebug>
#include <QThread>
#include <QTextStream>
#include <QVector>
#include "pimachine.h"

struct MachineData {
  int id;
  QThread *thread;
  PIMachine* machine;
};

class StateMachineController: public QThread
{
  Q_OBJECT
private:
  QVector<MachineData*> machines;
  int maxMachines;
  bool m_isRunning;
public:
  StateMachineController(): StateMachineController(1) {};
  StateMachineController(int _maxMachines);
  ~StateMachineController();
  int newMachine();
signals:
  void finished();
  void externalEvent(const QString &);
public slots:
  QString getMachineState(const int id);
  void stop() {m_isRunning = false; };
  QString sendEvent(const int id, const QString &ev);
  void stopMachine(const int id);
  QString initMachine(const int id, const QString &state);
  bool isRunning(const int id);
  void run();
};