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

#include "ags/plugins/core/global_api.h"
#include "ags/engine/ac/cd_audio.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_button.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_debug.h"
#include "ags/engine/ac/global_dialog.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_drawing_surface.h"
#include "ags/engine/ac/global_file.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_hotspot.h"
#include "ags/engine/ac/global_inventory_item.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_overlay.h"
#include "ags/engine/ac/global_palette.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/global_string.h"
#include "ags/engine/ac/math.h"
#include "ags/engine/ac/mouse.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void GlobalAPI::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(AbortGame, ScPl_sc_AbortGame);
	SCRIPT_METHOD_EXT(AddInventory, add_inventory);
	SCRIPT_METHOD_EXT(AddInventoryToCharacter, AddInventoryToCharacter);
	SCRIPT_METHOD_EXT(AnimateButton, AnimateButton);
	SCRIPT_METHOD_EXT(AnimateCharacter, scAnimateCharacter);
	SCRIPT_METHOD_EXT(AnimateCharacterEx, AnimateCharacterEx);
	SCRIPT_METHOD_EXT(AnimateObject, AnimateObject);
	SCRIPT_METHOD_EXT(AnimateObjectEx, AnimateObjectEx);
	SCRIPT_METHOD_EXT(AreCharactersColliding, AreCharactersColliding);
	SCRIPT_METHOD_EXT(AreCharObjColliding, AreCharObjColliding);
	SCRIPT_METHOD_EXT(AreObjectsColliding, AreObjectsColliding);
	SCRIPT_METHOD_EXT(AreThingsOverlapping, AreThingsOverlapping);
	SCRIPT_METHOD_EXT(CallRoomScript, CallRoomScript);
	SCRIPT_METHOD_EXT(CDAudio, cd_manager);
	SCRIPT_METHOD_EXT(CentreGUI, CentreGUI);
	SCRIPT_METHOD_EXT(ChangeCharacterView, ChangeCharacterView);
	SCRIPT_METHOD_EXT(ChangeCursorGraphic, ChangeCursorGraphic);
	SCRIPT_METHOD_EXT(ChangeCursorHotspot, ChangeCursorHotspot);
	SCRIPT_METHOD_EXT(ClaimEvent, ClaimEvent);
	SCRIPT_METHOD_EXT(CreateGraphicOverlay, CreateGraphicOverlay);
	SCRIPT_METHOD_EXT(CreateTextOverlay, ScPl_CreateTextOverlay);
	SCRIPT_METHOD_EXT(CyclePalette, CyclePalette);
	SCRIPT_METHOD_EXT(Debug, script_debug);
	SCRIPT_METHOD_EXT(DeleteSaveSlot, DeleteSaveSlot);
	SCRIPT_METHOD_EXT(DeleteSprite, free_dynamic_sprite);
	SCRIPT_METHOD_EXT(DisableCursorMode, disable_cursor_mode);
	SCRIPT_METHOD_EXT(DisableGroundLevelAreas, DisableGroundLevelAreas);
	SCRIPT_METHOD_EXT(DisableHotspot, DisableHotspot);
	SCRIPT_METHOD_EXT(DisableInterface, DisableInterface);
	SCRIPT_METHOD_EXT(DisableRegion, DisableRegion);
	SCRIPT_METHOD_EXT(Display, ScPl_Display);
	SCRIPT_METHOD_EXT(DisplayAt, ScPl_DisplayAt);
	SCRIPT_METHOD_EXT(DisplayAtY, DisplayAtY);
	SCRIPT_METHOD_EXT(DisplayMessage, DisplayMessage);
	SCRIPT_METHOD_EXT(DisplayMessageAtY, DisplayMessageAtY);
	SCRIPT_METHOD_EXT(DisplayMessageBar, DisplayMessageBar);
	SCRIPT_METHOD_EXT(DisplaySpeech, ScPl_sc_displayspeech);
	SCRIPT_METHOD_EXT(DisplaySpeechAt, DisplaySpeechAt);
	SCRIPT_METHOD_EXT(DisplaySpeechBackground, DisplaySpeechBackground);
	SCRIPT_METHOD_EXT(DisplayThought, ScPl_DisplayThought);
	SCRIPT_METHOD_EXT(DisplayTopBar, ScPl_DisplayTopBar);
	SCRIPT_METHOD_EXT(EnableCursorMode, enable_cursor_mode);
	SCRIPT_METHOD_EXT(EnableGroundLevelAreas, EnableGroundLevelAreas);
	SCRIPT_METHOD_EXT(EnableHotspot, EnableHotspot);
	SCRIPT_METHOD_EXT(EnableInterface, EnableInterface);
	SCRIPT_METHOD_EXT(EnableRegion, EnableRegion);
	SCRIPT_METHOD_EXT(EndCutscene, EndCutscene);
	SCRIPT_METHOD_EXT(FaceCharacter, FaceCharacter);
	SCRIPT_METHOD_EXT(FaceLocation, FaceLocation);
	SCRIPT_METHOD_EXT(FadeIn, FadeIn);
	SCRIPT_METHOD_EXT(FadeOut, my_fade_out);
	SCRIPT_METHOD_EXT(FileClose, FileClose);
	SCRIPT_METHOD_EXT(FileIsEOF, FileIsEOF);
	SCRIPT_METHOD_EXT(FileIsError, FileIsError);
	// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
	SCRIPT_METHOD_EXT(FileOpen, FileOpenCMode);
	SCRIPT_METHOD_EXT(FileRead, FileRead);
	SCRIPT_METHOD_EXT(FileReadInt, FileReadInt);
	SCRIPT_METHOD_EXT(FileReadRawChar, FileReadRawChar);
	SCRIPT_METHOD_EXT(FileReadRawInt, FileReadRawInt);
	SCRIPT_METHOD_EXT(FileWrite, FileWrite);
	SCRIPT_METHOD_EXT(FileWriteInt, FileWriteInt);
	SCRIPT_METHOD_EXT(FileWriteRawChar, FileWriteRawChar);
	SCRIPT_METHOD_EXT(FileWriteRawLine, FileWriteRawLine);
	SCRIPT_METHOD_EXT(FindGUIID, FindGUIID);
	SCRIPT_METHOD_EXT(FlipScreen, FlipScreen);
	SCRIPT_METHOD_EXT(FloatToInt, FloatToInt);
	SCRIPT_METHOD_EXT(FollowCharacter, FollowCharacter);
	SCRIPT_METHOD_EXT(FollowCharacterEx, FollowCharacterEx);
	SCRIPT_METHOD_EXT(GetBackgroundFrame, GetBackgroundFrame);
	SCRIPT_METHOD_EXT(GetButtonPic, GetButtonPic);
	SCRIPT_METHOD_EXT(GetCharacterAt, GetCharIDAtScreen);
	SCRIPT_METHOD_EXT(GetCharacterProperty, GetCharacterProperty);
	SCRIPT_METHOD_EXT(GetCharacterPropertyText, GetCharacterPropertyText);
	SCRIPT_METHOD_EXT(GetCurrentMusic, GetCurrentMusic);
	SCRIPT_METHOD_EXT(GetCursorMode, GetCursorMode);
	SCRIPT_METHOD_EXT(GetDialogOption, GetDialogOption);
	SCRIPT_METHOD_EXT(GetGameOption, GetGameOption);
	SCRIPT_METHOD_EXT(GetGameParameter, GetGameParameter);
	SCRIPT_METHOD_EXT(GetGameSpeed, GetGameSpeed);
	SCRIPT_METHOD_EXT(GetGlobalInt, GetGlobalInt);
	SCRIPT_METHOD_EXT(GetGlobalString, GetGlobalString);
	SCRIPT_METHOD_EXT(GetGraphicalVariable, GetGraphicalVariable);
	SCRIPT_METHOD_EXT(GetGUIAt, GetGUIAt);
	SCRIPT_METHOD_EXT(GetGUIObjectAt, GetGUIObjectAt);
	SCRIPT_METHOD_EXT(GetHotspotAt, GetHotspotIDAtScreen);
	SCRIPT_METHOD_EXT(GetHotspotName, GetHotspotName);
	SCRIPT_METHOD_EXT(GetHotspotPointX, GetHotspotPointX);
	SCRIPT_METHOD_EXT(GetHotspotPointY, GetHotspotPointY);
	SCRIPT_METHOD_EXT(GetHotspotProperty, GetHotspotProperty);
	SCRIPT_METHOD_EXT(GetHotspotPropertyText, GetHotspotPropertyText);
	SCRIPT_METHOD_EXT(GetInvAt, GetInvAt);
	SCRIPT_METHOD_EXT(GetInvGraphic, GetInvGraphic);
	SCRIPT_METHOD_EXT(GetInvName, GetInvName);
	SCRIPT_METHOD_EXT(GetInvProperty, GetInvProperty);
	SCRIPT_METHOD_EXT(GetInvPropertyText, GetInvPropertyText);
	//SCRIPT_METHOD_EXT(GetLanguageString",      (void*)GetLanguageString);
	SCRIPT_METHOD_EXT(GetLocationName, GetLocationName);
	SCRIPT_METHOD_EXT(GetLocationType, GetLocationType);
	SCRIPT_METHOD_EXT(GetMessageText, GetMessageText);
	SCRIPT_METHOD_EXT(GetMIDIPosition, GetMIDIPosition);
	SCRIPT_METHOD_EXT(GetMP3PosMillis, GetMP3PosMillis);
	SCRIPT_METHOD_EXT(GetObjectAt, GetObjectIDAtScreen);
	SCRIPT_METHOD_EXT(GetObjectBaseline, GetObjectBaseline);
	SCRIPT_METHOD_EXT(GetObjectGraphic, GetObjectGraphic);
	SCRIPT_METHOD_EXT(GetObjectName, GetObjectName);
	SCRIPT_METHOD_EXT(GetObjectProperty, GetObjectProperty);
	SCRIPT_METHOD_EXT(GetObjectPropertyText, GetObjectPropertyText);
	SCRIPT_METHOD_EXT(GetObjectX, GetObjectX);
	SCRIPT_METHOD_EXT(GetObjectY, GetObjectY);
	//  SCRIPT_METHOD_EXT(GetPalette",           (void*)scGetPal);
	SCRIPT_METHOD_EXT(GetPlayerCharacter, GetPlayerCharacter);
	SCRIPT_METHOD_EXT(GetRawTime, GetRawTime);
	SCRIPT_METHOD_EXT(GetRegionAt, GetRegionIDAtRoom);
	SCRIPT_METHOD_EXT(GetRoomProperty, Room_GetProperty);
	SCRIPT_METHOD_EXT(GetRoomPropertyText, GetRoomPropertyText);
	SCRIPT_METHOD_EXT(GetSaveSlotDescription, GetSaveSlotDescription);
	SCRIPT_METHOD_EXT(GetScalingAt, GetScalingAt);
	SCRIPT_METHOD_EXT(GetSliderValue, GetSliderValue);
	SCRIPT_METHOD_EXT(GetTextBoxText, GetTextBoxText);
	SCRIPT_METHOD_EXT(GetTextHeight, GetTextHeight);
	SCRIPT_METHOD_EXT(GetTextWidth, GetTextWidth);
	SCRIPT_METHOD_EXT(GetTime, sc_GetTime);
	SCRIPT_METHOD_EXT(GetTranslation, get_translation);
	SCRIPT_METHOD_EXT(GetTranslationName, GetTranslationName);
	SCRIPT_METHOD_EXT(GetViewportX, GetViewportX);
	SCRIPT_METHOD_EXT(GetViewportY, GetViewportY);
	SCRIPT_METHOD_EXT(GetWalkableAreaAtRoom, GetWalkableAreaAtRoom);
	SCRIPT_METHOD_EXT(GetWalkableAreaAt, GetWalkableAreaAtScreen);
	SCRIPT_METHOD_EXT(GetWalkableAreaAtScreen, GetWalkableAreaAtScreen);
	SCRIPT_METHOD_EXT(GiveScore, GiveScore);
	SCRIPT_METHOD_EXT(HasPlayerBeenInRoom, HasPlayerBeenInRoom);
	SCRIPT_METHOD_EXT(HideMouseCursor, HideMouseCursor);
	SCRIPT_METHOD_EXT(InputBox, sc_inputbox);
	SCRIPT_METHOD_EXT(InterfaceOff, InterfaceOff);
	SCRIPT_METHOD_EXT(InterfaceOn, InterfaceOn);
	SCRIPT_METHOD_EXT(IntToFloat, IntToFloat);
	SCRIPT_METHOD_EXT(InventoryScreen, sc_invscreen);
	SCRIPT_METHOD_EXT(IsButtonDown, IsButtonDown);
	SCRIPT_METHOD_EXT(IsChannelPlaying, IsChannelPlaying);
	SCRIPT_METHOD_EXT(IsGamePaused, IsGamePaused);
	SCRIPT_METHOD_EXT(IsGUIOn, IsGUIOn);
	SCRIPT_METHOD_EXT(IsInteractionAvailable, IsInteractionAvailable);
	SCRIPT_METHOD_EXT(IsInventoryInteractionAvailable, IsInventoryInteractionAvailable);
	SCRIPT_METHOD_EXT(IsInterfaceEnabled, IsInterfaceEnabled);
	SCRIPT_METHOD_EXT(IsKeyPressed, IsKeyPressed);
	SCRIPT_METHOD_EXT(IsMusicPlaying, IsMusicPlaying);
	SCRIPT_METHOD_EXT(IsMusicVoxAvailable, IsMusicVoxAvailable);
	SCRIPT_METHOD_EXT(IsObjectAnimating, IsObjectAnimating);
	SCRIPT_METHOD_EXT(IsObjectMoving, IsObjectMoving);
	SCRIPT_METHOD_EXT(IsObjectOn, IsObjectOn);
	SCRIPT_METHOD_EXT(IsOverlayValid, IsOverlayValid);
	SCRIPT_METHOD_EXT(IsSoundPlaying, IsSoundPlaying);
	SCRIPT_METHOD_EXT(IsTimerExpired, IsTimerExpired);
	SCRIPT_METHOD_EXT(IsTranslationAvailable, IsTranslationAvailable);
	SCRIPT_METHOD_EXT(IsVoxAvailable, IsVoxAvailable);
	SCRIPT_METHOD_EXT(ListBoxAdd, ListBoxAdd);
	SCRIPT_METHOD_EXT(ListBoxClear, ListBoxClear);
	SCRIPT_METHOD_EXT(ListBoxDirList, ListBoxDirList);
	SCRIPT_METHOD_EXT(ListBoxGetItemText, ListBoxGetItemText);
	SCRIPT_METHOD_EXT(ListBoxGetNumItems, ListBoxGetNumItems);
	SCRIPT_METHOD_EXT(ListBoxGetSelected, ListBoxGetSelected);
	SCRIPT_METHOD_EXT(ListBoxRemove, ListBoxRemove);
	SCRIPT_METHOD_EXT(ListBoxSaveGameList, ListBoxSaveGameList);
	SCRIPT_METHOD_EXT(ListBoxSetSelected, ListBoxSetSelected);
	SCRIPT_METHOD_EXT(ListBoxSetTopItem, ListBoxSetTopItem);
	SCRIPT_METHOD_EXT(LoadImageFile, LoadImageFile);
	SCRIPT_METHOD_EXT(LoadSaveSlotScreenshot, LoadSaveSlotScreenshot);
	SCRIPT_METHOD_EXT(LoseInventory, lose_inventory);
	SCRIPT_METHOD_EXT(LoseInventoryFromCharacter, LoseInventoryFromCharacter);
	SCRIPT_METHOD_EXT(MergeObject, MergeObject);
	SCRIPT_METHOD_EXT(MoveCharacter, MoveCharacter);
	SCRIPT_METHOD_EXT(MoveCharacterBlocking, MoveCharacterBlocking);
	SCRIPT_METHOD_EXT(MoveCharacterDirect, MoveCharacterDirect);
	SCRIPT_METHOD_EXT(MoveCharacterPath, MoveCharacterPath);
	SCRIPT_METHOD_EXT(MoveCharacterStraight, MoveCharacterStraight);
	SCRIPT_METHOD_EXT(MoveCharacterToHotspot, MoveCharacterToHotspot);
	SCRIPT_METHOD_EXT(MoveCharacterToObject, MoveCharacterToObject);
	SCRIPT_METHOD_EXT(MoveObject, MoveObject);
	SCRIPT_METHOD_EXT(MoveObjectDirect, MoveObjectDirect);
	SCRIPT_METHOD_EXT(MoveOverlay, MoveOverlay);
	SCRIPT_METHOD_EXT(MoveToWalkableArea, MoveToWalkableArea);
	SCRIPT_METHOD_EXT(NewRoom, NewRoom);
	SCRIPT_METHOD_EXT(NewRoomEx, NewRoomEx);
	SCRIPT_METHOD_EXT(NewRoomNPC, NewRoomNPC);
	SCRIPT_METHOD_EXT(ObjectOff, ObjectOff);
	SCRIPT_METHOD_EXT(ObjectOn, ObjectOn);
	SCRIPT_METHOD_EXT(ParseText, ParseText);
	SCRIPT_METHOD_EXT(PauseGame, PauseGame);
	SCRIPT_METHOD_EXT(PlayAmbientSound, PlayAmbientSound);
	SCRIPT_METHOD_EXT(PlayFlic, play_flc_file);
	SCRIPT_METHOD_EXT(PlayMP3File, PlayMP3File);
	SCRIPT_METHOD_EXT(PlayMusic, PlayMusicResetQueue);
	SCRIPT_METHOD_EXT(PlayMusicQueued, PlayMusicQueued);
	SCRIPT_METHOD_EXT(PlaySilentMIDI, PlaySilentMIDI);
	SCRIPT_METHOD_EXT(PlaySound, play_sound);
	SCRIPT_METHOD_EXT(PlaySoundEx, PlaySoundEx);
	SCRIPT_METHOD_EXT(PlayVideo, scrPlayVideo);
	SCRIPT_METHOD_EXT(ProcessClick, RoomProcessClick);
	SCRIPT_METHOD_EXT(QuitGame, QuitGame);
	SCRIPT_METHOD_EXT(Random, __Rand);
	SCRIPT_METHOD_EXT(RawClearScreen, RawClear);
	SCRIPT_METHOD_EXT(RawDrawCircle, RawDrawCircle);
	SCRIPT_METHOD_EXT(RawDrawFrameTransparent, RawDrawFrameTransparent);
	SCRIPT_METHOD_EXT(RawDrawImage, RawDrawImage);
	SCRIPT_METHOD_EXT(RawDrawImageOffset, RawDrawImageOffset);
	SCRIPT_METHOD_EXT(RawDrawImageResized, RawDrawImageResized);
	SCRIPT_METHOD_EXT(RawDrawImageTransparent, RawDrawImageTransparent);
	SCRIPT_METHOD_EXT(RawDrawLine, RawDrawLine);
	SCRIPT_METHOD_EXT(RawDrawRectangle, RawDrawRectangle);
	SCRIPT_METHOD_EXT(RawDrawTriangle, RawDrawTriangle);
	SCRIPT_METHOD_EXT(RawPrint, ScPl_RawPrint);
	SCRIPT_METHOD_EXT(RawPrintMessageWrapped, RawPrintMessageWrapped);
	SCRIPT_METHOD_EXT(RawRestoreScreen, RawRestoreScreen);
	SCRIPT_METHOD_EXT(RawRestoreScreenTinted, RawRestoreScreenTinted);
	SCRIPT_METHOD_EXT(RawSaveScreen, RawSaveScreen);
	SCRIPT_METHOD_EXT(RawSetColor, RawSetColor);
	SCRIPT_METHOD_EXT(RawSetColorRGB, RawSetColorRGB);
	SCRIPT_METHOD_EXT(RefreshMouse, RefreshMouse);
	SCRIPT_METHOD_EXT(ReleaseCharacterView, ReleaseCharacterView);
	SCRIPT_METHOD_EXT(ReleaseViewport, ReleaseViewport);
	SCRIPT_METHOD_EXT(RemoveObjectTint, RemoveObjectTint);
	SCRIPT_METHOD_EXT(RemoveOverlay, RemoveOverlay);
	SCRIPT_METHOD_EXT(RemoveWalkableArea, RemoveWalkableArea);
	SCRIPT_METHOD_EXT(ResetRoom, ResetRoom);
	SCRIPT_METHOD_EXT(RestartGame, restart_game);
	SCRIPT_METHOD_EXT(RestoreGameDialog, restore_game_dialog);
	SCRIPT_METHOD_EXT(RestoreGameSlot, RestoreGameSlot);
	SCRIPT_METHOD_EXT(RestoreWalkableArea, RestoreWalkableArea);
	SCRIPT_METHOD_EXT(RunAGSGame, RunAGSGame);
	SCRIPT_METHOD_EXT(RunCharacterInteraction, RunCharacterInteraction);
	SCRIPT_METHOD_EXT(RunDialog, RunDialog);
	SCRIPT_METHOD_EXT(RunHotspotInteraction, RunHotspotInteraction);
	SCRIPT_METHOD_EXT(RunInventoryInteraction, RunInventoryInteraction);
	SCRIPT_METHOD_EXT(RunObjectInteraction, RunObjectInteraction);
	SCRIPT_METHOD_EXT(RunRegionInteraction, RunRegionInteraction);
	SCRIPT_METHOD_EXT(Said, Said);
	SCRIPT_METHOD_EXT(SaidUnknownWord, SaidUnknownWord);
	SCRIPT_METHOD_EXT(SaveCursorForLocationChange, SaveCursorForLocationChange);
	SCRIPT_METHOD_EXT(SaveGameDialog, save_game_dialog);
	SCRIPT_METHOD_EXT(SaveGameSlot, save_game);
	SCRIPT_METHOD_EXT(SaveScreenShot, SaveScreenShot);
	SCRIPT_METHOD_EXT(SeekMIDIPosition, SeekMIDIPosition);
	SCRIPT_METHOD_EXT(SeekMODPattern, SeekMODPattern);
	SCRIPT_METHOD_EXT(SeekMP3PosMillis, SeekMP3PosMillis);
	SCRIPT_METHOD_EXT(SetActiveInventory, SetActiveInventory);
	SCRIPT_METHOD_EXT(SetAmbientTint, SetAmbientTint);
	SCRIPT_METHOD_EXT(SetAreaLightLevel, SetAreaLightLevel);
	SCRIPT_METHOD_EXT(SetAreaScaling, SetAreaScaling);
	SCRIPT_METHOD_EXT(SetBackgroundFrame, SetBackgroundFrame);
	SCRIPT_METHOD_EXT(SetButtonPic, SetButtonPic);
	SCRIPT_METHOD_EXT(SetButtonText, SetButtonText);
	SCRIPT_METHOD_EXT(SetChannelVolume, SetChannelVolume);
	SCRIPT_METHOD_EXT(SetCharacterBaseline, SetCharacterBaseline);
	SCRIPT_METHOD_EXT(SetCharacterClickable, SetCharacterClickable);
	SCRIPT_METHOD_EXT(SetCharacterFrame, SetCharacterFrame);
	SCRIPT_METHOD_EXT(SetCharacterIdle, SetCharacterIdle);
	SCRIPT_METHOD_EXT(SetCharacterIgnoreLight, SetCharacterIgnoreLight);
	SCRIPT_METHOD_EXT(SetCharacterIgnoreWalkbehinds, SetCharacterIgnoreWalkbehinds);
	SCRIPT_METHOD_EXT(SetCharacterProperty, SetCharacterProperty);
	SCRIPT_METHOD_EXT(SetCharacterBlinkView, SetCharacterBlinkView);
	SCRIPT_METHOD_EXT(SetCharacterSpeechView, SetCharacterSpeechView);
	SCRIPT_METHOD_EXT(SetCharacterSpeed, SetCharacterSpeed);
	SCRIPT_METHOD_EXT(SetCharacterSpeedEx, SetCharacterSpeedEx);
	SCRIPT_METHOD_EXT(SetCharacterTransparency, SetCharacterTransparency);
	SCRIPT_METHOD_EXT(SetCharacterView, SetCharacterView);
	SCRIPT_METHOD_EXT(SetCharacterViewEx, SetCharacterViewEx);
	SCRIPT_METHOD_EXT(SetCharacterViewOffset, SetCharacterViewOffset);
	SCRIPT_METHOD_EXT(SetCursorMode, set_cursor_mode);
	SCRIPT_METHOD_EXT(SetDefaultCursor, set_default_cursor);
	SCRIPT_METHOD_EXT(SetDialogOption, SetDialogOption);
	SCRIPT_METHOD_EXT(SetDigitalMasterVolume, SetDigitalMasterVolume);
	SCRIPT_METHOD_EXT(SetFadeColor, SetFadeColor);
	SCRIPT_METHOD_EXT(SetFrameSound, SetFrameSound);
	SCRIPT_METHOD_EXT(SetGameOption, SetGameOption);
	SCRIPT_METHOD_EXT(SetGameSpeed, SetGameSpeed);
	SCRIPT_METHOD_EXT(SetGlobalInt, SetGlobalInt);
	SCRIPT_METHOD_EXT(SetGlobalString, SetGlobalString);
	SCRIPT_METHOD_EXT(SetGraphicalVariable, SetGraphicalVariable);
	SCRIPT_METHOD_EXT(SetGUIBackgroundPic, SetGUIBackgroundPic);
	SCRIPT_METHOD_EXT(SetGUIClickable, SetGUIClickable);
	SCRIPT_METHOD_EXT(SetGUIObjectEnabled, SetGUIObjectEnabled);
	SCRIPT_METHOD_EXT(SetGUIObjectPosition, SetGUIObjectPosition);
	SCRIPT_METHOD_EXT(SetGUIObjectSize, SetGUIObjectSize);
	SCRIPT_METHOD_EXT(SetGUIPosition, SetGUIPosition);
	SCRIPT_METHOD_EXT(SetGUISize, SetGUISize);
	SCRIPT_METHOD_EXT(SetGUITransparency, SetGUITransparency);
	SCRIPT_METHOD_EXT(SetGUIZOrder, SetGUIZOrder);
	SCRIPT_METHOD_EXT(SetInvItemName, SetInvItemName);
	SCRIPT_METHOD_EXT(SetInvItemPic, set_inv_item_pic);
	SCRIPT_METHOD_EXT(SetInvDimensions, SetInvDimensions);
	SCRIPT_METHOD_EXT(SetLabelColor, SetLabelColor);
	SCRIPT_METHOD_EXT(SetLabelFont, SetLabelFont);
	SCRIPT_METHOD_EXT(SetLabelText, SetLabelText);
	SCRIPT_METHOD_EXT(SetMouseBounds, SetMouseBounds);
	SCRIPT_METHOD_EXT(SetMouseCursor, set_mouse_cursor);
	SCRIPT_METHOD_EXT(SetMousePosition, SetMousePosition);
	SCRIPT_METHOD_EXT(SetMultitaskingMode, SetMultitasking);
	SCRIPT_METHOD_EXT(SetMusicMasterVolume, SetMusicMasterVolume);
	SCRIPT_METHOD_EXT(SetMusicRepeat, SetMusicRepeat);
	SCRIPT_METHOD_EXT(SetMusicVolume, SetMusicVolume);
	SCRIPT_METHOD_EXT(SetNextCursorMode, SetNextCursor);
	SCRIPT_METHOD_EXT(SetNextScreenTransition, SetNextScreenTransition);
	SCRIPT_METHOD_EXT(SetNormalFont, SetNormalFont);
	SCRIPT_METHOD_EXT(SetObjectBaseline, SetObjectBaseline);
	SCRIPT_METHOD_EXT(SetObjectClickable, SetObjectClickable);
	SCRIPT_METHOD_EXT(SetObjectFrame, SetObjectFrame);
	SCRIPT_METHOD_EXT(SetObjectGraphic, SetObjectGraphic);
	SCRIPT_METHOD_EXT(SetObjectIgnoreWalkbehinds, SetObjectIgnoreWalkbehinds);
	SCRIPT_METHOD_EXT(SetObjectPosition, SetObjectPosition);
	SCRIPT_METHOD_EXT(SetObjectTint, SetObjectTint);
	SCRIPT_METHOD_EXT(SetObjectTransparency, SetObjectTransparency);
	SCRIPT_METHOD_EXT(SetObjectView, SetObjectView);
	//  SCRIPT_METHOD_EXT(SetPalette",           (void*)scSetPal);
	SCRIPT_METHOD_EXT(SetPalRGB, SetPalRGB);
	SCRIPT_METHOD_EXT(SetPlayerCharacter, SetPlayerCharacter);
	SCRIPT_METHOD_EXT(SetRegionTint, SetRegionTint);
	SCRIPT_METHOD_EXT(SetRestartPoint, SetRestartPoint);
	SCRIPT_METHOD_EXT(SetScreenTransition, SetScreenTransition);
	SCRIPT_METHOD_EXT(SetSkipSpeech, SetSkipSpeech);
	SCRIPT_METHOD_EXT(SetSliderValue, SetSliderValue);
	SCRIPT_METHOD_EXT(SetSoundVolume, SetSoundVolume);
	SCRIPT_METHOD_EXT(SetSpeechFont, SetSpeechFont);
	SCRIPT_METHOD_EXT(SetSpeechStyle, SetSpeechStyle);
	SCRIPT_METHOD_EXT(SetSpeechVolume, SetSpeechVolume);
	SCRIPT_METHOD_EXT(SetTalkingColor, SetTalkingColor);
	SCRIPT_METHOD_EXT(SetTextBoxFont, SetTextBoxFont);
	SCRIPT_METHOD_EXT(SetTextBoxText, SetTextBoxText);
	SCRIPT_METHOD_EXT(SetTextOverlay, ScPl_SetTextOverlay);
	SCRIPT_METHOD_EXT(SetTextWindowGUI, SetTextWindowGUI);
	SCRIPT_METHOD_EXT(SetTimer, script_SetTimer);
	SCRIPT_METHOD_EXT(SetViewport, SetViewport);
	SCRIPT_METHOD_EXT(SetVoiceMode, SetVoiceMode);
	SCRIPT_METHOD_EXT(SetWalkBehindBase, SetWalkBehindBase);
	SCRIPT_METHOD_EXT(ShakeScreen, ShakeScreen);
	SCRIPT_METHOD_EXT(ShakeScreenBackground, ShakeScreenBackground);
	SCRIPT_METHOD_EXT(ShowMouseCursor, ShowMouseCursor);
	SCRIPT_METHOD_EXT(SkipUntilCharacterStops, SkipUntilCharacterStops);
	SCRIPT_METHOD_EXT(StartCutscene, StartCutscene);
	SCRIPT_METHOD_EXT(StartRecording, scStartRecording);
	SCRIPT_METHOD_EXT(StopAmbientSound, StopAmbientSound);
	SCRIPT_METHOD_EXT(StopChannel, stop_and_destroy_channel);
	SCRIPT_METHOD_EXT(StopDialog, StopDialog);
	SCRIPT_METHOD_EXT(StopMoving, StopMoving);
	SCRIPT_METHOD_EXT(StopMusic, scr_StopMusic);
	SCRIPT_METHOD_EXT(StopObjectMoving, StopObjectMoving);
	SCRIPT_METHOD_EXT(StrCat, _sc_strcat);
	SCRIPT_METHOD_EXT(StrCaseComp, ags_stricmp);
	SCRIPT_METHOD_EXT(StrComp, strcmp);
	SCRIPT_METHOD_EXT(StrContains, StrContains);
	SCRIPT_METHOD_EXT(StrCopy, _sc_strcpy);
	SCRIPT_METHOD_EXT(StrFormat, ScPl_sc_sprintf);
	SCRIPT_METHOD_EXT(StrGetCharAt, StrGetCharAt);
	SCRIPT_METHOD_EXT(StringToInt, StringToInt);
	SCRIPT_METHOD_EXT(StrLen, strlen);
	SCRIPT_METHOD_EXT(StrSetCharAt, StrSetCharAt);
	SCRIPT_METHOD_EXT(StrToLowerCase, _sc_strlower);
	SCRIPT_METHOD_EXT(StrToUpperCase, _sc_strupper);
	SCRIPT_METHOD_EXT(TintScreen, TintScreen);
	SCRIPT_METHOD_EXT(UnPauseGame, UnPauseGame);
	SCRIPT_METHOD_EXT(UpdateInventory, update_invorder);
	SCRIPT_METHOD_EXT(UpdatePalette, UpdatePalette);
	SCRIPT_METHOD_EXT(Wait, scrWait);
	SCRIPT_METHOD_EXT(WaitKey, WaitKey);
	SCRIPT_METHOD_EXT(WaitMouseKey, WaitMouseKey);

}

