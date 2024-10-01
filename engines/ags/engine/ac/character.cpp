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
// AGS Character functions
//
//=============================================================================

#include "ags/engine/ac/character.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/lip_sync.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/update.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/util/string_compat.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/move_list.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

bool is_valid_character(int char_id) {
	return ((char_id >= 0) && (char_id < _GP(game).numcharacters));
}

bool AssertCharacter(const char *apiname, int char_id) {
	if ((char_id >= 0) && (char_id < _GP(game).numcharacters))
		return true;
	debug_script_warn("%s: invalid character id %d (range is 0..%d)", apiname, char_id, _GP(game).numcharacters - 1);
	return false;
}

void Character_AddInventory(CharacterInfo *chaa, ScriptInvItem *invi, int addIndex) {
	int ee;

	if (invi == nullptr)
		quit("!AddInventoryToCharacter: invalid inventory number");

	int inum = invi->id;

	if (chaa->inv[inum] >= 32000)
		quit("!AddInventory: cannot carry more than 32000 of one inventory item");

	chaa->inv[inum]++;

	int charid = chaa->index_id;

	if (_GP(game).options[OPT_DUPLICATEINV] == 0) {
		// Ensure it is only in the list once
		for (ee = 0; ee < _GP(charextra)[charid].invorder_count; ee++) {
			if (_GP(charextra)[charid].invorder[ee] == inum) {
				// They already have the item, so don't add it to the list
				if (chaa == _G(playerchar))
					run_on_event(GE_ADD_INV, RuntimeScriptValue().SetInt32(inum));
				return;
			}
		}
	}
	if (_GP(charextra)[charid].invorder_count >= MAX_INVORDER)
		quit("!Too many inventory items added, max 500 display at one time");

	if ((addIndex == SCR_NO_VALUE) ||
	        (addIndex >= _GP(charextra)[charid].invorder_count) ||
	        (addIndex < 0)) {
		// add new item at end of list
		_GP(charextra)[charid].invorder[_GP(charextra)[charid].invorder_count] = inum;
	} else {
		// insert new item at index
		for (ee = _GP(charextra)[charid].invorder_count - 1; ee >= addIndex; ee--)
			_GP(charextra)[charid].invorder[ee + 1] = _GP(charextra)[charid].invorder[ee];

		_GP(charextra)[charid].invorder[addIndex] = inum;
	}
	_GP(charextra)[charid].invorder_count++;
	GUI::MarkInventoryForUpdate(charid, charid == _GP(game).playercharacter);
	if (chaa == _G(playerchar))
		run_on_event(GE_ADD_INV, RuntimeScriptValue().SetInt32(inum));
}

void Character_AddWaypoint(CharacterInfo *chaa, int x, int y) {

	if (chaa->room != _G(displayed_room))
		quitprintf("!MoveCharacterPath: character %s is not in current room %d (it is in room %d)",
				   chaa->scrname, _G(displayed_room), chaa->room);

	// not already walking, so just do a normal move
	if (chaa->walking <= 0) {
		Character_Walk(chaa, x, y, IN_BACKGROUND, ANYWHERE);
		return;
	}

	MoveList *cmls = &_GP(mls)[chaa->walking % TURNING_AROUND];
	if (cmls->numstage >= MAXNEEDSTAGES) {
		debug_script_warn("Character::AddWaypoint: move is too complex, cannot add any further paths");
		return;
	}

	// They're already walking there anyway
	const Point &last_pos = cmls->GetLastPos();
	if (last_pos == Point(x, y))
		return;

	int move_speed_x, move_speed_y;
	chaa->get_effective_walkspeeds(move_speed_x, move_speed_y);
	if ((move_speed_x == 0) && (move_speed_y == 0)) {
		debug_script_warn("Character::AddWaypoint: called for '%s' with walk speed 0", chaa->scrname);
	}

	// There's an issue: the existing movelist is converted to room resolution,
	// so we do this trick: convert last step to mask resolution, before calling
	// a pathfinder api, and then we'll convert old and new last step back.
	// TODO: figure out a better way of processing this!
	const int last_stage = cmls->numstage - 1;
	cmls->pos[last_stage] = { room_to_mask_coord(cmls->pos[last_stage].X), room_to_mask_coord(cmls->pos[last_stage].Y) };
	const int dst_x = room_to_mask_coord(x);
	const int dst_y = room_to_mask_coord(y);
	if(add_waypoint_direct(cmls, dst_x, dst_y, move_speed_x, move_speed_y))
		convert_move_path_to_room_resolution(cmls, last_stage, last_stage + 1);
}

void Character_Animate(CharacterInfo *chaa, int loop, int delay, int repeat,
	int blocking, int direction, int sframe, int volume) {

	ValidateViewAnimVLF("Character.Animate", chaa->view, loop, sframe);
	ValidateViewAnimParams("Character.Animate", repeat, blocking, direction);

	animate_character(chaa, loop, delay, repeat, 0, direction, sframe, volume);

	if (blocking != 0)
		GameLoopUntilValueIsZero(&chaa->animating);
}

void Character_Animate5(CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction) {
	Character_Animate(chaa, loop, delay, repeat, blocking, direction, 0 /* first frame */, 100 /* full volume */);
}

void Character_Animate6(CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction, int sframe) {
	Character_Animate(chaa, loop, delay, repeat, blocking, direction, sframe, 100 /* full volume */);
}

void Character_ChangeRoomAutoPosition(CharacterInfo *chaa, int room, int newPos) {
	if (chaa->index_id != _GP(game).playercharacter) {
		quit("!Character.ChangeRoomAutoPosition can only be used with the player character.");
	}

	_G(new_room_pos) = newPos;

	if (_G(new_room_pos) == 0) {
		// auto place on other side of screen
		if (chaa->x <= _GP(thisroom).Edges.Left + 10)
			_G(new_room_pos) = 2000;
		else if (chaa->x >= _GP(thisroom).Edges.Right - 10)
			_G(new_room_pos) = 1000;
		else if (chaa->y <= _GP(thisroom).Edges.Top + 10)
			_G(new_room_pos) = 3000;
		else if (chaa->y >= _GP(thisroom).Edges.Bottom - 10)
			_G(new_room_pos) = 4000;

		if (_G(new_room_pos) < 3000)
			_G(new_room_pos) += chaa->y;
		else
			_G(new_room_pos) += chaa->x;
	}
	NewRoom(room);
}

void Character_ChangeRoom(CharacterInfo *chaa, int room, int x, int y) {
	Character_ChangeRoomSetLoop(chaa, room, x, y, SCR_NO_VALUE);
}

void Character_ChangeRoomSetLoop(CharacterInfo *chaa, int room, int x, int y, int direction) {

	if (chaa->index_id != _GP(game).playercharacter) {
		// NewRoomNPC
		if ((x != SCR_NO_VALUE) && (y != SCR_NO_VALUE)) {
			chaa->x = x;
			chaa->y = y;
			if (direction != SCR_NO_VALUE && direction >= 0) chaa->loop = direction;
		}
		chaa->prevroom = chaa->room;
		chaa->room = room;

		debug_script_log("%s moved to room %d, location %d,%d, loop %d",
		                 chaa->scrname, room, chaa->x, chaa->y, chaa->loop);

		return;
	}

	if ((x != SCR_NO_VALUE) && (y != SCR_NO_VALUE)) {
		// We cannot set character position right away,
		// because room switch will occur only after the script end,
		// and character position may be still changing meanwhile.
		_G(new_room_pos) = 0;

		// don't check X or Y bounds, so that they can do a
		// walk-in animation if they want
		_G(new_room_x) = x;
		_G(new_room_y) = y;
		if (direction != SCR_NO_VALUE)
			_G(new_room_loop) = direction;
	}

	NewRoom(room);
}


void Character_ChangeView(CharacterInfo *chap, int vii) {
	vii--;

	if ((vii < 0) || (vii >= _GP(game).numviews))
		quit("!ChangeCharacterView: invalid view number specified");

	// if animating, but not idle view, give warning message
	if ((chap->flags & CHF_FIXVIEW) && (chap->idleleft >= 0))
		debug_script_warn("Warning: ChangeCharacterView was used while the view was fixed - call ReleaseCharView first");

	// if the idle animation is playing we should release the view
	if (chap->idleleft < 0) {
		Character_UnlockView(chap);
		chap->idleleft = chap->idletime;
	}

	debug_script_log("%s: Change view to %d", chap->scrname, vii + 1);
	chap->defview = vii;
	chap->view = vii;
	stop_character_anim(chap);
	chap->frame = 0;
	chap->wait = 0;
	chap->walkwait = 0;
	_GP(charextra)[chap->index_id].animwait = 0;
	FindReasonableLoopForCharacter(chap);
}

enum DirectionalLoop {
	kDirLoop_Down      = 0,
	kDirLoop_Left      = 1,
	kDirLoop_Right     = 2,
	kDirLoop_Up        = 3,
	kDirLoop_DownRight = 4,
	kDirLoop_UpRight   = 5,
	kDirLoop_DownLeft  = 6,
	kDirLoop_UpLeft    = 7,

	kDirLoop_Default        = kDirLoop_Down,
	kDirLoop_LastOrthogonal = kDirLoop_Up,
	kDirLoop_Last           = kDirLoop_UpLeft,
};

// Internal direction-facing functions

DirectionalLoop GetDirectionalLoop(CharacterInfo *chinfo, int x_diff, int y_diff) {
	DirectionalLoop next_loop = kDirLoop_Left; // NOTE: default loop was Left for some reason

	const ViewStruct &chview  = _GP(views)[chinfo->view];
	const bool new_version    = _G(loaded_game_file_version) > kGameVersion_272;
	const bool has_down_loop  = ((chview.numLoops > kDirLoop_Down)  && (chview.loops[kDirLoop_Down].numFrames > 0));
	const bool has_up_loop    = ((chview.numLoops > kDirLoop_Up)    && (chview.loops[kDirLoop_Up].numFrames > 0));
	// NOTE: 3.+ games required left & right loops to be present at all times
	const bool has_left_loop  = new_version ||
	                            ((chview.numLoops > kDirLoop_Left)  && (chview.loops[kDirLoop_Left].numFrames > 0));
	const bool has_right_loop = new_version ||
	                            ((chview.numLoops > kDirLoop_Right) && (chview.loops[kDirLoop_Right].numFrames > 0));
	const bool has_diagonal_loops = useDiagonal(chinfo) == 0; // NOTE: useDiagonal returns 0 for "true"

	const bool want_horizontal = (abs(y_diff) < abs(x_diff)) ||
	                             (new_version && (!has_down_loop || !has_up_loop)) ||
	                             // NOTE: <= 2.72 games switch to horizontal loops only if both vertical ones are missing
	                             (!new_version && (!has_down_loop && !has_up_loop));
	if (want_horizontal) {
		const bool want_diagonal = has_diagonal_loops && (abs(y_diff) > abs(x_diff) / 2);
		if (!has_left_loop && !has_right_loop) {
			next_loop = kDirLoop_Down;
		} else if (has_right_loop && (x_diff > 0)) {
			next_loop = want_diagonal ? (y_diff < 0 ? kDirLoop_UpRight : kDirLoop_DownRight) :
			            kDirLoop_Right;
		} else if (has_left_loop && (x_diff <= 0)) {
			next_loop = want_diagonal ? (y_diff < 0 ? kDirLoop_UpLeft : kDirLoop_DownLeft) :
			            kDirLoop_Left;
		}
	} else {
		const bool want_diagonal = has_diagonal_loops && (abs(x_diff) > abs(y_diff) / 2);
		if (y_diff > 0 || !has_up_loop) {
			next_loop = want_diagonal ? (x_diff < 0 ? kDirLoop_DownLeft : kDirLoop_DownRight) :
			            kDirLoop_Down;
		} else {
			next_loop = want_diagonal ? (x_diff < 0 ? kDirLoop_UpLeft : kDirLoop_UpRight) :
			            kDirLoop_Up;
		}
	}
	return next_loop;
}

void FaceDirectionalLoop(CharacterInfo *char1, int direction, int blockingStyle) {
	// Change facing only if the desired direction is different
	if (direction != char1->loop) {
		if ((_GP(game).options[OPT_TURNTOFACELOC] != 0) &&
		        (_G(in_enters_screen) == 0)) {
			const int no_diagonal = useDiagonal(char1);
			const int highestLoopForTurning = no_diagonal != 1 ? kDirLoop_Last : kDirLoop_LastOrthogonal;
			if ((char1->loop <= highestLoopForTurning)) {
				// Turn to face new direction
				Character_StopMoving(char1);
				if (char1->on == 1) {
					// only do the turning if the character is not hidden
					// (otherwise GameLoopUntilNotMoving will never return)
					start_character_turning(char1, direction, no_diagonal);

					if ((blockingStyle == BLOCKING) || (blockingStyle == 1))
						GameLoopUntilNotMoving(&char1->walking);
				} else
					char1->loop = direction;
			} else
				char1->loop = direction;
		} else
			char1->loop = direction;
	}

	char1->frame = 0;
}

void FaceLocationXY(CharacterInfo *char1, int xx, int yy, int blockingStyle) {
	debug_script_log("%s: Face location %d,%d", char1->scrname, xx, yy);

	const int diffrx = xx - char1->x;
	const int diffry = yy - char1->y;

	if ((diffrx == 0) && (diffry == 0)) {
		// FaceLocation called on their current position - do nothing
		return;
	}

	FaceDirectionalLoop(char1, GetDirectionalLoop(char1, diffrx, diffry), blockingStyle);
}

// External direction-facing functions with validation

void Character_FaceDirection(CharacterInfo *char1, int direction, int blockingStyle) {
	if (char1 == nullptr)
		quit("!FaceDirection: invalid character specified");

	if (direction != SCR_NO_VALUE) {
		if (direction < 0 || direction > kDirLoop_Last)
			quit("!FaceDirection: invalid direction specified");

		FaceDirectionalLoop(char1, direction, blockingStyle);
	}
}

void Character_FaceLocation(CharacterInfo *char1, int xx, int yy, int blockingStyle) {
	if (char1 == nullptr)
		quit("!FaceLocation: invalid character specified");

	FaceLocationXY(char1, xx, yy, blockingStyle);
}

void Character_FaceObject(CharacterInfo *char1, ScriptObject *obj, int blockingStyle) {
	if (obj == nullptr)
		quit("!FaceObject: invalid object specified");

	FaceLocationXY(char1, _G(objs)[obj->id].x, _G(objs)[obj->id].y, blockingStyle);
}

void Character_FaceCharacter(CharacterInfo *char1, CharacterInfo *char2, int blockingStyle) {
	if (char2 == nullptr)
		quit("!FaceCharacter: invalid character specified");

	if (char1->room != char2->room)
		quitprintf("!FaceCharacter: characters %s and %s are in different rooms (room %d and room %d respectively)",
				   char1->scrname, char2->scrname, char1->room, char2->room);

	FaceLocationXY(char1, char2->x, char2->y, blockingStyle);
}

void Character_FollowCharacter(CharacterInfo *chaa, CharacterInfo *tofollow, int distaway, int eagerness) {

	if ((eagerness < 0) || (eagerness > 250))
		quit("!FollowCharacterEx: invalid eagerness: must be 0-250");

	if ((chaa->index_id == _GP(game).playercharacter) && (tofollow != nullptr) &&
	        (tofollow->room != chaa->room))
		quitprintf("!FollowCharacterEx: you cannot tell the player character %s, who is in room %d, to follow a character %s who is in another room %d",
				   chaa->scrname, chaa->room, tofollow->scrname, tofollow->room);

	if (tofollow != nullptr) {
		debug_script_log("%s: Start following %s (dist %d, eager %d)", chaa->scrname, tofollow->scrname, distaway, eagerness);
	} else {
		debug_script_log("%s: Stop following other character", chaa->scrname);
	}

	if ((chaa->following >= 0) &&
	        (chaa->followinfo == FOLLOW_ALWAYSONTOP)) {
		// if this character was following always-on-top, its baseline will
		// have been changed, so release it.
		chaa->baseline = -1;
	}

	if (tofollow == nullptr)
		chaa->following = -1;
	else
		chaa->following = tofollow->index_id;

	chaa->followinfo = (distaway << 8) | eagerness;

	chaa->flags &= ~CHF_BEHINDSHEPHERD;

	// special case for Always On Other Character
	if (distaway == FOLLOW_ALWAYSONTOP) {
		chaa->followinfo = FOLLOW_ALWAYSONTOP;
		if (eagerness == 1)
			chaa->flags |= CHF_BEHINDSHEPHERD;
	}

	if (chaa->animating & CHANIM_REPEAT)
		debug_script_warn("Warning: FollowCharacter called but the sheep is currently animating looped. It may never start to follow.");

}

int Character_IsCollidingWithChar(CharacterInfo *char1, CharacterInfo *char2) {
	if (char2 == nullptr)
		quit("!AreCharactersColliding: invalid char2");

	if (char1->room != char2->room) return 0; // not colliding

	if ((char1->y > char2->y - 5) && (char1->y < char2->y + 5)) ;
	else return 0;

	int w1 = game_to_data_coord(GetCharacterWidth(char1->index_id));
	int w2 = game_to_data_coord(GetCharacterWidth(char2->index_id));

	int xps1 = char1->x - w1 / 2;
	int xps2 = char2->x - w2 / 2;

	if ((xps1 >= xps2 - w1) & (xps1 <= xps2 + w2)) return 1;
	return 0;
}

