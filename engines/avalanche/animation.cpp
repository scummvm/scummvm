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
#include "avalanche/dialogs.h"
#include "avalanche/background.h"
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
		inf.close();
		return;
	}

	// Replace variable named 'soa' in the original code.
	inf.skip(2);

	if (!_stat._name.empty())
		_stat._name.clear();
	byte nameSize = inf.readByte();
	for (int i = 0; i < nameSize; i++)
		_stat._name += inf.readByte();
	inf.skip(12 - nameSize);

	byte commentSize = inf.readByte();
	for (int i = 0; i < commentSize; i++)
		_stat._comment += inf.readByte();
	inf.skip(16 - commentSize);

	_stat._frameNum = inf.readByte();
	_info._xLength = inf.readByte();
	_info._yLength = inf.readByte();
	_stat._seq = inf.readByte();
	_info._size = inf.readUint16LE();
	_stat._fgBubbleCol = (Color)inf.readByte();
	_stat._bgBubbleCol = (Color)inf.readByte();
	_stat._acciNum = inf.readByte();

	_animCount = 0; // = 1;
	_info._xWidth = _info._xLength / 8;
	if ((_info._xLength % 8) > 0)
		_info._xWidth++;
	for (int i = 0; i < _stat._frameNum; i++) {
		_info._sil[_animCount] = new SilType[11 * (_info._yLength + 1)];
		_info._mani[_animCount] = new ManiType[_info._size - 6];
		for (int j = 0; j <= _info._yLength; j++)
			inf.read((*_info._sil[_animCount])[j], _info._xWidth);
		inf.read(*_info._mani[_animCount], _info._size - 6);

		_animCount++;
	}
	_animCount++;

	CursorMan.showMouse(true);
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

void AnimationType::reset() {
	_quick = false;
	_id = 177;
}

void AnimationType::draw() {
	if ((_vanishIfStill) && (_moveX == 0) && (_moveY == 0))
		return;

	byte picnum = _facingDir * _stat._seq + _stepNum;

	_anim->_vm->_graphics->drawSprite(_info, picnum, _x, _y);
}

void AnimationType::turn(Direction whichway) {
	if (whichway == 8)
		_facingDir = kDirUp;
	else
		_facingDir = whichway;
}

void AnimationType::appear(int16 wx, int16 wy, Direction wf) {
	_x = (wx / 8) * 8;
	_y = wy;
	_oldX[_anim->_vm->_cp] = wx;
	_oldY[_anim->_vm->_cp] = wy;
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
	for (int i = 0; i < _anim->kSpriteNumbMax; i++) {
		AnimationType *spr = &_anim->_sprites[i];
		if (spr->_quick && (spr->_id != _id) && (_x + _info._xLength > spr->_x) && (_x < spr->_x + spr->_info._xLength) && (spr->_y == _y))
			return true;
	}

	return false;
}

void AnimationType::walk() {
	if (!_anim->_vm->_doingSpriteRun) {
		_oldX[_anim->_vm->_cp] = _x;
		_oldY[_anim->_vm->_cp] = _y;
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

		byte magicColor = _anim->checkFeet(_x, _x + _info._xLength, _oldY[_anim->_vm->_cp], _y, _info._yLength) - 1;
		// -1  is because the modified array indexes of magics[] compared to Pascal .

		if ((magicColor != 255) & (!_anim->_vm->_doingSpriteRun)) {
			MagicType *magic = &_anim->_vm->_magics[magicColor];
			switch (magic->_operation) {
			case AvalancheEngine::kMagicExclaim:
				bounce();
				_anim->_mustExclaim = true;
				_anim->_sayWhat = magic->_data;
				break;
			case AvalancheEngine::kMagicBounce:
				bounce();
				break;
			case AvalancheEngine::kMagicTransport:
				_anim->_vm->flipRoom((Room)(magic->_data >> 8), magic->_data & 0xff);
				break;
			case AvalancheEngine::kMagicUnfinished: {
				bounce();
				Common::String tmpStr = Common::String::format("%c%cSorry.%cThis place is not available yet!", Dialogs::kControlBell, Dialogs::kControlCenter, Dialogs::kControlRoman);
				_anim->_vm->_dialogs->displayText(tmpStr);
				}
				break;
			case AvalancheEngine::kMagicSpecial:
				_anim->callSpecial(magic->_data);
				break;
			case AvalancheEngine::kMagicOpenDoor:
				_anim->_vm->openDoor((Room)(magic->_data >> 8), magic->_data & 0xff, magicColor);
				break;
			}
		}
	}

	if (!_anim->_vm->_doingSpriteRun) {
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
	_x = _oldX[_anim->_vm->_cp];
	_y = _oldY[_anim->_vm->_cp];
	if (_doCheck)
		_anim->stopWalking();
	else
		stopWalk();
	_anim->_vm->drawDirection();
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
	PedType *curPed = &_anim->_vm->_peds[pedNum];

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
			turn(kDirUp);
		else
			turn(kDirDown);
	} else {
		if (_moveX < 0)
			turn(kDirLeft);
		else
			turn(kDirRight);
	}
}

