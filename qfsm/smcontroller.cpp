#include <QCoreApplication>
#include <QTimer>
#include "smcontroller.h"

StateMachineController::StateMachineController(int _maxMachines, int _timeout):
 maxMachines(_maxMachines), msTimeout(_timeout), m_isRunning(false)
{
  qInfo() << "stateMachineController::stateMachineController";
  // start();
}

StateMachineController::~StateMachineController()
{
  qInfo() << "stateMachineController::~StateMachineController";
  for( MachineData **m=machines.begin(); m < machines.end(); m++ )
  {
    (*m)->machine->stop();
    (*m)->thread->quit();
  }
  emit finished();
}

QJsonValue StateMachineController::error(QString err)
{
  QJsonObject res;
  res["status"] = -1;
  res["error"] = err;
  return QJsonValue(res);
}

int StateMachineController::newMachine()
{
  qInfo() << "stateMachineController::newMachine";
  if(machines.count() >= maxMachines) return -1; 
  MachineData *md = new MachineData();
  md->id = machines.count();  //TODO: Remove id from MachineData or from PIMachine ???
  md->machine = new PIMachine(md->id, this, msTimeout);
  md->thread = new QThread;
  machines.append(md);
  md->machine->moveToThread(md->thread);
  qInfo() << "stateMachineController::newMachine: starting machine" << md->id;
  md->thread->start();
  md->machine->start();
  qInfo() << "stateMachineController::newMachine: machine started. id=" << md->id << ", thread=" << md->thread;
  return md->id;
}

const QJsonValue StateMachineController::sendEvent(const int id, const QString &ev)
{
  MachineData &mdata = *(machines[id]);
  PIMachine *m = mdata.machine;
  return m->externalEventProcess(ev);
}

void StateMachineController::run()
{
  int argC = 0;
  QCoreApplication app(argC, nullptr);
  qInfo() << "StateMachineController::run() started";
  connect(this, &StateMachineController::stopping, &app, &QCoreApplication::quit);
  m_isRunning = true;
  app.exec();
}

void StateMachineController::stopMachine(const int id)
{
  qInfo() << "StateMachineController::stopMachine[" << id << "]";
  machines[id]->machine->stop();
  // delete machines[id]->machine;
  machines[id]->thread->exit();
  machines[id]->thread->deleteLater();
  machines.remove(id);
}

void StateMachineController::init(lua_State *L)
{
  qInfo() << "StateMachineController::init called";
  lConfig.lState = L;
  int rid;
  rid = luaL_ref(L, LUA_REGISTRYINDEX);
  lConfig.rid = rid;
  lua_rawgeti(L, LUA_REGISTRYINDEX, rid);
  lua_getfield(L, -1, "max_machines");
  int mm = lua_tointeger(L, -1);
  maxMachines = (mm == 0 ? maxMachines : mm);
  qInfo() << "StateMachineController::init maxMachines=" << maxMachines;
  lua_getfield(L, -2, "ms_timeout");
  mm = lua_tointeger(L, -1);
  msTimeout = (mm == 0 ? msTimeout : mm);
  qInfo() << "StateMachineController::init msTimeout=" << msTimeout;
  qInfo() << "StateMachineController::init finished";
}