int Character_IsCollidingWithObject(CharacterInfo *chin, ScriptObject *objid) {
	if (objid == nullptr)
		quit("!AreCharObjColliding: invalid object number");

	if (chin->room != _G(displayed_room))
		return 0;
	if (_G(objs)[objid->id].on != 1)
		return 0;

	Bitmap *checkblk = GetObjectImage(objid->id);
	int objWidth = checkblk->GetWidth();
	int objHeight = checkblk->GetHeight();
	int o1x = _G(objs)[objid->id].x;
	int o1y = _G(objs)[objid->id].y - game_to_data_coord(objHeight);

	Bitmap *charpic = GetCharacterImage(chin->index_id);

	int charWidth = charpic->GetWidth();
	int charHeight = charpic->GetHeight();
	int o2x = chin->x - game_to_data_coord(charWidth) / 2;
	int o2y = _GP(charextra)[chin->index_id].GetEffectiveY(chin) - 5; // only check feet

	if ((o2x >= o1x - game_to_data_coord(charWidth)) &&
	        (o2x <= o1x + game_to_data_coord(objWidth)) &&
	        (o2y >= o1y - 8) &&
	        (o2y <= o1y + game_to_data_coord(objHeight))) {
		// the character's feet are on the object
		if (_GP(game).options[OPT_PIXPERFECT] == 0)
			return 1;
		// check if they're on a transparent bit of the object
		int stxp = data_to_game_coord(o2x - o1x);
		int styp = data_to_game_coord(o2y - o1y);
		int maskcol = checkblk->GetMaskColor();
		int maskcolc = charpic->GetMaskColor();
		int thispix, thispixc;
		// check each pixel of the object along the char's feet
		for (int i = 0; i < charWidth; i += get_fixed_pixel_size(1)) {
			for (int j = 0; j < get_fixed_pixel_size(6); j += get_fixed_pixel_size(1)) {
				thispix = my_getpixel(checkblk, i + stxp, j + styp);
				thispixc = my_getpixel(charpic, i, j + (charHeight - get_fixed_pixel_size(5)));

				if ((thispix != -1) && (thispix != maskcol) &&
				        (thispixc != -1) && (thispixc != maskcolc))
					return 1;
			}
		}

	}
	return 0;
}

bool Character_IsInteractionAvailable(CharacterInfo *cchar, int mood) {

	_GP(play).check_interaction_only = 1;
	RunCharacterInteraction(cchar->index_id, mood);
	int ciwas = _GP(play).check_interaction_only;
	_GP(play).check_interaction_only = 0;
	return (ciwas == 2);
}

void Character_LockView(CharacterInfo *chap, int vii) {
	Character_LockViewEx(chap, vii, STOP_MOVING);
}

void Character_LockViewEx(CharacterInfo *chap, int vii, int stopMoving) {
	vii--; // convert to 0-based
	AssertView("SetCharacterView", vii);

	debug_script_log("%s: View locked to %d", chap->scrname, vii + 1);
	if (chap->idleleft < 0) {
		Character_UnlockView(chap);
		chap->idleleft = chap->idletime;
	}
	if (stopMoving != KEEP_MOVING) {
		Character_StopMoving(chap);
	}
	chap->view = vii;
	stop_character_anim(chap);
	FindReasonableLoopForCharacter(chap);
	chap->frame = 0;
	chap->wait = 0;
	chap->flags |= CHF_FIXVIEW;
	chap->pic_xoffs = 0;
	chap->pic_yoffs = 0;
}

void Character_LockViewAligned_Old(CharacterInfo *chap, int vii, int loop, int align) {
	Character_LockViewAlignedEx(chap, vii, loop, ConvertLegacyScriptAlignment((LegacyScriptAlignment)align), STOP_MOVING);
}

void Character_LockViewAlignedEx_Old(CharacterInfo *chap, int vii, int loop, int align, int stopMoving) {
	Character_LockViewAlignedEx(chap, vii, loop, ConvertLegacyScriptAlignment((LegacyScriptAlignment)align), stopMoving);
}

void Character_LockViewAligned(CharacterInfo *chap, int vii, int loop, int align) {
	Character_LockViewAlignedEx(chap, vii, loop, align, STOP_MOVING);
}

void Character_LockViewAlignedEx(CharacterInfo *chap, int vii, int loop, int align, int stopMoving) {
	if (chap->view < 0)
		quit("!SetCharacterLoop: character has invalid old view number");

	int sppic = _GP(views)[chap->view].loops[chap->loop].frames[chap->frame].pic;
	int leftSide = data_to_game_coord(chap->x) - _GP(game).SpriteInfos[sppic].Width / 2;

	Character_LockViewEx(chap, vii, stopMoving);

	AssertLoop("SetCharacterViewEx", chap->view, loop);

	chap->loop = loop;
	chap->frame = 0;
	int newpic = _GP(views)[chap->view].loops[chap->loop].frames[chap->frame].pic;
	int newLeft = data_to_game_coord(chap->x) - _GP(game).SpriteInfos[newpic].Width / 2;
	int xdiff = 0;

	if (align & kMAlignLeft)
		xdiff = leftSide - newLeft;
	else if (align & kMAlignHCenter)
		xdiff = 0;
	else if (align & kMAlignRight)
		xdiff = (leftSide + _GP(game).SpriteInfos[sppic].Width) - (newLeft + _GP(game).SpriteInfos[newpic].Width);
	else
		quit("!SetCharacterViewEx: invalid alignment type specified");

	chap->pic_xoffs = xdiff;
	chap->pic_yoffs = 0;
}

void Character_LockViewFrame(CharacterInfo *chaa, int view, int loop, int frame) {
	Character_LockViewFrameEx(chaa, view, loop, frame, STOP_MOVING);
}

void Character_LockViewFrameEx(CharacterInfo *chaa, int view, int loop, int frame, int stopMoving) {
	Character_LockViewEx(chaa, view, stopMoving);
	AssertFrame("SetCharacterFrame", view - 1, loop, frame);
	chaa->loop = loop;
	chaa->frame = frame;
}

void Character_LockViewOffset(CharacterInfo *chap, int vii, int xoffs, int yoffs) {
	Character_LockViewOffsetEx(chap, vii, xoffs, yoffs, STOP_MOVING);
}

void Character_LockViewOffsetEx(CharacterInfo *chap, int vii, int xoffs, int yoffs, int stopMoving) {
	Character_LockViewEx(chap, vii, stopMoving);

	// This function takes offsets in real game coordinates as opposed to script coordinates
	defgame_to_finalgame_coords(xoffs, yoffs);
	chap->pic_xoffs = xoffs;
	chap->pic_yoffs = yoffs;
}

void Character_LoseInventory(CharacterInfo *chap, ScriptInvItem *invi) {

	if (invi == nullptr)
		quit("!LoseInventoryFromCharacter: invalid inventory number");

	int inum = invi->id;

	if (chap->inv[inum] > 0)
		chap->inv[inum]--;

	if ((chap->activeinv == inum) & (chap->inv[inum] < 1)) {
		chap->activeinv = -1;
		if ((chap == _G(playerchar)) && (GetCursorMode() == MODE_USE))
			set_cursor_mode(0);
	}

	int charid = chap->index_id;

	if ((chap->inv[inum] == 0) || (_GP(game).options[OPT_DUPLICATEINV] > 0)) {
		int xx, tt;
		for (xx = 0; xx < _GP(charextra)[charid].invorder_count; xx++) {
			if (_GP(charextra)[charid].invorder[xx] == inum) {
				_GP(charextra)[charid].invorder_count--;
				for (tt = xx; tt < _GP(charextra)[charid].invorder_count; tt++)
					_GP(charextra)[charid].invorder[tt] = _GP(charextra)[charid].invorder[tt + 1];
				break;
			}
		}
	}
	GUI::MarkInventoryForUpdate(charid, charid == _GP(game).playercharacter);

	if (chap == _G(playerchar))
		run_on_event(GE_LOSE_INV, RuntimeScriptValue().SetInt32(inum));
}

void Character_PlaceOnWalkableArea(CharacterInfo *chap) {
	if (_G(displayed_room) < 0)
		quit("!Character.PlaceOnWalkableArea: no room is currently loaded");

	find_nearest_walkable_area(&chap->x, &chap->y);
}

void Character_RemoveTint(CharacterInfo *chaa) {

	if (chaa->flags & (CHF_HASTINT | CHF_HASLIGHT)) {
		debug_script_log("Un-tint %s", chaa->scrname);
		chaa->flags &= ~(CHF_HASTINT | CHF_HASLIGHT);
	} else {
		debug_script_warn("Character.RemoveTint called but character was not tinted");
	}
}

int Character_GetHasExplicitTint_Old(CharacterInfo *ch) {
	return ch->has_explicit_tint() || ch->has_explicit_light();
}

int Character_GetHasExplicitTint(CharacterInfo *ch) {
	return ch->has_explicit_tint();
}

void Character_Say(CharacterInfo *chaa, const char *text) {
	_DisplaySpeechCore(chaa->index_id, text);
}

void Character_SayAt(CharacterInfo *chaa, int x, int y, int width, const char *texx) {

	DisplaySpeechAt(x, y, width, chaa->index_id, texx);
}

ScriptOverlay *Character_SayBackground(CharacterInfo *chaa, const char *texx) {
	int ovltype = DisplaySpeechBackground(chaa->index_id, texx);
	auto *over = get_overlay(ovltype);
	if (!over)
		quit("!SayBackground internal error: no overlay");

	// Create script object with an internal ref, keep at least until internal timeout
	return create_scriptoverlay(*over, true);
}

void Character_SetAsPlayer(CharacterInfo *chaa) {

	// Set to same character, so ignore.
	// But only on versions > 2.61. The relevant entry in the 2.62 changelog is:
	//  - Fixed SetPlayerCharacter to do nothing at all if you pass the current
	//    player character to it (previously it was resetting the inventory layout)
	if ((_G(loaded_game_file_version) > kGameVersion_261) && (_GP(game).playercharacter == chaa->index_id))
		return;

	setup_player_character(chaa->index_id);

	//update_invorder();

	debug_script_log("%s is new player character", _G(playerchar)->scrname);

	// Within game_start, return now
	if (_G(displayed_room) < 0)
		return;

	// Ignore invalid room numbers for the character and just place him in
	// the current room for 2.x. Following script calls to NewRoom() will
	// make sure this still works as intended.
	if ((_G(loaded_game_file_version) <= kGameVersion_272) && (_G(playerchar)->room < 0))
		_G(playerchar)->room = _G(displayed_room);

	if (_G(displayed_room) != _G(playerchar)->room)
		NewRoom(_G(playerchar)->room);
	else   // make sure it doesn't run the region interactions
		_GP(play).player_on_region = GetRegionIDAtRoom(_G(playerchar)->x, _G(playerchar)->y);

	if ((_G(playerchar)->activeinv >= 0) && (_G(playerchar)->inv[_G(playerchar)->activeinv] < 1))
		_G(playerchar)->activeinv = -1;

	// They had inv selected, so change the cursor
	if (_G(cur_mode) == MODE_USE) {
		if (_G(playerchar)->activeinv < 0)
			SetNextCursor();
		else
			SetActiveInventory(_G(playerchar)->activeinv);
	}

}


void Character_SetIdleView(CharacterInfo *chaa, int iview, int itime) {

	if (iview == 1)
		quit("!SetCharacterIdle: view 1 cannot be used as an idle view, sorry.");

	// if an idle anim is currently playing, release it
	if (chaa->idleleft < 0)
		Character_UnlockView(chaa);

	chaa->idleview = iview - 1;
	// make sure they don't appear idle while idle anim is disabled
	if (iview < 1)
		itime = 10;
	chaa->idletime = itime;
	chaa->idleleft = itime;

	// if not currently animating, reset the wait counter
	if ((chaa->animating == 0) && (chaa->walking == 0))
		chaa->wait = 0;

	if (iview >= 1) {
		debug_script_log("Set %s idle view to %d (time %d)", chaa->scrname, iview, itime);
	} else {
		debug_script_log("%s idle view disabled", chaa->scrname);
	}
	if (chaa->flags & CHF_FIXVIEW) {
		debug_script_warn("SetCharacterIdle called while character view locked with SetCharacterView; idle ignored");
		debug_script_log("View locked, idle will not kick in until Released");
	}
	// if they switch to a swimming animation, kick it off immediately
	if (itime == 0)
		_GP(charextra)[chaa->index_id].process_idle_this_time = 1;

}

bool Character_GetHasExplicitLight(CharacterInfo *ch) {
	return ch->has_explicit_light();
}

int Character_GetLightLevel(CharacterInfo *ch) {
	return ch->has_explicit_light() ? _GP(charextra)[ch->index_id].tint_light : 0;
}

void Character_SetLightLevel(CharacterInfo *chaa, int light_level) {
	light_level = Math::Clamp(light_level, -100, 100);

	_GP(charextra)[chaa->index_id].tint_light = light_level;
	chaa->flags &= ~CHF_HASTINT;
	chaa->flags |= CHF_HASLIGHT;
}

int Character_GetTintRed(CharacterInfo *ch) {
	return ch->has_explicit_tint() ? _GP(charextra)[ch->index_id].tint_r : 0;
}

int Character_GetTintGreen(CharacterInfo *ch) {
	return ch->has_explicit_tint() ? _GP(charextra)[ch->index_id].tint_g : 0;
}

int Character_GetTintBlue(CharacterInfo *ch) {
	return ch->has_explicit_tint() ? _GP(charextra)[ch->index_id].tint_b : 0;
}

int Character_GetTintSaturation(CharacterInfo *ch) {
	return ch->has_explicit_tint() ? _GP(charextra)[ch->index_id].tint_level : 0;
}

int Character_GetTintLuminance(CharacterInfo *ch) {
	return ch->has_explicit_tint() ? ((_GP(charextra)[ch->index_id].tint_light * 10) / 25) : 0;
}

void Character_SetOption(CharacterInfo *chaa, int flag, int yesorno) {

	if ((yesorno < 0) || (yesorno > 1))
		quit("!SetCharacterProperty: last parameter must be 0 or 1");

	if (flag & CHF_MANUALSCALING) {
		// backwards compatibility fix
		Character_SetIgnoreScaling(chaa, yesorno);
	} else {
		chaa->flags &= ~flag;
		if (yesorno)
			chaa->flags |= flag;
	}
}

void Character_SetSpeed(CharacterInfo *chaa, int xspeed, int yspeed) {
	if ((xspeed == 0) || (yspeed == 0))
		quit("!SetCharacterSpeedEx: invalid speed value");
	if ((chaa->walking > 0) && (_G(loaded_game_file_version) < kGameVersion_361)) {
		debug_script_warn("Character_SetSpeed: cannot change speed while walking");
		return;
	}

	xspeed = Math::Clamp(xspeed, (int)INT16_MIN, (int)INT16_MAX);
	yspeed = Math::Clamp(yspeed, (int)INT16_MIN, (int)INT16_MAX);

	uint16_t old_speedx = chaa->walkspeed;
	uint16_t old_speedy = ((chaa->walkspeed_y == UNIFORM_WALK_SPEED) ? chaa->walkspeed : chaa->walkspeed_y);

	chaa->walkspeed = xspeed;
	if (yspeed == xspeed)
		chaa->walkspeed_y = UNIFORM_WALK_SPEED;
	else
		chaa->walkspeed_y = yspeed;

	if (chaa->walking > 0) {
		recalculate_move_speeds(&_GP(mls)[chaa->walking % TURNING_AROUND], old_speedx, old_speedy, xspeed, yspeed);
	}
}

void Character_StopMoving(CharacterInfo *charp) {

	int chaa = charp->index_id;
	if (chaa == _GP(play).skip_until_char_stops)
		EndSkippingUntilCharStops();

	if (_GP(charextra)[chaa].xwas != INVALID_X) {
		charp->x = _GP(charextra)[chaa].xwas;
		charp->y = _GP(charextra)[chaa].ywas;
		_GP(charextra)[chaa].xwas = INVALID_X;
	}
	if ((charp->walking > 0) && (charp->walking < TURNING_AROUND)) {
		// if it's not a MoveCharDirect, make sure they end up on a walkable area
		if ((_GP(mls)[charp->walking].direct == 0) && (charp->room == _G(displayed_room)))
			Character_PlaceOnWalkableArea(charp);

		debug_script_log("%s: stop moving", charp->scrname);

		charp->idleleft = charp->idletime;
		// restart the idle animation straight away
		_GP(charextra)[chaa].process_idle_this_time = 1;
	}
	if (charp->walking) {
		// If the character is currently moving, stop them and reset their frame
		charp->walking = 0;
		if ((charp->flags & CHF_MOVENOTWALK) == 0)
			charp->frame = 0;
	}
}

