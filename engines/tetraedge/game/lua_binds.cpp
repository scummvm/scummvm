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
#include "tetraedge/game/billboard.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/lua_binds.h"
#include "tetraedge/game/object3d.h"
#include "tetraedge/game/syberia_game.h"
#include "tetraedge/game/amerzone_game.h"
#include "tetraedge/to_lua.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_sound_manager.h"
#include "tetraedge/te/te_lua_thread.h"
#include "tetraedge/te/te_particle.h"

namespace Tetraedge {

namespace LuaBinds {

using namespace ToLua;

static void LoadObjectMaterials(const Common::String &objname) {
	Game *game = g_engine->getGame();
	bool result = game->scene().loadObjectMaterials(objname);
	// Not an error if it fails, eg ValAttic/16050 calls this but has no object
	if (!result)
		warning("[LoadObjectMaterials] Object \"%s\" doesn't exist or no Object in this scene.",
				objname.c_str());
}

static int tolua_ExportedFunctions_LoadObjectMaterials00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		LoadObjectMaterials(s1);
		return 0;
	}
	error("#ferror in function 'LoadObjectMaterials': %d %d %s", err.index, err.array, err.type);
}

static void LoadObjectMaterials(const Common::String &imgname, const Common::String &objname) {
	Game *game = g_engine->getGame();
	bool result = game->scene().loadObjectMaterials(Common::Path(imgname), objname);
	if (!result)
		error("[LoadObjectMaterials] Object \"%s\" doesn't exist in scene : \"%s\" or there is no material for this object.",
				objname.c_str(), imgname.c_str());
}

static int tolua_ExportedFunctions_LoadObjectMaterials01(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		LoadObjectMaterials(s1, s2);
		return 0;
	}
	return tolua_ExportedFunctions_LoadObjectMaterials00(L);
}

static void PlayMovie(Common::String vidpath, Common::String musicpath) {
	Application *app = g_engine->getApplication();
	app->mouseCursorLayout().load(app->defaultCursor());
	Game *game = g_engine->getGame();

	// WORKAROUND: Fix some broken paths in Amerzone
	if (musicpath == "Videos/sc19.ogg")
		musicpath = "Videos/019.ogg";
	if (vidpath == "Videos/sc18.ogv") {
		// Need the correct path for callback, call this first.
		game->playMovie(Common::Path(vidpath), Common::Path(musicpath));
		vidpath = "Videos/I_018_P2_001.ogv";
	}

	game->playMovie(Common::Path(vidpath), Common::Path(musicpath));
}

static void PlayMovie(const Common::String &vidpath, const Common::String &musicpath, double volume) {
	Game *game = g_engine->getGame();

	if (!game->playMovie(Common::Path(vidpath), Common::Path(musicpath), (float)volume)) {
		warning("[PlayMovie] Movie \"%s\" doesn't exist.", vidpath.c_str());
		return;
	}

	warning("TODO: call Game::addMoviePlayed");
	//game->addMoviePlayed(vidpath, musicpath);
}


static int tolua_ExportedFunctions_PlayMovie00(lua_State *L) {
	tolua_Error err;
	// Syberia 1 has 2 string args, Syberia 2 adds a double arg.
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		PlayMovie(s1, s2);
		return 0;
	} else if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isnumber(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		// Syberia II macOS - includes volume
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double d1 = tolua_tonumber(L, 3, 1.0);
		PlayMovie(s1, s2, d1);
		return 0;
	} else if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isstring(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		// iOS version.. sometimes has "video" as 3rd param?
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		Common::String s3(tolua_tostring(L, 3, nullptr));
		PlayMovie(s1, s2);
		return 0;
	}
	error("#ferror in function 'PlayMovie': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_PlayMovieAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		PlayMovie(s1, s2);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnMovieFinished";
		callback._luaParam = s1;
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName && cb._luaParam == s1)
				warning("PlayMovieAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();
	}
	error("#ferror in function 'PlayMovieAndWaitForEnd': %d %d %s", err.index, err.array, err.type);
}

static void AddRandomSound(const Common::String &s1, const Common::String &s2, float f1, float f2){
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->addRandomSound(s1, Common::Path(s2), f1, f2);
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

static bool Selected(const Common::String &obj) {
	Game *game = g_engine->getGame();
	return game->inventory().selectedObject() == obj;
}

static int tolua_ExportedFunctions_Selected00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		bool result = Selected(s1);
		tolua_pushboolean(L, result);
		return 1;
	}
	error("#ferror in function 'Selected': %d %d %s", err.index, err.array, err.type);
}

static void TakeObject_Amerzone(const Common::String &obj) {
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game && game->warpY());
	debug("TakeObject: lastObj %s, obj %s", game->lastHitObjectName().c_str(), obj.c_str());
	game->luaContext().setGlobal(game->lastHitObjectName(), true);
	game->warpY()->takeObject(game->lastHitObjectName());
	if (!obj.empty()) {
		game->addToBag(obj);
		g_engine->getSoundManager()->playFreeSound("Sounds/SFX/N_prendre.ogg", 1.0, "sfx");
	}
}

static void TakeObject_Syberia(const Common::String &obj) {
	Game *game = g_engine->getGame();
	if (!obj.empty())
		game->addToBag(obj);
}

static int tolua_ExportedFunctions_TakeObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		if (g_engine->gameIsAmerzone())
			TakeObject_Amerzone(s1);
		else
			TakeObject_Syberia(s1);
		return 0;
	}
	error("#ferror in function 'TakeObject': %d %d %s", err.index, err.array, err.type);
}

static void TakeObjectInHand(const Common::String &obj) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	// TODO: Set global _lastHitObjectName?? How is it used?
	//game->luaContext().setGlobal(_lastHitObjectName, true);
	if (!obj.empty())
		game->addToHand(obj);
}

static int tolua_ExportedFunctions_TakeObjectInHand00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		TakeObjectInHand(s1);
		return 0;
	}
	error("#ferror in function 'TakeObjectInHand': %d %d %s", err.index, err.array, err.type);
}

static void RemoveObject(const Common::String &obj) {
	Game *game = g_engine->getGame();
	game->inventory().removeObject(obj);
}

static int tolua_ExportedFunctions_RemoveObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		RemoveObject(s1);
		return 0;
	}
	error("#ferror in function 'RemoveObject': %d %d %s", err.index, err.array, err.type);
}

static void RemoveObject() {
	Game *game = g_engine->getGame();
	game->inventory().removeSelectedObject();
}

static int tolua_ExportedFunctions_RemoveObject01(lua_State *L) {
	tolua_Error err;
	if (!tolua_isnoobj(L, 1, &err)) {
		tolua_ExportedFunctions_RemoveObject00(L);
	} else {
		RemoveObject();
	}
	return 0;
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

static void ShowDocument(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->documentsBrowser().showDocument(name, 0);
}

static int tolua_ExportedFunctions_ShowDocument00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		ShowDocument(s1);
		return 0;
	}
	error("#ferror in function 'ShowDocument': %d %d %s", err.index, err.array, err.type);
}

static void HideDocument() {
	Game *game = g_engine->getGame();
	game->documentsBrowser().hideDocument();
}

static int tolua_ExportedFunctions_HideDocument00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnoobj(L, 1, &err)) {
		HideDocument();
		return 0;
	}
	error("#ferror in function 'HideDocument': %d %d %s", err.index, err.array, err.type);
}

static void AddDocument(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->documentsBrowser().addDocument(name);
}

static int tolua_ExportedFunctions_AddDocument00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		AddDocument(s1);
		return 0;
	}
	error("#ferror in function 'AddDocument': %d %d %s", err.index, err.array, err.type);
}

static bool IsFreemiumUnlocked() {
	// Only called in demo versions of the game, so the answer is no.
	return false;
}

static int tolua_ExportedFunctions_IsFreemiumUnlocked00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnoobj(L, 2, &err)) {
		bool result = IsFreemiumUnlocked();
		tolua_pushboolean(L, result);
		return 1;
	}
	error("#ferror in function 'IsFreemiumUnlocked': %d %d %s", err.index, err.array, err.type);
}

static void ReachedFreemiumLimit() {
	Application *app = g_engine->getApplication();
	app->upsellScreen().enter();
}

static int tolua_ExportedFunctions_ReachedFreemiumLimit00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnoobj(L, 2, &err)) {
		ReachedFreemiumLimit();
		return 0;
	}
	error("#ferror in function 'ReachedFreemiumLimit': %d %d %s", err.index, err.array, err.type);
}

static bool IsFacebookLiked() {
	// Only called in demo versions of the game, so the answer is no.
	return false;
}

static int tolua_ExportedFunctions_IsFacebookLiked00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnoobj(L, 2, &err)) {
		bool result = IsFacebookLiked();
		tolua_pushboolean(L, result);
		return 1;
	}
	error("#ferror in function 'IsFacebookLiked': %d %d %s", err.index, err.array, err.type);
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
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
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
		bool flag = tolua_toboolean(L, 3, false);
		ChangeWarp(s1, s2, flag);
		return 0;
	} else if (g_engine->gameIsAmerzone() && tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 1, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		bool flag = tolua_toboolean(L, 2, false);
		ChangeWarp(s1, "", flag);
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
		SetCharacterPlayerVisible(tolua_toboolean(L, 1, false));
		return 0;
	}
	error("#ferror in function 'SetCharacterPlayerVisible': %d %d %s", err.index, err.array, err.type);
}

static void MoveCharacterPlayerDisabled(bool val) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->_movePlayerCharacterDisabled = val;
}

static int tolua_ExportedFunctions_MoveCharacterPlayerDisabled00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		MoveCharacterPlayerDisabled(tolua_toboolean(L, 1, false));
		return 0;
	}
	error("#ferror in function 'MoveCharacterPlayerDisabled': %d %d %s", err.index, err.array, err.type);
}

static void SetRunMode(bool run) {
	Game *game = g_engine->getGame();
	game->scene()._character->walkMode(run ? "Jog" : "Walk");
}

static int tolua_ExportedFunctions_SetRunMode00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		SetRunMode(tolua_toboolean(L, 1, false));
		return 0;
	}
	error("#ferror in function 'SetRunMode': %d %d %s", err.index, err.array, err.type);
}

static void SetRunMode2(const Common::String &charName, const Common::String &mode) {
	Game *game = g_engine->getGame();
	Character *character = game->scene().character(charName);
	if (character == game->scene()._character)
		return;

	if (character) {
		character->walkMode(mode);
	} else {
		debug("[SetRunMode2] Character not found %s", charName.c_str());
	}
}

static int tolua_ExportedFunctions_SetRunMode200(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			 && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		SetRunMode2(s1, s2);
		return 0;
	}
	error("#ferror in function 'SetRunMode2': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterShadow(const Common::String &charName, bool val) {
	//Game *game = g_engine->getGame();
	//Character *character = game->scene().character(charName);
	// Note: the game fetches the character then does nothing here??
}

static int tolua_ExportedFunctions_SetCharacterShadow00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err)
			 && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		bool b1 = tolua_toboolean(L, 2, false);
		SetCharacterShadow(s1, b1);
		return 0;
	}
	error("#ferror in function 'SetCharacterShadow': %d %d %s", err.index, err.array, err.type);
}

