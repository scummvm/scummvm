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

#ifndef AGS_PLUGINS_CORE_GAME_H
#define AGS_PLUGINS_CORE_GAME_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Game : public ScriptContainer {
	BUILT_IN_HASH(Game)
public:
	virtual ~Game() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void IsAudioPlaying(ScriptMethodParams &params);
	void SetAudioTypeSpeechVolumeDrop(ScriptMethodParams &params);
	void SetAudioTypeVolume(ScriptMethodParams &params);
	void StopAudio(ScriptMethodParams &params);
	void ChangeTranslation(ScriptMethodParams &params);
	void DoOnceOnly(ScriptMethodParams &params);
	void GetColorFromRGB(ScriptMethodParams &params);
	void GetFrameCountForLoop(ScriptMethodParams &params);
	void GetLocationName(ScriptMethodParams &params);
	void GetLoopCountForView(ScriptMethodParams &params);
	void GetMODPattern(ScriptMethodParams &params);
	void GetRunNextSettingForLoop(ScriptMethodParams &params);
	void GetSaveSlotDescription(ScriptMethodParams &params);
	void GetViewFrame(ScriptMethodParams &params);
	void InputBox(ScriptMethodParams &params);
	void SetSaveGameDirectory(ScriptMethodParams &params);
	void StopAllSounds(ScriptMethodParams &params);
	void GetCharacterCount(ScriptMethodParams &params);
	void GetDialogCount(ScriptMethodParams &params);
	void GetFileName(ScriptMethodParams &params);
	void GetFontCount(ScriptMethodParams &params);
	void GetGlobalMessages(ScriptMethodParams &params);
	void GetGlobalStrings(ScriptMethodParams &params);
	void SetGlobalString(ScriptMethodParams &params);
	void GetGUICount(ScriptMethodParams &params);
	void GetIgnoreUserInputAfterTextTimeoutMs(ScriptMethodParams &params);
	void SetIgnoreUserInputAfterTextTimeoutMs(ScriptMethodParams &params);
	void GetInSkippableCutscene(ScriptMethodParams &params);
	void GetInventoryItemCount(ScriptMethodParams &params);
	void GetMinimumTextDisplayTimeMs(ScriptMethodParams &params);
	void SetMinimumTextDisplayTimeMs(ScriptMethodParams &params);
	void GetMouseCursorCount(ScriptMethodParams &params);
	void GetName(ScriptMethodParams &params);
	void SetName(ScriptMethodParams &params);
	void GetNormalFont(ScriptMethodParams &params);
	void SetNormalFont(ScriptMethodParams &params);
	void GetSkippingCutscene(ScriptMethodParams &params);
	void GetSpeechFont(ScriptMethodParams &params);
	void SetSpeechFont(ScriptMethodParams &params);
	void GetSpriteWidth(ScriptMethodParams &params);
	void GetSpriteHeight(ScriptMethodParams &params);
	void GetTextReadingSpeed(ScriptMethodParams &params);
	void SetTextReadingSpeed(ScriptMethodParams &params);
	void GetTranslationFilename(ScriptMethodParams &params);
	void GetUseNativeCoordinates(ScriptMethodParams &params);
	void GetViewCount(ScriptMethodParams &params);
	void PlayVoiceClip(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
