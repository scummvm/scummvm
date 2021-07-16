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

#include "ags/plugins/core/game.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_game.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Game::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Game::IsAudioPlaying^1, IsAudioPlaying);
	SCRIPT_METHOD_EXT(Game::SetAudioTypeSpeechVolumeDrop^2, SetAudioTypeSpeechVolumeDrop);
	SCRIPT_METHOD_EXT(Game::SetAudioTypeVolume^3, SetAudioTypeVolume);
	SCRIPT_METHOD_EXT(Game::StopAudio^1, StopAudio);
	SCRIPT_METHOD_EXT(Game::ChangeTranslation^1, ChangeTranslation);
	SCRIPT_METHOD_EXT(Game::DoOnceOnly^1, DoOnceOnly);
	SCRIPT_METHOD_EXT(Game::GetColorFromRGB^3, GetColorFromRGB);
	SCRIPT_METHOD_EXT(Game::GetFrameCountForLoop^2, GetFrameCountForLoop);
	SCRIPT_METHOD_EXT(Game::GetLocationName^2, GetLocationName);
	SCRIPT_METHOD_EXT(Game::GetLoopCountForView^1, GetLoopCountForView);
	SCRIPT_METHOD_EXT(Game::GetMODPattern^0, GetMODPattern);
	SCRIPT_METHOD_EXT(Game::GetRunNextSettingForLoop^2, GetRunNextSettingForLoop);
	SCRIPT_METHOD_EXT(Game::GetSaveSlotDescription^1, GetSaveSlotDescription);
	SCRIPT_METHOD_EXT(Game::GetViewFrame^3, GetViewFrame);
	SCRIPT_METHOD_EXT(Game::InputBox^1, InputBox);
	SCRIPT_METHOD_EXT(Game::SetSaveGameDirectory^1, SetSaveGameDirectory);
	SCRIPT_METHOD_EXT(Game::StopSound^1, StopAllSounds);
	SCRIPT_METHOD_EXT(Game::get_CharacterCount, GetCharacterCount);
	SCRIPT_METHOD_EXT(Game::get_DialogCount, GetDialogCount);
	SCRIPT_METHOD_EXT(Game::get_FileName, GetFileName);
	SCRIPT_METHOD_EXT(Game::get_FontCount, GetFontCount);
	SCRIPT_METHOD_EXT(Game::geti_GlobalMessages, GetGlobalMessages);
	SCRIPT_METHOD_EXT(Game::geti_GlobalStrings, GetGlobalStrings);
	SCRIPT_METHOD_EXT(Game::seti_GlobalStrings, SetGlobalString);
	SCRIPT_METHOD_EXT(Game::get_GUICount, GetGUICount);
	SCRIPT_METHOD_EXT(Game::get_IgnoreUserInputAfterTextTimeoutMs, GetIgnoreUserInputAfterTextTimeoutMs);
	SCRIPT_METHOD_EXT(Game::set_IgnoreUserInputAfterTextTimeoutMs, SetIgnoreUserInputAfterTextTimeoutMs);
	SCRIPT_METHOD_EXT(Game::get_InSkippableCutscene, GetInSkippableCutscene);
	SCRIPT_METHOD_EXT(Game::get_InventoryItemCount, GetInventoryItemCount);
	SCRIPT_METHOD_EXT(Game::get_MinimumTextDisplayTimeMs, GetMinimumTextDisplayTimeMs);
	SCRIPT_METHOD_EXT(Game::set_MinimumTextDisplayTimeMs, SetMinimumTextDisplayTimeMs);
	SCRIPT_METHOD_EXT(Game::get_MouseCursorCount, GetMouseCursorCount);
	SCRIPT_METHOD_EXT(Game::get_Name, GetName);
	SCRIPT_METHOD_EXT(Game::set_Name, SetName);
	SCRIPT_METHOD_EXT(Game::get_NormalFont, GetNormalFont);
	SCRIPT_METHOD_EXT(Game::set_NormalFont, SetNormalFont);
	SCRIPT_METHOD_EXT(Game::get_SkippingCutscene, GetSkippingCutscene);
	SCRIPT_METHOD_EXT(Game::get_SpeechFont, GetSpeechFont);
	SCRIPT_METHOD_EXT(Game::set_SpeechFont, SetSpeechFont);
	SCRIPT_METHOD_EXT(Game::geti_SpriteWidth, GetSpriteWidth);
	SCRIPT_METHOD_EXT(Game::geti_SpriteHeight, GetSpriteHeight);
	SCRIPT_METHOD_EXT(Game::get_TextReadingSpeed, GetTextReadingSpeed);
	SCRIPT_METHOD_EXT(Game::set_TextReadingSpeed, SetTextReadingSpeed);
	SCRIPT_METHOD_EXT(Game::get_TranslationFilename, GetTranslationFilename);
	SCRIPT_METHOD_EXT(Game::get_UseNativeCoordinates, GetUseNativeCoordinates);
	SCRIPT_METHOD_EXT(Game::get_ViewCount, GetViewCount);
	SCRIPT_METHOD_EXT(Game::PlayVoiceClip, PlayVoiceClip);
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
