/* QStateMachine submodule for Tarantool */
#include <iostream>
#include <tarantool/module.h>
#include <lua.hpp>
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

/* exported function */
static const char module_label[] = "qfsmdriver";
LUA_API "C" int luaopen_qmodule_qfsmlib(lua_State *L)
{
	/* result returned from require('ckit.lib') */
  	static const struct luaL_Reg methods [] = {
		{"func", ckit_func},
		{NULL, NULL}
	};

	lua_newtable(L);
	luaL_register(L, NULL, methods);
	// lua_setfield(L, -2, "__index");
	// lua_pushstring(L, module_label);
	// lua_setfield(L, -2, "__metatable");
	// lua_pop(L, 1);
	return 1;
}
/* vim: syntax=c ts=8 sts=8 sw=8 noet */
