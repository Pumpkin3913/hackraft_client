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
		throw varname + " is not a number.";
	}
	return(lua_tonumber(this->lua, -1));
}

std::string LuaConfig::get_string(std::string varname) {
	lua_getglobal(this->lua, varname.c_str());
	if(!lua_isstring(this->lua, -1)) {
		throw varname + " is not a string.";
	}
	return(lua_tostring(this->lua, -1));
}
