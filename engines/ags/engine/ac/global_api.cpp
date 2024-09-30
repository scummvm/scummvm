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
// Script API Functions
//
//=============================================================================

#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"

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
#include "ags/engine/ac/global_inventory_item.h"
#include "ags/engine/ac/global_inv_window.h"
#include "ags/engine/ac/global_label.h"
#include "ags/engine/ac/global_listbox.h"
#include "ags/engine/ac/global_mouse.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_overlay.h"
#include "ags/engine/ac/global_palette.h"
#include "ags/engine/ac/global_parser.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/global_slider.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/global_string.h"
#include "ags/engine/ac/global_textbox.h"
#include "ags/engine/ac/global_timer.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/global_video.h"
#include "ags/engine/ac/global_view_frame.h"
#include "ags/engine/ac/global_viewport.h"
#include "ags/engine/ac/global_walkable_area.h"
#include "ags/engine/ac/global_walk_behind.h"
#include "ags/engine/ac/math.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/parser.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/media/video/video.h"
#include "ags/shared/util/string_compat.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {



// void (char*texx, ...)
RuntimeScriptValue Sc_sc_AbortGame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(_sc_AbortGame, 1);
	_sc_AbortGame(scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void (int inum)
RuntimeScriptValue Sc_add_inventory(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(add_inventory);
}

// void (int charid, int inum)
RuntimeScriptValue Sc_AddInventoryToCharacter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(AddInventoryToCharacter);
}

// void (int guin, int objn, int view, int loop, int speed, int repeat)
RuntimeScriptValue Sc_AnimateButton(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT6(AnimateButton);
}

// void  (int chh, int loopn, int sppd, int rept)
RuntimeScriptValue Sc_AnimateCharacter4(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(AnimateCharacter4);
}

// void (int chh, int loopn, int sppd, int rept, int direction, int blocking)
RuntimeScriptValue Sc_AnimateCharacter6(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT6(AnimateCharacter6);
}

// void (int obn,int loopn,int spdd,int rept)
RuntimeScriptValue Sc_AnimateObject4(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(AnimateObject4);
}

// void (int obn,int loopn,int spdd,int rept, int direction, int blocking)
RuntimeScriptValue Sc_AnimateObject6(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT6(AnimateObject6);
}

// int (int cchar1,int cchar2)
RuntimeScriptValue Sc_AreCharactersColliding(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(AreCharactersColliding);
}

// int (int charid,int objid)
RuntimeScriptValue Sc_AreCharObjColliding(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(AreCharObjColliding);
}

// int (int obj1,int obj2)
RuntimeScriptValue Sc_AreObjectsColliding(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(AreObjectsColliding);
}

// int (int thing1, int thing2)
RuntimeScriptValue Sc_AreThingsOverlapping(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(AreThingsOverlapping);
}

// void  (int value)
RuntimeScriptValue Sc_CallRoomScript(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(CallRoomScript);
}

// int (int cmdd,int datt)
RuntimeScriptValue Sc_cd_manager(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(cd_manager);
}

// void  (int ifn)
RuntimeScriptValue Sc_CentreGUI(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(CentreGUI);
}

// void (int chaa,int vii)
RuntimeScriptValue Sc_ChangeCharacterView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(ChangeCharacterView);
}

extern RuntimeScriptValue Sc_ChangeCursorGraphic(const RuntimeScriptValue *params, int32_t param_count);

extern RuntimeScriptValue Sc_ChangeCursorHotspot(const RuntimeScriptValue *params, int32_t param_count);

// void ()
RuntimeScriptValue Sc_ClaimEvent(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(ClaimEvent);
}

// int (int xx,int yy,int slott,int trans)
RuntimeScriptValue Sc_CreateGraphicOverlay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT4(CreateGraphicOverlay);
}

// int (int xx,int yy,int wii,int fontid,int clr,char*texx, ...)
RuntimeScriptValue Sc_CreateTextOverlay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(CreateTextOverlay, 6);
	return RuntimeScriptValue().SetInt32(
	           CreateTextOverlay(params[0].IValue, params[1].IValue, params[2].IValue,
	                             params[3].IValue, params[4].IValue, scsf_buffer, DISPLAYTEXT_NORMALOVERLAY));
}

// void (int strt,int eend)
RuntimeScriptValue Sc_CyclePalette(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(CyclePalette);
}

// void (int cmdd,int dataa)
RuntimeScriptValue Sc_script_debug(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(script_debug);
}

// void  (int slnum)
RuntimeScriptValue Sc_DeleteSaveSlot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(DeleteSaveSlot);
}

// void  (int gotSlot)
RuntimeScriptValue Sc_free_dynamic_sprite(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(free_dynamic_sprite);
}

extern RuntimeScriptValue Sc_disable_cursor_mode(const RuntimeScriptValue *params, int32_t param_count);

// void (int alsoEffects)
RuntimeScriptValue Sc_DisableGroundLevelAreas(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(DisableGroundLevelAreas);
}

// void (int hsnum)
RuntimeScriptValue Sc_DisableHotspot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(DisableHotspot);
}

// void ()
RuntimeScriptValue Sc_DisableInterface(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(DisableInterface);
}

// void (int hsnum)
RuntimeScriptValue Sc_DisableRegion(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(DisableRegion);
}

// void (char*texx, ...)
RuntimeScriptValue Sc_Display(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(Display, 1);
	DisplaySimple(scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void (int xxp,int yyp,int widd,char*texx, ...)
RuntimeScriptValue Sc_DisplayAt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(DisplayAt, 4);
	DisplayAt(params[0].IValue, params[1].IValue, params[2].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void  (int ypos, char *texx)
RuntimeScriptValue Sc_DisplayAtY(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(DisplayAtY, 2);
	DisplayAtY(params[0].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void (int msnum)
RuntimeScriptValue Sc_DisplayMessage(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(DisplayMessage);
}

// void (int msnum, int ypos)
RuntimeScriptValue Sc_DisplayMessageAtY(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(DisplayMessageAtY);
}

// void (int ypos, int ttexcol, int backcol, char *title, int msgnum)
RuntimeScriptValue Sc_DisplayMessageBar(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3_POBJ_PINT(DisplayMessageBar, const char);
}

// void (int chid,char*texx, ...)
RuntimeScriptValue Sc_sc_displayspeech(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(DisplayAt, 2);
	__sc_displayspeech(params[0].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void  (int xx, int yy, int wii, int aschar, char*spch)
RuntimeScriptValue Sc_DisplaySpeechAt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4_POBJ(DisplaySpeechAt, const char);
}

// int (int charid,char*speel)
RuntimeScriptValue Sc_DisplaySpeechBackground(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT_POBJ(DisplaySpeechBackground, const char);
}

// void (int chid, const char*texx, ...)
RuntimeScriptValue Sc_DisplayThought(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(DisplayThought, 2);
	DisplayThought(params[0].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void (int ypos, int ttexcol, int backcol, char *title, char*texx, ...)
RuntimeScriptValue Sc_DisplayTopBar(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(DisplayTopBar, 5);
	DisplayTopBar(params[0].IValue, params[1].IValue, params[2].IValue, params[3].CStr, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

extern RuntimeScriptValue Sc_enable_cursor_mode(const RuntimeScriptValue *params, int32_t param_count);

// void ()
RuntimeScriptValue Sc_EnableGroundLevelAreas(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(EnableGroundLevelAreas);
}

// void (int hsnum)
RuntimeScriptValue Sc_EnableHotspot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(EnableHotspot);
}

// void ()
RuntimeScriptValue Sc_EnableInterface(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(EnableInterface);
}

// void (int hsnum)
RuntimeScriptValue Sc_EnableRegion(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(EnableRegion);
}

// int  ()
RuntimeScriptValue Sc_EndCutscene(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(EndCutscene);
}

// void (int cha,int toface)
RuntimeScriptValue Sc_FaceCharacter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(FaceCharacter);
}

// void (int cha, int xx, int yy)
RuntimeScriptValue Sc_FaceLocation(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(FaceLocation);
}

// void (int sppd)
RuntimeScriptValue Sc_FadeIn(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(FadeIn);
}

// void (int spdd)
RuntimeScriptValue Sc_FadeOut(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(FadeOut);
}

// void (int handle)
RuntimeScriptValue Sc_FileClose(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(FileClose);
}

// int  (int handle)
RuntimeScriptValue Sc_FileIsEOF(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(FileIsEOF);
}

// int (int handle)
RuntimeScriptValue Sc_FileIsError(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(FileIsError);
}

// int (const char*fnmm, const char* cmode)
RuntimeScriptValue Sc_FileOpenCMode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ2(FileOpenCMode, const char, const char);
}

// void (int handle,char*toread)
RuntimeScriptValue Sc_FileRead(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(FileRead, char);
}

// int (int handle)
RuntimeScriptValue Sc_FileReadInt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(FileReadInt);
}

// int8 (int handle)
RuntimeScriptValue Sc_FileReadRawChar(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(FileReadRawChar);
}

// int (int handle)
RuntimeScriptValue Sc_FileReadRawInt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(FileReadRawInt);
}

// void (int handle, const char *towrite)
RuntimeScriptValue Sc_FileWrite(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(FileWrite, const char);
}

// void (int handle,int into)
RuntimeScriptValue Sc_FileWriteInt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(FileWriteInt);
}

// void (int handle, int chartoWrite)
RuntimeScriptValue Sc_FileWriteRawChar(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(FileWriteRawChar);
}

// void (int handle, const char*towrite)
RuntimeScriptValue Sc_FileWriteRawLine(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(FileWriteRawLine, const char);
}

// int  (const char* GUIName)
RuntimeScriptValue Sc_FindGUIID(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(FindGUIID, const char);
}

// void (int amount)
RuntimeScriptValue Sc_FlipScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(FlipScreen);
}

// int (SCRIPT_FLOAT(value), int roundDirection)
RuntimeScriptValue Sc_FloatToInt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PFLOAT_PINT(FloatToInt);
}

// void (int who, int tofollow)
RuntimeScriptValue Sc_FollowCharacter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(FollowCharacter);
}

// void (int who, int tofollow, int distaway, int eagerness)
RuntimeScriptValue Sc_FollowCharacterEx(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(FollowCharacterEx);
}

// int ()
RuntimeScriptValue Sc_GetBackgroundFrame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetBackgroundFrame);
}

