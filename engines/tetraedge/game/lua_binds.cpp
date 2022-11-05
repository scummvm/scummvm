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

#include "tetraedge/tetraedge.h"

#include "tetraedge/game/application.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/lua_binds.h"
#include "tetraedge/to_lua.h"

namespace Tetraedge {

namespace LuaBinds {

using namespace ToLua;

static void PlayMovie(const Common::String &vidpath, const Common::String &musicpath) {
	Application *app = g_engine->getApplication();
	app->mouseCursorLayout().load("pictures/cursor.png");
	Game *game = g_engine->getGame();
	game->playMovie(vidpath, musicpath);
}

static int tolua_ExportedFunctions_PlayMovie00(lua_State *L) {
tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		PlayMovie(s1, s2);
		return 0;
	}
	error("#ferror in function 'PlayMovie': %d %d %s", err.index, err.array, err.type);
}

static void AddRandomSound(const Common::String &s1, const Common::String &s2, float f1, float f2){
	Game *game = g_engine->getGame();
	game->addRandomSound(s1, s2, f1, f2);
}

static int tolua_ExportedFunctions_AddRandomSound00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 1, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		float f1 = tolua_tonumber(L, 3, 0.0);
		float f2 = tolua_tonumber(L, 4, 1.0);
		AddRandomSound(s1, s2, f1, f2);
		return 0;
	}
	error("#ferror in function 'AddRandomSound': %d %d %s", err.index, err.array, err.type);
}

static void SetSoundStep(const Common::String &scene, const Common::String &step1, const Common::String &step2) {
	Game *game = g_engine->getGame();
	game->scene().setStep(scene, step1, step2);
}

static int tolua_ExportedFunctions_SetSoundStep00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isstring(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		Common::String s3(tolua_tostring(L, 3, nullptr));
		SetSoundStep(s1, s2, s3);
		return 0;
	}
	error("#ferror in function 'SetSoundStep': %d %d %s", err.index, err.array, err.type);
}

static void AddNumber(const Common::String &number) {
	Game *game = g_engine->getGame();
	if (!game->inventory().cellphone()->addNumber(number))
		warning("[AddNumber] Number \"%s\" already exist.", number.c_str());
}

static int tolua_ExportedFunctions_AddNumber00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		AddNumber(s1);
		return 0;
	}
	error("#ferror in function 'AddNumber': %d %d %s", err.index, err.array, err.type);
}

static void AddUnrecalAnim(const Common::String &newanim) {
	Application *app = g_engine->getApplication();
	Common::Array<Common::String> &anims = app->unrecalAnims();
	for (const Common::String &anim : anims) {
		if (anim == newanim)
			return;
	}
	anims.push_back(newanim);
}

static int tolua_ExportedFunctions_AddUnrecalAnim00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		AddUnrecalAnim(s1);
		return 0;
	}
	error("#ferror in function 'AddUnrecalAnim': %d %d %s", err.index, err.array, err.type);
}

static void UnlockArtwork(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->addArtworkUnlocked(name, true);
	Application *app = g_engine->getApplication();
	app->saveOptions("options.xml");
}

static int tolua_ExportedFunctions_UnlockArtwork00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		UnlockArtwork(s1);
		return 0;
	}
	error("#ferror in function 'UnlockArtwork': %d %d %s", err.index, err.array, err.type);
}

static void ChangeWarp(const Common::String &zone, const Common::String &scene, bool flag) {
	Game *game = g_engine->getGame();
	if (game->changeWarp(zone, scene, flag))
		return;
	warning("[ChangeWarp] Zone \"%s\" with number Scene \"%s\" don't exist. Please reload and change with correct name.",
				zone.c_str(), scene.c_str());
}

static int tolua_ExportedFunctions_ChangeWarp00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isboolean(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		bool flag = tolua_toboolean(L, 3, 0);
		ChangeWarp(s1, s2, flag);
		return 0;
	}
	error("#ferror in function 'ChangeWarp': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterPlayerVisible(bool val) {
	Game *game = g_engine->getGame();
	game->scene()._character->_model->setVisible(val);
}