void GlobalAPI::ScPl_sc_AbortGame(ScriptMethodParams &params) {
	Common::String texx = params.format(0);
	AGS3::_sc_AbortGame(texx.c_str());
}

void GlobalAPI::add_inventory(ScriptMethodParams &params) {
	PARAMS1(int, inum);
	AGS3::add_inventory(inum);
}

void GlobalAPI::AddInventoryToCharacter(ScriptMethodParams &params) {
	PARAMS2(int, charid, int, inum);
	AGS3::AddInventoryToCharacter(charid, inum);
}

void GlobalAPI::AnimateButton(ScriptMethodParams &params) {
	PARAMS6(int, guin, int, objn, int, view, int, loop, int, speed, int, repeat);
	AGS3::AnimateButton(guin, objn, view, loop, speed, repeat);
}

void GlobalAPI::scAnimateCharacter(ScriptMethodParams &params) {
	PARAMS4(int, chh, int, loopn, int, sppd, int, rept);
	AGS3::scAnimateCharacter(chh, loopn, sppd, rept);
}

void GlobalAPI::AnimateCharacterEx(ScriptMethodParams &params) {
	PARAMS6(int, chh, int, loopn, int, sppd, int, rept, int, direction, int, blocking);
	AGS3::AnimateCharacterEx(chh, loopn, sppd, rept, direction, blocking);
}