void Character_Tint(CharacterInfo *chaa, int red, int green, int blue, int opacity, int luminance) {
	if ((red < 0) || (green < 0) || (blue < 0) ||
	        (red > 255) || (green > 255) || (blue > 255) ||
	        (opacity < 0) || (opacity > 100) ||
	        (luminance < 0) || (luminance > 100))
		quit("!Character.Tint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

	debug_script_log("Set %s tint RGB(%d,%d,%d) %d%%", chaa->scrname, red, green, blue, opacity);

	_GP(charextra)[chaa->index_id].tint_r = red;
	_GP(charextra)[chaa->index_id].tint_g = green;
	_GP(charextra)[chaa->index_id].tint_b = blue;
	_GP(charextra)[chaa->index_id].tint_level = opacity;
	_GP(charextra)[chaa->index_id].tint_light = (luminance * 25) / 10;
	chaa->flags &= ~CHF_HASLIGHT;
	chaa->flags |= CHF_HASTINT;
}

void Character_Think(CharacterInfo *chaa, const char *text) {
	_DisplayThoughtCore(chaa->index_id, text);
}

void Character_UnlockView(CharacterInfo *chaa) {
	Character_UnlockViewEx(chaa, STOP_MOVING);
}

void Character_UnlockViewEx(CharacterInfo *chaa, int stopMoving) {
	if (chaa->flags & CHF_FIXVIEW) {
		debug_script_log("%s: Released view back to default", chaa->scrname);
	}
	chaa->flags &= ~CHF_FIXVIEW;
	chaa->view = chaa->defview;
	chaa->frame = 0;
	if (stopMoving != KEEP_MOVING) {
		Character_StopMoving(chaa);
	}
	if (chaa->view >= 0) {
		int maxloop = _GP(views)[chaa->view].numLoops;
		if (((chaa->flags & CHF_NODIAGONAL) != 0) && (maxloop > 4))
			maxloop = 4;
		FindReasonableLoopForCharacter(chaa);
	}
	stop_character_anim(chaa);
	chaa->idleleft = chaa->idletime;
	chaa->pic_xoffs = 0;
	chaa->pic_yoffs = 0;
	// restart the idle animation straight away
	_GP(charextra)[chaa->index_id].process_idle_this_time = 1;

}


void Character_Walk(CharacterInfo *chaa, int x, int y, int blocking, int direct) {
	walk_or_move_character(chaa, x, y, blocking, direct, true);
}

void Character_Move(CharacterInfo *chaa, int x, int y, int blocking, int direct) {
	walk_or_move_character(chaa, x, y, blocking, direct, false);
}

void Character_WalkStraight(CharacterInfo *chaa, int xx, int yy, int blocking) {

	if (chaa->room != _G(displayed_room))
		quitprintf("!MoveCharacterStraight: character %s is not in current room %d (it is in room %d)",
				   chaa->scrname, _G(displayed_room), chaa->room);

	int movetox = xx, movetoy = yy;

	set_wallscreen(prepare_walkable_areas(chaa->index_id));

	int fromXLowres = room_to_mask_coord(chaa->x);
	int fromYLowres = room_to_mask_coord(chaa->y);
	int toXLowres = room_to_mask_coord(xx);
	int toYLowres = room_to_mask_coord(yy);

	if (!can_see_from(fromXLowres, fromYLowres, toXLowres, toYLowres)) {
		int lastcx, lastcy;
		get_lastcpos(lastcx, lastcy);
		movetox = mask_to_room_coord(lastcx);
		movetoy = mask_to_room_coord(lastcy);
	}

	walk_or_move_character(chaa, movetox, movetoy, blocking, 1 /* use ANYWHERE */, true);
}

void Character_RunInteraction(CharacterInfo *chaa, int mood) {

	RunCharacterInteraction(chaa->index_id, mood);
}



// **** CHARACTER: PROPERTIES ****

int Character_GetProperty(CharacterInfo *chaa, const char *property) {
	if (!AssertCharacter("Character.GetProperty", chaa->index_id))
		return 0;
	return get_int_property(_GP(game).charProps[chaa->index_id], _GP(play).charProps[chaa->index_id], property);
}

void Character_GetPropertyText(CharacterInfo *chaa, const char *property, char *bufer) {
	if (!AssertCharacter("Character.GetPropertyText", chaa->index_id))
		return;
	get_text_property(_GP(game).charProps[chaa->index_id], _GP(play).charProps[chaa->index_id], property, bufer);
}

const char *Character_GetTextProperty(CharacterInfo *chaa, const char *property) {
	if (!AssertCharacter("Character.GetTextProperty", chaa->index_id))
		return nullptr;
	return get_text_property_dynamic_string(_GP(game).charProps[chaa->index_id], _GP(play).charProps[chaa->index_id], property);
}

bool Character_SetProperty(CharacterInfo *chaa, const char *property, int value) {
	if (!AssertCharacter("Character.SetProperty", chaa->index_id))
		return false;
	return set_int_property(_GP(play).charProps[chaa->index_id], property, value);
}

bool Character_SetTextProperty(CharacterInfo *chaa, const char *property, const char *value) {
	if (!AssertCharacter("Character.SetTextProperty", chaa->index_id))
		return false;
	return set_text_property(_GP(play).charProps[chaa->index_id], property, value);
}

ScriptInvItem *Character_GetActiveInventory(CharacterInfo *chaa) {

	if (chaa->activeinv <= 0)
		return nullptr;

	return &_G(scrInv)[chaa->activeinv];
}

void Character_SetActiveInventory(CharacterInfo *chaa, ScriptInvItem *iit) {
	if (iit == nullptr) {
		chaa->activeinv = -1;

		if (chaa->index_id == _GP(game).playercharacter) {

			if (GetCursorMode() == MODE_USE)
				set_cursor_mode(0);
		}
		GUI::MarkInventoryForUpdate(chaa->index_id, chaa->index_id == _GP(game).playercharacter);
		return;
	}

	if (chaa->inv[iit->id] < 1) {
		debug_script_warn("SetActiveInventory: character doesn't have any of that inventory");
		return;
	}

	chaa->activeinv = iit->id;

	if (chaa->index_id == _GP(game).playercharacter) {
		// if it's the player character, update mouse cursor
		update_inv_cursor(iit->id);
		set_cursor_mode(MODE_USE);
	}
	GUI::MarkInventoryForUpdate(chaa->index_id, chaa->index_id == _GP(game).playercharacter);
}

int Character_GetAnimating(CharacterInfo *chaa) {
	if (chaa->animating)
		return 1;
	return 0;
}

int Character_GetAnimationSpeed(CharacterInfo *chaa) {
	return chaa->animspeed;
}

void Character_SetAnimationSpeed(CharacterInfo *chaa, int newval) {
	chaa->animspeed = newval;
	if (_G(loaded_game_file_version) < kGameVersion_360_16)
		chaa->idle_anim_speed = chaa->animspeed + 5;
}

int Character_GetAnimationVolume(CharacterInfo *chaa) {
	return _GP(charextra)[chaa->index_id].anim_volume;
}

void Character_SetAnimationVolume(CharacterInfo *chaa, int newval) {

	_GP(charextra)[chaa->index_id].anim_volume = Math::Clamp(newval, 0, 100);
}

int Character_GetBaseline(CharacterInfo *chaa) {
	if (chaa->baseline < 1)
		return 0;

	return chaa->baseline;
}

void Character_SetBaseline(CharacterInfo *chaa, int basel) {

	chaa->baseline = basel;
}

int Character_GetBlinkInterval(CharacterInfo *chaa) {

	return chaa->blinkinterval;
}

void Character_SetBlinkInterval(CharacterInfo *chaa, int interval) {

	if (interval < 0)
		quit("!SetCharacterBlinkView: invalid blink interval");

	chaa->blinkinterval = interval;

	if (chaa->blinktimer > 0)
		chaa->blinktimer = chaa->blinkinterval;
}

int Character_GetBlinkView(CharacterInfo *chaa) {

	return chaa->blinkview + 1;
}

void Character_SetBlinkView(CharacterInfo *chaa, int vii) {

	if (((vii < 2) || (vii > _GP(game).numviews)) && (vii != -1))
		quit("!SetCharacterBlinkView: invalid view number");

	chaa->blinkview = vii - 1;
}

int Character_GetBlinkWhileThinking(CharacterInfo *chaa) {
	if (chaa->flags & CHF_NOBLINKANDTHINK)
		return 0;
	return 1;
}

void Character_SetBlinkWhileThinking(CharacterInfo *chaa, int yesOrNo) {
	chaa->flags &= ~CHF_NOBLINKANDTHINK;
	if (yesOrNo == 0)
		chaa->flags |= CHF_NOBLINKANDTHINK;
}

int Character_GetBlockingHeight(CharacterInfo *chaa) {

	return chaa->blocking_height;
}

void Character_SetBlockingHeight(CharacterInfo *chaa, int hit) {

	chaa->blocking_height = hit;
}

int Character_GetBlockingWidth(CharacterInfo *chaa) {

	return chaa->blocking_width;
}

void Character_SetBlockingWidth(CharacterInfo *chaa, int wid) {

	chaa->blocking_width = wid;
}

int Character_GetDiagonalWalking(CharacterInfo *chaa) {

	if (chaa->flags & CHF_NODIAGONAL)
		return 0;
	return 1;
}

void Character_SetDiagonalWalking(CharacterInfo *chaa, int yesorno) {

	chaa->flags &= ~CHF_NODIAGONAL;
	if (!yesorno)
		chaa->flags |= CHF_NODIAGONAL;
}

int Character_GetClickable(CharacterInfo *chaa) {

	if (chaa->flags & CHF_NOINTERACT)
		return 0;
	return 1;
}

void Character_SetClickable(CharacterInfo *chaa, int clik) {

	chaa->flags &= ~CHF_NOINTERACT;
	// if they don't want it clickable, set the relevant bit
	if (clik == 0)
		chaa->flags |= CHF_NOINTERACT;
}

int Character_GetID(CharacterInfo *chaa) {

	return chaa->index_id;

}

const char *Character_GetScriptName(CharacterInfo *chin) {
	return CreateNewScriptString(_GP(game).chars2[chin->index_id].scrname_new);
}

int Character_GetFrame(CharacterInfo *chaa) {
	return chaa->frame;
}

void Character_SetFrame(CharacterInfo *chaa, int newval) {
	chaa->frame = newval;
}

int Character_GetIdleView(CharacterInfo *chaa) {

	if (chaa->idleview < 1)
		return -1;

	return chaa->idleview + 1;
}

int Character_GetIInventoryQuantity(CharacterInfo *chaa, int index) {
	if ((index < 1) || (index >= _GP(game).numinvitems))
		quitprintf("!Character.InventoryQuantity: invalid inventory index %d", index);

	return chaa->inv[index];
}

int Character_HasInventory(CharacterInfo *chaa, ScriptInvItem *invi) {
	if (invi == nullptr)
		quit("!Character.HasInventory: NULL inventory item supplied");

	return (chaa->inv[invi->id] > 0) ? 1 : 0;
}

void Character_SetIInventoryQuantity(CharacterInfo *chaa, int index, int quant) {
	if ((index < 1) || (index >= _GP(game).numinvitems))
		quitprintf("!Character.InventoryQuantity: invalid inventory index %d", index);

	if ((quant < 0) || (quant > 32000))
		quitprintf("!Character.InventoryQuantity: invalid quantity %d", quant);

	chaa->inv[index] = quant;
}

int Character_GetIgnoreLighting(CharacterInfo *chaa) {

	if (chaa->flags & CHF_NOLIGHTING)
		return 1;
	return 0;
}

void Character_SetIgnoreLighting(CharacterInfo *chaa, int yesorno) {

	chaa->flags &= ~CHF_NOLIGHTING;
	if (yesorno)
		chaa->flags |= CHF_NOLIGHTING;
}

int Character_GetIgnoreScaling(CharacterInfo *chaa) {

	if (chaa->flags & CHF_MANUALSCALING)
		return 1;
	return 0;
}

void Character_SetIgnoreScaling(CharacterInfo *chaa, int yesorno) {

	if (yesorno) {
		// when setting IgnoreScaling to 1, should reset zoom level
		// like it used to in pre-2.71
		_GP(charextra)[chaa->index_id].zoom = 100;
	}
	Character_SetManualScaling(chaa, yesorno);
}

void Character_SetManualScaling(CharacterInfo *chaa, int yesorno) {

	chaa->flags &= ~CHF_MANUALSCALING;
	if (yesorno)
		chaa->flags |= CHF_MANUALSCALING;
}

int Character_GetIgnoreWalkbehinds(CharacterInfo *chaa) {

	if (chaa->flags & CHF_NOWALKBEHINDS)
		return 1;
	return 0;
}

void Character_SetIgnoreWalkbehinds(CharacterInfo *chaa, int yesorno) {
	if (_GP(game).options[OPT_BASESCRIPTAPI] >= kScriptAPI_v350)
		debug_script_warn("IgnoreWalkbehinds is not recommended for use, consider other solutions");
	chaa->flags &= ~CHF_NOWALKBEHINDS;
	if (yesorno)
		chaa->flags |= CHF_NOWALKBEHINDS;
}

int Character_GetMovementLinkedToAnimation(CharacterInfo *chaa) {

	if (chaa->flags & CHF_ANTIGLIDE)
		return 1;
	return 0;
}

void Character_SetMovementLinkedToAnimation(CharacterInfo *chaa, int yesorno) {

	chaa->flags &= ~CHF_ANTIGLIDE;
	if (yesorno)
		chaa->flags |= CHF_ANTIGLIDE;
}

int Character_GetLoop(CharacterInfo *chaa) {
	return chaa->loop;
}

void Character_SetLoop(CharacterInfo *chaa, int newval) {
	AssertLoop("Character.Loop", chaa->view, newval);

	chaa->loop = newval;
	if (chaa->frame >= _GP(views)[chaa->view].loops[chaa->loop].numFrames)
		chaa->frame = 0;
}

int Character_GetMoving(CharacterInfo *chaa) {
	if (chaa->walking)
		return 1;
	return 0;
}

int Character_GetDestinationX(CharacterInfo *chaa) {
	if (chaa->walking) {
		MoveList *cmls = &_GP(mls)[chaa->walking % TURNING_AROUND];
		return cmls->pos[cmls->numstage - 1].X;
	} else
		return chaa->x;
}

int Character_GetDestinationY(CharacterInfo *chaa) {
	if (chaa->walking) {
		MoveList *cmls = &_GP(mls)[chaa->walking % TURNING_AROUND];
		return cmls->pos[cmls->numstage - 1].Y;
	} else
		return chaa->y;
}

const char *Character_GetName(CharacterInfo *chaa) {
	return CreateNewScriptString(_GP(game).chars2[chaa->index_id].name_new.GetCStr());
}

void Character_SetName(CharacterInfo *chaa, const char *newName) {
	_GP(game).chars2[chaa->index_id].name_new = newName;
	// Fill legacy name fields, for compatibility with old scripts and plugins
	snprintf(chaa->name, LEGACY_MAX_CHAR_NAME_LEN, "%s", newName);
	GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
}

int Character_GetNormalView(CharacterInfo *chaa) {
	return chaa->defview + 1;
}

int Character_GetPreviousRoom(CharacterInfo *chaa) {
	return chaa->prevroom;
}

int Character_GetRoom(CharacterInfo *chaa) {
	return chaa->room;
}


int Character_GetScaleMoveSpeed(CharacterInfo *chaa) {

	if (chaa->flags & CHF_SCALEMOVESPEED)
		return 1;
	return 0;
}

void Character_SetScaleMoveSpeed(CharacterInfo *chaa, int yesorno) {

	if ((yesorno < 0) || (yesorno > 1))
		quit("Character.ScaleMoveSpeed: value must be true or false (1 or 0)");

	chaa->flags &= ~CHF_SCALEMOVESPEED;
	if (yesorno)
		chaa->flags |= CHF_SCALEMOVESPEED;
}

int Character_GetScaleVolume(CharacterInfo *chaa) {

	if (chaa->flags & CHF_SCALEVOLUME)
		return 1;
	return 0;
}

void Character_SetScaleVolume(CharacterInfo *chaa, int yesorno) {

	if ((yesorno < 0) || (yesorno > 1))
		quit("Character.ScaleVolume: value must be true or false (1 or 0)");

	chaa->flags &= ~CHF_SCALEVOLUME;
	if (yesorno)
		chaa->flags |= CHF_SCALEVOLUME;
}

int Character_GetScaling(CharacterInfo *chaa) {
	return _GP(charextra)[chaa->index_id].zoom;
}

void Character_SetScaling(CharacterInfo *chaa, int zoomlevel) {

	if ((chaa->flags & CHF_MANUALSCALING) == 0) {
		debug_script_warn("Character.Scaling: cannot set property unless ManualScaling is enabled");
		return;
	}
	int zoom_fixed = Math::Clamp(zoomlevel, 1, (int)(INT16_MAX)); // CharacterExtras.zoom is int16
	if (zoomlevel != zoom_fixed)
		debug_script_warn("Character.Scaling: scaling level must be between 1 and %d%%, asked for: %d",
			(int)(INT16_MAX), zoomlevel);

	_GP(charextra)[chaa->index_id].zoom = zoom_fixed;
}

int Character_GetSolid(CharacterInfo *chaa) {

	if (chaa->flags & CHF_NOBLOCKING)
		return 0;
	return 1;
}

void Character_SetSolid(CharacterInfo *chaa, int yesorno) {

	chaa->flags &= ~CHF_NOBLOCKING;
	if (!yesorno)
		chaa->flags |= CHF_NOBLOCKING;
}

int Character_GetSpeaking(CharacterInfo *chaa) {
	if (get_character_currently_talking() == chaa->index_id)
		return 1;

	return 0;
}

int Character_GetSpeechColor(CharacterInfo *chaa) {

	return chaa->talkcolor;
}

void Character_SetSpeechColor(CharacterInfo *chaa, int ncol) {

	chaa->talkcolor = ncol;
}

void Character_SetSpeechAnimationDelay(CharacterInfo *chaa, int newDelay) {
	if (_GP(game).options[OPT_GLOBALTALKANIMSPD] != 0) {
		debug_script_warn("Character.SpeechAnimationDelay cannot be set when global speech animation speed is enabled");
		return;
	}

	chaa->speech_anim_speed = newDelay;
}

int Character_GetIdleAnimationDelay(CharacterInfo *chaa) {
	return chaa->idle_anim_speed;
}

void Character_SetIdleAnimationDelay(CharacterInfo *chaa, int newDelay) {
	chaa->idle_anim_speed = newDelay;
}

int Character_GetSpeechView(CharacterInfo *chaa) {
	return chaa->talkview + 1;
}

void Character_SetSpeechView(CharacterInfo *chaa, int vii) {
	if (vii == -1) {
		chaa->talkview = -1;
		return;
	}

	if ((vii < 1) || (vii > _GP(game).numviews))
		quit("!SetCharacterSpeechView: invalid view number");

	chaa->talkview = vii - 1;
}

bool Character_GetThinking(CharacterInfo *chaa) {
	return _G(char_thinking) == chaa->index_id;
}

int Character_GetThinkingFrame(CharacterInfo *chaa) {
	if (_G(char_thinking) == chaa->index_id)
		return chaa->thinkview > 0 ? chaa->frame : -1;

	debug_script_warn("Character.ThinkingFrame: character is not currently thinking");
	return -1;
}

int Character_GetThinkView(CharacterInfo *chaa) {

	return chaa->thinkview + 1;
}

void Character_SetThinkView(CharacterInfo *chaa, int vii) {
	if (((vii < 2) || (vii > _GP(game).numviews)) && (vii != -1))
		quit("!SetCharacterThinkView: invalid view number");

	chaa->thinkview = vii - 1;
}

int Character_GetTransparency(CharacterInfo *chaa) {

	return GfxDef::LegacyTrans255ToTrans100(chaa->transparency);
}

void Character_SetTransparency(CharacterInfo *chaa, int trans) {

	if ((trans < 0) || (trans > 100))
		quit("!SetCharTransparent: transparency value must be between 0 and 100");

	chaa->transparency = GfxDef::Trans100ToLegacyTrans255(trans);
}

int Character_GetTurnBeforeWalking(CharacterInfo *chaa) {

	if (chaa->flags & CHF_NOTURNING)
		return 0;
	return 1;
}

void Character_SetTurnBeforeWalking(CharacterInfo *chaa, int yesorno) {

	chaa->flags &= ~CHF_NOTURNING;
	if (!yesorno)
		chaa->flags |= CHF_NOTURNING;
}

int Character_GetView(CharacterInfo *chaa) {
	return chaa->view + 1;
}

int Character_GetWalkSpeedX(CharacterInfo *chaa) {
	return chaa->walkspeed;
}

int Character_GetWalkSpeedY(CharacterInfo *chaa) {
	if (chaa->walkspeed_y != UNIFORM_WALK_SPEED)
		return chaa->walkspeed_y;

	return chaa->walkspeed;
}

int Character_GetX(CharacterInfo *chaa) {
	return chaa->x;
}

void Character_SetX(CharacterInfo *chaa, int newval) {
	chaa->x = newval;
}

int Character_GetY(CharacterInfo *chaa) {
	return chaa->y;
}

void Character_SetY(CharacterInfo *chaa, int newval) {
	chaa->y = newval;
}

int Character_GetZ(CharacterInfo *chaa) {
	return chaa->z;
}

void Character_SetZ(CharacterInfo *chaa, int newval) {
	chaa->z = newval;
}



int Character_GetSpeakingFrame(CharacterInfo *chaa) {

	if ((_G(face_talking) >= 0) && (_G(facetalkrepeat))) {
		if (_G(facetalkchar)->index_id == chaa->index_id) {
			return _G(facetalkframe);
		}
	} else if (_G(char_speaking) >= 0) {
		if (_G(char_speaking) == chaa->index_id) {
			return chaa->frame;
		}
	}

	debug_script_warn("Character.SpeakingFrame: character is not currently speaking");
	return -1;
}

//=============================================================================

// order of loops to turn character in circle from down to down
int turnlooporder[8] = {0, 6, 1, 7, 3, 5, 2, 4};

void walk_character(int chac, int tox, int toy, int ignwal, bool autoWalkAnims) {
	CharacterInfo *chin = &_GP(game).chars[chac];
	if (chin->room != _G(displayed_room))
		quitprintf("!MoveCharacter: character %s is not in current room %d (it is in room %d)",
				   chin->scrname, _G(displayed_room), chin->room);

	chin->flags &= ~CHF_MOVENOTWALK;

	if ((tox == chin->x) && (toy == chin->y)) {
		StopMoving(chac);
		debug_script_log("%s already at destination, not moving", chin->scrname);
		return;
	}

	if ((chin->animating) && (autoWalkAnims))
		stop_character_anim(chin);

	if (chin->idleleft < 0) {
		ReleaseCharacterView(chac);
		chin->idleleft = chin->idletime;
	}
	// stop them to make sure they're on a walkable area
	// but save their frame first so that if they're already
	// moving it looks smoother
	int oldframe = chin->frame;
	int waitWas = 0, animWaitWas = 0;
	float wasStepFrac = 0.f;
	// if they are currently walking, save the current Wait
	if (chin->walking) {
		waitWas = chin->walkwait;
		animWaitWas = _GP(charextra)[chac].animwait;
		const auto &movelist = _GP(mls)[chin->walking % TURNING_AROUND];
		// We set (fraction + 1), because movelist is always +1 ahead of current character pos;
		if (movelist.onpart > 0.f)
			wasStepFrac = movelist.GetPixelUnitFraction() + movelist.GetStepLength();
	}

	StopMoving(chac);
	chin->frame = oldframe;
	// use toxPassedIn cached variable so the hi-res co-ordinates
	// are still displayed as such
	debug_script_log("%s: Start move to %d,%d", chin->scrname, tox, toy);

	int move_speed_x, move_speed_y;
	chin->get_effective_walkspeeds(move_speed_x, move_speed_y);
	if ((move_speed_x == 0) && (move_speed_y == 0)) {
		debug_script_warn("MoveCharacter: called for '%s' with walk speed 0", chin->scrname);
	}

	// Convert src and dest coords to the mask resolution, for pathfinder
	const int src_x = room_to_mask_coord(chin->x);
	const int src_y = room_to_mask_coord(chin->y);
	const int dst_x = room_to_mask_coord(tox);
	const int dst_y = room_to_mask_coord(toy);

	int mslot = find_route(src_x, src_y, dst_x, dst_y, move_speed_x, move_speed_y, prepare_walkable_areas(chac), chac + CHMLSOFFS, 1, ignwal);

	if (mslot > 0) {
		chin->walking = mslot;
		_GP(mls)[mslot].direct = ignwal;
		convert_move_path_to_room_resolution(&_GP(mls)[mslot]);

		if (wasStepFrac > 0.f) {
			_GP(mls)[mslot].SetPixelUnitFraction(wasStepFrac);
		}

		// cancel any pending waits on current animations
		// or if they were already moving, keep the current wait -
		// this prevents a glitch if MoveCharacter is called when they
		// are already moving
		if (autoWalkAnims) {
			chin->walkwait = waitWas;
			_GP(charextra)[chac].animwait = animWaitWas;

			if (_GP(mls)[mslot].pos[0] != _GP(mls)[mslot].pos[1]) {
				fix_player_sprite(&_GP(mls)[mslot], chin);
			}
		} else
			chin->flags |= CHF_MOVENOTWALK;
	} else if (autoWalkAnims) // pathfinder couldn't get a route, stand them still
		chin->frame = 0;
}

int find_looporder_index(int curloop) {
	int rr;
	for (rr = 0; rr < 8; rr++) {
		if (turnlooporder[rr] == curloop)
			return rr;
	}
	return 0;
}

// returns 0 to use diagonal, 1 to not
int useDiagonal(CharacterInfo *char1) {
	if ((_GP(views)[char1->view].numLoops < 8) || ((char1->flags & CHF_NODIAGONAL) != 0))
		return 1;
	// If they have just provided standing frames for loops 4-7, to
	// provide smoother turning
	if (_GP(views)[char1->view].loops[4].numFrames < 2)
		return 2;
	return 0;
}

// returns 1 normally, or 0 if they only have horizontal animations
int hasUpDownLoops(CharacterInfo *char1) {
	// if no loops in the Down animation
	// or no loops in the Up animation
	if ((_GP(views)[char1->view].loops[0].numFrames < 1) ||
	        (_GP(views)[char1->view].numLoops < 4) ||
	        (_GP(views)[char1->view].loops[3].numFrames < 1)) {
		return 0;
	}

	return 1;
}

void start_character_turning(CharacterInfo *chinf, int useloop, int no_diagonal) {
	// work out how far round they have to turn
	int fromidx = find_looporder_index(chinf->loop);
	int toidx = find_looporder_index(useloop);
	//Display("Curloop: %d, needloop: %d",chinf->loop, useloop);
	int ii, go_anticlock = 0;
	// work out whether anticlockwise is quicker or not
	if ((toidx > fromidx) && ((toidx - fromidx) > 4))
		go_anticlock = 1;
	if ((toidx < fromidx) && ((fromidx - toidx) < 4))
		go_anticlock = 1;
	// strip any current turning_around stages
	chinf->walking = chinf->walking % TURNING_AROUND;
	if (go_anticlock)
		chinf->walking += TURNING_BACKWARDS;
	else
		go_anticlock = -1;

	// Allow the diagonal frames just for turning
	if (no_diagonal == 2)
		no_diagonal = 0;

	for (ii = fromidx; ii != toidx; ii -= go_anticlock) {
		// Wrap the loop order into range [0-7]
		if (ii < 0)
			ii = 7;
		if (ii >= 8)
			ii = 0;
		if (ii == toidx)
			break;
		if ((turnlooporder[ii] >= 4) && (no_diagonal > 0))
			continue; // there are no diagonal loops
		if (turnlooporder[ii] >= _GP(views)[chinf->view].numLoops)
			continue; // no such loop
		if (_GP(views)[chinf->view].loops[turnlooporder[ii]].numFrames < 1)
			continue; // no frames in such loop
		chinf->walking += TURNING_AROUND;
	}

}

void fix_player_sprite(MoveList *cmls, CharacterInfo *chinf) {
	const fixed xpmove = cmls->xpermove[cmls->onstage];
	const fixed ypmove = cmls->ypermove[cmls->onstage];

	// if not moving, do nothing
	if ((xpmove == 0) && (ypmove == 0))
		return;

	const int useloop = GetDirectionalLoop(chinf, xpmove, ypmove);

	if ((_GP(game).options[OPT_ROTATECHARS] == 0) || ((chinf->flags & CHF_NOTURNING) != 0)) {
		chinf->loop = useloop;
		return;
	}
	if ((chinf->loop > kDirLoop_LastOrthogonal) && ((chinf->flags & CHF_NODIAGONAL) != 0)) {
		// They've just been playing an animation with an extended loop number,
		// so don't try and rotate using it
		chinf->loop = useloop;
		return;
	}
	if ((chinf->loop >= _GP(views)[chinf->view].numLoops) ||
	        (_GP(views)[chinf->view].loops[chinf->loop].numFrames < 1) ||
	        (hasUpDownLoops(chinf) == 0)) {
		// Character is not currently on a valid loop, so don't try to rotate
		// eg. left/right only view, but current loop 0
		chinf->loop = useloop;
		return;
	}
	const int no_diagonal = useDiagonal(chinf);
	start_character_turning(chinf, useloop, no_diagonal);
}

// Check whether two characters have walked into each other
int has_hit_another_character(int sourceChar) {

	// if the character who's moving doesn't Bitmap *, don't bother checking
	if (_GP(game).chars[sourceChar].flags & CHF_NOBLOCKING)
		return -1;

	for (int ww = 0; ww < _GP(game).numcharacters; ww++) {
		if (_GP(game).chars[ww].on != 1) continue;
		if (_GP(game).chars[ww].room != _G(displayed_room)) continue;
		if (ww == sourceChar) continue;
		if (_GP(game).chars[ww].flags & CHF_NOBLOCKING) continue;

		if (is_char_on_another(sourceChar, ww, nullptr, nullptr)) {
			// we are now overlapping character 'ww'
			if ((_GP(game).chars[ww].walking) &&
			        ((_GP(game).chars[ww].flags & CHF_AWAITINGMOVE) == 0))
				return ww;
		}

	}
	return -1;
}

// Does the next move from the character's movelist.
// Returns 1 if they are now waiting for another char to move,
// otherwise returns 0
int doNextCharMoveStep(CharacterInfo *chi, int &char_index, CharacterExtras *chex) {
	int ntf = 0, xwas = chi->x, ywas = chi->y;

	if (do_movelist_move(chi->walking, chi->x, chi->y) == 2) {
		if ((chi->flags & CHF_MOVENOTWALK) == 0)
			fix_player_sprite(&_GP(mls)[chi->walking], chi);
	}

	ntf = has_hit_another_character(char_index);
	if (ntf >= 0) {
		chi->walkwait = 30;
		if (_GP(game).chars[ntf].walkspeed < 5)
			chi->walkwait += (5 - _GP(game).chars[ntf].walkspeed) * 5;
		// we are now waiting for the other char to move, so
		// make sure he doesn't stop for us too

		chi->flags |= CHF_AWAITINGMOVE;

		if ((chi->flags & CHF_MOVENOTWALK) == 0) {
			chi->frame = 0;
			chex->animwait = chi->walkwait;
		}

		if ((chi->walking < 1) || (chi->walking >= TURNING_AROUND)) ;
		else if (_GP(mls)[chi->walking].onpart > 0.f) {
			_GP(mls)[chi->walking].onpart -= 1.f;
			chi->x = xwas;
			chi->y = ywas;
		}
		debug_script_log("%s: Bumped into %s, waiting for them to move", chi->scrname, _GP(game).chars[ntf].scrname);
		return 1;
	}
	return 0;
}

bool is_char_walking_ndirect(CharacterInfo *chi) {
	return ((chi->walking > 0) && (chi->walking < TURNING_AROUND)) &&
		(_GP(mls)[chi->walking].direct == 0);
}

int find_nearest_walkable_area_within(int *xx, int *yy, int range, int step) {
	int ex, ey, nearest = 99999, thisis, nearx = 0, neary = 0;
	int startx = 0, starty = 14;
	int roomWidthLowRes = room_to_mask_coord(_GP(thisroom).Width);
	int roomHeightLowRes = room_to_mask_coord(_GP(thisroom).Height);
	int xwidth = roomWidthLowRes, yheight = roomHeightLowRes;

	int xLowRes = room_to_mask_coord(xx[0]);
	int yLowRes = room_to_mask_coord(yy[0]);
	int rightEdge = room_to_mask_coord(_GP(thisroom).Edges.Right);
	int leftEdge = room_to_mask_coord(_GP(thisroom).Edges.Left);
	int topEdge = room_to_mask_coord(_GP(thisroom).Edges.Top);
	int bottomEdge = room_to_mask_coord(_GP(thisroom).Edges.Bottom);

	// tweak because people forget to move the edges sometimes
	// if the player is already over the edge, ignore it
	if (xLowRes >= rightEdge) rightEdge = roomWidthLowRes;
	if (xLowRes <= leftEdge) leftEdge = 0;
	if (yLowRes >= bottomEdge) bottomEdge = roomHeightLowRes;
	if (yLowRes <= topEdge) topEdge = 0;

	if (range > 0) {
		startx = xLowRes - range;
		starty = yLowRes - range;
		xwidth = startx + range * 2;
		yheight = starty + range * 2;
		if (startx < 0) startx = 0;
		if (starty < 10) starty = 10;
		if (xwidth > roomWidthLowRes) xwidth = roomWidthLowRes;
		if (yheight > roomHeightLowRes) yheight = roomHeightLowRes;
	}

	for (ex = startx; ex < xwidth; ex += step) {
		for (ey = starty; ey < yheight; ey += step) {
			// non-walkalbe, so don't go here
			if (_GP(thisroom).WalkAreaMask->GetPixel(ex, ey) == 0) continue;
			// off a screen edge, don't move them there
			if ((ex <= leftEdge) || (ex >= rightEdge) ||
			        (ey <= topEdge) || (ey >= bottomEdge))
				continue;
			// otherwise, calculate distance from target
			thisis = (int) ::sqrt((double)((ex - xLowRes) * (ex - xLowRes) + (ey - yLowRes) * (ey - yLowRes)));
			if (thisis < nearest) {
				nearest = thisis;
				nearx = ex;
				neary = ey;
			}
		}
	}
	if (nearest < 90000) {
		xx[0] = mask_to_room_coord(nearx);
		yy[0] = mask_to_room_coord(neary);
		return 1;
	}

	return 0;
}

void find_nearest_walkable_area(int *xx, int *yy) {

	int pixValue = _GP(thisroom).WalkAreaMask->GetPixel(room_to_mask_coord(xx[0]), room_to_mask_coord(yy[0]));
	// only fix this code if the game was built with 2.61 or above
	if (pixValue == 0 || (_G(loaded_game_file_version) >= kGameVersion_261 && pixValue < 1)) {
		// First, check every 2 pixels within immediate area
		if (!find_nearest_walkable_area_within(xx, yy, 20, 2)) {
			// If not, check whole screen at 5 pixel intervals
			find_nearest_walkable_area_within(xx, yy, -1, 5);
		}
	}

}

void FindReasonableLoopForCharacter(CharacterInfo *chap) {

	if (chap->loop >= _GP(views)[chap->view].numLoops)
		chap->loop = kDirLoop_Default;
	if (_GP(views)[chap->view].numLoops < 1)
		quitprintf("!View %d does not have any loops", chap->view + 1);

	// if the current loop has no frames, find one that does
	if (_GP(views)[chap->view].loops[chap->loop].numFrames < 1) {
		for (int i = 0; i < _GP(views)[chap->view].numLoops; i++) {
			if (_GP(views)[chap->view].loops[i].numFrames > 0) {
				chap->loop = i;
				break;
			}
		}
	}

}

void walk_or_move_character(CharacterInfo *chaa, int x, int y, int blocking, int direct, bool isWalk) {
	if (chaa->on != 1) {
		debug_script_warn("MoveCharacterBlocking: character is turned off and cannot be moved");
		return;
	}

	if ((direct == ANYWHERE) || (direct == 1))
		walk_character(chaa->index_id, x, y, 1, isWalk);
	else if ((direct == WALKABLE_AREAS) || (direct == 0))
		walk_character(chaa->index_id, x, y, 0, isWalk);
	else
		quit("!Character.Walk: Direct must be ANYWHERE or WALKABLE_AREAS");

	if ((blocking == BLOCKING) || (blocking == 1))
		GameLoopUntilNotMoving(&chaa->walking);
	else if ((blocking != IN_BACKGROUND) && (blocking != 0))
		quit("!Character.Walk: Blocking must be BLOCKING or IN_BACKGROUND");

}

int wantMoveNow(CharacterInfo *chi, CharacterExtras *chex) {
	// check most likely case first
	if ((chex->zoom == 100) || ((chi->flags & CHF_SCALEMOVESPEED) == 0))
		return 1;

	// the % checks don't work when the counter is negative, so once
	// it wraps round, correct it
	while (chi->walkwaitcounter < 0) {
		chi->walkwaitcounter += 12000;
	}

	// scaling 170-200%, move 175% speed
	if (chex->zoom >= 170) {
		if ((chi->walkwaitcounter % 4) >= 1)
			return 2;
		else
			return 1;
	}
	// scaling 140-170%, move 150% speed
	else if (chex->zoom >= 140) {
		if ((chi->walkwaitcounter % 2) == 1)
			return 2;
		else
			return 1;
	}
	// scaling 115-140%, move 125% speed
	else if (chex->zoom >= 115) {
		if ((chi->walkwaitcounter % 4) >= 3)
			return 2;
		else
			return 1;
	}
	// scaling 80-120%, normal speed
	else if (chex->zoom >= 80)
		return 1;
	// scaling 60-80%, move 75% speed
	if (chex->zoom >= 60) {
		if ((chi->walkwaitcounter % 4) >= 1)
			return -1;
		else if (chex->xwas != INVALID_X) {
			// move the second half of the movement to make it smoother
			chi->x = chex->xwas;
			chi->y = chex->ywas;
			chex->xwas = INVALID_X;
		}
	}
	// scaling 30-60%, move 50% speed
	else if (chex->zoom >= 30) {
		if ((chi->walkwaitcounter % 2) == 1)
			return -1;
		else if (chex->xwas != INVALID_X) {
			// move the second half of the movement to make it smoother
			chi->x = chex->xwas;
			chi->y = chex->ywas;
			chex->xwas = INVALID_X;
		}
	}
	// scaling 0-30%, move 25% speed
	else {
		if ((chi->walkwaitcounter % 4) >= 3)
			return -1;
		if (((chi->walkwaitcounter % 4) == 1) && (chex->xwas != INVALID_X)) {
			// move the second half of the movement to make it smoother
			chi->x = chex->xwas;
			chi->y = chex->ywas;
			chex->xwas = INVALID_X;
		}

	}

	return 0;
}

void setup_player_character(int charid) {
	_GP(game).playercharacter = charid;
	_G(playerchar) = &_GP(game).chars[charid];
	_G(sc_PlayerCharPtr) = ccGetObjectHandleFromAddress(_G(playerchar));
	if (_G(loaded_game_file_version) < kGameVersion_270) {
		ccAddExternalScriptObject("player", _G(playerchar), &_GP(ccDynamicCharacter));
	}
}

// Animate character internal implementation;
// this function may be called by the game logic too, so we assume
// the arguments must be correct, and do not fix them up as we do for API functions.
void animate_character(CharacterInfo *chap, int loopn, int sppd, int rept, int noidleoverride, int direction, int sframe, int volume) {
	// If idle view in progress for the character (and this is not the
	// "start idle animation" animate_character call), stop the idle anim
	if ((chap->idleleft < 0) && (noidleoverride == 0)) {
		Character_UnlockView(chap);
		chap->idleleft = chap->idletime;
	}

	if ((chap->view < 0) || (chap->view > _GP(game).numviews) ||
		(loopn < 0) || (loopn >= _GP(views)[chap->view].numLoops)) {
		quitprintf("!AnimateCharacter: invalid view and/or loop\n"
				   "(trying to animate '%s' using view %d (range is 1..%d) and loop %d (view has %d loops)).",
				   chap->scrname, chap->view + 1, _GP(game).numviews, loopn, _GP(views)[chap->view].numLoops);
	}
	// NOTE: there's always frame 0 allocated for safety
	sframe = std::max(0, std::min(sframe, _GP(views)[chap->view].loops[loopn].numFrames - 1));
	debug_script_log("%s: Start anim view %d loop %d, spd %d, repeat %d, frame: %d",
					 chap->scrname, chap->view + 1, loopn, sppd, rept, sframe);

	Character_StopMoving(chap);

	chap->set_animating(rept != 0, direction == 0, sppd);
	chap->loop = loopn;
	chap->frame = SetFirstAnimFrame(chap->view, loopn, sframe, direction);

	chap->wait = sppd + _GP(views)[chap->view].loops[loopn].frames[chap->frame].speed;
	_GP(charextra)[chap->index_id].cur_anim_volume = Math::Clamp(volume, 0, 100);

	_GP(charextra)[chap->index_id].CheckViewFrame(chap);
}

void stop_character_anim(CharacterInfo *chap) { // TODO: may expand with resetting more properties,
  // but have to be careful to not break logic somewhere
	chap->animating = 0;
	_GP(charextra)[chap->index_id].cur_anim_volume = 100;
}

int GetCharacterFrameVolume(CharacterInfo *chi) {
	// We view the audio property relation as the relation of the entities:
	// system -> audio type -> audio emitter (character) -> animation's audio
	// therefore the sound volume is a multiplication of factors.
	int frame_vol = 100; // default to full volume
	// Try the active animation volume
	if (_GP(charextra)[chi->index_id].cur_anim_volume >= 0)
		frame_vol = _GP(charextra)[chi->index_id].cur_anim_volume;
	// Try the character's own animation volume property
	if (_GP(charextra)[chi->index_id].anim_volume >= 0)
		frame_vol = frame_vol * _GP(charextra)[chi->index_id].anim_volume / 100;
	// Try the character's zoom volume scaling (optional)
	// NOTE: historically scales only in 0-100 range :/
	if (chi->flags & CHF_SCALEVOLUME) {
		int zoom_level = _GP(charextra)[chi->index_id].zoom;
		if (zoom_level <= 0)
			zoom_level = 100;
		else
			zoom_level = std::min(zoom_level, 100);
		frame_vol = frame_vol * zoom_level / 100;
	}
	return frame_vol;
}

Bitmap *GetCharacterImage(int charid, bool *is_original) {
	// NOTE: the cached image will only be present in software render mode
	Bitmap *actsp = get_cached_character_image(charid);
	if (is_original)
		*is_original = !actsp; // no cached means we use original sprite
	if (actsp)
		return actsp;
	CharacterInfo *chin = &_GP(game).chars[charid];
	int sppic = _GP(views)[chin->view].loops[chin->loop].frames[chin->frame].pic;
	return _GP(spriteset)[sppic];
}

CharacterInfo *GetCharacterAtScreen(int xx, int yy) {
	int hsnum = GetCharIDAtScreen(xx, yy);
	if (hsnum < 0)
		return nullptr;
	return &_GP(game).chars[hsnum];
}

CharacterInfo *GetCharacterAtRoom(int x, int y) {
	int hsnum = is_pos_on_character(x, y);
	if (hsnum < 0)
		return nullptr;
	return &_GP(game).chars[hsnum];
}

void update_character_scale(int charid) {
	// Test for valid view and loop
	CharacterInfo &chin = _GP(game).chars[charid];
	if (chin.on == 0 || chin.room != _G(displayed_room))
		return; // not enabled, or in a different room

	CharacterExtras &chex = _GP(charextra)[charid];
	if (chin.view < 0) {
		quitprintf("!The character '%s' was turned on in the current room (room %d) but has not been assigned a view number.",
				   chin.scrname, _G(displayed_room));
	}
	if (chin.loop >= _GP(views)[chin.view].numLoops) {
		quitprintf("!The character '%s' could not be displayed because there was no loop %d of view %d.",
				   chin.scrname, chin.loop, chin.view + 1);
	}
	// If frame is too high -- fallback to the frame 0;
	// there's always at least 1 dummy frame at index 0
	if (chin.frame >= _GP(views)[chin.view].loops[chin.loop].numFrames) {
		chin.frame = 0;
	}

	int zoom, zoom_offs, scale_width, scale_height;
	update_object_scale(zoom, scale_width, scale_height,
						chin.x, chin.y, _GP(views)[chin.view].loops[chin.loop].frames[chin.frame].pic,
						chex.zoom, (chin.flags & CHF_MANUALSCALING) == 0);
	zoom_offs = (_GP(game).options[OPT_SCALECHAROFFSETS] != 0) ? zoom : 100;

	// Calculate the X & Y co-ordinates of where the sprite will be;
	// for the character sprite's origin is at the bottom-mid of a sprite.
	const int atxp = (data_to_game_coord(chin.x)) - scale_width / 2;
	const int atyp = (data_to_game_coord(chin.y) - scale_height)
					 // adjust the Y positioning for the character's Z co-ord
					 - (data_to_game_coord(chin.z) * zoom_offs / 100);

	// Save calculated properties
	chex.width = scale_width;
	chex.height = scale_height;
	chin.actx = atxp;
	chin.acty = atyp;
	chex.zoom = zoom;
	chex.zoom_offs = zoom_offs;
}

int is_pos_on_character(int xx, int yy) {
	int cc, sppic, lowestyp = 0, lowestwas = -1;
	for (cc = 0; cc < _GP(game).numcharacters; cc++) {
		if (_GP(game).chars[cc].room != _G(displayed_room)) continue;
		if (_GP(game).chars[cc].on == 0) continue;
		if (_GP(game).chars[cc].flags & CHF_NOINTERACT) continue;
		if (_GP(game).chars[cc].view < 0) continue;
		CharacterInfo *chin = &_GP(game).chars[cc];

		if ((chin->view < 0) ||
		        (chin->loop >= _GP(views)[chin->view].numLoops) ||
		        (chin->frame >= _GP(views)[chin->view].loops[chin->loop].numFrames)) {
			continue;
		}

		sppic = _GP(views)[chin->view].loops[chin->loop].frames[chin->frame].pic;
		int usewid = _GP(charextra)[cc].width;
		int usehit = _GP(charextra)[cc].height;
		if (usewid == 0) usewid = _GP(game).SpriteInfos[sppic].Width;
		if (usehit == 0) usehit = _GP(game).SpriteInfos[sppic].Height;
		int xxx = chin->x - game_to_data_coord(usewid) / 2;
		int yyy = _GP(charextra)[cc].GetEffectiveY(chin) - game_to_data_coord(usehit);

		int mirrored = _GP(views)[chin->view].loops[chin->loop].frames[chin->frame].flags & VFLG_FLIPSPRITE;

		bool is_original;
		Bitmap *theImage = GetCharacterImage(cc, &is_original);
		if (!is_original)
			mirrored = 0; // transformed image is already flipped

		if (is_pos_in_sprite(xx, yy, xxx, yyy, theImage,
		                     game_to_data_coord(usewid),
		                     game_to_data_coord(usehit), mirrored, is_original) == FALSE)
			continue;

		int use_base = chin->get_baseline();
		if (use_base < lowestyp) continue;
		lowestyp = use_base;
		lowestwas = cc;
	}
	_G(char_lowest_yp) = lowestyp;
	return lowestwas;
}

void get_char_blocking_rect(int charid, int *x1, int *y1, int *width, int *y2) {
	CharacterInfo *char1 = &_GP(game).chars[charid];
	int cwidth, fromx;

	if (char1->blocking_width < 1)
		cwidth = game_to_data_coord(GetCharacterWidth(charid)) - 4;
	else
		cwidth = char1->blocking_width;

	fromx = char1->x - cwidth / 2;
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
	if (y1)
		*y1 = char1->get_blocking_top();
	if (y2)
		*y2 = char1->get_blocking_bottom();
}

// Check whether the source char has walked onto character ww
int is_char_on_another(int sourceChar, int ww, int *fromxptr, int *cwidptr) {

	int fromx, cwidth;
	int y1, y2;
	get_char_blocking_rect(ww, &fromx, &y1, &cwidth, &y2);

	if (fromxptr)
		fromxptr[0] = fromx;
	if (cwidptr)
		cwidptr[0] = cwidth;

	// if the character trying to move is already on top of
	// this char somehow, allow them through
	if ((sourceChar >= 0) &&
	        // x/width are left and width co-ords, so they need >= and <
	        (_GP(game).chars[sourceChar].x >= fromx) &&
	        (_GP(game).chars[sourceChar].x < fromx + cwidth) &&
	        // y1/y2 are the top/bottom co-ords, so they need >= / <=
	        (_GP(game).chars[sourceChar].y >= y1) &&
	        (_GP(game).chars[sourceChar].y <= y2))
		return 1;

	return 0;
}

int my_getpixel(Bitmap *blk, int x, int y) {
	if ((x < 0) || (y < 0) || (x >= blk->GetWidth()) || (y >= blk->GetHeight()))
		return -1;

	// strip the alpha channel
	// TODO: is there a way to do this vtable thing with Bitmap?
	BITMAP *al_bmp = (BITMAP *)blk->GetAllegroBitmap();
	return al_bmp->getpixel(x, y) & 0x00ffffff;
}

int check_click_on_character(int xx, int yy, int mood) {
	int lowestwas = is_pos_on_character(xx, yy);
	if (lowestwas >= 0) {
		RunCharacterInteraction(lowestwas, mood);
		return 1;
	}
	return 0;
}

void _DisplaySpeechCore(int chid, const char *displbuf) {
	if (displbuf[0] == 0) {
		// no text, just update the current character who's speaking
		// this allows the portrait side to be switched with an empty
		// speech line
		_GP(play).swap_portrait_lastchar = chid;
		return;
	}

	// adjust timing of text (so that DisplaySpeech("%s", str) pauses
	// for the length of the string not 2 frames)
	int len = (int)strlen(displbuf);
	if (len > _G(source_text_length) + 3)
		_G(source_text_length) = len;

	DisplaySpeech(displbuf, chid);
}

void _DisplayThoughtCore(int chid, const char *displbuf) {
	// adjust timing of text (so that DisplayThought("%s", str) pauses
	// for the length of the string not 2 frames)
	int len = (int)strlen(displbuf);
	if (len > _G(source_text_length) + 3)
		_G(source_text_length) = len;

	int xpp = -1, ypp = -1, width = -1;

	if ((_GP(game).options[OPT_SPEECHTYPE] == 0) || (_GP(game).chars[chid].thinkview <= 0)) {
		// lucasarts-style, so we want a speech bubble actually above
		// their head (or if they have no think anim in Sierra-style)
		width = data_to_game_coord(_GP(play).speech_bubble_width);
		xpp = _GP(play).RoomToScreenX(data_to_game_coord(_GP(game).chars[chid].x)) - width / 2;
		if (xpp < 0)
			xpp = 0;
		// -1 will automatically put it above the char's head
		ypp = -1;
	}

	_displayspeech(displbuf, chid, xpp, ypp, width, 1);
}

void _displayspeech(const char *texx, int aschar, int xx, int yy, int widd, int isThought) {
	if (!is_valid_character(aschar))
		quit("!DisplaySpeech: invalid character");

	CharacterInfo *speakingChar = &_GP(game).chars[aschar];
	if ((speakingChar->view < 0) || (speakingChar->view >= _GP(game).numviews))
		quit("!DisplaySpeech: character has invalid view");

	if (_GP(play).screen_is_faded_out > 0)
		debug_script_warn("Warning: blocking Say call during fade-out.");
	if (_GP(play).text_overlay_on > 0) {
		debug_script_warn("DisplaySpeech: speech was already displayed (nested DisplaySpeech, perhaps room script and global script conflict?)");
		return;
	}

	EndSkippingUntilCharStops();

	_G(said_speech_line) = 1;

	if (_GP(play).bgspeech_stay_on_display == 0) {
		// remove any background speech
		auto &overs = get_overlays();
		for (auto &over : overs) {
			if (over.timeout > 0)
				remove_screen_overlay(over.type);
		}
	}
	_G(said_text) = 1;

	// the strings are pre-translated
	//texx = get_translation(texx);
	set_our_eip(150);

	int isPause = 1;
	// if the message is all .'s, don't display anything
	for (size_t aa = 0; texx[aa] != 0; aa++) {
		if (texx[aa] != '.') {
			isPause = 0;
			break;
		}
	}

	_GP(play).messagetime = GetTextDisplayTime(texx);
	_GP(play).speech_in_post_state = false;

	if (isPause) {
		postpone_scheduled_music_update_by(std::chrono::milliseconds(_GP(play).messagetime * 1000 / _G(frames_per_second)));
		// Set a post-state right away, as we only need to wait for a messagetime timer
		_GP(play).speech_in_post_state = true;
		GameLoopUntilValueIsNegative(&_GP(play).messagetime);
		post_display_cleanup();
		return;
	}

	int textcol = speakingChar->talkcolor;

	// if it's 0, it won't be recognised as speech
	if (textcol == 0)
		textcol = 16;

	Rect ui_view = _GP(play).GetUIViewport();
	int allowShrink = 0;
	int bwidth = widd;
	if (bwidth < 0)
		bwidth = ui_view.GetWidth() / 2 + ui_view.GetWidth() / 4;

	set_our_eip(151);

	int useview = speakingChar->talkview;
	if (isThought) {
		useview = speakingChar->thinkview;
		// view 0 is not valid for think views
		if (useview == 0)
			useview = -1;
		// speech bubble can shrink to fit
		allowShrink = 1;
		if (speakingChar->room != _G(displayed_room)) {
			// not in room, centre it
			xx = -1;
			yy = -1;
		}
	}

	if (useview >= _GP(game).numviews)
		quitprintf("!Character.Say: attempted to use view %d for animation, but it does not exist", useview + 1);

	if (_GP(game).options[OPT_SPEECHTYPE] == 3)
		remove_screen_overlay(OVER_COMPLETE);
	set_our_eip(1500);

	if (_GP(game).options[OPT_SPEECHTYPE] == 0)
		allowShrink = 1;

	// If has a valid speech view, and idle anim in progress for the character, then stop it
	if (useview >= 0 && speakingChar->idleleft < 0) {
		ReleaseCharacterView(aschar);
	}

	int tdxp = xx, tdyp = yy;
	int oldview = -1, oldloop = -1;
	int ovr_type = 0;
	_G(text_lips_offset) = 0;
	_G(text_lips_text) = texx;
	Bitmap *closeupface = nullptr;
	bool overlayPositionFixed = false;
	int charFrameWas = 0;
	int viewWasLocked = 0;
	if (speakingChar->flags & CHF_FIXVIEW)
		viewWasLocked = 1;

	// Start voice-over, if requested by the tokens in speech text
	try_auto_play_speech(texx, texx, aschar);

	if (speakingChar->room == _G(displayed_room)) {
		// If the character is in this room, go for it - otherwise
		// run the "else" clause which  does text in the middle of
		// the screen.
		set_our_eip(1501);

		if (speakingChar->walking)
			StopMoving(aschar);

		// save the frame we need to go back to
		// if they were moving, this will be 0 (because we just called
		// StopMoving); otherwise, it might be a specific animation
		// frame which we should return to
		if (viewWasLocked)
			charFrameWas = speakingChar->frame;

		if ((speakingChar->view < 0) || _GP(views)[speakingChar->view].numLoops == 0)
			quitprintf("!Character %s current view %d is invalid, or has no loops.", speakingChar->scrname, speakingChar->view + 1);
		// If current view is missing a loop - use loop 0
		if (speakingChar->loop >= _GP(views)[speakingChar->view].numLoops) {
			debug_script_warn("WARNING: Character %s current view %d does not have necessary loop %d; switching to loop 0.",
							  speakingChar->scrname, speakingChar->view + 1, speakingChar->loop);
			speakingChar->loop = 0;
		}

		set_our_eip(1504);

		// Calculate speech position based on character's position on screen
		auto view = FindNearestViewport(aschar);
		if (tdxp < 0)
			tdxp = view->RoomToScreen(data_to_game_coord(speakingChar->x), 0).first.X;
		if (tdxp < 2)
			tdxp = 2;
		tdxp = -tdxp;  // tell it to centre it ([ikm] not sure what's going on here... wrong comment?)

		if (tdyp < 0) {
			int sppic = _GP(views)[speakingChar->view].loops[speakingChar->loop].frames[0].pic;
			int height = (_GP(charextra)[aschar].height < 1) ? _GP(game).SpriteInfos[sppic].Height : _GP(charextra)[aschar].height;
			tdyp = view->RoomToScreen(0, data_to_game_coord(_GP(charextra)[aschar].GetEffectiveY(speakingChar)) - height).first.Y
			       - get_fixed_pixel_size(5);
			if (isThought) // if it's a thought, lift it a bit further up
				tdyp -= get_fixed_pixel_size(10);
		}
		if (tdyp < 5)
			tdyp = 5;

		set_our_eip(152);

		if ((useview >= 0) && (_GP(game).options[OPT_SPEECHTYPE] > 0)) {
			// Sierra-style close-up portrait

			if (_GP(play).swap_portrait_lastchar != aschar) {
				// if the portraits are set to Alternate, OR they are
				// set to Left but swap_portrait has been set to 1 (the old
				// method for enabling it), then swap them round
				if ((_GP(game).options[OPT_PORTRAITSIDE] == PORTRAIT_ALTERNATE) ||
				        ((_GP(game).options[OPT_PORTRAITSIDE] == 0) &&
				         (_GP(play).swap_portrait_side > 0))) {

					if (_GP(play).swap_portrait_side == 2)
						_GP(play).swap_portrait_side = 1;
					else
						_GP(play).swap_portrait_side = 2;
				}

				if (_GP(game).options[OPT_PORTRAITSIDE] == PORTRAIT_XPOSITION) {
					// Portrait side based on character X-positions
					if (_GP(play).swap_portrait_lastchar < 0) {
						// No previous character been spoken to
						// therefore, assume it's the player
						if (_GP(game).playercharacter != aschar && _GP(game).chars[_GP(game).playercharacter].room == speakingChar->room && _GP(game).chars[_GP(game).playercharacter].on == 1)
							_GP(play).swap_portrait_lastchar = _GP(game).playercharacter;
						else
							// The player's not here. Find another character in this room
							// that it could be
							for (int ce = 0; ce < _GP(game).numcharacters; ce++) {
								if ((_GP(game).chars[ce].room == speakingChar->room) &&
								        (_GP(game).chars[ce].on == 1) &&
								        (ce != aschar)) {
									_GP(play).swap_portrait_lastchar = ce;
									break;
								}
							}
					}

					if (_GP(play).swap_portrait_lastchar >= 0) {
						// if this character is right of the one before, put the
						// portrait on the right
						if (speakingChar->x > _GP(game).chars[_GP(play).swap_portrait_lastchar].x)
							_GP(play).swap_portrait_side = -1;
						else
							_GP(play).swap_portrait_side = 0;
					}
				}
				_GP(play).swap_portrait_lastlastchar = _GP(play).swap_portrait_lastchar;
				_GP(play).swap_portrait_lastchar = aschar;
			} else
				// If the portrait side is based on the character's X position and the same character is
				// speaking, compare against the previous *previous* character to see where the speech should be
				if (_GP(game).options[OPT_PORTRAITSIDE] == PORTRAIT_XPOSITION && _GP(play).swap_portrait_lastlastchar >= 0) {
					if (speakingChar->x > _GP(game).chars[_GP(play).swap_portrait_lastlastchar].x)
						_GP(play).swap_portrait_side = -1;
					else
						_GP(play).swap_portrait_side = 0;
				}

			// Determine whether to display the portrait on the left or right
			int portrait_on_right = 0;

			if (_GP(game).options[OPT_SPEECHTYPE] == 3) {
			}  // always on left with QFG-style speech
			else if ((_GP(play).swap_portrait_side == 1) ||
			         (_GP(play).swap_portrait_side == -1) ||
			         (_GP(game).options[OPT_PORTRAITSIDE] == PORTRAIT_RIGHT))
				portrait_on_right = 1;


			int bigx = 0, bigy = 0, kk;
			ViewStruct *viptr = &_GP(views)[useview];
			for (kk = 0; kk < viptr->loops[0].numFrames; kk++) {
				int tw = _GP(game).SpriteInfos[viptr->loops[0].frames[kk].pic].Width;
				if (tw > bigx) bigx = tw;
				tw = _GP(game).SpriteInfos[viptr->loops[0].frames[kk].pic].Height;
				if (tw > bigy) bigy = tw;
			}

			// if they accidentally used a large full-screen image as the sierra-style
			// talk view, correct it
			if ((_GP(game).options[OPT_SPEECHTYPE] != 3) && (bigx > ui_view.GetWidth() - get_fixed_pixel_size(50)))
				bigx = ui_view.GetWidth() - get_fixed_pixel_size(50);

			if (widd > 0)
				bwidth = widd - bigx;

			set_our_eip(153);
			int ovr_yp = get_fixed_pixel_size(20);
			int view_frame_x = 0;
			int view_frame_y = 0;
			_G(facetalk_qfg4_override_placement_x) = false;
			_G(facetalk_qfg4_override_placement_y) = false;

			if (_GP(game).options[OPT_SPEECHTYPE] == 3) {
				// QFG4-style whole screen picture
				closeupface = BitmapHelper::CreateBitmap(ui_view.GetWidth(), ui_view.GetHeight());
				closeupface->Clear(0);
				if (xx < 0 && _GP(play).speech_portrait_placement) {
					_G(facetalk_qfg4_override_placement_x) = true;
					view_frame_x = _GP(play).speech_portrait_x;
				}
				if (yy < 0 && _GP(play).speech_portrait_placement) {
					_G(facetalk_qfg4_override_placement_y) = true;
					view_frame_y = _GP(play).speech_portrait_y;
				} else {
					view_frame_y = ui_view.GetHeight() / 2 - _GP(game).SpriteInfos[viptr->loops[0].frames[0].pic].Height / 2;
				}
				bigx = ui_view.GetWidth() / 2 - get_fixed_pixel_size(20);
				ovr_type = OVER_COMPLETE;
				ovr_yp = 0;
				tdyp = -1;  // center vertically
			} else {
				// KQ6-style close-up face picture
				if (yy < 0 && _GP(play).speech_portrait_placement) {
					ovr_yp = _GP(play).speech_portrait_y;
				} else if (yy < 0)
					ovr_yp = adjust_y_for_guis(ovr_yp, true /* displayspeech is always blocking */);
				else
					ovr_yp = yy;

				closeupface = BitmapHelper::CreateTransparentBitmap(bigx + 1, bigy + 1);
				ovr_type = OVER_PICTURE;

				if (yy < 0)
					tdyp = ovr_yp + get_textwindow_top_border_height(_GP(play).speech_textwindow_gui);
			}
			const ViewFrame *vf = &viptr->loops[0].frames[0];
			const bool closeupface_has_alpha = (_GP(game).SpriteInfos[vf->pic].Flags & SPF_ALPHACHANNEL) != 0;
			DrawViewFrame(closeupface, vf, view_frame_x, view_frame_y);

			int overlay_x = get_fixed_pixel_size(10);
			if (xx < 0) {
				tdxp = bigx + get_textwindow_border_width(_GP(play).speech_textwindow_gui) / 2;
				if (_GP(play).speech_portrait_placement) {
					overlay_x = _GP(play).speech_portrait_x;
					tdxp += overlay_x + get_fixed_pixel_size(6);
				} else {
					tdxp += get_fixed_pixel_size(16);
				}

				int maxWidth = (ui_view.GetWidth() - tdxp) - get_fixed_pixel_size(5) -
				               get_textwindow_border_width(_GP(play).speech_textwindow_gui) / 2;

				if (bwidth > maxWidth)
					bwidth = maxWidth;
			} else {
				tdxp = xx + bigx + get_fixed_pixel_size(8);
				overlay_x = xx;
			}

			// allow the text box to be shrunk to fit the text
			allowShrink = 1;

			// if the portrait's on the right, swap it round
			if (portrait_on_right) {
				if ((xx < 0) || (widd < 0)) {
					tdxp = get_fixed_pixel_size(9);
					if (_GP(play).speech_portrait_placement) {
						overlay_x = (ui_view.GetWidth() - bigx) - _GP(play).speech_portrait_x;
						int maxWidth = overlay_x - tdxp - get_fixed_pixel_size(9) -
						               get_textwindow_border_width(_GP(play).speech_textwindow_gui) / 2;
						if (bwidth > maxWidth)
							bwidth = maxWidth;
					} else {
						overlay_x = (ui_view.GetWidth() - bigx) - get_fixed_pixel_size(5);
					}
				} else {
					overlay_x = (xx + widd - bigx) - get_fixed_pixel_size(5);
					tdxp = xx;
				}
				tdxp += get_textwindow_border_width(_GP(play).speech_textwindow_gui) / 2;
				allowShrink = 2;
			}
			if (_GP(game).options[OPT_SPEECHTYPE] == 3)
				overlay_x = 0;
			_G(face_talking) = add_screen_overlay(false, overlay_x, ovr_yp, ovr_type, closeupface, closeupface_has_alpha);
			_G(facetalkview) = useview;
			_G(facetalkloop) = 0;
			_G(facetalkframe) = 0;
			_G(facetalkwait) = viptr->loops[0].frames[0].speed + GetCharacterSpeechAnimationDelay(speakingChar);
			_G(facetalkrepeat) = (isThought) ? 0 : 1;
			_G(facetalkBlinkLoop) = 0;
			_G(facetalkAllowBlink) = 1;
			if ((isThought) && (speakingChar->flags & CHF_NOBLINKANDTHINK))
				_G(facetalkAllowBlink) = 0;
			_G(facetalkchar) = &_GP(game).chars[aschar];
			if (_G(facetalkchar)->blinktimer < 0)
				_G(facetalkchar)->blinktimer = _G(facetalkchar)->blinkinterval;
			textcol = -textcol;
			overlayPositionFixed = true;
			// Process the first portrait view frame
			const int frame_vol = _GP(charextra)[_G(facetalkchar)->index_id].GetFrameSoundVolume(_G(facetalkchar));
			CheckViewFrame(_G(facetalkview), _G(facetalkloop), _G(facetalkframe), frame_vol);
		} else if (useview >= 0) {
			// Lucasarts-style speech
			set_our_eip(154);

			oldview = speakingChar->view;
			oldloop = speakingChar->loop;

			speakingChar->set_animating(!isThought, // only repeat if speech, not thought
										true,       // always forwards
										GetCharacterSpeechAnimationDelay(speakingChar));

			speakingChar->view = useview;
			speakingChar->frame = 0;
			speakingChar->flags |= CHF_FIXVIEW;

			if ((speakingChar->view < 0) || _GP(views)[speakingChar->view].numLoops == 0)
				quitprintf("!Character %s speech view %d is invalid, or has no loops.", speakingChar->scrname, speakingChar->view + 1);
			// If speech view is missing a loop - use loop 0
			if (speakingChar->loop >= _GP(views)[speakingChar->view].numLoops) {
				debug_script_warn("WARNING: Character %s speech view %d does not have necessary loop %d; switching to loop 0.",
								  speakingChar->scrname, speakingChar->view + 1, speakingChar->loop);
				speakingChar->loop = 0;
			}

			_G(facetalkBlinkLoop) = speakingChar->loop;

			// set up the speed of the first frame
			speakingChar->wait = GetCharacterSpeechAnimationDelay(speakingChar) +
			                     _GP(views)[speakingChar->view].loops[speakingChar->loop].frames[0].speed;

			if (widd < 0) {
				bwidth = ui_view.GetWidth() / 2 + ui_view.GetWidth() / 6;
				// If they are close to the screen edge, make the text narrower
				int relx = _GP(play).RoomToScreenX(data_to_game_coord(speakingChar->x));
				if ((relx < ui_view.GetWidth() / 4) || (relx > ui_view.GetWidth() - (ui_view.GetWidth() / 4)))
					bwidth -= ui_view.GetWidth() / 5;
			}
			/*   this causes the text to bob up and down as they talk
			tdxp = OVR_AUTOPLACE;
			tdyp = aschar;*/
			if (!isThought)  // set up the lip sync if not thinking
				_G(char_speaking) = aschar;

		}
	} else
		allowShrink = 1;

	// it wants the centred position, so make it so
	if ((xx >= 0) && (tdxp < 0))
		tdxp -= widd / 2;

	// if they used DisplaySpeechAt, then use the supplied width
	if ((widd > 0) && (isThought == 0))
		allowShrink = 0;

	if (isThought)
		_G(char_thinking) = aschar;

	set_our_eip(155);
	display_main(tdxp, tdyp, bwidth, texx, DISPLAYTEXT_SPEECH, FONT_SPEECH, textcol, isThought, allowShrink, overlayPositionFixed);
	if (_G(abort_engine))
		return;

	set_our_eip(156);
	if ((_GP(play).in_conversation > 0) && (_GP(game).options[OPT_SPEECHTYPE] == 3))
		closeupface = nullptr;
	if (closeupface != nullptr)
		remove_screen_overlay(ovr_type);
	mark_screen_dirty();
	_G(face_talking) = -1;
	_G(facetalkchar) = nullptr;
	set_our_eip(157);
	if (oldview >= 0) {
		speakingChar->flags &= ~CHF_FIXVIEW;
		if (viewWasLocked)
			speakingChar->flags |= CHF_FIXVIEW;
		speakingChar->view = oldview;

		// Don't reset the loop in 2.x games
		if (_G(loaded_game_file_version) > kGameVersion_272)
			speakingChar->loop = oldloop;

		stop_character_anim(speakingChar);
		speakingChar->frame = charFrameWas;
		speakingChar->wait = 0;
		speakingChar->idleleft = speakingChar->idletime;
		// restart the idle animation straight away
		_GP(charextra)[aschar].process_idle_this_time = 1;
	}
	_G(char_speaking) = -1;
	_G(char_thinking) = -1;
	// Stop any blocking voice-over, if was started by this function
	if (_GP(play).IsBlockingVoiceSpeech())
		stop_voice_speech();
}

int get_character_currently_talking() {
	if ((_G(face_talking) >= 0) && (_G(facetalkrepeat)))
		return _G(facetalkchar)->index_id;
	else if (_G(char_speaking) >= 0)
		return _G(char_speaking);

	return -1;
}

void DisplaySpeech(const char *texx, int aschar) {
	_displayspeech(texx, aschar, -1, -1, -1, 0);
}

// Calculate which frame of the loop to use for this character of
// speech
int GetLipSyncFrame(const char *curtex, int *stroffs) {
	/*char *frameletters[MAXLIPSYNCFRAMES] =
	{"./,/ ", "A", "O", "F/V", "D/N/G/L/R", "B/P/M",
	"Y/H/K/Q/C", "I/T/E/X/th", "U/W", "S/Z/J/ch", NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};*/

	int bestfit_len = 0, bestfit = _GP(game).default_lipsync_frame;
	for (int aa = 0; aa < MAXLIPSYNCFRAMES; aa++) {
		char *tptr = _GP(game).lipSyncFrameLetters[aa];
		while (tptr[0] != 0) {
			int lenthisbit = strlen(tptr);
			if (strchr(tptr, '/'))
				lenthisbit = strchr(tptr, '/') - tptr;

			if ((ags_strnicmp(curtex, tptr, lenthisbit) == 0) && (lenthisbit > bestfit_len)) {
				bestfit = aa;
				bestfit_len = lenthisbit;
			}
			tptr += lenthisbit;
			while (tptr[0] == '/')
				tptr++;
		}
	}
	// If it's an unknown character, use the default frame
	if (bestfit_len == 0)
		bestfit_len = 1;
	*stroffs += bestfit_len;
	return bestfit;
}

int update_lip_sync(int talkview, int talkloop, int *talkframeptr) {
	int talkframe = talkframeptr[0];
	int talkwait = 0;

	// lip-sync speech
	const char *nowsaying = &_G(text_lips_text)[_G(text_lips_offset)];
	// if it's an apostraphe, skip it (we'll, I'll, etc)
	if (nowsaying[0] == '\'') {
		_G(text_lips_offset)++;
		nowsaying++;
	}

	if (_G(text_lips_offset) >= (int)strlen(_G(text_lips_text)))
		talkframe = 0;
	else {
		talkframe = GetLipSyncFrame(nowsaying, &_G(text_lips_offset));
		if (talkframe >= _GP(views)[talkview].loops[talkloop].numFrames)
			talkframe = 0;
	}

	talkwait = _G(loops_per_character) + _GP(views)[talkview].loops[talkloop].frames[talkframe].speed;

	talkframeptr[0] = talkframe;
	return talkwait;
}

void restore_characters() {
	for (int i = 0; i < _GP(game).numcharacters; ++i) {
		_GP(charextra)[i].zoom_offs = (_GP(game).options[OPT_SCALECHAROFFSETS] != 0) ? _GP(charextra)[i].zoom : 100;
	}
}

Rect GetCharacterRoomBBox(int charid, bool use_frame_0) {
	int width, height;
	const CharacterExtras &chex = _GP(charextra)[charid];
	const CharacterInfo &chin = _GP(game).chars[charid];
	int frame = use_frame_0 ? 0 : chin.frame;
	int pic = _GP(views)[chin.view].loops[chin.loop].frames[frame].pic;
	scale_sprite_size(pic, chex.zoom, &width, &height);
	return RectWH(chin.x - width / 2, chin.y - height, width, height);
}

PViewport FindNearestViewport(int charid) {
	Rect bbox = GetCharacterRoomBBox(charid, true);
	float min_dist = -1.f;
	PViewport nearest_view;
	for (int i = 0; i < _GP(play).GetRoomViewportCount(); ++i) {
		auto view = _GP(play).GetRoomViewport(i);
		if (!view->IsVisible())
			continue;
		auto cam = view->GetCamera();
		if (!cam)
			continue;
		Rect camr = cam->GetRect();
		float dist = DistanceBetween(bbox, camr);
		if (dist == 0.f)
			return view;
		if (min_dist < 0.f || dist < min_dist) {
			min_dist = dist;
			nearest_view = view;
		}
	}
	return nearest_view ? nearest_view : _GP(play).GetRoomViewport(0);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

CharacterInfo *Character_GetByName(const char *name) {
	return static_cast<CharacterInfo *>(ccGetScriptObjectAddress(name, _GP(ccDynamicCharacter).GetType()));
}

RuntimeScriptValue Sc_Character_GetByName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ(CharacterInfo, _GP(ccDynamicCharacter), Character_GetByName, const char);
}

// void | CharacterInfo *chaa, ScriptInvItem *invi, int addIndex
RuntimeScriptValue Sc_Character_AddInventory(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ_PINT(CharacterInfo, Character_AddInventory, ScriptInvItem);
}

// void | CharacterInfo *chaa, int x, int y
RuntimeScriptValue Sc_Character_AddWaypoint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(CharacterInfo, Character_AddWaypoint);
}