void AnimationType::stopWalk() {
	_moveX = 0;
	_moveY = 0;
	_homing = false;
}

void AnimationType::chatter() {
	_anim->_vm->_talkX = _x + _info._xLength / 2;
	_anim->_vm->_talkY = _y;
	_anim->_vm->_graphics->setDialogColor(_stat._bgBubbleCol, _stat._fgBubbleCol);
}

void AnimationType::remove() {
	_animCount--;
	_info._xWidth = _info._xLength / 8;
	if ((_info._xLength % 8) > 0)
		_info._xWidth++;
	for (int i = 0; i < _stat._frameNum; i++) {
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

void Animation::resetAnims() {
	setDirection(kDirStopped);
	for (int16 i = 0; i < kSpriteNumbMax; i++)
		_sprites[i].reset();
}

byte Animation::checkFeet(int16 x1, int16 x2, int16 oy, int16 y, byte yl) {
	if (!_vm->_alive)
		return 0;

	if (x1 < 0)
		x1 = 0;
	if (x2 > 639)
		x2 = 639;

	int16 minY = MIN(oy, y) + yl;
	int16 maxY = MAX(oy, y) + yl;

	return _vm->_graphics->getAlsoColor(x1, minY, x2, maxY);
}

byte Animation::geidaPed(byte ped) {
	switch (ped) {
	case 1:
		return 6;
	case 2:
	case 6:
		return 7;
	case 3:
	case 5:
		return 8;
	case 4:
		return 9;
	default:
		error("geidaPed(): Unhandled ped value %d", ped);
	}
}

void Animation::catacombMove(byte ped) {
	int32 here;
	uint16 xy_uint16;

	// XY_uint16 is cat_x+cat_y*256. Thus, every room in the
	// catacombs has a different number for it.
	xy_uint16 = _vm->_catacombX + _vm->_catacombY * 256;
	_geidaSpin = 0;

	switch (xy_uint16) {
	case 1801: // Exit catacombs
		_vm->flipRoom(kRoomLustiesRoom, 4);
		_vm->_dialogs->displayText("Phew! Nice to be out of there!");
		return;
	case 1033:{ // Oubliette
		_vm->flipRoom(kRoomOubliette, 1);
		Common::String tmpStr = Common::String::format("Oh, NO!%c1%c", Dialogs::kControlRegister, Dialogs::kControlSpeechBubble);
		_vm->_dialogs->displayText(tmpStr);
		}
		return;
	case 4:
		_vm->flipRoom(kRoomGeidas, 1);
		return;
	case 2307:
		_vm->flipRoom(kRoomLusties, 5);
		_vm->_dialogs->displayText("Oh no... here we go again...");
		_vm->_userMovesAvvy = false;
		_sprites[0]._moveY = 1;
		_sprites[0]._moveX = 0;
		return;
	}

	if (!_vm->_enterCatacombsFromLustiesRoom)
		_vm->loadRoom(29);
	here = _vm->kCatacombMap[_vm->_catacombY - 1][_vm->_catacombX - 1];

	switch (here & 0xf) { // West.
	case 0: // no connection (wall)
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 27);
		break;
	case 0x1: // no connection (wall + shield),
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 28); // ...shield.
		break;
	case 0x2: // wall with door
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door.
		break;
	case 0x3: // wall with door and shield
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door, and...
		_vm->_background->draw(-1, -1, 28); // ...shield.
		break;
	case 0x4: // no connection (wall + window),
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 4);  // ...window.
		break;
	case 0x5: // wall with door and window
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door, and...
		_vm->_background->draw(-1, -1, 4); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicNothing; // No door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 6); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_magics[1]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = AvalancheEngine::kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door, and...
		_vm->_background->draw(-1, -1, 6); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_magics[1]._operation = AvalancheEngine::kMagicNothing; // Sloping wall.
		_vm->_magics[2]._operation = AvalancheEngine::kMagicSpecial; // Straight wall.
		break;
	}

	/*  ---- */

	switch ((here & 0xf0) >> 4) { // East
	case 0: // no connection (wall)
		_vm->_magics[4]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = AvalancheEngine::kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 18);
		break;
	case 0x1: // no connection (wall + window),
		_vm->_magics[4]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = AvalancheEngine::kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 19); // ...window.
		break;
	case 0x2: // wall with door
		_vm->_magics[4]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = AvalancheEngine::kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 20); // ...door.
		break;
	case 0x3: // wall with door and window
		_vm->_magics[4]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = AvalancheEngine::kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 19); // ...door, and...
		_vm->_background->draw(-1, -1, 20); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_magics[4]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = AvalancheEngine::kMagicNothing; // No door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 17); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_magics[4]._operation = AvalancheEngine::kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = AvalancheEngine::kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = AvalancheEngine::kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 20); // ...door, and...
		_vm->_background->draw(-1, -1, 17); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_magics[4]._operation = AvalancheEngine::kMagicNothing; // Sloping wall.
		_vm->_magics[5]._operation = AvalancheEngine::kMagicSpecial; // Straight wall.
		_vm->_portals[6]._operation = AvalancheEngine::kMagicNothing; // Door.
		break;
	}

	switch ((here & 0xf00) >> 8) { // South
	case 0: // No connection.
		_vm->_magics[6]._operation = AvalancheEngine::kMagicBounce;
		_vm->_magics[11]._operation = AvalancheEngine::kMagicBounce;
		_vm->_magics[12]._operation = AvalancheEngine::kMagicBounce;
		break;
	case 0x1:
		_vm->_background->draw(-1, -1, 21);

		if ((xy_uint16 == 2051) && (_vm->_geidaFollows))
			_vm->_magics[12]._operation = AvalancheEngine::kMagicExclaim;
		else
			_vm->_magics[12]._operation = AvalancheEngine::kMagicSpecial; // Right exit south.

		_vm->_magics[6]._operation = AvalancheEngine::kMagicBounce;
		_vm->_magics[11]._operation = AvalancheEngine::kMagicBounce;
		break;
	case 0x2:
		_vm->_background->draw(-1, -1, 22);
		_vm->_magics[6]._operation = AvalancheEngine::kMagicSpecial; // Middle exit south.
		_vm->_magics[11]._operation = AvalancheEngine::kMagicBounce;
		_vm->_magics[12]._operation = AvalancheEngine::kMagicBounce;
		break;
	case 0x3:
		_vm->_background->draw(-1, -1, 23);
		_vm->_magics[11]._operation = AvalancheEngine::kMagicSpecial; // Left exit south.
		_vm->_magics[6]._operation = AvalancheEngine::kMagicBounce;
		_vm->_magics[12]._operation = AvalancheEngine::kMagicBounce;
		break;
	}

	switch ((here & 0xf000) >> 12) { // North
	case 0: // No connection
		_vm->_magics[0]._operation = AvalancheEngine::kMagicBounce;
		_vm->_portals[3]._operation = AvalancheEngine::kMagicNothing; // Door.
		break;
	// LEFT handles:
#if 0
	case 0x1:
		_vm->_celer->show_one(-1, -1, 4);
		_vm->magics[1].op = _vm->bounces; // { Left exit north. } { Change magic number! }
		_vm->portals[12].op = _vm->special; // { Door. }
		break;
#endif
	case 0x2:
		_vm->_background->draw(-1, -1, 3);
		_vm->_magics[0]._operation = AvalancheEngine::kMagicBounce; // Middle exit north.
		_vm->_portals[3]._operation = AvalancheEngine::kMagicSpecial; // Door.
		break;
#if 0
	case 0x3:
		_vm->_celer->show_one(-1, -1, 4);
		_vm->magics[1].op = _vm->bounces; // { Right exit north. } { Change magic number! }
		_vm->portals[12].op = _vm->special; // { Door. }
		break;
	// RIGHT handles:
	case 0x4:
		_vm->_celer->show_one(-1, -1, 3);
		_vm->magics[1].op = _vm->bounces; // { Left exit north. } { Change magic number! }
		_vm->portals[12].op = _vm->special; // { Door. }
		break;
#endif
	case 0x5:
		_vm->_background->draw(-1, -1, 2);
		_vm->_magics[0]._operation = AvalancheEngine::kMagicBounce; // Middle exit north.
		_vm->_portals[3]._operation = AvalancheEngine::kMagicSpecial; // Door.
		break;
#if 0
	case 0x6:
		_vm->_celer->show_one(-1, -1, 3);
		_vm->magics[1].op = _vm->bounces; // { Right exit north. }
		_vm->portals[12].op = _vm->special; // { Door. }
		break;
#endif
	// ARCHWAYS:
	case 0x7:
	case 0x8:
	case 0x9:
		_vm->_background->draw(-1, -1, 5);

		if (((here & 0xf000) >> 12) > 0x7)
			_vm->_background->draw(-1, -1, 30);
		if (((here & 0xf000) >> 12) == 0x9)
			_vm->_background->draw(-1, -1, 31);

		_vm->_magics[0]._operation = AvalancheEngine::kMagicSpecial; // Middle arch north.
		_vm->_portals[3]._operation = AvalancheEngine::kMagicNothing; // Door.
		break;
	// DECORATIONS:
	case 0xd: // No connection + WINDOW
		_vm->_magics[0]._operation = AvalancheEngine::kMagicBounce;
		_vm->_portals[3]._operation = AvalancheEngine::kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 13);
		break;
	case 0xe: // No connection + TORCH
		_vm->_magics[0]._operation = AvalancheEngine::kMagicBounce;
		_vm->_portals[3]._operation = AvalancheEngine::kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 7);
		break;
	// Recessed door:
	case 0xf:
		_vm->_magics[0]._operation = AvalancheEngine::kMagicNothing; // Door to Geida's room.
		_vm->_background->draw(-1, -1, 0);
		_vm->_portals[3]._operation = AvalancheEngine::kMagicSpecial; // Door.
		break;
	}

	switch (xy_uint16) {
	case 514:
		_vm->_background->draw(-1, -1, 16);
		break;     // [2,2] : "Art Gallery" sign over door.
	case 264:
		_vm->_background->draw(-1, -1, 8);
		break;      // [8,1] : "The Wrong Way!" sign.
	case 1797:
		_vm->_background->draw(-1, -1, 1);
		break;      // [5,7] : "Ite Mingite" sign.
	case 258:
		for (int i = 0; i <= 2; i++) { // [2,1] : Art gallery - pictures
			_vm->_background->draw(130 + i * 120, 70, 14);
			_vm->_background->draw(184 + i * 120, 78, 15);
		}
		break;
	case 1287:
		for (int i = 10; i <= 13; i++)
			_vm->_background->draw(-1, -1, i - 1);
		break; // [7,5] : 4 candles.
	case 776:
		_vm->_background->draw(-1, -1, 9);
		break;     // [8,3] : 1 candle.
	case 2049:
		_vm->_background->draw(-1, -1, 10);
		break;     // [1,8] : another candle.
	case 257:
		_vm->_background->draw(-1, -1, 11);
		_vm->_background->draw(-1, -1, 12);
		break; // [1,1] : the other two.
	}

	if ((_vm->_geidaFollows) && (ped > 0)) {
		if (!_sprites[1]._quick)  // If we don't already have her...
			_sprites[1].init(5, true, this); // ...Load Geida.
		appearPed(1, geidaPed(ped));
		_sprites[1]._callEachStepFl = true;
		_sprites[1]._eachStepProc = kProcGeida;
	}
}

