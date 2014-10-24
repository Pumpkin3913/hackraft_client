#include "luaconfig.h"

LuaConfig::LuaConfig(std::string filename) :
	lua(luaL_newstate())
{
	luaL_openlibs(this->lua);
	if(luaL_loadfile(this->lua, filename.c_str())) {
		throw "Unable to open file " + filename;
	}
	if(lua_pcall(this->lua, 0, 0, 0)) {
		throw "Unable to run file " + filename;
	}
}

LuaConfig::~LuaConfig() {
	lua_close(this->lua);
}

int LuaConfig::get_int(std::string varname) {
	lua_getglobal(this->lua, varname.c_str());
	if(!lua_isnumber(this->lua, -1)) {
		lua_pop(this->lua, 1);
		throw varname + " is not a number.";
	} else {
		int i = lua_tonumber(this->lua, -1);
		lua_pop(this->lua, 1);
		return(i);
	}
}

std::string LuaConfig::get_string(std::string varname) {
	lua_getglobal(this->lua, varname.c_str());
	if(!lua_isstring(this->lua, -1)) {
		lua_pop(this->lua, 1);
		throw varname + " is not a string.";
	} else {
		std::string s = lua_tostring(this->lua, -1);
		lua_pop(this->lua, 1);
		return(s);
	}
}

std::vector<int> LuaConfig::get_ints(std::string varname) {
	std::vector<int> ret;
	lua_getglobal(this->lua, varname.c_str());
	if(!lua_istable(this->lua, -1)) {
		lua_pop(this->lua, 1);
		throw varname + " is not a table.";
	} else {
		int n = luaL_len(this->lua, -1); // Get size of table.
		for(int i=1; i<=n; i++) {
			lua_rawgeti(this->lua, -1, i);
			if(!lua_isnumber(this->lua, -1)) {
				lua_pop(this->lua, 2);
				throw varname+"["+std::to_string(i)+"] is not a number.";
			}
			ret.push_back(lua_tonumber(this->lua, -1));
			lua_pop(this->lua, 1);
		}
	}
	return(ret);
}

std::vector<std::string> LuaConfig::get_strings(std::string varname) {
	std::vector<std::string> ret;
	lua_getglobal(this->lua, varname.c_str());
	if(!lua_istable(this->lua, -1)) {
		lua_pop(this->lua, 1);
		throw varname + " is not a table.";
	} else {
		int n = luaL_len(this->lua, -1); // Get size of table.
		for(int i=1; i<=n; i++) {
			lua_rawgeti(this->lua, -1, i);
			if(!lua_isstring(this->lua, -1)) {
				lua_pop(this->lua, 2);
				throw varname+"["+std::to_string(i)+"] is not a string.";
			}
			ret.push_back(lua_tostring(this->lua, -1));
			lua_pop(this->lua, 1);
		}
	}
	return(ret);
}