// void | CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction
RuntimeScriptValue Sc_Character_Animate5(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(CharacterInfo, Character_Animate5);
}

RuntimeScriptValue Sc_Character_Animate6(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT6(CharacterInfo, Character_Animate6);
}

RuntimeScriptValue Sc_Character_Animate(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT7(CharacterInfo, Character_Animate);
}

// void | CharacterInfo *chaa, int room, int x, int y
RuntimeScriptValue Sc_Character_ChangeRoom(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(CharacterInfo, Character_ChangeRoom);
}

RuntimeScriptValue Sc_Character_ChangeRoomSetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(CharacterInfo, Character_ChangeRoomSetLoop);
}

// void | CharacterInfo *chaa, int room, int newPos
RuntimeScriptValue Sc_Character_ChangeRoomAutoPosition(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(CharacterInfo, Character_ChangeRoomAutoPosition);
}

// void | CharacterInfo *chap, int vii
RuntimeScriptValue Sc_Character_ChangeView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_ChangeView);
}

// void | CharacterInfo *char1, CharacterInfo *char2, int blockingStyle
RuntimeScriptValue Sc_Character_FaceCharacter(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ_PINT(CharacterInfo, Character_FaceCharacter, CharacterInfo);
}

// void | CharacterInfo *char1, int direction, int blockingStyle
RuntimeScriptValue Sc_Character_FaceDirection(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(CharacterInfo, Character_FaceDirection);
}