static void AddCallback(const Common::String &charName, const Common::String &animName, const Common::String &fnName, float triggerFrame, float maxCalls) {
	Game *game = g_engine->getGame();
	Character *c = game->scene().character(charName);
	if (c) {
		c->addCallback(animName, fnName, triggerFrame, maxCalls);
	} else {
		warning("[AddCallback] Character's \"%s\" doesn't exist", charName.c_str());
	}
}

static int tolua_ExportedFunctions_AddCallback00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isstring(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
			&& tolua_isnumber(L, 5, 1, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		Common::String s3(tolua_tostring(L, 3, nullptr));
		double n1 = tolua_tonumber(L, 4, 0.0);
		double n2 = tolua_tonumber(L, 5, -1.0);
		AddCallback(s1, s2, s3, n1, n2);
		return 0;
	}
	error("#ferror in function 'AddCallback': %d %d %s", err.index, err.array, err.type);
}

static void AddCallbackPlayer(const Common::String &animName, const Common::String &fnName, float triggerFrame, float maxCalls) {
	Game *game = g_engine->getGame();
	Character *c = game->scene()._character;
	assert(c);
	c->addCallback(animName, fnName, triggerFrame, maxCalls);
}

static int tolua_ExportedFunctions_AddCallbackPlayer00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 1, &err)
			&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double n1 = tolua_tonumber(L, 3, 0.0);
		double n2 = tolua_tonumber(L, 4, -1.0);
		AddCallbackPlayer(s1, s2, n1, n2);
		return 0;
	}
	error("#ferror in function 'AddCallbackPlayer': %d %d %s", err.index, err.array, err.type);
}

static void DeleteCallback(const Common::String &charName, const Common::String &animName, const Common::String &fnName, float triggerFrame) {
	Game *game = g_engine->getGame();
	Character *c = game->scene().character(charName);
	if (c) {
		c->deleteCallback(animName, fnName, triggerFrame);
	} else {
		warning("[DeleteCallback] Character's \"%s\" doesn't exist", charName.c_str());
	}
}

static int tolua_ExportedFunctions_DeleteCallback00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isstring(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
			&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		Common::String s3(tolua_tostring(L, 3, nullptr));
		double n1 = tolua_tonumber(L, 4, 0.0);
		DeleteCallback(s1, s2, s3, n1);
		return 0;
	}
	error("#ferror in function 'DeleteCallback': %d %d %s", err.index, err.array, err.type);
}

static void DeleteCallbackPlayer(const Common::String &animName, const Common::String &fnName, float triggerFrame) {
	Game *game = g_engine->getGame();
	Character *c = game->scene()._character;
	assert(c);
	c->deleteCallback(animName, fnName, triggerFrame);
}

static int tolua_ExportedFunctions_DeleteCallbackPlayer00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s2(tolua_tostring(L, 1, nullptr));
		Common::String s3(tolua_tostring(L, 2, nullptr));
		double n1 = tolua_tonumber(L, 3, 0.0);
		DeleteCallbackPlayer(s2, s3, n1);
		return 0;
	}
	error("#ferror in function 'DeleteCallbackPlayer': %d %d %s", err.index, err.array, err.type);
}

static void AddMarker(const Common::String &markerName, const Common::String &imgPath, float x, float y,
				const Common::String &loctype, const Common::String &markerVal, float anchorX, float anchorY) {
	Game *game = g_engine->getGame();
	game->scene().addMarker(markerName, Common::Path(imgPath), x, y, loctype, markerVal, anchorX, anchorY);
}

static int tolua_ExportedFunctions_AddMarker00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
			&& tolua_isstring(L, 5, 1, &err) && tolua_isstring(L, 6, 1, &err)
			&& tolua_isnumber(L, 7, 1, &err) && tolua_isnumber(L, 8, 1, &err)
			&& tolua_isnoobj(L, 9, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double n1 = tolua_tonumber(L, 3, 0.0);
		double n2 = tolua_tonumber(L, 4, 0.0);
		Common::String s3(tolua_tostring(L, 5, ""));
		Common::String s4(tolua_tostring(L, 6, ""));
		double n3 = tolua_tonumber(L, 7, 0.0);
		double n4 = tolua_tonumber(L, 8, 0.0);
		AddMarker(s1, s2, n1, n2, s3, s4, n3, n4);
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
		bool b = tolua_toboolean(L, 2, false);
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
		SetVisibleCellphone(tolua_toboolean(L, 1, false));
		return 0;
	}
	error("#ferror in function 'SetVisibleCellphone': %d %d %s", err.index, err.array, err.type);
}

static void ShowObject(const Common::String &objName);

static void StartAnimation(const Common::String &name, int loops, bool repeat) {
	ShowObject(name);
	Game *game = g_engine->getGame();
	if (game->startAnimation(name, loops, repeat))
		return;

	// NOTE: Not error, some game scripts try to start animations that don't
	// exist.  eg, ValVoralberg/14030 loads anim 14020, which is in a different
	// zone
	warning("[StartAnimation] Animation \"%s\" doesn't exist.", name.c_str());
}

int tolua_ExportedFunctions_StartAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 1, &err)
		&& tolua_isboolean(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		double d1 = tolua_tonumber(L, 2, -1.0);
		bool b1 = tolua_toboolean(L, 3, false);
		StartAnimation(s1, d1, b1);
		return 0;
	}
	error("#ferror in function 'StartAnimation': %d %d %s", err.index, err.array, err.type);

}

int tolua_ExportedFunctions_StartAnimationAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 1, &err)
		&& tolua_isboolean(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		double d1 = tolua_tonumber(L, 2, -1.0);
		bool b1 = tolua_toboolean(L, 3, false);
		StartAnimation(s1, d1, b1);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnFinishedAnim";
		callback._luaParam = s1;
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName && cb._luaParam == s1)
				warning("StartAnimationAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();

	}
	error("#ferror in function 'StartAnimationAndWaitForEnd': %d %d %s", err.index, err.array, err.type);

}

static void RequestAutoSave() {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->setSaveRequested();
}

static int tolua_ExportedFunctions_RequestAutoSave00(lua_State *L) {
	RequestAutoSave();
	return 0;
}

static void SetVisibleButtonZoomed(bool val) {
	Game *game = g_engine->getGame();
	TeButtonLayout *btn = game->scene().hitObjectGui().buttonLayout("DeZoomedButton");
	if (!btn) {
		debug("[SetVisibleButtonZoomed] No \"DeZoomedButton\" in this scene");
	} else {
		btn->setVisible(val);
	}
}

static int tolua_ExportedFunctions_SetVisibleButtonZoomed00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		bool b1 = tolua_toboolean(L, 1, false);
		SetVisibleButtonZoomed(b1);
		return 0;
	}
	error("#ferror in function 'SetVisibleButtonZoomed': %d %d %s", err.index, err.array, err.type);

}

static void HideObject(const Common::String &objName) {
	Game *game = g_engine->getGame();
	TeIntrusivePtr<TeModel> model = game->scene().model(objName);
	if (model) {
		model->setVisible(false);
		return;
	}

	//debug("[HideObject] Object 3D \"%s\" doesn't exist.", objName.c_str());
	TeLayout *layout = game->scene().bgGui().layout(objName);
	if (layout) {
		layout->setVisible(false);
		return;
	}

	//debug("[HideObject] \"Set\" Object 2D \"%s\" doesn't exist.", objName.c_str());
	layout = game->forGui().layout(objName);
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

	//debug("[ShowObject] Object 3D \"%s\" doesn't exist.", objName.c_str());
	TeLayout *layout = game->scene().bgGui().layout(objName);
	if (layout) {
		layout->setVisible(true);
		return;
	}

	//debug("[ShowObject] \"Set\" Object 2D \"%s\" doesn't exist.", objName.c_str());
	layout = game->forGui().layout(objName);
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

static void PlaceCharacterOnDummy(const Common::String &charname, const Common::String &dummyname, float x, float y, float z) {
	Game *game = g_engine->getGame();
	Character *c = game->scene().character(charname);
	if (c) {
		InGameScene::Dummy dummy = game->scene().dummy(dummyname);
		c->_model->setPosition(dummy._position + TeVector3f32(x, y, z));
	} else {
		warning("[PlaceCharacterOnDummy] Character not found %s", charname.c_str());
	}
}

static int tolua_ExportedFunctions_PlaceCharacterOnDummy00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnumber(L, 5, 0, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		float f1 = tolua_tonumber(L, 3, 0.0);
		float f2 = tolua_tonumber(L, 4, 0.0);
		float f3 = tolua_tonumber(L, 5, 0.0);
		PlaceCharacterOnDummy(s1, s2, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetCharacterRotation': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterRotation(const Common::String &charname, float rx, float ry, float rz) {
	const TeQuaternion quat = TeQuaternion::fromEulerDegrees(TeVector3f32(rx, ry, rz));
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
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetCharacterRotation(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetCharacterRotation': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterOrientation(const Common::String &charname, float x, float y) {
	Game *game = g_engine->getGame();
	Character *c = game->scene().character(charname);
	if (c) {
		const TeVector3f32 pos = c->_model->position();
		const TeVector3f32 euler(0, atan2f(-(x - pos.x()), y - pos.z()), 0);
		c->_model->setRotation(TeQuaternion::fromEuler(euler));
	} else {
		warning("[SetCharacterOrientation] Character not found %s", charname.c_str());
	}
}

static int tolua_ExportedFunctions_SetCharacterOrientation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		SetCharacterOrientation(s1, f1, f2);
		return 0;
	}
	error("#ferror in function 'SetCharacterOrientation': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterAnimation(const Common::String &charname, const Common::String &animname, bool repeat, bool returnToIdle, int startframe, int endframe) {
	Game *game = g_engine->getGame();
	Character *c = game->scene().character(charname);
	assert(c);
	bool result = c->setAnimation(animname, repeat, returnToIdle, false, startframe, endframe);
	if (!result) {
		warning("[SetCharacterAnimation] Character's animation \"%s\" doesn't exist for the character\"%s\"  ",
			animname.c_str(), charname.c_str());
	}
}

static int tolua_ExportedFunctions_SetCharacterAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isboolean(L, 3, 1, &err) && tolua_isboolean(L, 4, 1, &err)
		&& tolua_isnumber(L, 5, 1, &err) && tolua_isnumber(L, 6, 1, &err)
		&& tolua_isnoobj(L, 7, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		bool b1 = tolua_toboolean(L, 3, true);
		bool b2 = tolua_toboolean(L, 4, false);
		double f3 = tolua_tonumber(L, 5, -1.0);
		double f4 = tolua_tonumber(L, 6, 9999.0);
		SetCharacterAnimation(s1, s2, b1, b2, (int)f3, (int)f4);
		return 0;
	}
	// Incorrectly called in scenes/ValTrain/19000 line 305 with a 0.5 parameter instead of bool
	warning("#ferror in function 'SetCharacterAnimation': %d %d %s", err.index, err.array, err.type);
	return 0;
}

static int tolua_ExportedFunctions_SetCharacterAnimationAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isboolean(L, 3, 1, &err) && tolua_isboolean(L, 4, 1, &err)
		&& tolua_isnumber(L, 5, 1, &err) && tolua_isnumber(L, 6, 1, &err)
		&& tolua_isnoobj(L, 7, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		bool b1 = tolua_toboolean(L, 3, true);
		bool b2 = tolua_toboolean(L, 4, false);
		double f3 = tolua_tonumber(L, 5, -1.0);
		double f4 = tolua_tonumber(L, 6, 9999.0);
		SetCharacterAnimation(s1, s2, b1, b2, (int)f3, (int)f4);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnCharacterAnimationFinished";
		callback._luaParam = s1;
		callback._luaParam2 = s2;
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName && cb._luaParam == s1 && cb._luaParam2 == s2)
				warning("SetCharacterAnimationAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();
	}
	error("#ferror in function 'SetCharacterAnimationAndWaitForEnd': %d %d %s", err.index, err.array, err.type);
}

