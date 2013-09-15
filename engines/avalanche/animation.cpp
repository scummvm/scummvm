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
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* TRIP5	Trippancy V - the sprite animation subsystem */

#include "avalanche/avalanche.h"

#include "avalanche/animation.h"
#include "avalanche/scrolls.h"
#include "avalanche/lucerna.h"
#include "avalanche/gyro.h"
#include "avalanche/celer.h"
#include "avalanche/sequence.h"
#include "avalanche/timer.h"

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/file.h"

namespace Avalanche {

void AnimationType::init(byte spritenum, bool doCheck, Animation *anim) {
	_anim = anim;

	const int32 idshould = -1317732048;

	if (spritenum == 177)
		return; // Already running!

	Common::File inf;
	Common::String filename = Common::String::format("sprite%d.avd", spritenum);
	if (!inf.open(filename))
		error("AVALANCHE: Trip: File not found: %s", filename.c_str());

	inf.seek(177);

	int32 id = inf.readSint32LE();
	if (id != idshould) {
		//output << '\7';
		inf.close();
		return;
	}

	inf.skip(2); // Replace variable named 'soa' in the original code.

	if (!_stat._name.empty())
		_stat._name.clear();
	byte nameSize = inf.readByte();
	for (byte i = 0; i < nameSize; i++)
		_stat._name += inf.readByte();
	inf.skip(12 - nameSize);

	//inf.skip(1); // Same as above.
	byte commentSize = inf.readByte();
	for (byte i = 0; i < commentSize; i++)
		_stat._comment += inf.readByte();
	inf.skip(16 - commentSize);

	_stat._frameNum = inf.readByte();
	_info._xLength = inf.readByte();
	_info._yLength = inf.readByte();
	_stat._seq = inf.readByte();
	_info._size = inf.readUint16LE();
	_stat._fgBubbleCol = inf.readByte();
	_stat._bgBubbleCol = inf.readByte();
	_stat._acciNum = inf.readByte();

	_animCount = 0; // = 1;
	_info._xWidth = _info._xLength / 8;
	if ((_info._xLength % 8) > 0)
		_info._xWidth++;
	for (byte i = 0; i < _stat._frameNum; i++) {
		_info._sil[_animCount] = new SilType[11 * (_info._yLength + 1)];
		//getmem(sil[totalnum-1], 11 * (a.yl + 1));
		_info._mani[_animCount] = new ManiType[_info._size - 6];
		//getmem(mani[totalnum-1], a.size - 6);
		for (byte j = 0; j <= _info._yLength; j++)
			inf.read((*_info._sil[_animCount])[j], _info._xWidth);
			//blockread(inf, (*sil[totalnum-1])[fv], xw);
		inf.read(*_info._mani[_animCount], _info._size - 6);
		//blockread(inf, *mani[totalnum-1], a.size - 6);

		_animCount++;
	}
	_animCount++;

	// on;
	_x = 0;
	_y = 0;
	_quick = true;
	_visible = false;
	_speedX = 3;
	_speedY = 1;
	if (spritenum == 1)
		_anim->updateSpeed(); // Just for the lights.

	_homing = false;
	_moveX = 0;
	_moveY = 0;
	_stepNum = 0;
	_doCheck = doCheck;
	_count = 0;
	_id = spritenum;
	_vanishIfStill = false;
	_callEachStepFl = false;

	inf.close();
}

void AnimationType::original() {
	_quick = false;
	_id = 177;
}

void AnimationType::draw() {
	if ((_vanishIfStill) && (_moveX == 0) && (_moveY == 0))
		return;
	byte picnum = _facingDir * _stat._seq + _stepNum; // There'll maybe problem because of the different array indexes in Pascal (starting from 1).

	_anim->_vm->_graphics->drawSprite(_info, picnum, _x, _y);
}

void AnimationType::turn(byte whichway) {
	if (whichway == 8)
		_facingDir = Animation::kDirUp;
	else
		_facingDir = whichway;
}

void AnimationType::appear(int16 wx, int16 wy, byte wf) {
	_x = (wx / 8) * 8;
	_y = wy;
	_oldX[_anim->_vm->_gyro->_cp] = wx;
	_oldY[_anim->_vm->_gyro->_cp] = wy;
	turn(wf);
	_visible = true;
	_moveX = 0;
	_moveY = 0;
}

/**
 * Check collision
 * @remarks	Originally called 'collision_check'
 */
bool AnimationType::checkCollision() {
	for (byte i = 0; i < _anim->kSpriteNumbMax; i++) {
		AnimationType *spr = &_anim->_sprites[i];
		if (spr->_quick && (spr->_id != _id) && (_x + _info._xLength > spr->_x) && (_x < spr->_x + spr->_info._xLength) && (spr->_y == _y))
			return true;
	}

	return false;
}

void AnimationType::walk() {
	if (_visible) {
		ByteField r;
		r._x1 = (_x / 8) - 1;
		if (r._x1 == 255)
			r._x1 = 0;
		r._y1 = _y - 2;
		r._x2 = ((_x + _info._xLength) / 8) + 1;
		r._y2 = _y + _info._yLength + 2;
	}

	if (!_anim->_vm->_gyro->_doingSpriteRun) {
		_oldX[_anim->_vm->_gyro->_cp] = _x;
		_oldY[_anim->_vm->_gyro->_cp] = _y;
		if (_homing)
			homeStep();
		_x += _moveX;
		_y += _moveY;
	}

	if (_doCheck) {
		if (checkCollision()) {
			bounce();
			return;
		}

		byte tc = _anim->checkFeet(_x, _x + _info._xLength, _oldY[_anim->_vm->_gyro->_cp], _y, _info._yLength) - 1;
		// -1  is because the modified array indexes of magics[] compared to Pascal .

		if ((tc != 255) & (!_anim->_vm->_gyro->_doingSpriteRun)) {
			MagicType *magic = &_anim->_vm->_gyro->_magics[tc];
			switch (magic->_operation) {
			case Gyro::kMagicExclaim:
				bounce();
				_anim->_mustExclaim = true;
				_anim->_sayWhat = magic->_data;
				break;
			case Gyro::kMagicBounce:
				bounce();
				break;
			case Gyro::kMagicTransport:
				_anim->flipRoom(magic->_data >> 8, magic->_data & 0xff);
				break;
			case Gyro::kMagicUnfinished: {
				bounce();
				Common::String tmpStr = Common::String::format("%c%cSorry.%cThis place is not available yet!", Scrolls::kControlBell, Scrolls::kControlCenter, Scrolls::kControlRoman);
				_anim->_vm->_scrolls->displayText(tmpStr);
				}
				break;
			case Gyro::kMagicSpecial:
				_anim->callSpecial(magic->_data);
				break;
			case Gyro::kMagicOpenDoor:
				_anim->openDoor(magic->_data >> 8, magic->_data & 0xff, tc);
				break;
			}
		}
	}

	if (!_anim->_vm->_gyro->_doingSpriteRun) {
		_count++;
		if (((_moveX != 0) || (_moveY != 0)) && (_count > 1)) {
			_stepNum++;
			if (_stepNum == _stat._seq)
				_stepNum = 0;
			_count = 0;
		}
	}
}

void AnimationType::bounce() {
	_x = _oldX[_anim->_vm->_gyro->_cp];
	_y = _oldY[_anim->_vm->_gyro->_cp];
	if (_doCheck)
		_anim->stopWalking();
	else
		stopWalk();
	_anim->_vm->_gyro->_onCanDoPageSwap = false;
	_anim->_vm->_lucerna->drawDirection();
	_anim->_vm->_gyro->_onCanDoPageSwap = true;
}

int8 AnimationType::getSign(int16 val) {
	if (val > 0)
		return 1;
	else if (val < 0)
		return -1;
	else
		return 0;
}

void AnimationType::walkTo(byte pedNum) {
	PedType *curPed = &_anim->_vm->_gyro->_peds[pedNum];

	setSpeed(getSign(curPed->_x - _x) * 4, getSign(curPed->_y - _y));
	_homingX = curPed->_x - _info._xLength / 2;
	_homingY = curPed->_y - _info._yLength;
	_homing = true;
}

void AnimationType::stopHoming() {
	_homing = false;
}

void AnimationType::homeStep() {
	int16 temp;

	if ((_homingX == _x) && (_homingY == _y)) {
		// touching the target
		stopWalk();
		return;
	}
	_moveX = 0;
	_moveY = 0;
	if (_homingY != _y) {
		temp = _homingY - _y;
		if (temp > 4)
			_moveY = 4;
		else if (temp < -4)
			_moveY = -4;
		else
			_moveY = temp;
	}
	if (_homingX != _x) {
		temp = _homingX - _x;
		if (temp > 4)
			_moveX = 4;
		else if (temp < -4)
			_moveX = -4;
		else
			_moveX = temp;
	}
}

void AnimationType::setSpeed(int8 xx, int8 yy) {
	_moveX = xx;
	_moveY = yy;
	if ((_moveX == 0) && (_moveY == 0))
		return; // no movement
	if (_moveX == 0) {
		// No horz movement
		if (_moveY < 0)
			turn(Animation::kDirUp);
		else
			turn(Animation::kDirDown);
	} else {
		if (_moveX < 0)
			turn(Animation::kDirLeft);
		else
			turn(Animation::kDirRight);
	}
}

void AnimationType::stopWalk() {
	_moveX = 0;
	_moveY = 0;
	_homing = false;
}

void AnimationType::chatter() {
	_anim->_vm->_gyro->_talkX = _x + _info._xLength / 2;
	_anim->_vm->_gyro->_talkY = _y;
	_anim->_vm->_gyro->_talkFontColor = _stat._fgBubbleCol;
	_anim->_vm->_gyro->_talkBackgroundColor = _stat._bgBubbleCol;
}

void AnimationType::remove() {
	_animCount--;
	_info._xWidth = _info._xLength / 8;
	if ((_info._xLength % 8) > 0)
		_info._xWidth++;
	for (byte i = 0; i < _stat._frameNum; i++) {
		assert(_animCount > 0);
		_animCount--;
		delete[] _info._mani[_animCount];
		delete[] _info._sil[_animCount];
	}

	_quick = false;
	_id = 177;
}

Animation::Animation(AvalancheEngine *vm) {
	_vm = vm;

	_mustExclaim = false;
}

Animation::~Animation() {
	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		if (_sprites[i]._quick)
			_sprites[i].remove();
	}
}