// int (int guin, int objn, int ptype)
RuntimeScriptValue Sc_GetButtonPic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT3(GetButtonPic);
}

// int  (int xx, int yy)
RuntimeScriptValue Sc_GetCharIDAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetCharIDAtScreen);
}

// int  (int cha, const char *property)
RuntimeScriptValue Sc_GetCharacterProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT_POBJ(GetCharacterProperty, const char);
}

// void  (int item, const char *property, char *bufer)
RuntimeScriptValue Sc_GetCharacterPropertyText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ2(GetCharacterPropertyText, const char, char);
}

// int ()
RuntimeScriptValue Sc_GetCurrentMusic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetCurrentMusic);
}

extern RuntimeScriptValue Sc_GetCursorMode(const RuntimeScriptValue *params, int32_t param_count);

// int  (int dlg, int opt)
RuntimeScriptValue Sc_GetDialogOption(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetDialogOption);
}

// int  (int opt)
RuntimeScriptValue Sc_GetGameOption(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetGameOption);
}

// int  (int parm, int data1, int data2, int data3)
RuntimeScriptValue Sc_GetGameParameter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT4(GetGameParameter);
}

// int ()
RuntimeScriptValue Sc_GetGameSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetGameSpeed);
}

// int (int index)
RuntimeScriptValue Sc_GetGlobalInt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetGlobalInt);
}

// void  (int index, char *strval)
RuntimeScriptValue Sc_GetGlobalString(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(GetGlobalString, char);
}

// int  (const char *varName)
RuntimeScriptValue Sc_GetGraphicalVariable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(GetGraphicalVariable, const char);
}

// int  (int xx,int yy)
RuntimeScriptValue Sc_GetGUIAt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetGUIAt);
}

// int  (int xx, int yy)
RuntimeScriptValue Sc_GetGUIObjectAt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetGUIObjectAt);
}

// int (int xxx,int yyy)
RuntimeScriptValue Sc_GetHotspotIDAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetHotspotIDAtScreen);
}

// void (int hotspot, char *buffer)
RuntimeScriptValue Sc_GetHotspotName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(GetHotspotName, char);
}

// int  (int hotspot)
RuntimeScriptValue Sc_GetHotspotPointX(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetHotspotPointX);
}

// int  (int hotspot)
RuntimeScriptValue Sc_GetHotspotPointY(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetHotspotPointY);
}

// int  (int hss, const char *property)
RuntimeScriptValue Sc_GetHotspotProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT_POBJ(GetHotspotProperty, const char);
}

// void  (int item, const char *property, char *bufer)
RuntimeScriptValue Sc_GetHotspotPropertyText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ2(GetHotspotPropertyText, const char, char);
}

// int  (int xxx, int yyy)
RuntimeScriptValue Sc_GetInvAt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetInvAt);
}

// int (int indx)
RuntimeScriptValue Sc_GetInvGraphic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetInvGraphic);
}

// void (int indx,char*buff)
RuntimeScriptValue Sc_GetInvName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(GetInvName, char);
}

// int  (int item, const char *property)
RuntimeScriptValue Sc_GetInvProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT_POBJ(GetInvProperty, const char);
}

// void  (int item, const char *property, char *bufer)
RuntimeScriptValue Sc_GetInvPropertyText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ2(GetInvPropertyText, const char, char);
}

// void (int xxx,int yyy,char*tempo)
RuntimeScriptValue Sc_GetLocationName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2_POBJ(GetLocationName, char);
}

// int (int xxx,int yyy)
RuntimeScriptValue Sc_GetLocationType(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetLocationType);
}

// void  (int msg, char *buffer)
RuntimeScriptValue Sc_GetMessageText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(GetMessageText, char);
}

// int  ()
RuntimeScriptValue Sc_GetMIDIPosition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetMIDIPosition);
}

// int  ()
RuntimeScriptValue Sc_GetMP3PosMillis(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetMP3PosMillis);
}

// int (int xx,int yy)
RuntimeScriptValue Sc_GetObjectIDAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetObjectIDAtScreen);
}

// int (int obn)
RuntimeScriptValue Sc_GetObjectBaseline(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetObjectBaseline);
}

// int (int obn)
RuntimeScriptValue Sc_GetObjectGraphic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetObjectGraphic);
}

// void (int obj, char *buffer)
RuntimeScriptValue Sc_GetObjectName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(GetObjectName, char);
}

// int  (int hss, const char *property)
RuntimeScriptValue Sc_GetObjectProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT_POBJ(GetObjectProperty, const char);
}

// void  (int item, const char *property, char *bufer)
RuntimeScriptValue Sc_GetObjectPropertyText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ2(GetObjectPropertyText, const char, char);
}

// int  (int objj)
RuntimeScriptValue Sc_GetObjectX(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetObjectX);
}

// int  (int objj)
RuntimeScriptValue Sc_GetObjectY(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetObjectY);
}

// int ()
RuntimeScriptValue Sc_GetPlayerCharacter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetPlayerCharacter);
}

// int  ()
RuntimeScriptValue Sc_GetRawTime(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetRawTime);
}

// int  (int xxx, int yyy)
RuntimeScriptValue Sc_GetRegionIDAtRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetRegionIDAtRoom);
}

// void  (const char *property, char *bufer)
RuntimeScriptValue Sc_GetRoomPropertyText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_POBJ2(GetRoomPropertyText, const char, char);
}

// int (int slnum,char*desbuf)
RuntimeScriptValue Sc_GetSaveSlotDescription(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT_POBJ(GetSaveSlotDescription, char);
}

// int  (int x, int y)
RuntimeScriptValue Sc_GetScalingAt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetScalingAt);
}

// int (int guin,int objn)
RuntimeScriptValue Sc_GetSliderValue(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetSliderValue);
}

// void (int guin, int objn, char*txbuf)
RuntimeScriptValue Sc_GetTextBoxText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2_POBJ(GetTextBoxText, char);
}

// int (char *text, int fontnum, int width)
RuntimeScriptValue Sc_GetTextHeight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ_PINT2(GetTextHeight, const char);
}

// int (char *text, int fontnum)
RuntimeScriptValue Sc_GetTextWidth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ_PINT(GetTextWidth, const char);
}

RuntimeScriptValue Sc_GetFontHeight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetFontHeight);
}

RuntimeScriptValue Sc_GetFontLineSpacing(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(GetFontLineSpacing);
}

// int (int whatti)
RuntimeScriptValue Sc_sc_GetTime(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(sc_GetTime);
}

// char * (const char *text)
RuntimeScriptValue Sc_get_translation(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ(const char, _GP(myScriptStringImpl), get_translation, const char);
}

// int  (char* buffer)
RuntimeScriptValue Sc_GetTranslationName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(GetTranslationName, char);
}

// int  ()
RuntimeScriptValue Sc_GetViewportX(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetViewportX);
}

// int  ()
RuntimeScriptValue Sc_GetViewportY(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetViewportY);
}

RuntimeScriptValue Sc_GetWalkableAreaAtRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetWalkableAreaAtRoom);
}

// int (int xxx,int yyy)
RuntimeScriptValue Sc_GetWalkableAreaAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(GetWalkableAreaAtScreen);
}

RuntimeScriptValue Sc_GetDrawingSurfaceForWalkableArea(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptDrawingSurface, GetDrawingSurfaceForWalkableArea);
}

RuntimeScriptValue Sc_GetDrawingSurfaceForWalkbehind(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptDrawingSurface, GetDrawingSurfaceForWalkbehind);
}

// void (int amnt)
RuntimeScriptValue Sc_GiveScore(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(GiveScore);
}

// int (int roomnum)
RuntimeScriptValue Sc_HasPlayerBeenInRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(HasPlayerBeenInRoom);
}

// void  ()
RuntimeScriptValue Sc_HideMouseCursor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(HideMouseCursor);
}

// void (const char*msg,char*bufr)
RuntimeScriptValue Sc_ShowInputBox(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_POBJ2(ShowInputBox, const char, char);
}

// void (int ifn)
RuntimeScriptValue Sc_InterfaceOff(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(InterfaceOff);
}

// void (int ifn)
RuntimeScriptValue Sc_InterfaceOn(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(InterfaceOn);
}

// FLOAT_RETURN_TYPE (int value)
RuntimeScriptValue Sc_IntToFloat(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_FLOAT_PINT(IntToFloat);
}

// void ()
RuntimeScriptValue Sc_sc_invscreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(sc_invscreen);
}

extern RuntimeScriptValue Sc_IsButtonDown(const RuntimeScriptValue *params, int32_t param_count);

// int (int chan)
RuntimeScriptValue Sc_IsChannelPlaying(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsChannelPlaying);
}

// int ()
RuntimeScriptValue Sc_IsGamePaused(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(IsGamePaused);
}

// int  (int guinum)
RuntimeScriptValue Sc_IsGUIOn(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsGUIOn);
}

// int  (int xx,int yy,int mood)
RuntimeScriptValue Sc_IsInteractionAvailable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT3(IsInteractionAvailable);
}

// int  (int item, int mood)
RuntimeScriptValue Sc_IsInventoryInteractionAvailable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(IsInventoryInteractionAvailable);
}

// int ()
RuntimeScriptValue Sc_IsInterfaceEnabled(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(IsInterfaceEnabled);
}

// int  (int keycode)
RuntimeScriptValue Sc_IsKeyPressed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsKeyPressed);
}