// void | CharacterInfo *char1, int xx, int yy, int blockingStyle
RuntimeScriptValue Sc_Character_FaceLocation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(CharacterInfo, Character_FaceLocation);
}

// void | CharacterInfo *char1, ScriptObject *obj, int blockingStyle
RuntimeScriptValue Sc_Character_FaceObject(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ_PINT(CharacterInfo, Character_FaceObject, ScriptObject);
}

// void | CharacterInfo *chaa, CharacterInfo *tofollow, int distaway, int eagerness
RuntimeScriptValue Sc_Character_FollowCharacter(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ_PINT2(CharacterInfo, Character_FollowCharacter, CharacterInfo);
}

// int (CharacterInfo *chaa, const char *property)
RuntimeScriptValue Sc_Character_GetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(CharacterInfo, Character_GetProperty, const char);
}

// void (CharacterInfo *chaa, const char *property, char *bufer)
RuntimeScriptValue Sc_Character_GetPropertyText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ2(CharacterInfo, Character_GetPropertyText, const char, char);
}

// const char* (CharacterInfo *chaa, const char *property)
RuntimeScriptValue Sc_Character_GetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_POBJ(CharacterInfo, const char, _GP(myScriptStringImpl), Character_GetTextProperty, const char);
}

RuntimeScriptValue Sc_Character_SetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ_PINT(CharacterInfo, Character_SetProperty, const char);
}