// This proc gets called whenever you touch a line defined as _vm->special.
void Animation::dawnDelay() {
	_vm->_timer->addTimer(2, Timer::kProcDawnDelay, Timer::kReasonDawndelay);
}

void Animation::callSpecial(uint16 which) {
	switch (which) {
	case 1: // _vm->special 1: Room 22: top of stairs.
		_vm->_background->draw(-1, -1, 0);
		_vm->_brummieStairs = 1;
		_vm->_magics[9]._operation = AvalancheEngine::kMagicNothing;
		_vm->_timer->addTimer(10, Timer::kProcStairs, Timer::kReasonBrummieStairs);
		stopWalking();
		_vm->_userMovesAvvy = false;
		break;
	case 2: // _vm->special 2: Room 22: bottom of stairs.
		_vm->_brummieStairs = 3;
		_vm->_magics[10]._operation = AvalancheEngine::kMagicNothing;
		_vm->_magics[11]._operation = AvalancheEngine::kMagicExclaim;
		_vm->_magics[11]._data = 5;
		_vm->_magics[3]._operation = AvalancheEngine::kMagicBounce; // Now works as planned!
		stopWalking();
		_vm->_dialogs->displayScrollChain('q', 26);
		_vm->_userMovesAvvy = true;
		break;
	case 3: // _vm->special 3: Room 71: triggers dart.
		_sprites[0].bounce(); // Must include that.

		if (!_arrowTriggered) {
			_arrowTriggered = true;
			appearPed(1, 3); // The dart starts at ped 4, and...
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
		_vm->dusk();
		_sprites[0].turn(kDirRight); // you'll see this after we get back from bootstrap
		_vm->_timer->addTimer(1, Timer::kProcGhostRoomPhew, Timer::kReasonGhostRoomPhew);
		//_vm->_enid->backToBootstrap(3); TODO: Replace it with proper ScummVM-friendly function(s)!  Do not remove until then!
		break;
	case 5:
		if (_vm->_friarWillTieYouUp) {
			// _vm->special 5: Room 42: touched tree, and get tied up.
			_vm->_magics[4]._operation = AvalancheEngine::kMagicBounce; // Boundary effect is now working again.
			_vm->_dialogs->displayScrollChain('q', 35);
			_sprites[0].remove();
			//tr[1].vanishifstill:=true;
			_vm->_background->draw(-1, -1, 1);
			_vm->_dialogs->displayScrollChain('q', 36);
			_vm->_tiedUp = true;
			_vm->_friarWillTieYouUp = false;
			_sprites[1].walkTo(2);
			_sprites[1]._vanishIfStill = true;
			_sprites[1]._doCheck = true; // One of them must have Check_Me switched on.
			_vm->setRoom(kPeopleFriarTuck, kRoomDummy); // Not here, then.
			_vm->_timer->addTimer(364, Timer::kProcHangAround, Timer::kReasonHangingAround);
		}
		break;
	case 6: // _vm->special 6: fall down oubliette.
		_vm->_userMovesAvvy = false;
		_sprites[0]._moveX = 3;
		_sprites[0]._moveY = 0;
		_sprites[0]._facingDir = kDirRight;
		_vm->_timer->addTimer(1, Timer::kProcFallDownOubliette, Timer::kReasonFallingDownOubliette);
		break;
	case 7: // _vm->special 7: stop falling down oubliette.
		_sprites[0]._visible = false;
		_vm->_magics[9]._operation = AvalancheEngine::kMagicNothing;
		stopWalking();
		_vm->_timer->loseTimer(Timer::kReasonFallingDownOubliette);
		//_vm->mblit(12, 80, 38, 160, 3, 0);
		//_vm->mblit(12, 80, 38, 160, 3, 1);
		_vm->_dialogs->displayText("Oh dear, you seem to be down the bottom of an oubliette.");
		_vm->_timer->addTimer(200, Timer::kProcMeetAvaroid, Timer::kReasonMeetingAvaroid);
		break;
	case 8:        // _vm->special 8: leave du Lustie's room.
		if ((_vm->_geidaFollows) && (!_vm->_lustieIsAsleep)) {
			_vm->_dialogs->displayScrollChain('q', 63);
			_sprites[1].turn(kDirDown);
			_sprites[1].stopWalk();
			_sprites[1]._callEachStepFl = false; // Geida
			_vm->gameOver();
		}
		break;
	case 9: // _vm->special 9: lose Geida to Robin Hood...
		if (!_vm->_geidaFollows)
			return;   // DOESN'T COUNT: no Geida.
		_sprites[1]._callEachStepFl = false; // She no longer follows Avvy around.
		_sprites[1].walkTo(3); // She walks to somewhere...
		_sprites[0].remove();     // Lose Avvy.
		_vm->_userMovesAvvy = false;
		_vm->_timer->addTimer(40, Timer::kProcRobinHoodAndGeida, Timer::kReasonRobinHoodAndGeida);
		break;
	case 10: // _vm->special 10: transfer north in catacombs.
		if ((_vm->_catacombX == 4) && (_vm->_catacombY == 1)) {
			// Into Geida's room.
			if (_vm->_objects[kObjectKey - 1])
				_vm->_dialogs->displayScrollChain('q', 62);
			else {
				_vm->_dialogs->displayScrollChain('q', 61);
				return;
			}
		}
		_vm->dusk();
		_vm->_catacombY--;
		catacombMove(4);
		if (_vm->_room != kRoomCatacombs)
			return;
		switch ((_vm->kCatacombMap[_vm->_catacombY - 1][_vm->_catacombX - 1] & 0xf00) >> 8) {
		case 0x1:
			appearPed(0, 11);
			break;
		case 0x3:
			appearPed(0, 10);
			break;
		default:
			appearPed(0, 3);
		}
		dawnDelay();
		break;
	case 11: // _vm->special 11: transfer east in catacombs.
		_vm->dusk();
		_vm->_catacombX++;
		catacombMove(1);
		if (_vm->_room != kRoomCatacombs)
			return;
		appearPed(0, 0);
		dawnDelay();
		break;
	case 12: // _vm->special 12: transfer south in catacombs.
		_vm->dusk();
		_vm->_catacombY++;
		catacombMove(2);
		if (_vm->_room != kRoomCatacombs)
			return;
		appearPed(0, 1);
		dawnDelay();
		break;
	case 13: // _vm->special 13: transfer west in catacombs.
		_vm->dusk();
		_vm->_catacombX--;
		catacombMove(3);
		if (_vm->_room != kRoomCatacombs)
			return;
		appearPed(0, 2);
		dawnDelay();
		break;
	}
}

void Animation::updateSpeed() {
	// Given that you've just changed the speed in _speedX, this adjusts _moveX.
	_sprites[0]._moveX = (_sprites[0]._moveX / 3) * _sprites[0]._speedX;
	_vm->_graphics->drawSpeedBar(_sprites[0]._speedX);
}

void Animation::setMoveSpeed(byte t, Direction dir) {
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
	default:
		break;
	}
}

