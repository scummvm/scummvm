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

#include "access/martian/martian_tunnel.h"
#include "access/martian/martian_game.h"

namespace Access {

namespace Martian {

MartianTunnel::MartianTunnel(MartianEngine *vm) : _vm(vm), _tunnelParam_ca42(0), _tunnelParam_ca44(0), _tunnelParam_ca46(0), _tunnelMoveFlag(0) {
}

MartianTunnel::~MartianTunnel() {
}

void MartianTunnel::tunnel2() {
	_tunnelParam_ca42 = 0x226;
	_tunnelParam_ca44 = 10;
	_tunnelParam_ca46 = 0x352;
	doTunnel();
}

void MartianTunnel::tunnel4() {
	_tunnelParam_ca42 = 0xabe;
	_tunnelParam_ca44 = 10;
	_tunnelParam_ca46 = 0x41a;
	doTunnel();
}

void MartianTunnel::doTunnel() {
	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_files->loadScreen(20, 0);
	Resource *res = _vm->_files->loadFile(20, 1);
	_vm->_objectsTable[20] = new SpriteResource(_vm, res);
	if (_vm->_inventory->_inv[40]._value == ITEM_IN_INVENTORY) {
		_vm->_screen->plotImage(_vm->_objectsTable[20], 8, Common::Point(140, 10));
	}
	_vm->_events->showCursor();
	_vm->_screen->forceFadeIn();
	_crawlFrame = 0;
	_tunnelMoveFlag = 0;

	warning("***TODO***: Init all the other tunnel variables here");

	drawArrowSprites();
	drawArrowSprites2();
	_vm->_room->_function = FN_NONE;

	/*
	while (true) {
		clearWorkScreenArea();
		tunnel_17f5c();
		tunnel_1888a();
		tunnel_18985();
		_vm->_buffer2.plotImage(_vm->_objectsTable[20], _crawlFrame, Common::Point(140, 94));
		copyBufBlockToScreen();
		if (_vm->_room->_function != FN_NONE)
			break;
		do {
			tunnel_doloop_18c65();
		} while (_tunnelStopLoop_ca27 == 0);
	}
	*/
	// FIXME: Quick hack skip this part
	g_system->displayMessageOnOSD(Common::U32String("Duct tunnel section not implemented yet!"));
	_vm->_flags[0x62] = 1;
	_vm->_flags[0x55] = 1;
	_vm->_room->_function = FN_CLEAR1;
	


	delete _vm->_objectsTable[20];
	_vm->_objectsTable[20] = nullptr;
}

void MartianTunnel::drawArrowSprites() {
	int x;
	int y;
	int frame;

	_vm->_events->hideCursor();
	_vm->_screen->plotImage(_vm->_objectsTable[20], 7, Common::Point(4, 0x50));
	if (_tunnelMoveFlag == 0) {
		x = 0x11;
		y = 0x50;
		frame = 9;
	} else if (_tunnelMoveFlag == 0x40) {
		x = 0x19;
		y = 0x57;
		frame = 12;
	} else if (_tunnelMoveFlag == 0x80) {
		x = 0xe;
		y = 0x5d;
		frame = 11;
	} else {
		x = 4;
		y = 0x57;
		frame = 10;
	}
	_vm->_screen->plotImage(_vm->_objectsTable[20], frame, Common::Point(x, y));
	_vm->_events->showCursor();
}
	
void MartianTunnel::drawArrowSprites2() {
	_vm->_events->hideCursor();
	_vm->_screen->plotImage(_vm->_objectsTable[20], 14, Common::Point(16, 0x58));
	if ((3 < _tunnel_ca20) && (_tunnel_ca20 < 0xd)) {
		_vm->_screen->plotImage(_vm->_objectsTable[20], 13, Common::Point(17, 0x59));
	}
	_vm->_events->showCursor();
}

void MartianTunnel::clearWorkScreenArea() {
	_vm->_buffer2.fillRect(Common::Rect(100, 60, 220, 140), 0);
}

void MartianTunnel::copyBufBlockToScreen() {
	_vm->_screen->copyBlock(&_vm->_buffer2, Common::Rect(100, 60, 220, 140));
}

}

} // end namespace Access