static void BlendCharacterAnimation(const Common::String &charname, const Common::String &animname, float blendAmount, bool repeat, bool returnToIdle) {
	Game *game = g_engine->getGame();
	Character *c = game->scene().character(charname);
	bool result = c && c->blendAnimation(animname, blendAmount, repeat, returnToIdle);
	if (!result) {
		warning("[BlendCharacterAnimation] Character's animation \"%s\" doesn't exist for the character\"%s\"  ",
			animname.c_str(), charname.c_str());
	}
}

static int tolua_ExportedFunctions_BlendCharacterAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isboolean(L, 4, 1, &err)
		&& tolua_isboolean(L, 5, 1, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double f1 = tolua_tonumber(L, 3, 0.0);
		bool b1 = tolua_toboolean(L, 4, true);
		bool b2 = tolua_toboolean(L, 5, false);
		BlendCharacterAnimation(s1, s2, f1, b1, b2);
		return 0;
	}
	error("#ferror in function 'BlendCharacterAnimation': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_BlendCharacterAnimationAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isboolean(L, 4, 1, &err)
		&& tolua_isboolean(L, 5, 1, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double f1 = tolua_tonumber(L, 3, 0.0);
		bool b1 = tolua_toboolean(L, 4, true);
		bool b2 = tolua_toboolean(L, 5, false);
		BlendCharacterAnimation(s1, s2, f1, b1, b2);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnCharacterAnimationFinished";
		callback._luaParam = s1;
		callback._luaParam2 = s2;
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName && cb._luaParam == s1 && cb._luaParam2 == s2)
				warning("BlendCharacterAnimationAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();
	}
	error("#ferror in function 'BlendCharacterAnimationAndWaitForEnd': %d %d %s", err.index, err.array, err.type);
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
		float f2 = tolua_tonumber(L, 4, 0.0);
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
		float f3 = tolua_tonumber(L, 4, 0.0);
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

	obj->model()->setRotation(TeQuaternion::fromEulerDegrees(TeVector3f32(x, y, z)));
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
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetGroundObjectRotation(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetGroundObjectRotation': %d %d %s", err.index, err.array, err.type);
}

static void TranslateGroundObject(const Common::String &name, float x, float y, float z, float time) {
	Game *game = g_engine->getGame();
	Object3D *obj = game->scene().object3D(name);
	if (!obj)
		error("[TranslateGroundObject] Object not found %s", name.c_str());
	TeVector3f32 pos = obj->model()->position();
	obj->_translateStart = pos;
	obj->_translateAmount = TeVector3f32(x, y, z);
	obj->_translateTimer.start();
	obj->_translateTime = time;
}

static int tolua_ExportedFunctions_TranslateGroundObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnumber(L, 5, 0, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		float f4 = tolua_tonumber(L, 5, 0.0);
		TranslateGroundObject(s1, f1, f2, f3, f4);
		return 0;
	}
	error("#ferror in function 'TranslateGroundObject': %d %d %s", err.index, err.array, err.type);
}

static void RotateGroundObject(const Common::String &name, float x, float y, float z, float time) {
	Game *game = g_engine->getGame();
	Object3D *obj = game->scene().object3D(name);
	if (!obj)
		error("[RotateGroundObject] Object not found %s", name.c_str());
	obj->_rotateStart = obj->model()->rotation();
	obj->_rotateAmount = TeVector3f32(x, y, z);
	obj->_rotateTimer.start();
	obj->_rotateTime = time;
}

static int tolua_ExportedFunctions_RotateGroundObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnumber(L, 5, 0, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		float f4 = tolua_tonumber(L, 5, 0.0);
		RotateGroundObject(s1, f1, f2, f3, f4);
		return 0;
	}
	error("#ferror in function 'RotateGroundObject': %d %d %s", err.index, err.array, err.type);
}

static void EnableLight(uint lightno, bool enable) {
	Game *game = g_engine->getGame();
	if (lightno >= game->scene().lights().size()) {
		error("[EnableLight] Light not found %d", lightno);
	}
	Common::SharedPtr<TeLight> light = game->scene().lights()[lightno];
	if (enable)
		light->enable(lightno);
	else
		light->disable(lightno);
}

static int tolua_ExportedFunctions_EnableLight00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		float f1 = tolua_tonumber(L, 1, 0.0);
		bool b1 = tolua_toboolean(L, 2, false);
		EnableLight(f1, b1);
		return 0;
	}
	error("#ferror in function 'EnableLight': %d %d %s", err.index, err.array, err.type);
}

static void LoadBillBoard(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->scene().loadBillboard(name);
}

static int tolua_ExportedFunctions_LoadBillBoard00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		LoadBillBoard(s1);
		return 0;
	}
	error("#ferror in function 'LoadBillBoard': %d %d %s", err.index, err.array, err.type);
}

static void SetBillboardPosition(const Common::String &name, float x, float y, float z) {
	Game *game = g_engine->getGame();
	Billboard *bb = game->scene().billboard(name);
	if (!bb) {
		error("[SetBillboardPosition] Billboard not found %s", name.c_str());
	}
	bb->position(TeVector3f32(x, y, z));
}

static int tolua_ExportedFunctions_SetBillboardPosition00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetBillboardPosition(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetBillboardPosition': %d %d %s", err.index, err.array, err.type);
}

static void SetBillboardPosition2(const Common::String &name, float x1, float y1, float x2, float y2, float z) {
	Game *game = g_engine->getGame();
	Billboard *bb = game->scene().billboard(name);
	if (!bb) {
		error("[SetBillboardPosition2] Billboard not found %s", name.c_str());
	}
	bb->position(TeVector3f32(x1, y1, 0.0));
	bb->position2(TeVector3f32(x2, y2, z));
}

static int tolua_ExportedFunctions_SetBillboardPosition200(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnumber(L, 5, 0, &err) && tolua_isnumber(L, 6, 0, &err)
		&& tolua_isnoobj(L, 7, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		float f4 = tolua_tonumber(L, 5, 0.0);
		float f5 = tolua_tonumber(L, 6, 0.0);
		SetBillboardPosition2(s1, f1, f2, f3, f4, f5);
		return 0;
	}
	error("#ferror in function 'SetBillboardPosition2': %d %d %s", err.index, err.array, err.type);
}

static void SetBillboardSize(const Common::String &name, float xs, float ys) {
	Game *game = g_engine->getGame();
	Billboard *bb = game->scene().billboard(name);
	if (!bb) {
		error("[SetBillboardSize] Billboard not found %s", name.c_str());
	}
	bb->size(TeVector2f32(xs, ys));
}

static int tolua_ExportedFunctions_SetBillboardSize00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		SetBillboardSize(s1, f1, f2);
		return 0;
	}
	error("#ferror in function 'SetBillboardSize': %d %d %s", err.index, err.array, err.type);
}

static void ShowBillboard(const Common::String &name) {
	Game *game = g_engine->getGame();
	Billboard *bb = game->scene().billboard(name);
	if (!bb) {
		// Syberia 2 uses billboard A1_RomHaut/11100/A11100-01-04.png but never
		// loads it..
		warning("[ShowBillboard] Billboard not found %s", name.c_str());
		return;
	}
	bb->model()->setVisible(true);
}

static int tolua_ExportedFunctions_ShowBillboard00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		ShowBillboard(s1);
		return 0;
	}
	error("#ferror in function 'ShowBillboard': %d %d %s", err.index, err.array, err.type);
}

static void HideBillboard(const Common::String &name) {
	Game *game = g_engine->getGame();
	Billboard *bb = game->scene().billboard(name);
	if (!bb) {
		warning("[HideBillboard] Billboard not found %s", name.c_str());
		return;
	}
	bb->model()->setVisible(false);
}

static int tolua_ExportedFunctions_HideBillboard00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		HideBillboard(s1);
		return 0;
	}
	error("#ferror in function 'HideBillboard': %d %d %s", err.index, err.array, err.type);
}

static void UnlockAchievement(int val) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->addToScore(val);
}

static int tolua_ExportedFunctions_UnlockAchievement00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		double d = tolua_tonumber(L, 1, 0.0);
		UnlockAchievement(d);
		return 0;
	}
	error("#ferror in function 'UnlockAchievement': %d %d %s", err.index, err.array, err.type);
}

static void Save(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->saveBackup(name);
}

static int tolua_ExportedFunctions_Save00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Save(s1);
		return 0;
	}
	error("#ferror in function 'Save': %d %d %s", err.index, err.array, err.type);
}

static void Wait(double seconds) {
	Game *game = g_engine->getGame();
	game->scene().waitTimeTimer().start();
	game->scene().waitTimeTimer().stop();
	game->scene().waitTimeTimer().start();
	game->scene().setWaitTime(seconds * 1000000.0);
}

static int tolua_ExportedFunctions_Wait00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		double d = tolua_tonumber(L, 1, 0.0);
		Wait(d);
		return 0;
	}
	error("#ferror in function 'Wait': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_WaitAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		double d = tolua_tonumber(L, 1, 0.0);
		Wait(d);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnWaitFinished";
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName)
				warning("WaitAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();
	}
	error("#ferror in function 'WaitAndWaitForEnd': %d %d %s", err.index, err.array, err.type);
}

static void FinishGame() {
	Game *game = g_engine->getGame();
	game->finishGame();
}

static int tolua_ExportedFunctions_FinishGame00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnoobj(L, 1, &err)) {
		FinishGame();
		return 0;
	}
	error("#ferror in function 'FinishGame': %d %d %s", err.index, err.array, err.type);
}

static void RequestMainMenu() {
	Game *game = g_engine->getGame();
	game->_returnToMainMenu = true;
}

static int tolua_ExportedFunctions_RequestMainMenu00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnoobj(L, 1, &err)) {
		RequestMainMenu();
		return 0;
	}
	error("#ferror in function 'RequestMainMenu': %d %d %s", err.index, err.array, err.type);
}

static void SetBackground(const Common::String &name) {
	Game *game = g_engine->getGame();
	if (!game->setBackground(Common::Path(name)))
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

static void LaunchDialog(const Common::String &name, uint param_2, const Common::String &charname,
						const Common::String &animfile, float animblend) {
	Game *game = g_engine->getGame();

	if (!game->launchDialog(name, param_2, charname, animfile, animblend))
		warning("[LaunchDialog] Dialog \"%s\" doesn't exist.", name.c_str());
}

static int tolua_ExportedFunctions_LaunchDialog00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 1, &err)
		&& tolua_isstring(L, 3, 1, &err) && tolua_isstring(L, 4, 1, &err)
		&& tolua_isnumber(L, 5, 1, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 1.0);
		Common::String s2(tolua_tostring(L, 3, ""));
		Common::String s3(tolua_tostring(L, 4, ""));
		float f2 = tolua_tonumber(L, 5, 0.0);
		LaunchDialog(s1, f1, s2, s3, f2);
		return 0;
	}
	error("#ferror in function 'LaunchDialog': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_LaunchDialogAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 1, &err)
		&& tolua_isstring(L, 3, 1, &err) && tolua_isstring(L, 4, 1, &err)
		&& tolua_isnumber(L, 5, 1, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 1.0);
		Common::String s2(tolua_tostring(L, 3, ""));
		Common::String s3(tolua_tostring(L, 4, ""));
		float f2 = tolua_tonumber(L, 5, 0.0);
		LaunchDialog(s1, f1, s2, s3, f2);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnDialogFinished";
		callback._luaParam = s1;
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName && cb._luaParam == callback._luaParam)
				warning("LaunchDialogAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();
	}
	error("#ferror in function 'LaunchDialogAndWaitForEnd': %d %d %s", err.index, err.array, err.type);
}

