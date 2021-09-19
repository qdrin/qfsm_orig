#include <QTextStream>
#include "stringtransition.h"

StringTransition::StringTransition(const QString& _value):
    m_value(_value)
{
    QTextStream cout(stdout);
    cout << "StringTransition(\"" << m_value << "\") called\n";
}

bool StringTransition::eventTest(QEvent *e)
{
    QTextStream cout(stdout);
    cout << "eventTest called: eventType: " << e->type() << "\n";
    cout.flush();
    if (e->type() != QEvent::Type(StringEvent::type())) // StringEvent
        return false;
    StringEvent *se = static_cast<StringEvent*>(e);
    cout << "eventTest(\"" << se->value() << "\" called\n";
    return (m_value == se->value());
}

void StringTransition::onTransition(QEvent *e) {
    QTextStream cout(stdout);
    cout << "onTrasition called\n";
    cout.flush();
}
