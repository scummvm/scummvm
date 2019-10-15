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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* TRIP5	Trippancy V - the sprite animation subsystem */

#include "common/system.h"
#include "avalanche/avalanche.h"
#include "avalanche/animation.h"

namespace Avalanche {

// Art gallery at 2,1; notice about this at 2,2.
const int32 Animation::kCatacombMap[8][8] = {
	// Geida's room
	// 1	   2	   3	   4	   5	   6	   7	   8
	{0x204,	 0x200,  0xd0f0, 0xf0ff, 0xff,   0xd20f, 0xd200, 0x200},
	{0x50f1, 0x20ff, 0x2ff,  0xff,   0xe0ff, 0x20ff, 0x200f, 0x7210},
	{0xe3f0, 0xe10f, 0x72f0, 0xff,   0xe0ff, 0xff,   0xff,   0x800f},
	{0x2201, 0x2030, 0x800f, 0x220,  0x20f,  0x30,   0xff,   0x23f}, // >> Oubliette
	{0x5024, 0xf3,   0xff,   0x200f, 0x22f0, 0x20f,  0x200,  0x7260},
	{0xf0,   0x2ff,  0xe2ff, 0xff,   0x200f, 0x50f0, 0x72ff, 0x201f},
	{0xf6,   0x220f, 0x22f0, 0x30f,  0xf0,   0x20f,  0x8200, 0x2f0}, // <<< In here
	{0x34,   0x200f, 0x51f0, 0x201f, 0xf1,   0x50ff, 0x902f, 0x2062}
};

AnimationType::AnimationType(Animation *anim) {
	_anim = anim;

	_xLength = 0;
	_yLength = 0;
	for (int i = 0; i < 24; i++) {
		_mani[i] = nullptr;
		_sil[i] = nullptr;
	}
	_frameNum = 0;
	_seq = 0;
	_characterId = 0;
	_count = 0;
	_facingDir = kDirNone;
	_stepNum = 0;
	_x = 0;
	_y = 0;
	_moveX = 0;
	_moveY = 0;
	_quick = false;
	_visible = false;
	_homing = false;
	_doCheck = false;
	_homingX = 0;
	_homingY = 0;
	_speedX = 0;
	_speedY = 0;
	_vanishIfStill = false;
	_callEachStepFl = false;
	_eachStepProc = Animation::kProcNone;
	_fgBubbleCol = kColorWhite;
	_bgBubbleCol = kColorBlack;
	_id = 177;
}

/**
 * Loads & sets up the sprite.
 */
void AnimationType::init(byte spritenum, bool doCheck) {
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
	// Skip real name Size (1 byte) then fixed sized zone containing name (12 bytes)
	inf.skip(1 + 12);
	// Skip real comment size (1 byte) then fixed sized zone containing comment (16 bytes)
	inf.skip(1 + 16);

	_frameNum = inf.readByte();
	_xLength = inf.readByte();
	_yLength = inf.readByte();
	_seq = inf.readByte();
	uint16 size = inf.readUint16LE();
	assert (size > 6);
	_fgBubbleCol = (Color)inf.readByte();
	_bgBubbleCol = (Color)inf.readByte();
	_characterId = inf.readByte();

	byte xWidth = _xLength / 8;
	if ((_xLength % 8) > 0)
		xWidth++;
	for (int i = 0; i < _frameNum; i++) {
		_sil[i] = new SilType[11 * (_yLength + 1)];
		_mani[i] = new ManiType[size - 6];
		for (int j = 0; j <= _yLength; j++)
			inf.read((*_sil[i])[j], xWidth);
		inf.read(*_mani[i], size - 6);
	}

	_x = 0;
	_y = 0;
	_quick = true;
	_visible = false;
	_speedX = kWalk;
	_speedY = 1;
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

/**
 * Just sets 'quick' to false.
 * @remarks	Originally called 'original'
 */
void AnimationType::reset() {
	_quick = false;
	_id = 177;
}

/**
 * Drops sprite onto screen.
 * @remarks	Originally called 'andexor'
 */
void AnimationType::draw() {
	if (_vanishIfStill && (_moveX == 0) && (_moveY == 0))
		return;

	byte picnum = _facingDir * _seq + _stepNum;

	_anim->_vm->_graphics->drawSprite(this, picnum, _x, _y);
}

/**
 * Turns character round.
 */
void AnimationType::turn(Direction whichway) {
	if (whichway == 8)
		_facingDir = kDirUp;
	else
		_facingDir = whichway;
}

/**
 * Switches it on.
 */
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
		AnimationType *spr = _anim->_sprites[i];
		if (spr->_quick && (spr->_id != _id) && (_x + _xLength > spr->_x) && (_x < spr->_x + spr->_xLength) && (spr->_y == _y))
			return true;
	}

