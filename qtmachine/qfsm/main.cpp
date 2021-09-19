#include <QThread>
#include <QCoreApplication>
#include <QStateMachine>
#include <QAbstractTransition>
#include <QTextStream>
#include <QtWidgets/QLabel>
#include <QTimer>
#include "pimachine.h"
#include "stringevent.h"
#include "stringtransition.h"


QTextStream cout(stdout);
QTextStream cin(stdin);

class Test: public QObject
{
    Q_OBJECT
private:
    PIMachine *m_machine;
public:
        Test(PIMachine *_m_machine = NULL): m_machine(_m_machine) {}
signals:
    void finished();
public slots:
    void run()
    {
        QEvent *ev;
        QString evs_array[] = {"2", "3", "1", "q"};
        QString choice;
        cout << "Start processing events\n";
        for(int i=0; i < 4; i++)
        {
            cout << "machine.isRunning(): " << m_machine->machine()->isRunning() << ", " <<
                 "state: " << m_machine->property("state").toString() << Qt::endl;
            choice = evs_array[i];
            if(choice == "q")
            {
                break;
            }
            // ev = new StringEvent(choice);
            // cout << "Test. new event: " << ev->type() << ", value=" << ((StringEvent *)ev)->value() << Qt::endl;
            m_machine->postEvent(choice);
            QThread::sleep(1);
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
    return app.exec();
}

#include "main.moc"
