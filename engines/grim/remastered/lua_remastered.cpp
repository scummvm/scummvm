/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
#include "common/savefile.h"
#include "common/config-manager.h"

#include "engines/grim/remastered/lua_remastered.h"
#include "engines/grim/remastered/overlay.h"
#include "engines/grim/remastered/hotspot.h"

#include "engines/grim/grim.h"
#include "engines/grim/font.h"
#include "engines/grim/resource.h"
#include "engines/grim/registry.h"

#include "engines/grim/localize.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/luadebug.h"

namespace Grim {


void Lua_Remastered::WidescreenCorrectionFactor() {
	warning("Stub function: WidescreenCorrectionFactor, returns 1");
	lua_pushnumber(1);
}

void Lua_Remastered::GetFontDimensions() {
	// Taken from Lua_v2 and modified
	lua_Object fontObj = lua_getparam(1);
	if (!lua_isuserdata(fontObj) || lua_tag(fontObj) != Font::getStaticTag())
		return;

	Font *font = Font::getPool().getObject(lua_getuserdata(fontObj));

	if (font) {
		int32 h = font->getKernedHeight();
		int32 w = font->getCharKernedWidth('w');
		lua_pushnumber(w);
		lua_pushnumber(h);
	} else {
		warning("Lua_Remastered::GetFontDimensions for invalid font: returns 0,0");
		lua_pushnumber(0.f);
		lua_pushnumber(0.f);
	}
}

void Lua_Remastered::GetTextObjectDimensions() {
	lua_Object textObj = lua_getparam(1);
	lua_Object coordsObj = lua_getparam(2);
	if (lua_isnumber(coordsObj)) {
		int val = lua_getnumber(coordsObj);
		warning("Stub function: GetTextObjectDimensions(%d)", val);
	}

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		lua_pushnumber(textObject->getWidth()); // REMASTERED HACK
		lua_pushnumber(textObject->getBitmapHeight()); // REMASTERED HACK
		// If the line is rjustified getX does us no good
		lua_pushnumber(textObject->getLineX(0));
		lua_pushnumber(textObject->getY());
	}
}

void Lua_Remastered::OverlayDimensions() {
	lua_Object overlayObj = lua_getparam(1);
	if (!lua_isuserdata(overlayObj) || lua_tag(overlayObj) != Overlay::getStaticTag())
		return;

	Overlay *overlay = Overlay::getPool().getObject(lua_getuserdata(overlayObj));
	lua_pushnumber(overlay->getWidth());
	lua_pushnumber(overlay->getHeight());
}

void Lua_Remastered::OverlayGetScreenSize() {
	warning("Stub function: OverlayGetScreenSize, returns 1920, 1080");
	lua_pushnumber(1920);
	lua_pushnumber(1080);
}

void Lua_Remastered::OverlayCreate() {
	warning("Stub function: OverlayCreate not done");
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	lua_Object param3 = lua_getparam(3);
	lua_Object param4 = lua_getparam(4);
	if (!lua_isstring(param1) || !lua_isnumber(param2) || !lua_isnumber(param3) || !lua_istable(param4)) {
		return;
	}
	const char *overlayName = lua_getstring(param1);
	float x = lua_getnumber(param2);
	float y = lua_getnumber(param3);

	lua_pushobject(param4);
	lua_pushstring("layer");
	lua_Object table = lua_gettable();
	float layer = lua_getnumber(table);

	Overlay *overlay = g_resourceloader->loadOverlay(overlayName);

	if (overlay) {
		overlay->setPos(x, y);
		overlay->setLayer(layer);
		lua_pushusertag(overlay->getId(), overlay->getTag());
	} else {
		lua_pushnil();
	}
}

void Lua_Remastered::OverlayDestroy() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != Overlay::getStaticTag())
		return;

	Overlay *overlay = Overlay::getPool().getObject(lua_getuserdata(actorObj));
	delete overlay;
}

