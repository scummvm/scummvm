/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/foreach.h"
#include "common/system.h"
#include "common/events.h"

#include "math/matrix3.h"

#include "engines/grim/debug.h"
#include "engines/grim/lua.h"
#include "engines/grim/actor.h"
#include "engines/grim/costume.h"
#include "engines/grim/registry.h"
#include "engines/grim/localize.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/set.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/model.h"
#include "engines/grim/primitives.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lualib.h"

namespace Grim {

void LuaObjects::add(float number) {
	Obj obj;
	obj._type = Obj::Number;
	obj._value.number = number;
	_objects.push_back(obj);
}

void LuaObjects::add(int number) {
	Obj obj;
	obj._type = Obj::Number;
	obj._value.number = number;
	_objects.push_back(obj);
}

void LuaObjects::add(const PoolObjectBase *object) {
	Obj obj;
	obj._type = Obj::Object;
	obj._value.object = object;
	_objects.push_back(obj);
}

void LuaObjects::add(const char *str) {
	Obj obj;
	obj._type = Obj::String;
	obj._value.string = str;
	_objects.push_back(obj);
}

void LuaObjects::addNil() {
	Obj obj;
	obj._type = Obj::Nil;
	_objects.push_back(obj);
}

void LuaObjects::pushObjects() const {
	for (Common::List<Obj>::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		const Obj &o = *i;
		switch (o._type) {
			case Obj::Nil:
				lua_pushnil();
				break;
			case Obj::Number:
				lua_pushnumber(o._value.number);
				break;
			case Obj::Object:
				LuaBase::instance()->pushobject(o._value.object);
				break;
			case Obj::String:
				lua_pushstring(o._value.string);
				break;
		}
	}
}


LuaBase *LuaBase::s_instance = nullptr;

LuaBase::LuaBase() :
		_translationMode(0), _frameTimeCollection(0) {
	s_instance = this;

	lua_iolibopen();
	lua_strlibopen();
	lua_mathlibopen();
}

LuaBase::~LuaBase() {
	s_instance = nullptr;

	lua_removelibslists();
	lua_close();
	lua_iolibclose();
}

// Entries in the system table
static struct {
	const char *name;
	int key;
} system_defaults[] = {
	{ "frameTime", 0 },
	{ "movieTime", 0 }
};

void LuaBase::registerLua() {
	// Register system table
	lua_Object system_table = lua_createtable();
	lua_pushobject(system_table);
	lua_setglobal("system");

	lua_pushobject(system_table);
	refSystemTable = lua_ref(1);

	for (unsigned i = 0; i < ARRAYSIZE(system_defaults); i++) {
		lua_pushobject(lua_getref(refSystemTable));
		lua_pushstring(system_defaults[i].name);
		lua_pushnumber(system_defaults[i].key);
		lua_settable();
	}

	// Create and populate system.controls table
	lua_Object controls_table = lua_createtable();
	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("controls");
	lua_pushobject(controls_table);
	lua_settable();

	for (int i = 0; controls[i].name; i++) {
		lua_pushobject(controls_table);
		lua_pushstring(controls[i].name);
		lua_pushnumber(controls[i].key);
		lua_settable();
	}

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("camChangeHandler");
	lua_pushcfunction(LUA_OPCODE(LuaBase, dummyHandler));
	lua_settable();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("axisHandler");
	lua_pushcfunction(LUA_OPCODE(LuaBase, dummyHandler));
	lua_settable();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("buttonHandler");
	lua_pushcfunction(LUA_OPCODE(LuaBase, dummyHandler));
	lua_settable();

	lua_pushobject(lua_getglobal("type"));
	refTypeOverride = lua_ref(true);
	lua_pushCclosure(LUA_OPCODE(LuaBase, typeOverride), 0);
	lua_setglobal("type");

	// Register constants for box types
	lua_pushnumber(Sector::NoneType);
	lua_setglobal("NONE");
	lua_pushnumber(Sector::WalkType);
	lua_setglobal("WALK");
	lua_pushnumber(Sector::CameraType);
	lua_setglobal("CAMERA");
	lua_pushnumber(Sector::SpecialType);
	lua_setglobal("SPECIAL");
	lua_pushnumber(Sector::HotType);
	lua_setglobal("HOT");

	lua_pushobject(lua_setfallback("concat", LUA_OPCODE(LuaBase, concatFallback)));
	refOldConcatFallback = lua_ref(1);

	// initialize Text globals
	lua_pushstring("x");
	refTextObjectX = lua_ref(true);
	lua_pushstring("y");
	refTextObjectY = lua_ref(true);
	lua_pushstring("font");
	refTextObjectFont = lua_ref(true);
	lua_pushstring("width");
	refTextObjectWidth = lua_ref(true);
	lua_pushstring("height");
	refTextObjectHeight = lua_ref(true);
	lua_pushstring("fgcolor");
	refTextObjectFGColor = lua_ref(true);
	lua_pushstring("bgcolor");
	refTextObjectBGColor = lua_ref(true);
	lua_pushstring("fxcolor");
	refTextObjectFXColor = lua_ref(true);
	lua_pushstring("hicolor");
	refTextObjectHIColor = lua_ref(true);
	lua_pushstring("duration");
	refTextObjectDuration = lua_ref(true);
	lua_pushstring("center");
	refTextObjectCenter = lua_ref(true);
	lua_pushstring("ljustify");
	refTextObjectLJustify = lua_ref(true);
	lua_pushstring("rjustify");
	refTextObjectRJustify = lua_ref(true);
	lua_pushstring("volume");
	refTextObjectVolume = lua_ref(true);
	lua_pushstring("pan");
	refTextObjectPan = lua_ref(true);
	lua_pushstring("background");
	refTextObjectBackground = lua_ref(true);
	lua_pushstring("layer");
	refTextObjectLayer = lua_ref(true);
	lua_pushstring("coords");
	refTextObjectCoords = lua_ref(true);
}

void LuaBase::forceDemo() {
	lua_pushnumber(1);
	lua_setglobal("DEMO");
}

struct luaL_reg baseOpcodes[] = {
	{ "  concatfallback", LUA_OPCODE(LuaBase, concatFallback) },
	{ "  typeoverride", LUA_OPCODE(LuaBase, typeOverride) },
	{ "  dfltcamera", LUA_OPCODE(LuaBase, dummyHandler) },
	{ "  dfltcontrol", LUA_OPCODE(LuaBase, dummyHandler) },
};

void LuaBase::registerOpcodes() {
	luaL_openlib(baseOpcodes, ARRAYSIZE(baseOpcodes));
}

void LuaBase::loadSystemScript() {
	dofile("_system.lua");
}

void LuaBase::boot() {
	lua_pushnil();      // resumeSave
	lua_pushnil();      // bootParam - not used in scripts
	lua_call("BOOT");
}

void LuaBase::update(int frameTime, int movieTime) {
	_frameTimeCollection += frameTime;
	if (_frameTimeCollection > 10000) {
		_frameTimeCollection = 0;
		lua_collectgarbage(0);
	}

	lua_beginblock();
	setFrameTime(frameTime);
	lua_endblock();

	lua_beginblock();
	setMovieTime(movieTime);
	lua_endblock();

	// Run asynchronous tasks
	lua_runtasks();
}

void LuaBase::setFrameTime(float frameTime) {
	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("frameTime");
	lua_pushnumber(frameTime);
	lua_settable();
}

void LuaBase::setMovieTime(float movieTime) {
	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("movieTime");
	lua_pushnumber(movieTime);
	lua_settable();
}

int LuaBase::dofile(const char *filename) {
	Common::SeekableReadStream *stream;
	stream = g_resourceloader->openNewStreamFile(filename);
	if (!stream) {
		Debug::warning(Debug::Engine, "Cannot find script %s", filename);
		return 2;
	}

	int32 size = stream->size();
	char *buffer = new char[size];
	stream->read(buffer, size);
	int result = lua_dobuffer(const_cast<char *>(buffer), size, const_cast<char *>(filename));
	delete stream;
	delete[] buffer;
	return result;
}

bool LuaBase::callback(const char *name) {
	LuaObjects o;
	return callback(name, o);
}

bool LuaBase::callback(const char *name, const LuaObjects &objects) {
	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring(name);
	lua_Object table = lua_gettable();

	if (lua_istable(table)) {
		lua_pushobject(table);
		lua_pushstring(name);
		lua_Object func = lua_gettable();
		if (lua_isfunction(func)) {
			lua_pushobject(table);
			objects.pushObjects();
			lua_callfunction(func);
		} else {
			lua_endblock();
			return false;
		}
	} else if (lua_isfunction(table)) {
		objects.pushObjects();
		lua_callfunction(table);
	} else if (!lua_isnil(table)) {
		lua_endblock();
		return false;
	}

	lua_endblock();
	return true;
}

bool LuaBase::getbool(int num) {
	return !lua_isnil(lua_getparam(num));
}

void LuaBase::pushbool(bool val) {
	if (val)
		lua_pushnumber(1);
	else
		lua_pushnil();
}

void LuaBase::pushobject(const PoolObjectBase *o) {
	lua_pushusertag(o->getId(), o->getTag());
}

Actor *LuaBase::getactor(lua_Object obj) {
	return Actor::getPool().getObject(lua_getuserdata(obj));
}

Bitmap *LuaBase::getbitmap(lua_Object obj) {
	return Bitmap::getPool().getObject(lua_getuserdata(obj));
}

TextObject *LuaBase::gettextobject(lua_Object obj) {
	return TextObject::getPool().getObject(lua_getuserdata(obj));
}

Font *LuaBase::getfont(lua_Object obj) {
	return Font::getPool().getObject(lua_getuserdata(obj));
}

Color LuaBase::getcolor(lua_Object obj) {
	return Color(lua_getuserdata(obj));
}

PrimitiveObject *LuaBase::getprimitive(lua_Object obj) {
	return PrimitiveObject::getPool().getObject(lua_getuserdata(obj));
}

ObjectState *LuaBase::getobjectstate(lua_Object obj) {
	return ObjectState::getPool().getObject(lua_getuserdata(obj));
}

void LuaBase::dummyHandler() {
}

bool LuaBase::findCostume(lua_Object costumeObj, Actor *actor, Costume **costume) {
	*costume = nullptr;
	if (lua_isnil(costumeObj))
		return true;
	if (lua_isnumber(costumeObj)) {
		/*		int num = (int)lua_getnumber(costumeObj);*/
		error("findCostume: search by Id not implemented");
		// TODO get costume by ID ?
	}
	if (lua_isstring(costumeObj)) {
		*costume = actor->findCostume(lua_getstring(costumeObj));
		return *costume != nullptr;
	}

	return false;
}

Common::String LuaBase::parseMsgText(const char *msg, char *msgId) {
	Common::String translation = g_localizer->localize(msg);
	const char *secondSlash = nullptr;

	if (msg[0] == '/' && msgId) {
		secondSlash = strchr(msg + 1, '/');
		if (secondSlash) {
			strncpy(msgId, msg + 1, secondSlash - msg - 1);
			msgId[secondSlash - msg - 1] = 0;
		} else {
			msgId[0] = 0;
		}
	}

	if (_translationMode == 1)
		return secondSlash;

	if (_translationMode == 2)
		return msg;

	return translation;
}

void LuaBase::parseSayLineTable(lua_Object paramObj, bool *background, int *vol, int *pan, float *x, float *y) {
	lua_Object tableObj;

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectX));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (x)
			*x = lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectY));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (y)
			*y = lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectBackground));
	tableObj = lua_gettable();
	if (tableObj) {
		if (background)
			*background = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectVolume));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (vol)
			*vol = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectPan));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (pan)
			*pan = (int)lua_getnumber(tableObj);
	}
}