static void PushAnswer(const Common::String &val, const Common::String &gui) {
	Application *app = g_engine->getApplication();
	const Common::String *locVal = app->loc().value(val);
	Common::String locValStr;
	if (locVal) {
		locValStr = *locVal;
	}
	Game *game = g_engine->getGame();
	game->question2().pushAnswer(val, locValStr, Common::Path(gui));
}

static int tolua_ExportedFunctions_PushAnswer00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		PushAnswer(s1, s2);
		return 0;
	}
	error("#ferror in function 'PushAnswer': %d %d %s", err.index, err.array, err.type);
}

static void HideAnswers() {
	g_engine->getGame()->question2().leave();
}

static int tolua_ExportedFunctions_HideAnswers00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnoobj(L, 1, &err)) {
		HideAnswers();
		return 0;
	}
	error("#ferror in function 'HideAnswers': %d %d %s", err.index, err.array, err.type);
}

static void PushTask(const Common::String &s1, const Common::String &s2) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
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

static void DeleteTask(const Common::String &s1, const Common::String &s2) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->objectif().deleteObjectif(s1, s2);
}

static int tolua_ExportedFunctions_DeleteTask00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		DeleteTask(s1, s2);
		return 0;
	}
	// Note: There is an incorrect call to this in scenes/ValChurch/13120/Logic13120.lua
	// which only passes a single string.
	warning("#ferror in function 'DeleteTask': %d %d %s", err.index, err.array, err.type);
	return 0;
}

static void SetVisibleButtonHelp(bool val) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->objectif().setVisibleButtonHelp(val);
}

static int tolua_ExportedFunctions_SetVisibleButtonHelp00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		bool b1 = tolua_toboolean(L, 1, false);
		SetVisibleButtonHelp(b1);
		return 0;
	}
	error("#ferror in function 'SetVisibleButtonHelp': %d %d %s", err.index, err.array, err.type);
}

static bool TestFileFlagSystemFlag(const Common::String &flagname, const Common::String &val) {
	// Syberia 1 always returns that it is Android. Syberia 2 doesn't.
	if (g_engine->gameType() == TetraedgeEngine::kSyberia
			&& flagname == "platform" && val == "Android")
		return true;
	return g_engine->getCore()->fileFlagSystemFlag(flagname) == val;
}

static int tolua_ExportedFunctions_TestFileFlagSystemFlag00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		bool result = TestFileFlagSystemFlag(s1, s2);
		tolua_pushboolean(L, result);
		return 1;
	}
	error("#ferror in function 'TestFileFlagSystemFlag': %d %d %s", err.index, err.array, err.type);
}

static void ExitZone(const Common::String &zone) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->setExitZone(zone);
}

static int tolua_ExportedFunctions_ExitZone00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		ExitZone(s1);
		return 0;
	}
	error("#ferror in function 'ExitZone': %d %d %s", err.index, err.array, err.type);
}

static void EnableRectBlocker(uint offset, bool enabled) {
	Game *game = g_engine->getGame();
	if (game->scene().rectBlockers().size() < offset)
		error("invalid rectblocker offset %d", offset);

	game->scene().rectBlockers()[offset]._enabled = enabled;
}

static int tolua_ExportedFunctions_EnableRectBlocker00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		double d1 = tolua_tonumber(L, 1, 0.0f);
		bool b1 = tolua_toboolean(L, 2, false);
		EnableRectBlocker((uint)d1, b1);
		return 0;
	}
	error("#ferror in function 'EnableRectBlocker': %d %d %s", err.index, err.array, err.type);
}

static void EnableBlocker(uint offset, bool enabled) {
	Game *game = g_engine->getGame();
	if (game->scene().blockers().size() < offset)
		error("invalid blocker offset %d", offset);

	game->scene().blockers()[offset]._enabled = enabled;
}

static int tolua_ExportedFunctions_EnableBlocker00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		double d1 = tolua_tonumber(L, 1, 0.0f);
		bool b1 = tolua_toboolean(L, 2, false);
		EnableBlocker((uint)d1, b1);
		return 0;
	}
	error("#ferror in function 'EnableBlocker': %d %d %s", err.index, err.array, err.type);
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
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isnumber(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double d1 = tolua_tonumber(L, 3, 1.0);
		AddAnchorZone(s1, s2, d1);
		return 0;
	}
	error("#ferror in function 'AddAnchorZone': %d %d %s", err.index, err.array, err.type);
}

static void ActivateAnchorZone(const Common::String &name, bool b) {
	if (!name.empty()) {
		Game *game = g_engine->getGame();
		game->scene().activateAnchorZone(name, b);
	}
}

static int tolua_ExportedFunctions_ActivateAnchorZone00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s(tolua_tostring(L, 1, nullptr));
		bool b = tolua_toboolean(L, 2, false);
		ActivateAnchorZone(s, b);
		return 0;
	}
	error("#ferror in function 'ActivateAnchorZone': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterLookChar(const Common::String &charname, const Common::String &destname, bool tall, float f) {
	Game *game = g_engine->getGame();
	Character *character = game->scene().character(charname);
	if (!character) {
		warning("[SetCharacterLookChar] Character \"%s\" doesn't exist", charname.c_str());
		return;
	}
	character->setLookingAtTallThing(tall);
	character->setCharLookingAtOffset(f);

	if (destname.empty()) {
		character->setCharLookingAt(nullptr);
	} else {
		Character *destchar = game->scene().character(destname);
		character->setCharLookingAt(destchar);
		if (destchar)
			return;
	}
	character->setLastHeadRotation(character->headRotation());
	character->setHasAnchor(false);
}

static int tolua_ExportedFunctions_SetCharacterLookChar00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isboolean(L, 3, 1, &err) && tolua_isnumber(L, 4, 1, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		bool b = tolua_toboolean(L, 3, true);
		float f = tolua_tonumber(L, 4, 0.0);
		SetCharacterLookChar(s1, s2, b, f);
		return 0;
	}
	error("#ferror in function 'SetCharacterLookChar': %d %d %s", err.index, err.array, err.type);
}

static uint Random(uint max) {
	return g_engine->getRandomNumber(max - 1);
}

static int tolua_ExportedFunctions_Random00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		double d1 = tolua_tonumber(L, 1, 0.0);
		uint result = Random(d1);
		tolua_pushnumber(L, result);
		return 1;
	}
	error("#ferror in function 'Random': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterMeshVisible(const Common::String &charName, const Common::String &meshName, bool val) {
	Character *character = g_engine->getGame()->scene().character(charName);
	if (character) {
		character->_model->setVisibleByName(meshName, val);
	} else {
		error("[SetCharacterMeshVisible] Character not found %s", charName.c_str());
	}
}

static int tolua_ExportedFunctions_SetCharacterMeshVisible00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isboolean(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		bool b = tolua_toboolean(L, 3, false);
		SetCharacterMeshVisible(s1, s2, b);
		return 0;
	}
	error("#ferror in function 'SetCharacterMeshVisible': %d %d %s", err.index, err.array, err.type);
}

static void SetRecallageY(const Common::String &charName, bool val) {
	Character *character = g_engine->getGame()->scene().character(charName);
	if (character) {
		character->setRecallageY(val);
	} else {
		// Not an error - some scenes in Syberia 2 look for a Kate that isn't
		// loaded yet.
		warning("[SetRecallageY] Character not found %s", charName.c_str());
	}
}

static int tolua_ExportedFunctions_SetRecallageY00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 1, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s(tolua_tostring(L, 1, nullptr));
		bool b = tolua_toboolean(L, 2, false);
		SetRecallageY(s, b);
		return 0;
	}
	error("#ferror in function 'SetRecallageY': %d %d %s", err.index, err.array, err.type);
}

static void DisabledZone(const Common::String &zone, bool disable) {
	Game *game = g_engine->getGame();
	if (!game->scene().markerGui().loaded())
		return;

	TeLayout *bg = game->scene().markerGui().layout("background");
	if (!bg) {
		warning("DisabledZone(%s): No background in markerGui", zone.c_str());
		return;
	}
	for (auto *child : bg->childList()) {
		TeLayout *childLayout = dynamic_cast<TeLayout *>(child);
		if (!childLayout)
			continue;
		if (child->name() == zone) {
			child->setVisible(!disable);
		}
	}
}

static int tolua_ExportedFunctions_DisabledZone00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 1, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s(tolua_tostring(L, 1, nullptr));
		bool b = tolua_toboolean(L, 2, true);
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
		bool b = tolua_toboolean(L, 2, true);
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
		bool b = tolua_toboolean(L, 1, false);
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
		//
		// WORKAROUND: Syberia 2 script A1_MonCour/12200/Logic12200.lua
		// calls this with (0.4, -1) - should be the other way around, and
		// the sound name should have ".ogg" on the end.
		//
		if (d2 == -1 && s1 == "sounds/SFX/AB_12220-01") {
			double tmp = d2;
			d2 = d1;
			d1 = tmp;
			s1 += ".ogg";
		}
		PlaySound(s1, d1, d2);
		return 0;
	}
	error("#ferror in function 'PlaySound': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_PlaySoundAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 1, &err) && tolua_isnumber(L, 3, 1, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		double d1 = tolua_tonumber(L, 2, -1.0);
		double d2 = tolua_tonumber(L, 3, 1.0);
		PlaySound(s1, d1, d2);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnFreeSoundFinished";
		callback._luaParam = s1;
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName && cb._luaParam == s1)
				warning("PlaySoundAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();
	}
	error("#ferror in function 'PlaySoundAndWaitForEnd': %d %d %s", err.index, err.array, err.type);
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
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
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

static void PlayMusic(const Common::String &path, float volume) {
	TeMusic &music = g_engine->getApplication()->music();
	// Note: stop and set repeat before starting,
	// very slightly different to original because we can't
	// change repeat value after starting.
	music.stop();
	music.repeat(g_engine->gameIsAmerzone());
	music.load(Common::Path(path));
	if (!g_engine->getApplication()->musicOn())
		return;
	music.play();
	music.volume(volume);
}

static int tolua_ExportedFunctions_PlayMusic00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 1, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 1.0);
		PlayMusic(s1, f1);
		return 0;
	}
	error("#ferror in function 'PlayMusic': %d %d %s", err.index, err.array, err.type);
}

static void SetObjectOnCharacter(const Common::String &charName, const Common::String &objName, const Common::String &boneName) {
	Game *game = g_engine->getGame();
	Object3D *obj3d = game->scene().object3D(objName);
	if (!obj3d) {
		warning("[SetObjectOnCharacter] Object not found %s", objName.c_str());
		return;
	}

	obj3d->_onCharName = charName;
	obj3d->_onCharBone = boneName;
}

