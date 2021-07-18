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

#include "ags/engine/ac/object.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/object_cache.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_object.h"
#include "ags/engine/ac/move_list.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

int Object_IsCollidingWithObject(ScriptObject *objj, ScriptObject *obj2) {
	return AreObjectsColliding(objj->id, obj2->id);
}

ScriptObject *GetObjectAtScreen(int xx, int yy) {
	int hsnum = GetObjectIDAtScreen(xx, yy);
	if (hsnum < 0)
		return nullptr;
	return &_G(scrObj)[hsnum];
}

ScriptObject *GetObjectAtRoom(int x, int y) {
	int hsnum = GetObjectIDAtRoom(x, y);
	if (hsnum < 0)
		return nullptr;
	return &_G(scrObj)[hsnum];
}

AGS_INLINE int is_valid_object(int obtest) {
	if ((obtest < 0) || (obtest >= _G(croom)->numobj)) return 0;
	return 1;
}

void Object_Tint(ScriptObject *objj, int red, int green, int blue, int saturation, int luminance) {
	SetObjectTint(objj->id, red, green, blue, saturation, luminance);
}

void Object_RemoveTint(ScriptObject *objj) {
	RemoveObjectTint(objj->id);
}

void Object_SetView(ScriptObject *objj, int view, int loop, int frame) {
	SetObjectFrame(objj->id, view, loop, frame);
}

void Object_SetTransparency(ScriptObject *objj, int trans) {
	SetObjectTransparency(objj->id, trans);
}

int Object_GetTransparency(ScriptObject *objj) {
	if (!is_valid_object(objj->id))
		quit("!Object.Transparent: invalid object number specified");

	return GfxDef::LegacyTrans255ToTrans100(_G(objs)[objj->id].transparent);
}

void Object_SetBaseline(ScriptObject *objj, int basel) {
	SetObjectBaseline(objj->id, basel);
}

int Object_GetBaseline(ScriptObject *objj) {
	return GetObjectBaseline(objj->id);
}

void Object_AnimateFrom(ScriptObject *objj, int loop, int delay, int repeat, int blocking, int direction, int sframe) {
	if (direction == FORWARDS)
		direction = 0;
	else if (direction == BACKWARDS)
		direction = 1;
	else
		quit("!Object.Animate: Invalid DIRECTION parameter");

	if ((blocking == BLOCKING) || (blocking == 1))
		blocking = 1;
	else if ((blocking == IN_BACKGROUND) || (blocking == 0))
		blocking = 0;
	else
		quit("!Object.Animate: Invalid BLOCKING parameter");

	AnimateObjectImpl(objj->id, loop, delay, repeat, direction, blocking, sframe);
}

void Object_Animate(ScriptObject *objj, int loop, int delay, int repeat, int blocking, int direction) {
	Object_AnimateFrom(objj, loop, delay, repeat, blocking, direction, 0);
}

void Object_StopAnimating(ScriptObject *objj) {
	if (!is_valid_object(objj->id))
		quit("!Object.StopAnimating: invalid object number");

	if (_G(objs)[objj->id].cycling) {
		_G(objs)[objj->id].cycling = 0;
		_G(objs)[objj->id].wait = 0;
	}
}

void Object_MergeIntoBackground(ScriptObject *objj) {
	MergeObject(objj->id);
}

void Object_StopMoving(ScriptObject *objj) {
	StopObjectMoving(objj->id);
}

void Object_SetVisible(ScriptObject *objj, int onoroff) {
	if (onoroff)
		ObjectOn(objj->id);
	else
		ObjectOff(objj->id);
}

int Object_GetView(ScriptObject *objj) {
	if (_G(objs)[objj->id].view == (uint16_t)-1)
		return 0;
	return _G(objs)[objj->id].view + 1;
}

int Object_GetLoop(ScriptObject *objj) {
	if (_G(objs)[objj->id].view == (uint16_t)-1)
		return 0;
	return _G(objs)[objj->id].loop;
}

int Object_GetFrame(ScriptObject *objj) {
	if (_G(objs)[objj->id].view == (uint16_t)-1)
		return 0;
	return _G(objs)[objj->id].frame;
}

int Object_GetVisible(ScriptObject *objj) {
	return IsObjectOn(objj->id);
}