void GlobalAPI::AnimateObject(ScriptMethodParams &params) {
	PARAMS4(int, obn, int, loopn, int, spdd, int, rept);
	AGS3::AnimateObject(obn, loopn, spdd, rept);
}

void GlobalAPI::AnimateObjectEx(ScriptMethodParams &params) {
	PARAMS6(int, obn, int, loopn, int, spdd, int, rept, int, direction, int, blocking);
	AGS3::AnimateObjectEx(obn, loopn, spdd, rept, direction, blocking);
}

void GlobalAPI::AreCharactersColliding(ScriptMethodParams &params) {
	PARAMS2(int, cchar1, int, cchar2);
	params._result = AGS3::AreCharactersColliding(cchar1, cchar2);
}

void GlobalAPI::AreCharObjColliding(ScriptMethodParams &params) {
	PARAMS2(int, charid, int, objid);
	params._result = AGS3::AreCharObjColliding(charid, objid);
}

void GlobalAPI::AreObjectsColliding(ScriptMethodParams &params) {
	PARAMS2(int, obj1, int, obj2);
	params._result = AGS3::AreObjectsColliding(obj1, obj2);
}

void GlobalAPI::AreThingsOverlapping(ScriptMethodParams &params) {
	PARAMS2(int, thing1, int, thing2);
	params._result = AGS3::AreThingsOverlapping(thing1, thing2);
}

