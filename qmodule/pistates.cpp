#include "pistates.h"
#include "pimachine.h"

// PISuspendingState ////////////////////////////////////////////////////////////
PISuspendingState::PISuspendingState(QState *_parent): QState(_parent) {}

void PISuspendingState::onEntry(QEvent *ev)
{
  qDebug() << "PISuspendingState::onEntry called\n";
  PIMachine *machine = static_cast<PIMachine*>(parent());
  machine->sendCallback(machine->id(), "suspend");
}

// PIProlonationState ////////////////////////////////////////////////////////////
PIProlongationState::PIProlongationState(QState *_parent): QState(_parent) {}
void PIProlongationState::onEntry(QEvent *ev)
{
  qDebug() << "PIProlongationState::onEntry called\n";
  PIMachine *machine = static_cast<PIMachine*>(parent());
  machine->sendCallback(machine->id(), "prolong");
}