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
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/objectcache.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/roomobject.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/viewframe.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/script/script.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gfx/gfx_def.h"

namespace AGS3 {

using namespace AGS::Shared;

#define OVERLAPPING_OBJECT 1000

extern RoomStatus *croom;
extern RoomObject *objs;
extern ViewStruct *views;
extern GameSetupStruct game;
extern ObjectCache objcache[MAX_ROOM_OBJECTS];
extern RoomStruct thisroom;
extern CharacterInfo *playerchar;
extern int displayed_room;
extern SpriteCache spriteset;
extern int actSpsCount;
extern Bitmap **actsps;
extern IDriverDependantBitmap **actspsbmp;
extern IGraphicsDriver *gfxDriver;

// Used for deciding whether a char or obj was closer
int obj_lowest_yp;

int GetObjectIDAtScreen(int scrx, int scry) {
	// translate screen co-ordinates to room co-ordinates
	VpPoint vpt = play.ScreenToRoomDivDown(scrx, scry);
	if (vpt.second < 0)
		return -1;
	return GetObjectIDAtRoom(vpt.first.X, vpt.first.Y);
}

int GetObjectIDAtRoom(int roomx, int roomy) {
	int aa, bestshotyp = -1, bestshotwas = -1;
	// Iterate through all objects in the room
	for (aa = 0; aa < croom->numobj; aa++) {
		if (objs[aa].on != 1) continue;
		if (objs[aa].flags & OBJF_NOINTERACT)
			continue;
		int xxx = objs[aa].x, yyy = objs[aa].y;
		int isflipped = 0;
		int spWidth = game_to_data_coord(objs[aa].get_width());
		int spHeight = game_to_data_coord(objs[aa].get_height());
		if (objs[aa].view >= 0)
			isflipped = views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame].flags & VFLG_FLIPSPRITE;

		Bitmap *theImage = GetObjectImage(aa, &isflipped);

		if (is_pos_in_sprite(roomx, roomy, xxx, yyy - spHeight, theImage,
			spWidth, spHeight, isflipped) == FALSE)
			continue;

		int usebasel = objs[aa].get_baseline();
		if (usebasel < bestshotyp) continue;

		bestshotwas = aa;
		bestshotyp = usebasel;
	}
	obj_lowest_yp = bestshotyp;
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

	objs[obj].tint_r = red;
	objs[obj].tint_g = green;
	objs[obj].tint_b = blue;
	objs[obj].tint_level = opacity;
	objs[obj].tint_light = (luminance * 25) / 10;
	objs[obj].flags &= ~OBJF_HASLIGHT;
	objs[obj].flags |= OBJF_HASTINT;
}

void RemoveObjectTint(int obj) {
	if (!is_valid_object(obj))
		quit("!RemoveObjectTint: invalid object");

	if (objs[obj].flags & (OBJF_HASTINT | OBJF_HASLIGHT)) {
		debug_script_log("Un-tint object %d", obj);
		objs[obj].flags &= ~(OBJF_HASTINT | OBJF_HASLIGHT);
	} else {
		debug_script_warn("RemoveObjectTint called but object was not tinted");
	}
}

void SetObjectView(int obn, int vii) {
	if (!is_valid_object(obn)) quit("!SetObjectView: invalid object number specified");
	debug_script_log("Object %d set to view %d", obn, vii);
	if ((vii < 1) || (vii > game.numviews)) {
		quitprintf("!SetObjectView: invalid view number (You said %d, max is %d)", vii, game.numviews);
	}
	vii--;

	objs[obn].view = vii;
	objs[obn].frame = 0;
	if (objs[obn].loop >= views[vii].numLoops)
		objs[obn].loop = 0;
	objs[obn].cycling = 0;
	objs[obn].num = views[vii].loops[0].frames[0].pic;
}

