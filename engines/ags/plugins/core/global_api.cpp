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

#include "ags/plugins/core/global_api.h"
#include "ags/engine/ac/cd_audio.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_button.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_date_time.h"
#include "ags/engine/ac/global_debug.h"
#include "ags/engine/ac/global_dialog.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_drawing_surface.h"
#include "ags/engine/ac/global_dynamic_sprite.h"
#include "ags/engine/ac/global_file.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_hotspot.h"
#include "ags/engine/ac/global_inv_window.h"
#include "ags/engine/ac/global_inventory_item.h"
#include "ags/engine/ac/global_label.h"
#include "ags/engine/ac/global_listbox.h"
#include "ags/engine/ac/global_mouse.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_overlay.h"
#include "ags/engine/ac/global_palette.h"
#include "ags/engine/ac/global_parser.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/global_slider.h"
#include "ags/engine/ac/global_string.h"
#include "ags/engine/ac/global_textbox.h"
#include "ags/engine/ac/global_timer.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/global_video.h"
#include "ags/engine/ac/global_view_frame.h"
#include "ags/engine/ac/global_viewport.h"
#include "ags/engine/ac/global_walk_behind.h"
#include "ags/engine/ac/global_walkable_area.h"
#include "ags/engine/ac/math.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/parser.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/media/audio/audio.h"
#include "ags/engine/media/video/video.h"
#include "ags/shared/util/string_compat.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void GlobalAPI::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AbortGame, GlobalAPI::ScPl_sc_AbortGame);
	SCRIPT_METHOD(AddInventory, GlobalAPI::add_inventory);
	SCRIPT_METHOD(AddInventoryToCharacter, GlobalAPI::AddInventoryToCharacter);
	SCRIPT_METHOD(AnimateButton, GlobalAPI::AnimateButton);
	SCRIPT_METHOD(AnimateCharacter, GlobalAPI::AnimateCharacter4);
	SCRIPT_METHOD(AnimateCharacterEx, GlobalAPI::AnimateCharacter6);
	SCRIPT_METHOD(AnimateObject, GlobalAPI::AnimateObject4);
	SCRIPT_METHOD(AnimateObjectEx, GlobalAPI::AnimateObject6);
	SCRIPT_METHOD(AreCharactersColliding, GlobalAPI::AreCharactersColliding);
	SCRIPT_METHOD(AreCharObjColliding, GlobalAPI::AreCharObjColliding);
	SCRIPT_METHOD(AreObjectsColliding, GlobalAPI::AreObjectsColliding);
	SCRIPT_METHOD(AreThingsOverlapping, GlobalAPI::AreThingsOverlapping);
	SCRIPT_METHOD(CallRoomScript, GlobalAPI::CallRoomScript);
	SCRIPT_METHOD(CDAudio, GlobalAPI::cd_manager);
	SCRIPT_METHOD(CentreGUI, GlobalAPI::CentreGUI);
	SCRIPT_METHOD(ChangeCharacterView, GlobalAPI::ChangeCharacterView);
	SCRIPT_METHOD(ChangeCursorGraphic, GlobalAPI::ChangeCursorGraphic);
	SCRIPT_METHOD(ChangeCursorHotspot, GlobalAPI::ChangeCursorHotspot);
	SCRIPT_METHOD(ClaimEvent, GlobalAPI::ClaimEvent);
	SCRIPT_METHOD(CreateGraphicOverlay, GlobalAPI::CreateGraphicOverlay);
	SCRIPT_METHOD(CreateTextOverlay, GlobalAPI::ScPl_CreateTextOverlay);
	SCRIPT_METHOD(CyclePalette, GlobalAPI::CyclePalette);
	SCRIPT_METHOD(Debug, GlobalAPI::script_debug);
	SCRIPT_METHOD(DeleteSaveSlot, GlobalAPI::DeleteSaveSlot);
	SCRIPT_METHOD(DeleteSprite, GlobalAPI::free_dynamic_sprite);
	SCRIPT_METHOD(DisableCursorMode, GlobalAPI::disable_cursor_mode);
	SCRIPT_METHOD(DisableGroundLevelAreas, GlobalAPI::DisableGroundLevelAreas);
	SCRIPT_METHOD(DisableHotspot, GlobalAPI::DisableHotspot);
	SCRIPT_METHOD(DisableInterface, GlobalAPI::DisableInterface);
	SCRIPT_METHOD(DisableRegion, GlobalAPI::DisableRegion);
	SCRIPT_METHOD(Display, GlobalAPI::ScPl_Display);
	SCRIPT_METHOD(DisplayAt, GlobalAPI::ScPl_DisplayAt);
	SCRIPT_METHOD(DisplayAtY, GlobalAPI::DisplayAtY);
	SCRIPT_METHOD(DisplayMessage, GlobalAPI::DisplayMessage);
	SCRIPT_METHOD(DisplayMessageAtY, GlobalAPI::DisplayMessageAtY);
	SCRIPT_METHOD(DisplayMessageBar, GlobalAPI::DisplayMessageBar);
	SCRIPT_METHOD(DisplaySpeech, GlobalAPI::ScPl_sc_displayspeech);
	SCRIPT_METHOD(DisplaySpeechAt, GlobalAPI::DisplaySpeechAt);
	SCRIPT_METHOD(DisplaySpeechBackground, GlobalAPI::DisplaySpeechBackground);
	SCRIPT_METHOD(DisplayThought, GlobalAPI::ScPl_DisplayThought);
	SCRIPT_METHOD(DisplayTopBar, GlobalAPI::ScPl_DisplayTopBar);
	SCRIPT_METHOD(EnableCursorMode, GlobalAPI::enable_cursor_mode);
	SCRIPT_METHOD(EnableGroundLevelAreas, GlobalAPI::EnableGroundLevelAreas);
	SCRIPT_METHOD(EnableHotspot, GlobalAPI::EnableHotspot);
	SCRIPT_METHOD(EnableInterface, GlobalAPI::EnableInterface);
	SCRIPT_METHOD(EnableRegion, GlobalAPI::EnableRegion);
	SCRIPT_METHOD(EndCutscene, GlobalAPI::EndCutscene);
	SCRIPT_METHOD(FaceCharacter, GlobalAPI::FaceCharacter);
	SCRIPT_METHOD(FaceLocation, GlobalAPI::FaceLocation);
	SCRIPT_METHOD(FadeIn, GlobalAPI::FadeIn);
	SCRIPT_METHOD(FadeOut, GlobalAPI::FadeOut);
	SCRIPT_METHOD(FileClose, GlobalAPI::FileClose);
	SCRIPT_METHOD(FileIsEOF, GlobalAPI::FileIsEOF);
	SCRIPT_METHOD(FileIsError, GlobalAPI::FileIsError);
	// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
	SCRIPT_METHOD(FileOpen, GlobalAPI::FileOpenCMode);
	SCRIPT_METHOD(FileRead, GlobalAPI::FileRead);
	SCRIPT_METHOD(FileReadInt, GlobalAPI::FileReadInt);
	SCRIPT_METHOD(FileReadRawChar, GlobalAPI::FileReadRawChar);
	SCRIPT_METHOD(FileReadRawInt, GlobalAPI::FileReadRawInt);
	SCRIPT_METHOD(FileWrite, GlobalAPI::FileWrite);
	SCRIPT_METHOD(FileWriteInt, GlobalAPI::FileWriteInt);
	SCRIPT_METHOD(FileWriteRawChar, GlobalAPI::FileWriteRawChar);
	SCRIPT_METHOD(FileWriteRawLine, GlobalAPI::FileWriteRawLine);
	SCRIPT_METHOD(FindGUIID, GlobalAPI::FindGUIID);
	SCRIPT_METHOD(FlipScreen, GlobalAPI::FlipScreen);
	SCRIPT_METHOD(FloatToInt, GlobalAPI::FloatToInt);
	SCRIPT_METHOD(FollowCharacter, GlobalAPI::FollowCharacter);
	SCRIPT_METHOD(FollowCharacterEx, GlobalAPI::FollowCharacterEx);
	SCRIPT_METHOD(GetBackgroundFrame, GlobalAPI::GetBackgroundFrame);
	SCRIPT_METHOD(GetButtonPic, GlobalAPI::GetButtonPic);
	SCRIPT_METHOD(GetCharacterAt, GlobalAPI::GetCharIDAtScreen);
	SCRIPT_METHOD(GetCharacterProperty, GlobalAPI::GetCharacterProperty);
	SCRIPT_METHOD(GetCharacterPropertyText, GlobalAPI::GetCharacterPropertyText);
	SCRIPT_METHOD(GetCurrentMusic, GlobalAPI::GetCurrentMusic);
	SCRIPT_METHOD(GetCursorMode, GlobalAPI::GetCursorMode);
	SCRIPT_METHOD(GetDialogOption, GlobalAPI::GetDialogOption);
	SCRIPT_METHOD(GetGameOption, GlobalAPI::GetGameOption);
	SCRIPT_METHOD(GetGameParameter, GlobalAPI::GetGameParameter);
	SCRIPT_METHOD(GetGameSpeed, GlobalAPI::GetGameSpeed);
	SCRIPT_METHOD(GetGlobalInt, GlobalAPI::GetGlobalInt);
	SCRIPT_METHOD(GetGlobalString, GlobalAPI::GetGlobalString);
	SCRIPT_METHOD(GetGraphicalVariable, GlobalAPI::GetGraphicalVariable);
	SCRIPT_METHOD(GetGUIAt, GlobalAPI::GetGUIAt);
	SCRIPT_METHOD(GetGUIObjectAt, GlobalAPI::GetGUIObjectAt);
	SCRIPT_METHOD(GetHotspotAt, GlobalAPI::GetHotspotIDAtScreen);
	SCRIPT_METHOD(GetHotspotName, GlobalAPI::GetHotspotName);
	SCRIPT_METHOD(GetHotspotPointX, GlobalAPI::GetHotspotPointX);
	SCRIPT_METHOD(GetHotspotPointY, GlobalAPI::GetHotspotPointY);
	SCRIPT_METHOD(GetHotspotProperty, GlobalAPI::GetHotspotProperty);
	SCRIPT_METHOD(GetHotspotPropertyText, GlobalAPI::GetHotspotPropertyText);
	SCRIPT_METHOD(GetInvAt, GlobalAPI::GetInvAt);
	SCRIPT_METHOD(GetInvGraphic, GlobalAPI::GetInvGraphic);
	SCRIPT_METHOD(GetInvName, GlobalAPI::GetInvName);
	SCRIPT_METHOD(GetInvProperty, GlobalAPI::GetInvProperty);
	SCRIPT_METHOD(GetInvPropertyText, GlobalAPI::GetInvPropertyText);
	SCRIPT_METHOD(GetLocationName, GlobalAPI::GetLocationName);
	SCRIPT_METHOD(GetLocationType, GlobalAPI::GetLocationType);
	SCRIPT_METHOD(GetMessageText, GlobalAPI::GetMessageText);
	SCRIPT_METHOD(GetMIDIPosition, GlobalAPI::GetMIDIPosition);
	SCRIPT_METHOD(GetMP3PosMillis, GlobalAPI::GetMP3PosMillis);
	SCRIPT_METHOD(GetObjectAt, GlobalAPI::GetObjectIDAtScreen);
	SCRIPT_METHOD(GetObjectBaseline, GlobalAPI::GetObjectBaseline);
	SCRIPT_METHOD(GetObjectGraphic, GlobalAPI::GetObjectGraphic);
	SCRIPT_METHOD(GetObjectName, GlobalAPI::GetObjectName);
	SCRIPT_METHOD(GetObjectProperty, GlobalAPI::GetObjectProperty);
	SCRIPT_METHOD(GetObjectPropertyText, GlobalAPI::GetObjectPropertyText);
	SCRIPT_METHOD(GetObjectX, GlobalAPI::GetObjectX);
	SCRIPT_METHOD(GetObjectY, GlobalAPI::GetObjectY);
	SCRIPT_METHOD(GetPlayerCharacter, GlobalAPI::GetPlayerCharacter);
	SCRIPT_METHOD(GetRawTime, GlobalAPI::GetRawTime);
	SCRIPT_METHOD(GetRegionAt, GlobalAPI::GetRegionIDAtRoom);
	SCRIPT_METHOD(GetRoomProperty, GlobalAPI::Room_GetProperty);
	SCRIPT_METHOD(GetRoomPropertyText, GlobalAPI::GetRoomPropertyText);
	SCRIPT_METHOD(GetSaveSlotDescription, GlobalAPI::GetSaveSlotDescription);
	SCRIPT_METHOD(GetScalingAt, GlobalAPI::GetScalingAt);
	SCRIPT_METHOD(GetSliderValue, GlobalAPI::GetSliderValue);
	SCRIPT_METHOD(GetTextBoxText, GlobalAPI::GetTextBoxText);
	SCRIPT_METHOD(GetTextHeight, GlobalAPI::GetTextHeight);
	SCRIPT_METHOD(GetTextWidth, GlobalAPI::GetTextWidth);
	SCRIPT_METHOD(GetTime, GlobalAPI::sc_GetTime);
	SCRIPT_METHOD(GetTranslation, GlobalAPI::get_translation);
	SCRIPT_METHOD(GetTranslationName, GlobalAPI::GetTranslationName);
	SCRIPT_METHOD(GetViewportX, GlobalAPI::GetViewportX);
	SCRIPT_METHOD(GetViewportY, GlobalAPI::GetViewportY);
	SCRIPT_METHOD(GetWalkableAreaAtRoom, GlobalAPI::GetWalkableAreaAtRoom);
	SCRIPT_METHOD(GetWalkableAreaAt, GlobalAPI::GetWalkableAreaAtScreen);
	SCRIPT_METHOD(GetWalkableAreaAtScreen, GlobalAPI::GetWalkableAreaAtScreen);
	SCRIPT_METHOD(GiveScore, GlobalAPI::GiveScore);
	SCRIPT_METHOD(HasPlayerBeenInRoom, GlobalAPI::HasPlayerBeenInRoom);
	SCRIPT_METHOD(HideMouseCursor, GlobalAPI::HideMouseCursor);
	SCRIPT_METHOD(InputBox, GlobalAPI::ShowInputBox);
	SCRIPT_METHOD(InterfaceOff, GlobalAPI::InterfaceOff);
	SCRIPT_METHOD(InterfaceOn, GlobalAPI::InterfaceOn);
	SCRIPT_METHOD(IntToFloat, GlobalAPI::IntToFloat);
	SCRIPT_METHOD(InventoryScreen, GlobalAPI::sc_invscreen);
	SCRIPT_METHOD(IsButtonDown, GlobalAPI::IsButtonDown);
	SCRIPT_METHOD(IsChannelPlaying, GlobalAPI::IsChannelPlaying);
	SCRIPT_METHOD(IsGamePaused, GlobalAPI::IsGamePaused);
	SCRIPT_METHOD(IsGUIOn, GlobalAPI::IsGUIOn);
	SCRIPT_METHOD(IsInteractionAvailable, GlobalAPI::IsInteractionAvailable);
	SCRIPT_METHOD(IsInventoryInteractionAvailable, GlobalAPI::IsInventoryInteractionAvailable);
	SCRIPT_METHOD(IsInterfaceEnabled, GlobalAPI::IsInterfaceEnabled);
	SCRIPT_METHOD(IsKeyPressed, GlobalAPI::IsKeyPressed);
	SCRIPT_METHOD(IsMusicPlaying, GlobalAPI::IsMusicPlaying);
	SCRIPT_METHOD(IsMusicVoxAvailable, GlobalAPI::IsMusicVoxAvailable);
	SCRIPT_METHOD(IsObjectAnimating, GlobalAPI::IsObjectAnimating);
	SCRIPT_METHOD(IsObjectMoving, GlobalAPI::IsObjectMoving);
	SCRIPT_METHOD(IsObjectOn, GlobalAPI::IsObjectOn);
	SCRIPT_METHOD(IsOverlayValid, GlobalAPI::IsOverlayValid);
	SCRIPT_METHOD(IsSoundPlaying, GlobalAPI::IsSoundPlaying);
	SCRIPT_METHOD(IsTimerExpired, GlobalAPI::IsTimerExpired);
	SCRIPT_METHOD(IsTranslationAvailable, GlobalAPI::IsTranslationAvailable);
	SCRIPT_METHOD(IsVoxAvailable, GlobalAPI::IsVoxAvailable);
	SCRIPT_METHOD(ListBoxAdd, GlobalAPI::ListBoxAdd);
	SCRIPT_METHOD(ListBoxClear, GlobalAPI::ListBoxClear);
	SCRIPT_METHOD(ListBoxDirList, GlobalAPI::ListBoxDirList);
	SCRIPT_METHOD(ListBoxGetItemText, GlobalAPI::ListBoxGetItemText);
	SCRIPT_METHOD(ListBoxGetNumItems, GlobalAPI::ListBoxGetNumItems);
	SCRIPT_METHOD(ListBoxGetSelected, GlobalAPI::ListBoxGetSelected);
	SCRIPT_METHOD(ListBoxRemove, GlobalAPI::ListBoxRemove);
	SCRIPT_METHOD(ListBoxSaveGameList, GlobalAPI::ListBoxSaveGameList);
	SCRIPT_METHOD(ListBoxSetSelected, GlobalAPI::ListBoxSetSelected);
	SCRIPT_METHOD(ListBoxSetTopItem, GlobalAPI::ListBoxSetTopItem);
	SCRIPT_METHOD(LoadImageFile, GlobalAPI::LoadImageFile);
	SCRIPT_METHOD(LoadSaveSlotScreenshot, GlobalAPI::LoadSaveSlotScreenshot);
	SCRIPT_METHOD(LoseInventory, GlobalAPI::lose_inventory);
	SCRIPT_METHOD(LoseInventoryFromCharacter, GlobalAPI::LoseInventoryFromCharacter);
	SCRIPT_METHOD(MergeObject, GlobalAPI::MergeObject);
	SCRIPT_METHOD(MoveCharacter, GlobalAPI::MoveCharacter);
	SCRIPT_METHOD(MoveCharacterBlocking, GlobalAPI::MoveCharacterBlocking);
	SCRIPT_METHOD(MoveCharacterDirect, GlobalAPI::MoveCharacterDirect);
	SCRIPT_METHOD(MoveCharacterPath, GlobalAPI::MoveCharacterPath);
	SCRIPT_METHOD(MoveCharacterStraight, GlobalAPI::MoveCharacterStraight);
	SCRIPT_METHOD(MoveCharacterToHotspot, GlobalAPI::MoveCharacterToHotspot);
	SCRIPT_METHOD(MoveCharacterToObject, GlobalAPI::MoveCharacterToObject);
	SCRIPT_METHOD(MoveObject, GlobalAPI::MoveObject);
	SCRIPT_METHOD(MoveObjectDirect, GlobalAPI::MoveObjectDirect);
	SCRIPT_METHOD(MoveOverlay, GlobalAPI::MoveOverlay);
	SCRIPT_METHOD(MoveToWalkableArea, GlobalAPI::MoveToWalkableArea);
	SCRIPT_METHOD(NewRoom, GlobalAPI::NewRoom);
	SCRIPT_METHOD(NewRoomEx, GlobalAPI::NewRoomEx);
	SCRIPT_METHOD(NewRoomNPC, GlobalAPI::NewRoomNPC);
	SCRIPT_METHOD(ObjectOff, GlobalAPI::ObjectOff);
	SCRIPT_METHOD(ObjectOn, GlobalAPI::ObjectOn);
	SCRIPT_METHOD(ParseText, GlobalAPI::ParseText);
	SCRIPT_METHOD(PauseGame, GlobalAPI::PauseGame);
	SCRIPT_METHOD(PlayAmbientSound, GlobalAPI::PlayAmbientSound);
	SCRIPT_METHOD(PlayFlic, GlobalAPI::PlayFlic);
	SCRIPT_METHOD(PlayMP3File, GlobalAPI::PlayMP3File);
	SCRIPT_METHOD(PlayMusic, GlobalAPI::PlayMusicResetQueue);
	SCRIPT_METHOD(PlayMusicQueued, GlobalAPI::PlayMusicQueued);
	SCRIPT_METHOD(PlaySilentMIDI, GlobalAPI::PlaySilentMIDI);
	SCRIPT_METHOD(PlaySound, GlobalAPI::play_sound);
	SCRIPT_METHOD(PlaySoundEx, GlobalAPI::PlaySoundEx);
	SCRIPT_METHOD(PlayVideo, GlobalAPI::PlayVideo);
	SCRIPT_METHOD(ProcessClick, GlobalAPI::RoomProcessClick);
	SCRIPT_METHOD(QuitGame, GlobalAPI::QuitGame);
	SCRIPT_METHOD(Random, GlobalAPI::__Rand);
	SCRIPT_METHOD(RawClearScreen, GlobalAPI::RawClear);
	SCRIPT_METHOD(RawDrawCircle, GlobalAPI::RawDrawCircle);
	SCRIPT_METHOD(RawDrawFrameTransparent, GlobalAPI::RawDrawFrameTransparent);
	SCRIPT_METHOD(RawDrawImage, GlobalAPI::RawDrawImage);
	SCRIPT_METHOD(RawDrawImageOffset, GlobalAPI::RawDrawImageOffset);
	SCRIPT_METHOD(RawDrawImageResized, GlobalAPI::RawDrawImageResized);
	SCRIPT_METHOD(RawDrawImageTransparent, GlobalAPI::RawDrawImageTransparent);
	SCRIPT_METHOD(RawDrawLine, GlobalAPI::RawDrawLine);
	SCRIPT_METHOD(RawDrawRectangle, GlobalAPI::RawDrawRectangle);
	SCRIPT_METHOD(RawDrawTriangle, GlobalAPI::RawDrawTriangle);
	SCRIPT_METHOD(RawPrint, GlobalAPI::ScPl_RawPrint);
	SCRIPT_METHOD(RawPrintMessageWrapped, GlobalAPI::RawPrintMessageWrapped);
	SCRIPT_METHOD(RawRestoreScreen, GlobalAPI::RawRestoreScreen);
	SCRIPT_METHOD(RawRestoreScreenTinted, GlobalAPI::RawRestoreScreenTinted);
	SCRIPT_METHOD(RawSaveScreen, GlobalAPI::RawSaveScreen);
	SCRIPT_METHOD(RawSetColor, GlobalAPI::RawSetColor);
	SCRIPT_METHOD(RawSetColorRGB, GlobalAPI::RawSetColorRGB);
	SCRIPT_METHOD(RefreshMouse, GlobalAPI::RefreshMouse);
	SCRIPT_METHOD(ReleaseCharacterView, GlobalAPI::ReleaseCharacterView);
	SCRIPT_METHOD(ReleaseViewport, GlobalAPI::ReleaseViewport);
	SCRIPT_METHOD(RemoveObjectTint, GlobalAPI::RemoveObjectTint);
	SCRIPT_METHOD(RemoveOverlay, GlobalAPI::RemoveOverlay);
	SCRIPT_METHOD(RemoveWalkableArea, GlobalAPI::RemoveWalkableArea);
	SCRIPT_METHOD(ResetRoom, GlobalAPI::ResetRoom);
	SCRIPT_METHOD(RestartGame, GlobalAPI::restart_game);
	SCRIPT_METHOD(RestoreGameDialog, GlobalAPI::restore_game_dialog);
	SCRIPT_METHOD(RestoreGameSlot, GlobalAPI::RestoreGameSlot);
	SCRIPT_METHOD(RestoreWalkableArea, GlobalAPI::RestoreWalkableArea);
	SCRIPT_METHOD(RunAGSGame, GlobalAPI::RunAGSGame);
	SCRIPT_METHOD(RunCharacterInteraction, GlobalAPI::RunCharacterInteraction);
	SCRIPT_METHOD(RunDialog, GlobalAPI::RunDialog);
	SCRIPT_METHOD(RunHotspotInteraction, GlobalAPI::RunHotspotInteraction);
	SCRIPT_METHOD(RunInventoryInteraction, GlobalAPI::RunInventoryInteraction);
	SCRIPT_METHOD(RunObjectInteraction, GlobalAPI::RunObjectInteraction);
	SCRIPT_METHOD(RunRegionInteraction, GlobalAPI::RunRegionInteraction);
	SCRIPT_METHOD(Said, GlobalAPI::Said);
	SCRIPT_METHOD(SaidUnknownWord, GlobalAPI::SaidUnknownWord);
	SCRIPT_METHOD(SaveCursorForLocationChange, GlobalAPI::SaveCursorForLocationChange);
	SCRIPT_METHOD(SaveGameDialog, GlobalAPI::save_game_dialog);
	SCRIPT_METHOD(SaveGameSlot, GlobalAPI::save_game);
	SCRIPT_METHOD(SaveScreenShot, GlobalAPI::SaveScreenShot);
	SCRIPT_METHOD(SeekMIDIPosition, GlobalAPI::SeekMIDIPosition);
	SCRIPT_METHOD(SeekMODPattern, GlobalAPI::SeekMODPattern);
	SCRIPT_METHOD(SeekMP3PosMillis, GlobalAPI::SeekMP3PosMillis);
	SCRIPT_METHOD(SetActiveInventory, GlobalAPI::SetActiveInventory);
	SCRIPT_METHOD(SetAmbientTint, GlobalAPI::SetAmbientTint);
	SCRIPT_METHOD(SetAreaLightLevel, GlobalAPI::SetAreaLightLevel);
	SCRIPT_METHOD(SetAreaScaling, GlobalAPI::SetAreaScaling);
	SCRIPT_METHOD(SetBackgroundFrame, GlobalAPI::SetBackgroundFrame);
	SCRIPT_METHOD(SetButtonPic, GlobalAPI::SetButtonPic);
	SCRIPT_METHOD(SetButtonText, GlobalAPI::SetButtonText);
	SCRIPT_METHOD(SetChannelVolume, GlobalAPI::SetChannelVolume);
	SCRIPT_METHOD(SetCharacterBaseline, GlobalAPI::SetCharacterBaseline);
	SCRIPT_METHOD(SetCharacterClickable, GlobalAPI::SetCharacterClickable);
	SCRIPT_METHOD(SetCharacterFrame, GlobalAPI::SetCharacterFrame);
	SCRIPT_METHOD(SetCharacterIdle, GlobalAPI::SetCharacterIdle);
	SCRIPT_METHOD(SetCharacterIgnoreLight, GlobalAPI::SetCharacterIgnoreLight);
	SCRIPT_METHOD(SetCharacterIgnoreWalkbehinds, GlobalAPI::SetCharacterIgnoreWalkbehinds);
	SCRIPT_METHOD(SetCharacterProperty, GlobalAPI::SetCharacterProperty);
	SCRIPT_METHOD(SetCharacterBlinkView, GlobalAPI::SetCharacterBlinkView);
	SCRIPT_METHOD(SetCharacterSpeechView, GlobalAPI::SetCharacterSpeechView);
	SCRIPT_METHOD(SetCharacterSpeed, GlobalAPI::SetCharacterSpeed);
	SCRIPT_METHOD(SetCharacterSpeedEx, GlobalAPI::SetCharacterSpeedEx);
	SCRIPT_METHOD(SetCharacterTransparency, GlobalAPI::SetCharacterTransparency);
	SCRIPT_METHOD(SetCharacterView, GlobalAPI::SetCharacterView);
	SCRIPT_METHOD(SetCharacterViewEx, GlobalAPI::SetCharacterViewEx);
	SCRIPT_METHOD(SetCharacterViewOffset, GlobalAPI::SetCharacterViewOffset);
	SCRIPT_METHOD(SetCursorMode, GlobalAPI::set_cursor_mode);
	SCRIPT_METHOD(SetDefaultCursor, GlobalAPI::set_default_cursor);
	SCRIPT_METHOD(SetDialogOption, GlobalAPI::SetDialogOption);
	SCRIPT_METHOD(SetDigitalMasterVolume, GlobalAPI::SetDigitalMasterVolume);
	SCRIPT_METHOD(SetFadeColor, GlobalAPI::SetFadeColor);
	SCRIPT_METHOD(SetFrameSound, GlobalAPI::SetFrameSound);
	SCRIPT_METHOD(SetGameOption, GlobalAPI::SetGameOption);
	SCRIPT_METHOD(SetGameSpeed, GlobalAPI::SetGameSpeed);
	SCRIPT_METHOD(SetGlobalInt, GlobalAPI::SetGlobalInt);
	SCRIPT_METHOD(SetGlobalString, GlobalAPI::SetGlobalString);
	SCRIPT_METHOD(SetGraphicalVariable, GlobalAPI::SetGraphicalVariable);
	SCRIPT_METHOD(SetGUIBackgroundPic, GlobalAPI::SetGUIBackgroundPic);
	SCRIPT_METHOD(SetGUIClickable, GlobalAPI::SetGUIClickable);
	SCRIPT_METHOD(SetGUIObjectEnabled, GlobalAPI::SetGUIObjectEnabled);
	SCRIPT_METHOD(SetGUIObjectPosition, GlobalAPI::SetGUIObjectPosition);
	SCRIPT_METHOD(SetGUIObjectSize, GlobalAPI::SetGUIObjectSize);
	SCRIPT_METHOD(SetGUIPosition, GlobalAPI::SetGUIPosition);
	SCRIPT_METHOD(SetGUISize, GlobalAPI::SetGUISize);
	SCRIPT_METHOD(SetGUITransparency, GlobalAPI::SetGUITransparency);
	SCRIPT_METHOD(SetGUIZOrder, GlobalAPI::SetGUIZOrder);
	SCRIPT_METHOD(SetInvItemName, GlobalAPI::SetInvItemName);
	SCRIPT_METHOD(SetInvItemPic, GlobalAPI::set_inv_item_pic);
	SCRIPT_METHOD(SetInvDimensions, GlobalAPI::SetInvDimensions);
	SCRIPT_METHOD(SetLabelColor, GlobalAPI::SetLabelColor);
	SCRIPT_METHOD(SetLabelFont, GlobalAPI::SetLabelFont);
	SCRIPT_METHOD(SetLabelText, GlobalAPI::SetLabelText);
	SCRIPT_METHOD(SetMouseBounds, GlobalAPI::SetMouseBounds);
	SCRIPT_METHOD(SetMouseCursor, GlobalAPI::set_mouse_cursor);
	SCRIPT_METHOD(SetMousePosition, GlobalAPI::SetMousePosition);
	SCRIPT_METHOD(SetMultitaskingMode, GlobalAPI::SetMultitasking);
	SCRIPT_METHOD(SetMusicMasterVolume, GlobalAPI::SetMusicMasterVolume);
	SCRIPT_METHOD(SetMusicRepeat, GlobalAPI::SetMusicRepeat);
	SCRIPT_METHOD(SetMusicVolume, GlobalAPI::SetMusicVolume);
	SCRIPT_METHOD(SetNextCursorMode, GlobalAPI::SetNextCursor);
	SCRIPT_METHOD(SetNextScreenTransition, GlobalAPI::SetNextScreenTransition);
	SCRIPT_METHOD(SetNormalFont, GlobalAPI::SetNormalFont);
	SCRIPT_METHOD(SetObjectBaseline, GlobalAPI::SetObjectBaseline);
	SCRIPT_METHOD(SetObjectClickable, GlobalAPI::SetObjectClickable);
	SCRIPT_METHOD(SetObjectFrame, GlobalAPI::SetObjectFrame);
	SCRIPT_METHOD(SetObjectGraphic, GlobalAPI::SetObjectGraphic);
	SCRIPT_METHOD(SetObjectIgnoreWalkbehinds, GlobalAPI::SetObjectIgnoreWalkbehinds);
	SCRIPT_METHOD(SetObjectPosition, GlobalAPI::SetObjectPosition);
	SCRIPT_METHOD(SetObjectTint, GlobalAPI::SetObjectTint);
	SCRIPT_METHOD(SetObjectTransparency, GlobalAPI::SetObjectTransparency);
	SCRIPT_METHOD(SetObjectView, GlobalAPI::SetObjectView);
	//  SCRIPT_METHOD(SetPalette", GlobalAPI::          (void*)scSetPal);
	SCRIPT_METHOD(SetPalRGB, GlobalAPI::SetPalRGB);
	SCRIPT_METHOD(SetPlayerCharacter, GlobalAPI::SetPlayerCharacter);
	SCRIPT_METHOD(SetRegionTint, GlobalAPI::SetRegionTint);
	SCRIPT_METHOD(SetRestartPoint, GlobalAPI::SetRestartPoint);
	SCRIPT_METHOD(SetScreenTransition, GlobalAPI::SetScreenTransition);
	SCRIPT_METHOD(SetSkipSpeech, GlobalAPI::SetSkipSpeech);
	SCRIPT_METHOD(SetSliderValue, GlobalAPI::SetSliderValue);
	SCRIPT_METHOD(SetSoundVolume, GlobalAPI::SetSoundVolume);
	SCRIPT_METHOD(SetSpeechFont, GlobalAPI::SetSpeechFont);
	SCRIPT_METHOD(SetSpeechStyle, GlobalAPI::SetSpeechStyle);
	SCRIPT_METHOD(SetSpeechVolume, GlobalAPI::SetSpeechVolume);
	SCRIPT_METHOD(SetTalkingColor, GlobalAPI::SetTalkingColor);
	SCRIPT_METHOD(SetTextBoxFont, GlobalAPI::SetTextBoxFont);
	SCRIPT_METHOD(SetTextBoxText, GlobalAPI::SetTextBoxText);
	SCRIPT_METHOD(SetTextOverlay, GlobalAPI::ScPl_SetTextOverlay);
	SCRIPT_METHOD(SetTextWindowGUI, GlobalAPI::SetTextWindowGUI);
	SCRIPT_METHOD(SetTimer, GlobalAPI::script_SetTimer);
	SCRIPT_METHOD(SetViewport, GlobalAPI::SetViewport);
	SCRIPT_METHOD(SetVoiceMode, GlobalAPI::SetVoiceMode);
	SCRIPT_METHOD(SetWalkBehindBase, GlobalAPI::SetWalkBehindBase);
	SCRIPT_METHOD(ShakeScreen, GlobalAPI::ShakeScreen);
	SCRIPT_METHOD(ShakeScreenBackground, GlobalAPI::ShakeScreenBackground);
	SCRIPT_METHOD(ShowMouseCursor, GlobalAPI::ShowMouseCursor);
	SCRIPT_METHOD(SkipUntilCharacterStops, GlobalAPI::SkipUntilCharacterStops);
	SCRIPT_METHOD(StartCutscene, GlobalAPI::StartCutscene);
	SCRIPT_METHOD(StartRecording, GlobalAPI::scStartRecording);
	SCRIPT_METHOD(StopAmbientSound, GlobalAPI::StopAmbientSound);
	SCRIPT_METHOD(StopChannel, GlobalAPI::stop_and_destroy_channel);
	SCRIPT_METHOD(StopDialog, GlobalAPI::StopDialog);
	SCRIPT_METHOD(StopMoving, GlobalAPI::StopMoving);
	SCRIPT_METHOD(StopMusic, GlobalAPI::scr_StopMusic);
	SCRIPT_METHOD(StopObjectMoving, GlobalAPI::StopObjectMoving);
	SCRIPT_METHOD(StrCat, GlobalAPI::_sc_strcat);
	SCRIPT_METHOD(StrCaseComp, GlobalAPI::ags_stricmp);
	SCRIPT_METHOD(StrComp, GlobalAPI::strcmp);
	SCRIPT_METHOD(StrContains, GlobalAPI::StrContains);
	SCRIPT_METHOD(StrCopy, GlobalAPI::_sc_strcpy);
	SCRIPT_METHOD(StrFormat, GlobalAPI::ScPl_sc_sprintf);
	SCRIPT_METHOD(StrGetCharAt, GlobalAPI::StrGetCharAt);
	SCRIPT_METHOD(StringToInt, GlobalAPI::StringToInt);
	SCRIPT_METHOD(StrLen, GlobalAPI::strlen);
	SCRIPT_METHOD(StrSetCharAt, GlobalAPI::StrSetCharAt);
	SCRIPT_METHOD(StrToLowerCase, GlobalAPI::_sc_strlower);
	SCRIPT_METHOD(StrToUpperCase, GlobalAPI::_sc_strupper);
	SCRIPT_METHOD(TintScreen, GlobalAPI::TintScreen);
	SCRIPT_METHOD(UnPauseGame, GlobalAPI::UnPauseGame);
	SCRIPT_METHOD(UpdateInventory, GlobalAPI::update_invorder);
	SCRIPT_METHOD(UpdatePalette, GlobalAPI::UpdatePalette);
	SCRIPT_METHOD(Wait, GlobalAPI::scrWait);
	SCRIPT_METHOD(WaitKey, GlobalAPI::WaitKey);
	SCRIPT_METHOD(WaitMouseKey, GlobalAPI::WaitMouseKey);
	SCRIPT_METHOD(WaitInput, GlobalAPI::WaitInput);
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

