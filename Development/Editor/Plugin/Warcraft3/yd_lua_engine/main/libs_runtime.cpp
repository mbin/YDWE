#include <base/lua/state.h>
#include <base/util/console.h>
#include <cstring>
#include "libs_runtime.h"

namespace base { namespace warcraft3 { namespace lua_engine {

	namespace runtime
	{
		int  version = 1;
		int  error_handle = 0;
		int  handle_level = 2;
		bool console = false;
		bool sleep = true;
		bool catch_crash = true;

		int  handle_ud_table = 0;
		int  thread_table = 0;
		int  callback_table = 0;

		void initialize()
		{
			error_handle = 0;
			handle_level = 2;
			console = false;
			sleep = true;
			catch_crash = true;

			handle_ud_table = 0;
			thread_table = 0;
			callback_table = 0;
		}

		void set_function(int& result, lua_State* L, int index)
		{
			if (lua_isfunction(L, index))
			{
				if (result != 0)
				{
					luaL_unref(L, LUA_REGISTRYINDEX, result);
				}

				lua_pushvalue(L, index);
				result = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			else if (lua_isnil(L, index))
			{
				if (result != 0)
				{
					luaL_unref(L, LUA_REGISTRYINDEX, result);
				}

				result = 0;
			}
		}

		void get_function(int result, lua_State* L)
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, result);
		}

		int thread_get_table(lua::state* ls)
		{
			if (runtime::thread_table == 0)
			{
				ls->newtable();
				{
					ls->newtable();
					{
						ls->pushstring("__mode");
						ls->pushstring("kv");
						ls->rawset(-3);
					}
					ls->setmetatable(-2);
				}
				runtime::thread_table = luaL_ref(ls->self(), LUA_REGISTRYINDEX);
			}
			lua_rawgeti(ls->self(), LUA_REGISTRYINDEX, runtime::thread_table);
			return 1;
		}

		int thread_create(lua::state* ls, int index)
		{
			thread_get_table(ls);
			ls->pushvalue(index);
			ls->rawget(-2);
			if (ls->isnil(-1))
			{
				ls->pop(1);
				ls->newthread();
			}
			else
			{
				ls->pushvalue(index);
				ls->pushnil();
				ls->rawset(-4);
			}
			ls->remove(-2);
			return 1;
		}

		int thread_save(lua::state* ls, int key, int value)
		{
			thread_get_table(ls);
			ls->pushvalue(key);
			ls->pushvalue(value);
			ls->rawset(-3);
			ls->pop(1);
			return 0;
		}

		int handle_ud_get_table(lua::state* ls)
		{
			if (runtime::handle_ud_table == 0)
			{
				ls->newtable();
				{
					ls->newtable();
					{
						ls->pushstring("__mode");
						ls->pushstring("kv");
						ls->rawset(-3);
					}
					ls->setmetatable(-2);
				}
				runtime::handle_ud_table = luaL_ref(ls->self(), LUA_REGISTRYINDEX);
			}
			lua_rawgeti(ls->self(), LUA_REGISTRYINDEX, runtime::handle_ud_table);
			return 1;
		}
	}

	int jass_runtime_set(lua_State* L)
	{
		lua::state* ls = (lua::state*)L;
		const char* name = ls->tostring(2);

		if (strcmp("error_handle", name) == 0)
		{
			runtime::set_function(runtime::error_handle, ls->self(), 3);
		}
		else if (strcmp("handle_level", name) == 0)
		{
			runtime::handle_level = ls->checkinteger(3);
		}
		else if (strcmp("console", name) == 0)
		{
			runtime::console = !!ls->toboolean(3);
			if (runtime::console)
			{
				console::enable();
				console::disable_close_button();
			}
			else
			{
				console::disable();
			}
		}
		else if (strcmp("sleep", name) == 0)
		{
			runtime::sleep = !!ls->toboolean(3);
		}
		else if (strcmp("catch_crash", name) == 0)
		{
			runtime::catch_crash = !!ls->toboolean(3);
		}
		
		return 0;
	}

	int jass_runtime_get(lua_State* L)
	{
		lua::state* ls = (lua::state*)L;
		const char* name = ls->tostring(2);

		if (strcmp("version", name) == 0)
		{
			ls->pushinteger(runtime::version);
			return 1;
		}
		else if (strcmp("error_handle", name) == 0)
		{
			runtime::get_function(runtime::error_handle, ls->self());
			return 1;
		}
		else if (strcmp("handle_level", name) == 0)
		{
			ls->pushinteger(runtime::handle_level);
			return 1;
		}
		else if (strcmp("console", name) == 0)
		{
			ls->pushboolean(runtime::console);
			return 1;
		}
		else if (strcmp("sleep", name) == 0)
		{
			ls->pushboolean(runtime::sleep);
			return 1;
		}
		else if (strcmp("catch_crash", name) == 0)
		{
			ls->pushboolean(runtime::catch_crash);
			return 1;
		}

		return 0;
	}

	int jass_runtime(lua_State *L)
	{
		lua::state* ls = (lua::state*)L;
		ls->newtable();
		{
			ls->newtable();
			{
				ls->pushstring("__index");
				ls->pushcclosure((lua::state::cfunction)jass_runtime_get, 0);
				ls->rawset(-3);

				ls->pushstring("__newindex");
				ls->pushcclosure((lua::state::cfunction)jass_runtime_set, 0);
				ls->rawset(-3);
			}
			ls->setmetatable(-2);
		}
		return 1;
	}

}}}