static int tolua_ExportedFunctions_SetCharacterPlayerVisible00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		SetCharacterPlayerVisible(tolua_toboolean(L, 1, 0));
		return 0;
	}
	error("#ferror in function 'SetCharacterPlayerVisible': %d %d %s", err.index, err.array, err.type);
}

static void MoveCharacterPlayerDisabled(bool val) {
	Game *game = g_engine->getGame();
	game->_movePlayerCharacterDisabled = val;
}

static int tolua_ExportedFunctions_MoveCharacterPlayerDisabled00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		MoveCharacterPlayerDisabled(tolua_toboolean(L, 1, 0));
		return 0;
	}
	error("#ferror in function 'MoveCharacterPlayerDisabled': %d %d %s", err.index, err.array, err.type);
}

static void AddMarker(const Common::String &markerName, const Common::String &imgPath, float x, float y,
				const Common::String &loctype, const Common::String &markerVal) {
	Game *game = g_engine->getGame();
	game->scene().addMarker(markerName, imgPath, x, y, loctype, markerVal);
}

static int tolua_ExportedFunctions_AddMarker00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
			&& tolua_isstring(L, 5, 1, &err) && tolua_isstring(L, 6, 1, &err)
			&& tolua_isnoobj(L, 7, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double n1 = tolua_tonumber(L, 3, 0.0);
		double n2 = tolua_tonumber(L, 4, 0.0);
		Common::String s3(tolua_tostring(L, 5, ""));
		Common::String s4(tolua_tostring(L, 6, ""));
		AddMarker(s1, s2, n1, n2, s3, s4);
		return 0;
	}
	error("#ferror in function 'AddMarker': %d %d %s", err.index, err.array, err.type);
}

static void SetVisibleMarker(const Common::String &markerName, bool val) {
	Game *game = g_engine->getGame();
	game->scene().setVisibleMarker(markerName, val);
}

static int tolua_ExportedFunctions_SetVisibleMarker00(lua_State *L) {
tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s(tolua_tostring(L, 1, nullptr));
		bool b = tolua_toboolean(L, 2, 0);
		SetVisibleMarker(s, b);
		return 0;
	}
	error("#ferror in function 'SetVisibleMarker': %d %d %s", err.index, err.array, err.type);
}

static void DeleteMarker(const Common::String &markerName) {
	Game *game = g_engine->getGame();
	game->scene().deleteMarker(markerName);
}

static int tolua_ExportedFunctions_DeleteMarker00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		DeleteMarker(s1);
		return 0;
	}
	error("#ferror in function 'DeleteMarker': %d %d %s", err.index, err.array, err.type);
}

static void SetVisibleCellphone(bool visible) {
	Game *game = g_engine->getGame();
	game->inventory().cellphone()->setVisible(visible);
}

static int tolua_ExportedFunctions_SetVisibleCellphone00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		SetVisibleCellphone(tolua_toboolean(L, 1, 0));
		return 0;
	}
	error("#ferror in function 'SetVisibleCellphone': %d %d %s", err.index, err.array, err.type);
}

static void RequestAutoSave() {
	Game *game = g_engine->getGame();
	game->setSaveRequested();
}

static int tolua_ExportedFunctions_RequestAutoSave00(lua_State *L) {
	RequestAutoSave();
	return 0;
}

static void HideObject(const Common::String &objName) {
	Game *game = g_engine->getGame();
	TeIntrusivePtr<TeModel> model = game->scene().model(objName);
	if (model) {
		model->setVisible(false);
		return;
	}

	debug("[HideObject] Object 3D \"%s\" doesn't exist.", objName.c_str());
	TeLayout *layout = game->scene().bgGui().layout(objName);
	if (layout) {
		layout->setVisible(false);
		return;
	}

	debug("[HideObject] \"Set\" Object 2D \"%s\" doesn't exist.", objName.c_str());
	layout = game->gui3().layout(objName);
	if (layout) {
		layout->setVisible(false);
		return;
	}

	debug("[HideObject] \"For\" Object 2D \"%s\" doesn't exist.", objName.c_str());
}

static int tolua_ExportedFunctions_HideObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		HideObject(s1);
		return 0;
	}
	error("#ferror in function 'HideObject': %d %d %s", err.index, err.array, err.type);
}