void Lua_Remastered::OverlayFade() {
	lua_Object overlayObj = lua_getparam(1);
	lua_Object fadeObj = lua_getparam(2);
	if (!lua_isuserdata(overlayObj) || lua_tag(overlayObj) != Overlay::getStaticTag())
		return;
	assert(lua_isnumber(fadeObj));

	int fadeType = lua_getnumber(fadeObj);
	warning("Stub function: OverlayFade(%d)", fadeType);
	Overlay *overlay = Overlay::getPool().getObject(lua_getuserdata(overlayObj));

	if (fadeType == 3) {
		delete overlay;
	}
}

void Lua_Remastered::OverlayMove() {
	lua_Object overlayObj = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	lua_Object param3 = lua_getparam(3);

	if (!lua_isuserdata(overlayObj) || lua_tag(overlayObj) != Overlay::getStaticTag())
		return;

	Overlay *overlay = Overlay::getPool().getObject(lua_getuserdata(overlayObj));
	float x = lua_getnumber(param2);
	float y = lua_getnumber(param3);
	overlay->setPos(x, y);
}

void Lua_Remastered::QueryActiveHotspots() {
	lua_Object param = lua_getparam(1);

	assert(lua_isnumber(param));

	warning("Stub function: QueryActiveHotspots(%f)", lua_getnumber(param));

	// TODO: Handle coord scaling better
	float scaleX = 1920.0f/1600;
	float scaleY = 1080.0f/900;
	Math::Vector2d pos(g_grim->_cursorX*scaleX, g_grim->_cursorY*scaleY);
	lua_Object result = lua_createtable();
	int count = 0;
	foreach (Hotspot *h, Hotspot::getPool()) {
		if (!h->_rect.containsPoint(pos)) {
			continue;
		}
		lua_Object inner = lua_createtable();

		lua_pushobject(inner);
		lua_pushstring("type");
		lua_pushstring("normal");
		lua_settable();

		lua_pushobject(inner);
		lua_pushstring("cursor");
		lua_pushnumber(0);
		lua_settable();

		lua_pushobject(inner);
		lua_pushstring("id");
		lua_pushstring(h->_name.c_str());
		lua_settable();

		lua_pushobject(inner);
		lua_pushstring("obj");
		lua_pushusertag(h->getId(), h->getTag());
		lua_settable();

		lua_pushobject(result);
		lua_pushnumber(count++);
		lua_pushobject(inner);
		lua_settable();
	}


	lua_pushobject(result);
}

void Lua_Remastered::GetLanguage() {
	warning("Stub function: GetLanguage, just guesswork");
	lua_pushnumber(g_grim->getLanguage());
}

void Lua_Remastered::SaveRegistryToDisk() {
	warning("Guesswork: SaveRegistryToDisk");
	g_registry->save();
	ConfMan.flushToDisk(); // Since we can't consistently exit yet, we force a write for now
}

void Lua_Remastered::GetCursorPosition() {
	lua_pushnumber(g_grim->_cursorX);
	lua_pushnumber(g_grim->_cursorX);
}

void Lua_Remastered::SetCursor() {
	lua_Object param1 = lua_getparam(1);

	assert(lua_isnumber(param1));

	warning("Stub function: SetCursor(%f)", lua_getnumber(param1));
}

void Lua_Remastered::GetPlatform() {
	warning("Stub function: GetPlatform, returns 1 (windows)");
	lua_pushnumber(1);
}

void Lua_Remastered::GetRemappedKeyName() {
	lua_Object param1 = lua_getparam(1);

	const char *str = "";
	if (lua_isstring(param1)) {
		str = lua_getstring(param1);
	}
	warning("Stub function: GetRemappedKeyName(%s), returns TODO", str);

	lua_pushstring("TODO");
}

void Lua_Remastered::GetRemappedKeyHint() {
	lua_Object param1 = lua_getparam(1);
	warning("Stub function: GetRemappedKeyHint(%s), returns key_empty.png", lua_getstring(param1));
	lua_pushstring("key_empty.png");
}

void Lua_Remastered::ImSetCommentaryVol() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: ImSetCommentaryVol(%f)", lua_getnumber(param1));
}

void Lua_Remastered::ImGetCommentaryVol() {
	warning("Stub function: ImGetCommentaryVol() returns 0");
	lua_pushnumber(0);
}