static int tolua_ExportedFunctions_SetObjectOnCharacter00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
			&& tolua_isstring(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		Common::String s3(tolua_tostring(L, 3, nullptr));
		SetObjectOnCharacter(s1, s2, s3);
		return 0;
	}
	error("#ferror in function 'SetObjectOnCharacter': %d %d %s", err.index, err.array, err.type);
}

static void SetObjectRotation(const Common::String &obj, float xr, float yr, float zr) {
	Game *game = g_engine->getGame();
	Object3D *obj3d = game->scene().object3D(obj);
	if (!obj3d) {
		warning("[SetObjectRotation] Object not found %s", obj.c_str());
		return;
	}
	obj3d->_objRotation = TeQuaternion::fromEulerDegrees(TeVector3f32(xr, yr, zr));
}

static int tolua_ExportedFunctions_SetObjectRotation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetObjectRotation(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetObjectRotation': %d %d %s", err.index, err.array, err.type);
}

static void SetObjectTranslation(const Common::String &obj, float x, float y, float z) {
	Game *game = g_engine->getGame();
	Object3D *obj3d = game->scene().object3D(obj);
	if (!obj3d) {
		warning("[SetObjectTranslation] Object not found %s", obj.c_str());
		return;
	}
	obj3d->_objTranslation = TeVector3f32(x, y, z);
}

static int tolua_ExportedFunctions_SetObjectTranslation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetObjectTranslation(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetObjectTranslation': %d %d %s", err.index, err.array, err.type);
}

static void SetObjectScale(const Common::String &obj, float xs, float ys, float zs) {
	Game *game = g_engine->getGame();
	Object3D *obj3d = game->scene().object3D(obj);
	if (!obj3d) {
		warning("[SetObjectScale] Object not found %s", obj.c_str());
		return;
	}
	obj3d->_objScale = TeVector3f32(xs, ys, zs);
}

static int tolua_ExportedFunctions_SetObjectScale00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetObjectScale(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetObjectScale': %d %d %s", err.index, err.array, err.type);
}

static void SetObjectFrames(const Common::String &obj, int start, int end) {
	Game *game = g_engine->getGame();
	Object3D *obj3d = game->scene().object3D(obj);
	if (!obj3d) {
		warning("[SetObjectFrames] Object not found %s", obj.c_str());
		return;
	}
	obj3d->_startFrame = start;
	obj3d->_endFrame = end;
}

static int tolua_ExportedFunctions_SetObjectFrames00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		SetObjectFrames(s1, (int)f1, (int)f2);
		return 0;
	}
	error("#ferror in function 'SetObjectFrames': %d %d %s", err.index, err.array, err.type);
}

static bool CurrentCharacterAnimation(const Common::String &charname, const Common::String &anim) {
	Character *character = g_engine->getGame()->scene().character(charname);

	if (!character) {
		debug("[CurrentCharacterAnimation] Character\'s\"%s\" doesn't exist", charname.c_str());
		return true;
	} else {
		return anim == character->curAnimName();
	}
}

static int tolua_ExportedFunctions_CurrentCharacterAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) && tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		bool result = CurrentCharacterAnimation(s1, s2);
		tolua_pushboolean(L, result);
		return 1;
	}
	error("#ferror in function 'CurrentCharacterAnimation': %d %d %s", err.index, err.array, err.type);
}

static void LoadCharacter(const Common::String &name) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->loadCharacter(name);
}

static int tolua_ExportedFunctions_LoadCharacter00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		LoadCharacter(s1);
		return 0;
	}
	error("#ferror in function 'LoadCharacter': %d %d %s", err.index, err.array, err.type);
}

static void UnloadCharacter(const Common::String &name) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->unloadCharacter(name);
}

static int tolua_ExportedFunctions_UnloadCharacter00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		UnloadCharacter(s1);
		return 0;
	}
	error("#ferror in function 'UnloadCharacter': %d %d %s", err.index, err.array, err.type);
}

static void MoveCharacterTo(const Common::String &charName, const Common::String &curveName, float curveStart, float curveEnd) {
	Game *game = g_engine->getGame();
	game->scene().moveCharacterTo(charName, curveName, curveStart, curveEnd);
}

static int tolua_ExportedFunctions_MoveCharacterTo00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		float f1 = tolua_tonumber(L, 3, 0.0);
		float f2 = tolua_tonumber(L, 4, 0.0);
		MoveCharacterTo(s1, s2, f1, f2);
		return 0;
	}
	error("#ferror in function 'MoveCharacterTo': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_MoveCharacterToAndWaitForEnd00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		float f1 = tolua_tonumber(L, 3, 0.0);
		float f2 = tolua_tonumber(L, 4, 0.0);
		MoveCharacterTo(s1, s2, f1, f2);

		SyberiaGame::YieldedCallback callback;
		callback._luaThread = TeLuaThread::threadFromState(L);
		callback._luaFnName = "OnDisplacementFinished";
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		for (const auto &cb : game->yieldedCallbacks()) {
			if (cb._luaFnName == callback._luaFnName)
				warning("MoveCharacterToAndWaitForEnd: Reentrency error, your are already in a yielded/sync function call");
		}
		game->yieldedCallbacks().push_back(callback);
		return callback._luaThread->yield();
	}
	error("#ferror in function 'MoveCharacterToAndWaitForEnd': %d %d %s", err.index, err.array, err.type);
}

static void MoveCharacterPlayerTo(float x, float y, float z, bool walkFlag) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	if (game->_movePlayerCharacterDisabled)
		return;

	TeVector3f32 dest(x, y, z);
	game->resetPreviousMousePos();

	Character *character = game->scene()._character;
	if (dest == game->posPlayer() && character->walkModeStr() == "Walk")
		return;

	if (game->walkTimer().running() && game->walkTimer().timeElapsed() < 300000 && game->runModeEnabled()) {
		uint64 elapsed = game->walkTimer().timeElapsed();
		game->walkTimer().stop();
		if (elapsed < 300000) {
			character->walkMode("Jog");
		}
	} else {
		game->walkTimer().stop();
		game->walkTimer().start();
		character->walkMode("Walk");
	}

	assert(character->freeMoveZone());
	game->_sceneCharacterVisibleFromLoad = false;
	TeIntrusivePtr<TeBezierCurve> curve = character->freeMoveZone()->curve(character->_model->position(), dest);
	if (!curve) {
		game->luaScript().execute("OnDisplacementFinished");
	} else {
		game->scene().setCurve(curve);
		character->setCurveStartLocation(TeVector3f32(0, 0, 0));
		character->placeOnCurve(curve);
		character->setCurveOffset(0);
		character->setAnimation(character->walkAnim(Character::WalkPart_Loop), true);
		character->walkTo(1.0, walkFlag);
		game->_isCharacterWalking = true;
		game->setPosPlayer(dest);
	}
}

static int tolua_ExportedFunctions_MoveCharacterPlayerTo00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isboolean(L, 4, 1, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		float f1 = tolua_tonumber(L, 1, 0.0);
		float f2 = tolua_tonumber(L, 2, 0.0);
		float f3 = tolua_tonumber(L, 3, 0.0);
		bool b1 = tolua_toboolean(L, 4, false);
		MoveCharacterPlayerTo(f1, f2, f3, b1);
		return 0;
	}
	error("#ferror in function 'MoveCharacterPlayerTo': %d %d %s", err.index, err.array, err.type);
}

static void EnableRunMode(bool val) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	game->setRunModeEnabled(val);
}

static int tolua_ExportedFunctions_EnableRunMode00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		bool b1 = tolua_toboolean(L, 1, false);
		EnableRunMode(b1);
		return 0;
	}
	error("#ferror in function 'EnableRunMode': %d %d %s", err.index, err.array, err.type);
}

static void SetModelPlayer(const Common::String &name) {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	Character *character = game->scene()._character;

	if (!character) {
		warning("[SetModelPlayer] Character not found %s", name.c_str());
		return;
	}

	if (character->_model->name() != name) {
		game->unloadPlayerCharacter(character->_model->name());
		if (!game->loadPlayerCharacter(name)) {
			warning("[SetModelPlayer] Can't load player character");
		}
		character->_model->setVisible(true);
	}
}

static int tolua_ExportedFunctions_SetModelPlayer00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, 0));
		SetModelPlayer(s1);
		return 0;
	}
	error("#ferror in function 'SetModelPlayer': %d %d %s", err.index, err.array, err.type);
}

static void BlendCharacterPlayerAnimation(const Common::String &anim, float amount, bool repeat, bool returnToIdle) {
	Game *game = g_engine->getGame();
	Character *character = game->scene()._character;
	if (character) {
		character->blendAnimation(anim, amount, repeat, returnToIdle);
	} else {
		warning("[BlendCharacterPlayerAnimation] Character doesn't exist");
	}

}

static int tolua_ExportedFunctions_BlendCharacterPlayerAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isboolean(L, 3, 1, &err) && tolua_isboolean(L, 4, 1, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		bool b1 = tolua_toboolean(L, 3, true);
		bool b2 = tolua_toboolean(L, 4, false);
		BlendCharacterPlayerAnimation(s1, f1, b1, b2);
		return 0;
	}
	error("#ferror in function 'BlendCharacterPlayerAnimation': %d %d %s", err.index, err.array, err.type);
}

static bool CurrentCharacterPlayerAnimation(const Common::String &anim) {
	Game *game = g_engine->getGame();
	Character *character = game->scene()._character;
	if (character) {
		return character->curAnimName() == anim;
	} else {
		warning("[CurrentCharacterPlayerAnimation] Character doesn't exist");
		return false;
	}
}

static int tolua_ExportedFunctions_CurrentCharacterPlayerAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		bool result = CurrentCharacterPlayerAnimation(s1);
		tolua_pushboolean(L, result);
		return 1;
	}
	error("#ferror in function 'CurrentCharacterPlayerAnimation': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterPlayerPosition(float x, float y, float z) {
	Game *game = g_engine->getGame();
	Character *character = game->scene()._character;
	if (character) {
		SetCharacterRotation(character->_model->name(), x, y, z);
	}
}

static int tolua_ExportedFunctions_SetCharacterPlayerRotation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		float f1 = tolua_tonumber(L, 1, 0.0);
		float f2 = tolua_tonumber(L, 2, 0.0);
		float f3 = tolua_tonumber(L, 3, 0.0);
		SetCharacterPlayerPosition(f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetCharacterPlayerRotation': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterPlayerPosition(const Common::String &zone, float x, float y, float z) {
	Game *game = g_engine->getGame();
	Character *character = game->scene()._character;
	if (character) {
		SetCharacterPosition(character->_model->name(), zone, x, y, z);
	}
}

static int tolua_ExportedFunctions_SetCharacterPlayerPosition00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 2, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetCharacterPlayerPosition(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetCharacterPlayerPosition': %d %d %s", err.index, err.array, err.type);
}

static void SetCharacterPlayerAnimation(Common::String animname, bool repeat, bool returnToIdle, int startframe, int endframe) {
	Game *game = g_engine->getGame();
	Character *c = game->scene()._character;
	if (!c) {
		warning("SetCharacterPlayerAnimation: no active character");
		return;
	}

	//
	// WORKAROUND: Typo in Syberia 2 A1_RomHaut/11110/Logic11110.lua
	//
	if (animname == "Kate/ka_esc_h2d.te3d")
		animname = "Kate/ka_esc_h2d.te3da";

	bool result = c->setAnimation(animname, repeat, returnToIdle, false, startframe, endframe);
	if (!result) {
		warning("[SetCharacterPlayerAnimation] Character's animation \"%s\" doesn't exist",
			animname.c_str());
	}
}