void GlobalAPI::CallRoomScript(ScriptMethodParams &params) {
	PARAMS1(int, value);
	AGS3::CallRoomScript(value);
}

void GlobalAPI::cd_manager(ScriptMethodParams &params) {
	PARAMS2(int, cmdd, int, datt);
	params._result = AGS3::cd_manager(cmdd, datt);
}

void GlobalAPI::CentreGUI(ScriptMethodParams &params) {
	PARAMS1(int, ifn);
	AGS3::CentreGUI(ifn);
}

void GlobalAPI::ChangeCharacterView(ScriptMethodParams &params) {
	PARAMS2(int, chaa, int, vii);
	AGS3::ChangeCharacterView(chaa, vii);
}

void GlobalAPI::ChangeCursorGraphic(ScriptMethodParams &params) {
	PARAMS2(int, curs, int, newslot);
	AGS3::ChangeCursorGraphic(curs, newslot);
}

void GlobalAPI::ChangeCursorHotspot(ScriptMethodParams &params) {
	PARAMS3(int, curs, int, x, int, y);
	AGS3::ChangeCursorHotspot(curs, x, y);
}

void GlobalAPI::ClaimEvent(ScriptMethodParams &params) {
	AGS3::ClaimEvent();
}

void GlobalAPI::CreateGraphicOverlay(ScriptMethodParams &params) {
	PARAMS4(int, xx, int, yy, int, slott, int, trans);
	params._result = AGS3::CreateGraphicOverlay(xx, yy, slott, trans);
}

