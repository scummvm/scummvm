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

#include "ags/engine/ac/global_room.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/script/script.h"
#include "ags/shared/util/math.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace Shared;

void SetAmbientTint(int red, int green, int blue, int opacity, int luminance) {
	if ((red < 0) || (green < 0) || (blue < 0) ||
	        (red > 255) || (green > 255) || (blue > 255) ||
	        (opacity < 0) || (opacity > 100) ||
	        (luminance < 0) || (luminance > 100))
		quit("!SetTint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

	debug_script_log("Set _GP(ambient) tint RGB(%d,%d,%d) %d%%", red, green, blue, opacity);

	_GP(play).rtint_enabled = opacity > 0;
	_GP(play).rtint_red = red;
	_GP(play).rtint_green = green;
	_GP(play).rtint_blue = blue;
	_GP(play).rtint_level = opacity;
	_GP(play).rtint_light = (luminance * 25) / 10;
}

void SetAmbientLightLevel(int light_level) {
	light_level = Math::Clamp(light_level, -100, 100);

	_GP(play).rtint_enabled = light_level != 0;
	_GP(play).rtint_level = 0;
	_GP(play).rtint_light = light_level;
}

void NewRoom(int nrnum) {
	if (nrnum < 0)
		quitprintf("!NewRoom: room change requested to invalid room number %d.", nrnum);

	if (_G(displayed_room) < 0) {
		// called from game_start; change the room where the game will start
		_G(playerchar)->room = nrnum;
		return;
	}


	debug_script_log("Room change requested to room %d", nrnum);
	EndSkippingUntilCharStops();

	can_run_delayed_command();

	if (_GP(play).stop_dialog_at_end != DIALOG_NONE) {
		if (_GP(play).stop_dialog_at_end == DIALOG_RUNNING)
			_GP(play).stop_dialog_at_end = DIALOG_NEWROOM + nrnum;
		else {
			quitprintf("!NewRoom: two NewRoom/RunDialog/StopDialog requests within dialog; last was called in \"%s\", line %d",
			           _GP(last_in_dialog_request_script_pos).Section.GetCStr(), _GP(last_in_dialog_request_script_pos).Line);
		}
		return;
	}

	get_script_position(_GP(last_in_dialog_request_script_pos));

	if (_G(in_leaves_screen) >= 0) {
		// NewRoom called from the Player Leaves Screen event -- just
		// change which room it will go to
		_G(in_leaves_screen) = nrnum;
	} else if (_G(in_enters_screen)) {
		setevent(EV_NEWROOM, nrnum);
		return;
	} else if (_G(in_inv_screen)) {
		_G(inv_screen_newroom) = nrnum;
		return;
	} else if ((_G(inside_script) == 0) & (_G(in_graph_script) == 0)) {
		// Compatibility: old games had a *possibly unintentional* effect:
		// if a character was walking, and a "change room" is called
		// *NOT* from a script, but by some other trigger,
		// they ended up forced to a walkable area in the next room.
		if (_G(loaded_game_file_version) < kGameVersion_300) {
			_G(new_room_placeonwalkable) = is_char_walking_ndirect(_G(playerchar));
		}

		new_room(nrnum, _G(playerchar));
		return;
	} else if (_G(inside_script)) {
		_G(curscript)->queue_action(ePSANewRoom, nrnum, "NewRoom");
		// we might be within a MoveCharacterBlocking -- the room
		// change should abort it
		if (is_char_walking_ndirect(_G(playerchar))) {
			// nasty hack - make sure it doesn't move the character
			// to a walkable area
			_G(mls)[_G(playerchar)->walking].direct = 1;
			StopMoving(_GP(game).playercharacter);
		}
	} else if (_G(in_graph_script))
		_G(gs_to_newroom) = nrnum;
}


void NewRoomEx(int nrnum, int newx, int newy) {
	Character_ChangeRoom(_G(playerchar), nrnum, newx, newy);
}

void NewRoomNPC(int charid, int nrnum, int newx, int newy) {
	if (!is_valid_character(charid))
		quit("!NewRoomNPC: invalid character");
	if (charid == _GP(game).playercharacter)
		quit("!NewRoomNPC: use NewRoomEx with the player character");

	Character_ChangeRoom(&_GP(game).chars[charid], nrnum, newx, newy);
}

void ResetRoom(int nrnum) {
	if (nrnum == _G(displayed_room))
		quit("!ResetRoom: cannot reset current room");
	if ((nrnum < 0) | (nrnum >= MAX_ROOMS))
		quit("!ResetRoom: invalid room number");

	if (isRoomStatusValid(nrnum)) {
		RoomStatus *roomstat = getRoomStatus(nrnum);
		roomstat->FreeScriptData();
		roomstat->FreeProperties();
		roomstat->beenhere = 0;
	}

	debug_script_log("Room %d reset to original state", nrnum);
}

int HasPlayerBeenInRoom(int roomnum) {
	if ((roomnum < 0) || (roomnum >= MAX_ROOMS))
		return 0;
	if (isRoomStatusValid(roomnum))
		return getRoomStatus(roomnum)->beenhere;
	else
		return 0;
}

void CallRoomScript(int value) {
	can_run_delayed_command();

	if (!_G(inside_script))
		quit("!CallRoomScript: not inside a script???");

	_GP(play).roomscript_finished = 0;
	RuntimeScriptValue rval_null;
	_G(curscript)->run_another("on_call", kScInstRoom, 1, RuntimeScriptValue().SetInt32(value), rval_null);
}

int HasBeenToRoom(int roomnum) {
	if ((roomnum < 0) || (roomnum >= MAX_ROOMS))
		quit("!HasBeenToRoom: invalid room number specified");

	if (isRoomStatusValid(roomnum))
		return getRoomStatus(roomnum)->beenhere;
	else
		return 0;
}

void GetRoomPropertyText(const char *property, char *bufer) {
	get_text_property(_GP(thisroom).Properties, _G(croom)->roomProps, property, bufer);
}

void SetBackgroundFrame(int frnum) {
	if ((frnum < -1) || (frnum != -1 && (size_t)frnum >= _GP(thisroom).BgFrameCount))
		quit("!SetBackgrondFrame: invalid frame number specified");
	if (frnum < 0) {
		_GP(play).bg_frame_locked = 0;
		return;
	}

	_GP(play).bg_frame_locked = 1;

	if (frnum == _GP(play).bg_frame) {
		// already on this frame, do nothing
		return;
	}

	_GP(play).bg_frame = frnum;
	on_background_frame_change();
}

int GetBackgroundFrame() {
	return _GP(play).bg_frame;
}

} // namespace AGS3