void Animation::appearPed(byte sprNum, byte pedNum) {
	AnimationType *curSpr = &_sprites[sprNum];
	PedType *curPed = &_vm->_peds[pedNum];
	curSpr->appear(curPed->_x - curSpr->_info._xLength / 2, curPed->_y - curSpr->_info._yLength, curPed->_direction);
	setMoveSpeed(sprNum, curPed->_direction);
}

void Animation::followAvalotY(byte tripnum) {
	if (_sprites[0]._facingDir == kDirLeft)
		return;
	if (_sprites[tripnum]._homing)
		_sprites[tripnum]._homingY = _sprites[1]._y;
	else {
		if (_sprites[tripnum]._y < _sprites[1]._y)
			_sprites[tripnum]._y++;
		else if (_sprites[tripnum]._y > _sprites[1]._y)
			_sprites[tripnum]._y--;
		else
			return;
		if (_sprites[tripnum]._moveX == 0)  {
			_sprites[tripnum]._stepNum++;
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
			_vm->_dialogs->displayScrollChain('Q', 47); // Complaint!
			_sprites[tripnum].remove(); // Deallocate the arrow.

			_vm->gameOver();

			_vm->_userMovesAvvy = false; // Stop the user from moving him.
			_vm->_timer->addTimer(55, Timer::kProcNaughtyDuke, Timer::kReasonNaughtyDuke);
		}
	} else { // Arrow has hit the wall!
		_sprites[tripnum].remove(); // Deallocate the arrow.
		_vm->_background->draw(-1, -1, 2); // Show pic of arrow stuck into the door.
		_vm->_arrowInTheDoor = true; // So that we can pick it up.
	}

}

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

