/* QStateMachine submodule for Tarantool */
#include <iostream>
#include <tarantool/module.h>
#include <lua.hpp>
#include "pimachine.h"
#include "stringtransition.h"
#include "smcontroller.h"

using namespace std;
static const char module_label[] = "__qfsm_driver";

/* internal function */
static int luaPismInit(struct lua_State *L)
{
  cout << "luaPismInit called\n";
  lua_pushinteger(L, 1);
	return 1; /* one return value */
}

static int luaPismClose(struct lua_State *L)
{
  cout << "luaPismClose called\n";
  lua_pushinteger(L, 1);
	return 1; /* one return value */
}

static int luaPismSendEvent(struct lua_State *L)
{
  cout << "luaPismSendEvent called\n";
  lua_pushinteger(L, 1);
  lua_pushstring(L, "luaPismSendEvent");
	return 2; /* one return value */
}

static int luaPismGet(struct lua_State *L)
{
  cout << "luaPismGet called\n";
  lua_pushinteger(L, 1);
  lua_pushstring(L, "luaPismGet");
	return 2; /* one return value */
}

static int new_machine(struct lua_State *L)
{
  cout << "new_machine called\n";
  PIMachine *machine = new PIMachine(nullptr);
  lua_pushinteger(L, 1);
  PIMachine **m = (PIMachine **)
    lua_newuserdata(L, sizeof(machine));
  *m = machine;
  cout << "machine address: " << *m << endl;
  luaL_getmetatable(L, module_label);
  lua_setmetatable(L, -2);
	return 2; /* one return value */
}

/* exported function */
LUA_API "C" int luaopen_qmodule_qfsmlib(lua_State *L)
{
  static const struct luaL_Reg methods [] = {
		{ "init", luaPismInit },
    { "send_event", luaPismSendEvent },
    { "get", luaPismGet },
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
		{NULL, NULL}
	};
	luaL_register(L, NULL, meta);
	return 1;
}
/* vim: syntax=c ts=8 sts=8 sw=8 noet */
