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
#include "common/savefile.h"
#include "common/config-manager.h"

#include "engines/grim/remastered/lua_remastered.h"
#include "engines/grim/remastered/overlay.h"

#include "engines/grim/grim.h"
#include "engines/grim/font.h"
#include "engines/grim/resource.h"
#include "engines/grim/registry.h"

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


void Lua_Remastered::OverlayDimensions() {
	lua_Object overlayObj = lua_getparam(1);
	if (!lua_isuserdata(overlayObj) || lua_tag(overlayObj) != Overlay::getStaticTag())
		return;

	Overlay *overlay = Overlay::getPool().getObject(lua_getuserdata(overlayObj));
	lua_pushnumber(overlay->getWidth());
	lua_pushnumber(overlay->getHeight());
}

void Lua_Remastered::OverlayGetScreenSize() {
	warning("Stub function: OverlayGetScreenSize, returns 1, 1");
	lua_pushnumber(1);
	lua_pushnumber(1);
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
	warning("Stub function: QueryActiveHotspots, returns empty table");
	lua_Object resObj = lua_createtable();
	lua_pushobject(resObj);
}

void Lua_Remastered::GetLanguage() {
	warning("Stub function: GetLanguage, returns 0");
	lua_pushnumber(0);
}

void Lua_Remastered::SaveRegistryToDisk() {
	warning("Guesswork: SaveRegistryToDisk");
	g_registry->save();
	ConfMan.flushToDisk(); // Since we can't consistently exit yet, we force a write for now
}

void Lua_Remastered::GetCursorPosition() {
	warning("Stub function: GetCursorPosition, returns 0, 0");
	lua_pushnumber(0);
	lua_pushnumber(0);
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
}

void Lua_Remastered::SetMouseSpeedScale() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: SetMouseSpeedScale(%f)", lua_getnumber(param1));
}

void Lua_Remastered::EnableCommentary() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: EnableCommentary(%f)", lua_getnumber(param1));
}

void Lua_Remastered::SetCommentary() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isstring(param1));
	warning("Stub function: SetCommentary(%s)", lua_getstring(param1));
}

void Lua_Remastered::HasHeardCommentary() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isstring(param1));
	warning("Stub function: HasHeardCommentary(%s) returns 1", lua_getstring(param1));
	lua_pushnumber(1);
}

void Lua_Remastered::SetResolutionScaling() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: SetResolutionScaling(%f)", lua_getnumber(param1));
}

void Lua_Remastered::IsConceptUnlocked() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: IsConceptUnlocked(%f)", lua_getnumber(param1));
}

void Lua_Remastered::UnlockConcept() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: UnlockConcept(%f)", lua_getnumber(param1));
}

void Lua_Remastered::UnlockCutscene() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: UnlockCutscene(%f)", lua_getnumber(param1));
}

void Lua_Remastered::IsCutsceneUnlocked() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: IsCutsceneUnlocked(%f) -> returns 1", lua_getnumber(param1));
	lua_pushnumber(1);
}

void Lua_Remastered::GetGameRenderMode() {
	warning("Stub function: GetGameRenderMode() - not all modes possible yet");
	lua_pushnumber(g_grim->getMode());
}

void Lua_Remastered::SetGameRenderMode() {
	lua_Object param1 = lua_getparam(1);
	assert(lua_isnumber(param1));
	warning("Stub function: SetGameRenderMode(%f)", lua_getnumber(param1));
	g_grim->setMode((GrimEngine::EngineMode)lua_getnumber(param1));
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
	assert(lua_isnumber(param8));
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
	warning("Stub function: AddHotspot(%s, %f, %f, %f, %f, %f, %f, %f, %s, %s, %f)", lua_getstring(param1), lua_getnumber(param2), lua_getnumber(param3), lua_getnumber(param4), lua_getnumber(param5), lua_getnumber(param6), lua_getnumber(param6), lua_getnumber(param7), lua_getnumber(param8), p9str, p10str, lua_getnumber(param11));  

	lua_pushnil();
}

void Lua_Remastered::GlobalSaveResolved() {
	warning("Stub function: GlobalSaveResolved() returns 1");
	lua_pushnumber(1);
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

static void stubError(const char *funcName) {
	error("Stub function: %s", funcName);
}

#define STUB_FUNC(name) void name() { stubWarning(#name); }
#define STUB_FUNC2(name) void name() { stubError(#name); }

STUB_FUNC(Lua_Remastered::PreloadCursors)
STUB_FUNC(Lua_Remastered::GetFindSaveGameStatus)
STUB_FUNC(Lua_Remastered::FindSaveGames)
STUB_FUNC(Lua_Remastered::InitiateFindSaveGames)
STUB_FUNC(Lua_Remastered::AreAchievementsInstalled)
STUB_FUNC(Lua_Remastered::UnlockAchievement)
STUB_FUNC(Lua_Remastered::SetAdvancedLighting)
STUB_FUNC(Lua_Remastered::PlayCurrentCommentary)
STUB_FUNC(Lua_Remastered::IsPlayingCommentary)
STUB_FUNC(Lua_Remastered::ClearCommentary)
STUB_FUNC(Lua_Remastered::LoadRemappedKeys)
STUB_FUNC(Lua_Remastered::StopCommentaryImmediately)
STUB_FUNC(Lua_Remastered::DestroyAllUIButtonsImmediately)
STUB_FUNC(Lua_Remastered::UpdateUIButtons)
STUB_FUNC(Lua_Remastered::OverlayClearCache)
STUB_FUNC(Lua_Remastered::RemoveHotspot)
STUB_FUNC(Lua_Remastered::OverlayFade)
STUB_FUNC(Lua_Remastered::HideMouseCursor)
STUB_FUNC(Lua_Remastered::SetCursor)
STUB_FUNC(Lua_Remastered::UpdateMouseCursor)
STUB_FUNC(Lua_Remastered::SetActorHKHackMode)
STUB_FUNC(Lua_Remastered::CacheCurrentWalkVector)
STUB_FUNC(Lua_Remastered::GetRemappedKeyHint)
STUB_FUNC(Lua_Remastered::SetKeyMappingMode)
STUB_FUNC(Lua_Remastered::New)
STUB_FUNC(Lua_Remastered::RemoveBorders)
STUB_FUNC(Lua_Remastered::GetSaveStatus)
STUB_FUNC(Lua_Remastered::StartCheckOfCrossSaveStatus)
STUB_FUNC(Lua_Remastered::GetCrossSaveStatus)


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
	{ "RemoveHotspot", LUA_OPCODE(Lua_Remastered, RemoveHotspot) },
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
};

void Lua_Remastered::registerOpcodes() {
	Lua_V1::registerOpcodes();

	// Register main opcodes functions
	luaL_openlib(remasteredMainOpcodes, ARRAYSIZE(remasteredMainOpcodes));
}

} // end of namespace Grim
