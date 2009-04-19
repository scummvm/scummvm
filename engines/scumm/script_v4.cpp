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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "scumm/scumm_v4.h"
#include "scumm/object.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v4, x)

void ScummEngine_v4::setupOpcodes() {
	ScummEngine_v5::setupOpcodes();

	OPCODE(0x25, o5_drawObject);
	OPCODE(0x45, o5_drawObject);
	OPCODE(0x65, o5_drawObject);
	OPCODE(0xa5, o5_drawObject);
	OPCODE(0xc5, o5_drawObject);
	OPCODE(0xe5, o5_drawObject);

	OPCODE(0x50, o4_pickupObject);
	OPCODE(0xd0, o4_pickupObject);

	OPCODE(0x5c, o4_oldRoomEffect);
	OPCODE(0xdc, o4_oldRoomEffect);

	OPCODE(0x0f, o4_ifState);
	OPCODE(0x2f, o4_ifNotState);
	OPCODE(0x4f, o4_ifState);
	OPCODE(0x6f, o4_ifNotState);
	OPCODE(0x8f, o4_ifState);
	OPCODE(0xaf, o4_ifNotState);
	OPCODE(0xcf, o4_ifState);
	OPCODE(0xef, o4_ifNotState);
}

void ScummEngine_v4::o4_ifState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	if (getState(a) != b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_v4::o4_ifNotState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	if (getState(a) == b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_v4::o4_pickupObject() {
	int obj = getVarOrDirectWord(PARAM_1);

	if (obj < 1) {
		error("pickupObjectOld received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	// Don't take an object twice
		return;

	// debug(0, "adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

void ScummEngine_v4::o4_oldRoomEffect() {
	int a;

	_opcode = fetchScriptByte();
	if ((_opcode & 0x1F) == 3) {
		a = getVarOrDirectWord(PARAM_1);

#if 1
		if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {
			// FIXME / TODO: OK the first thing to note is: at least in Zak256,
			// maybe also in other games, this opcode does a bit more. I added
			// some stubs here, but somebody with a full IDA or more knowledge
			// about this will have to fill in the gaps. At least now we know
			// that something is missing here :-)

			if (a == 4) {
				//printf("o5_oldRoomEffect ODDBALL: _opcode = 0x%x, a = 0x%x\n", _opcode, a);
				// No idea what byte_2FCCF is, but it's a globale boolean flag.
				// I only add it here as a temporary hack to make the pseudo code compile.
				// Maybe it is just there as a reentry protection guard, given
				// how it is used? It might also correspond to _screenEffectFlag.
				int byte_2FCCF = 0;

				// For now, we force a redraw of the screen background. This
				// way the Zak end credits seem to work mostly correct.
				VirtScreen *vs = &_virtscr[kMainVirtScreen];
				restoreBackground(Common::Rect(0, vs->topline, vs->w, vs->topline + vs->h));
				vs->setDirtyRange(0, vs->h);
				updateDirtyScreen(kMainVirtScreen);

				if (byte_2FCCF) {
					// Here now "sub_1C44" is called, which sets byte_2FCCF to 0 then
					// calls yet another sub (which also reads byte_2FCCF):

					byte_2FCCF = 0;
					//call sub_0BB3


					// Now sub_085C is called. This is quite simply: it sets
					// 0xF000 bytes. starting at 0x40000 to 0. No idea what that
					// buffer is, maybe a screen buffer, though. Note that
					// 0xF000 = 320*192.
					// Maybe this is also the charset mask being cleaned?

					// call sub_085C


					// And then sub_1C54 is called, which is almost identical to
					// the above sub_1C44, only it sets byte_2FCCF to 1:

					byte_2FCCF = 1;
					// call sub_0BB3

				} else {
					// Here only sub_085C is called (see comment above)

					// call sub_085C
				}
			return;
			}
#endif

		}
		if (a) {
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
	}
}

} // End of namespace Scumm