void Object_SetGraphic(ScriptObject *objj, int slott) {
	SetObjectGraphic(objj->id, slott);
}

int Object_GetGraphic(ScriptObject *objj) {
	return GetObjectGraphic(objj->id);
}

int GetObjectX(int objj) {
	if (!is_valid_object(objj)) quit("!GetObjectX: invalid object number");
	return _G(objs)[objj].x;
}

int Object_GetX(ScriptObject *objj) {
	return GetObjectX(objj->id);
}

int Object_GetY(ScriptObject *objj) {
	return GetObjectY(objj->id);
}

int Object_GetAnimating(ScriptObject *objj) {
	return IsObjectAnimating(objj->id);
}

int Object_GetMoving(ScriptObject *objj) {
	return IsObjectMoving(objj->id);
}

bool Object_HasExplicitLight(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_light();
}

bool Object_HasExplicitTint(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_tint();
}

int Object_GetLightLevel(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_light() ? _G(objs)[obj->id].tint_light : 0;
}

void Object_SetLightLevel(ScriptObject *objj, int light_level) {
	int obj = objj->id;
	if (!is_valid_object(obj))
		quit("!SetObjectTint: invalid object number specified");

	_G(objs)[obj].tint_light = light_level;
	_G(objs)[obj].flags &= ~OBJF_HASTINT;
	_G(objs)[obj].flags |= OBJF_HASLIGHT;
}

int Object_GetTintRed(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_tint() ? _G(objs)[obj->id].tint_r : 0;
}

int Object_GetTintGreen(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_tint() ? _G(objs)[obj->id].tint_g : 0;
}

int Object_GetTintBlue(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_tint() ? _G(objs)[obj->id].tint_b : 0;
}

int Object_GetTintSaturation(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_tint() ? _G(objs)[obj->id].tint_level : 0;
}

int Object_GetTintLuminance(ScriptObject *obj) {
	return _G(objs)[obj->id].has_explicit_tint() ? ((_G(objs)[obj->id].tint_light * 10) / 25) : 0;
}

void Object_SetPosition(ScriptObject *objj, int xx, int yy) {
	SetObjectPosition(objj->id, xx, yy);
}

void Object_SetX(ScriptObject *objj, int xx) {
	SetObjectPosition(objj->id, xx, _G(objs)[objj->id].y);
}

void Object_SetY(ScriptObject *objj, int yy) {
	SetObjectPosition(objj->id, _G(objs)[objj->id].x, yy);
}

void Object_GetName(ScriptObject *objj, char *buffer) {
	GetObjectName(objj->id, buffer);
}

const char *Object_GetName_New(ScriptObject *objj) {
	if (!is_valid_object(objj->id))
		quit("!Object.Name: invalid object number");

	return CreateNewScriptString(get_translation(_GP(thisroom).Objects[objj->id].Name.GetCStr()));
}

bool Object_IsInteractionAvailable(ScriptObject *oobj, int mood) {

	_GP(play).check_interaction_only = 1;
	RunObjectInteraction(oobj->id, mood);
	int ciwas = _GP(play).check_interaction_only;
	_GP(play).check_interaction_only = 0;
	return (ciwas == 2);
}

void Object_Move(ScriptObject *objj, int x, int y, int speed, int blocking, int direct) {
	if ((direct == ANYWHERE) || (direct == 1))
		direct = 1;
	else if ((direct == WALKABLE_AREAS) || (direct == 0))
		direct = 0;
	else
		quit("Object.Move: invalid DIRECT parameter");

	move_object(objj->id, x, y, speed, direct);

	if ((blocking == BLOCKING) || (blocking == 1))
		GameLoopUntilNotMoving(&_G(objs)[objj->id].moving);
	else if ((blocking != IN_BACKGROUND) && (blocking != 0))
		quit("Object.Move: invalid BLOCKING paramter");
}

void Object_SetClickable(ScriptObject *objj, int clik) {
	SetObjectClickable(objj->id, clik);
}

int Object_GetClickable(ScriptObject *objj) {
	if (!is_valid_object(objj->id))
		quit("!Object.Clickable: Invalid object specified");

	if (_G(objs)[objj->id].flags & OBJF_NOINTERACT)
		return 0;
	return 1;
}

