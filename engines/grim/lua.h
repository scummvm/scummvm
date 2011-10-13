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

#include "common/str.h"
#include "common/list.h"

namespace Grim {

typedef uint32 lua_Object; // from lua/lua.h

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

#define DECLARE_LUA_OPCODE(func) \
	inline static void static_##func() {\
		static_cast<LuaClass *>(LuaBase::instance())->func();\
	}\
	virtual void func()

#define LUA_OPCODE(class, func) \
	class::static_##func

class LuaObjects {
public:
	void add(float number);
	void add(int number);
	void add(const PoolObjectBase *obj);
	void add(const char *str);
	void addNil();

	void pushObjects() const;

private:
	struct Obj {
		enum {
			Nil,
			Number,
			Object,
			String
		} _type;
		union {
			float number;
			const PoolObjectBase *object;
			const char *string;
		} _value;
	};
	Common::List<Obj> _objects;
};

class LuaBase {
public:
	typedef LuaBase LuaClass;

	LuaBase();
	virtual ~LuaBase();
	inline static LuaBase *instance() { return s_instance; }

	int bundle_dofile(const char *filename);
	int single_dofile(const char *filename);

	bool getbool(int num);
	void pushbool(bool val);
	void pushobject(const PoolObjectBase *o);
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

	void update(int frameTime, int movieTime);
	void setFrameTime(float frameTime);
	void setMovieTime(float movieTime);
	virtual void registerLua();
	virtual void registerOpcodes();
	virtual void boot();

	bool callback(const char *name);
	bool callback(const char *name, const LuaObjects &objects);

	virtual void postRestoreHandle() { }

	DECLARE_LUA_OPCODE(dummyHandler);
	DECLARE_LUA_OPCODE(typeOverride);
	DECLARE_LUA_OPCODE(concatFallback);

private:
	// 0 - translate from '/msgId/'
	// 1 - don't translate - message after '/msgId'
	// 2 - return '/msgId/'
	int _translationMode;
	unsigned int _frameTimeCollection;

	int refSystemTable;
	int refTypeOverride;
	int refOldConcatFallback;
	int refTextObjectX;
	int refTextObjectY;
	int refTextObjectFont;
	int refTextObjectWidth;
	int refTextObjectHeight;
	int refTextObjectFGColor;
	int refTextObjectBGColor;
	int refTextObjectFXColor;
	int refTextObjectHIColor;
	int refTextObjectDuration;
	int refTextObjectCenter;
	int refTextObjectLJustify;
	int refTextObjectRJustify;
	int refTextObjectVolume;
	int refTextObjectBackground;
	int refTextObjectPan;

	static LuaBase *s_instance;
};

} // end of namespace Grim

#endif
