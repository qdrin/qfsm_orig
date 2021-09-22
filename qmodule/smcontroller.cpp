#include "smcontroller.h"

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
  md->machine = new PIMachine();
  md->thread = new QThread;
  machines.append(md);
  md->id = machines.count();
  md->machine->moveToThread(md->thread);
  connect(this, &StateMachineController::externalEvent, md->machine, &PIMachine::externalEventProcess);
  md->thread->start();
  md->machine->start();
}

void StateMachineController::sendEvent(const QString &ev)
{
  emit externalEvent(ev); 
}

void StateMachineController::run()
{
    QTextStream cout(stdout);
    QTextStream cin(stdin);
    cout << "Start processing events\n";
    m_isRunning = true;
    while(m_isRunning)
    {
      QThread::sleep(1);
      cout << ".";
    }
    cout << "StateMachineController::run() finished\n";
    emit finished();
}