void GlobalAPI::AnimateCharacter4(ScriptMethodParams &params) {
	PARAMS4(int, chh, int, loopn, int, sppd, int, rept);
	AGS3::AnimateCharacter4(chh, loopn, sppd, rept);
}

void GlobalAPI::AnimateCharacter6(ScriptMethodParams &params) {
	PARAMS6(int, chh, int, loopn, int, sppd, int, rept, int, direction, int, blocking);
	AGS3::AnimateCharacter6(chh, loopn, sppd, rept, direction, blocking);
}

void GlobalAPI::AnimateObject4(ScriptMethodParams &params) {
	PARAMS4(int, obn, int, loopn, int, spdd, int, rept);
	AGS3::AnimateObject4(obn, loopn, spdd, rept);
}

void GlobalAPI::AnimateObject6(ScriptMethodParams &params) {
	PARAMS6(int, obn, int, loopn, int, spdd, int, rept, int, direction, int, blocking);
	AGS3::AnimateObject6(obn, loopn, spdd, rept, direction, blocking);
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

void GlobalAPI::FadeOut(ScriptMethodParams &params) {
	PARAMS1(int, spdd);
	AGS3::FadeOut(spdd);
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

void GlobalAPI::GetMessageText(ScriptMethodParams &params) {
	PARAMS2(int, msg, char *, buffer);
	AGS3::GetMessageText(msg, buffer);
}

void GlobalAPI::GetMIDIPosition(ScriptMethodParams &params) {
	params._result = AGS3::GetMIDIPosition();
}

void GlobalAPI::GetMP3PosMillis(ScriptMethodParams &params) {
	params._result = AGS3::GetMP3PosMillis();
}

void GlobalAPI::GetObjectIDAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, scrx, int, scry);
	params._result = AGS3::GetObjectIDAtScreen(scrx, scry);
}

