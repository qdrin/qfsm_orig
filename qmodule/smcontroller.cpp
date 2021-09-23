#include <QCoreApplication>
#include "smcontroller.h"
#include <iostream>
using namespace std;

StateMachineController::~StateMachineController()
{
  for( MachineData **m=machines.begin(); m < machines.end(); m++ )
  {
    (*m)->machine->stop();
    (*m)->thread->quit();
    (*m)->thread->wait();
  }
}

int StateMachineController::newMachine()
{
  if(machines.count() >= maxMachines) return 0; 
  MachineData *md = new MachineData();
  md->id = machines.count();  //TODO: Remove id from MachineData or from PIMachine ???
  md->machine = new PIMachine(md->id);
  md->thread = new QThread;
  machines.append(md);
  md->machine->moveToThread(md->thread);
  connect(this, &StateMachineController::externalEvent, md->machine, &PIMachine::externalEventProcess);
  md->thread->start();
  md->machine->start();
  cout << "new machine started. id=" << md->id << ", thread=" << md->thread << endl; 
  return md->id;
}

void StateMachineController::sendEvent(const int id, const QString &ev)
{
  //TODO: Remove id and send event to particular thread
  emit externalEvent(id, ev); 
}

void StateMachineController::run()
{
  int argC = 0;
  QCoreApplication app(argC, nullptr);
  QTextStream cout(stdout);
  QTextStream cin(stdin);
  cout << "Start processing events\n";
  m_isRunning = true;
  app.exec();
}