void SetObjectFrame(int obn, int viw, int lop, int fra) {
	if (!is_valid_object(obn)) quit("!SetObjectFrame: invalid object number specified");
	viw--;
	if (viw < 0 || viw >= game.numviews) quitprintf("!SetObjectFrame: invalid view number used (%d, range is 0 - %d)", viw, game.numviews - 1);
	if (lop < 0 || lop >= views[viw].numLoops) quitprintf("!SetObjectFrame: invalid loop number used (%d, range is 0 - %d)", lop, views[viw].numLoops - 1);
	// AGS < 3.5.1 let user to pass literally any positive invalid frame value by silently reassigning it to zero...
	if (loaded_game_file_version < kGameVersion_351) {
		if (fra >= views[viw].loops[lop].numFrames) {
			debug_script_warn("SetObjectFrame: frame index out of range (%d, must be 0 - %d), set to 0", fra, views[viw].loops[lop].numFrames - 1);
			fra = 0;
		}
	}
	if (fra < 0 || fra >= views[viw].loops[lop].numFrames) quitprintf("!SetObjectFrame: invalid frame number used (%d, range is 0 - %d)", fra, views[viw].loops[lop].numFrames - 1);
	// AGS >= 3.2.0 do not let assign an empty loop
	// NOTE: pre-3.2.0 games are converting views from ViewStruct272 struct, always has at least 1 frame
	if (loaded_game_file_version >= kGameVersion_320) {
		if (views[viw].loops[lop].numFrames == 0)
			quit("!SetObjectFrame: specified loop has no frames");
	}
	objs[obn].view = viw;
	objs[obn].loop = lop;
	objs[obn].frame = fra;
	objs[obn].cycling = 0;
	objs[obn].num = views[viw].loops[lop].frames[fra].pic;
	CheckViewFrame(viw, objs[obn].loop, objs[obn].frame);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
void SetObjectTransparency(int obn, int trans) {
	if (!is_valid_object(obn)) quit("!SetObjectTransparent: invalid object number specified");
	if ((trans < 0) || (trans > 100)) quit("!SetObjectTransparent: transparency value must be between 0 and 100");

	objs[obn].transparent = GfxDef::Trans100ToLegacyTrans255(trans);
}



void SetObjectBaseline(int obn, int basel) {
	if (!is_valid_object(obn)) quit("!SetObjectBaseline: invalid object number specified");
	// baseline has changed, invalidate the cache
	if (objs[obn].baseline != basel) {
		objcache[obn].ywas = -9999;
		objs[obn].baseline = basel;
	}
}

int GetObjectBaseline(int obn) {
	if (!is_valid_object(obn)) quit("!GetObjectBaseline: invalid object number specified");

	if (objs[obn].baseline < 1)
		return 0;

	return objs[obn].baseline;
}

void AnimateObjectImpl(int obn, int loopn, int spdd, int rept, int direction, int blocking, int sframe) {
	if (obn >= MANOBJNUM) {
		scAnimateCharacter(obn - 100, loopn, spdd, rept);
		return;
	}
	if (!is_valid_object(obn))
		quit("!AnimateObject: invalid object number specified");
	if (objs[obn].view < 0)
		quit("!AnimateObject: object has not been assigned a view");
	if (loopn < 0 || loopn >= views[objs[obn].view].numLoops)
		quit("!AnimateObject: invalid loop number specified");
	if (sframe < 0 || sframe >= views[objs[obn].view].loops[loopn].numFrames)
		quit("!AnimateObject: invalid starting frame number specified");
	if ((direction < 0) || (direction > 1))
		quit("!AnimateObjectEx: invalid direction");
	if ((rept < 0) || (rept > 2))
		quit("!AnimateObjectEx: invalid repeat value");
	if (views[objs[obn].view].loops[loopn].numFrames < 1)
		quit("!AnimateObject: no frames in the specified view loop");

	debug_script_log("Obj %d start anim view %d loop %d, speed %d, repeat %d, frame %d", obn, objs[obn].view + 1, loopn, spdd, rept, sframe);

	objs[obn].cycling = rept + 1 + (direction * 10);
	objs[obn].loop = loopn;
	// reverse animation starts at the *previous frame*
	if (direction) {
		sframe--;
		if (sframe < 0)
			sframe = views[objs[obn].view].loops[loopn].numFrames - (-sframe);
	}
	objs[obn].frame = sframe;

	objs[obn].overall_speed = spdd;
	objs[obn].wait = spdd + views[objs[obn].view].loops[loopn].frames[objs[obn].frame].speed;
	objs[obn].num = views[objs[obn].view].loops[loopn].frames[objs[obn].frame].pic;
	CheckViewFrame(objs[obn].view, loopn, objs[obn].frame);

	if (blocking)
		GameLoopUntilValueIsZero(&objs[obn].cycling);
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
	//abuf = thisroom.BgFrames.Graphic[play.bg_frame];
	PBitmap bg_frame = thisroom.BgFrames[play.bg_frame].Graphic;
	if (bg_frame->GetColorDepth() != actsps[obn]->GetColorDepth())
		quit("!MergeObject: unable to merge object due to color depth differences");

	int xpos = data_to_game_coord(objs[obn].x);
	int ypos = (data_to_game_coord(objs[obn].y) - theHeight);

	draw_sprite_support_alpha(bg_frame.get(), false, xpos, ypos, actsps[obn], (game.SpriteInfos[objs[obn].num].Flags & SPF_ALPHACHANNEL) != 0);
	invalidate_screen();
	mark_current_background_dirty();

	//abuf = oldabuf;
	// mark the sprite as merged
	objs[obn].on = 2;
	debug_script_log("Object %d merged into background", obn);
}

void StopObjectMoving(int objj) {
	if (!is_valid_object(objj))
		quit("!StopObjectMoving: invalid object number");
	objs[objj].moving = 0;

	debug_script_log("Object %d stop moving", objj);
}

void ObjectOff(int obn) {
	if (!is_valid_object(obn)) quit("!ObjectOff: invalid object specified");
	// don't change it if on == 2 (merged)
	if (objs[obn].on == 1) {
		objs[obn].on = 0;
		debug_script_log("Object %d turned off", obn);
		StopObjectMoving(obn);
	}
}

void ObjectOn(int obn) {
	if (!is_valid_object(obn)) quit("!ObjectOn: invalid object specified");
	if (objs[obn].on == 0) {
		objs[obn].on = 1;
		debug_script_log("Object %d turned on", obn);
	}
}

int IsObjectOn(int objj) {
	if (!is_valid_object(objj)) quit("!IsObjectOn: invalid object number");

	// ==1 is on, ==2 is merged into background
	if (objs[objj].on == 1)
		return 1;

	return 0;
}

void SetObjectGraphic(int obn, int slott) {
	if (!is_valid_object(obn)) quit("!SetObjectGraphic: invalid object specified");

	if (objs[obn].num != slott) {
		objs[obn].num = slott;
		debug_script_log("Object %d graphic changed to slot %d", obn, slott);
	}
	objs[obn].cycling = 0;
	objs[obn].frame = 0;
	objs[obn].loop = 0;
	objs[obn].view = -1;
}

int GetObjectGraphic(int obn) {
	if (!is_valid_object(obn)) quit("!GetObjectGraphic: invalid object specified");
	return objs[obn].num;
}

int GetObjectY(int objj) {
	if (!is_valid_object(objj)) quit("!GetObjectY: invalid object number");
	return objs[objj].y;
}

int IsObjectAnimating(int objj) {
	if (!is_valid_object(objj)) quit("!IsObjectAnimating: invalid object number");
	return (objs[objj].cycling != 0) ? 1 : 0;
}

int IsObjectMoving(int objj) {
	if (!is_valid_object(objj)) quit("!IsObjectMoving: invalid object number");
	return (objs[objj].moving > 0) ? 1 : 0;
}

void SetObjectPosition(int objj, int tox, int toy) {
	if (!is_valid_object(objj))
		quit("!SetObjectPosition: invalid object number");

	if (objs[objj].moving > 0) {
		debug_script_warn("Object.SetPosition: cannot set position while object is moving");
		return;
	}

	objs[objj].x = tox;
	objs[objj].y = toy;
}

void GetObjectName(int obj, char *buffer) {
	VALIDATE_STRING(buffer);
	if (!is_valid_object(obj))
		quit("!GetObjectName: invalid object number");

	strcpy(buffer, get_translation(thisroom.Objects[obj].Name));
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
	objs[cha].flags &= ~OBJF_NOINTERACT;
	if (clik == 0)
		objs[cha].flags |= OBJF_NOINTERACT;
}

void SetObjectIgnoreWalkbehinds(int cha, int clik) {
	if (!is_valid_object(cha))
		quit("!SetObjectIgnoreWalkbehinds: Invalid object specified");
	if (game.options[OPT_BASESCRIPTAPI] >= kScriptAPI_v350)
		debug_script_warn("IgnoreWalkbehinds is not recommended for use, consider other solutions");
	objs[cha].flags &= ~OBJF_NOWALKBEHINDS;
	if (clik)
		objs[cha].flags |= OBJF_NOWALKBEHINDS;
	// clear the cache
	objcache[cha].ywas = -9999;
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
		cdata = playerchar->activeinv;
		play.usedinv = cdata;
	}
	evblockbasename = "object%d";
	evblocknum = aa;

	if (thisroom.Objects[aa].EventHandlers != nullptr) {
		if (passon >= 0) {
			if (run_interaction_script(thisroom.Objects[aa].EventHandlers.get(), passon, 4, (passon == 3)))
				return;
		}
		run_interaction_script(thisroom.Objects[aa].EventHandlers.get(), 4);  // any click on obj
	} else {
		if (passon >= 0) {
			if (run_interaction_event(&croom->intrObject[aa], passon, 4, (passon == 3)))
				return;
		}
		run_interaction_event(&croom->intrObject[aa], 4); // any click on obj
	}
}

