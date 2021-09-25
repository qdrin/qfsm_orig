#include <QCoreApplication>
#include <QTimer>
#include "smcontroller.h"

StateMachineController::StateMachineController(int _maxMachines):
 maxMachines(_maxMachines), m_isRunning(false)
{
  qDebug() << "stateMachineController::stateMachineController";
  start();
}

StateMachineController::~StateMachineController()
{
  qDebug() << "stateMachineController::~StateMachineController";
  for( MachineData **m=machines.begin(); m < machines.end(); m++ )
  {
    (*m)->machine->stop();
    (*m)->thread->quit();
    (*m)->thread->wait();
  }
}

int StateMachineController::newMachine()
{
  // qDebug() << "stateMachineController::newMachine";
  if(machines.count() >= maxMachines) return -1; 
  MachineData *md = new MachineData();
  md->id = machines.count();  //TODO: Remove id from MachineData or from PIMachine ???
  md->machine = new PIMachine(md->id, this);
  md->thread = new QThread;
  machines.append(md);
  md->machine->moveToThread(md->thread);
  connect(md->machine, &PIMachine::sendCallback, this, &StateMachineController::onSendCallback);
  md->thread->start();
  md->machine->start();
  qDebug() << "stateMachineController::newMachine: new machine started. id=" << md->id << ", thread=" << md->thread;
  return md->id;
}

QString StateMachineController::sendEvent(const int id, const QString &ev)
{
  qDebug() << "stateMachineController::sendEvent(" << id << ", " << ev << ")";
  return machines[id]->machine->externalEventProcess(ev);
}

void StateMachineController::onSendCallback(const int id, QString cbName)
{
  qDebug() << "StateMachineController::onSendCallback("<< id << ", " << cbName << ") called";
  lua_rawgeti(lConfig.lState, LUA_REGISTRYINDEX, lConfig.config);
  lua_getfield(lConfig.lState, -1, "callback_func");
  if(! lua_isfunction(lConfig.lState, -1)) {
    qDebug() << "StateMachineController::onSendCallback cannot find callback";
    return;
  }  
  lua_pushinteger(lConfig.lState, id);
  lua_pushstring(lConfig.lState, cbName.toStdString().c_str());
  lua_pcall(lConfig.lState, 2,0,0);
}

void StateMachineController::run()
{
  int argC = 0;
  QCoreApplication app(argC, nullptr);
  qDebug() << "StateMachineController::run() started";
  m_isRunning = true;
  app.exec();
}

QString StateMachineController::getMachineState(const int id)
{
  qDebug() << "StateMachineController::getMachineState";
  return machines[id]->machine->property("state").toString();
}

void StateMachineController::stopMachine(const int id)
{
  qDebug() << "StateMachineController::stopMachine[" << id << "]";
  machines[id]->machine->stop();
  // delete machines[id]->machine;
  machines[id]->thread->exit();
  machines[id]->thread->deleteLater();
  machines.remove(id);
}

QString StateMachineController::initMachine(const int id, const QString &state)
{
  return machines[id]->machine->init(state);
}

bool StateMachineController::isRunning(const int id)
{
  return machines[id]->machine->isRunning();
}

void StateMachineController::init(lua_State *L)
{
  qDebug() << "StateMachineController::init called";
  lConfig.lState = L;
  int rid;
  rid = luaL_ref(L, LUA_REGISTRYINDEX);
  lConfig.config = rid;
  qDebug() << "StateMachineController::setCallback finished";
}