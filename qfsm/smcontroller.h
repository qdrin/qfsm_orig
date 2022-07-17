#ifndef SMCONTROLLER_H
#define SMCONTROLLER_H

#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QTextStream>
#include <QVector>
#include <QJsonValue>
#include <QJsonObject>
#include "pimachine.h"
#include "lua.hpp"

struct MachineData {
  int id;
  QThread *thread;
  PIMachine *machine;
  QJsonValue result;
};

struct LuaConfig {
  lua_State *lState;
  int rid;
};

class StateMachineController: public QThread
{
  Q_OBJECT
private:
  int msTimeout;
  int maxMachines;
  QVector<MachineData*> machines;
  bool m_isRunning;
  LuaConfig lConfig;
  const QJsonValue result(int id);
  QJsonValue error(QString err);
public:
  StateMachineController(): StateMachineController(1) {};
  StateMachineController(int _maxMachines, int _timeout = 10);
  ~StateMachineController();
  int newMachine();
  PIMachine *machine(int id) { return machines[id]->machine; }
  void init(lua_State *L);
  void stop() { m_isRunning = false; emit stopping(); };
signals:
  void finished();
  void stopping();
public slots:
  const QJsonValue sendEvent(const int id, const QString &ev);
  void stopMachine(const int id);
  void run();
};

#endif  // SMCONTROLLER_H