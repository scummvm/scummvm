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

#ifndef AGS_PLUGINS_CORE_GAME_H
#define AGS_PLUGINS_CORE_GAME_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Game : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void IsAudioPlaying(ScriptMethodParams &params);
	static void SetAudioTypeSpeechVolumeDrop(ScriptMethodParams &params);
	static void SetAudioTypeVolume(ScriptMethodParams &params);
	static void StopAudio(ScriptMethodParams &params);
	static void ChangeTranslation(ScriptMethodParams &params);
	static void DoOnceOnly(ScriptMethodParams &params);
	static void GetColorFromRGB(ScriptMethodParams &params);
	static void GetFrameCountForLoop(ScriptMethodParams &params);
	static void GetLocationName(ScriptMethodParams &params);
	static void GetLoopCountForView(ScriptMethodParams &params);
	static void GetMODPattern(ScriptMethodParams &params);
	static void GetRunNextSettingForLoop(ScriptMethodParams &params);
	static void GetSaveSlotDescription(ScriptMethodParams &params);
	static void GetViewFrame(ScriptMethodParams &params);
	static void InputBox(ScriptMethodParams &params);
	static void SetSaveGameDirectory(ScriptMethodParams &params);
	static void StopAllSounds(ScriptMethodParams &params);
	static void GetCharacterCount(ScriptMethodParams &params);
	static void GetDialogCount(ScriptMethodParams &params);
	static void GetFileName(ScriptMethodParams &params);
	static void GetFontCount(ScriptMethodParams &params);
	static void GetGlobalMessages(ScriptMethodParams &params);
	static void GetGlobalStrings(ScriptMethodParams &params);
	static void SetGlobalString(ScriptMethodParams &params);
	static void GetGUICount(ScriptMethodParams &params);
	static void GetIgnoreUserInputAfterTextTimeoutMs(ScriptMethodParams &params);
	static void SetIgnoreUserInputAfterTextTimeoutMs(ScriptMethodParams &params);
	static void GetInSkippableCutscene(ScriptMethodParams &params);
	static void GetInventoryItemCount(ScriptMethodParams &params);
	static void GetMinimumTextDisplayTimeMs(ScriptMethodParams &params);
	static void SetMinimumTextDisplayTimeMs(ScriptMethodParams &params);
	static void GetMouseCursorCount(ScriptMethodParams &params);
	static void GetName(ScriptMethodParams &params);
	static void SetName(ScriptMethodParams &params);
	static void GetNormalFont(ScriptMethodParams &params);
	static void SetNormalFont(ScriptMethodParams &params);
	static void GetSkippingCutscene(ScriptMethodParams &params);
	static void GetSpeechFont(ScriptMethodParams &params);
	static void SetSpeechFont(ScriptMethodParams &params);
	static void GetSpriteWidth(ScriptMethodParams &params);
	static void GetSpriteHeight(ScriptMethodParams &params);
	static void GetTextReadingSpeed(ScriptMethodParams &params);
	static void SetTextReadingSpeed(ScriptMethodParams &params);
	static void GetTranslationFilename(ScriptMethodParams &params);
	static void GetUseNativeCoordinates(ScriptMethodParams &params);
	static void GetViewCount(ScriptMethodParams &params);
	static void PlayVoiceClip(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