static void ShowObject(const Common::String &objName) {
	Game *game = g_engine->getGame();
	TeIntrusivePtr<TeModel> model = game->scene().model(objName);
	if (model) {
		model->setVisible(true);
		return;
	}

	debug("[ShowObject] Object 3D \"%s\" doesn't exist.", objName.c_str());
	TeLayout *layout = game->scene().bgGui().layout(objName);
	if (layout) {
		layout->setVisible(true);
		return;
	}

	debug("[ShowObject] \"Set\" Object 2D \"%s\" doesn't exist.", objName.c_str());
	layout = game->gui3().layout(objName);
	if (layout) {
		layout->setVisible(true);
		return;
	}

	debug("[ShowObject] \"For\" Object 2D \"%s\" doesn't exist.", objName.c_str());
}

static int tolua_ExportedFunctions_ShowObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		ShowObject(s1);
		return 0;
	}
	error("#ferror in function 'ShowObject': %d %d %s", err.index, err.array, err.type);
}

static void LoadObject(const Common::String &objName) {
	Game *game = g_engine->getGame();
	game->scene().loadObject(objName);
}

static int tolua_ExportedFunctions_LoadObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		LoadObject(s1);
		return 0;
	}
	error("#ferror in function 'LoadObject': %d %d %s", err.index, err.array, err.type);
}

static void UnloadObject(const Common::String &objName) {
	Game *game = g_engine->getGame();
	game->scene().unloadObject(objName);
}

static int tolua_ExportedFunctions_UnloadObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		UnloadObject(s1);
		return 0;
	}
	error("#ferror in function 'UnloadObject': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterRotation(const Common::String &charname, float f1, float f2, float f3) {
	// TODO: check if this is good.
	TeQuaternion quat = TeQuaternion::fromEuler(TeVector3f32(f1 * M_PI / 180.0, f2 * M_PI / 180.0, f3 * M_PI / 180.0));
	Game *game = g_engine->getGame();
	Character *c = game->scene().character(charname);
	if (c) {
		c->_model->setRotation(quat);
	} else {
		warning("[SetCharacterRotation] Character not found %s", charname.c_str());
	}
}

static int tolua_ExportedFunctions_SetCharacterRotation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 1.0);
		SetCharacterRotation(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetCharacterRotation': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterPosition(const Common::String &charname, const Common::String &zonename, float f1, float f2, float f3) {
	Game *game = g_engine->getGame();
	game->scene().setPositionCharacter(charname, zonename, TeVector3f32(f1, f2, f3));
}

static int tolua_ExportedFunctions_SetCharacterPosition00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnumber(L, 5, 0, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		float f1 = tolua_tonumber(L, 3, 0.0);
		float f2 = tolua_tonumber(L, 4, 1.0);
		float f3 = tolua_tonumber(L, 5, 0.0);
		SetCharacterPosition(s1, s2, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetCharacterPosition': %d %d %s", err.index, err.array, err.type);
}

static void SetGroundObjectPosition(const Common::String &objname, float x, float y, float z) {
	Game *game = g_engine->getGame();
	Object3D *obj = game->scene().object3D(objname);
	if (!obj) {
		warning("[SetGroundObjectPosition] Object not found %s", objname.c_str());
		return;
	}
	obj->model()->setPosition(TeVector3f32(x, y, z));
	obj->model()->setVisible(true);
}

static int tolua_ExportedFunctions_SetGroundObjectPosition00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 1.0);
		SetGroundObjectPosition(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetGroundObjectPosition': %d %d %s", err.index, err.array, err.type);
}

static void SetGroundObjectRotation(const Common::String &objname, float x, float y, float z) {
	Game *game = g_engine->getGame();
	Object3D *obj = game->scene().object3D(objname);
	if (!obj) {
		warning("[SetGroundObjectRotation] Object not found %s", objname.c_str());
		return;
	}
	
	TeVector3f32 rotvec(x * M_PI / 180.0, y * M_PI / 180.0, z * M_PI / 180.0);
	obj->model()->setRotation(TeQuaternion::fromEuler(rotvec));
	obj->model()->setVisible(true);
}

static int tolua_ExportedFunctions_SetGroundObjectRotation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 1.0);
		SetGroundObjectRotation(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetGroundObjectRotation': %d %d %s", err.index, err.array, err.type);
}