// int ()
RuntimeScriptValue Sc_IsMusicPlaying(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(IsMusicPlaying);
}

// int  ()
RuntimeScriptValue Sc_IsMusicVoxAvailable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(IsMusicVoxAvailable);
}

// int (int objj)
RuntimeScriptValue Sc_IsObjectAnimating(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsObjectAnimating);
}

// int (int objj)
RuntimeScriptValue Sc_IsObjectMoving(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsObjectMoving);
}

// int  (int objj)
RuntimeScriptValue Sc_IsObjectOn(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsObjectOn);
}

// int (int ovrid)
RuntimeScriptValue Sc_IsOverlayValid(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsOverlayValid);
}

// int ()
RuntimeScriptValue Sc_IsSoundPlaying(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(IsSoundPlaying);
}

// int (int tnum)
RuntimeScriptValue Sc_IsTimerExpired(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsTimerExpired);
}

// int  ()
RuntimeScriptValue Sc_IsTranslationAvailable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(IsTranslationAvailable);
}

// int ()
RuntimeScriptValue Sc_IsVoxAvailable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(IsVoxAvailable);
}

// void (int guin, int objn, const char*newitem)
RuntimeScriptValue Sc_ListBoxAdd(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2_POBJ(ListBoxAdd, const char);
}

// void (int guin, int objn)
RuntimeScriptValue Sc_ListBoxClear(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(ListBoxClear);
}

// void  (int guin, int objn, const char*filemask)
RuntimeScriptValue Sc_ListBoxDirList(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2_POBJ(ListBoxDirList, const char);
}

// char* (int guin, int objn, int item, char*buffer)
RuntimeScriptValue Sc_ListBoxGetItemText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT3_POBJ(char, _GP(myScriptStringImpl), ListBoxGetItemText, char);
}

// int (int guin, int objn)
RuntimeScriptValue Sc_ListBoxGetNumItems(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(ListBoxGetNumItems);
}

// int (int guin, int objn)
RuntimeScriptValue Sc_ListBoxGetSelected(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(ListBoxGetSelected);
}

// void (int guin, int objn, int itemIndex)
RuntimeScriptValue Sc_ListBoxRemove(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(ListBoxRemove);
}

// int  (int guin, int objn)
RuntimeScriptValue Sc_ListBoxSaveGameList(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(ListBoxSaveGameList);
}

// void (int guin, int objn, int newsel)
RuntimeScriptValue Sc_ListBoxSetSelected(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(ListBoxSetSelected);
}

// void  (int guin, int objn, int item)
RuntimeScriptValue Sc_ListBoxSetTopItem(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(ListBoxSetTopItem);
}

// int (const char *filename)
RuntimeScriptValue Sc_LoadImageFile(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(LoadImageFile, const char);
}

// int (int slnum, int width, int height)
RuntimeScriptValue Sc_LoadSaveSlotScreenshot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT3(LoadSaveSlotScreenshot);
}

// void (int inum)
RuntimeScriptValue Sc_lose_inventory(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(lose_inventory);
}

// void (int charid, int inum)
RuntimeScriptValue Sc_LoseInventoryFromCharacter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(LoseInventoryFromCharacter);
}

// void (int obn)
RuntimeScriptValue Sc_MergeObject(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(MergeObject);
}

// void (int cc,int xx,int yy)
RuntimeScriptValue Sc_MoveCharacter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(MoveCharacter);
}

// void (int chaa,int xx,int yy,int direct)
RuntimeScriptValue Sc_MoveCharacterBlocking(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT4(MoveCharacterBlocking);
}

// void (int cc,int xx, int yy)
RuntimeScriptValue Sc_MoveCharacterDirect(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(MoveCharacterDirect);
}

// void  (int chac, int tox, int toy)
RuntimeScriptValue Sc_MoveCharacterPath(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(MoveCharacterPath);
}

// void (int cc,int xx, int yy)
RuntimeScriptValue Sc_MoveCharacterStraight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(MoveCharacterStraight);
}

// void (int chaa,int hotsp)
RuntimeScriptValue Sc_MoveCharacterToHotspot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(MoveCharacterToHotspot);
}

// void (int chaa,int obbj)
RuntimeScriptValue Sc_MoveCharacterToObject(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(MoveCharacterToObject);
}

// void (int objj,int xx,int yy,int spp)
RuntimeScriptValue Sc_MoveObject(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(MoveObject);
}

// void (int objj,int xx,int yy,int spp)
RuntimeScriptValue Sc_MoveObjectDirect(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(MoveObjectDirect);
}

// void (int ovrid, int newx,int newy)
RuntimeScriptValue Sc_MoveOverlay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(MoveOverlay);
}

// void (int charid)
RuntimeScriptValue Sc_MoveToWalkableArea(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(MoveToWalkableArea);
}

// void (int nrnum)
RuntimeScriptValue Sc_NewRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(NewRoom);
}

// void (int nrnum,int newx,int newy)
RuntimeScriptValue Sc_NewRoomEx(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(NewRoomEx);
}

// void (int charid, int nrnum, int newx, int newy)
RuntimeScriptValue Sc_NewRoomNPC(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(NewRoomNPC);
}

// void (int obn)
RuntimeScriptValue Sc_ObjectOff(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(ObjectOff);
}

// void (int obn)
RuntimeScriptValue Sc_ObjectOn(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(ObjectOn);
}

extern RuntimeScriptValue Sc_ParseText(const RuntimeScriptValue *params, int32_t param_count);

// void ()
RuntimeScriptValue Sc_PauseGame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(PauseGame);
}

// void  (int channel, int sndnum, int vol, int x, int y)
RuntimeScriptValue Sc_PlayAmbientSound(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT5(PlayAmbientSound);
}

// void (int numb,int playflags)
RuntimeScriptValue Sc_PlayFlic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(PlayFlic);
}

// void  (char *filename)
RuntimeScriptValue Sc_PlayMP3File(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_POBJ(PlayMP3File, const char);
}

// void (int newmus)
RuntimeScriptValue Sc_PlayMusicResetQueue(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(PlayMusicResetQueue);
}

// int (int musnum)
RuntimeScriptValue Sc_PlayMusicQueued(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(PlayMusicQueued);
}

// void  (int mnum)
RuntimeScriptValue Sc_PlaySilentMIDI(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(PlaySilentMIDI);
}

// int (int val1)
RuntimeScriptValue Sc_play_sound(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(play_sound);
}

// int (int val1, int channel)
RuntimeScriptValue Sc_PlaySoundEx(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(PlaySoundEx);
}

// void (const char* name, int skip, int flags)
RuntimeScriptValue Sc_PlayVideo(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_POBJ_PINT2(PlayVideo, const char);
}

// void (int dialog)
RuntimeScriptValue Sc_QuitGame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(QuitGame);
}

// int (int upto)
RuntimeScriptValue Sc_Rand(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(__Rand);
}

// void  (int clr)
RuntimeScriptValue Sc_RawClear(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RawClear);
}

// void  (int xx, int yy, int rad)
RuntimeScriptValue Sc_RawDrawCircle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(RawDrawCircle);
}

// void  (int frame, int translev)
RuntimeScriptValue Sc_RawDrawFrameTransparent(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(RawDrawFrameTransparent);
}

// void (int xx, int yy, int slot)
RuntimeScriptValue Sc_RawDrawImage(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(RawDrawImage);
}

// void (int xx, int yy, int slot)
RuntimeScriptValue Sc_RawDrawImageOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(RawDrawImageOffset);
}

// void (int xx, int yy, int gotSlot, int width, int height)
RuntimeScriptValue Sc_RawDrawImageResized(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT5(RawDrawImageResized);
}

// void (int xx, int yy, int slot, int trans)
RuntimeScriptValue Sc_RawDrawImageTransparent(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(RawDrawImageTransparent);
}

// void  (int fromx, int fromy, int tox, int toy)
RuntimeScriptValue Sc_RawDrawLine(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(RawDrawLine);
}

// void (int x1, int y1, int x2, int y2)
RuntimeScriptValue Sc_RawDrawRectangle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(RawDrawRectangle);
}

// void (int x1, int y1, int x2, int y2, int x3, int y3)
RuntimeScriptValue Sc_RawDrawTriangle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT6(RawDrawTriangle);
}

// void  (int xx, int yy, char*texx, ...)
RuntimeScriptValue Sc_RawPrint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(RawPrint, 3);
	RawPrint(params[0].IValue, params[1].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void  (int xx, int yy, int wid, int font, int msgm)
RuntimeScriptValue Sc_RawPrintMessageWrapped(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT5(RawPrintMessageWrapped);
}

// void ()
RuntimeScriptValue Sc_RawRestoreScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(RawRestoreScreen);
}

// void (int red, int green, int blue, int opacity)
RuntimeScriptValue Sc_RawRestoreScreenTinted(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(RawRestoreScreenTinted);
}

// void  ()
RuntimeScriptValue Sc_RawSaveScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(RawSaveScreen);
}

// void  (int clr)
RuntimeScriptValue Sc_RawSetColor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RawSetColor);
}

// void (int red, int grn, int blu)
RuntimeScriptValue Sc_RawSetColorRGB(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(RawSetColorRGB);
}

extern RuntimeScriptValue Sc_RefreshMouse(const RuntimeScriptValue *params, int32_t param_count);

// void (int chat)
RuntimeScriptValue Sc_ReleaseCharacterView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(ReleaseCharacterView);
}

// void ()
RuntimeScriptValue Sc_ReleaseViewport(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(ReleaseViewport);
}

// void (int obj)
RuntimeScriptValue Sc_RemoveObjectTint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RemoveObjectTint);
}

// void (int ovrid)
RuntimeScriptValue Sc_RemoveOverlay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RemoveOverlay);
}

// void (int areanum)
RuntimeScriptValue Sc_RemoveWalkableArea(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RemoveWalkableArea);
}