void LuaBase::setTextObjectParams(TextObjectCommon *textObject, lua_Object tableObj) {
	lua_Object keyObj;

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectX));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			float num = lua_getnumber(keyObj);
			if (g_grim->getGameType() == GType_MONKEY4)
				textObject->setX((int)(num * 320) + 320);
			else
				textObject->setX((int)num);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectY));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			float num = lua_getnumber(keyObj);
			if (g_grim->getGameType() == GType_MONKEY4)
				textObject->setY((int)(240 - (num * 240)));
			else
				textObject->setY((int)num);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFont));
	keyObj = lua_gettable();
	if (keyObj) {
		if (g_grim->getGameType() == GType_MONKEY4 && lua_isstring(keyObj)) {
			const char *str = lua_getstring(keyObj);
			Font *font = nullptr;
			foreach (Font *f, Font::getPool()) {
				if (f->getFilename() == str) {
					font = f;
				}
			}
			if (!font) {
				font = g_resourceloader->loadFont(str);
			}

			textObject->setFont(font);
		} else if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('F','O','N','T')) {
			textObject->setFont(getfont(keyObj));
		} else if (g_grim->getGameType() == GType_MONKEY4 && !textObject->getFont() && g_grim->getGamePlatform() == Common::kPlatformPS2) {
			// HACK:
			warning("HACK: No default font set for PS2-version, just picking one for now");
			textObject->setFont(*Font::getPool().begin());
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectWidth));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setWidth((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectHeight));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setHeight((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			textObject->setFGColor(getcolor(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectBGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//textObject->setBGColor(static_cast<Color *>(lua_getuserdata(keyObj)));
			warning("setTextObjectParams: dummy BGColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFXColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//textObject->setFXColor(static_cast<Color *>(lua_getuserdata(keyObj)));
			warning("setTextObjectParams: dummy FXColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectCenter));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(1); //5
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectLJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(2); //4
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectRJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(3); //6
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectDuration));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setDuration((int)lua_getnumber(keyObj));
		}
	}

	// FIXME: remove check once the major save version is updated
	// currently it is needed for backward compatibility of old savegames
	if (lua_getref(refTextObjectLayer) == LUA_NOOBJECT)
		return;
	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectLayer));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setLayer(lua_getnumber(keyObj));
		}
	}

	// FIXME: remove check once the major save version is updated
	// currently it is needed for backward compatibility of old savegames
	if (lua_getref(refTextObjectCoords) == LUA_NOOBJECT)
		return;
	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectCoords));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setCoords(lua_getnumber(keyObj));
		}
	}
}