	return false;
}

/**
 * Prepares for draw(), etc.
 */
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

		byte magicColor = _anim->checkFeet(_x, _x + _xLength, _oldY[_anim->_vm->_cp], _y, _yLength) - 1;
		// -1  is because the modified array indexes of magics[] compared to Pascal .

		if ((magicColor != 255) & !_anim->_vm->_doingSpriteRun) {
			MagicType *magic = &_anim->_vm->_magics[magicColor];
			switch (magic->_operation) {
			case kMagicExclaim:
				bounce();
				_anim->_mustExclaim = true;
				_anim->_sayWhat = magic->_data;
				break;
			case kMagicBounce:
				bounce();
				break;
			case kMagicTransport:
				_anim->_vm->flipRoom((Room)(magic->_data >> 8), magic->_data & 0xff);
				break;
			case kMagicUnfinished: {
				bounce();
				Common::String tmpStr = Common::String::format("%c%cSorry.%cThis place is not available yet!",
					kControlBell, kControlCenter, kControlRoman);
				_anim->_vm->_dialogs->displayText(tmpStr);
				}
				break;
			case kMagicSpecial:
				_anim->callSpecial(magic->_data);
				break;
			case kMagicOpenDoor:
				_anim->_vm->openDoor((Room)(magic->_data >> 8), magic->_data & 0xff, magicColor);
				break;
			case kMagicNothing:
			default:
				break;
			}
		}
	}

	if (!_anim->_vm->_doingSpriteRun) {
		_count++;
		if (((_moveX != 0) || (_moveY != 0)) && (_count > 1)) {
			_stepNum++;
			if (_stepNum == _seq)
				_stepNum = 0;
			_count = 0;
		}
	}
}

/**
 * Bounces off walls
 */
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

/**
 * Home in on a point.
 */
void AnimationType::walkTo(byte pedNum) {
	PedType *curPed = &_anim->_vm->_peds[pedNum];

	setSpeed(getSign(curPed->_x - _x) * 4, getSign(curPed->_y - _y));
	_homingX = curPed->_x - _xLength / 2;
	_homingY = curPed->_y - _yLength;
	_homing = true;
}

void AnimationType::stopHoming() {
	_homing = false;
}

/**
 * Calculates ix & iy for one homing step.
 */
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

/**
 * Sets ix & iy, non-homing, etc.
 */
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

/**
 * Stops the sprite from moving.
 */
void AnimationType::stopWalk() {
	_moveX = 0;
	_moveY = 0;
	_homing = false;
}

/**
 * Sets up talk vars.
 */
void AnimationType::chatter() {
	_anim->_vm->_dialogs->setTalkPos(_x + _xLength / 2, _y);
	_anim->_vm->_graphics->setDialogColor(_bgBubbleCol, _fgBubbleCol);
}

void AnimationType::remove() {
	for (int i = 0; i < _frameNum; i++) {
		delete[] _mani[i];
		delete[] _sil[i];
	}

	_quick = false;
	_id = 177;
}

Animation::Animation(AvalancheEngine *vm) {
	_vm = vm;
	_mustExclaim = false;

	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		_sprites[i] = new AnimationType(this);
	}

	_direction = kDirNone;
	_oldDirection = kDirNone;
	_arrowTriggered = false;
	_geidaSpin = 0;
	_geidaTime = 0;
	_sayWhat = 0;
}

Animation::~Animation() {
	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		AnimationType *curSpr = _sprites[i];

		if (curSpr->_quick)
			curSpr->remove();
		delete(curSpr);
	}
}

/**
 * Resets Animation variables.
 * @remarks	Originally called 'loadtrip'
 */