void Animation::loadAnims() {
	for (int16 i = 0; i < kSpriteNumbMax; i++)
		_sprites[i].original();
}

byte Animation::checkFeet(int16 x1, int16 x2, int16 oy, int16 y, byte yl) {
	// if not alive then begin checkfeet:=0; exit; end;
	byte a = 0;

	//setactivepage(2);
	if (x1 < 0)
		x1 = 0;
	if (x2 > 639)
		x2 = 639;
	if (oy < y) {
		for (int16 i = x1; i <= x2; i++) {
			for (int16 j = oy + yl; j <= y + yl; j++) {
				byte c = *(byte *)_vm->_graphics->_magics.getBasePtr(i, j);
				if (c > a)
					a = c;
			}
		}
	} else {
		for (int16 i = x1; i <= x2; i++) {
			for (int16 j = y + yl; j <= oy + yl; j++) {
				byte c = *(byte *)_vm->_graphics->_magics.getBasePtr(i, j);
				if (c > a)
					a = c;
			}
		}
	}

	//setactivepage(1 - cp);
	return a;
}

byte Animation::geidaPed(byte which) {
	switch (which) {
	case 1:
		return 7;
	case 2:
	case 6:
		return 8;
	case 3:
	case 5:
		return 9;
	case 4:
		return 10;
	default:
		return 0;
	}
}

