/* QStateMachine submodule for Tarantool */
#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <QString>
#include <QThread>
#include <QDebug>
#include <tarantool/module.h>
#include <lua.hpp>
#include "pimachine.h"
#include "stringtransition.h"
#include "smcontroller.h"

using namespace std;
static const char module_label[] = "__qfsm_driver";
StateMachineController *controller;

/* internal function */
static int luaPismInit(struct lua_State *L)
{
  int plen = lua_gettop(L);
  if(plen < 2) {
    lua_pushinteger(L, -1);
    lua_pushstring(L, "need machineId and event_type parameters");
    return 2;
  }
  int id = lua_tointeger(L, -2);
  const char* state = lua_tostring(L, -1);
  qDebug() << "luaPismSendEvent: id=" << id << ", state: " << state << Qt::endl;
  QString result = controller->initMachine(id, state);
  lua_pushinteger(L, 1);
  lua_pushstring(L, result.toStdString().c_str());
	return 2; /* one return value */
}

static int luaPismClose(struct lua_State *L)
{
  qDebug() << "luaPismClose called\n";
  int id = lua_tointeger(L, -1);
  controller->stopMachine(id);
  lua_pushinteger(L, 1);
	return 1; /* one return value */
}

static int luaPismSendEvent(struct lua_State *L)
{
  int plen = lua_gettop(L);
  if(plen < 2) {
    lua_pushinteger(L, -1);
    lua_pushstring(L, "need machineId and event_type parameters");
    return 2;
  }
  int id = lua_tointeger(L, -2);
  const char* ev = lua_tostring(L, -1);
  qDebug() << "luaPismSendEvent: id=" << id << ", event: " << ev << Qt::endl;
  QString result = controller->sendEvent(id, ev);
  lua_pushinteger(L, 1);
  lua_pushstring(L, result.toStdString().c_str());
	return 2;
}

static int luaPismGet(struct lua_State *L)
{
  int id = lua_tointeger(L, -1);
  qDebug() << "luaPismGet called with id=" << id << "\n";
  QString res = controller->getMachineState(id);
  lua_pushinteger(L, 1);
  lua_pushstring(L, res.toStdString().c_str());
	return 2; /* one return value */
}

static int luaPismIsRunning(struct lua_State *L)
{
  int id = lua_tointeger(L, -1);
  qDebug() << "luaPismIsRunning called with id=" << id << "\n";
  bool res = controller->isRunning(id);
  lua_pushinteger(L, 1);
  lua_pushboolean(L, res);
	return 2; /* one return value */
}

static int set_callbacks(struct lua_State *L)
{
  qDebug() << "qfsmlib.set_callback called\n";
  if(! lua_istable(L, -1)) {
    qDebug() << "qfsmlib.set_callbacks: argument must be a table\n";
    lua_pushinteger(L, -1);
    lua_pushstring(L, "qfsmlib.set_callbacks: argument must be a table");
    return 2;
  }
  lua_State *cb = luaL_newstate();
  lua_newtable(cb);
  lua_getfield(L, -1, "suspend");
  controller->setCallbacks(L);
  // lua_getfield(L, -1, "suspend");
  // lua_pcall(L, 0,0,0);
  // lua_getfield(L, -1, "prolong");
  // lua_pcall(L, 0, 0, 0);
  lua_pushinteger(L, 1);
  return 1;
}
static int new_machine(struct lua_State *L)
{
  qDebug() << "qfsmlib.new_machine called\n";
  lua_pushinteger(L, 1);
  int machine_id = controller->newMachine();
  qDebug() << "new_machine id=" << machine_id << Qt::endl;
  int num_of_result = 2;
  lua_pushinteger(L, machine_id);
  if(machine_id >= 0) {
    luaL_getmetatable(L, module_label);
    lua_setmetatable(L, -2);
  }
  else {
    lua_pushstring(L, "create new machine failed!");  
  }
	return num_of_result;
}

static int stop_machine(struct lua_State *L)
{
  qDebug() << "stop_machine called\n";
  controller->stop();
  lua_pushinteger(L, 1);
	return 1; /* one return value */
}

/* exported function */
LUA_API "C" int luaopen_qmodule_qfsmlib(lua_State *L)
{
  int argC = 0;
  QCoreApplication app(argC, nullptr);
  controller = new StateMachineController(2);
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
    { "set_callbacks", set_callbacks },
    { "stop", stop_machine },
		{NULL, NULL}
	};
  // QTimer::singleShot(0, controller, SLOT(run()));
	luaL_register(L, NULL, meta);
	return 1;
}
