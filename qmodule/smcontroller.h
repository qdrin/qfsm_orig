#include <QDebug>
#include <QThread>
#include <QTextStream>
#include <QVector>
#include "lua.hpp"
#include "pimachine.h"

struct MachineData {
  int id;
  QThread *thread;
  PIMachine* machine;
};

struct CallbackCollection {
  lua_State *lState;
  int callbacks;
};

class StateMachineController: public QThread
{
  Q_OBJECT
private:
  QVector<MachineData*> machines;
  int maxMachines;
  bool m_isRunning;
  CallbackCollection callbacks;
public:
  StateMachineController(): StateMachineController(1) {};
  StateMachineController(int _maxMachines);
  ~StateMachineController();
  int newMachine();
  void setCallbacks(lua_State *L);
signals:
  void finished();
  void externalEvent(const QString &);
public slots:
  QString getMachineState(const int id);
  void stop() {m_isRunning = false; };
  QString sendEvent(const int id, const QString &ev);
  void onSendCallback(const int id, QString cbName);
  void stopMachine(const int id);
  QString initMachine(const int id, const QString &state);
  bool isRunning(const int id);
  void run();
};