/* QStateMachine submodule for Tarantool */
#include <iostream>
#include <tarantool/module.h>
#include <lua.hpp>
#include "pimachine.h"
// #include <lua.h>
// #include <lualib.h>
// #include <lauxlib.h>

/* internal function */
static int ckit_func(struct lua_State *L)
{
	if (lua_gettop(L) < 2)
		luaL_error(L, "Usage: ckit_func(a: number, b: number)");
	int a = lua_tointeger(L, 1);
	int b = lua_tointeger(L, 2);
  std::cout << "Sum: " << a << " and " << b << std::endl;
	lua_pushinteger(L, a + b);
	return 1; /* one return value */
}

static int new_machine(struct lua_State *L)
{
  std::cout << "new_machine started\n";
  PIMachine *machine = new PIMachine;
  lua_pushstring(L, "PIMachine");
	return 1; /* one return value */
}

/* exported function */
static const char module_label[] = "qfsmdriver";
LUA_API "C" int luaopen_qmodule_qfsmlib(lua_State *L)
{
  lua_newtable(L);
  static const struct luaL_Reg methods [] = {
		{"cadd", ckit_func},
    {"new", new_machine},
		{NULL, NULL}
	};
  luaL_register(L, NULL, methods);
	return 1;
}
/* vim: syntax=c ts=8 sts=8 sw=8 noet */
