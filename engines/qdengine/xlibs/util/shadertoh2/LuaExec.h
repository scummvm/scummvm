#ifndef _LUAEXEC_H_
#define _LUAEXEC_H_

class LuaExec
{
	struct lua_State * lua;
public:
	LuaExec();
	~LuaExec();
	bool open(const char* buffer,int size,const char* chunk_name);
	void set(const char* name,float value);
	void set(const char* name,int value);
	void set(const char* name,const char* value);
	string run();
};

#endif  _LUAEXEC_H_