void Animation::catacombMove(byte ped) {
	int32 here;
	uint16 xy_uint16;
	byte fv;

	// XY_uint16 is cat_x+cat_y*256. Thus, every room in the
	// catacombs has a different number for it.



	xy_uint16 = _vm->_gyro->_catacombX + _vm->_gyro->_catacombY * 256;
	_vm->_gyro->_geidaSpin = 0;

	switch (xy_uint16) {
	case 1801: // Exit catacombs
		flipRoom(kRoomLustiesRoom, 4);
		_vm->_scrolls->displayText("Phew! Nice to be out of there!");
		return;
	case 1033:{ // Oubliette
		flipRoom(kRoomOubliette, 1);
		Common::String tmpStr = Common::String::format("Oh, NO!%c1%c", Scrolls::kControlRegister, Scrolls::kControlSpeechBubble);
		_vm->_scrolls->displayText(tmpStr);
		}
		return;
	case 4:
		flipRoom(kRoomGeidas, 1);
		return;
	case 2307:
		flipRoom(kRoomLusties, 5);
		_vm->_scrolls->displayText("Oh no... here we go again...");
		_vm->_gyro->_userMovesAvvy = false;
		_sprites[0]._moveY = 1;
		_sprites[0]._moveX = 0;
		return;
	}

	if (!_vm->_gyro->_enterCatacombsFromLustiesRoom)
		_vm->_lucerna->loadRoom(29);
	here = _vm->_gyro->kCatacombMap[_vm->_gyro->_catacombY - 1][_vm->_gyro->_catacombX - 1];

	switch (here & 0xf) { // West.
	case 0: // no connection (wall)
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28);
		break;
	case 0x1: // no connection (wall + shield),
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 29); // ...shield.
		break;
	case 0x2: // wall with door
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door.
		break;
	case 0x3: // wall with door and shield
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 29); // ...shield.
		break;
	case 0x4: // no connection (wall + window),
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 5);  // ...window.
		break;
	case 0x5: // wall with door and window
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 5); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicNothing; // No door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 7); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = Gyro::kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 7); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicNothing; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = Gyro::kMagicSpecial; // Straight wall.
		break;
	}

	/*  ---- */

	switch ((here & 0xf0) >> 4) { // East
	case 0: // no connection (wall)
		_vm->_gyro->_magics[4]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = Gyro::kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19);
		break;
	case 0x1: // no connection (wall + window),
		_vm->_gyro->_magics[4]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = Gyro::kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 20); // ...window.
		break;
	case 0x2: // wall with door
		_vm->_gyro->_magics[4]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = Gyro::kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); // ...door.
		break;
	case 0x3: // wall with door and window
		_vm->_gyro->_magics[4]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = Gyro::kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 20); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_gyro->_magics[4]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = Gyro::kMagicNothing; // No door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 18); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_gyro->_magics[4]._operation = Gyro::kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = Gyro::kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 18); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_gyro->_magics[4]._operation = Gyro::kMagicNothing; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicSpecial; // Straight wall.
		_vm->_gyro->_portals[6]._operation = Gyro::kMagicNothing; // Door.
		break;
	}

	/*  ---- */

	switch ((here & 0xf00) >> 8) { // South
	case 0: // No connection.
		_vm->_gyro->_magics[6]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_magics[11]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_magics[12]._operation = Gyro::kMagicBounce;
		break;
	case 0x1:
		_vm->_celer->drawBackgroundSprite(-1, -1, 22);

		if ((xy_uint16 == 2051) && (_vm->_gyro->_geidaFollows))
			_vm->_gyro->_magics[12]._operation = Gyro::kMagicExclaim;
		else
			_vm->_gyro->_magics[12]._operation = Gyro::kMagicSpecial; // Right exit south.

		_vm->_gyro->_magics[6]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_magics[11]._operation = Gyro::kMagicBounce;
		break;
	case 0x2:
		_vm->_celer->drawBackgroundSprite(-1, -1, 23);
		_vm->_gyro->_magics[6]._operation = Gyro::kMagicSpecial; // Middle exit south.
		_vm->_gyro->_magics[11]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_magics[12]._operation = Gyro::kMagicBounce;
		break;
	case 0x3:
		_vm->_celer->drawBackgroundSprite(-1, -1, 24);
		_vm->_gyro->_magics[11]._operation = Gyro::kMagicSpecial; // Left exit south.
		_vm->_gyro->_magics[6]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_magics[12]._operation = Gyro::kMagicBounce;
		break;
	}

	switch ((here & 0xf000) >> 12) { // North
	case 0: // No connection
		_vm->_gyro->_magics[0]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_portals[3]._operation = Gyro::kMagicNothing; // Door.
		break;
	// LEFT handles:
#if 0
	case 0x1:
		_vm->_celer->show_one(-1, -1, 4);
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; // { Left exit north. } { Change magic number! }
		_vm->_gyro->portals[12].op = _vm->_gyro->special; // { Door. }
		break;
#endif
	case 0x2:
		_vm->_celer->drawBackgroundSprite(-1, -1, 4);
		_vm->_gyro->_magics[0]._operation = Gyro::kMagicBounce; // Middle exit north.
		_vm->_gyro->_portals[3]._operation = Gyro::kMagicSpecial; // Door.
		break;
#if 0
	case 0x3:
		_vm->_celer->show_one(-1, -1, 4);
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; // { Right exit north. } { Change magic number! }
		_vm->_gyro->portals[12].op = _vm->_gyro->special; // { Door. }
		break;
	// RIGHT handles:
	case 0x4:
		_vm->_celer->show_one(-1, -1, 3);
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; // { Left exit north. } { Change magic number! }
		_vm->_gyro->portals[12].op = _vm->_gyro->special; // { Door. }
		break;
#endif
	case 0x5:
		_vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_gyro->_magics[0]._operation = Gyro::kMagicBounce; // Middle exit north.
		_vm->_gyro->_portals[3]._operation = Gyro::kMagicSpecial; // Door.
		break;
#if 0
	case 0x6:
		_vm->_celer->show_one(-1, -1, 3);
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; // { Right exit north. }
		_vm->_gyro->portals[12].op = _vm->_gyro->special; // { Door. }
		break;
#endif
	// ARCHWAYS:
	case 0x7:
	case 0x8:
	case 0x9: {
		_vm->_celer->drawBackgroundSprite(-1, -1, 6);

		if (((here & 0xf000) >> 12) > 0x7)
			_vm->_celer->drawBackgroundSprite(-1, -1, 31);
		if (((here & 0xf000) >> 12) == 0x9)
			_vm->_celer->drawBackgroundSprite(-1, -1, 32);

		_vm->_gyro->_magics[0]._operation = Gyro::kMagicSpecial; // Middle arch north.
		_vm->_gyro->_portals[3]._operation = Gyro::kMagicNothing; // Door.
	}
	break;
	// DECORATIONS:
	case 0xd: // No connection + WINDOW
		_vm->_gyro->_magics[0]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_portals[3]._operation = Gyro::kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 14);
		break;
	case 0xe: // No connection + TORCH
		_vm->_gyro->_magics[0]._operation = Gyro::kMagicBounce;
		_vm->_gyro->_portals[3]._operation = Gyro::kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 8);
		break;
	// Recessed door:
	case 0xf:
		_vm->_gyro->_magics[0]._operation = Gyro::kMagicNothing; // Door to Geida's room.
		_vm->_celer->drawBackgroundSprite(-1, -1, 1);
		_vm->_gyro->_portals[3]._operation = Gyro::kMagicSpecial; // Door.
		break;
	}

	switch (xy_uint16) {
	case 514:
		_vm->_celer->drawBackgroundSprite(-1, -1, 17);
		break;     // [2,2] : "Art Gallery" sign over door.
	case 264:
		_vm->_celer->drawBackgroundSprite(-1, -1, 9);
		break;      // [8,1] : "The Wrong Way!" sign.
	case 1797:
		_vm->_celer->drawBackgroundSprite(-1, -1, 2);
		break;      // [5,7] : "Ite Mingite" sign.
	case 258:
		for (fv = 0; fv <= 2; fv++) { // [2,1] : Art gallery - pictures
			_vm->_celer->drawBackgroundSprite(130 + fv * 120, 70, 15);
			_vm->_celer->drawBackgroundSprite(184 + fv * 120, 78, 16);
		}
		break;
	case 1287:
		for (fv = 10; fv <= 13; fv++)
			_vm->_celer->drawBackgroundSprite(-1, -1, fv);
		break; // [7,5] : 4 candles.
	case 776:
		_vm->_celer->drawBackgroundSprite(-1, -1, 10);
		break;     // [8,3] : 1 candle.
	case 2049:
		_vm->_celer->drawBackgroundSprite(-1, -1, 11);
		break;     // [1,8] : another candle.
	case 257:
		_vm->_celer->drawBackgroundSprite(-1, -1, 12);
		_vm->_celer->drawBackgroundSprite(-1, -1, 13);
		break; // [1,1] : the other two.
	}

	if ((_vm->_gyro->_geidaFollows) && (ped > 0)) {
		if (!_sprites[1]._quick)  // If we don't already have her...
			_sprites[1].init(5, true, this); // ...Load Geida.
		appearPed(2, geidaPed(ped) - 1);
		_sprites[1]._callEachStepFl = true;
		_sprites[1]._eachStepProc = kProcGeida;
	}
}