static int tolua_ExportedFunctions_SetCharacterPlayerAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 1, &err)
		&& tolua_isboolean(L, 3, 1, &err) && tolua_isnumber(L, 4, 1, &err)
		&& tolua_isnumber(L, 5, 1, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		bool b1 = tolua_toboolean(L, 2, true);
		bool b2 = tolua_toboolean(L, 3, false);
		double f3 = tolua_tonumber(L, 4, -1.0);
		double f4 = tolua_tonumber(L, 5, 9999.0);
		SetCharacterPlayerAnimation(s1, b1, b2, (int)f3, (int)f4);
		return 0;
	}
	warning("#ferror in function 'SetCharacterPlayerAnimation': %d %d %s", err.index, err.array, err.type);
	return 0;
}

static void AddUnlockedAnim(const Common::String &name) {
	// Note: does nothing, but we needed to add it..
}

static int tolua_ExportedFunctions_AddUnlockedAnim00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, 0));
		AddUnlockedAnim(s1);
		return 0;
	}
	error("#ferror in function 'AddUnlockedAnim': %d %d %s", err.index, err.array, err.type);
}

static void SetObjectMoveDest(const Common::String &obj, float x, float y, float z) {
	Object3D *obj3d = g_engine->getGame()->scene().object3D(obj);
	if (obj3d) {
		obj3d->setObjectMoveDest(TeVector3f32(x, y, z));
	} else {
		warning("[SetObjectMoveDest] Object not found %s", obj.c_str());
	}
}

static int tolua_ExportedFunctions_SetObjectMoveDest00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 0, &err)
		&& tolua_isnoobj(L, 5, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		float f2 = tolua_tonumber(L, 3, 0.0);
		float f3 = tolua_tonumber(L, 4, 0.0);
		SetObjectMoveDest(s1, f1, f2, f3);
		return 0;
	}
	error("#ferror in function 'SetObjectMoveDest': %d %d %s", err.index, err.array, err.type);
}

static void SetObjectMoveTime(const Common::String &obj, float f) {
	Object3D *obj3d = g_engine->getGame()->scene().object3D(obj);
	if (obj3d) {
		obj3d->setObjectMoveTime(f);
	} else {
		warning("[SetObjectMoveTime] Object not found %s", obj.c_str());
	}
}

static int tolua_ExportedFunctions_SetObjectMoveTime00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		float f1 = tolua_tonumber(L, 2, 0.0);
		SetObjectMoveTime(s1, f1);
		return 0;
	}
	error("#ferror in function 'SetObjectMoveTime': %d %d %s", err.index, err.array, err.type);
}

static void ActivateMask(const Common::String &name, bool val) {
	Game *game = g_engine->getGame();
	game->scene().activateMask(name, val);
}

static int tolua_ExportedFunctions_ActivateMask00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 0, &err)
		&& tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		bool b1 = tolua_toboolean(L, 2, false);
		ActivateMask(s1, b1);
		return 0;
	}
	error("#ferror in function 'ActivateMask': %d %d %s", err.index, err.array, err.type);
}

static void SetYoukiFollowKate(bool val) {
	g_engine->getGame()->scene().youkiManager().setFollowKate(val);
}

static int tolua_ExportedFunctions_SetYoukiFollowKate00(lua_State *L) {
	tolua_Error err;
	if (tolua_isboolean(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		bool b1 = tolua_toboolean(L, 1, false);
		SetYoukiFollowKate(b1);
		return 0;
	}
	error("#ferror in function 'SetYoukiFollowKate': %d %d %s", err.index, err.array, err.type);
}

static void AddRandomAnimation(const Common::String &character, const Common::String &anim, float f) {
	// This exists in the game, but does nothing.
}

static int tolua_ExportedFunctions_AddRandomAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isstring(L, 2, 0, &err) &&
		tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		Common::String s2(tolua_tostring(L, 2, nullptr));
		double d1 = tolua_tonumber(L, 3, 0.0);
		AddRandomAnimation(s1, s2, d1);
		return 0;
	}
	error("#ferror in function 'AddRandomAnimation': %d %d %s", err.index, err.array, err.type);
}

static void PlayRandomAnimation(const Common::String &character) {
	// This exists in the game, but does nothing.
}

static int tolua_ExportedFunctions_PlayRandomAnimation00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		PlayRandomAnimation(s1);
		return 0;
	}
	error("#ferror in function 'PlayRandomAnimation': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_PlaySmoke00(lua_State *L) {
	// This exists in the game, but does nothing.
	return 0;
}

static int tolua_ExportedFunctions_SmokeVisible00(lua_State *L) {
	// This exists in the game, but does nothing.
	return 0;
}

static int tolua_ExportedFunctions_PlaySnow00(lua_State *L) {
	// This exists in the game, but does nothing.
	return 0;
}

static int tolua_ExportedFunctions_PlaySnowCustom00(lua_State *L) {
	// This exists in the game, but does nothing.
	return 0;
}

static int tolua_ExportedFunctions_SnowCustomVisible00(lua_State *L) {
	// This exists in the game, but does nothing.
	return 0;
}

static int tolua_ExportedFunctions_RemoveRandomSound00(lua_State *L) {
	// This exists in the game, but does nothing.
	return 0;
}

static void PlayVerticalScrolling(float time) {
	g_engine->getGame()->scene().playVerticalScrolling(time);
}

static int tolua_ExportedFunctions_PlayVerticalScrolling00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		float f1 = tolua_tonumber(L, 1, 0.0);
		PlayVerticalScrolling(f1);
		return 0;
	}
	error("#ferror in function 'SetObjectMoveTime': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_GetNXPadType(lua_State *L) {
	// TODO: Actually implement this
	tolua_pushstring(L, "Handheld");
	return 1;
}

// Not your imagination, the implementation of these two is quite different to the others.
static int tolua_GetParticleIndex(lua_State *L) {
	Common::String s1(tolua_tostring(L, 1, nullptr));
	int idx = TeParticle::getIndex(s1);
	tolua_pushnumber(L, idx);
	return 1;
}

static int tolua_EnableParticle(lua_State *L) {
	double d1 = tolua_tonumber(L, 1, 0.0);
	if (d1 < 0) {
		warning("EnableParticle: Invalid particle %d requested", (int)d1);
		return 0;
	}
	TeParticle *p = (TeParticle *)TeParticle::getIndexedParticle((int)d1);
	if (p) {
		double d2 = tolua_tonumber(L, 2, 1.0);
		p->setEnabled((int)d2 != 0);
	}
	return 0;
}

// ////////////////////////////////////////////////////////////////////////