void Object_SetIgnoreScaling(ScriptObject *objj, int newval) {
	if (!is_valid_object(objj->id))
		quit("!Object.IgnoreScaling: Invalid object specified");

	_G(objs)[objj->id].flags &= ~OBJF_USEROOMSCALING;
	if (!newval)
		_G(objs)[objj->id].flags |= OBJF_USEROOMSCALING;

	// clear the cache
	_G(objcache)[objj->id].ywas = -9999;
}

int Object_GetIgnoreScaling(ScriptObject *objj) {
	if (!is_valid_object(objj->id))
		quit("!Object.IgnoreScaling: Invalid object specified");

	if (_G(objs)[objj->id].flags & OBJF_USEROOMSCALING)
		return 0;
	return 1;
}

void Object_SetSolid(ScriptObject *objj, int solid) {
	_G(objs)[objj->id].flags &= ~OBJF_SOLID;
	if (solid)
		_G(objs)[objj->id].flags |= OBJF_SOLID;
}

int Object_GetSolid(ScriptObject *objj) {
	if (_G(objs)[objj->id].flags & OBJF_SOLID)
		return 1;
	return 0;
}

void Object_SetBlockingWidth(ScriptObject *objj, int bwid) {
	_G(objs)[objj->id].blocking_width = bwid;
}

int Object_GetBlockingWidth(ScriptObject *objj) {
	return _G(objs)[objj->id].blocking_width;
}

void Object_SetBlockingHeight(ScriptObject *objj, int bhit) {
	_G(objs)[objj->id].blocking_height = bhit;
}

int Object_GetBlockingHeight(ScriptObject *objj) {
	return _G(objs)[objj->id].blocking_height;
}

int Object_GetID(ScriptObject *objj) {
	return objj->id;
}

void Object_SetIgnoreWalkbehinds(ScriptObject *chaa, int clik) {
	SetObjectIgnoreWalkbehinds(chaa->id, clik);
}

int Object_GetIgnoreWalkbehinds(ScriptObject *chaa) {
	if (!is_valid_object(chaa->id))
		quit("!Object.IgnoreWalkbehinds: Invalid object specified");

	if (_G(objs)[chaa->id].flags & OBJF_NOWALKBEHINDS)
		return 1;
	return 0;
}

void move_object(int objj, int tox, int toy, int spee, int ignwal) {

	if (!is_valid_object(objj))
		quit("!MoveObject: invalid object number");

	// AGS <= 2.61 uses MoveObject with spp=-1 internally instead of SetObjectPosition
	if ((_G(loaded_game_file_version) <= kGameVersion_261) && (spee == -1)) {
		_G(objs)[objj].x = tox;
		_G(objs)[objj].y = toy;
		return;
	}

	debug_script_log("Object %d start move to %d,%d", objj, tox, toy);

	int objX = room_to_mask_coord(_G(objs)[objj].x);
	int objY = room_to_mask_coord(_G(objs)[objj].y);
	tox = room_to_mask_coord(tox);
	toy = room_to_mask_coord(toy);

	set_route_move_speed(spee, spee);
	set_color_depth(8);
	int mslot = find_route(objX, objY, tox, toy, prepare_walkable_areas(-1), objj + 1, 1, ignwal);
	set_color_depth(_GP(game).GetColorDepth());
	if (mslot > 0) {
		_G(objs)[objj].moving = mslot;
		_G(mls)[mslot].direct = ignwal;
		convert_move_path_to_room_resolution(&_G(mls)[mslot]);
	}
}

void Object_RunInteraction(ScriptObject *objj, int mode) {
	RunObjectInteraction(objj->id, mode);
}

int Object_GetProperty(ScriptObject *objj, const char *property) {
	return GetObjectProperty(objj->id, property);
}

void Object_GetPropertyText(ScriptObject *objj, const char *property, char *bufer) {
	GetObjectPropertyText(objj->id, property, bufer);
}

const char *Object_GetTextProperty(ScriptObject *objj, const char *property) {
	return get_text_property_dynamic_string(_GP(thisroom).Objects[objj->id].Properties, _G(croom)->objProps[objj->id], property);
}