// This proc gets called whenever you touch a line defined as _vm->_gyro->special.
void Animation::dawnDelay() {
	_vm->_timer->addTimer(2, Timer::kProcDawnDelay, Timer::kReasonDawndelay);
}

void Animation::callSpecial(uint16 which) {
	switch (which) {
	case 1: // _vm->_gyro->special 1: Room 22: top of stairs.
		_vm->_celer->drawBackgroundSprite(-1, -1, 1);
		_vm->_gyro->_brummieStairs = 1;
		_vm->_gyro->_magics[9]._operation = Gyro::kMagicNothing;
		_vm->_timer->addTimer(10, Timer::kProcStairs, Timer::kReasonBrummieStairs);
		stopWalking();
		_vm->_gyro->_userMovesAvvy = false;
		break;
	case 2: // _vm->_gyro->special 2: Room 22: bottom of stairs.
		_vm->_gyro->_brummieStairs = 3;
		_vm->_gyro->_magics[10]._operation = Gyro::kMagicNothing;
		_vm->_gyro->_magics[11]._operation = Gyro::kMagicExclaim;
		_vm->_gyro->_magics[11]._data = 5;
		_vm->_gyro->_magics[3]._operation = Gyro::kMagicBounce; // Now works as planned!
		stopWalking();
		_vm->_scrolls->displayScrollChain('q', 26);
		_vm->_gyro->_userMovesAvvy = true;
		break;
	case 3: // _vm->_gyro->special 3: Room 71: triggers dart.
		_sprites[0].bounce(); // Must include that.

		if (!_vm->_gyro->_arrowTriggered) {
			_vm->_gyro->_arrowTriggered = true;
			appearPed(2, 3); // The dart starts at ped 4, and...
			_sprites[1].walkTo(4); // flies to ped 5 (- 1 for pascal to C conversion).
			_sprites[1]._facingDir = kDirUp; // Only face.
			// Should call some kind of Eachstep procedure which will deallocate
			// the sprite when it hits the wall, and replace it with the chunk
			// graphic of the arrow buried in the plaster. */

			// OK!
			_sprites[1]._callEachStepFl = true;
			_sprites[1]._eachStepProc = kProcArrow;
		}
		break;
	case 4: // This is the ghost room link.
		_vm->_lucerna->dusk();
		_sprites[0].turn(kDirRight); // you'll see this after we get back from bootstrap
		_vm->_timer->addTimer(1, Timer::kProcGhostRoomPhew, Timer::kReasonGhostRoomPhew);
		//_vm->_enid->backToBootstrap(3); TODO: Replace it with proper ScummVM-friendly function(s)!  Do not remove until then!
		break;
	case 5:
		if (_vm->_gyro->_friarWillTieYouUp) {
			// _vm->_gyro->special 5: Room 42: touched tree, and get tied up.
			_vm->_gyro->_magics[4]._operation = Gyro::kMagicBounce; // Boundary effect is now working again.
			_vm->_scrolls->displayScrollChain('q', 35);
			_sprites[0].remove();
			//tr[1].vanishifstill:=true;
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_scrolls->displayScrollChain('q', 36);
			_vm->_gyro->_tiedUp = true;
			_vm->_gyro->_friarWillTieYouUp = false;
			_sprites[1].walkTo(2);
			_sprites[1]._vanishIfStill = true;
			_sprites[1]._doCheck = true; // One of them must have Check_Me switched on.
			_vm->_gyro->_whereIs[Gyro::kPeopleFriarTuck - 150] = kRoomDummy; // Not here, then.
			_vm->_timer->addTimer(364, Timer::kProcHangAround, Timer::kReasonHangingAround);
		}
		break;
	case 6: // _vm->_gyro->special 6: fall down oubliette.
		_vm->_gyro->_userMovesAvvy = false;
		_sprites[0]._moveX = 3;
		_sprites[0]._moveY = 0;
		_sprites[0]._facingDir = kDirRight;
		_vm->_timer->addTimer(1, Timer::kProcFallDownOubliette, Timer::kReasonFallingDownOubliette);
		break;
	case 7: // _vm->_gyro->special 7: stop falling down oubliette.
		_sprites[0]._visible = false;
		_vm->_gyro->_magics[9]._operation = Gyro::kMagicNothing;
		stopWalking();
		_vm->_timer->loseTimer(Timer::kReasonFallingDownOubliette);
		//_vm->_lucerna->mblit(12, 80, 38, 160, 3, 0);
		//_vm->_lucerna->mblit(12, 80, 38, 160, 3, 1);
		_vm->_scrolls->displayText("Oh dear, you seem to be down the bottom of an oubliette.");
		_vm->_timer->addTimer(200, Timer::kProcMeetAvaroid, Timer::kReasonMeetingAvaroid);
		break;
	case 8:        // _vm->_gyro->special 8: leave du Lustie's room.
		if ((_vm->_gyro->_geidaFollows) && (!_vm->_gyro->_lustieIsAsleep)) {
			_vm->_scrolls->displayScrollChain('q', 63);
			_sprites[1].turn(kDirDown);
			_sprites[1].stopWalk();
			_sprites[1]._callEachStepFl = false; // Geida
			_vm->_lucerna->gameOver();
		}
		break;
	case 9: // _vm->_gyro->special 9: lose Geida to Robin Hood...
		if (!_vm->_gyro->_geidaFollows)
			return;   // DOESN'T COUNT: no Geida.
		_sprites[1]._callEachStepFl = false; // She no longer follows Avvy around.
		_sprites[1].walkTo(3); // She walks to somewhere...
		_sprites[0].remove();     // Lose Avvy.
		_vm->_gyro->_userMovesAvvy = false;
		_vm->_timer->addTimer(40, Timer::kProcRobinHoodAndGeida, Timer::kReasonRobinHoodAndGeida);
		break;
	case 10: // _vm->_gyro->special 10: transfer north in catacombs.
		if ((_vm->_gyro->_catacombX == 4) && (_vm->_gyro->_catacombY == 1)) {
			// Into Geida's room.
			if (_vm->_gyro->_objects[Gyro::kObjectKey - 1])
				_vm->_scrolls->displayScrollChain('q', 62);
			else {
				_vm->_scrolls->displayScrollChain('q', 61);
				return;
			}
		}
		_vm->_lucerna->dusk();
		_vm->_gyro->_catacombY--;
		catacombMove(4);
		if (_vm->_gyro->_room != kRoomCatacombs)
			return;
		switch ((_vm->_gyro->kCatacombMap[_vm->_gyro->_catacombY - 1][_vm->_gyro->_catacombX - 1] & 0xf00) >> 8) {
		case 0x1:
			appearPed(1, 11);
			break;
		case 0x3:
			appearPed(1, 10);
			break;
		default:
			appearPed(1, 3);
		}
		dawnDelay();
		break;
	case 11: // _vm->_gyro->special 11: transfer east in catacombs.
		_vm->_lucerna->dusk();
		_vm->_gyro->_catacombX++;
		catacombMove(1);
		if (_vm->_gyro->_room != kRoomCatacombs)
			return;
		appearPed(1, 0);
		dawnDelay();
		break;
	case 12: // _vm->_gyro->special 12: transfer south in catacombs.
		_vm->_lucerna->dusk();
		_vm->_gyro->_catacombY += 1;
		catacombMove(2);
		if (_vm->_gyro->_room != kRoomCatacombs)
			return;
		appearPed(1, 1);
		dawnDelay();
		break;
	case 13: // _vm->_gyro->special 13: transfer west in catacombs.
		_vm->_lucerna->dusk();
		_vm->_gyro->_catacombX--;
		catacombMove(3);
		if (_vm->_gyro->_room != kRoomCatacombs)
			return;
		appearPed(1, 2);
		dawnDelay();
		break;
	}
}