void GlobalAPI::GetObjectBaseline(ScriptMethodParams &params) {
	PARAMS1(int, obn);
	params._result = AGS3::GetObjectBaseline(obn);
}

void GlobalAPI::GetObjectGraphic(ScriptMethodParams &params) {
	PARAMS1(int, obn);
	params._result = AGS3::GetObjectGraphic(obn);
}

void GlobalAPI::GetObjectName(ScriptMethodParams &params) {
	PARAMS2(int, obj, char *, buffer);
	AGS3::GetObjectName(obj, buffer);
}

void GlobalAPI::GetObjectProperty(ScriptMethodParams &params) {
	PARAMS2(int, hss, const char *, property);
	params._result = AGS3::GetObjectProperty(hss, property);
}

void GlobalAPI::GetObjectPropertyText(ScriptMethodParams &params) {
	PARAMS3(int, item, const char *, property, char *, buffer);
	AGS3::GetObjectPropertyText(item, property, buffer);
}

void GlobalAPI::GetObjectX(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	params._result = AGS3::GetObjectX(obj);
}

void GlobalAPI::GetObjectY(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	params._result = AGS3::GetObjectY(obj);
}

void GlobalAPI::GetPlayerCharacter(ScriptMethodParams &params) {
	params._result = AGS3::GetPlayerCharacter();
}