RuntimeScriptValue Sc_Character_SetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ2(CharacterInfo, Character_SetTextProperty, const char, const char);
}

// int (CharacterInfo *chaa, ScriptInvItem *invi)
RuntimeScriptValue Sc_Character_HasInventory(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(CharacterInfo, Character_HasInventory, ScriptInvItem);
}

// int (CharacterInfo *char1, CharacterInfo *char2)
RuntimeScriptValue Sc_Character_IsCollidingWithChar(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(CharacterInfo, Character_IsCollidingWithChar, CharacterInfo);
}

// int (CharacterInfo *chin, ScriptObject *objid)
RuntimeScriptValue Sc_Character_IsCollidingWithObject(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(CharacterInfo, Character_IsCollidingWithObject, ScriptObject);
}

RuntimeScriptValue Sc_Character_IsInteractionAvailable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_PINT(CharacterInfo, Character_IsInteractionAvailable);
}

// void (CharacterInfo *chap, int vii)
RuntimeScriptValue Sc_Character_LockView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_LockView);
}

// void (CharacterInfo *chap, int vii, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewEx(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(CharacterInfo, Character_LockViewEx);
}

// void (CharacterInfo *chap, int vii, int loop, int align)
RuntimeScriptValue Sc_Character_LockViewAligned_Old(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewAligned_Old);
}