static void SetBackground(const Common::String &name) {
	Game *game = g_engine->getGame();
	if (!game->setBackground(name))
		warning("[SetBackground] Background \"%s\" doesn't exist.", name.c_str());
}

static int tolua_ExportedFunctions_SetBackground00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		SetBackground(s1);
		return 0;
	}
	error("#ferror in function 'SetBackground': %d %d %s", err.index, err.array, err.type);
}

static void PushTask(const Common::String &s1, const Common::String &s2) {
	Game *game = g_engine->getGame();
	game->objectif().pushObjectif(s1, s2);
}

static int tolua_ExportedFunctions_PushTask00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		PushTask(s1, s2);
		return 0;
	}
	error("#ferror in function 'PushTask': %d %d %s", err.index, err.array, err.type);
}

static void EnableRectBlocker(uint offset, bool enabled) {
	Game *game = g_engine->getGame();
	if (game->scene().rectBlockers().size() < offset)
		error("invalid rectblocker offset %d", offset);

	game->scene().rectBlockers()[offset]._x = enabled;
}

static int tolua_ExportedFunctions_EnableRectBlocker00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		double d1 = tolua_tonumber(L, 1, 0.0f);
		bool b1 = tolua_toboolean(L, 2, 0);
		EnableRectBlocker((uint)d1, b1);
		return 0;
	}
	error("#ferror in function 'EnableRectBlocker': %d %d %s", err.index, err.array, err.type);
}

static void AddAnchorZone(const Common::String &s1, const Common::String &s2, float f1) {
	if (s1.empty())
		return;

	Game *game = g_engine->getGame();

	if (s1.contains("Dummy")) {
		game->scene().addAnchorZone(s1, s2, f1);
	} else if (s1.contains("Int")) {
		if (game->scene().hitObjectGui().loaded()) {
			TeButtonLayout *layout = game->scene().hitObjectGui().buttonLayout(s2);
			if (!layout) {
				warning("[AddAnchorZone] Zone \"%s\" doesn't exist.", s2.c_str());
			} else {
				game->scene().addAnchorZone(s1, s2, f1);
			}
		}
	}
}

static int tolua_ExportedFunctions_AddAnchorZone00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnumber(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double d1 = tolua_tonumber(L, 3, 1.0);
		AddAnchorZone(s1, s2, d1);
		return 0;
	}
	error("#ferror in function 'AddAnchorZone': %d %d %s", err.index, err.array, err.type);
}

static void DisabledZone(const Common::String &s1, bool b) {
	Game *game = g_engine->getGame();
	if (!game->scene().markerGui().loaded())
		return;

	warning("TODO: Implement DisabledZone");
}

static int tolua_ExportedFunctions_DisabledZone00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 1, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s(tolua_tostring(L, 1, nullptr));
		bool b = tolua_toboolean(L, 2, 1);
		DisabledZone(s, b);
		return 0;
	}
	error("#ferror in function 'DisabledZone': %d %d %s", err.index, err.array, err.type);
}

static void DisabledInt(const Common::String &name, bool b) {
	Game *game = g_engine->getGame();
	if (!game->scene().hitObjectGui().loaded())
		return;

	TeButtonLayout *layout = game->scene().hitObjectGui().buttonLayout(name);
	if (layout) {
		layout->setVisible(!b);
	}
}

static int tolua_ExportedFunctions_DisabledInt00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 1, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s(tolua_tostring(L, 1, nullptr));
		bool b = tolua_toboolean(L, 2, 1);
		DisabledInt(s, b);
		return 0;
	}
	error("#ferror in function 'DisabledInt': %d %d %s", err.index, err.array, err.type);
}

static void LockCursor(bool b) {
	Application *app = g_engine->getApplication();
	app->lockCursorFromAction(b);
}

static int tolua_ExportedFunctions_LockCursor00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		bool b = tolua_toboolean(L, 1, 0);
		LockCursor(b);
		return 0;
	}
	error("#ferror in function 'LockCursor': %d %d %s", err.index, err.array, err.type);
}

static void PlaySound(const Common::String &name, int i1, float f2) {
	Game *game = g_engine->getGame();
	game->playSound(name, i1, f2);
}