static void LuaOpenBinds_Syberia(lua_State *L) {
	tolua_open(L);
	tolua_module(L, 0, 0);
	tolua_beginmodule(L, 0);
	tolua_function(L, "LoadObjectMaterials", tolua_ExportedFunctions_LoadObjectMaterials00);
	tolua_function(L, "LoadObjectMaterials", tolua_ExportedFunctions_LoadObjectMaterials01);
	tolua_function(L, "HideObject", tolua_ExportedFunctions_HideObject00);
	tolua_function(L, "ShowObject", tolua_ExportedFunctions_ShowObject00);
	// tolua_function(L, "ShowAllObjects", tolua_ExportedFunctions_ShowAllObjects00); // Unused
	tolua_function(L, "SetBackground", tolua_ExportedFunctions_SetBackground00);
	// tolua_function(L, "AddBlockingObject", tolua_ExportedFunctions_AddBlockingObject00); // Unused
	// tolua_function(L, "RemoveBlockingObject", tolua_ExportedFunctions_RemoveBlockingObject00); // Unused
	tolua_function(L, "ChangeWarp", tolua_ExportedFunctions_ChangeWarp00);
	tolua_function(L, "PlayMovie", tolua_ExportedFunctions_PlayMovie00);
	tolua_function(L, "PlayMovieAndWaitForEnd", tolua_ExportedFunctions_PlayMovieAndWaitForEnd00);
	// tolua_function(L, "StartAnimationPart", tolua_ExportedFunctions_StartAnimationPart00); // Unused
	tolua_function(L, "StartAnimation", tolua_ExportedFunctions_StartAnimation00);
	tolua_function(L, "StartAnimationAndWaitForEnd",
				 tolua_ExportedFunctions_StartAnimationAndWaitForEnd00);
	// tolua_function(L, "AddAnimToSet", tolua_ExportedFunctions_AddAnimToSet00); // Unused
	tolua_function(L, "RequestAutoSave", tolua_ExportedFunctions_RequestAutoSave00);
	tolua_function(L, "SetVisibleButtonZoomed", tolua_ExportedFunctions_SetVisibleButtonZoomed00);
	tolua_function(L, "AddMarker", tolua_ExportedFunctions_AddMarker00);
	tolua_function(L, "SetVisibleMarker", tolua_ExportedFunctions_SetVisibleMarker00);
	tolua_function(L, "DeleteMarker", tolua_ExportedFunctions_DeleteMarker00);
	tolua_function(L, "SetVisibleCellphone", tolua_ExportedFunctions_SetVisibleCellphone00);
	tolua_function(L, "DisabledZone", tolua_ExportedFunctions_DisabledZone00);
	tolua_function(L, "DisabledInt", tolua_ExportedFunctions_DisabledInt00);
	tolua_function(L, "LockCursor", tolua_ExportedFunctions_LockCursor00);
	// tolua_function(L, "SetCondition", tolua_ExportedFunctions_SetCondition00); // Unused
	// tolua_function(L, "UnsetCondition", tolua_ExportedFunctions_UnsetCondition00); // Unused
	// tolua_function(L, "TutoActive", tolua_ExportedFunctions_TutoActive00); // Unused
	tolua_function(L, "LaunchDialog", tolua_ExportedFunctions_LaunchDialog00);
	tolua_function(L, "LaunchDialogAndWaitForEnd", tolua_ExportedFunctions_LaunchDialogAndWaitForEnd00);
	tolua_function(L, "PushAnswer", tolua_ExportedFunctions_PushAnswer00);
	tolua_function(L, "HideAnswers", tolua_ExportedFunctions_HideAnswers00);
	tolua_function(L, "PushTask", tolua_ExportedFunctions_PushTask00);
	tolua_function(L, "DeleteTask", tolua_ExportedFunctions_DeleteTask00);
	tolua_function(L, "SetVisibleButtonHelp", tolua_ExportedFunctions_SetVisibleButtonHelp00);
	// tolua_function(L, "HideTasks", tolua_ExportedFunctions_HideTasks00); Not used.
	tolua_function(L, "PlaySound", tolua_ExportedFunctions_PlaySound00);
	tolua_function(L, "PlaySoundAndWaitForEnd", tolua_ExportedFunctions_PlaySoundAndWaitForEnd00);
	tolua_function(L, "StopSound", tolua_ExportedFunctions_StopSound00);
	tolua_function(L, "AddRandomSound", tolua_ExportedFunctions_AddRandomSound00);
	tolua_function(L, "PlayRandomSound", tolua_ExportedFunctions_PlayRandomSound00);
	tolua_function(L, "PlayMusic", tolua_ExportedFunctions_PlayMusic00);
	tolua_function(L, "SetSoundStep", tolua_ExportedFunctions_SetSoundStep00);
	tolua_function(L, "Selected", tolua_ExportedFunctions_Selected00);
	tolua_function(L, "TakeObject", tolua_ExportedFunctions_TakeObject00);
	tolua_function(L, "TakeObjectInHand", tolua_ExportedFunctions_TakeObjectInHand00); // Only used in Syberia 2
	tolua_function(L, "RemoveObject", tolua_ExportedFunctions_RemoveObject00);
	tolua_function(L, "RemoveObject", tolua_ExportedFunctions_RemoveObject01);
	tolua_function(L, "AddNumber", tolua_ExportedFunctions_AddNumber00);
	tolua_function(L, "ShowDocument", tolua_ExportedFunctions_ShowDocument00);
	// tolua_function(L, "ShowDocumentAndWaitForEnd", tolua_ExportedFunctions_ShowDocumentAndWaitForEnd00); // Unused
	tolua_function(L, "HideDocument", tolua_ExportedFunctions_HideDocument00); // Only used in Syberia 2
	tolua_function(L, "AddDocument", tolua_ExportedFunctions_AddDocument00); // Only used in Syberia 2
	tolua_function(L, "LoadCharacter", tolua_ExportedFunctions_LoadCharacter00);
	tolua_function(L, "UnloadCharacter", tolua_ExportedFunctions_UnloadCharacter00);
	// tolua_function(L, "GetRotationCharacter", tolua_ExportedFunctions_GetRotationCharacter00); // Unused
	// tolua_function(L, "GetXPositionCharacter", tolua_ExportedFunctions_GetXPositionCharacter00); // Unused
	// tolua_function(L, "GetYPositionCharacter", tolua_ExportedFunctions_GetYPositionCharacter00); // Unused
	// tolua_function(L, "GetZPositionCharacter", tolua_ExportedFunctions_GetZPositionCharacter00); // Unused
	tolua_function(L, "MoveCharacterTo", tolua_ExportedFunctions_MoveCharacterTo00);
	tolua_function(L, "MoveCharacterToAndWaitForEnd",
				 tolua_ExportedFunctions_MoveCharacterToAndWaitForEnd00);
	tolua_function(L, "MoveCharacterPlayerTo", tolua_ExportedFunctions_MoveCharacterPlayerTo00);
	// tolua_function(L, "MoveCharacterPlayerToAndWaitForEnd",
	//				tolua_ExportedFunctions_MoveCharacterPlayerToAndWaitForEnd00); // Unused
	// tolua_function(L, "MoveCharacterPlayerAtTo", tolua_ExportedFunctions_MoveCharacterPlayerAtTo00); // Unused
	tolua_function(L, "SetCharacterPosition", tolua_ExportedFunctions_SetCharacterPosition00);
	tolua_function(L, "PlaceCharacterOnDummy", tolua_ExportedFunctions_PlaceCharacterOnDummy00);
	tolua_function(L, "SetCharacterRotation", tolua_ExportedFunctions_SetCharacterRotation00);
	tolua_function(L, "SetCharacterOrientation", tolua_ExportedFunctions_SetCharacterOrientation00);
	tolua_function(L, "SetCharacterAnimation", tolua_ExportedFunctions_SetCharacterAnimation00);
	tolua_function(L, "SetCharacterAnimationAndWaitForEnd",
				 tolua_ExportedFunctions_SetCharacterAnimationAndWaitForEnd00);
	tolua_function(L, "BlendCharacterAnimation", tolua_ExportedFunctions_BlendCharacterAnimation00);
	tolua_function(L, "BlendCharacterAnimationAndWaitForEnd",
				 tolua_ExportedFunctions_BlendCharacterAnimationAndWaitForEnd00);
	tolua_function(L, "CurrentCharacterAnimation", tolua_ExportedFunctions_CurrentCharacterAnimation00);
	tolua_function(L, "SetCharacterPlayerVisible", tolua_ExportedFunctions_SetCharacterPlayerVisible00);
	tolua_function(L, "MoveCharacterPlayerDisabled",
				 tolua_ExportedFunctions_MoveCharacterPlayerDisabled00);
	tolua_function(L, "SetRunMode", tolua_ExportedFunctions_SetRunMode00);
	tolua_function(L, "SetRunMode2", tolua_ExportedFunctions_SetRunMode200);
	// tolua_function(L, "SetCharacterColor", tolua_ExportedFunctions_SetCharacterColor00); // Unused
	// tolua_function(L, "SetCharacterSound", tolua_ExportedFunctions_SetCharacterSound00); // Unused
	tolua_function(L, "SetCharacterShadow", tolua_ExportedFunctions_SetCharacterShadow00);
	tolua_function(L, "AddCallback", tolua_ExportedFunctions_AddCallback00);
	tolua_function(L, "AddCallbackPlayer", tolua_ExportedFunctions_AddCallbackPlayer00);
	// tolua_function(L, "AddCallbackAnimation2D", tolua_ExportedFunctions_AddCallbackAnimation2D00); // Unused
	tolua_function(L, "DeleteCallback", tolua_ExportedFunctions_DeleteCallback00); // Only used in Syberia 2
	tolua_function(L, "DeleteCallbackPlayer", tolua_ExportedFunctions_DeleteCallbackPlayer00); // Only used in Syberia 2
	// tolua_function(L, "DeleteCallbackAnimation2D", tolua_ExportedFunctions_DeleteCallbackAnimation2D00); // Unused
	tolua_function(L, "SetObjectOnCharacter", tolua_ExportedFunctions_SetObjectOnCharacter00);
	tolua_function(L, "SetObjectRotation", tolua_ExportedFunctions_SetObjectRotation00);
	tolua_function(L, "SetObjectTranslation", tolua_ExportedFunctions_SetObjectTranslation00);
	tolua_function(L, "SetObjectScale", tolua_ExportedFunctions_SetObjectScale00);
	tolua_function(L, "SetObjectFrames", tolua_ExportedFunctions_SetObjectFrames00);
	tolua_function(L, "LoadObject", tolua_ExportedFunctions_LoadObject00);
	tolua_function(L, "UnloadObject", tolua_ExportedFunctions_UnloadObject00);
	tolua_function(L, "SetGroundObjectPosition", tolua_ExportedFunctions_SetGroundObjectPosition00);
	tolua_function(L, "SetGroundObjectRotation", tolua_ExportedFunctions_SetGroundObjectRotation00);
	tolua_function(L, "TranslateGroundObject", tolua_ExportedFunctions_TranslateGroundObject00);
	tolua_function(L, "RotateGroundObject", tolua_ExportedFunctions_RotateGroundObject00); // Only used in Syberia 2
	// tolua_function(L, "SetLightPlayerCharacter", tolua_ExportedFunctions_SetLightPlayerCharacter00); // Unused
	// tolua_function(L, "SetLightPos", tolua_ExportedFunctions_SetLightPos00); // Unused
	tolua_function(L, "EnableLight", tolua_ExportedFunctions_EnableLight00);
	// tolua_function(L, "SetLightDiffuse", tolua_ExportedFunctions_SetLightDiffuse00); // Unused
	// tolua_function(L, "SetLightAmbient", tolua_ExportedFunctions_SetLightAmbient00); // Unused
	// tolua_function(L, "SetLightSpecular", tolua_ExportedFunctions_SetLightSpecular00); // Unused
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
	// tolua_function(L, "OpenFinalURL", tolua_ExportedFunctions_OpenFinalURL00); // Unused
	tolua_function(L, "FinishGame", tolua_ExportedFunctions_FinishGame00);
	tolua_function(L, "RequestMainMenu", tolua_ExportedFunctions_RequestMainMenu00);
	// tolua_function(L, "BFGRateImmediately", tolua_ExportedFunctions_BFGRateImmediately00); // Unused
	// tolua_function(L, "BFGReportEvent", tolua_ExportedFunctions_BFGReportEvent00); // Unused
	// tolua_function(L, "BFGReportEventWithValue", tolua_ExportedFunctions_BFGReportEventWithValue00); // Unused
	// tolua_function(L, "BFGReachedFreemiumLimit", tolua_ExportedFunctions_BFGReachedFreemiumLimit00); // Unused
	tolua_function(L, "TestFileFlagSystemFlag", tolua_ExportedFunctions_TestFileFlagSystemFlag00);
	// tolua_function(L, "PrintDebugMessage", tolua_ExportedFunctions_PrintDebugMessage00); // Unused
	tolua_function(L, "ExitZone", tolua_ExportedFunctions_ExitZone00);
	tolua_function(L, "EnableRectBlocker", tolua_ExportedFunctions_EnableRectBlocker00);
	tolua_function(L, "EnableBlocker", tolua_ExportedFunctions_EnableBlocker00);
	tolua_function(L, "AddAnchorZone", tolua_ExportedFunctions_AddAnchorZone00);
	tolua_function(L, "ActivateAnchorZone", tolua_ExportedFunctions_ActivateAnchorZone00);
	// tolua_function(L, "SetCharacterAnchor", tolua_ExportedFunctions_SetCharacterAnchor00); // Unused
	tolua_function(L, "SetCharacterLookChar", tolua_ExportedFunctions_SetCharacterLookChar00);
	tolua_function(L, "Random", tolua_ExportedFunctions_Random00);
	tolua_function(L, "SetCharacterMeshVisible", tolua_ExportedFunctions_SetCharacterMeshVisible00);
	tolua_function(L, "SetRecallageY", tolua_ExportedFunctions_SetRecallageY00);
	tolua_function(L, "IsFreemiumUnlocked", tolua_ExportedFunctions_IsFreemiumUnlocked00);
	tolua_function(L, "ReachedFreemiumLimit", tolua_ExportedFunctions_ReachedFreemiumLimit00);
	tolua_function(L, "IsFacebookLiked", tolua_ExportedFunctions_IsFacebookLiked00);

	tolua_function(L, "AddUnrecalAnim", tolua_ExportedFunctions_AddUnrecalAnim00);
	tolua_function(L, "UnlockArtwork", tolua_ExportedFunctions_UnlockArtwork00);

	// Syberia 2 specific functions.
	tolua_function(L, "EnableRunMode", tolua_ExportedFunctions_EnableRunMode00);
	tolua_function(L, "SetModelPlayer", tolua_ExportedFunctions_SetModelPlayer00);
	tolua_function(L, "BlendCharacterPlayerAnimation", tolua_ExportedFunctions_BlendCharacterPlayerAnimation00);
	tolua_function(L, "CurrentCharacterPlayerAnimation", tolua_ExportedFunctions_CurrentCharacterPlayerAnimation00);
	tolua_function(L, "SetCharacterPlayerRotation", tolua_ExportedFunctions_SetCharacterPlayerRotation00);
	tolua_function(L, "SetCharacterPlayerPosition", tolua_ExportedFunctions_SetCharacterPlayerPosition00);
	tolua_function(L, "SetCharacterPlayerAnimation", tolua_ExportedFunctions_SetCharacterPlayerAnimation00);
	tolua_function(L, "AddUnlockedAnim", tolua_ExportedFunctions_AddUnlockedAnim00);
	tolua_function(L, "SetObjectMoveDest", tolua_ExportedFunctions_SetObjectMoveDest00);
	tolua_function(L, "SetObjectMoveTime", tolua_ExportedFunctions_SetObjectMoveTime00);
	tolua_function(L, "ActivateMask", tolua_ExportedFunctions_ActivateMask00);
	tolua_function(L, "SetYoukiFollowKate", tolua_ExportedFunctions_SetYoukiFollowKate00);
	tolua_function(L, "AddRandomAnimation", tolua_ExportedFunctions_AddRandomAnimation00);
	tolua_function(L, "PlayRandomAnimation", tolua_ExportedFunctions_PlayRandomAnimation00);
	tolua_function(L, "PlaySmoke", tolua_ExportedFunctions_PlaySmoke00);
	tolua_function(L, "SmokeVisible", tolua_ExportedFunctions_SmokeVisible00);
	tolua_function(L, "PlaySnow", tolua_ExportedFunctions_PlaySnow00);
	tolua_function(L, "PlaySnowCustom", tolua_ExportedFunctions_PlaySnowCustom00);
	tolua_function(L, "SnowCustomVisible", tolua_ExportedFunctions_SnowCustomVisible00);
	tolua_function(L, "RemoveRandomSound", tolua_ExportedFunctions_RemoveRandomSound00);
	tolua_function(L, "PlayVerticalScrolling", tolua_ExportedFunctions_PlayVerticalScrolling00);
	tolua_function(L, "GetParticleIndex", tolua_GetParticleIndex);
	tolua_function(L, "EnableParticle", tolua_EnableParticle);
	tolua_function(L, "GetNXPadType", tolua_ExportedFunctions_GetNXPadType);

	tolua_endmodule(L);
}