bool Object_SetProperty(ScriptObject *objj, const char *property, int value) {
	return set_int_property(_G(croom)->objProps[objj->id], property, value);
}

bool Object_SetTextProperty(ScriptObject *objj, const char *property, const char *value) {
	return set_text_property(_G(croom)->objProps[objj->id], property, value);
}

void get_object_blocking_rect(int objid, int *x1, int *y1, int *width, int *y2) {
	RoomObject *tehobj = &_G(objs)[objid];
	int cwidth, fromx;

	if (tehobj->blocking_width < 1)
		cwidth = game_to_data_coord(tehobj->last_width) - 4;
	else
		cwidth = tehobj->blocking_width;

	fromx = tehobj->x + (game_to_data_coord(tehobj->last_width) / 2) - cwidth / 2;
	if (fromx < 0) {
		cwidth += fromx;
		fromx = 0;
	}
	if (fromx + cwidth >= mask_to_room_coord(_G(walkable_areas_temp)->GetWidth()))
		cwidth = mask_to_room_coord(_G(walkable_areas_temp)->GetWidth()) - fromx;

	if (x1)
		*x1 = fromx;
	if (width)
		*width = cwidth;
	if (y1) {
		if (tehobj->blocking_height > 0)
			*y1 = tehobj->y - tehobj->blocking_height / 2;
		else
			*y1 = tehobj->y - 2;
	}
	if (y2) {
		if (tehobj->blocking_height > 0)
			*y2 = tehobj->y + tehobj->blocking_height / 2;
		else
			*y2 = tehobj->y + 3;
	}
}

int isposinbox(int mmx, int mmy, int lf, int tp, int rt, int bt) {
	if ((mmx >= lf) & (mmx <= rt) & (mmy >= tp) & (mmy <= bt)) return TRUE;
	else return FALSE;
}

// xx,yy is the position in room co-ordinates that we are checking
// arx,ary is the sprite x/y co-ordinates
int is_pos_in_sprite(int xx, int yy, int arx, int ary, Bitmap *sprit, int spww, int sphh, int flipped) {
	if (spww == 0) spww = game_to_data_coord(sprit->GetWidth()) - 1;
	if (sphh == 0) sphh = game_to_data_coord(sprit->GetHeight()) - 1;

	if (isposinbox(xx, yy, arx, ary, arx + spww, ary + sphh) == FALSE)
		return FALSE;

	if (_GP(game).options[OPT_PIXPERFECT]) {
		// if it's transparent, or off the edge of the sprite, ignore
		int xpos = data_to_game_coord(xx - arx);
		int ypos = data_to_game_coord(yy - ary);

		if (_G(gfxDriver)->HasAcceleratedTransform()) {
			// hardware acceleration, so the sprite in memory will not have
			// been stretched, it will be original size. Thus, adjust our
			// calculations to compensate
			data_to_game_coords(&spww, &sphh);

			if (spww != sprit->GetWidth())
				xpos = (xpos * sprit->GetWidth()) / spww;
			if (sphh != sprit->GetHeight())
				ypos = (ypos * sprit->GetHeight()) / sphh;
		}

		if (flipped)
			xpos = (sprit->GetWidth() - 1) - xpos;

		int gpcol = my_getpixel(sprit, xpos, ypos);

		if ((gpcol == sprit->GetMaskColor()) || (gpcol == -1))
			return FALSE;
	}
	return TRUE;
}

// X and Y co-ordinates must be in native format (TODO: find out if this comment is still true)
int check_click_on_object(int roomx, int roomy, int mood) {
	int aa = GetObjectIDAtRoom(roomx, roomy);
	if (aa < 0) return 0;
	RunObjectInteraction(aa, mood);
	return 1;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void (ScriptObject *objj, int loop, int delay, int repeat, int blocking, int direction)
RuntimeScriptValue Sc_Object_Animate(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(ScriptObject, Object_Animate);
}

RuntimeScriptValue Sc_Object_AnimateFrom(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT6(ScriptObject, Object_AnimateFrom);
}

// int (ScriptObject *objj, ScriptObject *obj2)
RuntimeScriptValue Sc_Object_IsCollidingWithObject(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(ScriptObject, Object_IsCollidingWithObject, ScriptObject);
}

// void (ScriptObject *objj, char *buffer)
RuntimeScriptValue Sc_Object_GetName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(ScriptObject, Object_GetName, char);
}

