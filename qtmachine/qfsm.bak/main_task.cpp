#include <QCoreApplication>
#include <QStateMachine>
#include <QAbstractTransition>
#include <QTextStream>
#include <QtWidgets/QLabel>
#include <QTimer>


QTextStream cout(stdout);
QTextStream cin(stdin);

//----------------------------------------------------------------
// Определяем event строку
struct StringEvent : public QEvent
{
    QString value;

    StringEvent(const QString &val)
    : QEvent(QEvent::Type(QEvent::User+1)),
      value(val) { cout << "QEvent(\"" << value << "\") called\n"; }
};


//----------------------------------------------------------------
// Класс перехода по строке
class StringTransition : public QAbstractTransition
{
    Q_OBJECT
public:
    StringTransition(const QString &value)
        : m_value(value) { cout << "StringTransition(\"" << value << "\") called\n"; }

protected:
    virtual bool eventTest(QEvent *e) override
    {
        cout << "eventTest called\n";
        cout.flush();
        if (e->type() != QEvent::Type(QEvent::User+1)) // StringEvent
            return false;
        StringEvent *se = static_cast<StringEvent*>(e);
        cout << "eventTest(\"" << se->value << "\" called\n";
        return (m_value == se->value);
    }

    virtual void onTransition(QEvent *e)  override {
        cout << "onTrasition called\n";
        cout.flush();
    }

private:
    QString m_value;
};


//----------------------------------------------------------------
class Task : public QObject
{
    Q_OBJECT
private:
    QStateMachine machine;
    int evs_ind;
protected:
    void buildStateMachine();
public:
    Task(QObject *parent = 0) : QObject(parent), evs_ind(0) {
        buildStateMachine();
    }
    ~Task() { machine.stop(); }
    void postEvent(const QString &event) {
        machine.postEvent(new StringEvent(event));
    }
signals:
    void finished();
    void signal1();
    void signal2();
    void signal3();
public slots:
    void run()
    {
        QString evs_array[] = {"2", "3", "1"};
        if (evs_ind > 2) {
            emit finished();
        }
        QString evs = evs_array[evs_ind++];
        cout << "current state: '" << machine.property("state").toString() << "'\n";
        cout.flush();
        // postEvent(evs);
        emit signal2();
        cout << "new state: '" << machine.property("state").toString() << "'\n";
        cout.flush();
        // finishing task
        run();
    }
};

void Task::buildStateMachine()
{
    QState *s1 = new QState(&machine);
    QState *s2 = new QState(&machine);
    QState *s3 = new QState(&machine);
    s1->assignProperty(&machine, "state", "1");
    s2->assignProperty(&machine, "state", "2");
    s3->assignProperty(&machine, "state", "3");
    // QFinalState *sFinal = new QFinalState();
    // machine.setProperty("state", "UNDEFINED");

    StringTransition *t1 = new StringTransition("2");
    StringTransition *t2 = new StringTransition("3");
    StringTransition *t3 = new StringTransition("1");

    t1->setTargetState(s2);
    t2->setTargetState(s3);
    t3->setTargetState(s1);

    s1->addTransition(t1);
    s2->addTransition(t2);
    s3->addTransition(t3);
    s1->addTransition(this, SIGNAL(signal2()), s2);
    s2->addTransition(this, SIGNAL(signal3()), s3);
    s3->addTransition(this, SIGNAL(signal1()), s1);

    machine.setInitialState(s1);
    machine.setProperty("state", "UNDEFINED");
    machine.start();
    QTimer::singleShot(0, this, SLOT(run()));
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Task *task = new Task(&app);
    // QObject::connect(task, &Task::finished, &app, &QCoreApplication::quit, Qt::QueuedConnection);
    QObject::connect(task, SIGNAL(finished()), &app, SLOT(quit()));
    // This will run the task from the application event loop.
    return app.exec();
}

#include "main.moc"
