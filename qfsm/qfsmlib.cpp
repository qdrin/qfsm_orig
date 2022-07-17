/* QStateMachine submodule for Tarantool */
#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <tarantool/module.h>
#include <lua.hpp>
#include <module.h>
#include "smcontroller.h"
// #include "pimachine.h"
#include "stringtransition.h"

using namespace std;
static const char module_label[] = "__qfsm_driver";

StateMachineController *controller;

void debugStack(lua_State *L)
{
  int top = lua_gettop(L);
  QString p;
  qDebug() << "debugStack top" << top;
  for(int i=0; i < top; i++)
  {
    if(lua_isfunction(L, -i-1))
      p = "function";
    else
      p = QString(lua_tostring(L, -i-1));
    qDebug() << "debugStack L(" << -i-1 << ")" << p;
  }
}

int mlua_error(struct lua_State *L, int res, QString err)
{
  lua_pushinteger(L, res);
  lua_pushstring(L, err.toStdString().c_str());
  return 2;
}

int lua_operation_result(struct lua_State *L, QJsonValue result)
{
  QJsonObject r_res = result.toObject();
  int res = r_res["status"].toInt();
  if(res < 0)
    return mlua_error(L, res, r_res["error"].toString());
  lua_pushinteger(L, res);
  lua_pushstring(L, QJsonDocument(r_res["result"].toObject()).toJson(QJsonDocument::Compact).toStdString().c_str());
	return 2;
}

/* internal function */
static int luaPismInit(struct lua_State *L)
{
  int plen = lua_gettop(L);
  if(plen < 2) {
    lua_pop(L, plen);
    return mlua_error(L, -1, "qfsmlib: need 2 arguments: machineId and json string with state and properties attributes");
  }
  if(! lua_isnumber(L, -2)) {
    lua_pop(L, plen);
    return mlua_error(L, -1, "qfsmlib: machineId should be an integer");
  }
  int id = lua_tointeger(L, -2);
  QString state = QString(lua_tostring(L, -1));
  lua_pop(L, 2);
  QJsonDocument doc = QJsonDocument::fromJson(state.toUtf8());
  if(! doc.isObject()) {
    QString err = QString("qfsmlib.luaPismInit: 2-nd argument should be a valid json string. Got ") + state;
    qWarning() << "error" << err;
    return mlua_error(L, -1, err.toStdString().c_str());
  }
  QJsonValue jState = QJsonValue(doc.object());
  QJsonValue result = controller->machine(id)->init(jState);
  return lua_operation_result(L, result);
}

static int luaPismClose(struct lua_State *L)
{
  qInfo() << "qfsmlib.luaPismClose start";
  int id = lua_tointeger(L, -1);
  lua_pop(L, 1);
  controller->stopMachine(id);
  lua_pushinteger(L, 1);
	return 1; /* one return value */
}

static int luaPismSendEvent(struct lua_State *L)
{
  // qInfo() << "qfsmlib.luaPismSendEvent start";
  // debugStack(L);
  int plen = lua_gettop(L);
  QString param;
  if(plen < 2) {
    lua_pop(L, plen);
    return mlua_error(L, -1, "qfsmlib.luaPismSendEvent need machineId and event_type parameters");
  }
  int id = lua_tointeger(L, -2);
  QString event = QString(lua_tostring(L, -1));
  lua_pop(L, 2);
  QJsonDocument doc = QJsonDocument::fromJson(event.toUtf8());
  if(! doc.isObject()) {
    QString err = QString("qfsmlib.luaPismEvent: 2-nd argument should be a valid json string. Got ") + event;
    qWarning() << "error" << err;
    return mlua_error(L, -1, err.toStdString().c_str());
  }
  QJsonValue jEvent = QJsonValue(doc.object());
  QJsonValue result = controller->machine(id)->externalEventProcess(jEvent);
  // debugStack(L);
  // qDebug() << "luaPismSendEvent result" << result;
  return lua_operation_result(L, result);
}

static int luaPismGet(struct lua_State *L)
{
  int id = lua_tointeger(L, -1);
  lua_pop(L, 1);
  QJsonValue res = controller->machine(id)->getState();
  return lua_operation_result(L, res);
}

static int luaPismIsRunning(struct lua_State *L)
{
  // qDebug() << "luaPismIsRunning start";
  // debugStack(L);
  int id = lua_tointeger(L, -1);
  lua_pop(L, 1);
  bool res = controller->machine(id)->isRunning();
  lua_pushinteger(L, 1);
  lua_pushboolean(L, res);
  // debugStack(L);
	return 2; /* one return value */
}

static int init(struct lua_State *L)
{
  int plen = lua_gettop(L);
  if(! lua_istable(L, -1)) {
    qInfo() << "qfsmlib.init: argument must be a table";
    lua_pop(L, plen);
    return mlua_error(L, -1, "qfsmlib.init: argument must be a table");
  }
  controller->start();
  controller->init(L);
  lua_pop(L, 1);
  lua_pushinteger(L, 1);
  return 1;
}
static int new_machine(struct lua_State *L)
{
  int machine_id = controller->newMachine();
  // PIMachine *conn = controller->machine(machine_id);
  // lua_pop(L, 1);
  qDebug() << "qfsmlib.new machine_id" << machine_id;
  int num_of_result = 2;  // 3 with conn
  lua_pushinteger(L, 1);
  lua_pushinteger(L, machine_id);
  // PIMachine **conn_p = (PIMachine **)
  //   lua_newuserdata(L, sizeof(conn));
  // *conn_p = conn;
  if(machine_id >= 0) {
    luaL_getmetatable(L, module_label);
    lua_setmetatable(L, -2);
  }
  else {
    lua_pushstring(L, "create machine failed!"); 
    num_of_result++; 
  }
	return num_of_result;
}

static int stop_machine(struct lua_State *L)
{
  controller->stop();
  lua_pushinteger(L, 1);
	return 1; /* one return value */
}

/* exported function */
LUA_API "C" int luaopen_qfsm_qfsmlib(lua_State *L)
{
  int argC = 0;
  QCoreApplication app(argC, nullptr);
  controller = new StateMachineController(3);
  static const struct luaL_Reg methods [] = {
		{ "init", luaPismInit },
    { "send_event", luaPismSendEvent },
    { "get", luaPismGet },
    { "is_running", luaPismIsRunning },
    { "close", luaPismClose },
		{NULL, NULL}
	};
  luaL_newmetatable(L, module_label);
  lua_pushvalue(L, -1);
  luaL_register(L, NULL, methods);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, module_label);
	lua_setfield(L, -2, "__metatable");
  lua_pop(L, 1);
  //registering module-level methods
  lua_newtable(L);
	static const struct luaL_Reg meta [] = {
		{ "new", new_machine },
    { "init", init },
    { "stop", stop_machine },
		{NULL, NULL}
	};
  // QTimer::singleShot(0, controller, SLOT(run()));
  luaL_register(L, NULL, meta);
	return 1;
}