/**
 * Open the Door.
 * This slides the door open. The data really ought to be saved in
 * the Also file, and will be next time. However, for now, they're
 * here.
 * @remarks	Originally called 'open_the_door'
 */
void Animation::openDoor(byte whither, byte ped, byte magicnum) {
	switch (_vm->_gyro->_room) {
	case kRoomOutsideYours:
	case kRoomOutsideNottsPub:
	case kRoomOutsideDucks:
		_vm->_sequence->firstShow(1);
		_vm->_sequence->thenShow(2);
		_vm->_sequence->thenShow(3);
		break;
	case kRoomInsideCardiffCastle:
		_vm->_sequence->firstShow(1);
		_vm->_sequence->thenShow(5);
		break;
	case kRoomAvvysGarden:
	case kRoomEntranceHall:
	case kRoomInsideAbbey:
	case kRoomYourHall:
		_vm->_sequence->firstShow(1);
		_vm->_sequence->thenShow(2);
		break;
	case kRoomMusicRoom:
	case kRoomOutsideArgentPub:
		_vm->_sequence->firstShow(5);
		_vm->_sequence->thenShow(6);
		break;
	case kRoomLusties:
		switch (magicnum) {
		case 14:
			if (_vm->_gyro->_avvysInTheCupboard) {
				hideInCupboard();
				_vm->_sequence->firstShow(8);
				_vm->_sequence->thenShow(7);
				_vm->_sequence->startToClose();
				return;
			} else {
				appearPed(1, 5);
				_sprites[0]._facingDir = kDirRight; // added by TT 12/3/1995
				_vm->_sequence->firstShow(8);
				_vm->_sequence->thenShow(9);
			}
			break;
		case 12:
			_vm->_sequence->firstShow(4);
			_vm->_sequence->thenShow(5);
			_vm->_sequence->thenShow(6);
			break;
		}
		break;
	}

	_vm->_sequence->thenFlip(whither, ped);
	_vm->_sequence->startToOpen();
}