void GlobalAPI::ScPl_CreateTextOverlay(ScriptMethodParams &params) {
	PARAMS5(int, xx, int, yy, int, wii, int, fontid, int, clr);
	Common::String texx = params.format(5);

	params._result = AGS3::CreateTextOverlay(xx, yy, wii, fontid, clr,
		texx.c_str(), DISPLAYTEXT_NORMALOVERLAY);
}

void GlobalAPI::CyclePalette(ScriptMethodParams &params) {
	PARAMS2(int, strt, int, eend);
	AGS3::CyclePalette(strt, eend);
}

void GlobalAPI::script_debug(ScriptMethodParams &params) {
	PARAMS2(int, cmdd, int, dataa);
	AGS3::script_debug(cmdd, dataa);
}

void GlobalAPI::DeleteSaveSlot(ScriptMethodParams &params) {
	PARAMS1(int, slnum);
	AGS3::DeleteSaveSlot(slnum);
}

void GlobalAPI::free_dynamic_sprite(ScriptMethodParams &params) {
	PARAMS1(int, gotSlot);
	AGS3::free_dynamic_sprite(gotSlot);
}

void GlobalAPI::disable_cursor_mode(ScriptMethodParams &params) {
	PARAMS1(int, modd);
	AGS3::disable_cursor_mode(modd);
}

void GlobalAPI::DisableGroundLevelAreas(ScriptMethodParams &params) {
	PARAMS1(int, alsoEffects);
	AGS3::DisableGroundLevelAreas(alsoEffects);
}

void GlobalAPI::DisableHotspot(ScriptMethodParams &params) {
	PARAMS1(int, hsnum);
	AGS3::DisableHotspot(hsnum);
}

void GlobalAPI::DisableInterface(ScriptMethodParams &params) {
	AGS3::DisableInterface();
}

void GlobalAPI::DisableRegion(ScriptMethodParams &params) {
	PARAMS1(int, hsnum);
	AGS3::DisableRegion(hsnum);
}

void GlobalAPI::ScPl_Display(ScriptMethodParams &params) {
	Common::String texx = params.format(0);
	AGS3::DisplaySimple(texx.c_str());
}

void GlobalAPI::ScPl_DisplayAt(ScriptMethodParams &params) {
	PARAMS3(int, xxp, int, yyp, int, widd);
	Common::String texx = params.format(3);
	AGS3::DisplayAt(xxp, yyp, widd, texx.c_str());
}

void GlobalAPI::DisplayAtY(ScriptMethodParams &params) {
	PARAMS2(int, ypos, const char *, texx);
	AGS3::DisplayAtY(ypos, texx);
}

void GlobalAPI::DisplayMessage(ScriptMethodParams &params) {
	PARAMS1(int, msnum);
	AGS3::DisplayMessage(msnum);
}

void GlobalAPI::DisplayMessageAtY(ScriptMethodParams &params) {
	PARAMS2(int, msnum, int, ypos);
	AGS3::DisplayMessageAtY(msnum, ypos);
}

void GlobalAPI::DisplayMessageBar(ScriptMethodParams &params) {
	PARAMS5(int, ypos, int, ttexcol, int, backcol, const char *, title, int, msgnum);
	AGS3::DisplayMessageBar(ypos, ttexcol, backcol, title, msgnum);
}

void GlobalAPI::ScPl_sc_displayspeech(ScriptMethodParams &params) {
	PARAMS1(int, chid);
	Common::String texx = params.format(1);
	AGS3::__sc_displayspeech(chid, texx.c_str());
}

void GlobalAPI::DisplaySpeechAt(ScriptMethodParams &params) {
	PARAMS5(int, xx, int, yy, int, wii, int, aschar, const char *, spch);
	AGS3::DisplaySpeechAt(xx, yy, wii, aschar, spch);
}

void GlobalAPI::DisplaySpeechBackground(ScriptMethodParams &params) {
	PARAMS2(int, charid, const char *, speel);
	params._result = AGS3::DisplaySpeechBackground(charid, speel);
}

void GlobalAPI::ScPl_DisplayThought(ScriptMethodParams &params) {
	PARAMS1(int, chid);
	Common::String texx = params.format(1);
	AGS3::DisplayThought(chid, texx.c_str());
}

void GlobalAPI::ScPl_DisplayTopBar(ScriptMethodParams &params) {
	PARAMS4(int, ypos, int, ttexcol, int, backcol, char *, title);
	Common::String texx = params.format(4);
	AGS3::DisplayTopBar(ypos, ttexcol, backcol, title, texx.c_str());
}

void GlobalAPI::enable_cursor_mode(ScriptMethodParams &params) {
	PARAMS1(int, modd);
	AGS3::enable_cursor_mode(modd);
}

void GlobalAPI::EnableGroundLevelAreas(ScriptMethodParams &params) {
	AGS3::EnableGroundLevelAreas();
}

void GlobalAPI::EnableHotspot(ScriptMethodParams &params) {
	PARAMS1(int, hsnum);
	AGS3::EnableHotspot(hsnum);
}

void GlobalAPI::EnableInterface(ScriptMethodParams &params) {
	AGS3::EnableInterface();
}

void GlobalAPI::EnableRegion(ScriptMethodParams &params) {
	PARAMS1(int, hsnum);
	AGS3::EnableRegion(hsnum);
}

void GlobalAPI::EndCutscene(ScriptMethodParams &params) {
	params._result = AGS3::EndCutscene();
}

void GlobalAPI::FaceCharacter(ScriptMethodParams &params) {
	PARAMS2(int, cha, int, toface);
	AGS3::FaceCharacter(cha, toface);
}

void GlobalAPI::FaceLocation(ScriptMethodParams &params) {
	PARAMS3(int, cha, int, xx, int, yy);
	AGS3::FaceLocation(cha, xx, yy);
}

void GlobalAPI::FadeIn(ScriptMethodParams &params) {
	PARAMS1(int, sppd);
	AGS3::FadeIn(sppd);
}

void GlobalAPI::my_fade_out(ScriptMethodParams &params) {
	PARAMS1(int, spdd);
	AGS3::my_fade_out(spdd);
}

void GlobalAPI::FileClose(ScriptMethodParams &params) {
	PARAMS1(int32_t, handle);
	AGS3::FileClose(handle);
}

void GlobalAPI::FileIsEOF(ScriptMethodParams &params) {
	PARAMS1(int32_t, handle);
	params._result = AGS3::FileIsEOF(handle);
}

void GlobalAPI::FileIsError(ScriptMethodParams &params) {
	PARAMS1(int32_t, handle);
	params._result = AGS3::FileIsError(handle);
}

void GlobalAPI::FileOpenCMode(ScriptMethodParams &params) {
	PARAMS2(const char *, fnmm, const char *, cmode);
	params._result = AGS3::FileOpenCMode(fnmm, cmode);
}