int AreObjectsColliding(int obj1, int obj2) {
	if ((!is_valid_object(obj1)) | (!is_valid_object(obj2)))
		quit("!AreObjectsColliding: invalid object specified");

	return (AreThingsOverlapping(obj1 + OVERLAPPING_OBJECT, obj2 + OVERLAPPING_OBJECT)) ? 1 : 0;
}

int GetThingRect(int thing, _Rect *rect) {
	if (is_valid_character(thing)) {
		if (game.chars[thing].room != displayed_room)
			return 0;

		int charwid = game_to_data_coord(GetCharacterWidth(thing));
		rect->x1 = game.chars[thing].x - (charwid / 2);
		rect->x2 = rect->x1 + charwid;
		rect->y1 = game.chars[thing].get_effective_y() - game_to_data_coord(GetCharacterHeight(thing));
		rect->y2 = game.chars[thing].get_effective_y();
	} else if (is_valid_object(thing - OVERLAPPING_OBJECT)) {
		int objid = thing - OVERLAPPING_OBJECT;
		if (objs[objid].on != 1)
			return 0;
		rect->x1 = objs[objid].x;
		rect->x2 = objs[objid].x + game_to_data_coord(objs[objid].get_width());
		rect->y1 = objs[objid].y - game_to_data_coord(objs[objid].get_height());
		rect->y2 = objs[objid].y;
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
	return get_int_property(thisroom.Objects[hss].Properties, croom->objProps[hss], property);
}

void GetObjectPropertyText(int item, const char *property, char *bufer) {
	get_text_property(thisroom.Objects[item].Properties, croom->objProps[item], property, bufer);
}

Bitmap *GetObjectImage(int obj, int *isFlipped) {
	if (!gfxDriver->HasAcceleratedTransform()) {
		if (actsps[obj] != nullptr) {
			// the actsps image is pre-flipped, so no longer register the image as such
			if (isFlipped)
				*isFlipped = 0;

			return actsps[obj];
		}
	}
	return spriteset[objs[obj].num];
}

} // namespace AGS3
