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

//=============================================================================
//
// Registering symbols for the script system
//
//=============================================================================

#include "ags/shared/ac/game_struct_defines.h"

namespace AGS3 {

extern void RegisterAudioChannelAPI();
extern void RegisterAudioClipAPI();
extern void RegisterButtonAPI();
extern void RegisterCharacterAPI(ScriptAPIVersion base_api, ScriptAPIVersion compat_api);
extern void RegisterContainerAPI();
extern void RegisterDateTimeAPI();
extern void RegisterDialogAPI();
extern void RegisterDialogOptionsRenderingAPI();
extern void RegisterDrawingSurfaceAPI(ScriptAPIVersion base_api, ScriptAPIVersion compat_api);
extern void RegisterDynamicSpriteAPI();
extern void RegisterFileAPI();
extern void RegisterGameAPI();
extern void RegisterGlobalAPI();
extern void RegisterGUIAPI();
extern void RegisterGUIControlAPI();
extern void RegisterHotspotAPI();
extern void RegisterInventoryItemAPI();
extern void RegisterInventoryWindowAPI();
extern void RegisterLabelAPI();
extern void RegisterListBoxAPI();
extern void RegisterMathAPI();
extern void RegisterMouseAPI();
extern void RegisterObjectAPI();
extern void RegisterOverlayAPI();
extern void RegisterParserAPI();
extern void RegisterRegionAPI();
extern void RegisterRoomAPI();
extern void RegisterScreenAPI();
extern void RegisterSliderAPI();
extern void RegisterSpeechAPI(ScriptAPIVersion base_api, ScriptAPIVersion compat_api);
extern void RegisterStringAPI();
extern void RegisterSystemAPI();
extern void RegisterTextBoxAPI();
extern void RegisterViewFrameAPI();
extern void RegisterViewportAPI();

extern void RegisterStaticObjects();

void setup_script_exports(ScriptAPIVersion base_api, ScriptAPIVersion compat_api) {
	RegisterAudioChannelAPI();
	RegisterAudioClipAPI();
	RegisterButtonAPI();
	RegisterCharacterAPI(base_api, compat_api);
	RegisterContainerAPI();
	RegisterDateTimeAPI();
	RegisterDialogAPI();
	RegisterDialogOptionsRenderingAPI();
	RegisterDrawingSurfaceAPI(base_api, compat_api);
	RegisterDynamicSpriteAPI();
	RegisterFileAPI();
	RegisterGameAPI();
	RegisterGlobalAPI();
	RegisterGUIAPI();
	RegisterGUIControlAPI();
	RegisterHotspotAPI();
	RegisterInventoryItemAPI();
	RegisterInventoryWindowAPI();
	RegisterLabelAPI();
	RegisterListBoxAPI();
	RegisterMathAPI();
	RegisterMouseAPI();
	RegisterObjectAPI();
	RegisterOverlayAPI();
	RegisterParserAPI();
	RegisterRegionAPI();
	RegisterRoomAPI();
	RegisterScreenAPI();
	RegisterSliderAPI();
	RegisterSpeechAPI(base_api, compat_api);
	RegisterStringAPI();
	RegisterSystemAPI();
	RegisterTextBoxAPI();
	RegisterViewFrameAPI();
	RegisterViewportAPI();

	RegisterStaticObjects();
}

} // namespace AGS3