void Animation::resetAnims() {
	setDirection(kDirStopped);
	for (int16 i = 0; i < kSpriteNumbMax; i++)
		_sprites[i]->reset();
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

/**
 * When you enter a new position in the catacombs, this procedure should be
 * called. It changes the 'also' codes so that they may match the picture
 * on the screen.
 */
void Animation::catacombMove(byte ped) {
	// XY_uint16 is _catacombX+_catacombY*256. Thus, every room in the
	// catacombs has a different number for it.
	uint16 xy = _vm->_catacombX + _vm->_catacombY * 256;
	_geidaSpin = 0;

	switch (xy) {
	case 1801: // Exit catacombs
		_vm->flipRoom(kRoomLustiesRoom, 4);
		_vm->_dialogs->displayText("Phew! Nice to be out of there!");
		return;
	case 1033:{ // Oubliette
		_vm->flipRoom(kRoomOubliette, 1);
		Common::String tmpStr = Common::String::format("Oh, NO!%c1%c", kControlRegister, kControlSpeechBubble);
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
		_sprites[0]->_moveY = 1;
		_sprites[0]->_moveX = 0;
		return;
	default:
		break;
	}

	if (!_vm->_enterCatacombsFromLustiesRoom)
		_vm->loadRoom(29);
	int32 here = kCatacombMap[_vm->_catacombY - 1][_vm->_catacombX - 1];

	switch (here & 0xf) { // West.
	case 0: // no connection (wall)
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 27);
		break;
	case 0x1: // no connection (wall + shield),
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 28); // ...shield.
		break;
	case 0x2: // wall with door
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door.
		break;
	case 0x3: // wall with door and shield
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door, and...
		_vm->_background->draw(-1, -1, 28); // ...shield.
		break;
	case 0x4: // no connection (wall + window),
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 4);  // ...window.
		break;
	case 0x5: // wall with door and window
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door, and...
		_vm->_background->draw(-1, -1, 4); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicNothing; // No door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 6); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_magics[1]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[2]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[4]._operation = kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 27); // Wall, plus...
		_vm->_background->draw(-1, -1, 29); // ...door, and...
		_vm->_background->draw(-1, -1, 6); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_magics[1]._operation = kMagicNothing; // Sloping wall.
		_vm->_magics[2]._operation = kMagicSpecial; // Straight wall.
		break;
	default:
		break;
	}

	/*  ---- */

	switch ((here & 0xf0) >> 4) { // East
	case 0: // no connection (wall)
		_vm->_magics[4]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 18);
		break;
	case 0x1: // no connection (wall + window),
		_vm->_magics[4]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 19); // ...window.
		break;
	case 0x2: // wall with door
		_vm->_magics[4]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 20); // ...door.
		break;
	case 0x3: // wall with door and window
		_vm->_magics[4]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 19); // ...door, and...
		_vm->_background->draw(-1, -1, 20); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_magics[4]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = kMagicNothing; // No door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 17); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_magics[4]._operation = kMagicBounce; // Sloping wall.
		_vm->_magics[5]._operation = kMagicNothing; // Straight wall.
		_vm->_portals[6]._operation = kMagicSpecial; // Door.
		_vm->_background->draw(-1, -1, 18); // Wall, plus...
		_vm->_background->draw(-1, -1, 20); // ...door, and...
		_vm->_background->draw(-1, -1, 17); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_magics[4]._operation = kMagicNothing; // Sloping wall.
		_vm->_magics[5]._operation = kMagicSpecial; // Straight wall.
		_vm->_portals[6]._operation = kMagicNothing; // Door.
		break;
	default:
		break;
	}

	switch ((here & 0xf00) >> 8) { // South
	case 0: // No connection.
		_vm->_magics[6]._operation = kMagicBounce;
		_vm->_magics[11]._operation = kMagicBounce;
		_vm->_magics[12]._operation = kMagicBounce;
		break;
	case 0x1:
		_vm->_background->draw(-1, -1, 21);

		if ((xy == 2051) && _vm->_geidaFollows)
			_vm->_magics[12]._operation = kMagicExclaim;
		else
			_vm->_magics[12]._operation = kMagicSpecial; // Right exit south.

		_vm->_magics[6]._operation = kMagicBounce;
		_vm->_magics[11]._operation = kMagicBounce;
		break;
	case 0x2:
		_vm->_background->draw(-1, -1, 22);
		_vm->_magics[6]._operation = kMagicSpecial; // Middle exit south.
		_vm->_magics[11]._operation = kMagicBounce;
		_vm->_magics[12]._operation = kMagicBounce;
		break;
	case 0x3:
		_vm->_background->draw(-1, -1, 23);
		_vm->_magics[11]._operation = kMagicSpecial; // Left exit south.
		_vm->_magics[6]._operation = kMagicBounce;
		_vm->_magics[12]._operation = kMagicBounce;
		break;
	default:
		break;
	}

	switch ((here & 0xf000) >> 12) { // North
	case 0: // No connection
		_vm->_magics[0]._operation = kMagicBounce;
		_vm->_portals[3]._operation = kMagicNothing; // Door.
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
		_vm->_magics[0]._operation = kMagicBounce; // Middle exit north.
		_vm->_portals[3]._operation = kMagicSpecial; // Door.
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
		_vm->_magics[0]._operation = kMagicBounce; // Middle exit north.
		_vm->_portals[3]._operation = kMagicSpecial; // Door.
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

		_vm->_magics[0]._operation = kMagicSpecial; // Middle arch north.
		_vm->_portals[3]._operation = kMagicNothing; // Door.
		break;
	// DECORATIONS:
	case 0xd: // No connection + WINDOW
		_vm->_magics[0]._operation = kMagicBounce;
		_vm->_portals[3]._operation = kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 13);
		break;
	case 0xe: // No connection + TORCH
		_vm->_magics[0]._operation = kMagicBounce;
		_vm->_portals[3]._operation = kMagicNothing; // Door.
		_vm->_background->draw(-1, -1, 7);
		break;
	// Recessed door:
	case 0xf:
		_vm->_magics[0]._operation = kMagicNothing; // Door to Geida's room.
		_vm->_background->draw(-1, -1, 0);
		_vm->_portals[3]._operation = kMagicSpecial; // Door.
		break;
	default:
		break;
	}

	switch (xy) {
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
	default:
		break;
	}

	if (_vm->_geidaFollows && (ped > 0)) {
		AnimationType *spr1 = _sprites[1];

		if (!spr1->_quick)  // If we don't already have her...
			spr1->init(5, true); // ...Load Geida.
		appearPed(1, geidaPed(ped));
		spr1->_callEachStepFl = true;
		spr1->_eachStepProc = kProcFollowAvvy;
	}
}

