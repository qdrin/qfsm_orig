#include <QCoreApplication>
#include <QTimer>
#include "smcontroller.h"
#include <iostream>
using namespace std;

StateMachineController::StateMachineController(int _maxMachines):
 maxMachines(_maxMachines), m_isRunning(false)
{
  cout << "stateMachineController::stateMachineController\n";
  start();
  // evLoop = new QEventLoop;f
}

StateMachineController::~StateMachineController()
{
  cout << "stateMachineController::~StateMachineController\n";
  for( MachineData **m=machines.begin(); m < machines.end(); m++ )
  {
    (*m)->machine->stop();
    (*m)->thread->quit();
    (*m)->thread->wait();
  }
}

int StateMachineController::newMachine()
{
  cout << "stateMachineController::newMachine\n";
  if(machines.count() >= maxMachines) return -1; 
  MachineData *md = new MachineData();
  md->id = machines.count();  //TODO: Remove id from MachineData or from PIMachine ???
  md->machine = new PIMachine(md->id, this);
  md->thread = new QThread;
  machines.append(md);
  md->machine->moveToThread(md->thread);
  connect(this, &StateMachineController::externalEvent, md->machine, &PIMachine::externalEventProcess);
  md->thread->start();
  md->machine->start();
  cout << "new machine started. id=" << md->id << ", thread=" << md->thread << endl;
  cout << "machine isRunning(): " << md->machine->isRunning() << "\n";
  return md->id;
}

QString StateMachineController::sendEvent(const int id, const QString &ev)
{
  cout << "stateMachineController::sendEvent(" << id << ", " << ev.toStdString() << ")\n";
  return machines[id]->machine->externalEventProcess(ev);
}

void StateMachineController::run()
{
  int argC = 0;
  QCoreApplication app(argC, nullptr);
  cout << "StateMachineController::run() started\n";
  m_isRunning = true;
  app.exec();
}

QString StateMachineController::getMachineState(const int id)
{
  cout << "StateMachineController::getMachineState\n";
  return machines[id]->machine->property("state").toString();
}

void StateMachineController::stopMachine(const int id)
{
  cout << "StateMachineController::stopMachine[" << id << "]\n";
  machines[id]->machine->stop();
  // delete machines[id]->machine;
  machines[id]->thread->deleteLater();
  machines.remove(id);
}

QString StateMachineController::initMachine(const int id, const QString &state)
{
  cout << "StateMachineController::initMachine[" << id << "]: " << state.toStdString() << "\n";
  return machines[id]->machine->init(state);
}