void Animation::updateSpeed() {
	// Given that you've just changed the speed in triptype._speedX, this adjusts _moveX.

	_sprites[0]._moveX = (_sprites[0]._moveX / 3) * _sprites[0]._speedX;

	//setactivepage(3);

	if (_sprites[0]._speedX == _vm->_gyro->kRun)
		_vm->_graphics->_surface.drawLine(371, 199, 373, 199, kColorYellow);
	else
		_vm->_graphics->_surface.drawLine(336, 199, 338, 199, kColorYellow);

	if (_sprites[0]._speedX == _vm->_gyro->kRun)
		_vm->_graphics->_surface.drawLine(336, 199, 338, 199, kColorLightblue);
	else
		_vm->_graphics->_surface.drawLine(371, 199, 373, 199, kColorLightblue);

	//setactivepage(1 - cp);
}

void Animation::changeDirection(byte t, byte dir) {
	switch (dir) {
	case kDirUp:
		_sprites[t].setSpeed(0, -_sprites[t]._speedY);
		break;
	case kDirDown:
		_sprites[t].setSpeed(0, _sprites[t]._speedY);
		break;
	case kDirLeft:
		_sprites[t].setSpeed(-_sprites[t]._speedX,  0);
		break;
	case kDirRight:
		_sprites[t].setSpeed(_sprites[t]._speedX,  0);
		break;
	case kDirUpLeft:
		_sprites[t].setSpeed(-_sprites[t]._speedX, -_sprites[t]._speedY);
		break;
	case kDirUpRight:
		_sprites[t].setSpeed(_sprites[t]._speedX, -_sprites[t]._speedY);
		break;
	case kDirDownLeft:
		_sprites[t].setSpeed(-_sprites[t]._speedX, _sprites[t]._speedY);
		break;
	case kDirDownRight:
		_sprites[t].setSpeed(_sprites[t]._speedX, _sprites[t]._speedY);
		break;
	}
}

void Animation::appearPed(byte sprNum, byte pedNum) {
	AnimationType *curSpr = &_sprites[sprNum - 1];
	PedType *curPed = &_vm->_gyro->_peds[pedNum];
	curSpr->appear(curPed->_x - curSpr->_info._xLength / 2, curPed->_y - curSpr->_info._yLength, curPed->_direction);
	changeDirection(sprNum - 1, curPed->_direction);
}

// Eachstep procedures:
void Animation::followAvalotY(byte tripnum) {
	if (_sprites[0]._facingDir == kDirLeft)
		return;
	if (_sprites[tripnum]._homing)
		_sprites[tripnum]._homingY = _sprites[1]._y;
	else {
		if (_sprites[tripnum]._y < _sprites[1]._y)
			_sprites[tripnum]._y += 1;
		else if (_sprites[tripnum]._y > _sprites[1]._y)
			_sprites[tripnum]._y -= 1;
		else
			return;
		if (_sprites[tripnum]._moveX == 0)  {
			_sprites[tripnum]._stepNum += 1;
			if (_sprites[tripnum]._stepNum == _sprites[tripnum]._stat._seq)
				_sprites[tripnum]._stepNum = 0;
			_sprites[tripnum]._count = 0;
		}
	}
}

void Animation::backAndForth(byte tripnum) {
	if (!_sprites[tripnum]._homing) {
		if (_sprites[tripnum]._facingDir == kDirRight)
			_sprites[tripnum].walkTo(3);
		else
			_sprites[tripnum].walkTo(4);
	}
}

void Animation::faceAvvy(byte tripnum) {
	if (!_sprites[tripnum]._homing) {
		if (_sprites[0]._x >= _sprites[tripnum]._x)
			_sprites[tripnum]._facingDir = kDirRight;
		else
			_sprites[tripnum]._facingDir = kDirLeft;
	}
}

void Animation::arrowProcs(byte tripnum) {
	if (_sprites[tripnum]._homing) {
		// Arrow is still in flight.
		// We must check whether or not the arrow has collided tr[tripnum] Avvy's head.
		// This is so if: a) the bottom of the arrow is below Avvy's head,
		// b) the left of the arrow is left of the right of Avvy's head, and
		// c) the right of the arrow is right of the left of Avvy's head.
		if (((_sprites[tripnum]._y + _sprites[tripnum]._info._yLength) >= _sprites[0]._y) // A
				&& (_sprites[tripnum]._x <= (_sprites[0]._x + _sprites[0]._info._xLength)) // B
				&& ((_sprites[tripnum]._x + _sprites[tripnum]._info._xLength) >= _sprites[0]._x)) { // C
			// OK, it's hit him... what now?

			_sprites[1]._callEachStepFl = false; // prevent recursion.
			_vm->_scrolls->displayScrollChain('Q', 47); // Complaint!
			_sprites[tripnum].remove(); // Deallocate the arrow.
#if 0
			tr[1].done; { Deallocate normal pic of Avvy. }

			off;
			for byte fv:=0 to 1 do
			begin
			cp:=1-cp;
			getback;
			end;
			on;
#endif
			_vm->_lucerna->gameOver();

			_vm->_gyro->_userMovesAvvy = false; // Stop the user from moving him.
			_vm->_timer->addTimer(55, Timer::kProcNaughtyDuke, Timer::kReasonNaughtyDuke);
		}
	} else { // Arrow has hit the wall!
		_sprites[tripnum].remove(); // Deallocate the arrow.
		_vm->_celer->drawBackgroundSprite(-1, -1, 3); // Show pic of arrow stuck into the door.
		_vm->_gyro->_arrowInTheDoor = true; // So that we can pick it up.
	}

}