// void (int nrnum)
RuntimeScriptValue Sc_ResetRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(ResetRoom);
}

// void ()
RuntimeScriptValue Sc_restart_game(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(restart_game);
}

// void ()
RuntimeScriptValue Sc_restore_game_dialog(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(restore_game_dialog);
}

// void (int slnum)
RuntimeScriptValue Sc_RestoreGameSlot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RestoreGameSlot);
}

// void (int areanum)
RuntimeScriptValue Sc_RestoreWalkableArea(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RestoreWalkableArea);
}

// int  (char *newgame, unsigned int mode, int data)
RuntimeScriptValue Sc_RunAGSGame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ_PINT2(RunAGSGame, const char);
}

// void  (int cc, int mood)
RuntimeScriptValue Sc_RunCharacterInteraction(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(RunCharacterInteraction);
}

// void (int tum)
RuntimeScriptValue Sc_RunDialog(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(RunDialog);
}

// void  (int hotspothere, int mood)
RuntimeScriptValue Sc_RunHotspotInteraction(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(RunHotspotInteraction);
}

// void  (int iit, int modd)
RuntimeScriptValue Sc_RunInventoryInteraction(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(RunInventoryInteraction);
}

// void  (int aa, int mood)
RuntimeScriptValue Sc_RunObjectInteraction(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(RunObjectInteraction);
}

// void  (int regnum, int mood)
RuntimeScriptValue Sc_RunRegionInteraction(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(RunRegionInteraction);
}

extern RuntimeScriptValue Sc_Said(const RuntimeScriptValue *params, int32_t param_count);

// int  (char*buffer)
RuntimeScriptValue Sc_SaidUnknownWord(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(SaidUnknownWord, char);
}

extern RuntimeScriptValue Sc_SaveCursorForLocationChange(const RuntimeScriptValue *params, int32_t param_count);

// void ()
RuntimeScriptValue Sc_save_game_dialog(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(save_game_dialog);
}

// void (int slotn, const char*descript)
RuntimeScriptValue Sc_save_game(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(save_game, const char);
}

// int (char*namm)
RuntimeScriptValue Sc_SaveScreenShot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(SaveScreenShot, const char);
}

// void  (int position)
RuntimeScriptValue Sc_SeekMIDIPosition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SeekMIDIPosition);
}

// void (int patnum)
RuntimeScriptValue Sc_SeekMODPattern(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SeekMODPattern);
}

// void  (int posn)
RuntimeScriptValue Sc_SeekMP3PosMillis(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SeekMP3PosMillis);
}

// void (int iit)
RuntimeScriptValue Sc_SetActiveInventory(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetActiveInventory);
}

// void  (int red, int green, int blue, int opacity, int luminance)
RuntimeScriptValue Sc_SetAmbientTint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT5(SetAmbientTint);
}

RuntimeScriptValue Sc_SetAmbientLightLevel(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetAmbientLightLevel);
}

// void (int area, int brightness)
RuntimeScriptValue Sc_SetAreaLightLevel(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetAreaLightLevel);
}

// void (int area, int min, int max)
RuntimeScriptValue Sc_SetAreaScaling(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetAreaScaling);
}

// void (int frnum)
RuntimeScriptValue Sc_SetBackgroundFrame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetBackgroundFrame);
}

// void (int guin,int objn,int ptype,int slotn)
RuntimeScriptValue Sc_SetButtonPic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetButtonPic);
}

// void (int guin,int objn,char*newtx)
RuntimeScriptValue Sc_SetButtonText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2_POBJ(SetButtonText, const char);
}

// void (int chan, int newvol)
RuntimeScriptValue Sc_SetChannelVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetChannelVolume);
}

// void  (int obn, int basel)
RuntimeScriptValue Sc_SetCharacterBaseline(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterBaseline);
}

// void  (int cha, int clik)
RuntimeScriptValue Sc_SetCharacterClickable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterClickable);
}

// void (int chaa, int view, int loop, int frame)
RuntimeScriptValue Sc_SetCharacterFrame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetCharacterFrame);
}

// void (int who, int iview, int itime)
RuntimeScriptValue Sc_SetCharacterIdle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetCharacterIdle);
}

// void  (int who, int yesorno)
RuntimeScriptValue Sc_SetCharacterIgnoreLight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterIgnoreLight);
}

// void  (int cha, int clik)
RuntimeScriptValue Sc_SetCharacterIgnoreWalkbehinds(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterIgnoreWalkbehinds);
}

// void  (int who, int flag, int yesorno)
RuntimeScriptValue Sc_SetCharacterProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetCharacterProperty);
}

// void  (int chaa, int vii, int intrv)
RuntimeScriptValue Sc_SetCharacterBlinkView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetCharacterBlinkView);
}

// void  (int chaa, int vii)
RuntimeScriptValue Sc_SetCharacterSpeechView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterSpeechView);
}

// void (int chaa,int nspeed)
RuntimeScriptValue Sc_SetCharacterSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterSpeed);
}

// void (int chaa, int xspeed, int yspeed)
RuntimeScriptValue Sc_SetCharacterSpeedEx(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetCharacterSpeedEx);
}

// void (int obn,int trans)
RuntimeScriptValue Sc_SetCharacterTransparency(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterTransparency);
}

// void (int chaa,int vii)
RuntimeScriptValue Sc_SetCharacterView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetCharacterView);
}

// void  (int chaa, int vii, int loop, int align)
RuntimeScriptValue Sc_SetCharacterViewEx(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetCharacterViewEx);
}

// void  (int chaa, int vii, int xoffs, int yoffs)
RuntimeScriptValue Sc_SetCharacterViewOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetCharacterViewOffset);
}

extern RuntimeScriptValue Sc_set_cursor_mode(const RuntimeScriptValue *params, int32_t param_count);
extern RuntimeScriptValue Sc_set_default_cursor(const RuntimeScriptValue *params, int32_t param_count);

// void (int dlg,int opt,int onoroff)
RuntimeScriptValue Sc_SetDialogOption(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetDialogOption);
}

// void  (int newvol)
RuntimeScriptValue Sc_SetDigitalMasterVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetDigitalMasterVolume);
}

// void (int red, int green, int blue)
RuntimeScriptValue Sc_SetFadeColor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetFadeColor);
}

// void  (int vii, int loop, int frame, int sound)
RuntimeScriptValue Sc_SetFrameSound(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetFrameSound);
}

// int  (int opt, int setting)
RuntimeScriptValue Sc_SetGameOption(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(SetGameOption);
}

// void (int newspd)
RuntimeScriptValue Sc_SetGameSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetGameSpeed);
}

// void (int index,int valu)
RuntimeScriptValue Sc_SetGlobalInt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetGlobalInt);
}

extern RuntimeScriptValue Sc_SetGlobalString(const RuntimeScriptValue *params, int32_t param_count);

// void  (const char *varName, int p_value)
RuntimeScriptValue Sc_SetGraphicalVariable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_POBJ_PINT(SetGraphicalVariable, const char);
}

// void  (int guin, int slotn)
RuntimeScriptValue Sc_SetGUIBackgroundPic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetGUIBackgroundPic);
}

// void (int guin, int clickable)
RuntimeScriptValue Sc_SetGUIClickable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetGUIClickable);
}

// void (int guin, int objn, int enabled)
RuntimeScriptValue Sc_SetGUIObjectEnabled(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetGUIObjectEnabled);
}

// void (int guin, int objn, int xx, int yy)
RuntimeScriptValue Sc_SetGUIObjectPosition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetGUIObjectPosition);
}

// void (int ifn, int objn, int newwid, int newhit)
RuntimeScriptValue Sc_SetGUIObjectSize(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetGUIObjectSize);
}

// void (int ifn,int xx,int yy)
RuntimeScriptValue Sc_SetGUIPosition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetGUIPosition);
}

// void  (int ifn, int widd, int hitt)
RuntimeScriptValue Sc_SetGUISize(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetGUISize);
}

// void (int ifn, int trans)
RuntimeScriptValue Sc_SetGUITransparency(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetGUITransparency);
}

// void (int guin, int z)
RuntimeScriptValue Sc_SetGUIZOrder(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetGUIZOrder);
}

// void (int invi, const char *newName)
RuntimeScriptValue Sc_SetInvItemName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(SetInvItemName, const char);
}

// void (int invi, int piccy)
RuntimeScriptValue Sc_set_inv_item_pic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(set_inv_item_pic);
}

// void (int ww,int hh)
RuntimeScriptValue Sc_SetInvDimensions(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetInvDimensions);
}

// void (int guin,int objn, int colr)
RuntimeScriptValue Sc_SetLabelColor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetLabelColor);
}

// void (int guin,int objn, int fontnum)
RuntimeScriptValue Sc_SetLabelFont(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetLabelFont);
}

// void (int guin,int objn,char*newtx)
RuntimeScriptValue Sc_SetLabelText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2_POBJ(SetLabelText, const char);
}

extern RuntimeScriptValue Sc_SetMouseBounds(const RuntimeScriptValue *params, int32_t param_count);
extern RuntimeScriptValue Sc_set_mouse_cursor(const RuntimeScriptValue *params, int32_t param_count);
extern RuntimeScriptValue Sc_SetMousePosition(const RuntimeScriptValue *params, int32_t param_count);

// void  (int mode)
RuntimeScriptValue Sc_SetMultitasking(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetMultitasking);
}

// void (int newvol)
RuntimeScriptValue Sc_SetMusicMasterVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetMusicMasterVolume);
}

// void (int loopflag)
RuntimeScriptValue Sc_SetMusicRepeat(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetMusicRepeat);
}

// void (int newvol)
RuntimeScriptValue Sc_SetMusicVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetMusicVolume);
}

extern RuntimeScriptValue Sc_SetNextCursor(const RuntimeScriptValue *params, int32_t param_count);

// void (int newtrans)
RuntimeScriptValue Sc_SetNextScreenTransition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetNextScreenTransition);
}