// int (ScriptObject *objj, const char *property)
RuntimeScriptValue Sc_Object_GetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(ScriptObject, Object_GetProperty, const char);
}

// void (ScriptObject *objj, const char *property, char *bufer)
RuntimeScriptValue Sc_Object_GetPropertyText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ2(ScriptObject, Object_GetPropertyText, const char, char);
}

//const char* (ScriptObject *objj, const char *property)
RuntimeScriptValue Sc_Object_GetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_POBJ(ScriptObject, const char, _GP(myScriptStringImpl), Object_GetTextProperty, const char);
}

RuntimeScriptValue Sc_Object_SetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ_PINT(ScriptObject, Object_SetProperty, const char);
}

RuntimeScriptValue Sc_Object_SetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ2(ScriptObject, Object_SetTextProperty, const char, const char);
}

// void (ScriptObject *objj)
RuntimeScriptValue Sc_Object_MergeIntoBackground(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptObject, Object_MergeIntoBackground);
}

RuntimeScriptValue Sc_Object_IsInteractionAvailable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_PINT(ScriptObject, Object_IsInteractionAvailable);
}

// void (ScriptObject *objj, int x, int y, int speed, int blocking, int direct)
RuntimeScriptValue Sc_Object_Move(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(ScriptObject, Object_Move);
}

// void (ScriptObject *objj)
RuntimeScriptValue Sc_Object_RemoveTint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptObject, Object_RemoveTint);
}

// void (ScriptObject *objj, int mode)
RuntimeScriptValue Sc_Object_RunInteraction(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_RunInteraction);
}

RuntimeScriptValue Sc_Object_HasExplicitLight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(ScriptObject, Object_HasExplicitLight);
}

RuntimeScriptValue Sc_Object_HasExplicitTint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(ScriptObject, Object_HasExplicitTint);
}

RuntimeScriptValue Sc_Object_GetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetLightLevel);
}

RuntimeScriptValue Sc_Object_SetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetLightLevel);
}

RuntimeScriptValue Sc_Object_GetTintBlue(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetTintBlue);
}

RuntimeScriptValue Sc_Object_GetTintGreen(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetTintGreen);
}

RuntimeScriptValue Sc_Object_GetTintRed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetTintRed);
}

RuntimeScriptValue Sc_Object_GetTintSaturation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetTintSaturation);
}

RuntimeScriptValue Sc_Object_GetTintLuminance(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetTintLuminance);
}

// void (ScriptObject *objj, int xx, int yy)
RuntimeScriptValue Sc_Object_SetPosition(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptObject, Object_SetPosition);
}

// void (ScriptObject *objj, int view, int loop, int frame)
RuntimeScriptValue Sc_Object_SetView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(ScriptObject, Object_SetView);
}

// void (ScriptObject *objj)
RuntimeScriptValue Sc_Object_StopAnimating(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptObject, Object_StopAnimating);
}

// void (ScriptObject *objj)
RuntimeScriptValue Sc_Object_StopMoving(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptObject, Object_StopMoving);
}

// void (ScriptObject *objj, int red, int green, int blue, int saturation, int luminance)
RuntimeScriptValue Sc_Object_Tint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(ScriptObject, Object_Tint);
}

RuntimeScriptValue Sc_GetObjectAtRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptObject, _GP(ccDynamicObject), GetObjectAtRoom);
}

// ScriptObject *(int xx, int yy)
RuntimeScriptValue Sc_GetObjectAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptObject, _GP(ccDynamicObject), GetObjectAtScreen);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetAnimating(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetAnimating);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetBaseline(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetBaseline);
}

// void (ScriptObject *objj, int basel)
RuntimeScriptValue Sc_Object_SetBaseline(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetBaseline);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetBlockingHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetBlockingHeight);
}

// void (ScriptObject *objj, int bhit)
RuntimeScriptValue Sc_Object_SetBlockingHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetBlockingHeight);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetBlockingWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetBlockingWidth);
}

// void (ScriptObject *objj, int bwid)
RuntimeScriptValue Sc_Object_SetBlockingWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetBlockingWidth);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetClickable);
}

