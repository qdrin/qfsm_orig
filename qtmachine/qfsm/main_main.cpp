#include <QTimer>
#include <QStateMachine>
#include <QAbstractTransition>
#include <QTextStream>
#include <QtWidgets/QLabel>


QTextStream cout(stdout);
QTextStream cin(stdin);

// Определяем event строку
struct StringEvent : public QEvent
{
    StringEvent(const QString &val)
    : QEvent(QEvent::Type(QEvent::User+1)),
      value(val) { cout << "QEvent(\"" << value << "\") called\n"; }

    QString value;
};


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
        if (e->type() != QEvent::Type(QEvent::User+1)) // StringEvent
            return false;
        StringEvent *se = static_cast<StringEvent*>(e);
        return (m_value == se->value);
    }

    virtual void onTransition(QEvent *e)  override {
        cout << "onTrasition called\n";
    }

private:
    QString m_value;
};

int main(int argc, char *argv[])
{
    QString evs_array[] = {"transitionTo2", "transitionTo3", "transitionTo1"};
    QString evs;
    QEvent *ev;
    QString state;
    QStateMachine machine;
    QState *s1 = new QState(&machine);
    QState *s2 = new QState(&machine);
    QState *s3 = new QState(&machine);
    s1->assignProperty(&machine, "state", "1");
    s2->assignProperty(&machine, "state", "2");
    s3->assignProperty(&machine, "state", "3");
    // QFinalState *sFinal = new QFinalState();
    // machine.setProperty("state", "UNDEFINED");

    StringTransition *t1 = new StringTransition("transitionTo2");
    StringTransition *t2 = new StringTransition("transitionTo3");
    StringTransition *t3 = new StringTransition("transitionTo1");

    t1->setTargetState(s2);
    t2->setTargetState(s3);
    t3->setTargetState(s1);

    s1->addTransition(t1);
    s2->addTransition(t2);
    s3->addTransition(t3);

    machine.setInitialState(s1);
    machine.setProperty("state", "UNDEFINED");
    cout << "starting StateMachine\n";
    QTimer::singleShot( 0, &machine, SLOT( start() ) );
    machine.start();
    cout << "isRunning(): " << machine.isRunning() << Qt::endl;
    cout << "active(): " << machine.active() << Qt::endl;
    cout << "current state: '" << machine.property("state").toString() << "'\n";
    cout.flush();
    for(int i=0; i < 3; i++)
    {
        evs = evs_array[i];
        cout << "processing event '" << evs << "'\n";
        cout.flush();
        ev = new StringEvent(evs);
        // machine.postEvent(ev);
        state = machine.property("state").toString();
        cout << "new state: '" << state << "'\n";
        cout.flush();
    }
    machine.stop();
    cout << "Finish\n";
    return 0;
}

#include "main.moc"