void Lua_Remastered::SetLanguage() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: SetLanguage(%f)", lua_getnumber(param1));
	g_grim->setLanguage(lua_getnumber(param1));
	delete g_localizer;
	g_localizer = new Localizer();
}

void Lua_Remastered::SetMouseSpeedScale() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: SetMouseSpeedScale(%f)", lua_getnumber(param1));
}

// Commentary
void Lua_Remastered::EnableCommentary() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: EnableCommentary(%f)", lua_getnumber(param1));
}

void Lua_Remastered::SetCommentary() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isstring(param1));
	warning("Stub function: SetCommentary(%s)", lua_getstring(param1));
	g_grim->getCommentary()->setCurrentCommentary(lua_getstring(param1));
}

void Lua_Remastered::PlayCurrentCommentary() {
	warning("Stub function: PlayCurrentCommentary");
	g_grim->getCommentary()->playCurrentCommentary();
}

void Lua_Remastered::HasHeardCommentary() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isstring(param1));
	bool hasHeard = g_grim->getCommentary()->hasHeardCommentary(lua_getstring(param1));
	warning("Remastered function: HasHeardCommentary(%s) returns %d", lua_getstring(param1), hasHeard);
	if (hasHeard) {
		lua_pushnumber(1);
	} else {
		lua_pushnil();
	}
}

void Lua_Remastered::SetResolutionScaling() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: SetResolutionScaling(%f)", lua_getnumber(param1));
}

void Lua_Remastered::IsConceptUnlocked() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Remastered function: IsConceptUnlocked(%f) returns %d", lua_getnumber(param1), g_grim->isConceptEnabled(lua_getnumber(param1)));
	if (g_grim->isConceptEnabled(lua_getnumber(param1))) {
		lua_pushnumber(1);
	} else {
		lua_pushnil();
	}
}

void Lua_Remastered::UnlockConcept() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Remastered function: UnlockConcept(%f)", lua_getnumber(param1));
	g_grim->enableConcept(lua_getnumber(param1));
}

void Lua_Remastered::UnlockCutscene() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Remastered function: UnlockCutscene(%f)", lua_getnumber(param1));
	g_grim->enableCutscene(lua_getnumber(param1));
}

void Lua_Remastered::IsCutsceneUnlocked() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Remastered function: IsCutsceneUnlocked(%f) returns %d", lua_getnumber(param1), g_grim->isCutsceneEnabled(lua_getnumber(param1)));
	if (g_grim->isCutsceneEnabled(lua_getnumber(param1))) {
		lua_pushnumber(1);
	} else {
		lua_pushnil();
	}
}

void Lua_Remastered::GetGameRenderMode() {
	warning("Stub function: GetGameRenderMode() - not all modes possible yet");
	lua_pushnumber(g_grim->getMode());
}

void Lua_Remastered::SetGameRenderMode() {
	lua_Object param1 = lua_getparam(1);
	if (lua_isnil(param1)) {
		warning("SetGameRenderMode(nil) - Should it be possible to call this with nil?");
		return;
	}
	assert(lua_isnumber(param1));
	warning("Stub function: SetGameRenderMode(%f)", lua_getnumber(param1));
	g_grim->setMode((GrimEngine::EngineMode)(int)lua_getnumber(param1));
}