#if 0
procedure Spludwick_procs(tripnum:byte);
var fv:byte;
begin
	with tr[tripnum] do
	if not homing then { We only need to do anything if Spludwick *stops*
						walking. }
	with _vm->_gyro->dna do
	begin
	inc(DogfoodPos);
	if DogfoodPos=8 then DogfoodPos:=1;
	walkto(DogfoodPos);
	end;
end;
#endif

void Animation::grabAvvy(byte tripnum) {     // For Friar Tuck, in Nottingham.
	int16 tox = _sprites[0]._x + 17;
	int16 toy = _sprites[0]._y - 1;
	if ((_sprites[tripnum]._x == tox) && (_sprites[tripnum]._y == toy)) {
		_sprites[tripnum]._callEachStepFl = false;
		_sprites[tripnum]._facingDir = kDirLeft;
		_sprites[tripnum].stopWalk();
		// ... whatever ...
	} else {
		// Still some way to go.
		if (_sprites[tripnum]._x < tox) {
			_sprites[tripnum]._x += 5;
			if (_sprites[tripnum]._x > tox)
				_sprites[tripnum]._x = tox;
		}
		if (_sprites[tripnum]._y < toy)
			_sprites[tripnum]._y++;
		_sprites[tripnum]._stepNum++;
		if (_sprites[tripnum]._stepNum == _sprites[tripnum]._stat._seq)
			_sprites[tripnum]._stepNum = 0;
	}
}

void Animation::takeAStep(byte &tripnum) {
	if (_sprites[tripnum]._moveX == 0) {
		_sprites[tripnum]._stepNum++;
		if (_sprites[tripnum]._stepNum == _sprites[tripnum]._stat._seq)
			_sprites[tripnum]._stepNum = 0;
		_sprites[tripnum]._count = 0;
	}
}

void Animation::spin(byte whichway, byte &tripnum) {
	if (_sprites[tripnum]._facingDir != whichway) {
		_sprites[tripnum]._facingDir = whichway;
		if (_sprites[tripnum]._id == 2)
			return; // Not for Spludwick

		_vm->_gyro->_geidaSpin += 1;
		_vm->_gyro->_geidaTime = 20;
		if (_vm->_gyro->_geidaSpin == 5) {
			_vm->_scrolls->displayText("Steady on, Avvy, you'll make the poor girl dizzy!");
			_vm->_gyro->_geidaSpin = 0;
			_vm->_gyro->_geidaTime = 0; // knock out records
		}
	}
}

void Animation::geidaProcs(byte tripnum) {
	if (_vm->_gyro->_geidaTime > 0) {
		_vm->_gyro->_geidaTime--;
		if (_vm->_gyro->_geidaTime == 0)
			_vm->_gyro->_geidaSpin = 0;
	}

	if (_sprites[tripnum]._y < (_sprites[0]._y - 2)) {
		// Geida is further from the screen than Avvy.
		spin(kDirDown, tripnum);
		_sprites[tripnum]._moveY = 1;
		_sprites[tripnum]._moveX = 0;
		takeAStep(tripnum);
		return;
	} else if (_sprites[tripnum]._y > (_sprites[0]._y + 2)) {
		// Avvy is further from the screen than Geida.
		spin(kDirUp, tripnum);
		_sprites[tripnum]._moveY = -1;
		_sprites[tripnum]._moveX = 0;
		takeAStep(tripnum);
		return;
	}

	_sprites[tripnum]._moveY = 0;
	// These 12-s are not in the original, I added them to make the following method more "smooth".
	// Now the NPC which is following Avvy won't block his way and will walk next to him properly.
	if (_sprites[tripnum]._x < _sprites[0]._x - _sprites[0]._speedX * 8 - 12) {
		_sprites[tripnum]._moveX = _sprites[0]._speedX;
		spin(kDirRight, tripnum);
		takeAStep(tripnum);
	} else if (_sprites[tripnum]._x > _sprites[0]._x + _sprites[0]._speedX * 8 + 12) {
		_sprites[tripnum]._moveX = -_sprites[0]._speedX;
		spin(kDirLeft, tripnum);
		takeAStep(tripnum);
	} else
		_sprites[tripnum]._moveX = 0;
}

// That's all...

void Animation::drawSprites() {
	int8 order[5];
	byte temp;
	bool ok;

	for (int i = 0; i < 5; i++)
		order[i] = -1;

	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		if (_sprites[i]._quick && _sprites[i]._visible)
			order[i] = i;
	}

	do {
		ok = true;
		for (byte i = 0; i < 4; i++) {
			if (((order[i] != -1) && (order[i + 1] != -1))
					&& (_sprites[order[i]]._y > _sprites[order[i + 1]]._y)) {
				// Swap them!
				temp = order[i];
				order[i] = order[i + 1];
				order[i + 1] = temp;
				ok = false;
			}
		}
	} while (!ok);


	_vm->_graphics->refreshBackground();

	for (byte i = 0; i < 5; i++) {
		if (order[i] > -1)
			_sprites[order[i]].draw();
	}
}

/**
 * Animation links
 * @remarks	Originally called 'trippancy_link'
 */