void GlobalAPI::FileRead(ScriptMethodParams &params) {
	PARAMS2(int32_t, handle, char *, toread);
	AGS3::FileRead(handle, toread);
}

void GlobalAPI::FileReadInt(ScriptMethodParams &params) {
	PARAMS1(int32_t, handle);
	params._result = AGS3::FileReadInt(handle);
}

void GlobalAPI::FileReadRawChar(ScriptMethodParams &params) {
	PARAMS1(int32_t, handle);
	params._result = AGS3::FileReadRawChar(handle);
}

void GlobalAPI::FileReadRawInt(ScriptMethodParams &params) {
	PARAMS1(int32_t, handle);
	params._result = AGS3::FileReadRawInt(handle);
}

void GlobalAPI::FileWrite(ScriptMethodParams &params) {
	PARAMS2(int32_t, handle, const char *, towrite);
	AGS3::FileWrite(handle, towrite);
}

void GlobalAPI::FileWriteInt(ScriptMethodParams &params) {
	PARAMS2(int32_t, handle, int, into);
	AGS3::FileWriteInt(handle, into);
}

void GlobalAPI::FileWriteRawChar(ScriptMethodParams &params) {
	PARAMS2(int32_t, handle, int, chartoWrite);
	AGS3::FileWriteRawChar(handle, chartoWrite);
}

void GlobalAPI::FileWriteRawLine(ScriptMethodParams &params) {
	PARAMS2(int32_t, handle, const char *, towrite);
	AGS3::FileWriteRawLine(handle, towrite);
}

void GlobalAPI::FindGUIID(ScriptMethodParams &params) {
	PARAMS1(const char *, GUIName);
	params._result = AGS3::FindGUIID(GUIName);
}

void GlobalAPI::FlipScreen(ScriptMethodParams &params) {
	PARAMS1(int, amount);
	AGS3::FlipScreen(amount);
}

void GlobalAPI::FloatToInt(ScriptMethodParams &params) {
	PARAMS2(float, value, int, roundDirection);
	params._result = AGS3::FloatToInt(value, roundDirection);
}

void GlobalAPI::FollowCharacter(ScriptMethodParams &params) {
	PARAMS2(int, who, int, tofollow);
	AGS3::FollowCharacter(who, tofollow);
}

void GlobalAPI::FollowCharacterEx(ScriptMethodParams &params) {
	PARAMS4(int, who, int, tofollow, int, distaway, int, eagerness);
	AGS3::FollowCharacterEx(who, tofollow, distaway, eagerness);
}

void GlobalAPI::GetBackgroundFrame(ScriptMethodParams &params) {
	params._result = AGS3::GetBackgroundFrame();
}

void GlobalAPI::GetButtonPic(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, ptype);
	params._result = AGS3::GetButtonPic(guin, objn, ptype);
}

void GlobalAPI::GetCharIDAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetCharIDAtScreen(xx, yy);
}

void GlobalAPI::GetCharacterProperty(ScriptMethodParams &params) {
	PARAMS2(int, cha, const char *, property);
	params._result = AGS3::GetCharacterProperty(cha, property);
}

void GlobalAPI::GetCharacterPropertyText(ScriptMethodParams &params) {
	PARAMS3(int, item, const char *, property, char *, buffer);
	AGS3::GetCharacterPropertyText(item, property, buffer);
}

void GlobalAPI::GetCurrentMusic(ScriptMethodParams &params) {
	params._result = AGS3::GetCurrentMusic();
}

void GlobalAPI::GetCursorMode(ScriptMethodParams &params) {
	params._result = AGS3::GetCursorMode();
}

void GlobalAPI::GetDialogOption(ScriptMethodParams &params) {
	PARAMS2(int, dlg, int, opt);
	params._result = AGS3::GetDialogOption(dlg, opt);
}

void GlobalAPI::GetGameOption(ScriptMethodParams &params) {
	PARAMS1(int, opt);
	params._result = AGS3::GetGameOption(opt);
}

void GlobalAPI::GetGameParameter(ScriptMethodParams &params) {
	PARAMS4(int, parm, int, data1, int, data2, int, data3);
	params._result = AGS3::GetGameParameter(parm, data1, data2, data3);
}

void GlobalAPI::GetGameSpeed(ScriptMethodParams &params) {
	params._result = AGS3::GetGameSpeed();
}

void GlobalAPI::GetGlobalInt(ScriptMethodParams &params) {
	PARAMS1(int, index);
	params._result = AGS3::GetGlobalInt(index);
}

void GlobalAPI::GetGlobalString(ScriptMethodParams &params) {
	PARAMS2(int, index, char *, strval);
	AGS3::GetGlobalString(index, strval);
}

void GlobalAPI::GetGraphicalVariable(ScriptMethodParams &params) {
	PARAMS1(const char *, varName);
	params._result = AGS3::GetGraphicalVariable(varName);
}

void GlobalAPI::GetGUIAt(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetGUIAt(xx, yy);
}

void GlobalAPI::GetGUIObjectAt(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetGUIObjectAt(xx, yy);
}

void GlobalAPI::GetHotspotIDAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetHotspotIDAtScreen(xx, yy);
}

void GlobalAPI::GetHotspotName(ScriptMethodParams &params) {
	PARAMS2(int, hotspot, char *, buffer);
	AGS3::GetHotspotName(hotspot, buffer);
}

void GlobalAPI::GetHotspotPointX(ScriptMethodParams &params) {
	PARAMS1(int, hotspot);
	params._result = AGS3::GetHotspotPointX(hotspot);
}

void GlobalAPI::GetHotspotPointY(ScriptMethodParams &params) {
	PARAMS1(int, hotspot);
	params._result = AGS3::GetHotspotPointY(hotspot);
}

void GlobalAPI::GetHotspotProperty(ScriptMethodParams &params) {
	PARAMS2(int, hss, const char *, property);
	params._result = AGS3::GetHotspotProperty(hss, property);
}

void GlobalAPI::GetHotspotPropertyText(ScriptMethodParams &params) {
	PARAMS3(int, item, const char *, property, char *, buffer);
	AGS3::GetHotspotPropertyText(item, property, buffer);
}

void GlobalAPI::GetInvAt(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetInvAt(xx, yy);
}

void GlobalAPI::GetInvGraphic(ScriptMethodParams &params) {
	PARAMS1(int, indx);
	params._result = AGS3::GetInvGraphic(indx);
}

void GlobalAPI::GetInvName(ScriptMethodParams &params) {
	PARAMS2(int, indx, char *, buff);
	AGS3::GetInvName(indx, buff);
}

void GlobalAPI::GetInvProperty(ScriptMethodParams &params) {
	PARAMS2(int, item, const char *, property);
	params._result = AGS3::GetInvProperty(item, property);
}

void GlobalAPI::GetInvPropertyText(ScriptMethodParams &params) {
	PARAMS3(int, item, const char *, property, char *, buffer);
	AGS3::GetInvPropertyText(item, property, buffer);
}

void GlobalAPI::GetLocationName(ScriptMethodParams &params) {
	PARAMS3(int, xx, int, yy, char *, tempo);
	AGS3::GetLocationName(xx, yy, tempo);
}

void GlobalAPI::GetLocationType(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetLocationType(xx, yy);
}

// TODO: The rest
void GlobalAPI::GetMessageText(ScriptMethodParams &params) {}

void GlobalAPI::GetMIDIPosition(ScriptMethodParams &params) {}