void Lua_Remastered::AddHotspot() {
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	lua_Object param3 = lua_getparam(3);
	lua_Object param4 = lua_getparam(4);
	lua_Object param5 = lua_getparam(5);
	lua_Object param6 = lua_getparam(6);
	lua_Object param7 = lua_getparam(7);
	lua_Object param8 = lua_getparam(8);
	lua_Object param9 = lua_getparam(9);
	lua_Object param10 = lua_getparam(10);
	lua_Object param11 = lua_getparam(11);

	assert(lua_isstring(param1));
	assert(lua_isnumber(param2));
	assert(lua_isnumber(param3));
	assert(lua_isnumber(param4));
	assert(lua_isnumber(param5));
	assert(lua_isnumber(param6));
	assert(lua_isnumber(param7));
//	assert(lua_isnumber(param8));
	assert(lua_isnumber(param11));

	const char *p9str = "nil";
	const char *p10str = "nil";
	if (lua_isstring(param9)) {
		p9str = lua_getstring(param9);
	} else if (!lua_isnil(param9)) {
		assert(lua_isnil(param9));
	}

	if (lua_isstring(param10)) {
		p10str = lua_getstring(param10);
	} else if (!lua_isnil(param10)) {
		assert(lua_isnil(param10));
	}
	warning("Stub function: AddHotspot(%s, %f, %f, %f, %f, %f, %f, %f, %s, %s, %f)", lua_getstring(param1), lua_getnumber(param2), lua_getnumber(param3), lua_getnumber(param4), lua_getnumber(param5), lua_getnumber(param6), lua_getnumber(param7), lua_getnumber(param8), p9str, p10str, lua_getnumber(param11));  

	Hotspot *hotspot = new Hotspot(lua_getstring(param1), lua_getnumber(param2), lua_getnumber(param3), lua_getnumber(param4), lua_getnumber(param5));

	lua_pushusertag(hotspot->getId(), hotspot->getTag());

}

void Lua_Remastered::RemoveHotspot() {
	lua_Object hotspotObj = lua_getparam(1);
	Hotspot *hotspot = Hotspot::getPool().getObject(lua_getuserdata(hotspotObj));
	delete hotspot;
}

void Lua_Remastered::GlobalSaveResolved() {
	warning("Stub function: GlobalSaveResolved() returns 1");
	lua_pushnumber(1);
}

void Lua_Remastered::FindSaveGames() {
	warning("Stub function: FindSaveGames()");
	
	Common::SaveFileManager *saveMan = g_grim->getSaveFileManager();
	Common::StringArray saveFiles = saveMan->listSavefiles("grim_r???.sav");

	if (saveFiles.empty()) {
		lua_pushnil();
		return;
	}

	lua_Object result = lua_createtable();
	
	Common::StringArray::iterator it = saveFiles.begin();
	for (int i = 0; it != saveFiles.end(); ++it) {
		const char *filename  = (*it).c_str();
		warning("Savefile: %s", filename);
		SaveGame *savedState = SaveGame::openForLoading(filename);
		
		if (!savedState || !savedState->isCompatible()) {
			if (!savedState) {
				error("Savegame %s is invalid", filename);
			} else {
				error("Savegame %s is incompatible with this ScummVM build. Save version: %d.%d; current version: %d.%d",
					filename, savedState->saveMajorVersion(), savedState->saveMinorVersion(),
					SaveGame::SAVEGAME_MAJOR_VERSION, SaveGame::SAVEGAME_MINOR_VERSION);
			}
			delete savedState;
			return;
		}
		int slot = atoi((*it).c_str() + 6);
		Common::String str1;
		Common::String str2;
		int x;
		/*int32 dataSize = */savedState->beginSection('META');
		char str[200] = {};
		int32 strSize = 0;
		
		strSize = savedState->readLESint32();
		savedState->read(str, strSize);
		str1 = str;
		x = savedState->readLESint32();
		strSize = savedState->readLESint32();
		savedState->read(str, strSize);
		str2 = str;
		savedState->endSection();
		delete savedState;
		
		lua_pushobject(result);
		lua_pushnumber(i++);

		str2 = g_localizer->localize(str2.c_str());

		lua_Object keyVal = lua_createtable();
		// The key-value-mapping:
		{
			
			lua_pushobject(keyVal);
			lua_pushstring("slot");
			lua_pushnumber(slot);
			lua_settable();

			lua_pushobject(keyVal);
			lua_pushstring("title");
			lua_pushstring(str2.c_str());
			lua_settable();
			
			lua_pushobject(keyVal);
			lua_pushstring("timeDateString");
			lua_pushstring("Unknown");
			lua_settable();
			
			lua_pushobject(keyVal);
			lua_pushstring("mural_info");
			lua_pushstring(str1.c_str());
			lua_settable();
			
			lua_pushobject(keyVal);
			lua_pushstring("setIndex");
			lua_pushnumber(x);
			lua_settable();
		}
		lua_pushobject(keyVal);

		lua_settable();
	}

	lua_pushobject(result);
}