void LuaBase::typeOverride() {
	lua_Object data = lua_getparam(1);

	if (lua_isuserdata(data)) {
		switch (lua_tag(data)) {
			case MKTAG('A','C','T','R'):
				lua_pushstring("actor");
				lua_pushnumber(lua_tag(data));
				return;
			case MKTAG('C','O','S','T'):
				lua_pushstring("costume");
				lua_pushnumber(lua_tag(data));
				return;
			case MKTAG('S','E','T',' '):
				lua_pushstring("set");
				lua_pushnumber(lua_tag(data));
				return;
			case MKTAG('K','E','Y','F'):
				lua_pushstring("keyframe");
				lua_pushnumber(lua_tag(data));
				return;
			default:
				break;
		}
	}

	lua_pushobject(data);
	lua_callfunction(lua_getref(refTypeOverride));
	lua_Object param1 = lua_getresult(1);
	lua_Object param2 = lua_getresult(2);
	lua_pushobject(param1);
	lua_pushobject(param2);
}

void LuaBase::concatFallback() {
	lua_Object params[2];
	char result[200];
	char *strPtr;

	params[0] = lua_getparam(1);
	params[1] = lua_getparam(2);
	result[0] = 0;

	for (int i = 0; i < 2; i++) {
		if (!lua_isnil(params[i]) && !lua_isuserdata(params[i]) && !lua_isstring(params[i])) {
			lua_pushobject(params[0]);
			lua_pushobject(params[1]);
			lua_callfunction(lua_getref(refOldConcatFallback));
			lua_pushobject(lua_getresult(1));
			return;
		}

		int pos = strlen(result);
		strPtr = &result[pos];

		if (lua_isnil(params[i]))
			sprintf(strPtr, "(nil)");
		else if (lua_isstring(params[i]))
			sprintf(strPtr, "%s", lua_getstring(params[i]));
		else if (lua_tag(params[i]) == MKTAG('A','C','T','R')) {
			Actor *a = getactor(params[i]);
			sprintf(strPtr, "(actor%p:%s)", (void *)a,
					(a->getCurrentCostume() && a->getCurrentCostume()->getModelNodes()) ?
					a->getCurrentCostume()->getModelNodes()->_name : "");
		} else {
			lua_pushobject(params[0]);
			lua_pushobject(params[1]);
			lua_callfunction(lua_getref(refOldConcatFallback));
			lua_pushobject(lua_getresult(1));
			return;
		}
	}

	lua_pushstring(result);
}

}
