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

#include "ags/plugins/core/game.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_game.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Game::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Game::IsAudioPlaying^1, Game::IsAudioPlaying);
	SCRIPT_METHOD(Game::SetAudioTypeSpeechVolumeDrop^2, Game::SetAudioTypeSpeechVolumeDrop);
	SCRIPT_METHOD(Game::SetAudioTypeVolume^3, Game::SetAudioTypeVolume);
	SCRIPT_METHOD(Game::StopAudio^1, Game::StopAudio);
	SCRIPT_METHOD(Game::ChangeTranslation^1, Game::ChangeTranslation);
	SCRIPT_METHOD(Game::DoOnceOnly^1, Game::DoOnceOnly);
	SCRIPT_METHOD(Game::GetColorFromRGB^3, Game::GetColorFromRGB);
	SCRIPT_METHOD(Game::GetFrameCountForLoop^2, Game::GetFrameCountForLoop);
	SCRIPT_METHOD(Game::GetLocationName^2, Game::GetLocationName);
	SCRIPT_METHOD(Game::GetLoopCountForView^1, Game::GetLoopCountForView);
	SCRIPT_METHOD(Game::GetMODPattern^0, Game::GetMODPattern);
	SCRIPT_METHOD(Game::GetRunNextSettingForLoop^2, Game::GetRunNextSettingForLoop);
	SCRIPT_METHOD(Game::GetSaveSlotDescription^1, Game::GetSaveSlotDescription);
	SCRIPT_METHOD(Game::GetViewFrame^3, Game::GetViewFrame);
	SCRIPT_METHOD(Game::InputBox^1, Game::InputBox);
	SCRIPT_METHOD(Game::SetSaveGameDirectory^1, Game::SetSaveGameDirectory);
	SCRIPT_METHOD(Game::StopSound^1, Game::StopAllSounds);
	SCRIPT_METHOD(Game::get_CharacterCount, Game::GetCharacterCount);
	SCRIPT_METHOD(Game::get_DialogCount, Game::GetDialogCount);
	SCRIPT_METHOD(Game::get_FileName, Game::GetFileName);
	SCRIPT_METHOD(Game::get_FontCount, Game::GetFontCount);
	SCRIPT_METHOD(Game::geti_GlobalMessages, Game::GetGlobalMessages);
	SCRIPT_METHOD(Game::geti_GlobalStrings, Game::GetGlobalStrings);
	SCRIPT_METHOD(Game::seti_GlobalStrings, Game::SetGlobalString);
	SCRIPT_METHOD(Game::get_GUICount, Game::GetGUICount);
	SCRIPT_METHOD(Game::get_IgnoreUserInputAfterTextTimeoutMs, Game::GetIgnoreUserInputAfterTextTimeoutMs);
	SCRIPT_METHOD(Game::set_IgnoreUserInputAfterTextTimeoutMs, Game::SetIgnoreUserInputAfterTextTimeoutMs);
	SCRIPT_METHOD(Game::get_InSkippableCutscene, Game::GetInSkippableCutscene);
	SCRIPT_METHOD(Game::get_InventoryItemCount, Game::GetInventoryItemCount);
	SCRIPT_METHOD(Game::get_MinimumTextDisplayTimeMs, Game::GetMinimumTextDisplayTimeMs);
	SCRIPT_METHOD(Game::set_MinimumTextDisplayTimeMs, Game::SetMinimumTextDisplayTimeMs);
	SCRIPT_METHOD(Game::get_MouseCursorCount, Game::GetMouseCursorCount);
	SCRIPT_METHOD(Game::get_Name, Game::GetName);
	SCRIPT_METHOD(Game::set_Name, Game::SetName);
	SCRIPT_METHOD(Game::get_NormalFont, Game::GetNormalFont);
	SCRIPT_METHOD(Game::set_NormalFont, Game::SetNormalFont);
	SCRIPT_METHOD(Game::get_SkippingCutscene, Game::GetSkippingCutscene);
	SCRIPT_METHOD(Game::get_SpeechFont, Game::GetSpeechFont);
	SCRIPT_METHOD(Game::set_SpeechFont, Game::SetSpeechFont);
	SCRIPT_METHOD(Game::geti_SpriteWidth, Game::GetSpriteWidth);
	SCRIPT_METHOD(Game::geti_SpriteHeight, Game::GetSpriteHeight);
	SCRIPT_METHOD(Game::get_TextReadingSpeed, Game::GetTextReadingSpeed);
	SCRIPT_METHOD(Game::set_TextReadingSpeed, Game::SetTextReadingSpeed);
	SCRIPT_METHOD(Game::get_TranslationFilename, Game::GetTranslationFilename);
	SCRIPT_METHOD(Game::get_UseNativeCoordinates, Game::GetUseNativeCoordinates);
	SCRIPT_METHOD(Game::get_ViewCount, Game::GetViewCount);
	SCRIPT_METHOD(Game::PlayVoiceClip, Game::PlayVoiceClip);
}

