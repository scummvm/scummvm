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

#include "common/random.h"
#include "common/error.h"

#include "adl/adl_v5.h"
#include "adl/display.h"
#include "adl/graphics.h"

namespace Adl {

AdlEngine_v5::AdlEngine_v5(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine_v4(syst, gd) {
}

void AdlEngine_v5::initRoomState(RoomState &roomState) const {
	roomState.picture = 0xff;
	roomState.isFirstTime = 0xff;
}

byte AdlEngine_v5::restoreRoomState(byte room) {
	const RoomState &backup = getCurRegion().rooms[room - 1];

	if (backup.isFirstTime != 0xff) {
		getRoom(room).curPicture = getRoom(room).picture = backup.picture;

		// CHECKME: Why doesn't this just copy the flag unconditionally?
		if (backup.isFirstTime != 1) {
			getRoom(room).isFirstTime = false;
			return 0;
		}
	}

	return backup.isFirstTime;
}

AdlEngine_v5::RegionChunkType AdlEngine_v5::getRegionChunkType(const uint16 addr) const {
	switch (addr) {
	case 0x7b00:
		return kRegionChunkGlobalCmds;
	case 0x9500:
		return kRegionChunkRoomCmds;
	default:
		return AdlEngine_v4::getRegionChunkType(addr);
	}
}

void AdlEngine_v5::setupOpcodeTables() {
	AdlEngine_v4::setupOpcodeTables();

	_condOpcodes[0x0a] = opcode(&AdlEngine_v5::o_abortScript);

	_actOpcodes[0x0a] = opcode(&AdlEngine_v5::o_dummy);
	_actOpcodes[0x0b] = opcode(&AdlEngine_v5::o_setTextMode);
	_actOpcodes[0x0e] = opcode(&AdlEngine_v5::o_dummy);
	_actOpcodes[0x13] = opcode(&AdlEngine_v5::o_dummy);
}

int AdlEngine_v5::o_isNounNotInRoom(ScriptEnv &e) {
	OP_DEBUG_1("\t&& NO_SUCH_ITEMS_IN_ROOM(%s)", itemRoomStr(e.arg(1)).c_str());

	Common::List<Item>::const_iterator item;

	setVar(24, 0);

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->noun == e.getNoun()) {
			setVar(24, 1);

			if (item->room == roomArg(e.arg(1)))
				return -1;
		}

	return 1;
}

int AdlEngine_v5::o_abortScript(ScriptEnv &e) {
	OP_DEBUG_0("\t&& ABORT_SCRIPT()");

	_abortScript = true;
	setVar(2, 0);

	return -1;
}

int AdlEngine_v5::o_dummy(ScriptEnv &e) {
	OP_DEBUG_0("\tDUMMY()");

	return 0;
}

int AdlEngine_v5::o_setTextMode(ScriptEnv &e) {
	OP_DEBUG_1("\tSET_TEXT_MODE(%d)", e.arg(1));

	switch (e.arg(1)) {
	case 1:
		if (_linesPrinted != 0) {
			_display->printChar(_display->asciiToNative(' '));
			handleTextOverflow();
			_display->moveCursorTo(Common::Point(0, 23));
			_maxLines = 4;
		}
		return 1;
	case 2:
		_textMode = true;
		_display->setMode(Display::kModeText);
		_display->home();
		_maxLines = 24;
		_linesPrinted = 0;
		return 1;
	case 3:
		// We re-use the restarting flag here, to simulate a long jump
		_isRestarting = true;
		return -1;
	default:
		error("Invalid text mode %d", e.arg(1));
	}
}

int AdlEngine_v5::o_setRegionRoom(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_REGION_ROOM(%d, %d)", e.arg(1), e.arg(2));

	getCurRoom().curPicture = getCurRoom().picture;
	getCurRoom().isFirstTime = false;
	switchRegion(e.arg(1));
	_state.room = e.arg(2);
	restoreRoomState(_state.room);
	return -1;
}

int AdlEngine_v5::o_setRoomPic(ScriptEnv &e) {
	const byte isFirstTime = restoreRoomState(e.arg(1));

	// CHECKME: More peculiar isFirstTime handling (see also restoreRoomState).
	// Is this here to prevent changing the backed up flag from 1 to 0? Since
	// that could only happen if the room isFirstTime is 0 while the backed up flag
	// is 1, is this scenario even possible?
	if (isFirstTime != 0xff)
		getRoom(e.arg(1)).isFirstTime = isFirstTime;

	AdlEngine_v4::o_setRoomPic(e);
	return 2;
}

} // End of namespace Adl
