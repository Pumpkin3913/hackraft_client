#ifndef LUACONFIG_H
#define LUACONFIG_H

#include <string>
#include <vector>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class LuaConfig {
	private:
		lua_State * lua;
	public:
		LuaConfig(std::string filename);
		~LuaConfig();
		int get_int(std::string varname);
		std::string get_string(std::string varname);
		std::vector<int> get_ints(std::string varname);
		std::vector<std::string> get_strings(std::string varname);
};

#endif
