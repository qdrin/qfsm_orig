#include <QThread>
#include <QCoreApplication>
#include <QStateMachine>
#include <QAbstractTransition>
#include <QTextStream>
#include <QtWidgets/QLabel>
#include <QTimer>
#include "pimachine.h"
#include "stringtransition.h"


QTextStream cout(stdout);
QTextStream cin(stdin);

class StateMachineController: public QObject
{
    Q_OBJECT
private:
    PIMachine *pi_machine;
public:
        StateMachineController(PIMachine *_pi_machine = nullptr): pi_machine(_pi_machine) {}
        ~StateMachineController() {
            if (pi_machine != nullptr) {
                pi_machine->machine()->stop();
                delete pi_machine;
            }
        }
        PIMachine *newMachine() { pi_machine = pi_machine == nullptr ? new PIMachine : pi_machine; return pi_machine; }
signals:
    void finished();
    void externalEvent(QString);
public slots:
    void sendEvent(QString ev) { emit externalEvent(ev); };
    void run()
    {
        QString choice;
        cout << "Start processing events\n";
        while(true)
        {
            cout << "state: " << pi_machine->machine()->property("state").toString() << Qt::endl;
            cout << "Input eventType: ";
            cout.flush();
            cin >> choice;
            if(choice == "q")
            {
                break;
            }
            emit externalEvent(choice);
        }
        cout << "StateMachineController::run() finished\n";
        emit finished();
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    StateMachineController *contr = new StateMachineController;
    PIMachine *machine = contr->newMachine();

    QTimer::singleShot(0, contr, SLOT(run()));
    QObject::connect(contr, SIGNAL(finished()), &app, SLOT(quit()));
    QObject::connect(contr, &StateMachineController::externalEvent, machine, &PIMachine::externalEventProcess);
    return app.exec();
}

#include "main.moc"
