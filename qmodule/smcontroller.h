#include <QThread>
#include <QTextStream>
#include <QVector>
#include "pimachine.h"

struct MachineData {
  int id;
  QThread *thread;
  PIMachine* machine;
};

class StateMachineController: public QObject
{
  Q_OBJECT
private:
  QVector<MachineData*> machines;
  int maxMachines;
  bool m_isRunning;
public:
  StateMachineController(): StateMachineController(1) {};
  StateMachineController(int _maxMachines): maxMachines(_maxMachines), m_isRunning(false) { run(); };
  ~StateMachineController();
  int newMachine();
signals:
  void finished();
  void externalEvent(const int id, const QString &);
public slots:
  void stop() {m_isRunning = false; };
  void sendEvent(const int id, const QString &ev);
  void run();
};