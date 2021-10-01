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

#include "ags/engine/ac/global_object.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/object.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/object_cache.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/script/script.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

#define OVERLAPPING_OBJECT 1000

int GetObjectIDAtScreen(int scrx, int scry) {
	// translate screen co-ordinates to room co-ordinates
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(scrx, scry);
	if (vpt.second < 0)
		return -1;
	return GetObjectIDAtRoom(vpt.first.X, vpt.first.Y);
}

int GetObjectIDAtRoom(int roomx, int roomy) {
	int aa, bestshotyp = -1, bestshotwas = -1;
	// Iterate through all objects in the room
	for (aa = 0; aa < _G(croom)->numobj; aa++) {
		if (_G(objs)[aa].on != 1) continue;
		if (_G(objs)[aa].flags & OBJF_NOINTERACT)
			continue;
		int xxx = _G(objs)[aa].x, yyy = _G(objs)[aa].y;
		int isflipped = 0;
		int spWidth = game_to_data_coord(_G(objs)[aa].get_width());
		int spHeight = game_to_data_coord(_G(objs)[aa].get_height());
		if (_G(objs)[aa].view != (uint16_t)-1)
			isflipped = _G(views)[_G(objs)[aa].view].loops[_G(objs)[aa].loop].frames[_G(objs)[aa].frame].flags & VFLG_FLIPSPRITE;

		Bitmap *theImage = GetObjectImage(aa, &isflipped);

		if (is_pos_in_sprite(roomx, roomy, xxx, yyy - spHeight, theImage,
		                     spWidth, spHeight, isflipped) == FALSE)
			continue;

		int usebasel = _G(objs)[aa].get_baseline();
		if (usebasel < bestshotyp) continue;

		bestshotwas = aa;
		bestshotyp = usebasel;
	}
	_G(obj_lowest_yp) = bestshotyp;
	return bestshotwas;
}