static int tolua_ExportedFunctions_PlaySound00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 1, &err) && tolua_isnumber(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		double d1 = tolua_tonumber(L, 2, -1.0);
		double d2 = tolua_tonumber(L, 3, 1.0);
		PlaySound(s1, d1, d2);
		return 0;
	}
	error("#ferror in function 'PlaySound': %d %d %s", err.index, err.array, err.type);
}

static void StopSound(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->stopSound(name);
}

static int tolua_ExportedFunctions_StopSound00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		StopSound(s1);
		return 0;
	}
	error("#ferror in function 'StopSound': %d %d %s", err.index, err.array, err.type);
}

static void PlayRandomSound(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->playRandomSound(name);
}

static int tolua_ExportedFunctions_PlayRandomSound00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		PlayRandomSound(s1);
		return 0;
	}
	error("#ferror in function 'PlayRandomSound': %d %d %s", err.index, err.array, err.type);
}

static void PlayMusic(const Common::String &path) {
	TeMusic &music = g_engine->getApplication()->music();
	music.load(path);
	music.play();
	music.repeat(false);
	music.volume(1.0);
}

static int tolua_ExportedFunctions_PlayMusic00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		PlayMusic(s1);
		return 0;
	}
	error("#ferror in function 'PlayMusic': %d %d %s", err.index, err.array, err.type);
}


// ////////////////////////////////////////////////////////////////////////



