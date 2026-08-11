/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/lua/lua.h"
#include "common/textconsole.h"
#include "tetraedge/te/te_lua_gui_lua_callbacks.h"
#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_object.h"

namespace Tetraedge {

template <typename T> static T TeLuaTo(lua_State *L, int index) {
	void *ptr = lua_touserdata(L, index);
	if (!ptr)
		return nullptr;
	T obj = reinterpret_cast<T>(ptr);
	return dynamic_cast<T>(obj);
}

static Common::String TeLuaToTeString(lua_State *L, int index) {
	if (!lua_isstring(L, index)) {
		warning("TeLuaToTeString:: not a string");
		return "";
	} else {
		return lua_tolstring(L, index, nullptr);
	}
}

static long TeLuaToS32(lua_State *L, int index) {
	if (!lua_isnumber(L, index)) {
		warning("TeLuaToS32:: not a number");
		return 0;
	} else {
		return lua_tointeger(L, index);
	}
}

static long TeLuaToU32(lua_State *L, int index) {
	if (!lua_isnumber(L, index)) {
		warning("TeLuaToU32:: not a number");
		return 0;
	} else {
		return lua_tointeger(L, index);
	}
}

static float TeLuaToF32(lua_State *L, int index) {
	if (!lua_isnumber(L, index)) {
		warning("TeLuaToF32:: not a number");
		return 0.0f;
	} else {
		return lua_tonumber(L, index);
	}
}

static bool TeLuaToBool(lua_State *L, int index) {
	if (lua_type(L, index) != LUA_TBOOLEAN) {
		warning("TeLuaToBool:: not a bool");
		return false;
	} else {
		return lua_toboolean(L, index);
	}
}

static TeColor TeLuaToTeColor(lua_State *L, int index) {
	TeColor retval(255, 255, 255, 255);
	if (lua_type(L, index) != LUA_TTABLE) {
		warning("TeLuaToTeColor:: not a table");
		return retval;
	}

	lua_pushinteger(L, 1);
	index--;
	lua_gettable(L, index);
	if (lua_isnumber(L, -1)) {
		retval.r() = TeLuaToU32(L, -1);
	}
	lua_settop(L, -2);
	lua_pushinteger(L, 2);
	lua_gettable(L, index);

	if (lua_isnumber(L, -1)) {
		retval.g() = TeLuaToU32(L, -1);
	}

	lua_settop(L, -2);
	lua_pushinteger(L, 3);
	lua_gettable(L, index);
	if (lua_isnumber(L, -1)) {
		retval.b() = TeLuaToU32(L, -1);
	}

	lua_settop(L, -2);
	lua_pushinteger(L, 4);
	lua_gettable(L, index);
	if (lua_isnumber(L, -1)) {
		retval.a() = TeLuaToU32(L, -1);
	}

	lua_settop(L, -2);

	return retval;
}

static TeVector3f32 TeLuaToTeVector3f32(lua_State *L, int index, TeVector3f32 defaultVal) {
	TeVector3f32 retval = defaultVal;
	if (lua_type(L, index) != LUA_TTABLE) {
		warning("TeLuaToTeVector3f32:: not a table");
	} else {
		lua_pushinteger(L, 1);
		index--;
		lua_gettable(L, index);
		if (lua_isnumber(L, -1)) {
			retval.x() = TeLuaToF32(L, -1);
		}

		lua_settop(L, -2);
		lua_pushinteger(L, 2);
		lua_gettable(L, index);
		if (lua_isnumber(L, -1)) {
			retval.y() = TeLuaToF32(L, -1);
		}

		lua_settop(L, -2);
		lua_pushinteger(L, 3);
		lua_gettable(L, index);
		if (lua_isnumber(L, -1)) {
			retval.z() = TeLuaToF32(L, -1);
		}
		lua_settop(L, -2);
	}
	return retval;
}

static Common::Array<float> TeLuaToFloatArray(lua_State *L, int index) {
	Common::Array<float> result;
	int type = lua_type(L, index);
	if (type == LUA_TTABLE) {
		lua_pushnil(L);
		while (true) {
			int next = lua_next(L, index - 1);
			if (next == 0)
				break;
			result.push_back(TeLuaToF32(L, -1));
			lua_settop(L, -2);
		}
	} else {
		warning("TeLuaToF32TeArray:: the lua value is not a table");
	}
	return result;
}


static bool loadCommonLayoutItems(lua_State *L, const char *s, TeLayout *layout) {
	if (!strcmp(s, "name")) {
		layout->setName(TeLuaToTeString(L, -1));
	} else if (!strcmp(s, "sizeType")) {
		layout->setSizeType(static_cast<TeILayout::CoordinatesType>(TeLuaToS32(L, -1)));
	} else if (!strcmp(s, "size")) {
		TeVector3f32 lastSize = layout->userSize();
		TeVector3f32 size = TeLuaToTeVector3f32(L, -1, lastSize);
		layout->setSize(size);
	} else if (!strcmp(s, "ratio")) {
		layout->setRatio(TeLuaToF32(L, -1));
	} else if (!strcmp(s, "ratioMode")) {
		layout->setRatioMode(static_cast<TeILayout::RatioMode>(TeLuaToS32(L, -1)));
	} else if (!strcmp(s, "safeAreaRatio")) {
		layout->setSafeAreaRatio(TeLuaToF32(L, -1));
	} else if (!strcmp(s, "anchor")) {
		TeVector3f32 lastAnchor = layout->anchor();
		TeVector3f32 anchor = TeLuaToTeVector3f32(L, -1, lastAnchor);
		layout->setAnchor(anchor);
	} else if (!strcmp(s, "positionType")) {
		layout->setPositionType(static_cast<TeILayout::CoordinatesType>(TeLuaToS32(L, -1)));
	} else if (!strcmp(s, "position")) {
		TeVector3f32 lastPos = layout->userPosition();
		TeVector3f32 pos = TeLuaToTeVector3f32(L, -1, lastPos);
		layout->setPosition(pos);
	} else if (!strcmp(s, "visible")) {
		layout->setVisible(TeLuaToBool(L, -1));
	} else if (!strcmp(s, "color")) {
		layout->setColor(TeLuaToTeColor(L, -1));
	} else {
		return false;
	}
	return true;
}


// TODO: Fix this widescreen flag
static bool _g_bWidescreen = false;

int layoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("layoutBindings:: the lua value is not a table");
		return 0;
	}

	TeLayout *layout = new TeLayout();
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.layoutBindings] Unreconized attribute : %s", s);
			}
		} else if (type == LUA_TNUMBER) {
			Te3DObject2 *obj = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(obj);
		}
		lua_settop(L, -2);
	}

	if (layout->name().empty()) {
		layout->setName(Common::String::format("%p", (void *)layout));
	}
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeLayout *> &layouts = gui->layouts();
	if (!layouts.contains(layout->name())) {
		layouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("layoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

int listLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("listLayoutBindings:: the lua value is not a table");
		return 0;
	}

	TeListLayout *layout = new TeListLayout();
	lua_pushnil(L);

	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "direction")) {
				const TeVector3f32 lastDirection = layout->direction();
				const TeVector3f32 direction = TeLuaToTeVector3f32(L, -1, lastDirection);
				layout->setDirection(direction);
			} else if (!strcmp(s, "minimumMargin")) {
				const TeVector3f32 lastMinimumMargin = layout->minimumMargin();
				const TeVector3f32 minimumMargin = TeLuaToTeVector3f32(L, -1, lastMinimumMargin);
				layout->setMinimumMargin(minimumMargin);
			} else if (!strcmp(s, "maximumMargin")) {
				const TeVector3f32 lastMaximumMargin = layout->maximumMargin();
				const TeVector3f32 maximumMargin = TeLuaToTeVector3f32(L, -1, lastMaximumMargin);
				layout->setMaximumMargin(maximumMargin);
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.layoutBindings] Unreconized attribute : %s", s);
			}
		} else if (type == LUA_TNUMBER) {
			Te3DObject2 *obj = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(obj);
		}
		lua_settop(L, -2);
	}

	if (layout->name().empty()) {
		layout->setName(Common::String::format("%p", (void *)layout));
	}
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeListLayout *> &layouts = gui->listLayouts();
	if (!layouts.contains(layout->name())) {
		layouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("listLayoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

int spriteLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("spriteLayoutBindings:: the lua value is not a table");
		return 0;
	}

	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	lua_settop(L, -2);
	TeSpriteLayout *layout = new TeSpriteLayout();
	lua_pushnil(L);

	bool playNow = layout->_tiledSurfacePtr->_frameAnim._runTimer.running();
	int startingFrame = 0;
	int endingFrame = -1;
	Common::Path imgFullPath;
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, 0);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "image")) {
				Common::String imgPath = TeLuaToTeString(L, -1);
				// Slight hack.. fix double-slash that appears in some script files
				uint32 loc = imgPath.find("//");
				if (loc != Common::String::npos) {
					imgPath.replace(loc, 2, "/");
				}
				if (imgPath.substr(0, 2) == "./") {
					imgPath = imgPath.substr(0, 2);
					// NOTE: This is bad.. the scriptPath is a system-local path so the
					// separator may not be '/', we can't just make a Path from it like
					// this.  Fortunately it seems this is never actually used? No sprites
					// use './' in their data.
					warning("Taking non-portable code path to load image in spriteLayoutBindings");
					imgFullPath = Common::Path(gui->scriptPath()).getParent().join(imgPath);
				} else {
					imgFullPath = imgPath;
				}
			} else if (!strcmp(s, "leftCropping")) {
				layout->_tiledSurfacePtr->setLeftCropping(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "rightCropping")) {
				layout->_tiledSurfacePtr->setRightCropping(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "topCropping")) {
				layout->_tiledSurfacePtr->setTopCropping(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "bottomCropping")) {
				layout->_tiledSurfacePtr->setBottomCropping(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "loopCount")) {
				layout->_tiledSurfacePtr->_frameAnim.setLoopCount(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "play")) {
				playNow = TeLuaToBool(L, -1);
			} else if (!strcmp(s, "reversed")) {
				layout->_tiledSurfacePtr->_frameAnim.setReversed(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "startingFrame")) {
				startingFrame = TeLuaToU32(L, -1);
			} else if (!strcmp(s, "endingFrame")) {
				endingFrame = TeLuaToU32(L, -1);
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.layoutBindings] Unreconized attribute : %s", s);
			}
		}
		lua_settop(L, -2);
	}

	if (!imgFullPath.empty())
		layout->load(imgFullPath);

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_type(L, -2) == LUA_TNUMBER) {
			Te3DObject2 *object = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(object);
		}
		lua_settop(L, -2);
	}

	if (layout->name().empty()) {
		layout->setName(Common::String::format("%p", (void *)layout));
	}

	// WORKAROUND: ValStreet scene 11070 has a misnamed animation in the script.
	// All references to it in script refer to it with the "ab".
	// This scene is broken in the original game too.
	if (layout->name() == "11070-1")
		layout->setName("ab11070-1");

	TeICodec *codec = layout->_tiledSurfacePtr->codec();
	if (codec) {
		float frameRate = codec->frameRate();
		layout->_tiledSurfacePtr->_frameAnim.setStartTime((startingFrame / frameRate) * 1000.0 * 1000.0);
		if (endingFrame == -1) {
			layout->_tiledSurfacePtr->_frameAnim.setEndTime(FLT_MAX);
		} else {
			layout->_tiledSurfacePtr->_frameAnim.setEndTime((endingFrame / frameRate) * 1000.0 * 1000.0);
		}
	}

	// Slight divergence from original.. start playing only after setting
	// start/end values above as it makes more sense that way.
	if (playNow) {
		layout->play();
	} else {
		layout->stop();
	}

	if (!gui->spriteLayout(layout->name())) {
		TeLuaGUI::StringMap<TeSpriteLayout *> &spriteLayouts = gui->spriteLayouts();
		spriteLayouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("layoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

int buttonLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("buttonLayoutBindings:: the lua value is not a table");
		return 0;
	}

	TeButtonLayout *layout = new TeButtonLayout();
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "upLayout")) {
				layout->setUpLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "downLayout")) {
				layout->setDownLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "disabledLayout")) {
				layout->setDisabledLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "rollOverLayout")) {
				layout->setRollOverLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "hitZone")) {
				layout->setHitZone(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "enable")) {
				layout->setEnable(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "clickPassThrough")) {
				layout->setClickPassThrough(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "validationSound")) {
				layout->setValidationSound(Common::Path(TeLuaToTeString(L, -1)));
			} else if (!strcmp(s, "validationSoundVolume")) {
				layout->setValidationSoundVolume(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.buttonLayoutBindings] Unreconized attribute : %s", s);
			}
		}
		lua_settop(L, -2);
	}

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_type(L, -2) == LUA_TNUMBER) {
			Te3DObject2 *object = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(object);
		}
		lua_settop(L, -2);
	}

	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeButtonLayout *> &blayouts = gui->buttonLayouts();
	if (!blayouts.contains(layout->name())) {
		blayouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("buttonLayoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

int checkboxLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("checkboxLayoutBindings:: the lua value is not a table");
		return 0;
	}

	TeCheckboxLayout *layout = new TeCheckboxLayout();
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "activeLayout")) {
				layout->setActiveLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "unactiveLayout")) {
				layout->setUnactiveLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "activeDisabledLayout")) {
				layout->setActiveDisabledLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "unactiveDisabledLayout")) {
				layout->setUnactiveDisabledLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "activeRollOverLayout")) {
				layout->setActiveRollOverLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "unactiveRollOverLayout")) {
				layout->setUnactiveRollOverLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "hitZone")) {
				layout->setHitZone(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "clickPassThrough")) {
				layout->setClickPassThrough(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "activationSound")) {
				layout->setActivationSound(TeLuaToTeString(L, -1));
			} else if (!strcmp(s, "unactivationSound")) {
				layout->setUnactivationSound(TeLuaToTeString(L, -1));
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.checkboxLayoutBindings] Unreconized attribute : %s", s);
			}
		}
		lua_settop(L, -2);
	}

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_type(L, -2) == LUA_TNUMBER) {
			Te3DObject2 *object = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(object);
		}
		lua_settop(L, -2);
	}

	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeCheckboxLayout *> &blayouts = gui->checkboxLayouts();
	if (!blayouts.contains(layout->name())) {
		blayouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("checkboxLayoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

int layoutPositionLinearAnimationBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("layoutPositionLinearAnimationBindings:: the lua value is not a table");
		return 0;
	}

	TeCurveAnim2<TeLayout, TeVector3f32> *anim = new TeCurveAnim2<TeLayout, TeVector3f32>();
	lua_pushnil(L);
	Common::String name;
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (!strcmp(s, "name")) {
				name = TeLuaToTeString(L, -1);
			} else if (!strcmp(s, "duration")) {
				anim->_duration = TeLuaToF32(L, -1);
			} else if (!strcmp(s, "startValue")) {
				static const TeVector3f32 defaultStart(0.0f, 0.0f, 0.0f);
				anim->_startVal = TeLuaToTeVector3f32(L, -1, defaultStart);
			} else if (!strcmp(s, "endValue")) {
				static const TeVector3f32 defaultEnd(0.0f, 0.0f, 0.0f);
				anim->_endVal = TeLuaToTeVector3f32(L, -1, defaultEnd);
			} else if (!strcmp(s, "layout") || !strcmp(s, "pausable")) {
				// skip.
				// TODO: What should we do with "pausable" attribute?
			} else if (!strcmp(s, "curve")) {
				const Common::Array<float> curve = TeLuaToFloatArray(L, -1);
				anim->setCurve(curve);
			} else {
				error("[TeLuaGUI.layoutPositionLinearAnimationBindings] Unreconized attribute : %s", s);
			}
			lua_settop(L, -2);
		}
	}

	if (name.empty()) {
		name = Common::String::format("%p", (void *)anim);
	}
	anim->_callbackMethod = &TeLayout::setPosition;
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeCurveAnim2<TeLayout, TeVector3f32> *> &anims = gui->layoutPositionLinearAnimations();
	if (!anims.contains(name)) {
		anims.setVal(name, anim);
		lua_pushlightuserdata(L, (void *)(anim));
		return true;
	} else {
		warning("layoutPositionLinearAnimationBindings:: multiple objects with name %s", name.c_str());
		delete anim;
		return false;
	}
	return true;
}

int layoutAnchorLinearAnimationBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("layoutAnchorLinearAnimationBindings:: the lua value is not a table");
		return 0;
	}

	TeCurveAnim2<TeLayout, TeVector3f32> *anim = new TeCurveAnim2<TeLayout, TeVector3f32>();
	lua_pushnil(L);
	Common::String name;
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (!strcmp(s, "name")) {
				name = TeLuaToTeString(L, -1);
			} else if (!strcmp(s, "duration")) {
				anim->_duration = TeLuaToF32(L, -1);
			} else if (!strcmp(s, "startValue")) {
				static const TeVector3f32 defaultStart(0.0f, 0.0f, 0.0f);
				anim->_startVal = TeLuaToTeVector3f32(L, -1, defaultStart);
			} else if (!strcmp(s, "endValue")) {
				static const TeVector3f32 defaultEnd(0.0f, 0.0f, 0.0f);
				anim->_endVal = TeLuaToTeVector3f32(L, -1, defaultEnd);
			} else if (!strcmp(s, "layout")) {
				// skip.
			} else if (!strcmp(s, "curve")) {
				const Common::Array<float> curve = TeLuaToFloatArray(L, -1);
				anim->setCurve(curve);
			} else {
				error("[TeLuaGUI.layoutAnchorLinearAnimationBindings] Unreconized attribute : %s", s);
			}
			lua_settop(L, -2);
		}
	}

	if (name.empty()) {
		name = Common::String::format("%p", (void *)anim);
	}
	anim->_callbackMethod = &TeLayout::setAnchor;
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeCurveAnim2<TeLayout, TeVector3f32> *> &anims = gui->layoutAnchorLinearAnimations();
	if (!anims.contains(name)) {
		anims.setVal(name, anim);
		lua_pushlightuserdata(L, (void *)(anim));
		return true;
	} else {
		warning("layoutAnchorLinearAnimationBindings:: multiple objects with name %s", name.c_str());
		delete anim;
		return false;
	}
	return true;
}

int textLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("textLayoutBindings:: the lua value is not a table");
		return 0;
	}

	TeTextLayout *layout = new TeTextLayout();
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "text")) {
				layout->setText(TeLuaToTeString(L, -1));
			} else if (!strcmp(s, "interLine") || !strcmp(s, "interline")) {
				layout->setInterLine(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "wrapMode")) {
				layout->setWrapMode(static_cast<TeTextBase2::WrapMode>(TeLuaToS32(L, -1)));
			} else if (!strcmp(s, "textSizeType")) {
				layout->setTextSizeType(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "textSizeProportionalToWidth")) {
				layout->setTextSizeProportionalToWidth(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.textLayoutBindings] Unreconized attribute : %s", s);
			}
		} else if (type == LUA_TNUMBER) {
			Te3DObject2 *obj = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(obj);
		}
		lua_settop(L, -2);
	}

	if (layout->name().empty()) {
		layout->setName(Common::String::format("%p", (void *)layout));
	}
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);

	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeTextLayout *> &layouts = gui->textLayouts();
	if (!layouts.contains(layout->name())) {
		layouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("textLayoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

int clipLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("clipLayoutBindings:: the lua value is not a table");
		return 0;
	}

	error("TODO: Implement clipLayoutBindings.");
}

int colorLinearAnimationBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("colorLinearAnimationBindings:: the lua value is not a table");
		return 0;
	}

	TeCurveAnim2<Te3DObject2, TeColor> *anim = new TeCurveAnim2<Te3DObject2, TeColor>();
	lua_pushnil(L);
	Common::String name;
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (!strcmp(s, "name")) {
				name = TeLuaToTeString(L, -1);
			} else if (!strcmp(s, "duration")) {
				anim->_duration = TeLuaToF32(L, -1);
			} else if (!strcmp(s, "startValue")) {
				anim->_startVal = TeLuaToTeColor(L, -1);
			} else if (!strcmp(s, "endValue")) {
				anim->_endVal = TeLuaToTeColor(L, -1);
			} else if (!strcmp(s, "layout")) {
			} else if (!strcmp(s, "curve")) {
				const Common::Array<float> curve = TeLuaToFloatArray(L, -1);
				anim->setCurve(curve);
			} else {
				error("[TeLuaGUI.colorLinearAnimationBindings] Unreconized attribute : %s", s);
			}
			lua_settop(L, -2);
		}
	}

	if (name.empty()) {
		name = Common::String::format("%p", (void *)anim);
	}
	anim->_callbackMethod = &Te3DObject2::setColor;
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);
	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeCurveAnim2<Te3DObject2, TeColor> *> &anims = gui->colorLinearAnimations();
	if (!anims.contains(name)) {
		anims.setVal(name, anim);
		lua_pushlightuserdata(L, (void *)(anim));
		return true;
	} else {
		warning("colorLinearAnimationBindings:: multiple objects with name %s", name.c_str());
		delete anim;
		return false;
	}
	return true;
}

int rotationLinearAnimationBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("rotationLinearAnimationBindings:: the lua value is not a table");
		return 0;
	}

	error("TODO: Implement rotationLinearAnimationBindings.");
}

int scrollingLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("scrollingLayoutBindings:: the lua value is not a table");
		return 0;
	}

	TeScrollingLayout *layout = new TeScrollingLayout();
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "inertiaAnimationDuration")) {
				layout->setInertiaAnimationDuration(TeLuaToU32(L, -1));
			} else if (!strcmp(s, "inertiaAnimationCurve")) {
				layout->setInertiaAnimationCurve(TeLuaToFloatArray(L, -1));
			} else if (!strcmp(s, "direction")) {
				TeVector3f32 newdir = TeLuaToTeVector3f32(L, -1, layout->direction());
				layout->setDirection(newdir);
			} else if (!strcmp(s, "contentLayout")) {
				layout->setContentLayout(TeLuaTo<TeLayout*>(L, -1));
			} else if (!strcmp(s, "enclose")) {
				layout->setEnclose(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "mouseControl")) {
				layout->setMouseControl(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "autoScrollLoop")) {
				layout->setAutoScrollLoop(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "autoScrollDelay")) {
				layout->setAutoScrollDelay(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation1Enabled")) {
				layout->setAutoScrollAnimation1Enabled(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation1Delay")) {
				layout->setAutoScrollAnimation1Delay(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation1Speed")) {
				layout->setAutoScrollAnimation1Speed(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation1Curve")) {
				layout->setAutoScrollAnimation1Curve(TeLuaToFloatArray(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation2Enabled")) {
				layout->setAutoScrollAnimation2Enabled(TeLuaToBool(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation2Delay")) {
				layout->setAutoScrollAnimation2Delay(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation2Speed")) {
				layout->setAutoScrollAnimation2Speed(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "autoScrollAnimation2Curve")) {
				layout->setAutoScrollAnimation2Curve(TeLuaToFloatArray(L, -1));
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.scrollingLayoutBindings] Unreconized attribute : %s", s);
			}
		} else if (type == LUA_TNUMBER) {
			Te3DObject2 *obj = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(obj);
		}
		lua_settop(L, -2);
	}

	if (layout->name().empty()) {
		layout->setName(Common::String::format("%p", (void *)layout));
	}
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);

	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeScrollingLayout *> &layouts = gui->scrollingLayouts();
	if (!layouts.contains(layout->name())) {
		layouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("scrollingLayoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

int extendedTextLayoutBindings(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		warning("extendedTextLayoutBindings:: the lua value is not a table");
		return 0;
	}

	TeExtendedTextLayout *layout = new TeExtendedTextLayout();
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int type = lua_type(L, -2);
		if (type == LUA_TSTRING) {
			const char *s = lua_tolstring(L, -2, nullptr);
			if (loadCommonLayoutItems(L, s, layout)) {
				// do nothing.
			} else if (!strcmp(s, "text")) {
				layout->setText(TeLuaToTeString(L, -1));
			} else if (!strcmp(s, "interLine") || !strcmp(s, "interline")) {
				layout->setInterLine(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "wrapMode")) {
				layout->setWrapMode(static_cast<TeTextBase2::WrapMode>(TeLuaToS32(L, -1)));
			} else if (!strcmp(s, "autoScrollDelay")) {
				layout->setAutoScrollDelay(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "autoScrollSpeed")) {
				layout->setAutoScrollSpeed(TeLuaToF32(L, -1));
			} else if (!strcmp(s, "textSizeType")) {
				layout->setTextSizeType(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "textSizeProportionalToWidth")) {
				layout->setTextSizeProportionalToWidth(TeLuaToS32(L, -1));
			} else if (!strcmp(s, "consoleNoStretch")) {
				warning("TODO: Handle _g_bWidescreen");
				if (_g_bWidescreen) {
					layout->setScale(TeVector3f32(0.75f, 1.0f, 1.0f));
				}
			} else {
				warning("[TeLuaGUI.textLayoutBindings] Unreconized attribute : %s", s);
			}
		} else if (type == LUA_TNUMBER) {
			Te3DObject2 *obj = TeLuaTo<Te3DObject2*>(L, -1);
			layout->addChild(obj);
		}
		lua_settop(L, -2);
	}

	if (layout->name().empty()) {
		layout->setName(Common::String::format("%p", (void *)layout));
	}
	lua_pushstring(L, "__TeLuaGUIThis");
	lua_gettable(L, LUA_REGISTRYINDEX);

	TeLuaGUI *gui = TeLuaTo<TeLuaGUI*>(L, -1);
	TeLuaGUI::StringMap<TeExtendedTextLayout *> &layouts = gui->extendedTextLayouts();
	if (!layouts.contains(layout->name())) {
		layouts.setVal(layout->name(), layout);
		lua_pushlightuserdata(L, static_cast<Te3DObject2*>(layout));
		return true;
	} else {
		warning("extendedTextLayoutBindings:: multiple objects with name %s", layout->name().c_str());
		delete layout;
		return false;
	}
}

} // end namespace Tetraedge