void GlobalAPI::GetRawTime(ScriptMethodParams &params) {
	params._result = AGS3::GetRawTime();
}

void GlobalAPI::GetRegionIDAtRoom(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetRegionIDAtRoom(xx, yy);
}

void GlobalAPI::Room_GetProperty(ScriptMethodParams &params) {
	PARAMS1(const char *, property);
	params._result = AGS3::Room_GetProperty(property);
}

void GlobalAPI::GetRoomPropertyText(ScriptMethodParams &params) {
	PARAMS2(const char *, property, char *, buffer);
	AGS3::GetRoomPropertyText(property, buffer);
}

void GlobalAPI::GetSaveSlotDescription(ScriptMethodParams &params) {
	PARAMS2(int, slnum, char *, desbuf);
	params._result = AGS3::GetSaveSlotDescription(slnum, desbuf);
}

void GlobalAPI::GetScalingAt(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetScalingAt(x, y);
}

void GlobalAPI::GetSliderValue(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, objn);
	params._result = AGS3::GetSliderValue(guin, objn);
}

void GlobalAPI::GetTextBoxText(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, char *, txbuf);
	AGS3::GetTextBoxText(guin, objn, txbuf);
}

void GlobalAPI::GetTextHeight(ScriptMethodParams &params) {
	PARAMS3(const char *, text, int, fontnum, int, width);
	params._result = AGS3::GetTextHeight(text, fontnum, width);
}

void GlobalAPI::GetTextWidth(ScriptMethodParams &params) {
	PARAMS2(const char *, text, int, fontnum);
	params._result = AGS3::GetTextWidth(text, fontnum);
}

void GlobalAPI::sc_GetTime(ScriptMethodParams &params) {
	PARAMS1(int, whatti);
	params._result = AGS3::sc_GetTime(whatti);
}

void GlobalAPI::get_translation(ScriptMethodParams &params) {
	PARAMS1(const char *, text);
	params._result = AGS3::get_translation(text);
}

void GlobalAPI::GetTranslationName(ScriptMethodParams &params) {
	PARAMS1(char *, buffer);
	params._result = AGS3::GetTranslationName(buffer);
}

