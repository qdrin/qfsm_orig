#ifndef PISTATES_H
#define PISTATES_H
#include <QDebug>
#include <QState>

///////////////////////////////////////////////////////
class PISuspendingState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;  
public:
  PISuspendingState(QState *parent);
signals:
  void callbackSignal(QString);
};

///////////////////////////////////////////////////////
class PIProlongationState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;  
public:
  PIProlongationState(QState *parent);
signals:
  void callbackSignal(QString);
};
#endif // PISTATES_H