void Lua_Remastered::Load() {
	lua_Object fileName = lua_getparam(1);
//	lua_Object param2 = lua_getparam(2);
	
	if (lua_isnil(fileName)) {
		g_grim->loadGame("");
	} else if (lua_isnumber(fileName)) {
		int slot = lua_getnumber(fileName);
		Common::String saveGameFilename = Common::String::format("grim_r%03d.sav", slot);
		g_grim->loadGame(saveGameFilename.c_str());
	} else if (lua_isstring(fileName)) { // Check for number before this
		g_grim->loadGame(lua_getstring(fileName));
	} else {
		warning("Load() fileName is wrong");
		return;
	}
}

void Lua_Remastered::Save() {
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	lua_Object param3 = lua_getparam(3);
	lua_Object param4 = lua_getparam(4);
	assert(lua_isnumber(param1));
	assert(lua_isstring(param2));
	assert(lua_isnumber(param3));
	assert(lua_isstring(param4));
	
	int slot = lua_getnumber(param1);
	const char *p2Str = lua_getstring(param2);
	int p3Num = lua_getnumber(param3);
	const char *p4Str = lua_getstring(param4);
	
	warning("REMASTERED save: %d, %s, %d, %s", slot, p2Str, p3Num, p4Str);
	Common::String saveGameFilename = Common::String::format("grim_r%03d.sav", slot);
	g_grim->setSaveMetaData(p2Str, p3Num, p4Str);
	g_grim->saveGame(saveGameFilename.c_str());
}

void Lua_Remastered::ShowCursor() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: ShowCursor(%f)", lua_getnumber(param1));
}

void Lua_Remastered::ReadRegistryIntValue() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isstring(param1));
	warning("Stub function: ReadRegistryIntValue(%s) returns nil", lua_getstring(param1));
	lua_pushnil();
}

// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
    warning("Stub function: %s", funcName);
}
/*
static void stubError(const char *funcName) {
	error("Stub function: %s", funcName);
}*/

#define STUB_FUNC(name) void name() { stubWarning(#name); }
#define STUB_FUNC2(name) void name() { stubError(#name); }

STUB_FUNC(Lua_Remastered::PreloadCursors)
STUB_FUNC(Lua_Remastered::GetFindSaveGameStatus)
STUB_FUNC(Lua_Remastered::InitiateFindSaveGames)
STUB_FUNC(Lua_Remastered::AreAchievementsInstalled)
STUB_FUNC(Lua_Remastered::UnlockAchievement)
STUB_FUNC(Lua_Remastered::SetAdvancedLighting)
STUB_FUNC(Lua_Remastered::IsPlayingCommentary)
STUB_FUNC(Lua_Remastered::ClearCommentary)
STUB_FUNC(Lua_Remastered::LoadRemappedKeys)
STUB_FUNC(Lua_Remastered::StopCommentaryImmediately)
STUB_FUNC(Lua_Remastered::DestroyAllUIButtonsImmediately)
STUB_FUNC(Lua_Remastered::UpdateUIButtons)
STUB_FUNC(Lua_Remastered::OverlayClearCache)
STUB_FUNC(Lua_Remastered::LinkHotspot)
STUB_FUNC(Lua_Remastered::UpdateHotspot)
STUB_FUNC(Lua_Remastered::HideMouseCursor)
STUB_FUNC(Lua_Remastered::UpdateMouseCursor)
STUB_FUNC(Lua_Remastered::SetActorHKHackMode)
STUB_FUNC(Lua_Remastered::CacheCurrentWalkVector)
STUB_FUNC(Lua_Remastered::SetKeyMappingMode)
STUB_FUNC(Lua_Remastered::ResetKeyMappingToDefault)
STUB_FUNC(Lua_Remastered::SaveRemappedKeys)
STUB_FUNC(Lua_Remastered::New)
STUB_FUNC(Lua_Remastered::RemoveBorders)
STUB_FUNC(Lua_Remastered::GetSaveStatus)
STUB_FUNC(Lua_Remastered::StartCheckOfCrossSaveStatus)
STUB_FUNC(Lua_Remastered::GetCrossSaveStatus)
STUB_FUNC(Lua_Remastered::GetFloorWalkPos)
STUB_FUNC(Lua_Remastered::CursorMovieEnabled)