void GlobalAPI::GetViewportX(ScriptMethodParams &params) {
	params._result = AGS3::GetViewportX();
}

void GlobalAPI::GetViewportY(ScriptMethodParams &params) {
	params._result = AGS3::GetViewportY();
}

void GlobalAPI::GetWalkableAreaAtRoom(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetWalkableAreaAtRoom(x, y);
}

void GlobalAPI::GetWalkableAreaAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetWalkableAreaAtScreen(x, y);
}

void GlobalAPI::GiveScore(ScriptMethodParams &params) {
	PARAMS1(int, amount);
	AGS3::GiveScore(amount);
}

void GlobalAPI::HasPlayerBeenInRoom(ScriptMethodParams &params) {
	PARAMS1(int, roomnum);
	params._result = AGS3::HasPlayerBeenInRoom(roomnum);
}

void GlobalAPI::HideMouseCursor(ScriptMethodParams &params) {
	AGS3::HideMouseCursor();
}

void GlobalAPI::ShowInputBox(ScriptMethodParams &params) {
	PARAMS2(const char *, msg, char *, bufr);
	AGS3::ShowInputBox(msg, bufr);
}

void GlobalAPI::InterfaceOff(ScriptMethodParams &params) {
	PARAMS1(int, ifn);
	AGS3::InterfaceOff(ifn);
}

void GlobalAPI::InterfaceOn(ScriptMethodParams &params) {
	PARAMS1(int, ifn);
	AGS3::InterfaceOn(ifn);
}

void GlobalAPI::IntToFloat(ScriptMethodParams &params) {
	PARAMS1(int, value);
	params._result = AGS3::IntToFloat(value);
}

void GlobalAPI::sc_invscreen(ScriptMethodParams &params) {
	AGS3::sc_invscreen();
}

void GlobalAPI::IsButtonDown(ScriptMethodParams &params) {
	PARAMS1(int, which);
	params._result = AGS3::IsButtonDown(which);
}

void GlobalAPI::IsChannelPlaying(ScriptMethodParams &params) {
	PARAMS1(int, chan);
	params._result = AGS3::IsChannelPlaying(chan);
}

void GlobalAPI::IsGamePaused(ScriptMethodParams &params) {
	params._result = AGS3::IsGamePaused();
}

void GlobalAPI::IsGUIOn(ScriptMethodParams &params) {
	PARAMS1(int, guinum);
	params._result = AGS3::IsGUIOn(guinum);
}

void GlobalAPI::IsInteractionAvailable(ScriptMethodParams &params) {
	PARAMS3(int, xx, int, yy, int, mood);
	params._result = AGS3::IsInteractionAvailable(xx, yy, mood);
}

void GlobalAPI::IsInventoryInteractionAvailable(ScriptMethodParams &params) {
	PARAMS2(int, item, int, mood);
	params._result = AGS3::IsInventoryInteractionAvailable(item, mood);
}

void GlobalAPI::IsInterfaceEnabled(ScriptMethodParams &params) {
	params._result = AGS3::IsInterfaceEnabled();
}

void GlobalAPI::IsKeyPressed(ScriptMethodParams &params) {
	PARAMS1(int, keycode);
	params._result = AGS3::IsKeyPressed(keycode);
}

void GlobalAPI::IsMusicPlaying(ScriptMethodParams &params) {
	params._result = AGS3::IsMusicPlaying();
}

void GlobalAPI::IsMusicVoxAvailable(ScriptMethodParams &params) {
	params._result = AGS3::IsMusicVoxAvailable();
}

void GlobalAPI::IsObjectAnimating(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	params._result = AGS3::IsObjectAnimating(obj);
}

void GlobalAPI::IsObjectMoving(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	params._result = AGS3::IsObjectMoving(obj);
}

void GlobalAPI::IsObjectOn(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	params._result = AGS3::IsObjectOn(obj);
}

void GlobalAPI::IsOverlayValid(ScriptMethodParams &params) {
	PARAMS1(int, ovrid);
	params._result = AGS3::IsOverlayValid(ovrid);
}

void GlobalAPI::IsSoundPlaying(ScriptMethodParams &params) {
	params._result = AGS3::IsSoundPlaying();
}

void GlobalAPI::IsTimerExpired(ScriptMethodParams &params) {
	PARAMS1(int, tnum);
	params._result = AGS3::IsTimerExpired(tnum);
}

void GlobalAPI::IsTranslationAvailable(ScriptMethodParams &params) {
	params._result = AGS3::IsTranslationAvailable();
}

void GlobalAPI::IsVoxAvailable(ScriptMethodParams &params) {
	params._result = AGS3::IsVoxAvailable();
}

void GlobalAPI::ListBoxAdd(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, const char *, newitem);
	AGS3::ListBoxAdd(guin, objn, newitem);
}

void GlobalAPI::ListBoxClear(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, objn);
	AGS3::ListBoxClear(guin, objn);
}

void GlobalAPI::ListBoxDirList(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, const char *, filemask);
	AGS3::ListBoxDirList(guin, objn, filemask);
}

void GlobalAPI::ListBoxGetItemText(ScriptMethodParams &params) {
	PARAMS4(int, guin, int, objn, int, item, char *, buffer);
	params._result = AGS3::ListBoxGetItemText(guin, objn, item, buffer);
}

void GlobalAPI::ListBoxGetNumItems(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, objn);
	params._result = AGS3::ListBoxGetNumItems(guin, objn);
}

void GlobalAPI::ListBoxGetSelected(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, objn);
	params._result = AGS3::ListBoxGetSelected(guin, objn);
}

void GlobalAPI::ListBoxRemove(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, itemIndex);
	AGS3::ListBoxRemove(guin, objn, itemIndex);
}

void GlobalAPI::ListBoxSaveGameList(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, objn);
	params._result = AGS3::ListBoxSaveGameList(guin, objn);
}

void GlobalAPI::ListBoxSetSelected(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, newsel);
	AGS3::ListBoxSetSelected(guin, objn, newsel);
}

void GlobalAPI::ListBoxSetTopItem(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, item);
	AGS3::ListBoxSetTopItem(guin, objn, item);
}

void GlobalAPI::LoadImageFile(ScriptMethodParams &params) {
	PARAMS1(const char *, filename);
	params._result = AGS3::LoadImageFile(filename);
}

void GlobalAPI::LoadSaveSlotScreenshot(ScriptMethodParams &params) {
	PARAMS3(int, slnum, int, width, int, height);
	params._result = AGS3::LoadSaveSlotScreenshot(slnum, width, height);
}

void GlobalAPI::lose_inventory(ScriptMethodParams &params) {
	PARAMS1(int, inum);
	AGS3::lose_inventory(inum);
}

void GlobalAPI::LoseInventoryFromCharacter(ScriptMethodParams &params) {
	PARAMS2(int, charid, int, inum);
	AGS3::LoseInventoryFromCharacter(charid, inum);
}

void GlobalAPI::MergeObject(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	AGS3::MergeObject(obj);
}

void GlobalAPI::MoveCharacter(ScriptMethodParams &params) {
	PARAMS3(int, cc, int, xx, int, yy);
	AGS3::MoveCharacter(cc, xx, yy);
}

void GlobalAPI::MoveCharacterBlocking(ScriptMethodParams &params) {
	PARAMS4(int, chaa, int, xx, int, yy, int, direct);
	params._result = AGS3::MoveCharacterBlocking(chaa, xx, yy, direct);
}

void GlobalAPI::MoveCharacterDirect(ScriptMethodParams &params) {
	PARAMS3(int, cc, int, xx, int, yy);
	AGS3::MoveCharacterDirect(cc, xx, yy);
}

void GlobalAPI::MoveCharacterPath(ScriptMethodParams &params) {
	PARAMS3(int, chac, int, tox, int, toy);
	AGS3::MoveCharacterPath(chac, tox, toy);
}

void GlobalAPI::MoveCharacterStraight(ScriptMethodParams &params) {
	PARAMS3(int, cc, int, xx, int, yy);
	AGS3::MoveCharacterStraight(cc, xx, yy);
}

void GlobalAPI::MoveCharacterToHotspot(ScriptMethodParams &params) {
	PARAMS2(int, chaa, int, hotsp);
	AGS3::MoveCharacterToHotspot(chaa, hotsp);
}

void GlobalAPI::MoveCharacterToObject(ScriptMethodParams &params) {
	PARAMS2(int, chaa, int, obbj);
	AGS3::MoveCharacterToObject(chaa, obbj);
}

void GlobalAPI::MoveObject(ScriptMethodParams &params) {
	PARAMS4(int, obj, int, xx, int, yy, int, spp);
	AGS3::MoveObject(obj, xx, yy, spp);
}

void GlobalAPI::MoveObjectDirect(ScriptMethodParams &params) {
	PARAMS4(int, obj, int, xx, int, yy, int, spp);
	AGS3::MoveObjectDirect(obj, xx, yy, spp);
}

void GlobalAPI::MoveOverlay(ScriptMethodParams &params) {
	PARAMS3(int, ovrid, int, newx, int, newy);
	AGS3::MoveOverlay(ovrid, newx, newy);
}

void GlobalAPI::MoveToWalkableArea(ScriptMethodParams &params) {
	PARAMS1(int, charid);
	AGS3::MoveToWalkableArea(charid);
}

void GlobalAPI::NewRoom(ScriptMethodParams &params) {
	PARAMS1(int, nrnum);
	AGS3::NewRoom(nrnum);
}

void GlobalAPI::NewRoomEx(ScriptMethodParams &params) {
	PARAMS3(int, nrnum, int, newx, int, newy);
	AGS3::NewRoomEx(nrnum, newx, newy);
}

void GlobalAPI::NewRoomNPC(ScriptMethodParams &params) {
	PARAMS4(int, charid, int, nrnum, int, newx, int, newy);
	AGS3::NewRoomNPC(charid, nrnum, newx, newy);
}

void GlobalAPI::ObjectOff(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	AGS3::ObjectOff(obj);
}

void GlobalAPI::ObjectOn(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	AGS3::ObjectOn(obj);
}

void GlobalAPI::ParseText(ScriptMethodParams &params) {
	PARAMS1(const char *, text);
	AGS3::ParseText(text);
}

void GlobalAPI::PauseGame(ScriptMethodParams &params) {
	AGS3::PauseGame();
}

void GlobalAPI::PlayAmbientSound(ScriptMethodParams &params) {
	PARAMS5(int, channel, int, sndnum, int, vol, int, x, int, y);
	AGS3::PlayAmbientSound(channel, sndnum, vol, x, y);
}

void GlobalAPI::PlayFlic(ScriptMethodParams &params) {
	PARAMS2(int, numb, int, playflags);
	AGS3::PlayFlic(numb, playflags);
}