void LuaOpenBinds(lua_State *L) {
	tolua_open(L);
	tolua_module(L, 0, 0);
	tolua_beginmodule(L, 0);
	/*
	tolua_function(L, "LoadObjectMaterials", tolua_ExportedFunctions_LoadObjectMaterials00);
	tolua_function(L, "LoadObjectMaterials", tolua_ExportedFunctions_LoadObjectMaterials01);*/
	tolua_function(L, "HideObject", tolua_ExportedFunctions_HideObject00);
	tolua_function(L, "ShowObject", tolua_ExportedFunctions_ShowObject00);
	/*tolua_function(L, "ShowAllObjects", tolua_ExportedFunctions_ShowAllObjects00);*/
	tolua_function(L, "SetBackground", tolua_ExportedFunctions_SetBackground00);/*
	tolua_function(L, "AddBlockingObject", tolua_ExportedFunctions_AddBlockingObject00);
	tolua_function(L, "RemoveBlockingObject", tolua_ExportedFunctions_RemoveBlockingObject00);*/
	tolua_function(L, "ChangeWarp", tolua_ExportedFunctions_ChangeWarp00);
	tolua_function(L, "PlayMovie", tolua_ExportedFunctions_PlayMovie00);
	/*tolua_function(L, "PlayMovieAndWaitForEnd", tolua_ExportedFunctions_PlayMovieAndWaitForEnd00);
	tolua_function(L, "StartAnimationPart", tolua_ExportedFunctions_StartAnimationPart00);
	tolua_function(L, "StartAnimation", tolua_ExportedFunctions_StartAnimation00);
	tolua_function(L, "StartAnimationAndWaitForEnd",
				 tolua_ExportedFunctions_StartAnimationAndWaitForEnd00);
	tolua_function(L, "AddAnimToSet", tolua_ExportedFunctions_AddAnimToSet00); */
	tolua_function(L, "RequestAutoSave", tolua_ExportedFunctions_RequestAutoSave00);
	/*tolua_function(L, "SetVisibleButtonZoomed", tolua_ExportedFunctions_SetVisibleButtonZoomed00);*/
	tolua_function(L, "AddMarker", tolua_ExportedFunctions_AddMarker00);
	tolua_function(L, "SetVisibleMarker", tolua_ExportedFunctions_SetVisibleMarker00);
	tolua_function(L, "DeleteMarker", tolua_ExportedFunctions_DeleteMarker00);
	tolua_function(L, "SetVisibleCellphone", tolua_ExportedFunctions_SetVisibleCellphone00);
	tolua_function(L, "DisabledZone", tolua_ExportedFunctions_DisabledZone00);
	tolua_function(L, "DisabledInt", tolua_ExportedFunctions_DisabledInt00);
	tolua_function(L, "LockCursor", tolua_ExportedFunctions_LockCursor00);
	/*tolua_function(L, "SetCondition", tolua_ExportedFunctions_SetCondition00);
	tolua_function(L, "UnsetCondition", tolua_ExportedFunctions_UnsetCondition00);
	tolua_function(L, "TutoActive", tolua_ExportedFunctions_TutoActive00);
	tolua_function(L, "LaunchDialog", tolua_ExportedFunctions_LaunchDialog00);
	tolua_function(L, "LaunchDialogAndWaitForEnd", tolua_ExportedFunctions_LaunchDialogAndWaitForEnd00);
	tolua_function(L, "PushAnswer", tolua_ExportedFunctions_PushAnswer00);
	tolua_function(L, "HideAnswers", tolua_ExportedFunctions_HideAnswers00);*/
	tolua_function(L, "PushTask", tolua_ExportedFunctions_PushTask00);
	/*tolua_function(L, "DeleteTask", tolua_ExportedFunctions_DeleteTask00);
	tolua_function(L, "SetVisibleButtonHelp", tolua_ExportedFunctions_SetVisibleButtonHelp00);
	tolua_function(L, "HideTasks", tolua_ExportedFunctions_HideTasks00);*/
	tolua_function(L, "PlaySound", tolua_ExportedFunctions_PlaySound00);
	/*tolua_function(L, "PlaySoundAndWaitForEnd", tolua_ExportedFunctions_PlaySoundAndWaitForEnd00);*/
	tolua_function(L, "StopSound", tolua_ExportedFunctions_StopSound00);
	tolua_function(L, "AddRandomSound", tolua_ExportedFunctions_AddRandomSound00);
	tolua_function(L, "PlayRandomSound", tolua_ExportedFunctions_PlayRandomSound00);
	tolua_function(L, "PlayMusic", tolua_ExportedFunctions_PlayMusic00);
	tolua_function(L, "SetSoundStep", tolua_ExportedFunctions_SetSoundStep00);
	/*tolua_function(L, "Selected", tolua_ExportedFunctions_Selected00);
	tolua_function(L, "TakeObject", tolua_ExportedFunctions_TakeObject00);
	tolua_function(L, "TakeObjectInHand", tolua_ExportedFunctions_TakeObjectInHand00);
	tolua_function(L, "RemoveObject", tolua_ExportedFunctions_RemoveObject00);
	tolua_function(L, "RemoveObject", tolua_ExportedFunctions_RemoveObject01);*/
	tolua_function(L, "AddNumber", tolua_ExportedFunctions_AddNumber00);
	/*tolua_function(L, "ShowDocument", tolua_ExportedFunctions_ShowDocument00);
	tolua_function(L, "ShowDocumentAndWaitForEnd", tolua_ExportedFunctions_ShowDocumentAndWaitForEnd00);
	tolua_function(L, "HideDocument", tolua_ExportedFunctions_HideDocument00);
	tolua_function(L, "AddDocument", tolua_ExportedFunctions_AddDocument00);
	tolua_function(L, "LoadCharacter", tolua_ExportedFunctions_LoadCharacter00);
	tolua_function(L, "UnloadCharacter", tolua_ExportedFunctions_UnloadCharacter00);
	tolua_function(L, "GetRotationCharacter", tolua_ExportedFunctions_GetRotationCharacter00);
	tolua_function(L, "GetXPositionCharacter", tolua_ExportedFunctions_GetXPositionCharacter00);
	tolua_function(L, "GetYPositionCharacter", tolua_ExportedFunctions_GetYPositionCharacter00);
	tolua_function(L, "GetZPositionCharacter", tolua_ExportedFunctions_GetZPositionCharacter00);
	tolua_function(L, "MoveCharacterTo", tolua_ExportedFunctions_MoveCharacterTo00);
	tolua_function(L, "MoveCharacterToAndWaitForEnd",
				 tolua_ExportedFunctions_MoveCharacterToAndWaitForEnd00);
	tolua_function(L, "MoveCharacterPlayerTo", tolua_ExportedFunctions_MoveCharacterPlayerTo00);
	tolua_function(L, "MoveCharacterPlayerToAndWaitForEnd",
				 tolua_ExportedFunctions_MoveCharacterPlayerToAndWaitForEnd00);
	tolua_function(L, "MoveCharacterPlayerAtTo", tolua_ExportedFunctions_MoveCharacterPlayerAtTo00);*/
	tolua_function(L, "SetCharacterPosition", tolua_ExportedFunctions_SetCharacterPosition00);
	/*tolua_function(L, "PlaceCharacterOnDummy", tolua_ExportedFunctions_PlaceCharacterOnDummy00);*/
	tolua_function(L, "SetCharacterRotation", tolua_ExportedFunctions_SetCharacterRotation00);
	/*tolua_function(L, "SetCharacterOrientation", tolua_ExportedFunctions_SetCharacterOrientation00);
	tolua_function(L, "SetCharacterAnimation", tolua_ExportedFunctions_SetCharacterAnimation00);
	tolua_function(L, "SetCharacterAnimationAndWaitForEnd",
				 tolua_ExportedFunctions_SetCharacterAnimationAndWaitForEnd00);
	tolua_function(L, "BlendCharacterAnimation", tolua_ExportedFunctions_BlendCharacterAnimation00);
	tolua_function(L, "BlendCharacterAnimationAndWaitForEnd",
				 tolua_ExportedFunctions_BlendCharacterAnimationAndWaitForEnd00);
	tolua_function(L, "CurrentCharacterAnimation", tolua_ExportedFunctions_CurrentCharacterAnimation00);*/
	tolua_function(L, "SetCharacterPlayerVisible", tolua_ExportedFunctions_SetCharacterPlayerVisible00);
	tolua_function(L, "MoveCharacterPlayerDisabled",
				 tolua_ExportedFunctions_MoveCharacterPlayerDisabled00);
	/*tolua_function(L, "SetRunMode", tolua_ExportedFunctions_SetRunMode00);
	tolua_function(L, "SetRunMode2", tolua_ExportedFunctions_SetRunMode200);
	tolua_function(L, "SetCharacterColor", tolua_ExportedFunctions_SetCharacterColor00);
	tolua_function(L, "SetCharacterSound", tolua_ExportedFunctions_SetCharacterSound00);
	tolua_function(L, "SetCharacterShadow", tolua_ExportedFunctions_SetCharacterShadow00);
	tolua_function(L, "AddCallback", tolua_ExportedFunctions_AddCallback00);
	tolua_function(L, "AddCallbackPlayer", tolua_ExportedFunctions_AddCallbackPlayer00);
	tolua_function(L, "AddCallbackAnimation2D", tolua_ExportedFunctions_AddCallbackAnimation2D00);
	tolua_function(L, "DeleteCallback", tolua_ExportedFunctions_DeleteCallback00);
	tolua_function(L, "DeleteCallbackPlayer", tolua_ExportedFunctions_DeleteCallbackPlayer00);
	tolua_function(L, "DeleteCallbackAnimation2D", tolua_ExportedFunctions_DeleteCallbackAnimation2D00);
	tolua_function(L, "SetObjectOnCharacter", tolua_ExportedFunctions_SetObjectOnCharacter00);
	tolua_function(L, "SetObjectRotation", tolua_ExportedFunctions_SetObjectRotation00);
	tolua_function(L, "SetObjectTranslation", tolua_ExportedFunctions_SetObjectTranslation00);
	tolua_function(L, "SetObjectScale", tolua_ExportedFunctions_SetObjectScale00);
	tolua_function(L, "SetObjectFrames", tolua_ExportedFunctions_SetObjectFrames00);*/
	tolua_function(L, "LoadObject", tolua_ExportedFunctions_LoadObject00);
	tolua_function(L, "UnloadObject", tolua_ExportedFunctions_UnloadObject00);
	tolua_function(L, "SetGroundObjectPosition", tolua_ExportedFunctions_SetGroundObjectPosition00);
	tolua_function(L, "SetGroundObjectRotation", tolua_ExportedFunctions_SetGroundObjectRotation00);
	/*tolua_function(L, "TranslateGroundObject", tolua_ExportedFunctions_TranslateGroundObject00);
	tolua_function(L, "RotateGroundObject", tolua_ExportedFunctions_RotateGroundObject00);
	tolua_function(L, "SetLightPlayerCharacter", tolua_ExportedFunctions_SetLightPlayerCharacter00);
	tolua_function(L, "SetLightPos", tolua_ExportedFunctions_SetLightPos00);
	tolua_function(L, "EnableLight", tolua_ExportedFunctions_EnableLight00);
	tolua_function(L, "SetLightDiffuse", tolua_ExportedFunctions_SetLightDiffuse00);
	tolua_function(L, "SetLightAmbient", tolua_ExportedFunctions_SetLightAmbient00);
	tolua_function(L, "SetLightSpecular", tolua_ExportedFunctions_SetLightSpecular00);
	tolua_function(L, "LoadBillBoard", tolua_ExportedFunctions_LoadBillBoard00);
	tolua_function(L, "SetBillboardPosition", tolua_ExportedFunctions_SetBillboardPosition00);
	tolua_function(L, "SetBillboardPosition2", tolua_ExportedFunctions_SetBillboardPosition200);
	tolua_function(L, "SetBillboardSize", tolua_ExportedFunctions_SetBillboardSize00);
	tolua_function(L, "ShowBillboard", tolua_ExportedFunctions_ShowBillboard00);
	tolua_function(L, "HideBillboard", tolua_ExportedFunctions_HideBillboard00);
	tolua_function(L, "UnlockAchievement", tolua_ExportedFunctions_UnlockAchievement00);
	tolua_function(L, "Save", tolua_ExportedFunctions_Save00);
	tolua_function(L, "Wait", tolua_ExportedFunctions_Wait00);
	tolua_function(L, "WaitAndWaitForEnd", tolua_ExportedFunctions_WaitAndWaitForEnd00);
	tolua_function(L, "OpenFinalURL", tolua_ExportedFunctions_OpenFinalURL00);
	tolua_function(L, "FinishGame", tolua_ExportedFunctions_FinishGame00);
	tolua_function(L, "RequestMainMenu", tolua_ExportedFunctions_RequestMainMenu00);
	tolua_function(L, "BFGRateImmediately", tolua_ExportedFunctions_BFGRateImmediately00);
	tolua_function(L, "BFGReportEvent", tolua_ExportedFunctions_BFGReportEvent00);
	tolua_function(L, "BFGReportEventWithValue", tolua_ExportedFunctions_BFGReportEventWithValue00);
	tolua_function(L, "BFGReachedFreemiumLimit", tolua_ExportedFunctions_BFGReachedFreemiumLimit00);
	tolua_function(L, "TestFileFlagSystemFlag", tolua_ExportedFunctions_TestFileFlagSystemFlag00);
	tolua_function(L, "PrintDebugMessage", tolua_ExportedFunctions_PrintDebugMessage00);
	tolua_function(L, "ExitZone", tolua_ExportedFunctions_ExitZone00);*/
	tolua_function(L, "EnableRectBlocker", tolua_ExportedFunctions_EnableRectBlocker00);
	/*tolua_function(L, "EnableBlocker", tolua_ExportedFunctions_EnableBlocker00);*/
	tolua_function(L, "AddAnchorZone", tolua_ExportedFunctions_AddAnchorZone00);/*
	tolua_function(L, "ActivateAnchorZone", tolua_ExportedFunctions_ActivateAnchorZone00);
	tolua_function(L, "SetCharacterAnchor", tolua_ExportedFunctions_SetCharacterAnchor00);
	tolua_function(L, "SetCharacterLookChar", tolua_ExportedFunctions_SetCharacterLookChar00);
	tolua_function(L, "Random", tolua_ExportedFunctions_Random00);
	tolua_function(L, "SetCharacterMeshVisible", tolua_ExportedFunctions_SetCharacterMeshVisible00);
	tolua_function(L, "SetRecallageY", tolua_ExportedFunctions_SetRecallageY00);
	tolua_function(L, "IsFreemiumUnlocked", tolua_ExportedFunctions_IsFreemiumUnlocked00);
	tolua_function(L, "ReachedFreemiumLimit", tolua_ExportedFunctions_ReachedFreemiumLimit00);*/
	tolua_function(L, "AddUnrecalAnim", tolua_ExportedFunctions_AddUnrecalAnim00);
	tolua_function(L, "UnlockArtwork", tolua_ExportedFunctions_UnlockArtwork00);

	tolua_endmodule(L);
}


}

} // end namespace Tetraedge
