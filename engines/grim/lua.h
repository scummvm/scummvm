/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_LUA_HH
#define GRIM_LUA_HH

#include "engines/grim/lua/lua.h"

namespace Grim {

class Actor;
class PoolColor;
class Costume;
class Font;
class ObjectState;
class PrimitiveObject;
class TextObject;
class TextObjectDefaults;
class TextObjectCommon;
class PoolObjectBase;

extern int refSystemTable;
extern int refTypeOverride;
extern int refOldConcatFallback;
extern int refTextObjectX;
extern int refTextObjectY;
extern int refTextObjectFont;
extern int refTextObjectWidth;
extern int refTextObjectHeight;
extern int refTextObjectFGColor;
extern int refTextObjectBGColor;
extern int refTextObjectFXColor;
extern int refTextObjectHIColor;
extern int refTextObjectDuration;
extern int refTextObjectCenter;
extern int refTextObjectLJustify;
extern int refTextObjectRJustify;
extern int refTextObjectVolume;
extern int refTextObjectBackground;
extern int refTextObjectPan;

#define DECLARE_LUA_OPCODE(func) \
	inline static void static_##func() {\
		static_cast<LuaClass *>(LuaBase::instance())->func();\
	}\
	virtual void func()

#define LUA_OPCODE(class, func) \
	class::static_##func

class LuaBase {
public:
	typedef LuaBase LuaClass;

	LuaBase();
	virtual ~LuaBase();
	inline static LuaBase *instance() { return s_instance; }

	bool getbool(int num);
	void pushbool(bool val);
	void pushobject(PoolObjectBase *o);
	int getobject(lua_Object obj);
	Actor *getactor(lua_Object obj);
	TextObject *gettextobject(lua_Object obj);
	Font *getfont(lua_Object obj);
	PoolColor *getcolor(lua_Object obj);
	PrimitiveObject *getprimitive(lua_Object obj);
	ObjectState *getobjectstate(lua_Object obj);

	virtual bool findCostume(lua_Object costumeObj, Actor *actor, Costume **costume);
	virtual Common::String parseMsgText(const char *msg, char *msgId);
	virtual void parseSayLineTable(lua_Object paramObj, bool *background, int *vol, int *pan, int *x, int *y);
	virtual void setTextObjectParams(TextObjectCommon *textObject, lua_Object tableObj);

	void setFrameTime(float frameTime);
	void setMovieTime(float movieTime);
	virtual void registerLua();
	virtual void registerOpcodes();
	virtual void boot();

	DECLARE_LUA_OPCODE(dummyHandler);
	DECLARE_LUA_OPCODE(typeOverride);
	DECLARE_LUA_OPCODE(concatFallback);

private:
	// 0 - translate from '/msgId/'
	// 1 - don't translate - message after '/msgId'
	// 2 - return '/msgId/'
	int _translationMode;

	static LuaBase *s_instance;
};

} // end of namespace Grim

#endif
