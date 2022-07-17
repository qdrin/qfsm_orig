#ifndef PISTATES_H
#define PISTATES_H
#include <QDebug>
#include <QState>
#include <QJsonValue>
#include <QJsonObject>
#include "qfsmlib.h"

///////////////////////////////////////////////////////
// born to be initial states for activated, paymentOn and priceOn substates
// allows to choose the first nested state depending on event type (resend this event on entry from itself)
class PIEntryNestedState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIEntryNestedState(QState *parent);
signals:
  void externalSignal(QString);
};

//---------------------- usage -------------------------
///////////////////////////////////////////////////////
class PISuspendingState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PISuspendingState(QState *parent);
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
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIResumingState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIResumingState(QState *parent);
};

///////////////////////////////////////////////////////
class PISuspendedState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
  void onExit(QEvent *) override;
public:
  PISuspendedState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIPendingDisconnectState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIPendingDisconnectState(QState *parent);
};

///////////////////////////////////////////////////////
class PIDisconnectionState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIDisconnectionState(QState *parent);
};

//------------------------- price ---------------------
///////////////////////////////////////////////////////
class PIPriceOnEntryState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIPriceOnEntryState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIPriceOffState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIPriceOffState(QState *parent);
signals:
  void externalSignal(QString, QString reason = NULL);
};

///////////////////////////////////////////////////////
class PIPriceChangingState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIPriceChangingState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIPriceChangedState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
  void onExit(QEvent *) override;
public:
  PIPriceChangedState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIPriceNotChangedState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
  void onExit(QEvent *) override;
public:
  PIPriceNotChangedState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PICommercialState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PICommercialState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PITrialState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PITrialState(QState *parent);
signals:
  void externalSignal(QString);
};

//-------------------------- payment ------------------------
///////////////////////////////////////////////////////
class PIPaidState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIPaidState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIWaitingPaymentState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
  void onExit(QEvent *) override;
public:
  PIWaitingPaymentState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIPaymentOnState: public QState
{
  Q_OBJECT
protected:
  void onExit(QEvent *) override;
public:
  PIPaymentOnState(QState *parent);
signals:
  void externalSignal(QString, QString);
};

///////////////////////////////////////////////////////
class PINotPaidState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
  void onExit(QEvent *) override;
public:
  PINotPaidState(QState *parent);
signals:
  void externalSignal(QString);
};

///////////////////////////////////////////////////////
class PIPaymentStoppingState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
  void onExit(QEvent *) override;
public:
  PIPaymentStoppingState(QState *parent);
signals:
  void externalSignal(QString, QString = NULL);
};

///////////////////////////////////////////////////////
class PIPaymentStoppedState: public QState
{
  Q_OBJECT
protected:
  void onEntry(QEvent *) override;
public:
  PIPaymentStoppedState(QState *parent);
signals:
  void externalSignal(QString, QString = NULL);
};
#endif // PISTATES_H