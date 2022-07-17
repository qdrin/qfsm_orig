#ifndef PIMACHINE_H
#define PIMACHINE_H

#include <QDebug>
#include <QObjectList>
#include <QStateMachine>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include "lua.hpp"
#include "qfsmlib.h"

struct LC {
  lua_State *lState;
  int directFuncRef;
  int callbackFuncRef;
  int rid;
};

enum ProductStates {ACTIVE, ACTIVE_TRIAL, ABORTED, PENDING_ACTIVATE, SUSPENDED, PENDING_DISCONNECT, DISCONNECT, UNDEFINED};
enum OperationStates {SUCCESS, ERROR, PENDING, IDLE, INIT};

struct MachineStatus {
  bool busy;
  bool hasTransition;
  OperationStates opState;
  QJsonValue result;
  int reason;
  QString error;
};

class PIMachine: public QStateMachine
{
    Q_OBJECT
private:
  static const QList<QString> m_processedProperties;
  const int msTimeout;
  QJsonObject m_config;       // Данные из конфиг-файла
  QJsonObject m_product;      // поля продукта
  QJsonObject m_properties;   // Вычисляемые данные из внешнего мира, не из БД
  QJsonArray m_tasks;         // Задания, которые надо создать
  QJsonArray m_deleteTasks;   // Задания, которые надо удалить
  MachineStatus status;
  bool busy;
  const int m_id;
  QObject *m_parent;  // delete?
  LC lConfig;  // Чтобы вызывать колбэки и прямые функции типа state_eol
  void buildMachine();
  QJsonObject getMachineState(const QObject *from);
  bool loadStateOnly(QState* from, const QJsonObject &state);
  bool loadState(QState* from, const QJsonValue &data);
  void resetStatus()
    {status.opState = IDLE; status.hasTransition = false, status.reason = 1; status.result=QJsonValue::Null; status.error = QString(); status.busy=false;};
  void resetInitialStates();
public:
  const static QMap<ProductStates, QString> statusMap;
  PIMachine(const int _id, QObject *_parent = nullptr, const int _timeout = 10);
  int id() { return m_id; }
  const MachineStatus &getOperationStatus() { return status; }
  bool isBusy() { return status.busy; };
  void release() { status.busy = false; emit released(); };
  QJsonValue getState();
  const QJsonValue result();
  const QJsonValue externalEventProcess(const QJsonValue event);
  bool setHasTransition(bool _status) { status.hasTransition = _status; return status.hasTransition; }
  // const QString currentTariffMode();  // TODO: Remove
  // const QString nextTariffMode();     // TODO: Remove
  const QJsonValue currentPrice(QString key = "");
  void createTask(const QJsonValue task);
  void deleteTask(const QJsonValue task);
  int suspendDuration();
  QJsonValue config(QString key);
  QJsonValue product(QString key);
  QJsonValue setProduct(QString key, QJsonValue val);
  int targetStateEol(QString curState);
  int termDate(QString reason);
  bool pricesEquals();
  QJsonValue prop(QString key) { return m_properties[key]; };
  void increasePeriods(int increment = 1);
signals:
  void externalSignal(const QString eventType);
  void hasStopped();
  void hasStarted();
  void released();  // Ушел в "свободно"
  void occupied();  // Ушел в "занято"
  void machineError(const QString error, const int reason = -1);
  void transitionCompleted();
public slots:
  void onTransitionCompleted();
  QJsonValue init(const QJsonValue &state);
  void onMachineError(const QString err, const int reason = -1);
  // void onDebug() {};
};
#endif // PIMACHINE_H
