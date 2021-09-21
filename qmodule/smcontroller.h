#include "pimachine.h"
#include <QTextStream>

class StateMachineController: public QObject
{
    Q_OBJECT
private:
    PIMachine *pi_machine;
public:
        StateMachineController(PIMachine *_pi_machine = nullptr): pi_machine(_pi_machine) {}
        ~StateMachineController() {
            if (pi_machine != nullptr) {
                pi_machine->stop();
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
        QTextStream cout(stdout);
        QTextStream cin(stdin);
        QString choice;
        cout << "Start processing events\n";
        while(true)
        {
            cout << "state: " << pi_machine->property("state").toString() << Qt::endl;
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