// void (ScriptObject *objj, int clik)
RuntimeScriptValue Sc_Object_SetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetClickable);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetFrame);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetGraphic);
}

// void (ScriptObject *objj, int slott)
RuntimeScriptValue Sc_Object_SetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetGraphic);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetID);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetIgnoreScaling(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetIgnoreScaling);
}

// void (ScriptObject *objj, int newval)
RuntimeScriptValue Sc_Object_SetIgnoreScaling(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetIgnoreScaling);
}

// int (ScriptObject *chaa)
RuntimeScriptValue Sc_Object_GetIgnoreWalkbehinds(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetIgnoreWalkbehinds);
}

// void (ScriptObject *chaa, int clik)
RuntimeScriptValue Sc_Object_SetIgnoreWalkbehinds(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetIgnoreWalkbehinds);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetLoop);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetMoving(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetMoving);
}

// const char* (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetName_New(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(ScriptObject, const char, _GP(myScriptStringImpl), Object_GetName_New);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetSolid(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetSolid);
}

// void (ScriptObject *objj, int solid)
RuntimeScriptValue Sc_Object_SetSolid(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetSolid);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetTransparency);
}

// void (ScriptObject *objj, int trans)
RuntimeScriptValue Sc_Object_SetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetTransparency);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetView);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetVisible);
}

// void (ScriptObject *objj, int onoroff)
RuntimeScriptValue Sc_Object_SetVisible(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetVisible);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetX);
}

// void (ScriptObject *objj, int xx)
RuntimeScriptValue Sc_Object_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetX);
}

// int (ScriptObject *objj)
RuntimeScriptValue Sc_Object_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptObject, Object_GetY);
}

// void (ScriptObject *objj, int yy)
RuntimeScriptValue Sc_Object_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptObject, Object_SetY);
}