void SetObjectTint(int obj, int red, int green, int blue, int opacity, int luminance) {
	if ((red < 0) || (green < 0) || (blue < 0) ||
	        (red > 255) || (green > 255) || (blue > 255) ||
	        (opacity < 0) || (opacity > 100) ||
	        (luminance < 0) || (luminance > 100))
		quit("!SetObjectTint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

	if (!is_valid_object(obj))
		quit("!SetObjectTint: invalid object number specified");

	debug_script_log("Set object %d tint RGB(%d,%d,%d) %d%%", obj, red, green, blue, opacity);

	_G(objs)[obj].tint_r = red;
	_G(objs)[obj].tint_g = green;
	_G(objs)[obj].tint_b = blue;
	_G(objs)[obj].tint_level = opacity;
	_G(objs)[obj].tint_light = (luminance * 25) / 10;
	_G(objs)[obj].flags &= ~OBJF_HASLIGHT;
	_G(objs)[obj].flags |= OBJF_HASTINT;
}

void RemoveObjectTint(int obj) {
	if (!is_valid_object(obj))
		quit("!RemoveObjectTint: invalid object");

	if (_G(objs)[obj].flags & (OBJF_HASTINT | OBJF_HASLIGHT)) {
		debug_script_log("Un-tint object %d", obj);
		_G(objs)[obj].flags &= ~(OBJF_HASTINT | OBJF_HASLIGHT);
	} else {
		debug_script_warn("RemoveObjectTint called but object was not tinted");
	}
}

void SetObjectView(int obn, int vii) {
	if (!is_valid_object(obn)) quit("!SetObjectView: invalid object number specified");
	debug_script_log("Object %d set to view %d", obn, vii);
	if ((vii < 1) || (vii > _GP(game).numviews)) {
		quitprintf("!SetObjectView: invalid view number (You said %d, max is %d)", vii, _GP(game).numviews);
	}
	vii--;

	if (vii > UINT16_MAX) {
		debug_script_warn("Warning: object's (id %d) view %d is outside of internal range (%d), reset to no view",
		                  obn, vii + 1, UINT16_MAX + 1);
		SetObjectGraphic(obn, 0);
		return;
	}

	_G(objs)[obn].view = (uint16_t)vii;
	_G(objs)[obn].frame = 0;
	if (_G(objs)[obn].loop >= _G(views)[vii].numLoops)
		_G(objs)[obn].loop = 0;
	_G(objs)[obn].cycling = 0;
	int pic = _G(views)[vii].loops[0].frames[0].pic;
	_G(objs)[obn].num = Math::InRangeOrDef<uint16_t>(pic, 0);
	if (pic > UINT16_MAX)
		debug_script_warn("Warning: object's (id %d) sprite %d is outside of internal range (%d), reset to 0", obn, pic, UINT16_MAX);
}

void SetObjectFrame(int obn, int viw, int lop, int fra) {
	if (!is_valid_object(obn)) quit("!SetObjectFrame: invalid object number specified");
	viw--;
	if (viw < 0 || viw >= _GP(game).numviews) quitprintf("!SetObjectFrame: invalid view number used (%d, range is 0 - %d)", viw, _GP(game).numviews - 1);
	if (lop < 0 || lop >= _G(views)[viw].numLoops) quitprintf("!SetObjectFrame: invalid loop number used (%d, range is 0 - %d)", lop, _G(views)[viw].numLoops - 1);
	// historically AGS let user to pass literally any positive invalid frame value by silently reassigning it to zero...
	if (fra < 0 || fra >= _G(views)[viw].loops[lop].numFrames) {
		if (_G(views)[viw].loops[lop].numFrames == 0) // NOTE: we have a dummy frame allocated for this case
			debug_script_warn("SetObjectFrame: specified loop %d has no frames, will fallback to dummy frame", lop);
		else
			debug_script_warn("SetObjectFrame: frame index out of range (%d, must be 0 - %d), set to 0", fra, _G(views)[viw].loops[lop].numFrames - 1);
		fra = 0;
	}
	if (viw > UINT16_MAX || lop > UINT16_MAX || fra > UINT16_MAX) {
		debug_script_warn("Warning: object's (id %d) view/loop/frame (%d/%d/%d) is outside of internal range (%d/%d/%d), reset to no view",
			obn, viw + 1, lop, fra, UINT16_MAX + 1, UINT16_MAX, UINT16_MAX);
		SetObjectGraphic(obn, 0);
		return;
	}

	_G(objs)[obn].view = (uint16_t)viw;
	if (lop >= 0)
		_G(objs)[obn].loop = (uint16_t)lop;
	if (fra >= 0)
		_G(objs)[obn].frame = (uint16_t)fra;

	_G(objs)[obn].view = viw;
	_G(objs)[obn].loop = lop;
	_G(objs)[obn].frame = fra;
	_G(objs)[obn].cycling = 0;
	int pic = _G(views)[viw].loops[lop].frames[fra].pic;
	_G(objs)[obn].num = Math::InRangeOrDef<uint16_t>(pic, 0);
	if (pic > UINT16_MAX)
		debug_script_warn("Warning: object's (id %d) sprite %d is outside of internal range (%d), reset to 0", obn, pic, UINT16_MAX);
	CheckViewFrame(viw, _G(objs)[obn].loop, _G(objs)[obn].frame);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
void SetObjectTransparency(int obn, int trans) {
	if (!is_valid_object(obn)) quit("!SetObjectTransparent: invalid object number specified");
	if ((trans < 0) || (trans > 100)) quit("!SetObjectTransparent: transparency value must be between 0 and 100");

	_G(objs)[obn].transparent = GfxDef::Trans100ToLegacyTrans255(trans);
}



void SetObjectBaseline(int obn, int basel) {
	if (!is_valid_object(obn)) quit("!SetObjectBaseline: invalid object number specified");
	// baseline has changed, invalidate the cache
	if (_G(objs)[obn].baseline != basel) {
		_G(objcache)[obn].ywas = -9999;
		_G(objs)[obn].baseline = basel;
	}
}

int GetObjectBaseline(int obn) {
	if (!is_valid_object(obn)) quit("!GetObjectBaseline: invalid object number specified");

	if (_G(objs)[obn].baseline < 1)
		return 0;

	return _G(objs)[obn].baseline;
}

void AnimateObjectImpl(int obn, int loopn, int spdd, int rept, int direction, int blocking, int sframe) {
	if (obn >= MANOBJNUM) {
		scAnimateCharacter(obn - 100, loopn, spdd, rept);
		return;
	}
	if (!is_valid_object(obn))
		quit("!AnimateObject: invalid object number specified");
	if (_G(objs)[obn].view == (uint16_t)-1)
		quit("!AnimateObject: object has not been assigned a view");
	if (loopn < 0 || loopn >= _G(views)[_G(objs)[obn].view].numLoops)
		quit("!AnimateObject: invalid loop number specified");
	if (sframe < 0 || sframe >= _G(views)[_G(objs)[obn].view].loops[loopn].numFrames)
		quit("!AnimateObject: invalid starting frame number specified");
	if ((direction < 0) || (direction > 1))
		quit("!AnimateObjectEx: invalid direction");
	if ((rept < 0) || (rept > 2))
		quit("!AnimateObjectEx: invalid repeat value");
	if (_G(views)[_G(objs)[obn].view].loops[loopn].numFrames < 1)
		quit("!AnimateObject: no frames in the specified view loop");

	// reverse animation starts at the *previous frame*
	if (direction) {
		sframe--;
		if (sframe < 0)
			sframe = _G(views)[_G(objs)[obn].view].loops[loopn].numFrames - (-sframe);
	}

	if (loopn > UINT16_MAX || sframe > UINT16_MAX) {
		debug_script_warn("Warning: object's (id %d) loop/frame (%d/%d) is outside of internal range (%d/%d), cancel animation",
		                  obn, loopn, sframe, UINT16_MAX, UINT16_MAX);
		return;
	}

	debug_script_log("Obj %d start anim view %d loop %d, speed %d, repeat %d, frame %d", obn, _G(objs)[obn].view + 1, loopn, spdd, rept, sframe);

	_G(objs)[obn].cycling = rept + 1 + (direction * 10);
	_G(objs)[obn].loop = (uint16_t)loopn;
	_G(objs)[obn].frame = (uint16_t)sframe;
	_G(objs)[obn].overall_speed = spdd;
	_G(objs)[obn].wait = spdd + _G(views)[_G(objs)[obn].view].loops[loopn].frames[_G(objs)[obn].frame].speed;
	int pic = _G(views)[_G(objs)[obn].view].loops[loopn].frames[_G(objs)[obn].frame].pic;
	_G(objs)[obn].num = Math::InRangeOrDef<uint16_t>(pic, 0);
	if (pic > UINT16_MAX)
		debug_script_warn("Warning: object's (id %d) sprite %d is outside of internal range (%d), reset to 0", obn, pic, UINT16_MAX);
	CheckViewFrame(_G(objs)[obn].view, loopn, _G(objs)[obn].frame);

	if (blocking)
		GameLoopUntilValueIsZero(&_G(objs)[obn].cycling);
}

void AnimateObjectEx(int obn, int loopn, int spdd, int rept, int direction, int blocking) {
	AnimateObjectImpl(obn, loopn, spdd, rept, direction, blocking, 0);
}

void AnimateObject(int obn, int loopn, int spdd, int rept) {
	AnimateObjectImpl(obn, loopn, spdd, rept, 0, 0, 0);
}

void MergeObject(int obn) {
	if (!is_valid_object(obn)) quit("!MergeObject: invalid object specified");
	int theHeight;

	construct_object_gfx(obn, nullptr, &theHeight, true);

	//Bitmap *oldabuf = graphics->bmp;
	//abuf = _GP(thisroom).BgFrames.Graphic[_GP(play).bg_frame];
	PBitmap bg_frame = _GP(thisroom).BgFrames[_GP(play).bg_frame].Graphic;
	if (bg_frame->GetColorDepth() != _G(actsps)[obn]->GetColorDepth())
		quit("!MergeObject: unable to merge object due to color depth differences");

	int xpos = data_to_game_coord(_G(objs)[obn].x);
	int ypos = (data_to_game_coord(_G(objs)[obn].y) - theHeight);

	draw_sprite_support_alpha(bg_frame.get(), false, xpos, ypos, _G(actsps)[obn], (_GP(game).SpriteInfos[_G(objs)[obn].num].Flags & SPF_ALPHACHANNEL) != 0);
	invalidate_screen();
	mark_current_background_dirty();

	//abuf = oldabuf;
	// mark the sprite as merged
	_G(objs)[obn].on = 2;
	debug_script_log("Object %d merged into background", obn);
}

void StopObjectMoving(int objj) {
	if (!is_valid_object(objj))
		quit("!StopObjectMoving: invalid object number");
	_G(objs)[objj].moving = 0;

	debug_script_log("Object %d stop moving", objj);
}

void ObjectOff(int obn) {
	if (!is_valid_object(obn)) quit("!ObjectOff: invalid object specified");
	// don't change it if on == 2 (merged)
	if (_G(objs)[obn].on == 1) {
		_G(objs)[obn].on = 0;
		debug_script_log("Object %d turned off", obn);
		StopObjectMoving(obn);
	}
}

void ObjectOn(int obn) {
	if (!is_valid_object(obn)) quit("!ObjectOn: invalid object specified");
	if (_G(objs)[obn].on == 0) {
		_G(objs)[obn].on = 1;
		debug_script_log("Object %d turned on", obn);
	}
}

int IsObjectOn(int objj) {
	if (!is_valid_object(objj)) quit("!IsObjectOn: invalid object number");

	// ==1 is on, ==2 is merged into background
	if (_G(objs)[objj].on == 1)
		return 1;

	return 0;
}

void SetObjectGraphic(int obn, int slott) {
	if (!is_valid_object(obn)) quit("!SetObjectGraphic: invalid object specified");

	if (_G(objs)[obn].num != slott) {
		_G(objs)[obn].num = Math::InRangeOrDef<uint16_t>(slott, 0);
		if (slott > UINT16_MAX)
			debug_script_warn("Warning: object's (id %d) sprite %d is outside of internal range (%d), reset to 0", obn, slott, UINT16_MAX);
		debug_script_log("Object %d graphic changed to slot %d", obn, slott);
	}
	_G(objs)[obn].cycling = 0;
	_G(objs)[obn].frame = 0;
	_G(objs)[obn].loop = 0;
	_G(objs)[obn].view = (uint16_t)-1;
}

int GetObjectGraphic(int obn) {
	if (!is_valid_object(obn)) quit("!GetObjectGraphic: invalid object specified");
	return _G(objs)[obn].num;
}

int GetObjectY(int objj) {
	if (!is_valid_object(objj)) quit("!GetObjectY: invalid object number");
	return _G(objs)[objj].y;
}

int IsObjectAnimating(int objj) {
	if (!is_valid_object(objj)) quit("!IsObjectAnimating: invalid object number");
	return (_G(objs)[objj].cycling != 0) ? 1 : 0;
}

int IsObjectMoving(int objj) {
	if (!is_valid_object(objj)) quit("!IsObjectMoving: invalid object number");
	return (_G(objs)[objj].moving > 0) ? 1 : 0;
}

void SetObjectPosition(int objj, int tox, int toy) {
	if (!is_valid_object(objj))
		quit("!SetObjectPosition: invalid object number");

	if (_G(objs)[objj].moving > 0) {
		debug_script_warn("Object.SetPosition: cannot set position while object is moving");
		return;
	}

	_G(objs)[objj].x = tox;
	_G(objs)[objj].y = toy;
}

void GetObjectName(int obj, char *buffer) {
	VALIDATE_STRING(buffer);
	if (!is_valid_object(obj))
		quit("!GetObjectName: invalid object number");

	strcpy(buffer, get_translation(_GP(thisroom).Objects[obj].Name.GetCStr()));
}

void MoveObject(int objj, int xx, int yy, int spp) {
	move_object(objj, xx, yy, spp, 0);
}
void MoveObjectDirect(int objj, int xx, int yy, int spp) {
	move_object(objj, xx, yy, spp, 1);
}

void SetObjectClickable(int cha, int clik) {
	if (!is_valid_object(cha))
		quit("!SetObjectClickable: Invalid object specified");
	_G(objs)[cha].flags &= ~OBJF_NOINTERACT;
	if (clik == 0)
		_G(objs)[cha].flags |= OBJF_NOINTERACT;
}

void SetObjectIgnoreWalkbehinds(int cha, int clik) {
	if (!is_valid_object(cha))
		quit("!SetObjectIgnoreWalkbehinds: Invalid object specified");
	if (_GP(game).options[OPT_BASESCRIPTAPI] >= kScriptAPI_v350)
		debug_script_warn("IgnoreWalkbehinds is not recommended for use, consider other solutions");
	_G(objs)[cha].flags &= ~OBJF_NOWALKBEHINDS;
	if (clik)
		_G(objs)[cha].flags |= OBJF_NOWALKBEHINDS;
	// clear the cache
	_G(objcache)[cha].ywas = -9999;
}

void RunObjectInteraction(int aa, int mood) {
	if (!is_valid_object(aa))
		quit("!RunObjectInteraction: invalid object number for current room");
	int passon = -1, cdata = -1;
	if (mood == MODE_LOOK) passon = 0;
	else if (mood == MODE_HAND) passon = 1;
	else if (mood == MODE_TALK) passon = 2;
	else if (mood == MODE_PICKUP) passon = 5;
	else if (mood == MODE_CUSTOM1) passon = 6;
	else if (mood == MODE_CUSTOM2) passon = 7;
	else if (mood == MODE_USE) {
		passon = 3;
		cdata = _G(playerchar)->activeinv;
		_GP(play).usedinv = cdata;
	}
	_G(evblockbasename) = "object%d";
	_G(evblocknum) = aa;

	if (_GP(thisroom).Objects[aa].EventHandlers != nullptr) {
		if (passon >= 0) {
			if (run_interaction_script(_GP(thisroom).Objects[aa].EventHandlers.get(), passon, 4, (passon == 3)))
				return;
		}
		run_interaction_script(_GP(thisroom).Objects[aa].EventHandlers.get(), 4);  // any click on obj
	} else {
		if (passon >= 0) {
			if (run_interaction_event(&_G(croom)->intrObject[aa], passon, 4, (passon == 3)))
				return;
		}
		run_interaction_event(&_G(croom)->intrObject[aa], 4);  // any click on obj
	}
}

int AreObjectsColliding(int obj1, int obj2) {
	if ((!is_valid_object(obj1)) | (!is_valid_object(obj2)))
		quit("!AreObjectsColliding: invalid object specified");

	return (AreThingsOverlapping(obj1 + OVERLAPPING_OBJECT, obj2 + OVERLAPPING_OBJECT)) ? 1 : 0;
}

int GetThingRect(int thing, _Rect *rect) {
	if (is_valid_character(thing)) {
		if (_GP(game).chars[thing].room != _G(displayed_room))
			return 0;

		int charwid = game_to_data_coord(GetCharacterWidth(thing));
		rect->x1 = _GP(game).chars[thing].x - (charwid / 2);
		rect->x2 = rect->x1 + charwid;
		rect->y1 = _GP(game).chars[thing].get_effective_y() - game_to_data_coord(GetCharacterHeight(thing));
		rect->y2 = _GP(game).chars[thing].get_effective_y();
	} else if (is_valid_object(thing - OVERLAPPING_OBJECT)) {
		int objid = thing - OVERLAPPING_OBJECT;
		if (_G(objs)[objid].on != 1)
			return 0;
		rect->x1 = _G(objs)[objid].x;
		rect->x2 = _G(objs)[objid].x + game_to_data_coord(_G(objs)[objid].get_width());
		rect->y1 = _G(objs)[objid].y - game_to_data_coord(_G(objs)[objid].get_height());
		rect->y2 = _G(objs)[objid].y;
	} else
		quit("!AreThingsOverlapping: invalid parameter");

	return 1;
}

int AreThingsOverlapping(int thing1, int thing2) {
	_Rect r1, r2;
	// get the bounding rectangles, and return 0 if the object/char
	// is currently turned off
	if (GetThingRect(thing1, &r1) == 0)
		return 0;
	if (GetThingRect(thing2, &r2) == 0)
		return 0;

	if ((r1.x2 > r2.x1) && (r1.x1 < r2.x2) &&
	        (r1.y2 > r2.y1) && (r1.y1 < r2.y2)) {
		// determine how far apart they are
		// take the smaller of the X distances as the overlapping amount
		int xdist = abs(r1.x2 - r2.x1);
		if (abs(r1.x1 - r2.x2) < xdist)
			xdist = abs(r1.x1 - r2.x2);
		// take the smaller of the Y distances
		int ydist = abs(r1.y2 - r2.y1);
		if (abs(r1.y1 - r2.y2) < ydist)
			ydist = abs(r1.y1 - r2.y2);
		// the overlapping amount is the smaller of the X and Y ovrlap
		if (xdist < ydist)
			return xdist;
		else
			return ydist;
		//    return 1;
	}
	return 0;
}

int GetObjectProperty(int hss, const char *property) {
	if (!is_valid_object(hss))
		quit("!GetObjectProperty: invalid object");
	return get_int_property(_GP(thisroom).Objects[hss].Properties, _G(croom)->objProps[hss], property);
}

void GetObjectPropertyText(int item, const char *property, char *bufer) {
	get_text_property(_GP(thisroom).Objects[item].Properties, _G(croom)->objProps[item], property, bufer);
}

Bitmap *GetObjectImage(int obj, int *isFlipped) {
	if (!_G(gfxDriver)->HasAcceleratedTransform()) {
		if (_G(actsps)[obj] != nullptr) {
			// the _G(actsps) image is pre-flipped, so no longer register the image as such
			if (isFlipped)
				*isFlipped = 0;

			return _G(actsps)[obj];
		}
	}

	return _GP(spriteset)[_G(objs)[obj].num];
}

} // namespace AGS3
