#ifndef STRINGEVENT_H
#define STRINGEVENT_H

#include <QEvent>
#include <QString>

class StringEvent : public QEvent
{
    const QString m_value;
    static QEvent::Type m_eventType;

public:
    static QEvent::Type type();
    QString value() { return m_value; }
    StringEvent(const QString& _value);
};
#endif // STRINGEVENT_H