// void (CharacterInfo *chap, int vii, int loop, int align, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewAlignedEx_Old(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewAlignedEx_Old);
}

RuntimeScriptValue Sc_Character_LockViewAligned(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewAligned);
}

RuntimeScriptValue Sc_Character_LockViewAlignedEx(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewAlignedEx);
}

// void (CharacterInfo *chaa, int view, int loop, int frame)
RuntimeScriptValue Sc_Character_LockViewFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewFrame);
}

// void (CharacterInfo *chaa, int view, int loop, int frame, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewFrameEx(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewFrameEx);
}

// void (CharacterInfo *chap, int vii, int xoffs, int yoffs)
RuntimeScriptValue Sc_Character_LockViewOffset(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewOffset);
}

// void (CharacterInfo *chap, int vii, int xoffs, int yoffs, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewOffsetEx(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewOffsetEx);
}

// void (CharacterInfo *chap, ScriptInvItem *invi)
RuntimeScriptValue Sc_Character_LoseInventory(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(CharacterInfo, Character_LoseInventory, ScriptInvItem);
}

// void (CharacterInfo *chaa, int x, int y, int blocking, int direct)
RuntimeScriptValue Sc_Character_Move(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(CharacterInfo, Character_Move);
}

// void (CharacterInfo *chap)
RuntimeScriptValue Sc_Character_PlaceOnWalkableArea(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(CharacterInfo, Character_PlaceOnWalkableArea);
}

// void (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_RemoveTint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(CharacterInfo, Character_RemoveTint);
}

// void (CharacterInfo *chaa, int mood)
RuntimeScriptValue Sc_Character_RunInteraction(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_RunInteraction);
}

// void (CharacterInfo *chaa, const char *texx, ...)
RuntimeScriptValue Sc_Character_Say(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_SCRIPT_SPRINTF(Character_Say, 1);
	Character_Say((CharacterInfo *)self, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

RuntimeScriptValue Sc_Character_SayAt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_SCRIPT_SPRINTF(Character_SayAt, 4);
	Character_SayAt((CharacterInfo *)self, params[0].IValue, params[1].IValue, params[2].IValue, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

RuntimeScriptValue Sc_Character_SayBackground(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_SCRIPT_SPRINTF(Character_SayBackground, 1);
	auto *ret_obj = Character_SayBackground((CharacterInfo *)self, scsf_buffer);
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj);
}

// void (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_SetAsPlayer(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(CharacterInfo, Character_SetAsPlayer);
}

// void (CharacterInfo *chaa, int iview, int itime)
RuntimeScriptValue Sc_Character_SetIdleView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(CharacterInfo, Character_SetIdleView);
}

RuntimeScriptValue Sc_Character_GetHasExplicitLight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(CharacterInfo, Character_GetHasExplicitLight);
}

RuntimeScriptValue Sc_Character_GetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetLightLevel);
}

RuntimeScriptValue Sc_Character_SetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetLightLevel);
}

RuntimeScriptValue Sc_Character_GetTintBlue(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetTintBlue);
}

RuntimeScriptValue Sc_Character_GetTintGreen(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetTintGreen);
}

RuntimeScriptValue Sc_Character_GetTintRed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetTintRed);
}

RuntimeScriptValue Sc_Character_GetTintSaturation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetTintSaturation);
}

RuntimeScriptValue Sc_Character_GetTintLuminance(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetTintLuminance);
}

// void (CharacterInfo *chaa, int xspeed, int yspeed)
RuntimeScriptValue Sc_Character_SetSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(CharacterInfo, Character_SetSpeed);
}

// void (CharacterInfo *charp)
RuntimeScriptValue Sc_Character_StopMoving(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(CharacterInfo, Character_StopMoving);
}

// void (CharacterInfo *chaa, const char *texx, ...)
RuntimeScriptValue Sc_Character_Think(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_SCRIPT_SPRINTF(Character_Think, 1);
	Character_Think((CharacterInfo *)self, scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}

//void (CharacterInfo *chaa, int red, int green, int blue, int opacity, int luminance)
RuntimeScriptValue Sc_Character_Tint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(CharacterInfo, Character_Tint);
}

// void (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_UnlockView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(CharacterInfo, Character_UnlockView);
}

// void (CharacterInfo *chaa, int stopMoving)
RuntimeScriptValue Sc_Character_UnlockViewEx(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_UnlockViewEx);
}

// void (CharacterInfo *chaa, int x, int y, int blocking, int direct)
RuntimeScriptValue Sc_Character_Walk(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(CharacterInfo, Character_Walk);
}

// void (CharacterInfo *chaa, int xx, int yy, int blocking)
RuntimeScriptValue Sc_Character_WalkStraight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT3(CharacterInfo, Character_WalkStraight);
}

RuntimeScriptValue Sc_GetCharacterAtRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(CharacterInfo, _GP(ccDynamicCharacter), GetCharacterAtRoom);
}

// CharacterInfo *(int xx, int yy)
RuntimeScriptValue Sc_GetCharacterAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(CharacterInfo, _GP(ccDynamicCharacter), GetCharacterAtScreen);
}

// ScriptInvItem* (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetActiveInventory(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(CharacterInfo, ScriptInvItem, _GP(ccDynamicInv), Character_GetActiveInventory);
}

// void (CharacterInfo *chaa, ScriptInvItem* iit)
RuntimeScriptValue Sc_Character_SetActiveInventory(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(CharacterInfo, Character_SetActiveInventory, ScriptInvItem);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetAnimating(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetAnimating);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetAnimationSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetAnimationSpeed);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetAnimationSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetAnimationSpeed);
}

RuntimeScriptValue Sc_Character_GetAnimationVolume(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetAnimationVolume);
}

RuntimeScriptValue Sc_Character_SetAnimationVolume(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetAnimationVolume);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBaseline(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetBaseline);
}

// void (CharacterInfo *chaa, int basel)
RuntimeScriptValue Sc_Character_SetBaseline(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBaseline);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlinkInterval(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetBlinkInterval);
}

// void (CharacterInfo *chaa, int interval)
RuntimeScriptValue Sc_Character_SetBlinkInterval(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlinkInterval);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlinkView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetBlinkView);
}

// void (CharacterInfo *chaa, int vii)
RuntimeScriptValue Sc_Character_SetBlinkView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlinkView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlinkWhileThinking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetBlinkWhileThinking);
}

// void (CharacterInfo *chaa, int yesOrNo)
RuntimeScriptValue Sc_Character_SetBlinkWhileThinking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlinkWhileThinking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlockingHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetBlockingHeight);
}

// void (CharacterInfo *chaa, int hit)
RuntimeScriptValue Sc_Character_SetBlockingHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlockingHeight);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlockingWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetBlockingWidth);
}

// void (CharacterInfo *chaa, int wid)
RuntimeScriptValue Sc_Character_SetBlockingWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlockingWidth);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetClickable);
}

// void (CharacterInfo *chaa, int clik)
RuntimeScriptValue Sc_Character_SetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetClickable);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetDiagonalWalking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetDiagonalWalking);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetDiagonalWalking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetDiagonalWalking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetFrame);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetFrame);
}

RuntimeScriptValue Sc_Character_GetHasExplicitTint_Old(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetHasExplicitTint_Old);
}

RuntimeScriptValue Sc_Character_GetHasExplicitTint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetHasExplicitTint);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetID);
}

RuntimeScriptValue Sc_Character_GetScriptName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(CharacterInfo, const char, _GP(myScriptStringImpl), Character_GetScriptName);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIdleView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetIdleView);
}

// int (CharacterInfo *chaa, int index)
RuntimeScriptValue Sc_Character_GetIInventoryQuantity(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(CharacterInfo, Character_GetIInventoryQuantity);
}

// void (CharacterInfo *chaa, int index, int quant)
RuntimeScriptValue Sc_Character_SetIInventoryQuantity(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(CharacterInfo, Character_SetIInventoryQuantity);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIgnoreLighting(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetIgnoreLighting);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetIgnoreLighting(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetIgnoreLighting);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIgnoreScaling(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetIgnoreScaling);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetIgnoreScaling(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetIgnoreScaling);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIgnoreWalkbehinds(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetIgnoreWalkbehinds);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetIgnoreWalkbehinds(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetIgnoreWalkbehinds);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetLoop);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetLoop);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetManualScaling(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetManualScaling);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetMovementLinkedToAnimation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetMovementLinkedToAnimation);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetMovementLinkedToAnimation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetMovementLinkedToAnimation);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetMoving(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetMoving);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetDestinationX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetDestinationX);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetDestinationY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetDestinationY);
}

// const char* (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(CharacterInfo, const char, _GP(myScriptStringImpl), Character_GetName);
}

// void (CharacterInfo *chaa, const char *newName)
RuntimeScriptValue Sc_Character_SetName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(CharacterInfo, Character_SetName, const char);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetNormalView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetNormalView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetPreviousRoom(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetPreviousRoom);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetRoom(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetRoom);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetScaleMoveSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetScaleMoveSpeed);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetScaleMoveSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetScaleMoveSpeed);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetScaleVolume(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetScaleVolume);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetScaleVolume(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetScaleVolume);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetScaling(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetScaling);
}

// void (CharacterInfo *chaa, int zoomlevel)
RuntimeScriptValue Sc_Character_SetScaling(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetScaling);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSolid(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetSolid);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetSolid(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSolid);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeaking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetSpeaking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeakingFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetSpeakingFrame);
}

// int (CharacterInfo *cha)
RuntimeScriptValue Sc_GetCharacterSpeechAnimationDelay(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, GetCharacterSpeechAnimationDelay);
}