void GlobalAPI::GetMP3PosMillis(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectIDAtScreen(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectBaseline(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectGraphic(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectName(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectProperty(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectPropertyText(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectX(ScriptMethodParams &params) {}

void GlobalAPI::GetObjectY(ScriptMethodParams &params) {}

void GlobalAPI::GetPlayerCharacter(ScriptMethodParams &params) {}

void GlobalAPI::GetRawTime(ScriptMethodParams &params) {}

void GlobalAPI::GetRegionIDAtRoom(ScriptMethodParams &params) {}

void GlobalAPI::Room_GetProperty(ScriptMethodParams &params) {}

void GlobalAPI::GetRoomPropertyText(ScriptMethodParams &params) {}

void GlobalAPI::GetSaveSlotDescription(ScriptMethodParams &params) {}

void GlobalAPI::GetScalingAt(ScriptMethodParams &params) {}

void GlobalAPI::GetSliderValue(ScriptMethodParams &params) {}

void GlobalAPI::GetTextBoxText(ScriptMethodParams &params) {}

void GlobalAPI::GetTextHeight(ScriptMethodParams &params) {}

void GlobalAPI::GetTextWidth(ScriptMethodParams &params) {}

void GlobalAPI::sc_GetTime(ScriptMethodParams &params) {}

void GlobalAPI::get_translation(ScriptMethodParams &params) {}

void GlobalAPI::GetTranslationName(ScriptMethodParams &params) {}

void GlobalAPI::GetViewportX(ScriptMethodParams &params) {}

void GlobalAPI::GetViewportY(ScriptMethodParams &params) {}

void GlobalAPI::GetWalkableAreaAtRoom(ScriptMethodParams &params) {}

void GlobalAPI::GetWalkableAreaAtScreen(ScriptMethodParams &params) {}

void GlobalAPI::GiveScore(ScriptMethodParams &params) {}

void GlobalAPI::HasPlayerBeenInRoom(ScriptMethodParams &params) {}

void GlobalAPI::HideMouseCursor(ScriptMethodParams &params) {}

void GlobalAPI::sc_inputbox(ScriptMethodParams &params) {}

void GlobalAPI::InterfaceOff(ScriptMethodParams &params) {}

void GlobalAPI::InterfaceOn(ScriptMethodParams &params) {}

void GlobalAPI::IntToFloat(ScriptMethodParams &params) {}

void GlobalAPI::sc_invscreen(ScriptMethodParams &params) {}

void GlobalAPI::IsButtonDown(ScriptMethodParams &params) {}

void GlobalAPI::IsChannelPlaying(ScriptMethodParams &params) {}

void GlobalAPI::IsGamePaused(ScriptMethodParams &params) {}

void GlobalAPI::IsGUIOn(ScriptMethodParams &params) {}

void GlobalAPI::IsInteractionAvailable(ScriptMethodParams &params) {}

void GlobalAPI::IsInventoryInteractionAvailable(ScriptMethodParams &params) {}

void GlobalAPI::IsInterfaceEnabled(ScriptMethodParams &params) {}

void GlobalAPI::IsKeyPressed(ScriptMethodParams &params) {}

void GlobalAPI::IsMusicPlaying(ScriptMethodParams &params) {}

void GlobalAPI::IsMusicVoxAvailable(ScriptMethodParams &params) {}

void GlobalAPI::IsObjectAnimating(ScriptMethodParams &params) {}

void GlobalAPI::IsObjectMoving(ScriptMethodParams &params) {}

void GlobalAPI::IsObjectOn(ScriptMethodParams &params) {}

void GlobalAPI::IsOverlayValid(ScriptMethodParams &params) {}

void GlobalAPI::IsSoundPlaying(ScriptMethodParams &params) {}

void GlobalAPI::IsTimerExpired(ScriptMethodParams &params) {}

void GlobalAPI::IsTranslationAvailable(ScriptMethodParams &params) {}

void GlobalAPI::IsVoxAvailable(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxAdd(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxClear(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxDirList(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxGetItemText(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxGetNumItems(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxGetSelected(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxRemove(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxSaveGameList(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxSetSelected(ScriptMethodParams &params) {}

void GlobalAPI::ListBoxSetTopItem(ScriptMethodParams &params) {}

void GlobalAPI::LoadImageFile(ScriptMethodParams &params) {}

void GlobalAPI::LoadSaveSlotScreenshot(ScriptMethodParams &params) {}

void GlobalAPI::lose_inventory(ScriptMethodParams &params) {}

void GlobalAPI::LoseInventoryFromCharacter(ScriptMethodParams &params) {}

void GlobalAPI::MergeObject(ScriptMethodParams &params) {}

void GlobalAPI::MoveCharacter(ScriptMethodParams &params) {}

void GlobalAPI::MoveCharacterBlocking(ScriptMethodParams &params) {}

void GlobalAPI::MoveCharacterDirect(ScriptMethodParams &params) {}

void GlobalAPI::MoveCharacterPath(ScriptMethodParams &params) {}

void GlobalAPI::MoveCharacterStraight(ScriptMethodParams &params) {}

void GlobalAPI::MoveCharacterToHotspot(ScriptMethodParams &params) {}

void GlobalAPI::MoveCharacterToObject(ScriptMethodParams &params) {}

void GlobalAPI::MoveObject(ScriptMethodParams &params) {}

void GlobalAPI::MoveObjectDirect(ScriptMethodParams &params) {}

void GlobalAPI::MoveOverlay(ScriptMethodParams &params) {}

void GlobalAPI::MoveToWalkableArea(ScriptMethodParams &params) {}

void GlobalAPI::NewRoom(ScriptMethodParams &params) {}

void GlobalAPI::NewRoomEx(ScriptMethodParams &params) {}

void GlobalAPI::NewRoomNPC(ScriptMethodParams &params) {}

void GlobalAPI::ObjectOff(ScriptMethodParams &params) {}

void GlobalAPI::ObjectOn(ScriptMethodParams &params) {}

void GlobalAPI::ParseText(ScriptMethodParams &params) {}

void GlobalAPI::PauseGame(ScriptMethodParams &params) {}

void GlobalAPI::PlayAmbientSound(ScriptMethodParams &params) {}

void GlobalAPI::play_flc_file(ScriptMethodParams &params) {}

void GlobalAPI::PlayMP3File(ScriptMethodParams &params) {}

void GlobalAPI::PlayMusicResetQueue(ScriptMethodParams &params) {}

void GlobalAPI::PlayMusicQueued(ScriptMethodParams &params) {}

void GlobalAPI::PlaySilentMIDI(ScriptMethodParams &params) {}

void GlobalAPI::play_sound(ScriptMethodParams &params) {}

void GlobalAPI::PlaySoundEx(ScriptMethodParams &params) {}

void GlobalAPI::scrPlayVideo(ScriptMethodParams &params) {}

void GlobalAPI::RoomProcessClick(ScriptMethodParams &params) {}

void GlobalAPI::QuitGame(ScriptMethodParams &params) {}

void GlobalAPI::__Rand(ScriptMethodParams &params) {}

void GlobalAPI::RawClear(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawCircle(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawFrameTransparent(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawImage(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawImageOffset(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawImageResized(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawImageTransparent(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawLine(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawRectangle(ScriptMethodParams &params) {}

void GlobalAPI::RawDrawTriangle(ScriptMethodParams &params) {}

void GlobalAPI::ScPl_RawPrint(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	Common::String texx = params.format(2);

	AGS3::RawPrint(xx, yy, texx.c_str());
}

void GlobalAPI::RawPrintMessageWrapped(ScriptMethodParams &params) {}

void GlobalAPI::RawRestoreScreen(ScriptMethodParams &params) {}

void GlobalAPI::RawRestoreScreenTinted(ScriptMethodParams &params) {}

void GlobalAPI::RawSaveScreen(ScriptMethodParams &params) {}

void GlobalAPI::RawSetColor(ScriptMethodParams &params) {}

void GlobalAPI::RawSetColorRGB(ScriptMethodParams &params) {}

void GlobalAPI::RefreshMouse(ScriptMethodParams &params) {}

void GlobalAPI::ReleaseCharacterView(ScriptMethodParams &params) {}

void GlobalAPI::ReleaseViewport(ScriptMethodParams &params) {}

void GlobalAPI::RemoveObjectTint(ScriptMethodParams &params) {}

void GlobalAPI::RemoveOverlay(ScriptMethodParams &params) {}

void GlobalAPI::RemoveWalkableArea(ScriptMethodParams &params) {}

void GlobalAPI::ResetRoom(ScriptMethodParams &params) {}

void GlobalAPI::restart_game(ScriptMethodParams &params) {}

void GlobalAPI::restore_game_dialog(ScriptMethodParams &params) {}

void GlobalAPI::RestoreGameSlot(ScriptMethodParams &params) {}

void GlobalAPI::RestoreWalkableArea(ScriptMethodParams &params) {}

void GlobalAPI::RunAGSGame(ScriptMethodParams &params) {}

void GlobalAPI::RunCharacterInteraction(ScriptMethodParams &params) {}

void GlobalAPI::RunDialog(ScriptMethodParams &params) {}

void GlobalAPI::RunHotspotInteraction(ScriptMethodParams &params) {}

void GlobalAPI::RunInventoryInteraction(ScriptMethodParams &params) {}

void GlobalAPI::RunObjectInteraction(ScriptMethodParams &params) {}

void GlobalAPI::RunRegionInteraction(ScriptMethodParams &params) {}

void GlobalAPI::Said(ScriptMethodParams &params) {}

void GlobalAPI::SaidUnknownWord(ScriptMethodParams &params) {}

void GlobalAPI::SaveCursorForLocationChange(ScriptMethodParams &params) {}

void GlobalAPI::save_game_dialog(ScriptMethodParams &params) {}

void GlobalAPI::save_game(ScriptMethodParams &params) {}

void GlobalAPI::SaveScreenShot(ScriptMethodParams &params) {}

void GlobalAPI::SeekMIDIPosition(ScriptMethodParams &params) {}

void GlobalAPI::SeekMODPattern(ScriptMethodParams &params) {}

void GlobalAPI::SeekMP3PosMillis(ScriptMethodParams &params) {}

void GlobalAPI::SetActiveInventory(ScriptMethodParams &params) {}

void GlobalAPI::SetAmbientTint(ScriptMethodParams &params) {}

void GlobalAPI::SetAreaLightLevel(ScriptMethodParams &params) {}

void GlobalAPI::SetAreaScaling(ScriptMethodParams &params) {}

void GlobalAPI::SetBackgroundFrame(ScriptMethodParams &params) {}

void GlobalAPI::SetButtonPic(ScriptMethodParams &params) {}

void GlobalAPI::SetButtonText(ScriptMethodParams &params) {}

void GlobalAPI::SetChannelVolume(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterBaseline(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterClickable(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterFrame(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterIdle(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterIgnoreLight(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterIgnoreWalkbehinds(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterProperty(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterBlinkView(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterSpeechView(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterSpeed(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterSpeedEx(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterTransparency(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterView(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterViewEx(ScriptMethodParams &params) {}

void GlobalAPI::SetCharacterViewOffset(ScriptMethodParams &params) {}

void GlobalAPI::set_cursor_mode(ScriptMethodParams &params) {}

void GlobalAPI::set_default_cursor(ScriptMethodParams &params) {}

void GlobalAPI::SetDialogOption(ScriptMethodParams &params) {}

void GlobalAPI::SetDigitalMasterVolume(ScriptMethodParams &params) {}

void GlobalAPI::SetFadeColor(ScriptMethodParams &params) {}

void GlobalAPI::SetFrameSound(ScriptMethodParams &params) {}

void GlobalAPI::SetGameOption(ScriptMethodParams &params) {}

void GlobalAPI::SetGameSpeed(ScriptMethodParams &params) {}

void GlobalAPI::SetGlobalInt(ScriptMethodParams &params) {}

void GlobalAPI::SetGlobalString(ScriptMethodParams &params) {}

void GlobalAPI::SetGraphicalVariable(ScriptMethodParams &params) {}

void GlobalAPI::SetGUIBackgroundPic(ScriptMethodParams &params) {}

void GlobalAPI::SetGUIClickable(ScriptMethodParams &params) {}

void GlobalAPI::SetGUIObjectEnabled(ScriptMethodParams &params) {}

void GlobalAPI::SetGUIObjectPosition(ScriptMethodParams &params) {}

void GlobalAPI::SetGUIObjectSize(ScriptMethodParams &params) {}

void GlobalAPI::SetGUIPosition(ScriptMethodParams &params) {}

void GlobalAPI::SetGUISize(ScriptMethodParams &params) {}

void GlobalAPI::SetGUITransparency(ScriptMethodParams &params) {}

void GlobalAPI::SetGUIZOrder(ScriptMethodParams &params) {}

void GlobalAPI::SetInvItemName(ScriptMethodParams &params) {}

void GlobalAPI::set_inv_item_pic(ScriptMethodParams &params) {}

void GlobalAPI::SetInvDimensions(ScriptMethodParams &params) {}

void GlobalAPI::SetLabelColor(ScriptMethodParams &params) {}

void GlobalAPI::SetLabelFont(ScriptMethodParams &params) {}

void GlobalAPI::SetLabelText(ScriptMethodParams &params) {}

void GlobalAPI::SetMouseBounds(ScriptMethodParams &params) {}

void GlobalAPI::set_mouse_cursor(ScriptMethodParams &params) {}

void GlobalAPI::SetMousePosition(ScriptMethodParams &params) {}

void GlobalAPI::SetMultitasking(ScriptMethodParams &params) {}

void GlobalAPI::SetMusicMasterVolume(ScriptMethodParams &params) {}

void GlobalAPI::SetMusicRepeat(ScriptMethodParams &params) {}

void GlobalAPI::SetMusicVolume(ScriptMethodParams &params) {}

void GlobalAPI::SetNextCursor(ScriptMethodParams &params) {}

void GlobalAPI::SetNextScreenTransition(ScriptMethodParams &params) {}

void GlobalAPI::SetNormalFont(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectBaseline(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectClickable(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectFrame(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectGraphic(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectIgnoreWalkbehinds(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectPosition(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectTint(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectTransparency(ScriptMethodParams &params) {}

void GlobalAPI::SetObjectView(ScriptMethodParams &params) {}

void GlobalAPI::SetPalRGB(ScriptMethodParams &params) {}

void GlobalAPI::SetPlayerCharacter(ScriptMethodParams &params) {}

void GlobalAPI::SetRegionTint(ScriptMethodParams &params) {}

void GlobalAPI::SetRestartPoint(ScriptMethodParams &params) {}

void GlobalAPI::SetScreenTransition(ScriptMethodParams &params) {}

void GlobalAPI::SetSkipSpeech(ScriptMethodParams &params) {}

void GlobalAPI::SetSliderValue(ScriptMethodParams &params) {}

void GlobalAPI::SetSoundVolume(ScriptMethodParams &params) {}

void GlobalAPI::SetSpeechFont(ScriptMethodParams &params) {}

void GlobalAPI::SetSpeechStyle(ScriptMethodParams &params) {}

void GlobalAPI::SetSpeechVolume(ScriptMethodParams &params) {}

void GlobalAPI::SetTalkingColor(ScriptMethodParams &params) {}

void GlobalAPI::SetTextBoxFont(ScriptMethodParams &params) {

}

void GlobalAPI::SetTextBoxText(ScriptMethodParams &params) {

}

void GlobalAPI::ScPl_SetTextOverlay(ScriptMethodParams &params) {
	PARAMS6(int, ovrid, int, xx, int, yy, int, wii, int, fontid, int, clr);
	Common::String texx = params.format(6);

	AGS3::SetTextOverlay(ovrid, xx, yy, wii, fontid, clr, texx.c_str());
}

void GlobalAPI::SetTextWindowGUI(ScriptMethodParams &params) {

}

void GlobalAPI::script_SetTimer(ScriptMethodParams &params) {

}

void GlobalAPI::SetViewport(ScriptMethodParams &params) {

}

void GlobalAPI::SetVoiceMode(ScriptMethodParams &params) {

}

void GlobalAPI::SetWalkBehindBase(ScriptMethodParams &params) {

}

void GlobalAPI::ShakeScreen(ScriptMethodParams &params) {

}

void GlobalAPI::ShakeScreenBackground(ScriptMethodParams &params) {

}

void GlobalAPI::ShowMouseCursor(ScriptMethodParams &params) {

}

void GlobalAPI::SkipUntilCharacterStops(ScriptMethodParams &params) {

}

void GlobalAPI::StartCutscene(ScriptMethodParams &params) {

}

void GlobalAPI::scStartRecording(ScriptMethodParams &params) {

}

void GlobalAPI::StopAmbientSound(ScriptMethodParams &params) {

}

void GlobalAPI::stop_and_destroy_channel(ScriptMethodParams &params) {

}

void GlobalAPI::StopDialog(ScriptMethodParams &params) {

}

void GlobalAPI::StopMoving(ScriptMethodParams &params) {

}

void GlobalAPI::scr_StopMusic(ScriptMethodParams &params) {

}

void GlobalAPI::StopObjectMoving(ScriptMethodParams &params) {

}

void GlobalAPI::_sc_strcat(ScriptMethodParams &params) {

}

void GlobalAPI::ags_stricmp(ScriptMethodParams &params) {

}

void GlobalAPI::strcmp(ScriptMethodParams &params) {

}

void GlobalAPI::StrContains(ScriptMethodParams &params) {

}

void GlobalAPI::_sc_strcpy(ScriptMethodParams &params) {

}

void GlobalAPI::ScPl_sc_sprintf(ScriptMethodParams &params) {
	PARAMS1(char *, destt);
	Common::String texx = params.format(1);

	AGS3::_sc_strcpy(destt, texx.c_str());
}

void GlobalAPI::StrGetCharAt(ScriptMethodParams &params) {
}

void GlobalAPI::StringToInt(ScriptMethodParams &params) {
}

void GlobalAPI::strlen(ScriptMethodParams &params) {
}

void GlobalAPI::StrSetCharAt(ScriptMethodParams &params) {
}

void GlobalAPI::_sc_strlower(ScriptMethodParams &params) {
}

void GlobalAPI::_sc_strupper(ScriptMethodParams &params) {
}

void GlobalAPI::TintScreen(ScriptMethodParams &params) {
}

void GlobalAPI::UnPauseGame(ScriptMethodParams &params) {
}

void GlobalAPI::update_invorder(ScriptMethodParams &params) {
}

void GlobalAPI::UpdatePalette(ScriptMethodParams &params) {
}

void GlobalAPI::scrWait(ScriptMethodParams &params) {
}

void GlobalAPI::WaitKey(ScriptMethodParams &params) {
}

void GlobalAPI::WaitMouseKey(ScriptMethodParams &params) {
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