extern RuntimeScriptValue Sc_SetNormalFont(const RuntimeScriptValue *params, int32_t param_count);

// void  (int obn, int basel)
RuntimeScriptValue Sc_SetObjectBaseline(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetObjectBaseline);
}

// void  (int cha, int clik)
RuntimeScriptValue Sc_SetObjectClickable(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetObjectClickable);
}

// void (int obn,int viw,int lop,int fra)
RuntimeScriptValue Sc_SetObjectFrame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetObjectFrame);
}

// void (int obn,int slott)
RuntimeScriptValue Sc_SetObjectGraphic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetObjectGraphic);
}

// void  (int cha, int clik)
RuntimeScriptValue Sc_SetObjectIgnoreWalkbehinds(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetObjectIgnoreWalkbehinds);
}

// void (int objj, int tox, int toy)
RuntimeScriptValue Sc_SetObjectPosition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetObjectPosition);
}

// void (int obj, int red, int green, int blue, int opacity, int luminance)
RuntimeScriptValue Sc_SetObjectTint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT6(SetObjectTint);
}

// void (int obn,int trans)
RuntimeScriptValue Sc_SetObjectTransparency(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetObjectTransparency);
}

// void (int obn,int vii)
RuntimeScriptValue Sc_SetObjectView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetObjectView);
}

// void (int inndx,int rr,int gg,int bb)
RuntimeScriptValue Sc_SetPalRGB(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetPalRGB);
}

// void (int newchar)
RuntimeScriptValue Sc_SetPlayerCharacter(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetPlayerCharacter);
}

// void  (int area, int red, int green, int blue, int amount)
RuntimeScriptValue Sc_SetRegionTint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT5(SetRegionTint);
}

// void ()
RuntimeScriptValue Sc_SetRestartPoint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(SetRestartPoint);
}

// void (int newtrans)
RuntimeScriptValue Sc_SetScreenTransition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetScreenTransition);
}

// void  (int newval)
RuntimeScriptValue Sc_SetSkipSpeech(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_PARAM_COUNT(SetSkipSpeech, 1);
	SetSkipSpeech((SkipSpeechStyle)params[0].IValue);
	return RuntimeScriptValue((int32_t)0);
}

// void (int guin,int objn, int valn)
RuntimeScriptValue Sc_SetSliderValue(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetSliderValue);
}

// void (int newvol)
RuntimeScriptValue Sc_SetSoundVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetSoundVolume);
}

extern RuntimeScriptValue Sc_SetSpeechFont(const RuntimeScriptValue *params, int32_t param_count);

// void  (int newstyle)
RuntimeScriptValue Sc_SetSpeechStyle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetSpeechStyle);
}

// void (int newvol)
RuntimeScriptValue Sc_SetSpeechVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetSpeechVolume);
}

// void (int chaa,int ncol)
RuntimeScriptValue Sc_SetTalkingColor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetTalkingColor);
}

// void (int guin,int objn, int fontnum)
RuntimeScriptValue Sc_SetTextBoxFont(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(SetTextBoxFont);
}

// void (int guin, int objn, char*txbuf)
RuntimeScriptValue Sc_SetTextBoxText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2_POBJ(SetTextBoxText, const char);
}

// void (int ovrid,int xx,int yy,int wii,int fontid,int clr,char*texx,...)
RuntimeScriptValue Sc_SetTextOverlay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(SetTextOverlay, 7);
	SetTextOverlay(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue,
	               params[4].IValue, params[5].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

// void  (int guinum)
RuntimeScriptValue Sc_SetTextWindowGUI(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetTextWindowGUI);
}

// void (int tnum,int timeout)
RuntimeScriptValue Sc_script_SetTimer(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(script_SetTimer);
}

// void (int offsx,int offsy)
RuntimeScriptValue Sc_SetViewport(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetViewport);
}

// void  (int newmod)
RuntimeScriptValue Sc_SetVoiceMode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetVoiceMode);
}

// void (int wa,int bl)
RuntimeScriptValue Sc_SetWalkBehindBase(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetWalkBehindBase);
}

// void (int severe)
RuntimeScriptValue Sc_ShakeScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(ShakeScreen);
}

// void  (int delay, int amount, int length)
RuntimeScriptValue Sc_ShakeScreenBackground(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(ShakeScreenBackground);
}

// void  ()
RuntimeScriptValue Sc_ShowMouseCursor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(ShowMouseCursor);
}

RuntimeScriptValue Sc_SkipCutscene(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(SkipCutscene);
}

// void (int cc)
RuntimeScriptValue Sc_SkipUntilCharacterStops(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SkipUntilCharacterStops);
}

// void  (int skipwith)
RuntimeScriptValue Sc_StartCutscene(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(StartCutscene);
}

// void  (int keyToStop)
RuntimeScriptValue Sc_scStartRecording(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(scStartRecording);
}

// void  (int channel)
RuntimeScriptValue Sc_StopAmbientSound(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(StopAmbientSound);
}

// void  (int chid)
RuntimeScriptValue Sc_stop_and_destroy_channel(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(stop_and_destroy_channel);
}

// void ()
RuntimeScriptValue Sc_StopDialog(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(StopDialog);
}

// void (int chaa)
RuntimeScriptValue Sc_StopMoving(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(StopMoving);
}

// void ()
RuntimeScriptValue Sc_scr_StopMusic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(scr_StopMusic);
}

// void (int objj)
RuntimeScriptValue Sc_StopObjectMoving(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(StopObjectMoving);
}

// void (char*s1,char*s2)
RuntimeScriptValue Sc_sc_strcat(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_PARAM_COUNT(_sc_strcat, 2);
	_sc_strcat((char *)params[0].Ptr, (const char *)params[1].Ptr);
	// NOTE: tests with old (<= 2.60) AGS show that StrCat returned the second string
	// (could be result of UB, but we are doing this for more accurate emulation)
	return params[1];
}

RuntimeScriptValue Sc_stricmp(const RuntimeScriptValue *params, int32_t param_count) {
	// Calling C stdlib function ags_stricmp
	API_SCALL_INT_POBJ2(ags_stricmp, const char, const char);
}

RuntimeScriptValue Sc_strcmp(const RuntimeScriptValue *params, int32_t param_count) {
	// Calling C stdlib function strcmp
	API_SCALL_INT_POBJ2(strcmp, const char, const char);
}

// int  (const char *s1, const char *s2)
RuntimeScriptValue Sc_StrContains(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ2(StrContains, const char, const char);
}

// void (char*s1, const char*s2);
RuntimeScriptValue Sc_sc_strcpy(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_PARAM_COUNT(_sc_strcpy, 2);
	_sc_strcpy((char *)params[0].Ptr, (const char *)params[1].Ptr);
	return params[0];
}

// void (char*destt, const char*texx, ...);
RuntimeScriptValue Sc_sc_sprintf(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(_sc_sprintf, 2);
	_sc_strcpy(params[0].CStr, scsf_buffer);
	return params[0];
}

// int  (char *strin, int posn)
RuntimeScriptValue Sc_StrGetCharAt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ_PINT(StrGetCharAt, const char);
}

// int (const char*stino)
RuntimeScriptValue Sc_StringToInt(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(StringToInt, const char);
}

RuntimeScriptValue Sc_strlen(const RuntimeScriptValue *params, int32_t param_count) {
	// Calling C stdlib function strlen
	API_SCALL_INT_POBJ(strlen, const char);
}

// void  (char *strin, int posn, int nchar)
RuntimeScriptValue Sc_StrSetCharAt(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_PARAM_COUNT(StrSetCharAt, 3);
	StrSetCharAt((char *)params[0].Ptr, params[1].IValue, params[2].IValue);
	return params[0];
}

// void  (char *desbuf)
RuntimeScriptValue Sc_sc_strlower(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_PARAM_COUNT(_sc_strlower, 1);
	_sc_strlower((char *)params[0].Ptr);
	return params[0];
}

// void  (char *desbuf)
RuntimeScriptValue Sc_sc_strupper(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_PARAM_COUNT(_sc_strupper, 1);
	_sc_strupper((char *)params[0].Ptr);
	return params[0];
}

// void (int red, int grn, int blu)
RuntimeScriptValue Sc_TintScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(TintScreen);
}

// void ()
RuntimeScriptValue Sc_UnPauseGame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(UnPauseGame);
}

// void ()
RuntimeScriptValue Sc_update_invorder(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(update_invorder);
}

// void ()
RuntimeScriptValue Sc_UpdatePalette(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(UpdatePalette);
}

// void (int nloops)
RuntimeScriptValue Sc_scrWait(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(scrWait);
}

// int (int nloops)
RuntimeScriptValue Sc_WaitKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(WaitKey);
}

RuntimeScriptValue Sc_WaitMouse(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(WaitMouse);
}

// int (int nloops)
RuntimeScriptValue Sc_WaitMouseKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(WaitMouseKey);
}

// int (int input_flags, int nloops)
RuntimeScriptValue Sc_WaitInput(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(WaitInput);
}

RuntimeScriptValue Sc_SkipWait(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(SkipWait);
}

