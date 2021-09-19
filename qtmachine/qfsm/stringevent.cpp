#include "stringevent.h"
#include <QTextStream>

QEvent::Type StringEvent::m_eventType = QEvent::None;

StringEvent::StringEvent(const QString& _value):
  QEvent(QEvent::Type(StringEvent::type())),
  m_value(_value)
{
    QTextStream cout(stdout);
    cout << "QEvent(\"" << m_value << "\") called\n";
}

QEvent::Type StringEvent::type()
{
    if(m_eventType == QEvent::None)
    {
        int generatedType = QEvent::registerEventType();
        m_eventType = static_cast<QEvent::Type>(generatedType);
    }
    return m_eventType;
}