void GlobalAPI::PlayMP3File(ScriptMethodParams &params) {
	PARAMS1(const char *, filename);
	AGS3::PlayMP3File(filename);
}

void GlobalAPI::PlayMusicResetQueue(ScriptMethodParams &params) {
	PARAMS1(int, newmus);
	AGS3::PlayMusicResetQueue(newmus);
}

void GlobalAPI::PlayMusicQueued(ScriptMethodParams &params) {
	PARAMS1(int, musnum);
	AGS3::PlayMusicQueued(musnum);
}

void GlobalAPI::PlaySilentMIDI(ScriptMethodParams &params) {
	PARAMS1(int, musnum);
	AGS3::PlaySilentMIDI(musnum);
}

void GlobalAPI::play_sound(ScriptMethodParams &params) {
	PARAMS1(int, sndnum);
	params._result = AGS3::play_sound(sndnum);
}

void GlobalAPI::PlaySoundEx(ScriptMethodParams &params) {
	PARAMS2(int, sndnum, int, channel);
	params._result = AGS3::PlaySoundEx(sndnum, channel);
}

void GlobalAPI::PlayVideo(ScriptMethodParams &params) {
	PARAMS3(const char *, name, int, skip, int, flags);
	AGS3::PlayVideo(name, skip, flags);
}

void GlobalAPI::RoomProcessClick(ScriptMethodParams &params) {
	PARAMS3(int, xx, int, yy, int, mood);
	AGS3::RoomProcessClick(xx, yy, mood);
}

void GlobalAPI::QuitGame(ScriptMethodParams &params) {
	PARAMS1(int, dialog);
	AGS3::QuitGame(dialog);
}

void GlobalAPI::__Rand(ScriptMethodParams &params) {
	PARAMS1(int, upto);
	params._result = AGS3::__Rand(upto);
}

void GlobalAPI::RawClear(ScriptMethodParams &params) {
	PARAMS1(int, clr);
	AGS3::RawClear(clr);
}

void GlobalAPI::RawDrawCircle(ScriptMethodParams &params) {
	PARAMS3(int, xx, int, yy, int, rad);
	AGS3::RawDrawCircle(xx, yy, rad);
}

void GlobalAPI::RawDrawFrameTransparent(ScriptMethodParams &params) {
	PARAMS2(int, frame, int, translev);
	AGS3::RawDrawFrameTransparent(frame, translev);
}

void GlobalAPI::RawDrawImage(ScriptMethodParams &params) {
	PARAMS3(int, xx, int, yy, int, slot);
	AGS3::RawDrawImage(xx, yy, slot);
}

void GlobalAPI::RawDrawImageOffset(ScriptMethodParams &params) {
	PARAMS3(int, xx, int, yy, int, slot);
	AGS3::RawDrawImageOffset(xx, yy, slot);
}

void GlobalAPI::RawDrawImageResized(ScriptMethodParams &params) {
	PARAMS5(int, xx, int, yy, int, gotSlot, int, width, int, height);
	AGS3::RawDrawImageResized(xx, yy, gotSlot, width, height);
}

void GlobalAPI::RawDrawImageTransparent(ScriptMethodParams &params) {
	PARAMS4(int, xx, int, yy, int, slot, int, opacity);
	AGS3::RawDrawImageTransparent(xx, yy, slot, opacity);
}

void GlobalAPI::RawDrawLine(ScriptMethodParams &params) {
	PARAMS4(int, fromx, int, fromy, int, tox, int, toy);
	AGS3::RawDrawLine(fromx, fromy, tox, toy);
}

void GlobalAPI::RawDrawRectangle(ScriptMethodParams &params) {
	PARAMS4(int, x1, int, y1, int, x2, int, y2);
	AGS3::RawDrawRectangle(x1, y1, x2, y2);
}

void GlobalAPI::RawDrawTriangle(ScriptMethodParams &params) {
	PARAMS6(int, x1, int, y1, int, x2, int, y2, int, x3, int, y3);
	AGS3::RawDrawTriangle(x1, y1, x2, y2, x3, y3);
}

void GlobalAPI::ScPl_RawPrint(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	Common::String texx = params.format(2);

	AGS3::RawPrint(xx, yy, texx.c_str());
}

void GlobalAPI::RawPrintMessageWrapped(ScriptMethodParams &params) {
	PARAMS5(int, xx, int, yy, int, wid, int, font, int, msgm);
	AGS3::RawPrintMessageWrapped(xx, yy, wid, font, msgm);
}

void GlobalAPI::RawRestoreScreen(ScriptMethodParams &params) {
	AGS3::RawRestoreScreen();
}

void GlobalAPI::RawRestoreScreenTinted(ScriptMethodParams &params) {
	PARAMS4(int, red, int, green, int, blue, int, opacity);
	AGS3::RawRestoreScreenTinted(red, green, blue, opacity);
}

void GlobalAPI::RawSaveScreen(ScriptMethodParams &params) {
	AGS3::RawSaveScreen();
}

void GlobalAPI::RawSetColor(ScriptMethodParams &params) {
	PARAMS1(int, clr);
	AGS3::RawSetColor(clr);
}

void GlobalAPI::RawSetColorRGB(ScriptMethodParams &params) {
	PARAMS3(int, red, int, grn, int, blu);
	AGS3::RawSetColorRGB(red, grn, blu);
}

void GlobalAPI::RefreshMouse(ScriptMethodParams &params) {
	AGS3::RefreshMouse();
}

void GlobalAPI::ReleaseCharacterView(ScriptMethodParams &params) {
	PARAMS1(int, chat);
	AGS3::ReleaseCharacterView(chat);
}

void GlobalAPI::ReleaseViewport(ScriptMethodParams &params) {
	AGS3::ReleaseViewport();
}

void GlobalAPI::RemoveObjectTint(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	AGS3::RemoveObjectTint(obj);
}

void GlobalAPI::RemoveOverlay(ScriptMethodParams &params) {
	PARAMS1(int, ovrid);
	AGS3::RemoveOverlay(ovrid);
}

void GlobalAPI::RemoveWalkableArea(ScriptMethodParams &params) {
	PARAMS1(int, areanum);
	AGS3::RemoveWalkableArea(areanum);
}

void GlobalAPI::ResetRoom(ScriptMethodParams &params) {
	PARAMS1(int, nrnum);
	AGS3::ResetRoom(nrnum);
}

void GlobalAPI::restart_game(ScriptMethodParams &params) {
	AGS3::restart_game();
}

void GlobalAPI::restore_game_dialog(ScriptMethodParams &params) {
	AGS3::restore_game_dialog();
}

void GlobalAPI::RestoreGameSlot(ScriptMethodParams &params) {
	PARAMS1(int, slnum);
	AGS3::RestoreGameSlot(slnum);
}

void GlobalAPI::RestoreWalkableArea(ScriptMethodParams &params) {
	PARAMS1(int, areanum);
	AGS3::RestoreWalkableArea(areanum);
}

void GlobalAPI::RunAGSGame(ScriptMethodParams &params) {
	PARAMS3(const char *, newgame, unsigned int, mode, int, data);
	params._result = AGS3::RunAGSGame(newgame, mode, data);
}

void GlobalAPI::RunCharacterInteraction(ScriptMethodParams &params) {
	PARAMS2(int, cc, int, mood);
	AGS3::RunCharacterInteraction(cc, mood);
}

void GlobalAPI::RunDialog(ScriptMethodParams &params) {
	PARAMS1(int, tum);
	AGS3::RunDialog(tum);
}

void GlobalAPI::RunHotspotInteraction(ScriptMethodParams &params) {
	PARAMS2(int, hotspothere, int, mood);
	AGS3::RunHotspotInteraction(hotspothere, mood);
}

void GlobalAPI::RunInventoryInteraction(ScriptMethodParams &params) {
	PARAMS2(int, iit, int, modd);
	AGS3::RunInventoryInteraction(iit, modd);
}

void GlobalAPI::RunObjectInteraction(ScriptMethodParams &params) {
	PARAMS2(int, aa, int, mood);
	AGS3::RunObjectInteraction(aa, mood);
}

void GlobalAPI::RunRegionInteraction(ScriptMethodParams &params) {
	PARAMS2(int, regnum, int, mood);
	AGS3::RunRegionInteraction(regnum, mood);
}

void GlobalAPI::Said(ScriptMethodParams &params) {
	PARAMS1(const char *, checkwords);
	params._result = AGS3::Said(checkwords);
}

void GlobalAPI::SaidUnknownWord(ScriptMethodParams &params) {
	PARAMS1(char *, buffer);
	params._result = AGS3::SaidUnknownWord(buffer);
}

void GlobalAPI::SaveCursorForLocationChange(ScriptMethodParams &params) {
	AGS3::SaveCursorForLocationChange();
}

void GlobalAPI::save_game_dialog(ScriptMethodParams &params) {
	AGS3::save_game_dialog();
}

void GlobalAPI::save_game(ScriptMethodParams &params) {
	PARAMS2(int, slotn, const char *, descript);
	AGS3::save_game(slotn, descript);
}

void GlobalAPI::SaveScreenShot(ScriptMethodParams &params) {
	PARAMS1(const char *, namm);
	params._result = AGS3::SaveScreenShot(namm);
}

void GlobalAPI::SeekMIDIPosition(ScriptMethodParams &params) {
	PARAMS1(int, position);
	AGS3::SeekMIDIPosition(position);
}

void GlobalAPI::SeekMODPattern(ScriptMethodParams &params) {
	PARAMS1(int, patnum);
	AGS3::SeekMODPattern(patnum);
}

void GlobalAPI::SeekMP3PosMillis(ScriptMethodParams &params) {
	PARAMS1(int, position);
	AGS3::SeekMP3PosMillis(position);
}

void GlobalAPI::SetActiveInventory(ScriptMethodParams &params) {
	PARAMS1(int, iit);
	AGS3::SetActiveInventory(iit);
}

void GlobalAPI::SetAmbientTint(ScriptMethodParams &params) {
	PARAMS5(int, red, int, green, int, blue, int, opacity, int, luminance);
	AGS3::SetAmbientTint(red, green, blue, opacity, luminance);
}

void GlobalAPI::SetAreaLightLevel(ScriptMethodParams &params) {
	PARAMS2(int, area, int, brightness);
	AGS3::SetAreaLightLevel(area, brightness);
}

void GlobalAPI::SetAreaScaling(ScriptMethodParams &params) {
	PARAMS3(int, area, int, min, int, max);
	AGS3::SetAreaScaling(area, min, max);
}

void GlobalAPI::SetBackgroundFrame(ScriptMethodParams &params) {
	PARAMS1(int, frnum);
	AGS3::SetBackgroundFrame(frnum);
}