void Animation::animLink() {
	if (_vm->_gyro->_dropdownActive | _vm->_gyro->_onToolbar | _vm->_gyro->_seeScroll)
		return;
	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		if (_sprites[i]._quick && _sprites[i]._visible)
			_sprites[i].walk();
	}

	drawSprites();

	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		if (_sprites[i]._quick && _sprites[i]._callEachStepFl) {
			switch (_sprites[i]._eachStepProc) {
			case kProcFollowAvvyY :
				followAvalotY(i);
				break;
			case kProcBackAndForth :
				backAndForth(i);
				break;
			case kProcFaceAvvy :
				faceAvvy(i);
				break;
			case kProcArrow :
				arrowProcs(i);
				break;
				//    PROCSpludwick_procs : spludwick_procs(fv);
			case kProcGrabAvvy :
				grabAvvy(i);
				break;
			case kProcGeida :
				geidaProcs(i);
				break;
			}
		}
	}

	if (_mustExclaim) {
		_mustExclaim = false;
		_vm->_scrolls->displayScrollChain('x', _sayWhat);
	}
}

void Animation::stopWalking() {
	_sprites[0].stopWalk();
	_direction = kDirStopped;
	if (_vm->_gyro->_alive)
		_sprites[0]._stepNum = 1;
}

/**
 * Hide in the cupboard
 * @remarks	Originally called 'hide_in_the_cupboard'
 */
void Animation::hideInCupboard() {
	if (_vm->_gyro->_avvysInTheCupboard) {
		if (_vm->_gyro->_wearing == Acci::kNothing) {
			Common::String tmpStr = Common::String::format("%cAVVY!%cGet dressed first!", Scrolls::kControlItalic, Scrolls::kControlRoman);
			_vm->_scrolls->displayText(tmpStr);
		} else {
			_sprites[0]._visible = true;
			_vm->_gyro->_userMovesAvvy = true;
			appearPed(1, 2); // Walk out of the cupboard.
			_vm->_scrolls->displayText("You leave the cupboard. Nice to be out of there!");
			_vm->_gyro->_avvysInTheCupboard = false;
			_vm->_sequence->firstShow(8);
			_vm->_sequence->thenShow(7);
			_vm->_sequence->startToClose();
		}
	} else {
		// Not hiding in the cupboard
		_sprites[0]._visible = false;
		_vm->_gyro->_userMovesAvvy = false;
		Common::String tmpStr = Common::String::format("You walk into the room...%cIt seems to be an empty, " \
			"but dusty, cupboard. Hmmmm... you leave the door slightly open to avoid suffocation.", Scrolls::kControlParagraph);
		_vm->_scrolls->displayText(tmpStr);
		_vm->_gyro->_avvysInTheCupboard = true;
		_vm->_celer->drawBackgroundSprite(-1, -1, 8);
	}
}

void Animation::flipRoom(byte room, byte ped) {
	if (!_vm->_gyro->_alive) {
		// You can't leave the room if you're dead.
		_sprites[0]._moveX = 0;
		_sprites[0]._moveY = 0; // Stop him from moving.
		return;
	}

	if ((room == kRoomDummy) && (_vm->_gyro->_room == kRoomLusties)) {
		hideInCupboard();
		return;
	}

	if ((_vm->_gyro->_jumpStatus > 0) && (_vm->_gyro->_room == kRoomInsideCardiffCastle)) {
		// You can't *jump* out of Cardiff Castle!
		_sprites[0]._moveX = 0;
		return;
	}

	_vm->_lucerna->exitRoom(_vm->_gyro->_room);
	_vm->_lucerna->dusk();

	for (int16 i = 1; i < kSpriteNumbMax; i++) {
		if (_sprites[i]._quick)
			_sprites[i].remove();
	} // Deallocate sprite

	if (_vm->_gyro->_room == kRoomLustiesRoom)
		_vm->_gyro->_enterCatacombsFromLustiesRoom = true;

	_vm->_lucerna->enterRoom(room, ped);
	appearPed(1, ped - 1);
	_vm->_gyro->_enterCatacombsFromLustiesRoom = false;
	_oldDirection = _direction;
	_direction = _sprites[0]._facingDir;
	_vm->_lucerna->drawDirection();

	_vm->_lucerna->dawn();

	// Tidy up after mouse. I know it's a kludge...
	//  tidy_after_mouse;
}

bool Animation::inField(byte which) {
	FieldType *curField = &_vm->_gyro->_fields[which - 1]; // Pascal -> C: different array indexes.
	int16 yy = _sprites[0]._y + _sprites[0]._info._yLength;

	return (_sprites[0]._x >= curField->_x1) && (_sprites[0]._x <= curField->_x2) && (yy >= curField->_y1) && (yy <= curField->_y2);
}

bool Animation::nearDoor() {
	if (_vm->_gyro->_fieldNum < 8) {
		// there ARE no doors here!
		return false;
	}

	int16 ux = _sprites[0]._x;
	int16 uy = _sprites[0]._y + _sprites[0]._info._yLength;

	for (byte i = 8; i < _vm->_gyro->_fieldNum; i++) {
		FieldType *curField = &_vm->_gyro->_fields[i];
		if ((ux >= curField->_x1) && (ux <= curField->_x2) && (uy >= curField->_y1) && (uy <= curField->_y2))
			return true;
	}

	return false;
}

void Animation::handleMoveKey(const Common::Event &event) {
	if (!_vm->_gyro->_userMovesAvvy)
		return;

	if (_vm->_dropdown->_activeMenuItem._activeNow)
		_vm->_parser->tryDropdown();
	else {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
			if (_direction != kDirUp) {
				_direction = kDirUp;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_DOWN:
			if (_direction != kDirDown) {
				_direction = kDirDown;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_LEFT:
			if (_direction != kDirLeft) {
				_direction = kDirLeft;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_RIGHT:
			if (_direction != kDirRight) {
				_direction = kDirRight;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_PAGEUP:
			if (_direction != kDirUpRight) {
				_direction = kDirUpRight;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_PAGEDOWN:
			if (_direction != kDirDownRight) {
				_direction = kDirDownRight;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_END:
			if (_direction != kDirDownLeft) {
				_direction = kDirDownLeft;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_HOME:
			if (_direction != kDirUpLeft) {
				_direction = kDirUpLeft;
				changeDirection(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_KP5:
			stopWalking();
			break;
		default:
			break;
		}
	}
}


} // End of namespace Avalanche.