// ======== Amerzone-specific bind functions below ========

static int tolua_ExportedFunctions_PrintDebugMessage00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		debug("%s", s1.c_str());
		return 0;
	}
	error("#ferror in function 'PrintDebugMessage': %d %d %s", err.index, err.array, err.type);
}

static void PutObject(const Common::String &name, bool enable) {
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	game->warpY()->putObject(name, enable);
}

static int tolua_ExportedFunctions_PutObject00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isboolean(L, 2, 1, &err)
			&& tolua_isnoobj(L, 3, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		bool b1 = tolua_toboolean(L, 2, 1);
		PutObject(s1, b1);
		return 0;
	}
	error("#ferror in function 'PutObject': %d %d %s", err.index, err.array, err.type);
}

static void StartAnimationPart(const Common::String &name, int startFrame, int endFrame, int repCount, bool flag) {
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	// Note parameter order changes
	game->warpY()->startAnimationPart(name, repCount, startFrame, endFrame, flag);
}

static int tolua_ExportedFunctions_StartAnimationPart00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnumber(L, 4, 1, &err)
		&& tolua_isboolean(L, 5, 1, &err) && tolua_isnoobj(L, 6, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		double d1 = tolua_tonumber(L, 2, 0.0);
		double d2 = tolua_tonumber(L, 3, 0.0);
		double d3 = tolua_tonumber(L, 4, -1.0);
		bool b1 = tolua_tonumber(L, 5, 0);
		StartAnimationPart(s1, (int)d1, (int)d2, (int)d3, b1);
		return 0;
	}
	error("#ferror in function 'SetAnimationPart': %d %d %s", err.index, err.array, err.type);
}

static void RemoveObject_Amerzone(const Common::String &name) {
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	game->warpY()->takeObject(name);
}

static int tolua_ExportedFunctions_RemoveObject00_Amerzone(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		RemoveObject_Amerzone(s1);
		return 0;
	}
	error("#ferror in function 'RemoveObject': %d %d %s", err.index, err.array, err.type);
}

static void AddToBag(const Common::String &name) {
	Game *game = g_engine->getGame();
	game->addToBag(name);
	TeSoundManager *sndMgr = g_engine->getSoundManager();
	sndMgr->playFreeSound("Sounds/SFX/N_prendre.ogg");
}

static int tolua_ExportedFunctions_AddToBag00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		AddToBag(s1);
		return 0;
	}
	error("#ferror in function 'AddToBag': %d %d %s", err.index, err.array, err.type);
}

void SaveGame(const Common::String &name) {
	g_engine->getGame()->saveBackup(name);
}

static int tolua_ExportedFunctions_SaveGame00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		SaveGame(s1);
		return 0;
	}
	error("#ferror in function 'SaveGame': %d %d %s", err.index, err.array, err.type);
}

static void SetMarker(const Common::String &name, int imgNo, long markerId) {
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	game->warpY()->configMarker(name, imgNo, markerId);
}

static int tolua_ExportedFunctions_SetMarker00(lua_State *L) {
	tolua_Error err;
	if (tolua_isstring(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
		&& tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		Common::String s1(tolua_tostring(L, 1, nullptr));
		double d1 = tolua_tonumber(L, 2, 0.0);
		double d2 = tolua_tonumber(L, 3, 0.0);
		SetMarker(s1, (int)d1, (long)d2);
		return 0;
	}
	error("#ferror in function 'SetMarker': %d %d %s", err.index, err.array, err.type);
}

static void LookAt(int x, int y) {
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	// Note: Reverse the angles to what the game does, because we apply them
	// using fromEuler.
	game->setAngleX(x);
	int yval = y - 360;
	if (y < 90)
		yval = y;
	game->setAngleY(yval);
}

static int tolua_ExportedFunctions_LookAt00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
			&& tolua_isnoobj(L, 3, &err)) {
		double d1 = tolua_tonumber(L, 1, 0.0);
		double d2 = tolua_tonumber(L, 2, 0.0);
		LookAt((int)d1, (int)d2);
		return 0;
	}
	error("#ferror in function 'LookAt': %d %d %s", err.index, err.array, err.type);
}

static void ShowPuzzle(int x, int y = 0, int z = 0) {
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	game->showPuzzle(x, y, z);
}

static int tolua_ExportedFunctions_ShowPuzzle00(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnoobj(L, 2, &err)) {
		double d1 = tolua_tonumber(L, 1, 0.0);
		ShowPuzzle((int)d1);
		return 0;
	}
	error("#ferror in function 'ShowPuzzle': %d %d %s", err.index, err.array, err.type);
}

static int tolua_ExportedFunctions_ShowPuzzle01(lua_State *L) {
	tolua_Error err;
	if (tolua_isnumber(L, 1, 0, &err) && tolua_isnumber(L, 2, 0, &err)
			&& tolua_isnumber(L, 3, 0, &err) && tolua_isnoobj(L, 4, &err)) {
		double d1 = tolua_tonumber(L, 1, 0.0);
		double d2 = tolua_tonumber(L, 2, 0.0);
		double d3 = tolua_tonumber(L, 3, 0.0);
		ShowPuzzle((int)d1, (int)d2, (int)d3);
		return 0;
	}
	return tolua_ExportedFunctions_ShowPuzzle00(L);
}


static void LuaOpenBinds_Amerzone(lua_State *L) {
	tolua_open(L);
	tolua_module(L, 0, 0);
	tolua_beginmodule(L, 0);

	tolua_function(L, "Selected", tolua_ExportedFunctions_Selected00);
	//tolua_function(L, "TestObjectLimit", tolua_ExportedFunctions_TestObjectLimit00); // unused
	tolua_function(L, "PrintDebugMessage", tolua_ExportedFunctions_PrintDebugMessage00);
	//tolua_function(L, "SetCondition", tolua_ExportedFunctions_SetCondition00); // unused
	//tolua_function(L, "UnsetCondition", tolua_ExportedFunctions_UnsetCondition00); // unused
	tolua_function(L, "TakeObject", tolua_ExportedFunctions_TakeObject00);
	tolua_function(L, "PutObject", tolua_ExportedFunctions_PutObject00);
	// This is not the same as RemoveObject from Syberia.
	tolua_function(L, "RemoveObject", tolua_ExportedFunctions_RemoveObject00_Amerzone);
	tolua_function(L, "StartAnimationPart", tolua_ExportedFunctions_StartAnimationPart00);
	//tolua_function(L, "StartAnimation", tolua_ExportedFunctions_StartAnimation00); // unused
	//tolua_function(L, "AnimationSetToEnd", tolua_ExportedFunctions_AnimationSetToEnd00); // unused
	//tolua_function(L, "AnimationSetToStart", tolua_ExportedFunctions_AnimationSetToStart00); // unused
	//tolua_function(L, "DrawText", tolua_ExportedFunctions_DrawText00); // unused
	tolua_function(L, "ChangeWarp", tolua_ExportedFunctions_ChangeWarp00);
	tolua_function(L, "AddToBag", tolua_ExportedFunctions_AddToBag00);
	// Note: same as RemoveObject in Syberia
	tolua_function(L, "RemoveFromBag", tolua_ExportedFunctions_RemoveObject00);
	tolua_function(L, "RemoveFromBag", tolua_ExportedFunctions_RemoveObject01);
	tolua_function(L, "SaveGame", tolua_ExportedFunctions_SaveGame00);
	tolua_function(L, "SetMarker", tolua_ExportedFunctions_SetMarker00);
	tolua_function(L, "LookAt", tolua_ExportedFunctions_LookAt00);
	//tolua_function(L, "Wait", tolua_ExportedFunctions_Wait00); // unused
	tolua_function(L, "PlaySound", tolua_ExportedFunctions_PlaySound00);
	tolua_function(L, "StopSound", tolua_ExportedFunctions_StopSound00);
	tolua_function(L, "PlayMusic", tolua_ExportedFunctions_PlayMusic00);
	tolua_function(L, "ShowPuzzle", tolua_ExportedFunctions_ShowPuzzle00);
	tolua_function(L, "ShowPuzzle", tolua_ExportedFunctions_ShowPuzzle01);
	tolua_function(L, "PlayMovie", tolua_ExportedFunctions_PlayMovie00);
	//tolua_function(L, "SetFOV", tolua_ExportedFunctions_SetFOV00); // unused
	//tolua_function(L, "LoadSprite", tolua_ExportedFunctions_LoadSprite00); // unused
	//tolua_function(L, "UnloadSprite", tolua_ExportedFunctions_UnloadSprite00); // unused
	//tolua_function(L, "PushAnswer", tolua_ExportedFunctions_PushAnswer00); // unused
	tolua_function(L, "FinishGame", tolua_ExportedFunctions_FinishGame00);
	tolua_function(L, "ShowDocument", tolua_ExportedFunctions_ShowDocument00);
	tolua_function(L, "HideDocument", tolua_ExportedFunctions_HideDocument00);
	tolua_function(L, "AddDocument", tolua_ExportedFunctions_AddDocument00);
	//tolua_function(L, "ClearImportantDocuments", tolua_ExportedFunctions_ClearImportantDocuments00); // unused
	//tolua_function(L, "SetImportantDocument", tolua_ExportedFunctions_SetImportantDocument00); // unused
	tolua_function(L, "TestFileFlagSystemFlag", tolua_ExportedFunctions_TestFileFlagSystemFlag00);
	//tolua_function(L, "SetViewAngleXLimits", tolua_ExportedFunctions_SetViewAngleXLimits00); // unused
	//tolua_function(L, "SetViewAngleYLimits", tolua_ExportedFunctions_SetViewAngleYLimits00); // unused

	tolua_endmodule(L);
}

void LuaOpenBinds(lua_State *L) {
	if (g_engine->gameIsAmerzone())
		LuaOpenBinds_Amerzone(L);
	else
		LuaOpenBinds_Syberia(L);
}


}

} // end namespace Tetraedge