struct luaL_reg remasteredMainOpcodes[] = {
	{ "GetPlatform", LUA_OPCODE(Lua_Remastered, GetPlatform) },
	{ "GetLanguage", LUA_OPCODE(Lua_Remastered, GetLanguage) },
	{ "PreloadCursors", LUA_OPCODE(Lua_Remastered, PreloadCursors) },
	{ "AreAchievementsInstalled", LUA_OPCODE(Lua_Remastered, AreAchievementsInstalled) },
	{ "UnlockAchievement", LUA_OPCODE(Lua_Remastered, UnlockAchievement) },
	{ "ImGetCommentaryVol", LUA_OPCODE(Lua_Remastered, ImGetCommentaryVol) },
	{ "ImSetCommentaryVol", LUA_OPCODE(Lua_Remastered, ImSetCommentaryVol) },
	{ "SetMouseSpeedScale", LUA_OPCODE(Lua_Remastered, SetMouseSpeedScale) },
	{ "SetResolutionScaling", LUA_OPCODE(Lua_Remastered, SetResolutionScaling) },
	{ "SetAdvancedLighting", LUA_OPCODE(Lua_Remastered, SetAdvancedLighting) },
	{ "SetLanguage", LUA_OPCODE(Lua_Remastered, SetLanguage) },
	{ "PlayCurrentCommentary", LUA_OPCODE(Lua_Remastered, PlayCurrentCommentary) },
	{ "IsPlayingCommentary", LUA_OPCODE(Lua_Remastered, IsPlayingCommentary) },
	{ "EnableCommentary", LUA_OPCODE(Lua_Remastered, EnableCommentary) },
	{ "ClearCommentary", LUA_OPCODE(Lua_Remastered, ClearCommentary) },
	{ "HasHeardCommentary", LUA_OPCODE(Lua_Remastered, HasHeardCommentary) },
	{ "SetCommentary", LUA_OPCODE(Lua_Remastered, SetCommentary) },
	{ "LoadRemappedKeys", LUA_OPCODE(Lua_Remastered, LoadRemappedKeys) },
	{ "GlobalSaveResolved", LUA_OPCODE(Lua_Remastered, GlobalSaveResolved) },
	{ "StopCommentaryImmediately", LUA_OPCODE(Lua_Remastered, StopCommentaryImmediately) },
	{ "ReadRegistryIntValue", LUA_OPCODE(Lua_Remastered, ReadRegistryIntValue) },
	{ "DestroyAllUIButtonsImmediately", LUA_OPCODE(Lua_Remastered, DestroyAllUIButtonsImmediately) },
	{ "UpdateUIButtons", LUA_OPCODE(Lua_Remastered, UpdateUIButtons) },
	{ "GetGameRenderMode", LUA_OPCODE(Lua_Remastered, GetGameRenderMode) },
	{ "SetGameRenderMode", LUA_OPCODE(Lua_Remastered, SetGameRenderMode) },
	{ "WidescreenCorrectionFactor", LUA_OPCODE(Lua_Remastered, WidescreenCorrectionFactor) },
	{ "OverlayCreate", LUA_OPCODE(Lua_Remastered, OverlayCreate) },
	{ "OverlayClearCache", LUA_OPCODE(Lua_Remastered, OverlayClearCache) },
	{ "OverlayDimensions", LUA_OPCODE(Lua_Remastered, OverlayDimensions) },
	{ "OverlayDestroy", LUA_OPCODE(Lua_Remastered, OverlayDestroy) },
	{ "OverlayFade", LUA_OPCODE(Lua_Remastered, OverlayFade) },
	{ "OverlayGetScreenSize", LUA_OPCODE(Lua_Remastered, OverlayGetScreenSize) },
	{ "OverlayMove", LUA_OPCODE(Lua_Remastered, OverlayMove) },
	{ "AddHotspot", LUA_OPCODE(Lua_Remastered, AddHotspot) },
	{ "LinkHotspot", LUA_OPCODE(Lua_Remastered, LinkHotspot) },
	{ "RemoveHotspot", LUA_OPCODE(Lua_Remastered, RemoveHotspot) },
	{ "UpdateHotspot", LUA_OPCODE(Lua_Remastered, UpdateHotspot) },
	{ "QueryActiveHotspots", LUA_OPCODE(Lua_Remastered, QueryActiveHotspots) },
	{ "HideMouseCursor", LUA_OPCODE(Lua_Remastered, HideMouseCursor) },
	{ "ShowCursor", LUA_OPCODE(Lua_Remastered, ShowCursor) },
	{ "UpdateMouseCursor", LUA_OPCODE(Lua_Remastered, UpdateMouseCursor) },
	{ "GetCursorPosition", LUA_OPCODE(Lua_Remastered, GetCursorPosition) },
	{ "SetCursor", LUA_OPCODE(Lua_Remastered, SetCursor) },
	{ "UnlockCutscene", LUA_OPCODE(Lua_Remastered, UnlockCutscene) },
	{ "IsCutsceneUnlocked", LUA_OPCODE(Lua_Remastered, IsCutsceneUnlocked) },
	{ "SetActorHKHackMode", LUA_OPCODE(Lua_Remastered, SetActorHKHackMode) },
	{ "CacheCurrentWalkVector", LUA_OPCODE(Lua_Remastered, CacheCurrentWalkVector) },
	{ "UnlockConcept", LUA_OPCODE(Lua_Remastered, UnlockConcept) },
	{ "IsConceptUnlocked", LUA_OPCODE(Lua_Remastered, IsConceptUnlocked) },
	{ "GetRemappedKeyHint", LUA_OPCODE(Lua_Remastered, GetRemappedKeyHint) },
	{ "SetKeyMappingMode", LUA_OPCODE(Lua_Remastered, SetKeyMappingMode) },
	{ "ResetKeyMappingToDefault", LUA_OPCODE(Lua_Remastered, ResetKeyMappingToDefault) },
	{ "SaveRemappedKeys", LUA_OPCODE(Lua_Remastered, SaveRemappedKeys) },
	{ "SaveRegistryToDisk", LUA_OPCODE(Lua_Remastered, SaveRegistryToDisk) },	
	{ "InitiateFindSaveGames", LUA_OPCODE(Lua_Remastered, InitiateFindSaveGames) },
	{ "GetFindSaveGameStatus", LUA_OPCODE(Lua_Remastered, GetFindSaveGameStatus) },
	{ "FindSaveGames", LUA_OPCODE(Lua_Remastered, FindSaveGames) },
	{ "GetRemappedKeyName", LUA_OPCODE(Lua_Remastered, GetRemappedKeyName) },
	{ "New", LUA_OPCODE(Lua_Remastered, New) },
	{ "RemoveBorders", LUA_OPCODE(Lua_Remastered, RemoveBorders) },
	{ "GetSaveStatus", LUA_OPCODE(Lua_Remastered, GetSaveStatus) },
	{ "StartCheckOfCrossSaveStatus", LUA_OPCODE(Lua_Remastered, StartCheckOfCrossSaveStatus) },
	{ "GetCrossSaveStatus", LUA_OPCODE(Lua_Remastered, GetCrossSaveStatus) },
	{ "GetFloorWalkPos", LUA_OPCODE(Lua_Remastered, GetFloorWalkPos) },
	{ "CursorMovieEnabled", LUA_OPCODE(Lua_Remastered, CursorMovieEnabled) },
};

void Lua_Remastered::registerOpcodes() {
	Lua_V1::registerOpcodes();

	// Register main opcodes functions
	luaL_openlib(remasteredMainOpcodes, ARRAYSIZE(remasteredMainOpcodes));
}

} // end of namespace Grim