void Game::IsAudioPlaying(ScriptMethodParams &params) {
	PARAMS1(int, audioType);
	params._result = AGS3::Game_IsAudioPlaying(audioType);
}

void Game::SetAudioTypeSpeechVolumeDrop(ScriptMethodParams &params) {
	PARAMS2(int, audioType, int, volumeDrop);
	AGS3::Game_SetAudioTypeSpeechVolumeDrop(audioType, volumeDrop);
}

void Game::SetAudioTypeVolume(ScriptMethodParams &params) {
	PARAMS3(int, audioType, int, volume, int, changeType);
	AGS3::Game_SetAudioTypeVolume(audioType, volume, changeType);
}

void Game::StopAudio(ScriptMethodParams &params) {
	PARAMS1(int, audioType);
	AGS3::Game_StopAudio(audioType);
}

void Game::ChangeTranslation(ScriptMethodParams &params) {
	PARAMS1(const char *, newFilename);
	params._result = AGS3::Game_ChangeTranslation(newFilename);
}

void Game::DoOnceOnly(ScriptMethodParams &params) {
	PARAMS1(const char *, token);
	params._result = AGS3::Game_DoOnceOnly(token);
}

void Game::GetColorFromRGB(ScriptMethodParams &params) {
	PARAMS3(int, red, int, grn, int, blu);
	params._result = AGS3::Game_GetColorFromRGB(red, grn, blu);
}

void Game::GetFrameCountForLoop(ScriptMethodParams &params) {
	PARAMS2(int, viewNumber, int, loopNumber);
	params._result = AGS3::Game_GetFrameCountForLoop(viewNumber, loopNumber);
}

void Game::GetLocationName(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::Game_GetLocationName(x, y);
}

void Game::GetLoopCountForView(ScriptMethodParams &params) {
	PARAMS1(int, viewNumber);
	params._result = AGS3::Game_GetLoopCountForView(viewNumber);
}

void Game::GetMODPattern(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetMODPattern();
}

void Game::GetRunNextSettingForLoop(ScriptMethodParams &params) {
	PARAMS2(int, viewNumber, int, loopNumber);
	params._result = AGS3::Game_GetRunNextSettingForLoop(viewNumber, loopNumber);
}

void Game::GetSaveSlotDescription(ScriptMethodParams &params) {
	PARAMS1(int, slnum);
	params._result = AGS3::Game_GetSaveSlotDescription(slnum);
}

void Game::GetViewFrame(ScriptMethodParams &params) {
	PARAMS3(int, viewNumber, int, loopNumber, int, frame);
	params._result = AGS3::Game_GetViewFrame(viewNumber, loopNumber, frame);
}

void Game::InputBox(ScriptMethodParams &params) {
	PARAMS1(const char *, msg);
	params._result = AGS3::Game_InputBox(msg);
}

