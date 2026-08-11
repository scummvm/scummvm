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

#ifndef AGS_PLUGINS_CORE_GLOBAL_API_H
#define AGS_PLUGINS_CORE_GLOBAL_API_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class GlobalAPI : public ScriptContainer {
	BUILT_IN_HASH(GlobalAPI)
public:
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void ScPl_sc_AbortGame(ScriptMethodParams &params);
	void add_inventory(ScriptMethodParams &params);
	void AddInventoryToCharacter(ScriptMethodParams &params);
	void AnimateButton(ScriptMethodParams &params);
	void AnimateCharacter4(ScriptMethodParams &params);
	void AnimateCharacter6(ScriptMethodParams &params);
	void AnimateObject4(ScriptMethodParams &params);
	void AnimateObject6(ScriptMethodParams &params);
	void AreCharactersColliding(ScriptMethodParams &params);
	void AreCharObjColliding(ScriptMethodParams &params);
	void AreObjectsColliding(ScriptMethodParams &params);
	void AreThingsOverlapping(ScriptMethodParams &params);
	void CallRoomScript(ScriptMethodParams &params);
	void cd_manager(ScriptMethodParams &params);
	void CentreGUI(ScriptMethodParams &params);
	void ChangeCharacterView(ScriptMethodParams &params);
	void ChangeCursorGraphic(ScriptMethodParams &params);
	void ChangeCursorHotspot(ScriptMethodParams &params);
	void ClaimEvent(ScriptMethodParams &params);
	void CreateGraphicOverlay(ScriptMethodParams &params);
	void ScPl_CreateTextOverlay(ScriptMethodParams &params);
	void CyclePalette(ScriptMethodParams &params);
	void script_debug(ScriptMethodParams &params);
	void DeleteSaveSlot(ScriptMethodParams &params);
	void free_dynamic_sprite(ScriptMethodParams &params);
	void disable_cursor_mode(ScriptMethodParams &params);
	void DisableGroundLevelAreas(ScriptMethodParams &params);
	void DisableHotspot(ScriptMethodParams &params);
	void DisableInterface(ScriptMethodParams &params);
	void DisableRegion(ScriptMethodParams &params);
	void ScPl_Display(ScriptMethodParams &params);
	void ScPl_DisplayAt(ScriptMethodParams &params);
	void DisplayAtY(ScriptMethodParams &params);
	void DisplayMessage(ScriptMethodParams &params);
	void DisplayMessageAtY(ScriptMethodParams &params);
	void DisplayMessageBar(ScriptMethodParams &params);
	void ScPl_sc_displayspeech(ScriptMethodParams &params);
	void DisplaySpeechAt(ScriptMethodParams &params);
	void DisplaySpeechBackground(ScriptMethodParams &params);
	void ScPl_DisplayThought(ScriptMethodParams &params);
	void ScPl_DisplayTopBar(ScriptMethodParams &params);
	void enable_cursor_mode(ScriptMethodParams &params);
	void EnableGroundLevelAreas(ScriptMethodParams &params);
	void EnableHotspot(ScriptMethodParams &params);
	void EnableInterface(ScriptMethodParams &params);
	void EnableRegion(ScriptMethodParams &params);
	void EndCutscene(ScriptMethodParams &params);
	void FaceCharacter(ScriptMethodParams &params);
	void FaceLocation(ScriptMethodParams &params);
	void FadeIn(ScriptMethodParams &params);
	void FadeOut(ScriptMethodParams &params);
	void FileClose(ScriptMethodParams &params);
	void FileIsEOF(ScriptMethodParams &params);
	void FileIsError(ScriptMethodParams &params);
	// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
	void FileOpenCMode(ScriptMethodParams &params);
	void FileRead(ScriptMethodParams &params);
	void FileReadInt(ScriptMethodParams &params);
	void FileReadRawChar(ScriptMethodParams &params);
	void FileReadRawInt(ScriptMethodParams &params);
	void FileWrite(ScriptMethodParams &params);
	void FileWriteInt(ScriptMethodParams &params);
	void FileWriteRawChar(ScriptMethodParams &params);
	void FileWriteRawLine(ScriptMethodParams &params);
	void FindGUIID(ScriptMethodParams &params);
	void FlipScreen(ScriptMethodParams &params);
	void FloatToInt(ScriptMethodParams &params);
	void FollowCharacter(ScriptMethodParams &params);
	void FollowCharacterEx(ScriptMethodParams &params);
	void GetBackgroundFrame(ScriptMethodParams &params);
	void GetButtonPic(ScriptMethodParams &params);
	void GetCharIDAtScreen(ScriptMethodParams &params);
	void GetCharacterProperty(ScriptMethodParams &params);
	void GetCharacterPropertyText(ScriptMethodParams &params);
	void GetCurrentMusic(ScriptMethodParams &params);
	void GetCursorMode(ScriptMethodParams &params);
	void GetDialogOption(ScriptMethodParams &params);
	void GetGameOption(ScriptMethodParams &params);
	void GetGameParameter(ScriptMethodParams &params);
	void GetGameSpeed(ScriptMethodParams &params);
	void GetGlobalInt(ScriptMethodParams &params);
	void GetGlobalString(ScriptMethodParams &params);
	void GetGraphicalVariable(ScriptMethodParams &params);
	void GetGUIAt(ScriptMethodParams &params);
	void GetGUIObjectAt(ScriptMethodParams &params);
	void GetHotspotIDAtScreen(ScriptMethodParams &params);
	void GetHotspotName(ScriptMethodParams &params);
	void GetHotspotPointX(ScriptMethodParams &params);
	void GetHotspotPointY(ScriptMethodParams &params);
	void GetHotspotProperty(ScriptMethodParams &params);
	void GetHotspotPropertyText(ScriptMethodParams &params);
	void GetInvAt(ScriptMethodParams &params);
	void GetInvGraphic(ScriptMethodParams &params);
	void GetInvName(ScriptMethodParams &params);
	void GetInvProperty(ScriptMethodParams &params);
	void GetInvPropertyText(ScriptMethodParams &params);
	void GetLocationName(ScriptMethodParams &params);
	void GetLocationType(ScriptMethodParams &params);
	void GetMessageText(ScriptMethodParams &params);
	void GetMIDIPosition(ScriptMethodParams &params);
	void GetMP3PosMillis(ScriptMethodParams &params);
	void GetObjectIDAtScreen(ScriptMethodParams &params);
	void GetObjectBaseline(ScriptMethodParams &params);
	void GetObjectGraphic(ScriptMethodParams &params);
	void GetObjectName(ScriptMethodParams &params);
	void GetObjectProperty(ScriptMethodParams &params);
	void GetObjectPropertyText(ScriptMethodParams &params);
	void GetObjectX(ScriptMethodParams &params);
	void GetObjectY(ScriptMethodParams &params);
	void GetPlayerCharacter(ScriptMethodParams &params);
	void GetRawTime(ScriptMethodParams &params);
	void GetRegionIDAtRoom(ScriptMethodParams &params);
	void Room_GetProperty(ScriptMethodParams &params);
	void GetRoomPropertyText(ScriptMethodParams &params);
	void GetSaveSlotDescription(ScriptMethodParams &params);
	void GetScalingAt(ScriptMethodParams &params);
	void GetSliderValue(ScriptMethodParams &params);
	void GetTextBoxText(ScriptMethodParams &params);
	void GetTextHeight(ScriptMethodParams &params);
	void GetTextWidth(ScriptMethodParams &params);
	void sc_GetTime(ScriptMethodParams &params);
	void get_translation(ScriptMethodParams &params);
	void GetTranslationName(ScriptMethodParams &params);
	void GetViewportX(ScriptMethodParams &params);
	void GetViewportY(ScriptMethodParams &params);
	void GetWalkableAreaAtRoom(ScriptMethodParams &params);
	void GetWalkableAreaAtScreen(ScriptMethodParams &params);
	void GiveScore(ScriptMethodParams &params);
	void HasPlayerBeenInRoom(ScriptMethodParams &params);
	void HideMouseCursor(ScriptMethodParams &params);
	void ShowInputBox(ScriptMethodParams &params);
	void InterfaceOff(ScriptMethodParams &params);
	void InterfaceOn(ScriptMethodParams &params);
	void IntToFloat(ScriptMethodParams &params);
	void sc_invscreen(ScriptMethodParams &params);
	void IsButtonDown(ScriptMethodParams &params);
	void IsChannelPlaying(ScriptMethodParams &params);
	void IsGamePaused(ScriptMethodParams &params);
	void IsGUIOn(ScriptMethodParams &params);
	void IsInteractionAvailable(ScriptMethodParams &params);
	void IsInventoryInteractionAvailable(ScriptMethodParams &params);
	void IsInterfaceEnabled(ScriptMethodParams &params);
	void IsKeyPressed(ScriptMethodParams &params);
	void IsMusicPlaying(ScriptMethodParams &params);
	void IsMusicVoxAvailable(ScriptMethodParams &params);
	void IsObjectAnimating(ScriptMethodParams &params);
	void IsObjectMoving(ScriptMethodParams &params);
	void IsObjectOn(ScriptMethodParams &params);
	void IsOverlayValid(ScriptMethodParams &params);
	void IsSoundPlaying(ScriptMethodParams &params);
	void IsTimerExpired(ScriptMethodParams &params);
	void IsTranslationAvailable(ScriptMethodParams &params);
	void IsVoxAvailable(ScriptMethodParams &params);
	void ListBoxAdd(ScriptMethodParams &params);
	void ListBoxClear(ScriptMethodParams &params);
	void ListBoxDirList(ScriptMethodParams &params);
	void ListBoxGetItemText(ScriptMethodParams &params);
	void ListBoxGetNumItems(ScriptMethodParams &params);
	void ListBoxGetSelected(ScriptMethodParams &params);
	void ListBoxRemove(ScriptMethodParams &params);
	void ListBoxSaveGameList(ScriptMethodParams &params);
	void ListBoxSetSelected(ScriptMethodParams &params);
	void ListBoxSetTopItem(ScriptMethodParams &params);
	void LoadImageFile(ScriptMethodParams &params);
	void LoadSaveSlotScreenshot(ScriptMethodParams &params);
	void lose_inventory(ScriptMethodParams &params);
	void LoseInventoryFromCharacter(ScriptMethodParams &params);
	void MergeObject(ScriptMethodParams &params);
	void MoveCharacter(ScriptMethodParams &params);
	void MoveCharacterBlocking(ScriptMethodParams &params);
	void MoveCharacterDirect(ScriptMethodParams &params);
	void MoveCharacterPath(ScriptMethodParams &params);
	void MoveCharacterStraight(ScriptMethodParams &params);
	void MoveCharacterToHotspot(ScriptMethodParams &params);
	void MoveCharacterToObject(ScriptMethodParams &params);
	void MoveObject(ScriptMethodParams &params);
	void MoveObjectDirect(ScriptMethodParams &params);
	void MoveOverlay(ScriptMethodParams &params);
	void MoveToWalkableArea(ScriptMethodParams &params);
	void NewRoom(ScriptMethodParams &params);
	void NewRoomEx(ScriptMethodParams &params);
	void NewRoomNPC(ScriptMethodParams &params);
	void ObjectOff(ScriptMethodParams &params);
	void ObjectOn(ScriptMethodParams &params);
	void ParseText(ScriptMethodParams &params);
	void PauseGame(ScriptMethodParams &params);
	void PlayAmbientSound(ScriptMethodParams &params);
	void PlayFlic(ScriptMethodParams &params);
	void PlayMP3File(ScriptMethodParams &params);
	void PlayMusicResetQueue(ScriptMethodParams &params);
	void PlayMusicQueued(ScriptMethodParams &params);
	void PlaySilentMIDI(ScriptMethodParams &params);
	void play_sound(ScriptMethodParams &params);
	void PlaySoundEx(ScriptMethodParams &params);
	void PlayVideo(ScriptMethodParams &params);
	void RoomProcessClick(ScriptMethodParams &params);
	void QuitGame(ScriptMethodParams &params);
	void __Rand(ScriptMethodParams &params);
	void RawClear(ScriptMethodParams &params);
	void RawDrawCircle(ScriptMethodParams &params);
	void RawDrawFrameTransparent(ScriptMethodParams &params);
	void RawDrawImage(ScriptMethodParams &params);
	void RawDrawImageOffset(ScriptMethodParams &params);
	void RawDrawImageResized(ScriptMethodParams &params);
	void RawDrawImageTransparent(ScriptMethodParams &params);
	void RawDrawLine(ScriptMethodParams &params);
	void RawDrawRectangle(ScriptMethodParams &params);
	void RawDrawTriangle(ScriptMethodParams &params);
	void ScPl_RawPrint(ScriptMethodParams &params);
	void RawPrintMessageWrapped(ScriptMethodParams &params);
	void RawRestoreScreen(ScriptMethodParams &params);
	void RawRestoreScreenTinted(ScriptMethodParams &params);
	void RawSaveScreen(ScriptMethodParams &params);
	void RawSetColor(ScriptMethodParams &params);
	void RawSetColorRGB(ScriptMethodParams &params);
	void RefreshMouse(ScriptMethodParams &params);
	void ReleaseCharacterView(ScriptMethodParams &params);
	void ReleaseViewport(ScriptMethodParams &params);
	void RemoveObjectTint(ScriptMethodParams &params);
	void RemoveOverlay(ScriptMethodParams &params);
	void RemoveWalkableArea(ScriptMethodParams &params);
	void ResetRoom(ScriptMethodParams &params);
	void restart_game(ScriptMethodParams &params);
	void restore_game_dialog(ScriptMethodParams &params);
	void RestoreGameSlot(ScriptMethodParams &params);
	void RestoreWalkableArea(ScriptMethodParams &params);
	void RunAGSGame(ScriptMethodParams &params);
	void RunCharacterInteraction(ScriptMethodParams &params);
	void RunDialog(ScriptMethodParams &params);
	void RunHotspotInteraction(ScriptMethodParams &params);
	void RunInventoryInteraction(ScriptMethodParams &params);
	void RunObjectInteraction(ScriptMethodParams &params);
	void RunRegionInteraction(ScriptMethodParams &params);
	void Said(ScriptMethodParams &params);
	void SaidUnknownWord(ScriptMethodParams &params);
	void SaveCursorForLocationChange(ScriptMethodParams &params);
	void save_game_dialog(ScriptMethodParams &params);
	void save_game(ScriptMethodParams &params);
	void SaveScreenShot(ScriptMethodParams &params);
	void SeekMIDIPosition(ScriptMethodParams &params);
	void SeekMODPattern(ScriptMethodParams &params);
	void SeekMP3PosMillis(ScriptMethodParams &params);
	void SetActiveInventory(ScriptMethodParams &params);
	void SetAmbientTint(ScriptMethodParams &params);
	void SetAreaLightLevel(ScriptMethodParams &params);
	void SetAreaScaling(ScriptMethodParams &params);
	void SetBackgroundFrame(ScriptMethodParams &params);
	void SetButtonPic(ScriptMethodParams &params);
	void SetButtonText(ScriptMethodParams &params);
	void SetChannelVolume(ScriptMethodParams &params);
	void SetCharacterBaseline(ScriptMethodParams &params);
	void SetCharacterClickable(ScriptMethodParams &params);
	void SetCharacterFrame(ScriptMethodParams &params);
	void SetCharacterIdle(ScriptMethodParams &params);
	void SetCharacterIgnoreLight(ScriptMethodParams &params);
	void SetCharacterIgnoreWalkbehinds(ScriptMethodParams &params);
	void SetCharacterProperty(ScriptMethodParams &params);
	void SetCharacterBlinkView(ScriptMethodParams &params);
	void SetCharacterSpeechView(ScriptMethodParams &params);
	void SetCharacterSpeed(ScriptMethodParams &params);
	void SetCharacterSpeedEx(ScriptMethodParams &params);
	void SetCharacterTransparency(ScriptMethodParams &params);
	void SetCharacterView(ScriptMethodParams &params);
	void SetCharacterViewEx(ScriptMethodParams &params);
	void SetCharacterViewOffset(ScriptMethodParams &params);
	void set_cursor_mode(ScriptMethodParams &params);
	void set_default_cursor(ScriptMethodParams &params);
	void SetDialogOption(ScriptMethodParams &params);
	void SetDigitalMasterVolume(ScriptMethodParams &params);
	void SetFadeColor(ScriptMethodParams &params);
	void SetFrameSound(ScriptMethodParams &params);
	void SetGameOption(ScriptMethodParams &params);
	void SetGameSpeed(ScriptMethodParams &params);
	void SetGlobalInt(ScriptMethodParams &params);
	void SetGlobalString(ScriptMethodParams &params);
	void SetGraphicalVariable(ScriptMethodParams &params);
	void SetGUIBackgroundPic(ScriptMethodParams &params);
	void SetGUIClickable(ScriptMethodParams &params);
	void SetGUIObjectEnabled(ScriptMethodParams &params);
	void SetGUIObjectPosition(ScriptMethodParams &params);
	void SetGUIObjectSize(ScriptMethodParams &params);
	void SetGUIPosition(ScriptMethodParams &params);
	void SetGUISize(ScriptMethodParams &params);
	void SetGUITransparency(ScriptMethodParams &params);
	void SetGUIZOrder(ScriptMethodParams &params);
	void SetInvItemName(ScriptMethodParams &params);
	void set_inv_item_pic(ScriptMethodParams &params);
	void SetInvDimensions(ScriptMethodParams &params);
	void SetLabelColor(ScriptMethodParams &params);
	void SetLabelFont(ScriptMethodParams &params);
	void SetLabelText(ScriptMethodParams &params);
	void SetMouseBounds(ScriptMethodParams &params);
	void set_mouse_cursor(ScriptMethodParams &params);
	void SetMousePosition(ScriptMethodParams &params);
	void SetMultitasking(ScriptMethodParams &params);
	void SetMusicMasterVolume(ScriptMethodParams &params);
	void SetMusicRepeat(ScriptMethodParams &params);
	void SetMusicVolume(ScriptMethodParams &params);
	void SetNextCursor(ScriptMethodParams &params);
	void SetNextScreenTransition(ScriptMethodParams &params);
	void SetNormalFont(ScriptMethodParams &params);
	void SetObjectBaseline(ScriptMethodParams &params);
	void SetObjectClickable(ScriptMethodParams &params);
	void SetObjectFrame(ScriptMethodParams &params);
	void SetObjectGraphic(ScriptMethodParams &params);
	void SetObjectIgnoreWalkbehinds(ScriptMethodParams &params);
	void SetObjectPosition(ScriptMethodParams &params);
	void SetObjectTint(ScriptMethodParams &params);
	void SetObjectTransparency(ScriptMethodParams &params);
	void SetObjectView(ScriptMethodParams &params);
	void SetPalRGB(ScriptMethodParams &params);
	void SetPlayerCharacter(ScriptMethodParams &params);
	void SetRegionTint(ScriptMethodParams &params);
	void SetRestartPoint(ScriptMethodParams &params);
	void SetScreenTransition(ScriptMethodParams &params);
	void SetSkipSpeech(ScriptMethodParams &params);
	void SetSliderValue(ScriptMethodParams &params);
	void SetSoundVolume(ScriptMethodParams &params);
	void SetSpeechFont(ScriptMethodParams &params);
	void SetSpeechStyle(ScriptMethodParams &params);
	void SetSpeechVolume(ScriptMethodParams &params);
	void SetTalkingColor(ScriptMethodParams &params);
	void SetTextBoxFont(ScriptMethodParams &params);
	void SetTextBoxText(ScriptMethodParams &params);
	void ScPl_SetTextOverlay(ScriptMethodParams &params);
	void SetTextWindowGUI(ScriptMethodParams &params);
	void script_SetTimer(ScriptMethodParams &params);
	void SetViewport(ScriptMethodParams &params);
	void SetVoiceMode(ScriptMethodParams &params);
	void SetWalkBehindBase(ScriptMethodParams &params);
	void ShakeScreen(ScriptMethodParams &params);
	void ShakeScreenBackground(ScriptMethodParams &params);
	void ShowMouseCursor(ScriptMethodParams &params);
	void SkipUntilCharacterStops(ScriptMethodParams &params);
	void StartCutscene(ScriptMethodParams &params);
	void scStartRecording(ScriptMethodParams &params);
	void StopAmbientSound(ScriptMethodParams &params);
	void stop_and_destroy_channel(ScriptMethodParams &params);
	void StopDialog(ScriptMethodParams &params);
	void StopMoving(ScriptMethodParams &params);
	void scr_StopMusic(ScriptMethodParams &params);
	void StopObjectMoving(ScriptMethodParams &params);
	void _sc_strcat(ScriptMethodParams &params);
	void ags_stricmp(ScriptMethodParams &params);
	void strcmp(ScriptMethodParams &params);
	void StrContains(ScriptMethodParams &params);
	void _sc_strcpy(ScriptMethodParams &params);
	void ScPl_sc_sprintf(ScriptMethodParams &params);
	void StrGetCharAt(ScriptMethodParams &params);
	void StringToInt(ScriptMethodParams &params);
	void strlen(ScriptMethodParams &params);
	void StrSetCharAt(ScriptMethodParams &params);
	void _sc_strlower(ScriptMethodParams &params);
	void _sc_strupper(ScriptMethodParams &params);
	void TintScreen(ScriptMethodParams &params);
	void UnPauseGame(ScriptMethodParams &params);
	void update_invorder(ScriptMethodParams &params);
	void UpdatePalette(ScriptMethodParams &params);
	void scrWait(ScriptMethodParams &params);
	void WaitKey(ScriptMethodParams &params);
	void WaitMouseKey(ScriptMethodParams &params);
	void WaitInput(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