void RegisterObjectAPI() {
	ccAddExternalObjectFunction("Object::Animate^5", Sc_Object_Animate);
	ccAddExternalObjectFunction("Object::Animate^6", Sc_Object_AnimateFrom);
	ccAddExternalObjectFunction("Object::IsCollidingWithObject^1", Sc_Object_IsCollidingWithObject);
	ccAddExternalObjectFunction("Object::GetName^1", Sc_Object_GetName);
	ccAddExternalObjectFunction("Object::GetProperty^1", Sc_Object_GetProperty);
	ccAddExternalObjectFunction("Object::GetPropertyText^2", Sc_Object_GetPropertyText);
	ccAddExternalObjectFunction("Object::GetTextProperty^1", Sc_Object_GetTextProperty);
	ccAddExternalObjectFunction("Object::SetProperty^2", Sc_Object_SetProperty);
	ccAddExternalObjectFunction("Object::SetTextProperty^2", Sc_Object_SetTextProperty);
	ccAddExternalObjectFunction("Object::IsInteractionAvailable^1", Sc_Object_IsInteractionAvailable);
	ccAddExternalObjectFunction("Object::MergeIntoBackground^0", Sc_Object_MergeIntoBackground);
	ccAddExternalObjectFunction("Object::Move^5", Sc_Object_Move);
	ccAddExternalObjectFunction("Object::RemoveTint^0", Sc_Object_RemoveTint);
	ccAddExternalObjectFunction("Object::RunInteraction^1", Sc_Object_RunInteraction);
	ccAddExternalObjectFunction("Object::SetLightLevel^1", Sc_Object_SetLightLevel);
	ccAddExternalObjectFunction("Object::SetPosition^2", Sc_Object_SetPosition);
	ccAddExternalObjectFunction("Object::SetView^3", Sc_Object_SetView);
	ccAddExternalObjectFunction("Object::StopAnimating^0", Sc_Object_StopAnimating);
	ccAddExternalObjectFunction("Object::StopMoving^0", Sc_Object_StopMoving);
	ccAddExternalObjectFunction("Object::Tint^5", Sc_Object_Tint);

	// static
	ccAddExternalStaticFunction("Object::GetAtRoomXY^2", Sc_GetObjectAtRoom);
	ccAddExternalStaticFunction("Object::GetAtScreenXY^2", Sc_GetObjectAtScreen);

	ccAddExternalObjectFunction("Object::get_Animating", Sc_Object_GetAnimating);
	ccAddExternalObjectFunction("Object::get_Baseline", Sc_Object_GetBaseline);
	ccAddExternalObjectFunction("Object::set_Baseline", Sc_Object_SetBaseline);
	ccAddExternalObjectFunction("Object::get_BlockingHeight", Sc_Object_GetBlockingHeight);
	ccAddExternalObjectFunction("Object::set_BlockingHeight", Sc_Object_SetBlockingHeight);
	ccAddExternalObjectFunction("Object::get_BlockingWidth", Sc_Object_GetBlockingWidth);
	ccAddExternalObjectFunction("Object::set_BlockingWidth", Sc_Object_SetBlockingWidth);
	ccAddExternalObjectFunction("Object::get_Clickable", Sc_Object_GetClickable);
	ccAddExternalObjectFunction("Object::set_Clickable", Sc_Object_SetClickable);
	ccAddExternalObjectFunction("Object::get_Frame", Sc_Object_GetFrame);
	ccAddExternalObjectFunction("Object::get_Graphic", Sc_Object_GetGraphic);
	ccAddExternalObjectFunction("Object::set_Graphic", Sc_Object_SetGraphic);
	ccAddExternalObjectFunction("Object::get_ID", Sc_Object_GetID);
	ccAddExternalObjectFunction("Object::get_IgnoreScaling", Sc_Object_GetIgnoreScaling);
	ccAddExternalObjectFunction("Object::set_IgnoreScaling", Sc_Object_SetIgnoreScaling);
	ccAddExternalObjectFunction("Object::get_IgnoreWalkbehinds", Sc_Object_GetIgnoreWalkbehinds);
	ccAddExternalObjectFunction("Object::set_IgnoreWalkbehinds", Sc_Object_SetIgnoreWalkbehinds);
	ccAddExternalObjectFunction("Object::get_Loop", Sc_Object_GetLoop);
	ccAddExternalObjectFunction("Object::get_Moving", Sc_Object_GetMoving);
	ccAddExternalObjectFunction("Object::get_Name", Sc_Object_GetName_New);
	ccAddExternalObjectFunction("Object::get_Solid", Sc_Object_GetSolid);
	ccAddExternalObjectFunction("Object::set_Solid", Sc_Object_SetSolid);
	ccAddExternalObjectFunction("Object::get_Transparency", Sc_Object_GetTransparency);
	ccAddExternalObjectFunction("Object::set_Transparency", Sc_Object_SetTransparency);
	ccAddExternalObjectFunction("Object::get_View", Sc_Object_GetView);
	ccAddExternalObjectFunction("Object::get_Visible", Sc_Object_GetVisible);
	ccAddExternalObjectFunction("Object::set_Visible", Sc_Object_SetVisible);
	ccAddExternalObjectFunction("Object::get_X", Sc_Object_GetX);
	ccAddExternalObjectFunction("Object::set_X", Sc_Object_SetX);
	ccAddExternalObjectFunction("Object::get_Y", Sc_Object_GetY);
	ccAddExternalObjectFunction("Object::set_Y", Sc_Object_SetY);

	ccAddExternalObjectFunction("Object::get_HasExplicitLight", Sc_Object_HasExplicitLight);
	ccAddExternalObjectFunction("Object::get_HasExplicitTint", Sc_Object_HasExplicitTint);
	ccAddExternalObjectFunction("Object::get_LightLevel", Sc_Object_GetLightLevel);
	ccAddExternalObjectFunction("Object::set_LightLevel", Sc_Object_SetLightLevel);
	ccAddExternalObjectFunction("Object::get_TintBlue", Sc_Object_GetTintBlue);
	ccAddExternalObjectFunction("Object::get_TintGreen", Sc_Object_GetTintGreen);
	ccAddExternalObjectFunction("Object::get_TintRed", Sc_Object_GetTintRed);
	ccAddExternalObjectFunction("Object::get_TintSaturation", Sc_Object_GetTintSaturation);
	ccAddExternalObjectFunction("Object::get_TintLuminance", Sc_Object_GetTintLuminance);
}

} // namespace AGS3