void Animation::spin(Direction dir, byte &tripnum) {
	if (_sprites[tripnum]._facingDir == dir)
		return;

	_sprites[tripnum]._facingDir = dir;
	if (_sprites[tripnum]._id == 2)
		return; // Not for Spludwick

	_geidaSpin++;
	_geidaTime = 20;
	if (_geidaSpin == 5) {
		_vm->_dialogs->displayText("Steady on, Avvy, you'll make the poor girl dizzy!");
		_geidaSpin = 0;
		_geidaTime = 0; // knock out records
	}
}

void Animation::geidaProcs(byte tripnum) {
	if (_geidaTime > 0) {
		_geidaTime--;
		if (_geidaTime == 0)
			_geidaSpin = 0;
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
		for (int i = 0; i < 4; i++) {
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

	for (int i = 0; i < 5; i++) {
		if (order[i] > -1)
			_sprites[order[i]].draw();
	}
}

/**
 * Animation links
 * @remarks	Originally called 'trippancy_link'
 */
void Animation::animLink() {
	if (_vm->_menu->isActive() | _vm->_seeScroll)
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
		_vm->_dialogs->displayScrollChain('x', _sayWhat);
	}
}

void Animation::stopWalking() {
	_sprites[0].stopWalk();
	_direction = kDirStopped;
	if (_vm->_alive)
		_sprites[0]._stepNum = 1;
}

/**
 * Hide in the cupboard
 * @remarks	Originally called 'hide_in_the_cupboard'
 */
void Animation::hideInCupboard() {
	if (_vm->_avvysInTheCupboard) {
		if (_vm->_parser->_wearing == kObjectDummy) {
			Common::String tmpStr = Common::String::format("%cAVVY!%cGet dressed first!", Dialogs::kControlItalic, Dialogs::kControlRoman);
			_vm->_dialogs->displayText(tmpStr);
		} else {
			_sprites[0]._visible = true;
			_vm->_userMovesAvvy = true;
			appearPed(0, 2); // Walk out of the cupboard.
			_vm->_dialogs->displayText("You leave the cupboard. Nice to be out of there!");
			_vm->_avvysInTheCupboard = false;
			_vm->_sequence->startCupboardSeq();
		}
	} else {
		// Not hiding in the cupboard
		_sprites[0]._visible = false;
		_vm->_userMovesAvvy = false;
		Common::String tmpStr = Common::String::format("You walk into the room...%cIt seems to be an empty, " \
			"but dusty, cupboard. Hmmmm... you leave the door slightly open to avoid suffocation.", Dialogs::kControlParagraph);
		_vm->_dialogs->displayText(tmpStr);
		_vm->_avvysInTheCupboard = true;
		_vm->_background->draw(-1, -1, 7);
	}
}

bool Animation::inField(byte which) {
	FieldType *curField = &_vm->_fields[which];
	int16 yy = _sprites[0]._y + _sprites[0]._info._yLength;

	return (_sprites[0]._x >= curField->_x1) && (_sprites[0]._x <= curField->_x2) && (yy >= curField->_y1) && (yy <= curField->_y2);
}

bool Animation::nearDoor() {
	if (_vm->_fieldNum < 8) {
		// there ARE no doors here!
		return false;
	}

	int16 ux = _sprites[0]._x;
	int16 uy = _sprites[0]._y + _sprites[0]._info._yLength;

	for (int i = 8; i < _vm->_fieldNum; i++) {
		FieldType *curField = &_vm->_fields[i];
		if ((ux >= curField->_x1) && (ux <= curField->_x2) && (uy >= curField->_y1) && (uy <= curField->_y2))
			return true;
	}

	return false;
}

void Animation::handleMoveKey(const Common::Event &event) {
	if (!_vm->_userMovesAvvy)
		return;

	if (_vm->_menu->_activeMenuItem._activeNow)
		_vm->_parser->tryDropdown();
	else {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
			if (_direction != kDirUp) {
				_direction = kDirUp;
				setMoveSpeed(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_DOWN:
			if (_direction != kDirDown) {
				_direction = kDirDown;
				setMoveSpeed(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_LEFT:
			if (_direction != kDirLeft) {
				_direction = kDirLeft;
				setMoveSpeed(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_RIGHT:
			if (_direction != kDirRight) {
				_direction = kDirRight;
				setMoveSpeed(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_PAGEUP:
			if (_direction != kDirUpRight) {
				_direction = kDirUpRight;
				setMoveSpeed(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_PAGEDOWN:
			if (_direction != kDirDownRight) {
				_direction = kDirDownRight;
				setMoveSpeed(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_END:
			if (_direction != kDirDownLeft) {
				_direction = kDirDownLeft;
				setMoveSpeed(0, _direction);
			} else
				stopWalking();
			break;
		case Common::KEYCODE_HOME:
			if (_direction != kDirUpLeft) {
				_direction = kDirUpLeft;
				setMoveSpeed(0, _direction);
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

void Animation::setDirection(Direction dir) {
	_direction = dir;
}

void Animation::setOldDirection(Direction dir) {
	_oldDirection = dir;
}

Direction Animation::getDirection() {
	return _direction;
}

Direction Animation::getOldDirection() {
	return _oldDirection;
}

void Animation::resetVariables() {
	_geidaSpin = 0;
	_geidaTime = 0;
	_arrowTriggered = false;
}

void Animation::synchronize(Common::Serializer &sz) {
	sz.syncAsByte(_direction);
	sz.syncAsByte(_geidaSpin);
	sz.syncAsByte(_geidaTime);

	byte spriteNum = 0;
	if (sz.isSaving()) {
		for (int i = 0; i < kSpriteNumbMax; i++) {
			if (_sprites[i]._quick)
				spriteNum++;
		}
	}
	sz.syncAsByte(spriteNum);

	if (sz.isLoading()) {
		for (int i = 0; i < kSpriteNumbMax; i++) { // Deallocate sprites.
			AnimationType *spr = &_sprites[i];
			if (spr->_quick)
				spr->remove();
		}
	}

	for (int i = 0; i < spriteNum; i++) {
		AnimationType *spr = &_sprites[i];
		sz.syncAsByte(spr->_id);
		sz.syncAsByte(spr->_doCheck);

		if (sz.isLoading()) {
			spr->_quick = true;
			spr->init(spr->_id, spr->_doCheck, this);
		}

		sz.syncAsByte(spr->_moveX);
		sz.syncAsByte(spr->_moveY);
		sz.syncAsByte(spr->_facingDir);
		sz.syncAsByte(spr->_stepNum);
		sz.syncAsByte(spr->_visible);
		sz.syncAsByte(spr->_homing);
		sz.syncAsByte(spr->_count);
		sz.syncAsByte(spr->_info._xWidth);
		sz.syncAsByte(spr->_speedX);
		sz.syncAsByte(spr->_speedY);
		sz.syncAsByte(spr->_animCount);
		sz.syncAsSint16LE(spr->_homingX);
		sz.syncAsSint16LE(spr->_homingY);
		sz.syncAsByte(spr->_callEachStepFl);
		sz.syncAsByte(spr->_eachStepProc);
		sz.syncAsByte(spr->_vanishIfStill);
		sz.syncAsSint16LE(spr->_x);
		sz.syncAsSint16LE(spr->_y);

		if (sz.isLoading() && spr->_visible)
			spr->appear(spr->_x, spr->_y, spr->_facingDir);
	}

	sz.syncAsByte(_arrowTriggered);
}

} // End of namespace Avalanche.
