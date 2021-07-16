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

#ifndef AGS_PLUGINS_CORE_GLOBAL_API_H
#define AGS_PLUGINS_CORE_GLOBAL_API_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class GlobalAPI : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void ScPl_sc_AbortGame(ScriptMethodParams &params);
	static void add_inventory(ScriptMethodParams &params);
	static void AddInventoryToCharacter(ScriptMethodParams &params);
	static void AnimateButton(ScriptMethodParams &params);
	static void scAnimateCharacter(ScriptMethodParams &params);
	static void AnimateCharacterEx(ScriptMethodParams &params);
	static void AnimateObject(ScriptMethodParams &params);
	static void AnimateObjectEx(ScriptMethodParams &params);
	static void AreCharactersColliding(ScriptMethodParams &params);
	static void AreCharObjColliding(ScriptMethodParams &params);
	static void AreObjectsColliding(ScriptMethodParams &params);
	static void AreThingsOverlapping(ScriptMethodParams &params);
	static void CallRoomScript(ScriptMethodParams &params);
	static void cd_manager(ScriptMethodParams &params);
	static void CentreGUI(ScriptMethodParams &params);
	static void ChangeCharacterView(ScriptMethodParams &params);
	static void ChangeCursorGraphic(ScriptMethodParams &params);
	static void ChangeCursorHotspot(ScriptMethodParams &params);
	static void ClaimEvent(ScriptMethodParams &params);
	static void CreateGraphicOverlay(ScriptMethodParams &params);
	static void ScPl_CreateTextOverlay(ScriptMethodParams &params);
	static void CyclePalette(ScriptMethodParams &params);
	static void script_debug(ScriptMethodParams &params);
	static void DeleteSaveSlot(ScriptMethodParams &params);
	static void free_dynamic_sprite(ScriptMethodParams &params);
	static void disable_cursor_mode(ScriptMethodParams &params);
	static void DisableGroundLevelAreas(ScriptMethodParams &params);
	static void DisableHotspot(ScriptMethodParams &params);
	static void DisableInterface(ScriptMethodParams &params);
	static void DisableRegion(ScriptMethodParams &params);
	static void ScPl_Display(ScriptMethodParams &params);
	static void ScPl_DisplayAt(ScriptMethodParams &params);
	static void DisplayAtY(ScriptMethodParams &params);
	static void DisplayMessage(ScriptMethodParams &params);
	static void DisplayMessageAtY(ScriptMethodParams &params);
	static void DisplayMessageBar(ScriptMethodParams &params);
	static void ScPl_sc_displayspeech(ScriptMethodParams &params);
	static void DisplaySpeechAt(ScriptMethodParams &params);
	static void DisplaySpeechBackground(ScriptMethodParams &params);
	static void ScPl_DisplayThought(ScriptMethodParams &params);
	static void ScPl_DisplayTopBar(ScriptMethodParams &params);
	static void enable_cursor_mode(ScriptMethodParams &params);
	static void EnableGroundLevelAreas(ScriptMethodParams &params);
	static void EnableHotspot(ScriptMethodParams &params);
	static void EnableInterface(ScriptMethodParams &params);
	static void EnableRegion(ScriptMethodParams &params);
	static void EndCutscene(ScriptMethodParams &params);
	static void FaceCharacter(ScriptMethodParams &params);
	static void FaceLocation(ScriptMethodParams &params);
	static void FadeIn(ScriptMethodParams &params);
	static void my_fade_out(ScriptMethodParams &params);
	static void FileClose(ScriptMethodParams &params);
	static void FileIsEOF(ScriptMethodParams &params);
	static void FileIsError(ScriptMethodParams &params);
	// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
	static void FileOpenCMode(ScriptMethodParams &params);
	static void FileRead(ScriptMethodParams &params);
	static void FileReadInt(ScriptMethodParams &params);
	static void FileReadRawChar(ScriptMethodParams &params);
	static void FileReadRawInt(ScriptMethodParams &params);
	static void FileWrite(ScriptMethodParams &params);
	static void FileWriteInt(ScriptMethodParams &params);
	static void FileWriteRawChar(ScriptMethodParams &params);
	static void FileWriteRawLine(ScriptMethodParams &params);
	static void FindGUIID(ScriptMethodParams &params);
	static void FlipScreen(ScriptMethodParams &params);
	static void FloatToInt(ScriptMethodParams &params);
	static void FollowCharacter(ScriptMethodParams &params);
	static void FollowCharacterEx(ScriptMethodParams &params);
	static void GetBackgroundFrame(ScriptMethodParams &params);
	static void GetButtonPic(ScriptMethodParams &params);
	static void GetCharIDAtScreen(ScriptMethodParams &params);
	static void GetCharacterProperty(ScriptMethodParams &params);
	static void GetCharacterPropertyText(ScriptMethodParams &params);
	static void GetCurrentMusic(ScriptMethodParams &params);
	static void GetCursorMode(ScriptMethodParams &params);
	static void GetDialogOption(ScriptMethodParams &params);
	static void GetGameOption(ScriptMethodParams &params);
	static void GetGameParameter(ScriptMethodParams &params);
	static void GetGameSpeed(ScriptMethodParams &params);
	static void GetGlobalInt(ScriptMethodParams &params);
	static void GetGlobalString(ScriptMethodParams &params);
	static void GetGraphicalVariable(ScriptMethodParams &params);
	static void GetGUIAt(ScriptMethodParams &params);
	static void GetGUIObjectAt(ScriptMethodParams &params);
	static void GetHotspotIDAtScreen(ScriptMethodParams &params);
	static void GetHotspotName(ScriptMethodParams &params);
	static void GetHotspotPointX(ScriptMethodParams &params);
	static void GetHotspotPointY(ScriptMethodParams &params);
	static void GetHotspotProperty(ScriptMethodParams &params);
	static void GetHotspotPropertyText(ScriptMethodParams &params);
	static void GetInvAt(ScriptMethodParams &params);
	static void GetInvGraphic(ScriptMethodParams &params);
	static void GetInvName(ScriptMethodParams &params);
	static void GetInvProperty(ScriptMethodParams &params);
	static void GetInvPropertyText(ScriptMethodParams &params);
	static void GetLocationName(ScriptMethodParams &params);
	static void GetLocationType(ScriptMethodParams &params);
	static void GetMessageText(ScriptMethodParams &params);
	static void GetMIDIPosition(ScriptMethodParams &params);
	static void GetMP3PosMillis(ScriptMethodParams &params);
	static void GetObjectIDAtScreen(ScriptMethodParams &params);
	static void GetObjectBaseline(ScriptMethodParams &params);
	static void GetObjectGraphic(ScriptMethodParams &params);
	static void GetObjectName(ScriptMethodParams &params);
	static void GetObjectProperty(ScriptMethodParams &params);
	static void GetObjectPropertyText(ScriptMethodParams &params);
	static void GetObjectX(ScriptMethodParams &params);
	static void GetObjectY(ScriptMethodParams &params);
	static void GetPlayerCharacter(ScriptMethodParams &params);
	static void GetRawTime(ScriptMethodParams &params);
	static void GetRegionIDAtRoom(ScriptMethodParams &params);
	static void Room_GetProperty(ScriptMethodParams &params);
	static void GetRoomPropertyText(ScriptMethodParams &params);
	static void GetSaveSlotDescription(ScriptMethodParams &params);
	static void GetScalingAt(ScriptMethodParams &params);
	static void GetSliderValue(ScriptMethodParams &params);
	static void GetTextBoxText(ScriptMethodParams &params);
	static void GetTextHeight(ScriptMethodParams &params);
	static void GetTextWidth(ScriptMethodParams &params);
	static void sc_GetTime(ScriptMethodParams &params);
	static void get_translation(ScriptMethodParams &params);
	static void GetTranslationName(ScriptMethodParams &params);
	static void GetViewportX(ScriptMethodParams &params);
	static void GetViewportY(ScriptMethodParams &params);
	static void GetWalkableAreaAtRoom(ScriptMethodParams &params);
	static void GetWalkableAreaAtScreen(ScriptMethodParams &params);
	static void GiveScore(ScriptMethodParams &params);
	static void HasPlayerBeenInRoom(ScriptMethodParams &params);
	static void HideMouseCursor(ScriptMethodParams &params);
	static void sc_inputbox(ScriptMethodParams &params);
	static void InterfaceOff(ScriptMethodParams &params);
	static void InterfaceOn(ScriptMethodParams &params);
	static void IntToFloat(ScriptMethodParams &params);
	static void sc_invscreen(ScriptMethodParams &params);
	static void IsButtonDown(ScriptMethodParams &params);
	static void IsChannelPlaying(ScriptMethodParams &params);
	static void IsGamePaused(ScriptMethodParams &params);
	static void IsGUIOn(ScriptMethodParams &params);
	static void IsInteractionAvailable(ScriptMethodParams &params);
	static void IsInventoryInteractionAvailable(ScriptMethodParams &params);
	static void IsInterfaceEnabled(ScriptMethodParams &params);
	static void IsKeyPressed(ScriptMethodParams &params);
	static void IsMusicPlaying(ScriptMethodParams &params);
	static void IsMusicVoxAvailable(ScriptMethodParams &params);
	static void IsObjectAnimating(ScriptMethodParams &params);
	static void IsObjectMoving(ScriptMethodParams &params);
	static void IsObjectOn(ScriptMethodParams &params);
	static void IsOverlayValid(ScriptMethodParams &params);
	static void IsSoundPlaying(ScriptMethodParams &params);
	static void IsTimerExpired(ScriptMethodParams &params);
	static void IsTranslationAvailable(ScriptMethodParams &params);
	static void IsVoxAvailable(ScriptMethodParams &params);
	static void ListBoxAdd(ScriptMethodParams &params);
	static void ListBoxClear(ScriptMethodParams &params);
	static void ListBoxDirList(ScriptMethodParams &params);
	static void ListBoxGetItemText(ScriptMethodParams &params);
	static void ListBoxGetNumItems(ScriptMethodParams &params);
	static void ListBoxGetSelected(ScriptMethodParams &params);
	static void ListBoxRemove(ScriptMethodParams &params);
	static void ListBoxSaveGameList(ScriptMethodParams &params);
	static void ListBoxSetSelected(ScriptMethodParams &params);
	static void ListBoxSetTopItem(ScriptMethodParams &params);
	static void LoadImageFile(ScriptMethodParams &params);
	static void LoadSaveSlotScreenshot(ScriptMethodParams &params);
	static void lose_inventory(ScriptMethodParams &params);
	static void LoseInventoryFromCharacter(ScriptMethodParams &params);
	static void MergeObject(ScriptMethodParams &params);
	static void MoveCharacter(ScriptMethodParams &params);
	static void MoveCharacterBlocking(ScriptMethodParams &params);
	static void MoveCharacterDirect(ScriptMethodParams &params);
	static void MoveCharacterPath(ScriptMethodParams &params);
	static void MoveCharacterStraight(ScriptMethodParams &params);
	static void MoveCharacterToHotspot(ScriptMethodParams &params);
	static void MoveCharacterToObject(ScriptMethodParams &params);
	static void MoveObject(ScriptMethodParams &params);
	static void MoveObjectDirect(ScriptMethodParams &params);
	static void MoveOverlay(ScriptMethodParams &params);
	static void MoveToWalkableArea(ScriptMethodParams &params);
	static void NewRoom(ScriptMethodParams &params);
	static void NewRoomEx(ScriptMethodParams &params);
	static void NewRoomNPC(ScriptMethodParams &params);
	static void ObjectOff(ScriptMethodParams &params);
	static void ObjectOn(ScriptMethodParams &params);
	static void ParseText(ScriptMethodParams &params);
	static void PauseGame(ScriptMethodParams &params);
	static void PlayAmbientSound(ScriptMethodParams &params);
	static void play_flc_file(ScriptMethodParams &params);
	static void PlayMP3File(ScriptMethodParams &params);
	static void PlayMusicResetQueue(ScriptMethodParams &params);
	static void PlayMusicQueued(ScriptMethodParams &params);
	static void PlaySilentMIDI(ScriptMethodParams &params);
	static void play_sound(ScriptMethodParams &params);
	static void PlaySoundEx(ScriptMethodParams &params);
	static void scrPlayVideo(ScriptMethodParams &params);
	static void RoomProcessClick(ScriptMethodParams &params);
	static void QuitGame(ScriptMethodParams &params);
	static void __Rand(ScriptMethodParams &params);
	static void RawClear(ScriptMethodParams &params);
	static void RawDrawCircle(ScriptMethodParams &params);
	static void RawDrawFrameTransparent(ScriptMethodParams &params);
	static void RawDrawImage(ScriptMethodParams &params);
	static void RawDrawImageOffset(ScriptMethodParams &params);
	static void RawDrawImageResized(ScriptMethodParams &params);
	static void RawDrawImageTransparent(ScriptMethodParams &params);
	static void RawDrawLine(ScriptMethodParams &params);
	static void RawDrawRectangle(ScriptMethodParams &params);
	static void RawDrawTriangle(ScriptMethodParams &params);
	static void ScPl_RawPrint(ScriptMethodParams &params);
	static void RawPrintMessageWrapped(ScriptMethodParams &params);
	static void RawRestoreScreen(ScriptMethodParams &params);
	static void RawRestoreScreenTinted(ScriptMethodParams &params);
	static void RawSaveScreen(ScriptMethodParams &params);
	static void RawSetColor(ScriptMethodParams &params);
	static void RawSetColorRGB(ScriptMethodParams &params);
	static void RefreshMouse(ScriptMethodParams &params);
	static void ReleaseCharacterView(ScriptMethodParams &params);
	static void ReleaseViewport(ScriptMethodParams &params);
	static void RemoveObjectTint(ScriptMethodParams &params);
	static void RemoveOverlay(ScriptMethodParams &params);
	static void RemoveWalkableArea(ScriptMethodParams &params);
	static void ResetRoom(ScriptMethodParams &params);
	static void restart_game(ScriptMethodParams &params);
	static void restore_game_dialog(ScriptMethodParams &params);
	static void RestoreGameSlot(ScriptMethodParams &params);
	static void RestoreWalkableArea(ScriptMethodParams &params);
	static void RunAGSGame(ScriptMethodParams &params);
	static void RunCharacterInteraction(ScriptMethodParams &params);
	static void RunDialog(ScriptMethodParams &params);
	static void RunHotspotInteraction(ScriptMethodParams &params);
	static void RunInventoryInteraction(ScriptMethodParams &params);
	static void RunObjectInteraction(ScriptMethodParams &params);
	static void RunRegionInteraction(ScriptMethodParams &params);
	static void Said(ScriptMethodParams &params);
	static void SaidUnknownWord(ScriptMethodParams &params);
	static void SaveCursorForLocationChange(ScriptMethodParams &params);
	static void save_game_dialog(ScriptMethodParams &params);
	static void save_game(ScriptMethodParams &params);
	static void SaveScreenShot(ScriptMethodParams &params);
	static void SeekMIDIPosition(ScriptMethodParams &params);
	static void SeekMODPattern(ScriptMethodParams &params);
	static void SeekMP3PosMillis(ScriptMethodParams &params);
	static void SetActiveInventory(ScriptMethodParams &params);
	static void SetAmbientTint(ScriptMethodParams &params);
	static void SetAreaLightLevel(ScriptMethodParams &params);
	static void SetAreaScaling(ScriptMethodParams &params);
	static void SetBackgroundFrame(ScriptMethodParams &params);
	static void SetButtonPic(ScriptMethodParams &params);
	static void SetButtonText(ScriptMethodParams &params);
	static void SetChannelVolume(ScriptMethodParams &params);
	static void SetCharacterBaseline(ScriptMethodParams &params);
	static void SetCharacterClickable(ScriptMethodParams &params);
	static void SetCharacterFrame(ScriptMethodParams &params);
	static void SetCharacterIdle(ScriptMethodParams &params);
	static void SetCharacterIgnoreLight(ScriptMethodParams &params);
	static void SetCharacterIgnoreWalkbehinds(ScriptMethodParams &params);
	static void SetCharacterProperty(ScriptMethodParams &params);
	static void SetCharacterBlinkView(ScriptMethodParams &params);
	static void SetCharacterSpeechView(ScriptMethodParams &params);
	static void SetCharacterSpeed(ScriptMethodParams &params);
	static void SetCharacterSpeedEx(ScriptMethodParams &params);
	static void SetCharacterTransparency(ScriptMethodParams &params);
	static void SetCharacterView(ScriptMethodParams &params);
	static void SetCharacterViewEx(ScriptMethodParams &params);
	static void SetCharacterViewOffset(ScriptMethodParams &params);
	static void set_cursor_mode(ScriptMethodParams &params);
	static void set_default_cursor(ScriptMethodParams &params);
	static void SetDialogOption(ScriptMethodParams &params);
	static void SetDigitalMasterVolume(ScriptMethodParams &params);
	static void SetFadeColor(ScriptMethodParams &params);
	static void SetFrameSound(ScriptMethodParams &params);
	static void SetGameOption(ScriptMethodParams &params);
	static void SetGameSpeed(ScriptMethodParams &params);
	static void SetGlobalInt(ScriptMethodParams &params);
	static void SetGlobalString(ScriptMethodParams &params);
	static void SetGraphicalVariable(ScriptMethodParams &params);
	static void SetGUIBackgroundPic(ScriptMethodParams &params);
	static void SetGUIClickable(ScriptMethodParams &params);
	static void SetGUIObjectEnabled(ScriptMethodParams &params);
	static void SetGUIObjectPosition(ScriptMethodParams &params);
	static void SetGUIObjectSize(ScriptMethodParams &params);
	static void SetGUIPosition(ScriptMethodParams &params);
	static void SetGUISize(ScriptMethodParams &params);
	static void SetGUITransparency(ScriptMethodParams &params);
	static void SetGUIZOrder(ScriptMethodParams &params);
	static void SetInvItemName(ScriptMethodParams &params);
	static void set_inv_item_pic(ScriptMethodParams &params);
	static void SetInvDimensions(ScriptMethodParams &params);
	static void SetLabelColor(ScriptMethodParams &params);
	static void SetLabelFont(ScriptMethodParams &params);
	static void SetLabelText(ScriptMethodParams &params);
	static void SetMouseBounds(ScriptMethodParams &params);
	static void set_mouse_cursor(ScriptMethodParams &params);
	static void SetMousePosition(ScriptMethodParams &params);
	static void SetMultitasking(ScriptMethodParams &params);
	static void SetMusicMasterVolume(ScriptMethodParams &params);
	static void SetMusicRepeat(ScriptMethodParams &params);
	static void SetMusicVolume(ScriptMethodParams &params);
	static void SetNextCursor(ScriptMethodParams &params);
	static void SetNextScreenTransition(ScriptMethodParams &params);
	static void SetNormalFont(ScriptMethodParams &params);
	static void SetObjectBaseline(ScriptMethodParams &params);
	static void SetObjectClickable(ScriptMethodParams &params);
	static void SetObjectFrame(ScriptMethodParams &params);
	static void SetObjectGraphic(ScriptMethodParams &params);
	static void SetObjectIgnoreWalkbehinds(ScriptMethodParams &params);
	static void SetObjectPosition(ScriptMethodParams &params);
	static void SetObjectTint(ScriptMethodParams &params);
	static void SetObjectTransparency(ScriptMethodParams &params);
	static void SetObjectView(ScriptMethodParams &params);
	static void SetPalRGB(ScriptMethodParams &params);
	static void SetPlayerCharacter(ScriptMethodParams &params);
	static void SetRegionTint(ScriptMethodParams &params);
	static void SetRestartPoint(ScriptMethodParams &params);
	static void SetScreenTransition(ScriptMethodParams &params);
	static void SetSkipSpeech(ScriptMethodParams &params);
	static void SetSliderValue(ScriptMethodParams &params);
	static void SetSoundVolume(ScriptMethodParams &params);
	static void SetSpeechFont(ScriptMethodParams &params);
	static void SetSpeechStyle(ScriptMethodParams &params);
	static void SetSpeechVolume(ScriptMethodParams &params);
	static void SetTalkingColor(ScriptMethodParams &params);
	static void SetTextBoxFont(ScriptMethodParams &params);
	static void SetTextBoxText(ScriptMethodParams &params);
	static void ScPl_SetTextOverlay(ScriptMethodParams &params);
	static void SetTextWindowGUI(ScriptMethodParams &params);
	static void script_SetTimer(ScriptMethodParams &params);
	static void SetViewport(ScriptMethodParams &params);
	static void SetVoiceMode(ScriptMethodParams &params);
	static void SetWalkBehindBase(ScriptMethodParams &params);
	static void ShakeScreen(ScriptMethodParams &params);
	static void ShakeScreenBackground(ScriptMethodParams &params);
	static void ShowMouseCursor(ScriptMethodParams &params);
	static void SkipUntilCharacterStops(ScriptMethodParams &params);
	static void StartCutscene(ScriptMethodParams &params);
	static void scStartRecording(ScriptMethodParams &params);
	static void StopAmbientSound(ScriptMethodParams &params);
	static void stop_and_destroy_channel(ScriptMethodParams &params);
	static void StopDialog(ScriptMethodParams &params);
	static void StopMoving(ScriptMethodParams &params);
	static void scr_StopMusic(ScriptMethodParams &params);
	static void StopObjectMoving(ScriptMethodParams &params);
	static void _sc_strcat(ScriptMethodParams &params);
	static void ags_stricmp(ScriptMethodParams &params);
	static void strcmp(ScriptMethodParams &params);
	static void StrContains(ScriptMethodParams &params);
	static void _sc_strcpy(ScriptMethodParams &params);
	static void ScPl_sc_sprintf(ScriptMethodParams &params);
	static void StrGetCharAt(ScriptMethodParams &params);
	static void StringToInt(ScriptMethodParams &params);
	static void strlen(ScriptMethodParams &params);
	static void StrSetCharAt(ScriptMethodParams &params);
	static void _sc_strlower(ScriptMethodParams &params);
	static void _sc_strupper(ScriptMethodParams &params);
	static void TintScreen(ScriptMethodParams &params);
	static void UnPauseGame(ScriptMethodParams &params);
	static void update_invorder(ScriptMethodParams &params);
	static void UpdatePalette(ScriptMethodParams &params);
	static void scrWait(ScriptMethodParams &params);
	static void WaitKey(ScriptMethodParams &params);
	static void WaitMouseKey(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