/**
 * This proc gets called whenever you touch a line defined as _vm->special.
 */
void Animation::dawnDelay() {
	_vm->_timer->addTimer(2, Timer::kProcDawnDelay, Timer::kReasonDawndelay);
}

void Animation::callSpecial(uint16 which) {
	switch (which) {
	case 1: // _vm->special 1: Room 22: top of stairs.
		_vm->_background->draw(-1, -1, 0);
		_vm->_brummieStairs = 1;
		_vm->_magics[9]._operation = kMagicNothing;
		_vm->_timer->addTimer(10, Timer::kProcStairs, Timer::kReasonBrummieStairs);
		stopWalking();
		_vm->_userMovesAvvy = false;
		break;
	case 2: // _vm->special 2: Room 22: bottom of stairs.
		_vm->_brummieStairs = 3;
		_vm->_magics[10]._operation = kMagicNothing;
		_vm->_magics[11]._operation = kMagicExclaim;
		_vm->_magics[11]._data = 5;
		_vm->_magics[3]._operation = kMagicBounce; // Now works as planned!
		stopWalking();
		_vm->_dialogs->displayScrollChain('Q', 26);
		_vm->_userMovesAvvy = true;
		break;
	case 3: // _vm->special 3: Room 71: triggers dart.
		_sprites[0]->bounce(); // Must include that.

		if (!_arrowTriggered) {
			_arrowTriggered = true;

			AnimationType *spr1 = _sprites[1];
			appearPed(1, 3); // The dart starts at ped 4, and...
			spr1->walkTo(4); // flies to ped 5 (- 1 for pascal to C conversion).
			spr1->_facingDir = kDirUp; // Only face.
			// Should call some kind of Eachstep procedure which will deallocate
			// the sprite when it hits the wall, and replace it with the chunk
			// graphic of the arrow buried in the plaster. */

			// OK!
			spr1->_callEachStepFl = true;
			spr1->_eachStepProc = kProcArrow;
		}
		break;
	case 4: // This is the ghost room link.
		_sprites[0]->turn(kDirRight); // You'll see this after we get back.
		_vm->_timer->addTimer(1, Timer::kProcGhostRoomPhew, Timer::kReasonGhostRoomPhew);
		_vm->_ghostroom->run();
		break;
	case 5:
		if (_vm->_friarWillTieYouUp) {
			// _vm->special 5: Room 42: touched tree, and get tied up.
			_vm->_magics[4]._operation = kMagicBounce; // Boundary effect is now working again.
			_vm->_dialogs->displayScrollChain('Q', 35);
			_sprites[0]->remove();

			AnimationType *spr1 = _sprites[1];
			_vm->_background->draw(-1, -1, 1);
			_vm->_dialogs->displayScrollChain('Q', 36);
			_vm->_tiedUp = true;
			_vm->_friarWillTieYouUp = false;
			spr1->walkTo(2);
			spr1->_vanishIfStill = true;
			spr1->_doCheck = true; // One of them must have Check_Me switched on.
			_vm->setRoom(kPeopleFriarTuck, kRoomDummy); // Not here, then.
			_vm->_timer->addTimer(364, Timer::kProcHangAround, Timer::kReasonHangingAround);
		}
		break;
	case 6: {
		// _vm->special 6: fall down oubliette.
		AnimationType *avvy = _sprites[0];
		_vm->_userMovesAvvy = false;
		avvy->_moveX = 3;
		avvy->_moveY = 0;
		avvy->_facingDir = kDirRight;
		_vm->_timer->addTimer(1, Timer::kProcFallDownOubliette, Timer::kReasonFallingDownOubliette);
		}
		break;
	case 7: // _vm->special 7: stop falling down oubliette.
		_sprites[0]->_visible = false;
		_vm->_magics[9]._operation = kMagicNothing;
		stopWalking();
		_vm->_timer->loseTimer(Timer::kReasonFallingDownOubliette);
		//_vm->mblit(12, 80, 38, 160, 3, 0);
		//_vm->mblit(12, 80, 38, 160, 3, 1);
		_vm->_dialogs->displayText("Oh dear, you seem to be down the bottom of an oubliette.");
		_vm->_timer->addTimer(200, Timer::kProcMeetAvaroid, Timer::kReasonMeetingAvaroid);
		break;
	case 8:        // _vm->special 8: leave du Lustie's room.
		if (_vm->_geidaFollows && !_vm->_lustieIsAsleep) {
			AnimationType *spr1 = _sprites[1];
			_vm->_dialogs->displayScrollChain('Q', 63);
			spr1->turn(kDirDown);
			spr1->stopWalk();
			spr1->_callEachStepFl = false; // Geida
			_vm->gameOver();
		}
		break;
	case 9: {
		// _vm->special 9: lose Geida to Robin Hood...
		if (!_vm->_geidaFollows)
			return;   // DOESN'T COUNT: no Geida.
		AnimationType *spr1 = _sprites[1];
		spr1->_callEachStepFl = false; // She no longer follows Avvy around.
		spr1->walkTo(3); // She walks to somewhere...
		_sprites[0]->remove();     // Lose Avvy.
		_vm->_userMovesAvvy = false;
		_vm->_timer->addTimer(40, Timer::kProcRobinHoodAndGeida, Timer::kReasonRobinHoodAndGeida);
		}
		break;
	case 10: // _vm->special 10: transfer north in catacombs.
		if ((_vm->_catacombX == 4) && (_vm->_catacombY == 1)) {
			// Into Geida's room.
			if (_vm->_objects[kObjectKey - 1])
				_vm->_dialogs->displayScrollChain('Q', 62);
			else {
				_vm->_dialogs->displayScrollChain('Q', 61);
				return;
			}
		}
		_vm->fadeOut();
		_vm->_catacombY--;
		catacombMove(4);
		if (_vm->_room != kRoomCatacombs)
			return;
		switch ((kCatacombMap[_vm->_catacombY - 1][_vm->_catacombX - 1] & 0xf00) >> 8) {
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
		_vm->fadeOut();
		_vm->_catacombX++;
		catacombMove(1);
		if (_vm->_room != kRoomCatacombs)
			return;
		appearPed(0, 0);
		dawnDelay();
		break;
	case 12: // _vm->special 12: transfer south in catacombs.
		_vm->fadeOut();
		_vm->_catacombY++;
		catacombMove(2);
		if (_vm->_room != kRoomCatacombs)
			return;
		appearPed(0, 1);
		dawnDelay();
		break;
	case 13: // _vm->special 13: transfer west in catacombs.
		_vm->fadeOut();
		_vm->_catacombX--;
		catacombMove(3);
		if (_vm->_room != kRoomCatacombs)
			return;
		appearPed(0, 2);
		dawnDelay();
		break;
	default:
		break;
	}
}

void Animation::updateSpeed() {
	AnimationType *avvy = _sprites[0];
	// Given that you've just changed the speed in _speedX, this adjusts _moveX.
	avvy->_moveX = (avvy->_moveX / 3) * avvy->_speedX;
	_vm->_graphics->drawSpeedBar(avvy->_speedX);
}

void Animation::setMoveSpeed(byte t, Direction dir) {
	AnimationType *spr = _sprites[t];
	switch (dir) {
	case kDirUp:
		spr->setSpeed(0, -spr->_speedY);
		break;
	case kDirDown:
		spr->setSpeed(0, spr->_speedY);
		break;
	case kDirLeft:
		spr->setSpeed(-spr->_speedX,  0);
		break;
	case kDirRight:
		spr->setSpeed(spr->_speedX,  0);
		break;
	case kDirUpLeft:
		spr->setSpeed(-spr->_speedX, -spr->_speedY);
		break;
	case kDirUpRight:
		spr->setSpeed(spr->_speedX, -spr->_speedY);
		break;
	case kDirDownLeft:
		spr->setSpeed(-spr->_speedX, spr->_speedY);
		break;
	case kDirDownRight:
		spr->setSpeed(spr->_speedX, spr->_speedY);
		break;
	default:
		break;
	}
}

void Animation::appearPed(byte sprNum, byte pedNum) {
	AnimationType *curSpr = _sprites[sprNum];
	PedType *curPed = &_vm->_peds[pedNum];
	curSpr->appear(curPed->_x - curSpr->_xLength / 2, curPed->_y - curSpr->_yLength, curPed->_direction);
	setMoveSpeed(sprNum, curPed->_direction);
}

/**
 * @remarks	Originally called 'follow_avvy_y'
 */
void Animation::followAvalotY(byte tripnum) {
	if (_sprites[0]->_facingDir == kDirLeft)
		return;

	AnimationType *tripSpr = _sprites[tripnum];
	AnimationType *spr1 = _sprites[1];

	if (tripSpr->_homing)
		tripSpr->_homingY = spr1->_y;
	else {
		if (tripSpr->_y < spr1->_y)
			tripSpr->_y++;
		else if (tripSpr->_y > spr1->_y)
			tripSpr->_y--;
		else
			return;

		if (tripSpr->_moveX == 0)  {
			tripSpr->_stepNum++;
			if (tripSpr->_stepNum == tripSpr->_seq)
				tripSpr->_stepNum = 0;
			tripSpr->_count = 0;
		}
	}
}

void Animation::backAndForth(byte tripnum) {
	AnimationType *tripSpr = _sprites[tripnum];

	if (!tripSpr->_homing) {
		if (tripSpr->_facingDir == kDirRight)
			tripSpr->walkTo(3);
		else
			tripSpr->walkTo(4);
	}
}

void Animation::faceAvvy(byte tripnum) {
	AnimationType *tripSpr = _sprites[tripnum];

	if (!tripSpr->_homing) {
		if (_sprites[0]->_x >= tripSpr->_x)
			tripSpr->_facingDir = kDirRight;
		else
			tripSpr->_facingDir = kDirLeft;
	}
}

void Animation::arrowProcs(byte tripnum) {
	AnimationType *tripSpr = _sprites[tripnum];
	AnimationType *avvy = _sprites[tripnum];

	if (tripSpr->_homing) {
		// Arrow is still in flight.
		// We must check whether or not the arrow has collided tr[tripnum] Avvy's head.
		// This is so if: a) the bottom of the arrow is below Avvy's head,
		// b) the left of the arrow is left of the right of Avvy's head, and
		// c) the right of the arrow is right of the left of Avvy's head.
		if ((tripSpr->_y + tripSpr->_yLength >= avvy->_y) // A
			&& (tripSpr->_x <= avvy->_x + avvy->_xLength) // B
			&& (tripSpr->_x + tripSpr->_xLength >= avvy->_x)) { // C
			// OK, it's hit him... what now?

			_sprites[1]->_callEachStepFl = false; // prevent recursion.
			_vm->_dialogs->displayScrollChain('Q', 47); // Complaint!
			tripSpr->remove(); // Deallocate the arrow.

			_vm->gameOver();

			_vm->_userMovesAvvy = false; // Stop the user from moving him.
			_vm->_timer->addTimer(55, Timer::kProcNaughtyDuke, Timer::kReasonNaughtyDuke);
		}
	} else { // Arrow has hit the wall!
		tripSpr->remove(); // Deallocate the arrow.
		_vm->_background->draw(-1, -1, 2); // Show pic of arrow stuck into the door.
		_vm->_arrowInTheDoor = true; // So that we can pick it up.
	}
}

void Animation::grabAvvy(byte tripnum) {     // For Friar Tuck, in Nottingham.
	AnimationType *tripSpr = _sprites[tripnum];
	AnimationType *avvy = _sprites[0];

	int16 tox = avvy->_x + 17;
	int16 toy = avvy->_y - 1;
	if ((tripSpr->_x == tox) && (tripSpr->_y == toy)) {
		tripSpr->_callEachStepFl = false;
		tripSpr->_facingDir = kDirLeft;
		tripSpr->stopWalk();
		// ... whatever ...
	} else {
		// Still some way to go.
		if (tripSpr->_x < tox) {
			tripSpr->_x += 5;
			if (tripSpr->_x > tox)
				tripSpr->_x = tox;
		}
		if (tripSpr->_y < toy)
			tripSpr->_y++;
		tripSpr->_stepNum++;
		if (tripSpr->_stepNum == tripSpr->_seq)
			tripSpr->_stepNum = 0;
	}
}

void Animation::takeAStep(byte &tripnum) {
	AnimationType *tripSpr = _sprites[tripnum];

	if (tripSpr->_moveX == 0) {
		tripSpr->_stepNum++;
		if (tripSpr->_stepNum == tripSpr->_seq)
			tripSpr->_stepNum = 0;
		tripSpr->_count = 0;
	}
}

void Animation::spin(Direction dir, byte &tripnum) {
	AnimationType *tripSpr = _sprites[tripnum];

	if (tripSpr->_facingDir == dir)
		return;

	tripSpr->_facingDir = dir;
	if (tripSpr->_id == 2)
		return; // Not for Spludwick

	_geidaSpin++;
	_geidaTime = 20;
	if (_geidaSpin == 5) {
		_vm->_dialogs->displayText("Steady on, Avvy, you'll make the poor girl dizzy!");
		_geidaSpin = 0;
		_geidaTime = 0; // knock out records
	}
}

void Animation::follow(byte tripnum) {
	AnimationType *tripSpr = _sprites[tripnum];
	AnimationType *avvy = _sprites[0];

	if (_geidaTime > 0) {
		_geidaTime--;
		if (_geidaTime == 0)
			_geidaSpin = 0;
	}

	if (tripSpr->_y < (avvy->_y - 2)) {
		// The following NPC is further from the screen than Avvy.
		spin(kDirDown, tripnum);
		tripSpr->_moveY = 1;
		tripSpr->_moveX = 0;
		takeAStep(tripnum);
		return;
	} else if (tripSpr->_y > (avvy->_y + 2)) {
		// Avvy is further from the screen than the following NPC.
		spin(kDirUp, tripnum);
		tripSpr->_moveY = -1;
		tripSpr->_moveX = 0;
		takeAStep(tripnum);
		return;
	}

	tripSpr->_moveY = 0;
	// These 12-s are not in the original, I added them to make the following method more "smooth".
	// Now the NPC which is following Avvy won't block his way and will walk next to him properly.
	if (tripSpr->_x < avvy->_x - avvy->_speedX * 8 - 12) {
		tripSpr->_moveX = avvy->_speedX;
		spin(kDirRight, tripnum);
		takeAStep(tripnum);
	} else if (tripSpr->_x > avvy->_x + avvy->_speedX * 8 + 12) {
		tripSpr->_moveX = -avvy->_speedX;
		spin(kDirLeft, tripnum);
		takeAStep(tripnum);
	} else
		tripSpr->_moveX = 0;
}

/**
 * @remarks	Originally called 'call_andexors'
 */
void Animation::drawSprites() {
	int8 order[5];
	byte temp;
	bool ok;

	for (int i = 0; i < 5; i++)
		order[i] = -1;

	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		AnimationType *curSpr = _sprites[i];
		if (curSpr->_quick && curSpr->_visible)
			order[i] = i;
	}

	do {
		ok = true;
		for (int i = 0; i < 4; i++) {
			if ((order[i] != -1) && (order[i + 1] != -1) && (_sprites[order[i]]->_y > _sprites[order[i + 1]]->_y)) {
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
			_sprites[order[i]]->draw();
	}
}

/**
 * Animation links
 * @remarks	Originally called 'trippancy_link'
 */
void Animation::animLink() {
	if (_vm->_dropdown->isActive() || !_vm->_animationsEnabled)
		return;

	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		AnimationType *curSpr = _sprites[i];
		if (curSpr->_quick && curSpr->_visible)
			curSpr->walk();
	}

	drawSprites();

	for (int16 i = 0; i < kSpriteNumbMax; i++) {
		AnimationType *curSpr = _sprites[i];
		if (curSpr->_quick && curSpr->_callEachStepFl) {
			switch (curSpr->_eachStepProc) {
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
			case kProcFollowAvvy :
				follow(i);
				break;
			default:
				break;
			}
		}
	}

	if (_mustExclaim) {
		_mustExclaim = false;
		_vm->_dialogs->displayScrollChain('X', _sayWhat);
	}
}

void Animation::stopWalking() {
	AnimationType *avvy = _sprites[0];

	avvy->stopWalk();
	_direction = kDirStopped;
	if (_vm->_alive)
		avvy->_stepNum = 1;
}

/**
 * Hide in the cupboard
 * @remarks	Originally called 'hide_in_the_cupboard'
 */
void Animation::hideInCupboard() {
	if (_vm->_avvysInTheCupboard) {
		if (_vm->_parser->_wearing == kObjectDummy) {
			Common::String tmpStr = Common::String::format("%cAVVY!%cGet dressed first!", kControlItalic, kControlRoman);
			_vm->_dialogs->displayText(tmpStr);
		} else {
			_sprites[0]->_visible = true;
			_vm->_userMovesAvvy = true;
			appearPed(0, 2); // Walk out of the cupboard.
			_vm->_dialogs->displayText("You leave the cupboard. Nice to be out of there!");
			_vm->_avvysInTheCupboard = false;
			_vm->_sequence->startCupboardSeq();
		}
	} else {
		// Not hiding in the cupboard
		_sprites[0]->_visible = false;
		_vm->_userMovesAvvy = false;
		Common::String tmpStr = Common::String::format("You walk into the room...%cIt seems to be an empty, " \
			"but dusty, cupboard. Hmmmm... you leave the door slightly open to avoid suffocation.", kControlParagraph);
		_vm->_dialogs->displayText(tmpStr);
		_vm->_avvysInTheCupboard = true;
		_vm->_background->draw(-1, -1, 7);
	}
}

/**
 * Returns true if you're within field "which".
 */
bool Animation::inField(byte which) {
	AnimationType *avvy = _sprites[0];

	FieldType *curField = &_vm->_fields[which];
	int16 yy = avvy->_y + avvy->_yLength;

	return (avvy->_x >= curField->_x1) && (avvy->_x <= curField->_x2) && (yy >= curField->_y1) && (yy <= curField->_y2);
}

/**
 * Returns True if you're near a door.
 */
bool Animation::nearDoor() {
	if (_vm->_fieldNum < 8)
		// there ARE no doors here!
		return false;

	AnimationType *avvy = _sprites[0];

	int16 ux = avvy->_x;
	int16 uy = avvy->_y + avvy->_yLength;

	for (int i = 8; i < _vm->_fieldNum; i++) {
		FieldType *curField = &_vm->_fields[i];
		if ((ux >= curField->_x1) && (ux <= curField->_x2) && (uy >= curField->_y1) && (uy <= curField->_y2))
			return true;
	}

	return false;
}

/**
 * @remarks	Originally called 'tripkey'
 */
void Animation::handleMoveKey(const Common::Event &event) {
	if (!_vm->_userMovesAvvy)
		return;

	if (_vm->_dropdown->_activeMenuItem._activeNow)
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

/**
* Draws a part of the lightning bolt for thunder().
* @remarks	Originally called 'zl'
*/
void Animation::drawLightning(int16 x1, int16 y1, int16 x2, int16 y2) {
	_vm->_graphics->drawLine(x1, y1 - 1, x2, y2 - 1, 1, 3, kColorBlue);
	_vm->_graphics->drawLine(x1, y1, x2, y2, 1, 1, kColorLightcyan);
}

/**
* Plays the actual thunder animation when Avvy (the player) swears too much.
* @remarks	Originally called 'zonk'
*/
void Animation::thunder() {
	_vm->_graphics->setBackgroundColor(kColorYellow);

	_vm->_graphics->saveScreen();

	int x = _vm->_animation->_sprites[0]->_x + _vm->_animation->_sprites[0]->_xLength / 2;
	int y = _vm->_animation->_sprites[0]->_y;

	for (int i = 0; i < 256; i++) {
		_vm->_sound->playNote(270 - i, 1);

		drawLightning(640, 0, 0, y / 4);
		drawLightning(0, y / 4, 640, y / 2);
		drawLightning(640, y / 2, x, y);
		_vm->_graphics->refreshScreen();

		_vm->_sound->playNote(2700 - 10 * i, 5);
		_vm->_system->delayMillis(5);
		_vm->_sound->playNote(270 - i, 1);

		_vm->_graphics->restoreScreen();
		_vm->_sound->playNote(2700 - 10 * i, 5);
		_vm->_system->delayMillis(5);
	}

	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();

	_vm->_graphics->setBackgroundColor(kColorBlack);
}

/**
* Makes the screen wobble.
*/
void Animation::wobble() {
	_vm->_graphics->saveScreen();

	for (int i = 0; i < 26; i++) {
		_vm->_graphics->shiftScreen();
		_vm->_graphics->refreshScreen();
		_vm->_system->delayMillis(i * 7);

		_vm->_graphics->restoreScreen();
		_vm->_system->delayMillis(i * 7);
	}

	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
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

void Animation::setAvvyClothes(int id) {
	AnimationType *spr = _sprites[0];
	if (spr->_id == id)
		return;

	int16 x = spr->_x;
	int16 y = spr->_y;
	spr->remove();
	spr->init(id, true);
	spr->appear(x, y, kDirLeft);
	spr->_visible = false;
}

int Animation::getAvvyClothes() {
	return _sprites[0]->_id;
}

void Animation::resetVariables() {
	setDirection(kDirUp);
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
			if (_sprites[i]->_quick)
				spriteNum++;
		}
	}
	sz.syncAsByte(spriteNum);

	if (sz.isLoading()) {
		for (int i = 0; i < kSpriteNumbMax; i++) { // Deallocate sprites.
			AnimationType *spr = _sprites[i];
			if (spr->_quick)
				spr->remove();
		}
	}

	for (int i = 0; i < spriteNum; i++) {
		AnimationType *spr = _sprites[i];
		sz.syncAsByte(spr->_id);
		sz.syncAsByte(spr->_doCheck);

		if (sz.isLoading()) {
			spr->_quick = true;
			spr->init(spr->_id, spr->_doCheck);
		}

		sz.syncAsByte(spr->_moveX);
		sz.syncAsByte(spr->_moveY);
		sz.syncAsByte(spr->_facingDir);
		sz.syncAsByte(spr->_stepNum);
		sz.syncAsByte(spr->_visible);
		sz.syncAsByte(spr->_homing);
		sz.syncAsByte(spr->_count);
		sz.syncAsByte(spr->_speedX);
		sz.syncAsByte(spr->_speedY);
		sz.syncAsByte(spr->_frameNum);
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
