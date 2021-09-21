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

class Test: public QObject
{
    Q_OBJECT
private:
    PIMachine *m_machine;
public:
        Test(PIMachine *_m_machine = nullptr): m_machine(_m_machine) {}
signals:
    void finished();
    void externalEvent(QString);
public slots:
    void run()
    {
        QString choice;
        cout << "Start processing events\n";
        while(true)
        {
            cout << "state: " << m_machine->machine()->property("state").toString() << Qt::endl;
            cout << "Input eventType: ";
            cout.flush();
            cin >> choice;
            if(choice == "q")
            {
                break;
            }
            emit externalEvent(choice);
        }
        cout << "Test::run() finished\n";
        emit finished();
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    PIMachine machine;
    Test *test = new Test(&machine);

    QTimer::singleShot(0, test, SLOT(run()));
    QObject::connect(test, SIGNAL(finished()), &app, SLOT(quit()));
    QObject::connect(test, &Test::externalEvent, &machine, &PIMachine::externalEventProcess);
    return app.exec();
}

#include "main.moc"