void RegisterGlobalAPI() {
	ScFnRegister global_api[] = {
		{"AbortGame", Sc_sc_AbortGame},
		{"AddInventory", API_FN_PAIR(add_inventory)},
		{"AddInventoryToCharacter", API_FN_PAIR(AddInventoryToCharacter)},
		{"AnimateButton", API_FN_PAIR(AnimateButton)},
		{"AnimateCharacter", API_FN_PAIR(AnimateCharacter4)},
		{"AnimateCharacterEx", API_FN_PAIR(AnimateCharacter6)},
		{"AnimateObject", API_FN_PAIR(AnimateObject4)},
		{"AnimateObjectEx", API_FN_PAIR(AnimateObject6)},
		{"AreCharactersColliding", API_FN_PAIR(AreCharactersColliding)},
		{"AreCharObjColliding", API_FN_PAIR(AreCharObjColliding)},
		{"AreObjectsColliding", API_FN_PAIR(AreObjectsColliding)},
		{"AreThingsOverlapping", API_FN_PAIR(AreThingsOverlapping)},
		{"CallRoomScript", API_FN_PAIR(CallRoomScript)},
		{"CDAudio", API_FN_PAIR(cd_manager)},
		{"CentreGUI", API_FN_PAIR(CentreGUI)},
		{"ChangeCharacterView", API_FN_PAIR(ChangeCharacterView)},
		{"ChangeCursorGraphic", API_FN_PAIR(ChangeCursorGraphic)},
		{"ChangeCursorHotspot", API_FN_PAIR(ChangeCursorHotspot)},
		{"ClaimEvent", API_FN_PAIR(ClaimEvent)},
		{"CreateGraphicOverlay", API_FN_PAIR(CreateGraphicOverlay)},
		{"CreateTextOverlay", Sc_CreateTextOverlay},
		{"CyclePalette", API_FN_PAIR(CyclePalette)},
		{"Debug", API_FN_PAIR(script_debug)},
		{"DeleteSaveSlot", API_FN_PAIR(DeleteSaveSlot)},
		{"DeleteSprite", API_FN_PAIR(free_dynamic_sprite)},
		{"DisableCursorMode", API_FN_PAIR(disable_cursor_mode)},
		{"DisableGroundLevelAreas", API_FN_PAIR(DisableGroundLevelAreas)},
		{"DisableHotspot", API_FN_PAIR(DisableHotspot)},
		{"DisableInterface", API_FN_PAIR(DisableInterface)},
		{"DisableRegion", API_FN_PAIR(DisableRegion)},
		{"Display", Sc_Display},
		{"DisplayAt", Sc_DisplayAt},
		// CHECKME: this function was non-variadic prior to 3.6.1, but AGS compiler does
		// not produce "name^argnum" symbol id for non-member functions for some reason :/
		// do we have to do anything about this here? like, test vs script API version...
		{"DisplayAtY", Sc_DisplayAtY},
		{"DisplayMessage", API_FN_PAIR(DisplayMessage)},
		{"DisplayMessageAtY", API_FN_PAIR(DisplayMessageAtY)},
		{"DisplayMessageBar", API_FN_PAIR(DisplayMessageBar)},
		{"DisplaySpeech", Sc_sc_displayspeech},
		{"DisplaySpeechAt", API_FN_PAIR(DisplaySpeechAt)},
		{"DisplaySpeechBackground", API_FN_PAIR(DisplaySpeechBackground)},
		{"DisplayThought", Sc_DisplayThought},
		{"DisplayTopBar", Sc_DisplayTopBar},
		{"EnableCursorMode", API_FN_PAIR(enable_cursor_mode)},
		{"EnableGroundLevelAreas", API_FN_PAIR(EnableGroundLevelAreas)},
		{"EnableHotspot", API_FN_PAIR(EnableHotspot)},
		{"EnableInterface", API_FN_PAIR(EnableInterface)},
		{"EnableRegion", API_FN_PAIR(EnableRegion)},
		{"EndCutscene", API_FN_PAIR(EndCutscene)},
		{"FaceCharacter", API_FN_PAIR(FaceCharacter)},
		{"FaceLocation", API_FN_PAIR(FaceLocation)},
		{"FadeIn", API_FN_PAIR(FadeIn)},
		{"FadeOut", API_FN_PAIR(FadeOut)},
		{"FileClose", API_FN_PAIR(FileClose)},
		{"FileIsEOF", API_FN_PAIR(FileIsEOF)},
		{"FileIsError", API_FN_PAIR(FileIsError)},
		// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
		{"FileOpen", API_FN_PAIR(FileOpenCMode)},
		{"FileRead", API_FN_PAIR(FileRead)},
		{"FileReadInt", API_FN_PAIR(FileReadInt)},
		{"FileReadRawChar", API_FN_PAIR(FileReadRawChar)},
		{"FileReadRawInt", API_FN_PAIR(FileReadRawInt)},
		{"FileWrite", API_FN_PAIR(FileWrite)},
		{"FileWriteInt", API_FN_PAIR(FileWriteInt)},
		{"FileWriteRawChar", API_FN_PAIR(FileWriteRawChar)},
		{"FileWriteRawLine", API_FN_PAIR(FileWriteRawLine)},
		{"FindGUIID", API_FN_PAIR(FindGUIID)},
		{"FlipScreen", API_FN_PAIR(FlipScreen)},
		{"FloatToInt", API_FN_PAIR(FloatToInt)},
		{"FollowCharacter", API_FN_PAIR(FollowCharacter)},
		{"FollowCharacterEx", API_FN_PAIR(FollowCharacterEx)},
		{"GetBackgroundFrame", API_FN_PAIR(GetBackgroundFrame)},
		{"GetButtonPic", API_FN_PAIR(GetButtonPic)},
		{"GetCharacterAt", API_FN_PAIR(GetCharIDAtScreen)},
		{"GetCharacterProperty", API_FN_PAIR(GetCharacterProperty)},
		{"GetCharacterPropertyText", API_FN_PAIR(GetCharacterPropertyText)},
		{"GetCurrentMusic", API_FN_PAIR(GetCurrentMusic)},
		{"GetCursorMode", API_FN_PAIR(GetCursorMode)},
		{"GetDialogOption", API_FN_PAIR(GetDialogOption)},
		{"GetGameOption", API_FN_PAIR(GetGameOption)},
		{"GetGameParameter", API_FN_PAIR(GetGameParameter)},
		{"GetGameSpeed", API_FN_PAIR(GetGameSpeed)},
		{"GetGlobalInt", API_FN_PAIR(GetGlobalInt)},
		{"GetGlobalString", API_FN_PAIR(GetGlobalString)},
		{"GetGraphicalVariable", API_FN_PAIR(GetGraphicalVariable)},
		{"GetGUIAt", API_FN_PAIR(GetGUIAt)},
		{"GetGUIObjectAt", API_FN_PAIR(GetGUIObjectAt)},
		{"GetHotspotAt", API_FN_PAIR(GetHotspotIDAtScreen)},
		{"GetHotspotName", API_FN_PAIR(GetHotspotName)},
		{"GetHotspotPointX", API_FN_PAIR(GetHotspotPointX)},
		{"GetHotspotPointY", API_FN_PAIR(GetHotspotPointY)},
		{"GetHotspotProperty", API_FN_PAIR(GetHotspotProperty)},
		{"GetHotspotPropertyText", API_FN_PAIR(GetHotspotPropertyText)},
		{"GetInvAt", API_FN_PAIR(GetInvAt)},
		{"GetInvGraphic", API_FN_PAIR(GetInvGraphic)},
		{"GetInvName", API_FN_PAIR(GetInvName)},
		{"GetInvProperty", API_FN_PAIR(GetInvProperty)},
		{"GetInvPropertyText", API_FN_PAIR(GetInvPropertyText)},
		{"GetLocationName", API_FN_PAIR(GetLocationName)},
		{"GetLocationType", API_FN_PAIR(GetLocationType)},
		{"GetMessageText", API_FN_PAIR(GetMessageText)},
		{"GetMIDIPosition", API_FN_PAIR(GetMIDIPosition)},
		{"GetMP3PosMillis", API_FN_PAIR(GetMP3PosMillis)},
		{"GetObjectAt", API_FN_PAIR(GetObjectIDAtScreen)},
		{"GetObjectBaseline", API_FN_PAIR(GetObjectBaseline)},
		{"GetObjectGraphic", API_FN_PAIR(GetObjectGraphic)},
		{"GetObjectName", API_FN_PAIR(GetObjectName)},
		{"GetObjectProperty", API_FN_PAIR(GetObjectProperty)},
		{"GetObjectPropertyText", API_FN_PAIR(GetObjectPropertyText)},
		{"GetObjectX", API_FN_PAIR(GetObjectX)},
		{"GetObjectY", API_FN_PAIR(GetObjectY)},
		{"GetPlayerCharacter", API_FN_PAIR(GetPlayerCharacter)},
		{"GetRawTime", API_FN_PAIR(GetRawTime)},
		{"GetRegionAt", API_FN_PAIR(GetRegionIDAtRoom)},
		{"GetRoomProperty", API_FN_PAIR(Room_GetProperty)},
		{"GetRoomPropertyText", API_FN_PAIR(GetRoomPropertyText)},
		{"GetSaveSlotDescription", API_FN_PAIR(GetSaveSlotDescription)},
		{"GetScalingAt", API_FN_PAIR(GetScalingAt)},
		{"GetSliderValue", API_FN_PAIR(GetSliderValue)},
		{"GetTextBoxText", API_FN_PAIR(GetTextBoxText)},
		{"GetTextHeight", API_FN_PAIR(GetTextHeight)},
		{"GetTextWidth", API_FN_PAIR(GetTextWidth)},
		{"GetFontHeight", API_FN_PAIR(GetFontHeight)},
		{"GetFontLineSpacing", API_FN_PAIR(GetFontLineSpacing)},
		{"GetTime", API_FN_PAIR(sc_GetTime)},
		{"GetTranslation", API_FN_PAIR(get_translation)},
		{"GetTranslationName", API_FN_PAIR(GetTranslationName)},
		{"GetViewportX", API_FN_PAIR(GetViewportX)},
		{"GetViewportY", API_FN_PAIR(GetViewportY)},
		{"GetWalkableAreaAtRoom", API_FN_PAIR(GetWalkableAreaAtRoom)},
		{"GetWalkableAreaAt", API_FN_PAIR(GetWalkableAreaAtScreen)},
		{"GetWalkableAreaAtScreen", API_FN_PAIR(GetWalkableAreaAtScreen)},
		{"GetDrawingSurfaceForWalkableArea", API_FN_PAIR(GetDrawingSurfaceForWalkableArea)},
		{"GetDrawingSurfaceForWalkbehind", API_FN_PAIR(GetDrawingSurfaceForWalkbehind)},
		{"GiveScore", API_FN_PAIR(GiveScore)},
		{"HasPlayerBeenInRoom", API_FN_PAIR(HasPlayerBeenInRoom)},
		{"HideMouseCursor", API_FN_PAIR(HideMouseCursor)},
		{"InputBox", API_FN_PAIR(ShowInputBox)},
		{"InterfaceOff", API_FN_PAIR(InterfaceOff)},
		{"InterfaceOn", API_FN_PAIR(InterfaceOn)},
		{"IntToFloat", API_FN_PAIR(IntToFloat)},
		{"InventoryScreen", API_FN_PAIR(sc_invscreen)},
		{"IsButtonDown", API_FN_PAIR(IsButtonDown)},
		{"IsChannelPlaying", API_FN_PAIR(IsChannelPlaying)},
		{"IsGamePaused", API_FN_PAIR(IsGamePaused)},
		{"IsGUIOn", API_FN_PAIR(IsGUIOn)},
		{"IsInteractionAvailable", API_FN_PAIR(IsInteractionAvailable)},
		{"IsInventoryInteractionAvailable", API_FN_PAIR(IsInventoryInteractionAvailable)},
		{"IsInterfaceEnabled", API_FN_PAIR(IsInterfaceEnabled)},
		{"IsKeyPressed", API_FN_PAIR(IsKeyPressed)},
		{"IsMusicPlaying", API_FN_PAIR(IsMusicPlaying)},
		{"IsMusicVoxAvailable", API_FN_PAIR(IsMusicVoxAvailable)},
		{"IsObjectAnimating", API_FN_PAIR(IsObjectAnimating)},
		{"IsObjectMoving", API_FN_PAIR(IsObjectMoving)},
		{"IsObjectOn", API_FN_PAIR(IsObjectOn)},
		{"IsOverlayValid", API_FN_PAIR(IsOverlayValid)},
		{"IsSoundPlaying", API_FN_PAIR(IsSoundPlaying)},
		{"IsTimerExpired", API_FN_PAIR(IsTimerExpired)},
		{"IsTranslationAvailable", API_FN_PAIR(IsTranslationAvailable)},
		{"IsVoxAvailable", API_FN_PAIR(IsVoxAvailable)},
		{"ListBoxAdd", API_FN_PAIR(ListBoxAdd)},
		{"ListBoxClear", API_FN_PAIR(ListBoxClear)},
		{"ListBoxDirList", API_FN_PAIR(ListBoxDirList)},
		{"ListBoxGetItemText", API_FN_PAIR(ListBoxGetItemText)},
		{"ListBoxGetNumItems", API_FN_PAIR(ListBoxGetNumItems)},
		{"ListBoxGetSelected", API_FN_PAIR(ListBoxGetSelected)},
		{"ListBoxRemove", API_FN_PAIR(ListBoxRemove)},
		{"ListBoxSaveGameList", API_FN_PAIR(ListBoxSaveGameList)},
		{"ListBoxSetSelected", API_FN_PAIR(ListBoxSetSelected)},
		{"ListBoxSetTopItem", API_FN_PAIR(ListBoxSetTopItem)},
		{"LoadImageFile", API_FN_PAIR(LoadImageFile)},
		{"LoadSaveSlotScreenshot", API_FN_PAIR(LoadSaveSlotScreenshot)},
		{"LoseInventory", API_FN_PAIR(lose_inventory)},
		{"LoseInventoryFromCharacter", API_FN_PAIR(LoseInventoryFromCharacter)},
		{"MergeObject", API_FN_PAIR(MergeObject)},
		{"MoveCharacter", API_FN_PAIR(MoveCharacter)},
		{"MoveCharacterBlocking", API_FN_PAIR(MoveCharacterBlocking)},
		{"MoveCharacterDirect", API_FN_PAIR(MoveCharacterDirect)},
		{"MoveCharacterPath", API_FN_PAIR(MoveCharacterPath)},
		{"MoveCharacterStraight", API_FN_PAIR(MoveCharacterStraight)},
		{"MoveCharacterToHotspot", API_FN_PAIR(MoveCharacterToHotspot)},
		{"MoveCharacterToObject", API_FN_PAIR(MoveCharacterToObject)},
		{"MoveObject", API_FN_PAIR(MoveObject)},
		{"MoveObjectDirect", API_FN_PAIR(MoveObjectDirect)},
		{"MoveOverlay", API_FN_PAIR(MoveOverlay)},
		{"MoveToWalkableArea", API_FN_PAIR(MoveToWalkableArea)},
		{"NewRoom", API_FN_PAIR(NewRoom)},
		{"NewRoomEx", API_FN_PAIR(NewRoomEx)},
		{"NewRoomNPC", API_FN_PAIR(NewRoomNPC)},
		{"ObjectOff", API_FN_PAIR(ObjectOff)},
		{"ObjectOn", API_FN_PAIR(ObjectOn)},
		{"ParseText", API_FN_PAIR(ParseText)},
		{"PauseGame", API_FN_PAIR(PauseGame)},
		{"PlayAmbientSound", API_FN_PAIR(PlayAmbientSound)},
		{"PlayFlic", API_FN_PAIR(PlayFlic)},
		{"PlayMP3File", API_FN_PAIR(PlayMP3File)},
		{"PlayMusic", API_FN_PAIR(PlayMusicResetQueue)},
		{"PlayMusicQueued", API_FN_PAIR(PlayMusicQueued)},
		{"PlaySilentMIDI", API_FN_PAIR(PlaySilentMIDI)},
		{"PlaySound", API_FN_PAIR(play_sound)},
		{"PlaySoundEx", API_FN_PAIR(PlaySoundEx)},
		{"PlayVideo", API_FN_PAIR(PlayVideo)},
		{"QuitGame", API_FN_PAIR(QuitGame)},
		{"Random", Sc_Rand},
		{"RawClearScreen", API_FN_PAIR(RawClear)},
		{"RawDrawCircle", API_FN_PAIR(RawDrawCircle)},
		{"RawDrawFrameTransparent", API_FN_PAIR(RawDrawFrameTransparent)},
		{"RawDrawImage", API_FN_PAIR(RawDrawImage)},
		{"RawDrawImageOffset", API_FN_PAIR(RawDrawImageOffset)},
		{"RawDrawImageResized", API_FN_PAIR(RawDrawImageResized)},
		{"RawDrawImageTransparent", API_FN_PAIR(RawDrawImageTransparent)},
		{"RawDrawLine", API_FN_PAIR(RawDrawLine)},
		{"RawDrawRectangle", API_FN_PAIR(RawDrawRectangle)},
		{"RawDrawTriangle", API_FN_PAIR(RawDrawTriangle)},
		{"RawPrint", Sc_RawPrint},
		{"RawPrintMessageWrapped", API_FN_PAIR(RawPrintMessageWrapped)},
		{"RawRestoreScreen", API_FN_PAIR(RawRestoreScreen)},
		{"RawRestoreScreenTinted", API_FN_PAIR(RawRestoreScreenTinted)},
		{"RawSaveScreen", API_FN_PAIR(RawSaveScreen)},
		{"RawSetColor", API_FN_PAIR(RawSetColor)},
		{"RawSetColorRGB", API_FN_PAIR(RawSetColorRGB)},
		{"RefreshMouse", API_FN_PAIR(RefreshMouse)},
		{"ReleaseCharacterView", API_FN_PAIR(ReleaseCharacterView)},
		{"ReleaseViewport", API_FN_PAIR(ReleaseViewport)},
		{"RemoveObjectTint", API_FN_PAIR(RemoveObjectTint)},
		{"RemoveOverlay", API_FN_PAIR(RemoveOverlay)},
		{"RemoveWalkableArea", API_FN_PAIR(RemoveWalkableArea)},
		{"ResetRoom", API_FN_PAIR(ResetRoom)},
		{"RestartGame", API_FN_PAIR(restart_game)},
		{"RestoreGameDialog", API_FN_PAIR(restore_game_dialog)},
		{"RestoreGameSlot", API_FN_PAIR(RestoreGameSlot)},
		{"RestoreWalkableArea", API_FN_PAIR(RestoreWalkableArea)},
		{"RunAGSGame", API_FN_PAIR(RunAGSGame)},
		{"RunCharacterInteraction", API_FN_PAIR(RunCharacterInteraction)},
		{"RunDialog", API_FN_PAIR(RunDialog)},
		{"RunHotspotInteraction", API_FN_PAIR(RunHotspotInteraction)},
		{"RunInventoryInteraction", API_FN_PAIR(RunInventoryInteraction)},
		{"RunObjectInteraction", API_FN_PAIR(RunObjectInteraction)},
		{"RunRegionInteraction", API_FN_PAIR(RunRegionInteraction)},
		{"Said", API_FN_PAIR(Said)},
		{"SaidUnknownWord", API_FN_PAIR(SaidUnknownWord)},
		{"SaveCursorForLocationChange", API_FN_PAIR(SaveCursorForLocationChange)},
		{"SaveGameDialog", API_FN_PAIR(save_game_dialog)},
		{"SaveGameSlot", API_FN_PAIR(save_game)},
		{"SaveScreenShot", API_FN_PAIR(SaveScreenShot)},
		{"SeekMIDIPosition", API_FN_PAIR(SeekMIDIPosition)},
		{"SeekMODPattern", API_FN_PAIR(SeekMODPattern)},
		{"SeekMP3PosMillis", API_FN_PAIR(SeekMP3PosMillis)},
		{"SetActiveInventory", API_FN_PAIR(SetActiveInventory)},
		{"SetAmbientTint", API_FN_PAIR(SetAmbientTint)},
		{"SetAmbientLightLevel", API_FN_PAIR(SetAmbientLightLevel)},
		{"SetAreaLightLevel", API_FN_PAIR(SetAreaLightLevel)},
		{"SetAreaScaling", API_FN_PAIR(SetAreaScaling)},
		{"SetBackgroundFrame", API_FN_PAIR(SetBackgroundFrame)},
		{"SetButtonPic", API_FN_PAIR(SetButtonPic)},
		{"SetButtonText", API_FN_PAIR(SetButtonText)},
		{"SetChannelVolume", API_FN_PAIR(SetChannelVolume)},
		{"SetCharacterBaseline", API_FN_PAIR(SetCharacterBaseline)},
		{"SetCharacterClickable", API_FN_PAIR(SetCharacterClickable)},
		{"SetCharacterFrame", API_FN_PAIR(SetCharacterFrame)},
		{"SetCharacterIdle", API_FN_PAIR(SetCharacterIdle)},
		{"SetCharacterIgnoreLight", API_FN_PAIR(SetCharacterIgnoreLight)},
		{"SetCharacterIgnoreWalkbehinds", API_FN_PAIR(SetCharacterIgnoreWalkbehinds)},
		{"SetCharacterProperty", API_FN_PAIR(SetCharacterProperty)},
		{"SetCharacterBlinkView", API_FN_PAIR(SetCharacterBlinkView)},
		{"SetCharacterSpeechView", API_FN_PAIR(SetCharacterSpeechView)},
		{"SetCharacterSpeed", API_FN_PAIR(SetCharacterSpeed)},
		{"SetCharacterSpeedEx", API_FN_PAIR(SetCharacterSpeedEx)},
		{"SetCharacterTransparency", API_FN_PAIR(SetCharacterTransparency)},
		{"SetCharacterView", API_FN_PAIR(SetCharacterView)},
		{"SetCharacterViewEx", API_FN_PAIR(SetCharacterViewEx)},
		{"SetCharacterViewOffset", API_FN_PAIR(SetCharacterViewOffset)},
		{"SetCursorMode", API_FN_PAIR(set_cursor_mode)},
		{"SetDefaultCursor", API_FN_PAIR(set_default_cursor)},
		{"SetDialogOption", API_FN_PAIR(SetDialogOption)},
		{"SetDigitalMasterVolume", API_FN_PAIR(SetDigitalMasterVolume)},
		{"SetFadeColor", API_FN_PAIR(SetFadeColor)},
		{"SetFrameSound", API_FN_PAIR(SetFrameSound)},
		{"SetGameOption", API_FN_PAIR(SetGameOption)},
		{"SetGameSpeed", API_FN_PAIR(SetGameSpeed)},
		{"SetGlobalInt", API_FN_PAIR(SetGlobalInt)},
		{"SetGlobalString", API_FN_PAIR(SetGlobalString)},
		{"SetGraphicalVariable", API_FN_PAIR(SetGraphicalVariable)},
		{"SetGUIBackgroundPic", API_FN_PAIR(SetGUIBackgroundPic)},
		{"SetGUIClickable", API_FN_PAIR(SetGUIClickable)},
		{"SetGUIObjectEnabled", API_FN_PAIR(SetGUIObjectEnabled)},
		{"SetGUIObjectPosition", API_FN_PAIR(SetGUIObjectPosition)},
		{"SetGUIObjectSize", API_FN_PAIR(SetGUIObjectSize)},
		{"SetGUIPosition", API_FN_PAIR(SetGUIPosition)},
		{"SetGUISize", API_FN_PAIR(SetGUISize)},
		{"SetGUITransparency", API_FN_PAIR(SetGUITransparency)},
		{"SetGUIZOrder", API_FN_PAIR(SetGUIZOrder)},
		{"SetInvItemName", API_FN_PAIR(SetInvItemName)},
		{"SetInvItemPic", API_FN_PAIR(set_inv_item_pic)},
		{"SetInvDimensions", API_FN_PAIR(SetInvDimensions)},
		{"SetLabelColor", API_FN_PAIR(SetLabelColor)},
		{"SetLabelFont", API_FN_PAIR(SetLabelFont)},
		{"SetLabelText", API_FN_PAIR(SetLabelText)},
		{"SetMouseBounds", API_FN_PAIR(SetMouseBounds)},
		{"SetMouseCursor", API_FN_PAIR(set_mouse_cursor)},
		{"SetMousePosition", API_FN_PAIR(SetMousePosition)},
		{"SetMultitaskingMode", API_FN_PAIR(SetMultitasking)},
		{"SetMusicMasterVolume", API_FN_PAIR(SetMusicMasterVolume)},
		{"SetMusicRepeat", API_FN_PAIR(SetMusicRepeat)},
		{"SetMusicVolume", API_FN_PAIR(SetMusicVolume)},
		{"SetNextCursorMode", API_FN_PAIR(SetNextCursor)},
		{"SetNextScreenTransition", API_FN_PAIR(SetNextScreenTransition)},
		{"SetNormalFont", API_FN_PAIR(SetNormalFont)},
		{"SetObjectBaseline", API_FN_PAIR(SetObjectBaseline)},
		{"SetObjectClickable", API_FN_PAIR(SetObjectClickable)},
		{"SetObjectFrame", API_FN_PAIR(SetObjectFrame)},
		{"SetObjectGraphic", API_FN_PAIR(SetObjectGraphic)},
		{"SetObjectIgnoreWalkbehinds", API_FN_PAIR(SetObjectIgnoreWalkbehinds)},
		{"SetObjectPosition", API_FN_PAIR(SetObjectPosition)},
		{"SetObjectTint", API_FN_PAIR(SetObjectTint)},
		{"SetObjectTransparency", API_FN_PAIR(SetObjectTransparency)},
		{"SetObjectView", API_FN_PAIR(SetObjectView)},
		{"SetPalRGB", API_FN_PAIR(SetPalRGB)},
		{"SetPlayerCharacter", API_FN_PAIR(SetPlayerCharacter)},
		{"SetRegionTint", API_FN_PAIR(SetRegionTint)},
		{"SetRestartPoint", API_FN_PAIR(SetRestartPoint)},
		{"SetScreenTransition", API_FN_PAIR(SetScreenTransition)},
		{"SetSkipSpeech", API_FN_PAIR(SetSkipSpeech)},
		{"SetSliderValue", API_FN_PAIR(SetSliderValue)},
		{"SetSoundVolume", API_FN_PAIR(SetSoundVolume)},
		{"SetSpeechFont", API_FN_PAIR(SetSpeechFont)},
		{"SetSpeechStyle", API_FN_PAIR(SetSpeechStyle)},
		{"SetSpeechVolume", API_FN_PAIR(SetSpeechVolume)},
		{"SetTalkingColor", API_FN_PAIR(SetTalkingColor)},
		{"SetTextBoxFont", API_FN_PAIR(SetTextBoxFont)},
		{"SetTextBoxText", API_FN_PAIR(SetTextBoxText)},
		{"SetTextOverlay", Sc_SetTextOverlay},
		{"SetTextWindowGUI", API_FN_PAIR(SetTextWindowGUI)},
		{"SetTimer", API_FN_PAIR(script_SetTimer)},
		{"SetViewport", API_FN_PAIR(SetViewport)},
		{"SetVoiceMode", API_FN_PAIR(SetVoiceMode)},
		{"SetWalkBehindBase", API_FN_PAIR(SetWalkBehindBase)},
		{"ShakeScreen", API_FN_PAIR(ShakeScreen)},
		{"ShakeScreenBackground", API_FN_PAIR(ShakeScreenBackground)},
		{"ShowMouseCursor", API_FN_PAIR(ShowMouseCursor)},
		{"SkipCutscene", API_FN_PAIR(SkipCutscene)},
		{"SkipUntilCharacterStops", API_FN_PAIR(SkipUntilCharacterStops)},
		{"StartCutscene", API_FN_PAIR(StartCutscene)},
		{"StartRecording", API_FN_PAIR(scStartRecording)},
		{"StopAmbientSound", API_FN_PAIR(StopAmbientSound)},
		{"StopChannel", API_FN_PAIR(stop_and_destroy_channel)},
		{"StopDialog", API_FN_PAIR(StopDialog)},
		{"StopMoving", API_FN_PAIR(StopMoving)},
		{"StopMusic", API_FN_PAIR(scr_StopMusic)},
		{"StopObjectMoving", API_FN_PAIR(StopObjectMoving)},
		{"StrCat", Sc_sc_strcat},
		{"StrCaseComp", Sc_stricmp},
		{"StrComp", API_FN_PAIR(strcmp)},
		{"StrContains", API_FN_PAIR(StrContains)},
		{"StrCopy", Sc_sc_strcpy},
		{"StrFormat", Sc_sc_sprintf},
		{"StrGetCharAt", API_FN_PAIR(StrGetCharAt)},
		{"StringToInt", API_FN_PAIR(StringToInt)},
		{"StrLen", API_FN_PAIR(strlen)},
		{"StrSetCharAt", API_FN_PAIR(StrSetCharAt)},
		{"StrToLowerCase", Sc_sc_strlower},
		{"StrToUpperCase", Sc_sc_strupper},
		{"TintScreen", API_FN_PAIR(TintScreen)},
		{"UnPauseGame", API_FN_PAIR(UnPauseGame)},
		{"UpdateInventory", API_FN_PAIR(update_invorder)},
		{"UpdatePalette", API_FN_PAIR(UpdatePalette)},
		{"Wait", API_FN_PAIR(scrWait)},
		{"WaitKey", API_FN_PAIR(WaitKey)},
		{"WaitMouse", API_FN_PAIR(WaitMouse)},
		{"WaitMouseKey", API_FN_PAIR(WaitMouseKey)},
		{"WaitInput", API_FN_PAIR(WaitInput)},
		{"SkipWait", API_FN_PAIR(SkipWait)},
	};

	ccAddExternalFunctions361(global_api);
}

} // namespace AGS3