void Game::SetSaveGameDirectory(ScriptMethodParams &params) {
	PARAMS1(const char *, newFolder);
	params._result = AGS3::Game_SetSaveGameDirectory(newFolder);
}

void Game::StopAllSounds(ScriptMethodParams &params) {
	PARAMS1(int, evenAmbient);
	AGS3::StopAllSounds(evenAmbient);
}

void Game::GetCharacterCount(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetCharacterCount();
}

void Game::GetDialogCount(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetDialogCount();
}

void Game::GetFileName(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetFileName();
}

void Game::GetFontCount(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetFontCount();
}

void Game::GetGlobalMessages(ScriptMethodParams &params) {
	PARAMS1(int, index);
	params._result = AGS3::Game_GetGlobalMessages(index);
}

void Game::GetGlobalStrings(ScriptMethodParams &params) {
	PARAMS1(int, index);
	params._result = AGS3::Game_GetGlobalStrings(index);
}

void Game::SetGlobalString(ScriptMethodParams &params) {
	PARAMS2(int, index, const char *, newVal);
	AGS3::SetGlobalString(index, newVal);
}

void Game::GetGUICount(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetGUICount();
}

void Game::GetIgnoreUserInputAfterTextTimeoutMs(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetIgnoreUserInputAfterTextTimeoutMs();
}

void Game::SetIgnoreUserInputAfterTextTimeoutMs(ScriptMethodParams &params) {
	PARAMS1(int, newValueMs);
	AGS3::Game_SetIgnoreUserInputAfterTextTimeoutMs(newValueMs);
}

void Game::GetInSkippableCutscene(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetInSkippableCutscene();
}

void Game::GetInventoryItemCount(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetInventoryItemCount();
}

void Game::GetMinimumTextDisplayTimeMs(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetMinimumTextDisplayTimeMs();
}

void Game::SetMinimumTextDisplayTimeMs(ScriptMethodParams &params) {
	PARAMS1(int, newTextMinTime);
	AGS3::Game_SetMinimumTextDisplayTimeMs(newTextMinTime);
}

void Game::GetMouseCursorCount(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetMouseCursorCount();
}

void Game::GetName(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetName();
}

void Game::SetName(ScriptMethodParams &params) {
	PARAMS1(const char *, newName);
	AGS3::Game_SetName(newName);
}

void Game::GetNormalFont(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetNormalFont();
}

void Game::SetNormalFont(ScriptMethodParams &params) {
	PARAMS1(int, fontNum);
	AGS3::SetNormalFont(fontNum);
}

void Game::GetSkippingCutscene(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetName();
}

void Game::GetSpeechFont(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetSpeechFont();
}

void Game::SetSpeechFont(ScriptMethodParams &params) {
	PARAMS1(int, fontNum);
	AGS3::SetSpeechFont(fontNum);
}

void Game::GetSpriteWidth(ScriptMethodParams &params) {
	PARAMS1(int, spriteNum);
	params._result = AGS3::Game_GetSpriteWidth(spriteNum);
}

void Game::GetSpriteHeight(ScriptMethodParams &params) {
	PARAMS1(int, spriteNum);
	params._result = AGS3::Game_GetSpriteHeight(spriteNum);
}

void Game::GetTextReadingSpeed(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetTextReadingSpeed();
}

void Game::SetTextReadingSpeed(ScriptMethodParams &params) {
	PARAMS1(int, newTextSpeed);
	AGS3::Game_SetTextReadingSpeed(newTextSpeed);
}

void Game::GetTranslationFilename(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetTranslationFilename();
}

void Game::GetUseNativeCoordinates(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetUseNativeCoordinates();
}

void Game::GetViewCount(ScriptMethodParams &params) {
	params._result = AGS3::Game_GetViewCount();
}

void Game::PlayVoiceClip(ScriptMethodParams &params) {
	PARAMS3(CharacterInfo *, ch, int, sndid, bool, as_speech);
	params._result = AGS3::PlayVoiceClip(ch, sndid, as_speech);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