void GlobalAPI::SetButtonPic(ScriptMethodParams &params) {
	PARAMS4(int, guin, int, objn, int, ptype, int, slotn);
	AGS3::SetButtonPic(guin, objn, ptype, slotn);
}

void GlobalAPI::SetButtonText(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, const char *, newtx);
	AGS3::SetButtonText(guin, objn, newtx);
}

void GlobalAPI::SetChannelVolume(ScriptMethodParams &params) {
	PARAMS2(int, chan, int, newvol);
	AGS3::SetChannelVolume(chan, newvol);
}

void GlobalAPI::SetCharacterBaseline(ScriptMethodParams &params) {
	PARAMS2(int, obn, int, basel);
	AGS3::SetCharacterBaseline(obn, basel);
}

void GlobalAPI::SetCharacterClickable(ScriptMethodParams &params) {
	PARAMS2(int, cha, int, clik);
	AGS3::SetCharacterClickable(cha, clik);
}

void GlobalAPI::SetCharacterFrame(ScriptMethodParams &params) {
	PARAMS4(int, chaa, int, view, int, loop, int, frame);
	AGS3::SetCharacterFrame(chaa, view, loop, frame);
}

void GlobalAPI::SetCharacterIdle(ScriptMethodParams &params) {
	PARAMS3(int, who, int, iview, int, itime);
	AGS3::SetCharacterIdle(who, iview, itime);
}

void GlobalAPI::SetCharacterIgnoreLight(ScriptMethodParams &params) {
	PARAMS2(int, who, int, yesorno);
	AGS3::SetCharacterIgnoreLight(who, yesorno);
}

void GlobalAPI::SetCharacterIgnoreWalkbehinds(ScriptMethodParams &params) {
	PARAMS2(int, cha, int, clik);
	AGS3::SetCharacterIgnoreWalkbehinds(cha, clik);
}

void GlobalAPI::SetCharacterProperty(ScriptMethodParams &params) {
	PARAMS3(int, who, int, flag, int, yesorno);
	AGS3::SetCharacterProperty(who, flag, yesorno);
}

void GlobalAPI::SetCharacterBlinkView(ScriptMethodParams &params) {
	PARAMS3(int, chaa, int, vii, int, intrv);
	AGS3::SetCharacterBlinkView(chaa, vii, intrv);
}

void GlobalAPI::SetCharacterSpeechView(ScriptMethodParams &params) {
	PARAMS2(int, chaa, int, vii);
	AGS3::SetCharacterSpeechView(chaa, vii);
}

void GlobalAPI::SetCharacterSpeed(ScriptMethodParams &params) {
	PARAMS2(int, chaa, int, nspeed);
	AGS3::SetCharacterSpeed(chaa, nspeed);
}

void GlobalAPI::SetCharacterSpeedEx(ScriptMethodParams &params) {
	PARAMS3(int, chaa, int, xspeed, int, yspeed);
	AGS3::SetCharacterSpeedEx(chaa, xspeed, yspeed);
}

void GlobalAPI::SetCharacterTransparency(ScriptMethodParams &params) {
	PARAMS2(int, obn, int, trans);
	AGS3::SetCharacterTransparency(obn, trans);
}

void GlobalAPI::SetCharacterView(ScriptMethodParams &params) {
	PARAMS2(int, chaa, int, vii);
	AGS3::SetCharacterView(chaa, vii);
}

void GlobalAPI::SetCharacterViewEx(ScriptMethodParams &params) {
	PARAMS4(int, chaa, int, vii, int, loop, int, align);
	AGS3::SetCharacterViewEx(chaa, vii, loop, align);
}

void GlobalAPI::SetCharacterViewOffset(ScriptMethodParams &params) {
	PARAMS4(int, chaa, int, vii, int, xoffs, int, yoffs);
	AGS3::SetCharacterViewOffset(chaa, vii, xoffs, yoffs);
}

void GlobalAPI::set_cursor_mode(ScriptMethodParams &params) {
	PARAMS1(int, mode);
	AGS3::set_cursor_mode(mode);
}

void GlobalAPI::set_default_cursor(ScriptMethodParams &params) {
	AGS3::set_default_cursor();
}

void GlobalAPI::SetDialogOption(ScriptMethodParams &params) {
	PARAMS3(int, dlg, int, opt, int, onoroff);
	bool dlg_script = params.size() == 3 ? false : params[3];

	AGS3::SetDialogOption(dlg, opt, onoroff, dlg_script);
}

void GlobalAPI::SetDigitalMasterVolume(ScriptMethodParams &params) {
	PARAMS1(int, newvol);
	AGS3::SetDigitalMasterVolume(newvol);
}

void GlobalAPI::SetFadeColor(ScriptMethodParams &params) {
	PARAMS3(int, red, int, green, int, blue);
	AGS3::SetFadeColor(red, green, blue);
}

void GlobalAPI::SetFrameSound(ScriptMethodParams &params) {
	PARAMS4(int, vii, int, loop, int, frame, int, sound);
	AGS3::SetFrameSound(vii, loop, frame, sound);
}

void GlobalAPI::SetGameOption(ScriptMethodParams &params) {
	PARAMS2(int, opt, int, setting);
	AGS3::SetGameOption(opt, setting);
}

void GlobalAPI::SetGameSpeed(ScriptMethodParams &params) {
	PARAMS1(int, newSpeed);
	AGS3::SetGameSpeed(newSpeed);
}

void GlobalAPI::SetGlobalInt(ScriptMethodParams &params) {
	PARAMS2(int, index, int, newval);
	AGS3::SetGlobalInt(index, newval);
}

void GlobalAPI::SetGlobalString(ScriptMethodParams &params) {
	PARAMS2(int, index, const char *, newval);
	AGS3::SetGlobalString(index, newval);
}

void GlobalAPI::SetGraphicalVariable(ScriptMethodParams &params) {
	PARAMS2(const char *, varName, int, p_value);
	AGS3::SetGraphicalVariable(varName, p_value);
}

void GlobalAPI::SetGUIBackgroundPic(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, slotn);
	AGS3::SetGUIBackgroundPic(guin, slotn);
}

void GlobalAPI::SetGUIClickable(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, clickable);
	AGS3::SetGUIClickable(guin, clickable);
}

void GlobalAPI::SetGUIObjectEnabled(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, enabled);
	AGS3::SetGUIObjectEnabled(guin, objn, enabled);
}

void GlobalAPI::SetGUIObjectPosition(ScriptMethodParams &params) {
	PARAMS4(int, guin, int, objn, int, xx, int, yy);
	AGS3::SetGUIObjectPosition(guin, objn, xx, yy);
}

void GlobalAPI::SetGUIObjectSize(ScriptMethodParams &params) {
	PARAMS4(int, ifn, int, objn, int, newwid, int, newhit);
	AGS3::SetGUIObjectSize(ifn, objn, newwid, newhit);
}

void GlobalAPI::SetGUIPosition(ScriptMethodParams &params) {
	PARAMS3(int, ifn, int, xx, int, yy);
	AGS3::SetGUIPosition(ifn, xx, yy);
}

void GlobalAPI::SetGUISize(ScriptMethodParams &params) {
	PARAMS3(int, ifn, int, widd, int, hitt);
	AGS3::SetGUISize(ifn, widd, hitt);
}

void GlobalAPI::SetGUITransparency(ScriptMethodParams &params) {
	PARAMS2(int, ifn, int, trans);
	AGS3::SetGUITransparency(ifn, trans);
}

void GlobalAPI::SetGUIZOrder(ScriptMethodParams &params) {
	PARAMS2(int, guin, int, z);
	AGS3::SetGUIZOrder(guin, z);
}

void GlobalAPI::SetInvItemName(ScriptMethodParams &params) {
	PARAMS2(int, invi, const char *, newName);
	AGS3::SetInvItemName(invi, newName);
}

void GlobalAPI::set_inv_item_pic(ScriptMethodParams &params) {
	PARAMS2(int, invi, int, piccy);
	AGS3::set_inv_item_pic(invi, piccy);
}

void GlobalAPI::SetInvDimensions(ScriptMethodParams &params) {
	PARAMS2(int, ww, int, hh);
	AGS3::SetInvDimensions(ww, hh);
}

void GlobalAPI::SetLabelColor(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, colr);
	AGS3::SetLabelColor(guin, objn, colr);
}

void GlobalAPI::SetLabelFont(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, fontnum);
	AGS3::SetLabelFont(guin, objn, fontnum);
}

void GlobalAPI::SetLabelText(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, const char *, newtx);
	AGS3::SetLabelText(guin, objn, newtx);
}

void GlobalAPI::SetMouseBounds(ScriptMethodParams &params) {
	PARAMS4(int, x1, int, y1, int, x2, int, y2);
	AGS3::SetMouseBounds(x1, y1, x2, y2);
}

void GlobalAPI::set_mouse_cursor(ScriptMethodParams &params) {
	PARAMS1(int, newcurs);
	AGS3::set_mouse_cursor(newcurs);
}

void GlobalAPI::SetMousePosition(ScriptMethodParams &params) {
	PARAMS2(int, newx, int, newy);
	AGS3::SetMousePosition(newx, newy);
}

void GlobalAPI::SetMultitasking(ScriptMethodParams &params) {
	PARAMS1(int, mode);
	AGS3::SetMultitasking(mode);
}

void GlobalAPI::SetMusicMasterVolume(ScriptMethodParams &params) {
	PARAMS1(int, newvol);
	AGS3::SetMusicMasterVolume(newvol);
}

void GlobalAPI::SetMusicRepeat(ScriptMethodParams &params) {
	PARAMS1(int, loopflag);
	AGS3::SetMusicRepeat(loopflag);
}

void GlobalAPI::SetMusicVolume(ScriptMethodParams &params) {
	PARAMS1(int, newvol);
	AGS3::SetMusicVolume(newvol);
}

void GlobalAPI::SetNextCursor(ScriptMethodParams &params) {
	AGS3::SetNextCursor();
}

void GlobalAPI::SetNextScreenTransition(ScriptMethodParams &params) {
	PARAMS1(int, newtrans);
	AGS3::SetNextScreenTransition(newtrans);
}

void GlobalAPI::SetNormalFont(ScriptMethodParams &params) {
	PARAMS1(int, fontnum);
	AGS3::SetNormalFont(fontnum);
}

void GlobalAPI::SetObjectBaseline(ScriptMethodParams &params) {
	PARAMS2(int, obn, int, basel);
	AGS3::SetObjectBaseline(obn, basel);
}

void GlobalAPI::SetObjectClickable(ScriptMethodParams &params) {
	PARAMS2(int, cha, int, clik);
	AGS3::SetObjectClickable(cha, clik);
}

