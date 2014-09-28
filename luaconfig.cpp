#include "luaconfig.h"

LuaConfig::LuaConfig(std::string filename) :
	lua(luaL_newstate())
{
	luaL_openlibs(this->lua);
	if(luaL_loadfile(this->lua, filename)) {
		throw "Unable to open file " + filename;
	}
}

LuaConfig::~LuaConfig() {
	lua_close(this->lua);
}

int LuaConfig::get_int(std::string varname) {
	lua_getglobal(this->lua, varname);
	if(!lua_isnumber(this->lua, -1)) {
		throw varname + " is not a number.";
	}
	return(lua_tonumber(this->lua, -1));
}

std::string LuaConfig::get_string(std::string varname) {
	lua_getglobal(this->lua, varname);
	if(!lua_isstring(this->lua, -1)) {
		throw varname + " is not a string.";
	}
	return(lua_tostring(this->lua, -1));
}

