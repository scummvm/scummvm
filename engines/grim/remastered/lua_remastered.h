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

#ifndef GRIM_LUA_REMASTERED
#define GRIM_LUA_REMASTERED

#include "engines/grim/lua_v1.h"
#include "engines/grim/remastered/commentary.h"

namespace Grim {

class Lua_Remastered : public Lua_V1 {
public:
	typedef Lua_Remastered LuaClass;
	void registerOpcodes() override;

protected:

	// Overrides from Lua_V1
	DECLARE_LUA_OPCODE(GetFontDimensions) override;
	DECLARE_LUA_OPCODE(GetTextObjectDimensions) override;
	DECLARE_LUA_OPCODE(Load) override;
	DECLARE_LUA_OPCODE(Save) override;

	// Remastered
	DECLARE_LUA_OPCODE(GetPlatform);
	DECLARE_LUA_OPCODE(GetLanguage);
	DECLARE_LUA_OPCODE(PreloadCursors);
	DECLARE_LUA_OPCODE(ReadRegistryIntValue);
	DECLARE_LUA_OPCODE(WidescreenCorrectionFactor);
	DECLARE_LUA_OPCODE(InitiateFindSaveGames);
	DECLARE_LUA_OPCODE(FindSaveGames);
	DECLARE_LUA_OPCODE(GetFindSaveGameStatus);
	DECLARE_LUA_OPCODE(AreAchievementsInstalled);
	DECLARE_LUA_OPCODE(UnlockAchievement);
	DECLARE_LUA_OPCODE(ImGetCommentaryVol);
	DECLARE_LUA_OPCODE(ImSetCommentaryVol);
	DECLARE_LUA_OPCODE(SetMouseSpeedScale);
	DECLARE_LUA_OPCODE(SetResolutionScaling);
	DECLARE_LUA_OPCODE(SetAdvancedLighting);
	DECLARE_LUA_OPCODE(SetLanguage);
	DECLARE_LUA_OPCODE(PlayCurrentCommentary);
	DECLARE_LUA_OPCODE(IsPlayingCommentary);
	DECLARE_LUA_OPCODE(EnableCommentary);
	DECLARE_LUA_OPCODE(ClearCommentary);
	DECLARE_LUA_OPCODE(HasHeardCommentary);
	DECLARE_LUA_OPCODE(SetCommentary);
	DECLARE_LUA_OPCODE(LoadRemappedKeys);
	DECLARE_LUA_OPCODE(GlobalSaveResolved);
	DECLARE_LUA_OPCODE(StopCommentaryImmediately);
	DECLARE_LUA_OPCODE(DestroyAllUIButtonsImmediately);
	DECLARE_LUA_OPCODE(UpdateUIButtons);
	DECLARE_LUA_OPCODE(OverlayClearCache);
	DECLARE_LUA_OPCODE(GetGameRenderMode);
	DECLARE_LUA_OPCODE(SetGameRenderMode);
	DECLARE_LUA_OPCODE(OverlayMove);
	DECLARE_LUA_OPCODE(OverlayCreate);
	DECLARE_LUA_OPCODE(OverlayDestroy);
	DECLARE_LUA_OPCODE(OverlayFade);
	DECLARE_LUA_OPCODE(OverlayGetScreenSize);
	DECLARE_LUA_OPCODE(OverlayDimensions);
	DECLARE_LUA_OPCODE(AddHotspot);
	DECLARE_LUA_OPCODE(LinkHotspot);
	DECLARE_LUA_OPCODE(RemoveHotspot);
	DECLARE_LUA_OPCODE(UpdateHotspot);
	DECLARE_LUA_OPCODE(QueryActiveHotspots);
	DECLARE_LUA_OPCODE(HideMouseCursor);
	DECLARE_LUA_OPCODE(SetCursor);
	DECLARE_LUA_OPCODE(ShowCursor);
	DECLARE_LUA_OPCODE(UpdateMouseCursor);
	DECLARE_LUA_OPCODE(GetCursorPosition);
	DECLARE_LUA_OPCODE(UnlockCutscene);
	DECLARE_LUA_OPCODE(IsCutsceneUnlocked);
	DECLARE_LUA_OPCODE(SetActorHKHackMode);
	DECLARE_LUA_OPCODE(CacheCurrentWalkVector);
	DECLARE_LUA_OPCODE(UnlockConcept);
	DECLARE_LUA_OPCODE(IsConceptUnlocked);
	DECLARE_LUA_OPCODE(SaveRegistryToDisk);
	DECLARE_LUA_OPCODE(GetRemappedKeyName);
	DECLARE_LUA_OPCODE(GetRemappedKeyHint);
	DECLARE_LUA_OPCODE(SetKeyMappingMode);
	DECLARE_LUA_OPCODE(ResetKeyMappingToDefault);
	DECLARE_LUA_OPCODE(SaveRemappedKeys);
	DECLARE_LUA_OPCODE(New);
	DECLARE_LUA_OPCODE(RemoveBorders);
	DECLARE_LUA_OPCODE(GetSaveStatus);
	DECLARE_LUA_OPCODE(StartCheckOfCrossSaveStatus);
	DECLARE_LUA_OPCODE(GetCrossSaveStatus);
	DECLARE_LUA_OPCODE(GetFloorWalkPos);
	DECLARE_LUA_OPCODE(CursorMovieEnabled);
};

} // end of namespace Grim

#endif