// void (CharacterInfo *chaa, int newDelay)
RuntimeScriptValue Sc_Character_SetSpeechAnimationDelay(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSpeechAnimationDelay);
}

RuntimeScriptValue Sc_Character_GetIdleAnimationDelay(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetIdleAnimationDelay);
}

// void (CharacterInfo *chaa, int newDelay)
RuntimeScriptValue Sc_Character_SetIdleAnimationDelay(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetIdleAnimationDelay);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeechColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetSpeechColor);
}

// void (CharacterInfo *chaa, int ncol)
RuntimeScriptValue Sc_Character_SetSpeechColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSpeechColor);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeechView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetSpeechView);
}

// void (CharacterInfo *chaa, int vii)
RuntimeScriptValue Sc_Character_SetSpeechView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSpeechView);
}

RuntimeScriptValue Sc_Character_GetThinking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(CharacterInfo, Character_GetThinking);
}

RuntimeScriptValue Sc_Character_GetThinkingFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetThinkingFrame);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetThinkView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetThinkView);
}

// void (CharacterInfo *chaa, int vii)
RuntimeScriptValue Sc_Character_SetThinkView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetThinkView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetTransparency);
}

// void (CharacterInfo *chaa, int trans)
RuntimeScriptValue Sc_Character_SetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetTransparency);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetTurnBeforeWalking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetTurnBeforeWalking);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetTurnBeforeWalking(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetTurnBeforeWalking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetWalkSpeedX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetWalkSpeedX);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetWalkSpeedY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetWalkSpeedY);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetX);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetX);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetY);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetY);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetZ(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(CharacterInfo, Character_GetZ);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetZ(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetZ);
}

//=============================================================================
//
// Exclusive variadic API implementation for Plugins
//
//=============================================================================

void ScPl_Character_Say(CharacterInfo *chaa, const char *texx, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(texx);
	Character_Say(chaa, scsf_buffer);
}

void ScPl_Character_SayAt(CharacterInfo *chaa, int x, int y, int width, const char *texx, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(texx);
	Character_SayAt(chaa, x, y, width, scsf_buffer);
}

ScriptOverlay *ScPl_Character_SayBackground(CharacterInfo *chaa, const char *texx, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(texx);
	return Character_SayBackground(chaa, scsf_buffer);
}

void ScPl_Character_Think(CharacterInfo *chaa, const char *texx, ...) {
	API_PLUGIN_SCRIPT_SPRINTF(texx);
	Character_Think(chaa, scsf_buffer);
}

void RegisterCharacterAPI(ScriptAPIVersion base_api, ScriptAPIVersion /* compat_api */) {
	ScFnRegister character_api[] = {
		{"Character::GetAtRoomXY^2", API_FN_PAIR(GetCharacterAtRoom)},
		{"Character::GetAtScreenXY^2", API_FN_PAIR(GetCharacterAtScreen)},
		{"Character::GetByName", API_FN_PAIR(Character_GetByName)},

		{"Character::AddInventory^2", API_FN_PAIR(Character_AddInventory)},
		{"Character::AddWaypoint^2", API_FN_PAIR(Character_AddWaypoint)},
		{"Character::Animate^5", API_FN_PAIR(Character_Animate5)},
		{"Character::Animate^6", API_FN_PAIR(Character_Animate6)},
		{"Character::Animate^7", API_FN_PAIR(Character_Animate)},
		{"Character::ChangeRoom^3", API_FN_PAIR(Character_ChangeRoom)},
		{"Character::ChangeRoom^4", API_FN_PAIR(Character_ChangeRoomSetLoop)},
		{"Character::ChangeRoomAutoPosition^2", API_FN_PAIR(Character_ChangeRoomAutoPosition)},
		{"Character::ChangeView^1", API_FN_PAIR(Character_ChangeView)},
		{"Character::FaceCharacter^2", API_FN_PAIR(Character_FaceCharacter)},
		{"Character::FaceDirection^2", API_FN_PAIR(Character_FaceDirection)},
		{"Character::FaceLocation^3", API_FN_PAIR(Character_FaceLocation)},
		{"Character::FaceObject^2", API_FN_PAIR(Character_FaceObject)},
		{"Character::FollowCharacter^3", API_FN_PAIR(Character_FollowCharacter)},
		{"Character::GetProperty^1", API_FN_PAIR(Character_GetProperty)},
		{"Character::GetPropertyText^2", API_FN_PAIR(Character_GetPropertyText)},
		{"Character::GetTextProperty^1", API_FN_PAIR(Character_GetTextProperty)},
		{"Character::SetProperty^2", API_FN_PAIR(Character_SetProperty)},
		{"Character::SetTextProperty^2", API_FN_PAIR(Character_SetTextProperty)},
		{"Character::HasInventory^1", API_FN_PAIR(Character_HasInventory)},
		{"Character::IsCollidingWithChar^1", API_FN_PAIR(Character_IsCollidingWithChar)},
		{"Character::IsCollidingWithObject^1", API_FN_PAIR(Character_IsCollidingWithObject)},
		{"Character::IsInteractionAvailable^1", API_FN_PAIR(Character_IsInteractionAvailable)},
		{"Character::LockView^1", API_FN_PAIR(Character_LockView)},
		{"Character::LockView^2", API_FN_PAIR(Character_LockViewEx)},
		{"Character::LockViewFrame^3", API_FN_PAIR(Character_LockViewFrame)},
		{"Character::LockViewFrame^4", API_FN_PAIR(Character_LockViewFrameEx)},
		{"Character::LockViewOffset^3", API_FN_PAIR(Character_LockViewOffset)},
		{"Character::LockViewOffset^4", API_FN_PAIR(Character_LockViewOffsetEx)},
		{"Character::LoseInventory^1", API_FN_PAIR(Character_LoseInventory)},
		{"Character::Move^4", API_FN_PAIR(Character_Move)},
		{"Character::PlaceOnWalkableArea^0", API_FN_PAIR(Character_PlaceOnWalkableArea)},
		{"Character::RemoveTint^0", API_FN_PAIR(Character_RemoveTint)},
		{"Character::RunInteraction^1", API_FN_PAIR(Character_RunInteraction)},
		{"Character::Say^101", Sc_Character_Say},
		// old non-variadic variants
		{"Character::SayAt^4", API_FN_PAIR(Character_SayAt)},
		{"Character::SayBackground^1", API_FN_PAIR(Character_SayBackground)},
		// newer variadic variants
		{"Character::SayAt^104", Sc_Character_SayAt},
		{"Character::SayBackground^101", Sc_Character_SayBackground},
		{"Character::SetAsPlayer^0", API_FN_PAIR(Character_SetAsPlayer)},
		{"Character::SetIdleView^2", API_FN_PAIR(Character_SetIdleView)},
		{"Character::SetLightLevel^1", API_FN_PAIR(Character_SetLightLevel)},
		{"Character::SetWalkSpeed^2", API_FN_PAIR(Character_SetSpeed)},
		{"Character::StopMoving^0", API_FN_PAIR(Character_StopMoving)},
		{"Character::Think^101", Sc_Character_Think},
		{"Character::Tint^5", API_FN_PAIR(Character_Tint)},
		{"Character::UnlockView^0", API_FN_PAIR(Character_UnlockView)},
		{"Character::UnlockView^1", API_FN_PAIR(Character_UnlockViewEx)},
		{"Character::Walk^4", API_FN_PAIR(Character_Walk)},
		{"Character::WalkStraight^3", API_FN_PAIR(Character_WalkStraight)},

		{"Character::get_ActiveInventory", API_FN_PAIR(Character_GetActiveInventory)},
		{"Character::set_ActiveInventory", API_FN_PAIR(Character_SetActiveInventory)},
		{"Character::get_Animating", API_FN_PAIR(Character_GetAnimating)},
		{"Character::get_AnimationSpeed", API_FN_PAIR(Character_GetAnimationSpeed)},
		{"Character::set_AnimationSpeed", API_FN_PAIR(Character_SetAnimationSpeed)},
		{"Character::get_AnimationVolume", API_FN_PAIR(Character_GetAnimationVolume)},
		{"Character::set_AnimationVolume", API_FN_PAIR(Character_SetAnimationVolume)},
		{"Character::get_Baseline", API_FN_PAIR(Character_GetBaseline)},
		{"Character::set_Baseline", API_FN_PAIR(Character_SetBaseline)},
		{"Character::get_BlinkInterval", API_FN_PAIR(Character_GetBlinkInterval)},
		{"Character::set_BlinkInterval", API_FN_PAIR(Character_SetBlinkInterval)},
		{"Character::get_BlinkView", API_FN_PAIR(Character_GetBlinkView)},
		{"Character::set_BlinkView", API_FN_PAIR(Character_SetBlinkView)},
		{"Character::get_BlinkWhileThinking", API_FN_PAIR(Character_GetBlinkWhileThinking)},
		{"Character::set_BlinkWhileThinking", API_FN_PAIR(Character_SetBlinkWhileThinking)},
		{"Character::get_BlockingHeight", API_FN_PAIR(Character_GetBlockingHeight)},
		{"Character::set_BlockingHeight", API_FN_PAIR(Character_SetBlockingHeight)},
		{"Character::get_BlockingWidth", API_FN_PAIR(Character_GetBlockingWidth)},
		{"Character::set_BlockingWidth", API_FN_PAIR(Character_SetBlockingWidth)},
		{"Character::get_Clickable", API_FN_PAIR(Character_GetClickable)},
		{"Character::set_Clickable", API_FN_PAIR(Character_SetClickable)},
		{"Character::get_DestinationX", API_FN_PAIR(Character_GetDestinationX)},
		{"Character::get_DestinationY", API_FN_PAIR(Character_GetDestinationY)},
		{"Character::get_DiagonalLoops", API_FN_PAIR(Character_GetDiagonalWalking)},
		{"Character::set_DiagonalLoops", API_FN_PAIR(Character_SetDiagonalWalking)},
		{"Character::get_Frame", API_FN_PAIR(Character_GetFrame)},
		{"Character::set_Frame", API_FN_PAIR(Character_SetFrame)},
		{"Character::get_ID", API_FN_PAIR(Character_GetID)},
		{"Character::get_IdleView", API_FN_PAIR(Character_GetIdleView)},
		{"Character::get_IdleAnimationDelay", API_FN_PAIR(Character_GetIdleAnimationDelay)},
		{"Character::set_IdleAnimationDelay", API_FN_PAIR(Character_SetIdleAnimationDelay)},
		{"Character::geti_InventoryQuantity", API_FN_PAIR(Character_GetIInventoryQuantity)},
		{"Character::seti_InventoryQuantity", API_FN_PAIR(Character_SetIInventoryQuantity)},
		{"Character::get_IgnoreLighting", API_FN_PAIR(Character_GetIgnoreLighting)},
		{"Character::set_IgnoreLighting", API_FN_PAIR(Character_SetIgnoreLighting)},
		{"Character::get_IgnoreScaling", API_FN_PAIR(Character_GetIgnoreScaling)},
		{"Character::set_IgnoreScaling", API_FN_PAIR(Character_SetIgnoreScaling)},
		{"Character::get_IgnoreWalkbehinds", API_FN_PAIR(Character_GetIgnoreWalkbehinds)},
		{"Character::set_IgnoreWalkbehinds", API_FN_PAIR(Character_SetIgnoreWalkbehinds)},
		{"Character::get_Loop", API_FN_PAIR(Character_GetLoop)},
		{"Character::set_Loop", API_FN_PAIR(Character_SetLoop)},
		{"Character::get_ManualScaling", API_FN_PAIR(Character_GetIgnoreScaling)},
		{"Character::set_ManualScaling", API_FN_PAIR(Character_SetManualScaling)},
		{"Character::get_MovementLinkedToAnimation", API_FN_PAIR(Character_GetMovementLinkedToAnimation)},
		{"Character::set_MovementLinkedToAnimation", API_FN_PAIR(Character_SetMovementLinkedToAnimation)},
		{"Character::get_Moving", API_FN_PAIR(Character_GetMoving)},
		{"Character::get_Name", API_FN_PAIR(Character_GetName)},
		{"Character::set_Name", API_FN_PAIR(Character_SetName)},
		{"Character::get_NormalView", API_FN_PAIR(Character_GetNormalView)},
		{"Character::get_PreviousRoom", API_FN_PAIR(Character_GetPreviousRoom)},
		{"Character::get_Room", API_FN_PAIR(Character_GetRoom)},
		{"Character::get_ScaleMoveSpeed", API_FN_PAIR(Character_GetScaleMoveSpeed)},
		{"Character::set_ScaleMoveSpeed", API_FN_PAIR(Character_SetScaleMoveSpeed)},
		{"Character::get_ScaleVolume", API_FN_PAIR(Character_GetScaleVolume)},
		{"Character::set_ScaleVolume", API_FN_PAIR(Character_SetScaleVolume)},
		{"Character::get_Scaling", API_FN_PAIR(Character_GetScaling)},
		{"Character::set_Scaling", API_FN_PAIR(Character_SetScaling)},
		{"Character::get_ScriptName", API_FN_PAIR(Character_GetScriptName)},
		{"Character::get_Solid", API_FN_PAIR(Character_GetSolid)},
		{"Character::set_Solid", API_FN_PAIR(Character_SetSolid)},
		{"Character::get_Speaking", API_FN_PAIR(Character_GetSpeaking)},
		{"Character::get_SpeakingFrame", API_FN_PAIR(Character_GetSpeakingFrame)},
		{"Character::get_SpeechAnimationDelay", API_FN_PAIR(GetCharacterSpeechAnimationDelay)},
		{"Character::set_SpeechAnimationDelay", API_FN_PAIR(Character_SetSpeechAnimationDelay)},
		{"Character::get_SpeechColor", API_FN_PAIR(Character_GetSpeechColor)},
		{"Character::set_SpeechColor", API_FN_PAIR(Character_SetSpeechColor)},
		{"Character::get_SpeechView", API_FN_PAIR(Character_GetSpeechView)},
		{"Character::set_SpeechView", API_FN_PAIR(Character_SetSpeechView)},
		{"Character::get_Thinking", API_FN_PAIR(Character_GetThinking)},
		{"Character::get_ThinkingFrame", API_FN_PAIR(Character_GetThinkingFrame)},
		{"Character::get_ThinkView", API_FN_PAIR(Character_GetThinkView)},
		{"Character::set_ThinkView", API_FN_PAIR(Character_SetThinkView)},
		{"Character::get_Transparency", API_FN_PAIR(Character_GetTransparency)},
		{"Character::set_Transparency", API_FN_PAIR(Character_SetTransparency)},
		{"Character::get_TurnBeforeWalking", API_FN_PAIR(Character_GetTurnBeforeWalking)},
		{"Character::set_TurnBeforeWalking", API_FN_PAIR(Character_SetTurnBeforeWalking)},
		{"Character::get_View", API_FN_PAIR(Character_GetView)},
		{"Character::get_WalkSpeedX", API_FN_PAIR(Character_GetWalkSpeedX)},
		{"Character::get_WalkSpeedY", API_FN_PAIR(Character_GetWalkSpeedY)},
		{"Character::get_X", API_FN_PAIR(Character_GetX)},
		{"Character::set_X", API_FN_PAIR(Character_SetX)},
		{"Character::get_x", API_FN_PAIR(Character_GetX)},
		{"Character::set_x", API_FN_PAIR(Character_SetX)},
		{"Character::get_Y", API_FN_PAIR(Character_GetY)},
		{"Character::set_Y", API_FN_PAIR(Character_SetY)},
		{"Character::get_y", API_FN_PAIR(Character_GetY)},
		{"Character::set_y", API_FN_PAIR(Character_SetY)},
		{"Character::get_Z", API_FN_PAIR(Character_GetZ)},
		{"Character::set_Z", API_FN_PAIR(Character_SetZ)},
		{"Character::get_z", API_FN_PAIR(Character_GetZ)},
		{"Character::set_z", API_FN_PAIR(Character_SetZ)},
		{"Character::get_HasExplicitLight", API_FN_PAIR(Character_GetHasExplicitLight)},
		{"Character::get_LightLevel", API_FN_PAIR(Character_GetLightLevel)},
		{"Character::get_TintBlue", API_FN_PAIR(Character_GetTintBlue)},
		{"Character::get_TintGreen", API_FN_PAIR(Character_GetTintGreen)},
		{"Character::get_TintRed", API_FN_PAIR(Character_GetTintRed)},
		{"Character::get_TintSaturation", API_FN_PAIR(Character_GetTintSaturation)},
		{"Character::get_TintLuminance", API_FN_PAIR(Character_GetTintLuminance)},
	};

	ccAddExternalFunctions361(character_api);

	// Few functions have to be selected based on API level
	if (base_api < kScriptAPI_v350) {
		ccAddExternalObjectFunction361("Character::LockViewAligned^3", API_FN_PAIR(Character_LockViewAligned_Old));
		ccAddExternalObjectFunction361("Character::LockViewAligned^4", API_FN_PAIR(Character_LockViewAlignedEx_Old));
	} else {
		ccAddExternalObjectFunction361("Character::LockViewAligned^3", API_FN_PAIR(Character_LockViewAligned));
		ccAddExternalObjectFunction361("Character::LockViewAligned^4", API_FN_PAIR(Character_LockViewAlignedEx));
	}

	if (base_api < kScriptAPI_v341) {
		ccAddExternalObjectFunction361("Character::get_HasExplicitTint", API_FN_PAIR(Character_GetHasExplicitTint_Old));
	} else {
		ccAddExternalObjectFunction361("Character::get_HasExplicitTint", API_FN_PAIR(Character_GetHasExplicitTint));
	}
}

} // namespace AGS3