void GlobalAPI::SetObjectFrame(ScriptMethodParams &params) {
	PARAMS4(int, obn, int, viw, int, lop, int, fra);
	AGS3::SetObjectFrame(obn, viw, lop, fra);
}

void GlobalAPI::SetObjectGraphic(ScriptMethodParams &params) {
	PARAMS2(int, obn, int, slott);
	AGS3::SetObjectGraphic(obn, slott);
}

void GlobalAPI::SetObjectIgnoreWalkbehinds(ScriptMethodParams &params) {
	PARAMS2(int, cha, int, clik);
	AGS3::SetObjectIgnoreWalkbehinds(cha, clik);
}

void GlobalAPI::SetObjectPosition(ScriptMethodParams &params) {
	PARAMS3(int, objj, int, tox, int, toy);
	AGS3::SetObjectPosition(objj, tox, toy);
}

void GlobalAPI::SetObjectTint(ScriptMethodParams &params) {
	PARAMS6(int, obj, int, red, int, green, int, blue, int, opacity, int, luminance);
	AGS3::SetObjectTint(obj, red, green, blue, opacity, luminance);
}

void GlobalAPI::SetObjectTransparency(ScriptMethodParams &params) {
	PARAMS2(int, obn, int, trans);
	AGS3::SetObjectTransparency(obn, trans);
}

void GlobalAPI::SetObjectView(ScriptMethodParams &params) {
	PARAMS2(int, obn, int, vii);
	AGS3::SetObjectView(obn, vii);
}

void GlobalAPI::SetPalRGB(ScriptMethodParams &params) {
	PARAMS4(int, inndx, int, rr, int, gg, int, bb);
	AGS3::SetPalRGB(inndx, rr, gg, bb);
}

void GlobalAPI::SetPlayerCharacter(ScriptMethodParams &params) {
	PARAMS1(int, newchar);
	AGS3::SetPlayerCharacter(newchar);
}

void GlobalAPI::SetRegionTint(ScriptMethodParams &params) {
	PARAMS5(int, area, int, red, int, green, int, blue, int, amount);
	int luminance = params.size() == 5 ? 100 : params[5];

	AGS3::SetRegionTint(area, red, green, blue, amount, luminance);
}

void GlobalAPI::SetRestartPoint(ScriptMethodParams &params) {
	AGS3::SetRestartPoint();
}

void GlobalAPI::SetScreenTransition(ScriptMethodParams &params) {
	PARAMS1(int, newtrans);
	AGS3::SetScreenTransition(newtrans);
}

void GlobalAPI::SetSkipSpeech(ScriptMethodParams &params) {
	PARAMS1(SkipSpeechStyle, newval);
	AGS3::SetSkipSpeech(newval);
}

void GlobalAPI::SetSliderValue(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, valn);
	AGS3::SetSliderValue(guin, objn, valn);
}

void GlobalAPI::SetSoundVolume(ScriptMethodParams &params) {
	PARAMS1(int, newvol);
	AGS3::SetSoundVolume(newvol);
}

void GlobalAPI::SetSpeechFont(ScriptMethodParams &params) {
	PARAMS1(int, fontnum);
	AGS3::SetSpeechFont(fontnum);
}

void GlobalAPI::SetSpeechStyle(ScriptMethodParams &params) {
	PARAMS1(int, newstyle);
	AGS3::SetSpeechStyle(newstyle);
}

void GlobalAPI::SetSpeechVolume(ScriptMethodParams &params) {
	PARAMS1(int, newvol);
	AGS3::SetSpeechVolume(newvol);
}

void GlobalAPI::SetTalkingColor(ScriptMethodParams &params) {
	PARAMS2(int, chaa, int, ncol);
	AGS3::SetTalkingColor(chaa, ncol);
}

void GlobalAPI::SetTextBoxFont(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, int, fontnum);
	AGS3::SetTextBoxFont(guin, objn, fontnum);
}

void GlobalAPI::SetTextBoxText(ScriptMethodParams &params) {
	PARAMS3(int, guin, int, objn, const char *, txbuf);
	AGS3::SetTextBoxText(guin, objn, txbuf);
}

void GlobalAPI::ScPl_SetTextOverlay(ScriptMethodParams &params) {
	PARAMS6(int, ovrid, int, xx, int, yy, int, wii, int, fontid, int, clr);
	Common::String texx = params.format(6);

	AGS3::SetTextOverlay(ovrid, xx, yy, wii, fontid, clr, texx.c_str());
}

void GlobalAPI::SetTextWindowGUI(ScriptMethodParams &params) {
	PARAMS1(int, guinum);
	AGS3::SetTextWindowGUI(guinum);
}

void GlobalAPI::script_SetTimer(ScriptMethodParams &params) {
	PARAMS2(int, tnum, int, timeout);
	AGS3::script_SetTimer(tnum, timeout);
}

void GlobalAPI::SetViewport(ScriptMethodParams &params) {
	PARAMS2(int, offsx, int, offsy);
	AGS3::SetViewport(offsx, offsy);
}

void GlobalAPI::SetVoiceMode(ScriptMethodParams &params) {
	PARAMS1(int, newmod);
	AGS3::SetVoiceMode(newmod);
}

void GlobalAPI::SetWalkBehindBase(ScriptMethodParams &params) {
	PARAMS2(int, wa, int, bl);
	AGS3::SetWalkBehindBase(wa, bl);
}

void GlobalAPI::ShakeScreen(ScriptMethodParams &params) {
	PARAMS1(int, severe);
	AGS3::ShakeScreen(severe);
}

void GlobalAPI::ShakeScreenBackground(ScriptMethodParams &params) {
	PARAMS3(int, delay, int, amount, int, length);
	AGS3::ShakeScreenBackground(delay, amount, length);
}

void GlobalAPI::ShowMouseCursor(ScriptMethodParams &params) {
	AGS3::ShowMouseCursor();
}

void GlobalAPI::SkipUntilCharacterStops(ScriptMethodParams &params) {
	PARAMS1(int, cc);
	AGS3::SkipUntilCharacterStops(cc);
}

void GlobalAPI::StartCutscene(ScriptMethodParams &params) {
	PARAMS1(int, skipwith);
	AGS3::StartCutscene(skipwith);
}

void GlobalAPI::scStartRecording(ScriptMethodParams &params) {
	PARAMS1(int, keyToStop);
	AGS3::scStartRecording(keyToStop);
}

void GlobalAPI::StopAmbientSound(ScriptMethodParams &params) {
	PARAMS1(int, channel);
	AGS3::StopAmbientSound(channel);
}

void GlobalAPI::stop_and_destroy_channel(ScriptMethodParams &params) {
	PARAMS1(int, channel);
	AGS3::stop_and_destroy_channel(channel);
}

void GlobalAPI::StopDialog(ScriptMethodParams &params) {
	AGS3::StopDialog();
}

void GlobalAPI::StopMoving(ScriptMethodParams &params) {
	PARAMS1(int, chaa);
	AGS3::StopMoving(chaa);
}

void GlobalAPI::scr_StopMusic(ScriptMethodParams &params) {
	AGS3::scr_StopMusic();
}

void GlobalAPI::StopObjectMoving(ScriptMethodParams &params) {
	PARAMS1(int, obj);
	AGS3::StopObjectMoving(obj);
}

void GlobalAPI::_sc_strcat(ScriptMethodParams &params) {
	PARAMS2(char *, s1, const char *, s2);
	AGS3::_sc_strcat(s1, s2);
}

void GlobalAPI::ags_stricmp(ScriptMethodParams &params) {
	PARAMS2(const char *, s1, const char *, s2);
	params._result = AGS3::ags_stricmp(s1, s2);
}

void GlobalAPI::strcmp(ScriptMethodParams &params) {
	PARAMS2(const char *, s1, const char *, s2);
	params._result = ::strcmp(s1, s2);
}

void GlobalAPI::StrContains(ScriptMethodParams &params) {
	PARAMS2(const char *, s1, const char *, s2);
	params._result = AGS3::StrContains(s1, s2);
}

void GlobalAPI::_sc_strcpy(ScriptMethodParams &params) {
	PARAMS2(char *, destt, const char *, text);
	AGS3::_sc_strcpy(destt, text);
}

void GlobalAPI::ScPl_sc_sprintf(ScriptMethodParams &params) {
	PARAMS1(char *, destt);
	Common::String texx = params.format(1);

	AGS3::_sc_strcpy(destt, texx.c_str());
}

void GlobalAPI::StrGetCharAt(ScriptMethodParams &params) {
	PARAMS2(const char *, strin, int, posn);
	params._result = AGS3::StrGetCharAt(strin, posn);
}

void GlobalAPI::StringToInt(ScriptMethodParams &params) {
	PARAMS1(const char *, val);
	params._result = AGS3::StringToInt(val);
}

void GlobalAPI::strlen(ScriptMethodParams &params) {
	PARAMS1(const char *, val);
	params._result = ::strlen(val);
}

void GlobalAPI::StrSetCharAt(ScriptMethodParams &params) {
	PARAMS3(char *, strin, int, posn, int, nchar);
	AGS3::StrSetCharAt(strin, posn, nchar);
}

void GlobalAPI::_sc_strlower(ScriptMethodParams &params) {
	PARAMS1(char *, desbuf);
	AGS3::_sc_strlower(desbuf);
}

void GlobalAPI::_sc_strupper(ScriptMethodParams &params) {
	PARAMS1(char *, desbuf);
	AGS3::_sc_strupper(desbuf);
}

void GlobalAPI::TintScreen(ScriptMethodParams &params) {
	PARAMS3(int, red, int, grn, int, blu);
	AGS3::TintScreen(red, grn, blu);
}

void GlobalAPI::UnPauseGame(ScriptMethodParams &params) {
	AGS3::UnPauseGame();
}

void GlobalAPI::update_invorder(ScriptMethodParams &params) {
	AGS3::update_invorder();
}

void GlobalAPI::UpdatePalette(ScriptMethodParams &params) {
	AGS3::UpdatePalette();
}

void GlobalAPI::scrWait(ScriptMethodParams &params) {
	PARAMS1(int, nloops);
	AGS3::scrWait(nloops);
}

void GlobalAPI::WaitKey(ScriptMethodParams &params) {
	PARAMS1(int, nloops);
	params._result = AGS3::WaitKey(nloops);
}

void GlobalAPI::WaitMouseKey(ScriptMethodParams &params) {
	PARAMS1(int, nloops);
	params._result = AGS3::WaitMouseKey(nloops);
}

void GlobalAPI::WaitInput(ScriptMethodParams &params) {
	PARAMS2(int, input_flags, int, nloops);
	params._result = AGS3::WaitInput(input_flags, nloops);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
