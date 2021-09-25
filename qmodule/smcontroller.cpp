#include <QCoreApplication>
#include <QTimer>
#include "smcontroller.h"

StateMachineController::StateMachineController(int _maxMachines):
 maxMachines(_maxMachines), m_isRunning(false)
{
  qDebug() << "stateMachineController::stateMachineController\n";
  start();
}

StateMachineController::~StateMachineController()
{
  qDebug() << "stateMachineController::~StateMachineController\n";
  for( MachineData **m=machines.begin(); m < machines.end(); m++ )
  {
    (*m)->machine->stop();
    (*m)->thread->quit();
    (*m)->thread->wait();
  }
}

int StateMachineController::newMachine()
{
  // qDebug() << "stateMachineController::newMachine\n";
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
  qDebug() << "stateMachineController::newMachine: new machine started. id=" << md->id << ", thread=" << md->thread << Qt::endl;
  return md->id;
}

QString StateMachineController::sendEvent(const int id, const QString &ev)
{
  qDebug() << "stateMachineController::sendEvent(" << id << ", " << ev << ")\n";
  return machines[id]->machine->externalEventProcess(ev);
}

void StateMachineController::onSendCallback(const int id, QString cbName)
{
  qDebug() << "StateMachineController::onSendCallback("<< id << ", " << cbName << ") called\n";
  const char* key;
  qDebug() << "pushing" << callbacks.callbacks << "ref\n";
  lua_rawgeti(callbacks.lState, LUA_REGISTRYINDEX, callbacks.callbacks);
  if(lua_istable(callbacks.lState, -1)) qDebug() << "callback is table\n";
  lua_getfield(callbacks.lState, -1, cbName.toStdString().c_str());
  lua_pushinteger(callbacks.lState, id);
  lua_pcall(callbacks.lState, 1,0,0);
}

void StateMachineController::run()
{
  int argC = 0;
  QCoreApplication app(argC, nullptr);
  qDebug() << "StateMachineController::run() started\n";
  m_isRunning = true;
  app.exec();
}

QString StateMachineController::getMachineState(const int id)
{
  qDebug() << "StateMachineController::getMachineState\n";
  return machines[id]->machine->property("state").toString();
}

void StateMachineController::stopMachine(const int id)
{
  qDebug() << "StateMachineController::stopMachine[" << id << "]\n";
  machines[id]->machine->stop();
  // delete machines[id]->machine;
  machines[id]->thread->exit();
  machines[id]->thread->deleteLater();
  machines.remove(id);
}

QString StateMachineController::initMachine(const int id, const QString &state)
{
  qDebug() << "StateMachineController::initMachine[" << id << "]: " << state << "\n";
  return machines[id]->machine->init(state);
}

bool StateMachineController::isRunning(const int id)
{
  return machines[id]->machine->isRunning();
}

void StateMachineController::setCallbacks(lua_State *L)
{
  qDebug() << "StateMachineController::setCallbacks called\n";
  callbacks.lState = L;
  qDebug() << "LUA_REGISTRYINDEX" << LUA_REGISTRYINDEX << Qt::endl;
  int rid;
  rid = luaL_ref(L, LUA_REGISTRYINDEX);
  callbacks.callbacks = rid;
  qDebug() << "StateMachineController::setCallbacks finished\n";
}