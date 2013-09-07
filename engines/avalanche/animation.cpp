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
#include "avalanche/scrolls2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/visa2.h"
#include "avalanche/gyro2.h"
#include "avalanche/celer2.h"
#include "avalanche/sequence2.h"
#include "avalanche/timeout2.h"
#include "avalanche/enid2.h"

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/file.h"

namespace Avalanche {

void AnimationType::init(byte spritenum, bool do_check, Animation *tr) {
	_tr = tr;

	const int32 idshould = -1317732048;
	byte fv;
	int32 id;
	Common::File inf;

	if (spritenum == 177)
		return; // Already running!

	Common::String filename;
	filename = filename.format("sprite%d.avd", spritenum);
	if (!inf.open(filename)) {
		warning("AVALANCHE: Trip: File not found: %s", filename.c_str());
		return;
	}

	inf.seek(177);

	id = inf.readSint32LE();
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
	_stat.seq = inf.readByte();
	_info._size = inf.readUint16LE();
	_stat._fgBubbleCol = inf.readByte();
	_stat._bgBubbleCol = inf.readByte();
	_stat.accinum = inf.readByte();

	totalnum = 0; // = 1;
	_info._xWidth = _info._xLength / 8;
	if ((_info._xLength % 8) > 0)
		_info._xWidth++;
	for (byte aa = 0; aa < _stat._frameNum; aa++) {

		_info._sil[totalnum] = new SilType[11 * (_info._yLength + 1)];
		//getmem(sil[totalnum-1], 11 * (a.yl + 1));
		_info._mani[totalnum] = new ManiType[_info._size - 6];
		//getmem(mani[totalnum-1], a.size - 6);
		for (fv = 0; fv <= _info._yLength; fv++)
			inf.read((*_info._sil[totalnum])[fv], _info._xWidth);
			//blockread(inf, (*sil[totalnum-1])[fv], xw);
		inf.read(*_info._mani[totalnum], _info._size - 6);
		//blockread(inf, *mani[totalnum-1], a.size - 6);

		totalnum++;
	}
	totalnum++;

	// on;
	_x = 0;
	_y = 0;
	quick = true;
	_visible = false;
	xs = 3;
	ys = 1;
	if (spritenum == 1)
		_tr->newspeed(); // Just for the lights.

	homing = false;
	ix = 0;
	iy = 0;
	step = 0;
	check_me = do_check;
	count = 0;
	whichsprite = spritenum;
	vanishifstill = false;
	call_eachstep = false;

	inf.close();
}

void AnimationType::original() {
	quick = false;
	whichsprite = 177;
}

void AnimationType::andexor() {
	if ((vanishifstill) && (ix == 0) && (iy == 0))
		return;
	byte picnum = face * _stat.seq + step; // There'll maybe problem because of the different array indexes in Pascal (starting from 1).

	_tr->_vm->_graphics->drawSprite(_info, picnum, _x, _y);
}

void AnimationType::turn(byte whichway) {
	if (whichway == 8)
		face = Animation::kDirUp;
	else
		face = whichway;
}

void AnimationType::appear(int16 wx, int16 wy, byte wf) {
	_x = (wx / 8) * 8;
	_y = wy;
	_oldX[_tr->_vm->_gyro->_cp] = wx;
	_oldY[_tr->_vm->_gyro->_cp] = wy;
	turn(wf);
	_visible = true;
	ix = 0;
	iy = 0;
}

/**
 * Check collision
 * @remarks	Originally called 'collision_check'
 */
bool AnimationType::checkCollision() {
	for (byte i = 0; i < _tr->kSpriteNumbMax; i++) {
		if (_tr->tr[i].quick && (_tr->tr[i].whichsprite != whichsprite) &&
			((_x + _info._xLength) > _tr->tr[i]._x) &&
			(_x < (_tr->tr[i]._x + _tr->tr[i]._info._xLength)) &&
			(_tr->tr[i]._y == _y))
				return true;
	}

	return false;
}

void AnimationType::walk() {
	byte tc;
	ByteField r;


	if (_visible) {
		r._x1 = (_x / 8) - 1;
		if (r._x1 == 255)
			r._x1 = 0;
		r._y1 = _y - 2;
		r._x2 = ((_x + _info._xLength) / 8) + 1;
		r._y2 = _y + _info._yLength + 2;

		_tr->getset[1 - _tr->_vm->_gyro->_cp].remember(r);
	}

	if (!_tr->_vm->_gyro->_doingSpriteRun) {
		_oldX[_tr->_vm->_gyro->_cp] = _x;
		_oldY[_tr->_vm->_gyro->_cp] = _y;
		if (homing)
			homestep();
		_x = _x + ix;
		_y = _y + iy;
	}

	if (check_me) {
		if (checkCollision()) {
			bounce();
			return;
		}

		tc = _tr->checkfeet(_x, _x + _info._xLength, _oldY[_tr->_vm->_gyro->_cp], _y, _info._yLength) - 1;
		// -1  is because the modified array indexes of magics[] compared to Pascal .

		if ((tc != 255) & (!_tr->_vm->_gyro->_doingSpriteRun)) {
			switch (_tr->_vm->_gyro->_magics[tc]._operation) {
			case Gyro::kMagicExclaim: {
				bounce();
				_tr->mustexclaim = true;
				_tr->saywhat = _tr->_vm->_gyro->_magics[tc]._data;
				}
				break;
			case Gyro::kMagicBounce:
				bounce();
				break;
			case Gyro::kMagicTransport:
				_tr->fliproom(_tr->_vm->_gyro->_magics[tc]._data >> 8, _tr->_vm->_gyro->_magics[tc]._data & 0xff);
				break;
			case Gyro::kMagicUnfinished: {
				bounce();
				_tr->_vm->_scrolls->displayText("\7Sorry.\3\rThis place is not available yet!");
				}
				break;
			case Gyro::kMagicSpecial:
				_tr->call_special(_tr->_vm->_gyro->_magics[tc]._data);
				break;
			case Gyro::kMagicOpenDoor:
				_tr->open_the_door(_tr->_vm->_gyro->_magics[tc]._data >> 8, _tr->_vm->_gyro->_magics[tc]._data & 0xff, tc);
				break;
			}
		}
	}

	if (!_tr->_vm->_gyro->_doingSpriteRun) {
		count++;
		if (((ix != 0) || (iy != 0)) && (count > 1)) {
			step++;
			if (step == _stat.seq)
				step = 0;
			count = 0;
		}
	}
}

void AnimationType::bounce() {
	_x = _oldX[_tr->_vm->_gyro->_cp];
	_y = _oldY[_tr->_vm->_gyro->_cp];
	if (check_me)
		_tr->stopwalking();
	else
		stopwalk();
	_tr->_vm->_gyro->_onCanDoPageSwap = false;
	_tr->_vm->_lucerna->drawDirection();
	_tr->_vm->_gyro->_onCanDoPageSwap = true;
}

int8 AnimationType::sgn(int16 val) {
	if (val > 0)
		return 1;
	else if (val < 0)
		return -1;
	else
		return 0;
}

void AnimationType::walkto(byte pednum) {
	pednum--; // Pascal -> C conversion: different array indexes.
	speed(sgn(_tr->_vm->_gyro->_peds[pednum]._x - _x) * 4, sgn(_tr->_vm->_gyro->_peds[pednum]._y - _y));
	hx = _tr->_vm->_gyro->_peds[pednum]._x - _info._xLength / 2;
	hy = _tr->_vm->_gyro->_peds[pednum]._y - _info._yLength;
	homing = true;
}

void AnimationType::stophoming() {
	homing = false;
}

void AnimationType::homestep() {
	int16 temp;

	if ((hx == _x) && (hy == _y)) {
		// touching the target
		stopwalk();
		return;
	}
	ix = 0;
	iy = 0;
	if (hy != _y) {
		temp = hy - _y;
		if (temp > 4)
			iy = 4;
		else if (temp < -4)
			iy = -4;
		else
			iy = temp;
	}
	if (hx != _x) {
		temp = hx - _x;
		if (temp > 4)
			ix = 4;
		else if (temp < -4)
			ix = -4;
		else
			ix = temp;
	}
}

void AnimationType::speed(int8 xx, int8 yy) {
	ix = xx;
	iy = yy;
	if ((ix == 0) && (iy == 0))
		return; // no movement
	if (ix == 0) {
		// No horz movement
		if (iy < 0)
			turn(_tr->kDirUp);
		else
			turn(_tr->kDirDown);
	} else {
		if (ix < 0)
			turn(_tr->kDirLeft);
		else
			turn(_tr->kDirRight);
	}
}

void AnimationType::stopwalk() {
	ix = 0;
	iy = 0;
	homing = false;
}

void AnimationType::chatter() {
	_tr->_vm->_gyro->_talkX = _x + _info._xLength / 2;
	_tr->_vm->_gyro->_talkY = _y;
	_tr->_vm->_gyro->_talkFontColor = _stat._fgBubbleCol;
	_tr->_vm->_gyro->_talkBackgroundColor = _stat._bgBubbleCol;
}

void AnimationType::set_up_saver(trip_saver_type &v) {
	v.whichsprite = whichsprite;
	v.face = face;
	v.step = step;
	v.x = _x;
	v.y = _y;
	v.ix = ix;
	v.iy = iy;
	v.visible = _visible;
	v.homing = homing;
	v.check_me = check_me;
	v.count = count;
	v.xw = _info._xWidth;
	v.xs = xs;
	v.ys = ys;
	v.totalnum = totalnum;
	v.hx = hx;
	v.hy = hy;
	v.call_eachstep = call_eachstep;
	v.eachstep = eachstep;
	v.vanishifstill = vanishifstill;
}

void AnimationType::unload_saver(trip_saver_type v) {
	whichsprite = v.whichsprite;
	face = v.face;
	step = v.step;
	_x = v.x;
	_y = v.y;
	ix = v.ix;
	iy = v.iy;
	_visible = v.visible;
	homing = v.homing;
	check_me = v.check_me;
	count = v.count;
	_info._xWidth = v.xw;
	xs = v.xs;
	ys = v.ys;
	totalnum = v.totalnum;
	hx = v.hx;
	hy = v.hy;
	call_eachstep = v.call_eachstep;
	eachstep = v.eachstep;
	vanishifstill = v.vanishifstill;
}

void AnimationType::savedata(Common::File &f) {
	warning("STUB: triptype::savedata()");
}

void AnimationType::loaddata(Common::File &f) {
	warning("STUB: triptype::loaddata()");
}

void AnimationType::save_data_to_mem(uint16 &where) {
	warning("STUB: triptype::save_data_to_mem()");
}

void AnimationType::load_data_from_mem(uint16 &where) {
	warning("STUB: triptype::load_data_from_mem()");
}

AnimationType *AnimationType::done() {
	Common::String xx;

	//  nds:=num div seq;
	totalnum--;
	_info._xWidth = _info._xLength / 8;
	if ((_info._xLength % 8) > 0)
		_info._xWidth++;
	for (byte aa = 0; aa < _stat._frameNum; aa++) {
		totalnum--;
		delete[] _info._mani[totalnum];
		delete[] _info._sil[totalnum];
	}

	quick = false;
	whichsprite = 177;
	return this;
}

getsettype *getsettype::init() {
	numleft = 0; // initialize array pointer
	return this;
}

void getsettype::remember(ByteField r) {
	numleft++;
	//if (numleft > maxgetset)
	//	error("Trip::remember() : runerr_Getset_Overflow");
	gs[numleft] = r;
}

void getsettype::recall(ByteField &r) {
	r = gs[numleft];
	numleft--;
}






Animation::Animation(AvalancheEngine *vm) {
	_vm = vm;

	getsetclear();
	mustexclaim = false;
}

Animation::~Animation() {
	for (byte i = 0; i < kSpriteNumbMax; i++) {
		if (tr[i].quick)
			tr[i].done();
	}
}

void Animation::loadtrip() {
	for (int16 gm = 0; gm < kSpriteNumbMax; gm++)
		tr[gm].original();

	for (uint16 i = 0; i < sizeof(aa); i++)
		aa[i] = 0;
}

byte Animation::checkfeet(int16 x1, int16 x2, int16 oy, int16 y, byte yl) {
	byte a, c;
	int16 fv, ff;

	// if not alive then begin checkfeet:=0; exit; end;
	a = 0;

	//setactivepage(2);
	if (x1 < 0)
		x1 = 0;
	if (x2 > 639)
		x2 = 639;
	if (oy < y) {
		for (fv = x1; fv <= x2; fv++) {
			for (ff = oy + yl; ff <= y + yl; ff++) {
				c = *(byte *)_vm->_graphics->_magics.getBasePtr(fv, ff);
				if (c > a)
					a = c;
			}
		}
	} else {
		for (fv = x1; fv <= x2; fv++) {
			for (ff = y + yl; ff <= oy + yl; ff++) {
				c = *(byte *)_vm->_graphics->_magics.getBasePtr(fv, ff);
				if (c > a)
					a = c;
			}
		}
	}

	//setactivepage(1 - cp);
	return a;
}

byte Animation::geida_ped(byte which) {
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

void Animation::catamove(byte ped) {
/* When you enter a new position in the catacombs, this procedure should
	be called. It changes the Also codes so that they may match the picture
	on the screen. (Coming soon: It draws up the screen, too.) */

	int32 here;
	uint16 xy_uint16;
	byte fv;

	// XY_uint16 is cat_x+cat_y*256. Thus, every room in the
	// catacombs has a different number for it.



	xy_uint16 = _vm->_gyro->_dna._catacombX + _vm->_gyro->_dna._catacombY * 256;
	_vm->_gyro->_dna._geidaSpin = 0;

	switch (xy_uint16) {
	case 1801: // Exit catacombs
		fliproom(r__lustiesroom, 4);
		_vm->_scrolls->displayText("Phew! Nice to be out of there!");
		return;
	case 1033: // Oubliette
		fliproom(r__oubliette, 1);
		_vm->_scrolls->displayText(Common::String("Oh, NO!") + _vm->_scrolls->kControlRegister + '1' + _vm->_scrolls->kControlSpeechBubble);
		return;
	case 4:
		fliproom(r__geidas, 1);
		return;
	case 2307:
		fliproom(r__lusties, 5);
		_vm->_scrolls->displayText("Oh no... here we go again...");
		_vm->_gyro->_dna._userMovesAvvy = false;
		tr[0].iy = 1;
		tr[0].ix = 0;
		return;
	}

	if (!_vm->_gyro->_dna._enterCatacombsFromLustiesRoom)
		_vm->_lucerna->loadRoom(29);
	here = _vm->_gyro->kCatacombMap[_vm->_gyro->_dna._catacombY - 1][_vm->_gyro->_dna._catacombX - 1];

	switch (here & 0xf) { // West.
	case 0: // no connection (wall)
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28);
		break;
	case 0x1: // no connection (wall + shield),
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 29); // ...shield.
		break;
	case 0x2: // wall with door
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door.
		break;
	case 0x3: // wall with door and shield
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 29); // ...shield.
		break;
	case 0x4: // no connection (wall + window),
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 5);  // ...window.
		break;
	case 0x5: // wall with door and window
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 5); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicNothing; // No door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 7); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[4]._operation = _vm->_gyro->kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 7); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicNothing; // Sloping wall.
		_vm->_gyro->_magics[2]._operation = _vm->_gyro->kMagicSpecial; // Straight wall.
		break;
	}

	/*  ---- */

	switch ((here & 0xf0) >> 4) { // East
	case 0: // no connection (wall)
		_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = _vm->_gyro->kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19);
		break;
	case 0x1: // no connection (wall + window),
		_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = _vm->_gyro->kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 20); // ...window.
		break;
	case 0x2: // wall with door
		_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = _vm->_gyro->kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); // ...door.
		break;
	case 0x3: // wall with door and window
		_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = _vm->_gyro->kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 20); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); // ...window.
		break;
	case 0x6: // no connection (wall + torches),
		_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = _vm->_gyro->kMagicNothing; // No door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 18); // ...torches.
		break;
	case 0x7: // wall with door and torches
		_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicBounce; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicNothing; // Straight wall.
		_vm->_gyro->_portals[6]._operation = _vm->_gyro->kMagicSpecial; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); // Wall, plus...
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); // ...door, and...
		_vm->_celer->drawBackgroundSprite(-1, -1, 18); // ...torches.
		break;
	case 0xf: // straight-through corridor.
		_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicNothing; // Sloping wall.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicSpecial; // Straight wall.
		_vm->_gyro->_portals[6]._operation = _vm->_gyro->kMagicNothing; // Door.
		break;
	}

	/*  ---- */

	switch ((here & 0xf00) >> 8) { // South
	case 0: // No connection.
		_vm->_gyro->_magics[6]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_magics[11]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_magics[12]._operation = _vm->_gyro->kMagicBounce;
		break;
	case 0x1:
		_vm->_celer->drawBackgroundSprite(-1, -1, 22);

		if ((xy_uint16 == 2051) && (_vm->_gyro->_dna._geidaFollows))
			_vm->_gyro->_magics[12]._operation = _vm->_gyro->kMagicExclaim;
		else
			_vm->_gyro->_magics[12]._operation = _vm->_gyro->kMagicSpecial; // Right exit south.

		_vm->_gyro->_magics[6]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_magics[11]._operation = _vm->_gyro->kMagicBounce;
		break;
	case 0x2:
		_vm->_celer->drawBackgroundSprite(-1, -1, 23);
		_vm->_gyro->_magics[6]._operation = _vm->_gyro->kMagicSpecial; // Middle exit south.
		_vm->_gyro->_magics[11]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_magics[12]._operation = _vm->_gyro->kMagicBounce;
		break;
	case 0x3:
		_vm->_celer->drawBackgroundSprite(-1, -1, 24);
		_vm->_gyro->_magics[11]._operation = _vm->_gyro->kMagicSpecial; // Left exit south.
		_vm->_gyro->_magics[6]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_magics[12]._operation = _vm->_gyro->kMagicBounce;
		break;
	}

	switch ((here & 0xf000) >> 12) { // North
	case 0: // No connection
		_vm->_gyro->_magics[0]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_portals[3]._operation = _vm->_gyro->kMagicNothing; // Door.
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
		_vm->_gyro->_magics[0]._operation = _vm->_gyro->kMagicBounce; // Middle exit north.
		_vm->_gyro->_portals[3]._operation = _vm->_gyro->kMagicSpecial; // Door.
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
		_vm->_gyro->_magics[0]._operation = _vm->_gyro->kMagicBounce; // Middle exit north.
		_vm->_gyro->_portals[3]._operation = _vm->_gyro->kMagicSpecial; // Door.
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

		_vm->_gyro->_magics[0]._operation = _vm->_gyro->kMagicSpecial; // Middle arch north.
		_vm->_gyro->_portals[3]._operation = _vm->_gyro->kMagicNothing; // Door.
	}
	break;
	// DECORATIONS:
	case 0xd: // No connection + WINDOW
		_vm->_gyro->_magics[0]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_portals[3]._operation = _vm->_gyro->kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 14);
		break;
	case 0xe: // No connection + TORCH
		_vm->_gyro->_magics[0]._operation = _vm->_gyro->kMagicBounce;
		_vm->_gyro->_portals[3]._operation = _vm->_gyro->kMagicNothing; // Door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 8);
		break;
	// Recessed door:
	case 0xf:
		_vm->_gyro->_magics[0]._operation = _vm->_gyro->kMagicNothing; // Door to Geida's room.
		_vm->_celer->drawBackgroundSprite(-1, -1, 1);
		_vm->_gyro->_portals[3]._operation = _vm->_gyro->kMagicSpecial; // Door.
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

	if ((_vm->_gyro->_dna._geidaFollows) && (ped > 0)) {
		if (!tr[1].quick)  // If we don't already have her...
			tr[1].init(5, true, this); // ...Load Geida.
		apped(2, geida_ped(ped));
		tr[1].call_eachstep = true;
		tr[1].eachstep = kProcGeida;
	}
}



// This proc gets called whenever you touch a line defined as _vm->_gyro->special.
void Animation::dawndelay() {
	_vm->_timeout->set_up_timer(2, _vm->_timeout->procdawn_delay, _vm->_timeout->reason_dawndelay);
}

void Animation::call_special(uint16 which) {
	switch (which) {
	case 1: // _vm->_gyro->special 1: Room 22: top of stairs.
		_vm->_celer->drawBackgroundSprite(-1, -1, 1);
		_vm->_gyro->_dna._brummieStairs = 1;
		_vm->_gyro->_magics[9]._operation = _vm->_gyro->kMagicNothing;
		_vm->_timeout->set_up_timer(10, _vm->_timeout->procstairs, _vm->_timeout->reason_brummiestairs);
		stopwalking();
		_vm->_gyro->_dna._userMovesAvvy = false;
		break;
	case 2: // _vm->_gyro->special 2: Room 22: bottom of stairs.
		_vm->_gyro->_dna._brummieStairs = 3;
		_vm->_gyro->_magics[10]._operation = _vm->_gyro->kMagicNothing;
		_vm->_gyro->_magics[11]._operation = _vm->_gyro->kMagicExclaim;
		_vm->_gyro->_magics[11]._data = 5;
		_vm->_gyro->_magics[3]._operation = _vm->_gyro->kMagicBounce; // Now works as planned!
		stopwalking();
		_vm->_visa->dixi('q', 26);
		_vm->_gyro->_dna._userMovesAvvy = true;
		break;
	case 3: // _vm->_gyro->special 3: Room 71: triggers dart.
		tr[0].bounce(); // Must include that.

		if (!_vm->_gyro->_dna._arrowTriggered) {
			_vm->_gyro->_dna._arrowTriggered = true;
			apped(2, 4); // The dart starts at ped 4, and...
			tr[1].walkto(5); // flies to ped 5.
			tr[1].face = kDirUp; // Only face.
			// Should call some kind of Eachstep procedure which will deallocate
			// the sprite when it hits the wall, and replace it with the chunk
			// graphic of the arrow buried in the plaster. */

			// OK!
			tr[1].call_eachstep = true;
			tr[1].eachstep = kProcArrow;
		}
		break;
	case 4: // This is the ghost room link.
		_vm->_lucerna->dusk();
		tr[0].turn(kDirRight); // you'll see this after we get back from bootstrap
		_vm->_timeout->set_up_timer(1, _vm->_timeout->procghost_room_phew, _vm->_timeout->reason_ghost_room_phew);
		_vm->_enid->backToBootstrap(3);
		break;
	case 5:
		if (_vm->_gyro->_dna._friarWillTieYouUp) {
			// _vm->_gyro->special 5: Room 42: touched tree, and get tied up.
			_vm->_gyro->_magics[4]._operation = _vm->_gyro->kMagicBounce; // Boundary effect is now working again.
			_vm->_visa->dixi('q', 35);
			tr[0].done();
			//tr[1].vanishifstill:=true;
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_visa->dixi('q', 36);
			_vm->_gyro->_dna._tiedUp = true;
			_vm->_gyro->_dna._friarWillTieYouUp = false;
			tr[1].walkto(3);
			tr[1].vanishifstill = true;
			tr[1].check_me = true; // One of them must have Check_Me switched on.
			_vm->_gyro->_whereIs[_vm->_gyro->kPeopleFriarTuck - 150] = 177; // Not here, then.
			_vm->_timeout->set_up_timer(364, _vm->_timeout->prochang_around, _vm->_timeout->reason_hanging_around);
		}
		break;
	case 6: // _vm->_gyro->special 6: fall down oubliette.
		_vm->_gyro->_dna._userMovesAvvy = false;
		tr[0].ix = 3;
		tr[0].iy = 0;
		tr[0].face = kDirRight;
		_vm->_timeout->set_up_timer(1, _vm->_timeout->procfall_down_oubliette, _vm->_timeout->reason_falling_down_oubliette);
		break;
	case 7: // _vm->_gyro->special 7: stop falling down oubliette.
		tr[0]._visible = false;
		_vm->_gyro->_magics[9]._operation = _vm->_gyro->kMagicNothing;
		stopwalking();
		_vm->_timeout->lose_timer(_vm->_timeout->reason_falling_down_oubliette);
		//_vm->_lucerna->mblit(12, 80, 38, 160, 3, 0);
		//_vm->_lucerna->mblit(12, 80, 38, 160, 3, 1);
		_vm->_scrolls->displayText("Oh dear, you seem to be down the bottom of an oubliette.");
		_vm->_timeout->set_up_timer(200, _vm->_timeout->procmeet_avaroid, _vm->_timeout->reason_meeting_avaroid);
		break;
	case 8:        // _vm->_gyro->special 8: leave du Lustie's room.
		if ((_vm->_gyro->_dna._geidaFollows) && (!_vm->_gyro->_dna._lustieIsAsleep)) {
			_vm->_visa->dixi('q', 63);
			tr[1].turn(kDirDown);
			tr[1].stopwalk();
			tr[1].call_eachstep = false; // Geida
			_vm->_lucerna->gameOver();
		}
		break;
	case 9: // _vm->_gyro->special 9: lose Geida to Robin Hood...
		if (!_vm->_gyro->_dna._geidaFollows)
			return;   // DOESN'T COUNT: no Geida.
		tr[1].call_eachstep = false; // She no longer follows Avvy around.
		tr[1].walkto(4); // She walks to somewhere...
		tr[0].done();     // Lose Avvy.
		_vm->_gyro->_dna._userMovesAvvy = false;
		_vm->_timeout->set_up_timer(40, _vm->_timeout->procrobin_hood_and_geida, _vm->_timeout->reason_robin_hood_and_geida);
		break;
	case 10: // _vm->_gyro->special 10: transfer north in catacombs.
		if ((_vm->_gyro->_dna._catacombX == 4) && (_vm->_gyro->_dna._catacombY == 1)) {
			// Into Geida's room.
			if (_vm->_gyro->_dna._objects[_vm->_gyro->kObjectKey - 1])
				_vm->_visa->dixi('q', 62);
			else {
				_vm->_visa->dixi('q', 61);
				return;
			}
		}
		_vm->_lucerna->dusk();
		_vm->_gyro->_dna._catacombY--;
		catamove(4);
		if (_vm->_gyro->_dna._room != r__catacombs)
			return;
		switch ((_vm->_gyro->kCatacombMap[_vm->_gyro->_dna._catacombY - 1][_vm->_gyro->_dna._catacombX - 1] & 0xf00) >> 8) {
		case 0x1:
			apped(1, 12);
			break;
		case 0x3:
			apped(1, 11);
			break;
		default:
			apped(1, 4);
		}
		getback();
		dawndelay();
		break;
	case 11: // _vm->_gyro->special 11: transfer east in catacombs.
		_vm->_lucerna->dusk();
		_vm->_gyro->_dna._catacombX++;
		catamove(1);
		if (_vm->_gyro->_dna._room != r__catacombs)
			return;
		apped(1, 1);
		getback();
		dawndelay();
		break;
	case 12: // _vm->_gyro->special 12: transfer south in catacombs.
		_vm->_lucerna->dusk();
		_vm->_gyro->_dna._catacombY += 1;
		catamove(2);
		if (_vm->_gyro->_dna._room != r__catacombs)
			return;
		apped(1, 2);
		getback();
		dawndelay();
		break;
	case 13: // _vm->_gyro->special 13: transfer west in catacombs.
		_vm->_lucerna->dusk();
		_vm->_gyro->_dna._catacombX--;
		catamove(3);
		if (_vm->_gyro->_dna._room != r__catacombs)
			return;
		apped(1, 3);
		getback();
		dawndelay();
		break;
	}
}



void Animation::open_the_door(byte whither, byte ped, byte magicnum) {
// This slides the door open. (The data really ought to be saved in
// the Also file, and will be next time. However, for now, they're
// here.)
	switch (_vm->_gyro->_dna._room) {
	case r__outsideyours:
	case r__outsidenottspub:
	case r__outsideducks:
		_vm->_sequence->first_show(1);
		_vm->_sequence->then_show(2);
		_vm->_sequence->then_show(3);
		break;
	case r__insidecardiffcastle:
		_vm->_sequence->first_show(1);
		_vm->_sequence->then_show(5);
		break;
	case r__avvysgarden:
	case r__entrancehall:
	case r__insideabbey:
	case r__yourhall:
		_vm->_sequence->first_show(1);
		_vm->_sequence->then_show(2);
		break;
	case r__musicroom:
	case r__outsideargentpub:
		_vm->_sequence->first_show(5);
		_vm->_sequence->then_show(6);
		break;
	case r__lusties:
		switch (magicnum) {
		case 14:
			if (_vm->_gyro->_dna._avvysInTheCupboard) {
				hide_in_the_cupboard();
				_vm->_sequence->first_show(8);
				_vm->_sequence->then_show(7);
				_vm->_sequence->start_to_close();
				return;
			} else {
				apped(1, 6);
				tr[0].face = kDirRight; // added by TT 12/3/1995
				_vm->_sequence->first_show(8);
				_vm->_sequence->then_show(9);
			}
			break;
		case 12:
			_vm->_sequence->first_show(4);
			_vm->_sequence->then_show(5);
			_vm->_sequence->then_show(6);
			break;
		}
		break;
	}

	_vm->_sequence->then_flip(whither, ped);
	_vm->_sequence->start_to_open();
}

void Animation::newspeed() {
	// Given that you've just changed the speed in triptype.xs, this adjusts ix.
	const ByteField lightspace = {40, 199, 47, 199};
	byte page_;

	tr[0].ix = (tr[0].ix / 3) * tr[0].xs;

	//setactivepage(3);

	if (tr[0].xs == _vm->_gyro->kRun)
		_vm->_graphics->_surface.drawLine(371, 199, 373, 199, kColorYellow);
	else
		_vm->_graphics->_surface.drawLine(336, 199, 338, 199, kColorYellow);

	if (tr[0].xs == _vm->_gyro->kRun)
		_vm->_graphics->_surface.drawLine(336, 199, 338, 199, kColorLightblue);
	else
		_vm->_graphics->_surface.drawLine(371, 199, 373, 199, kColorLightblue);

	//setactivepage(1 - cp);

	for (page_ = 0; page_ <= 1; page_++)
		getset[page_].remember(lightspace);

}

void Animation::rwsp(byte t, byte dir) {
	switch (dir) {
	case kDirUp:
		tr[t].speed(0, -tr[t].ys);
		break;
	case kDirDown:
		tr[t].speed(0, tr[t].ys);
		break;
	case kDirLeft:
		tr[t].speed(-tr[t].xs,  0);
		break;
	case kDirRight:
		tr[t].speed(tr[t].xs,  0);
		break;
	case kDirUpLeft:
		tr[t].speed(-tr[t].xs, -tr[t].ys);
		break;
	case kDirUpRight:
		tr[t].speed(tr[t].xs, -tr[t].ys);
		break;
	case kDirDownLeft:
		tr[t].speed(-tr[t].xs, tr[t].ys);
		break;
	case kDirDownRight:
		tr[t].speed(tr[t].xs, tr[t].ys);
		break;
	}
}

void Animation::apped(byte trn, byte np) {
	trn--;
	np--;
	tr[trn].appear(_vm->_gyro->_peds[np]._x - tr[trn]._info._xLength / 2, _vm->_gyro->_peds[np]._y - tr[trn]._info._yLength, _vm->_gyro->_peds[np]._direction);
	rwsp(trn, _vm->_gyro->_peds[np]._direction);
}

#if 0
   function overlap(x1,y1,x2,y2,x3,y3,x4,y4:uint16):bool;
	begin // By De Morgan's law:
	overlap:=(x2>=x3) and (x4>=x1) and (y2>=y3) and (y4>=y1);
	end;
#endif
	//x1,x2 - as _vm->_gyro->bytefield, but *8. y1,y2 - as _vm->_gyro->bytefield.
	//x3,y3 = mx,my. x4,y4 = mx+16,my+16.

void Animation::getback() {
	// Super_Off;
#if 0
	while (getset[1 - _vm->_gyro->cp].numleft > 0) {
		getset[1 - _vm->_gyro->cp].recall(r);


		bool endangered = false;
		if overlaps_with_mouse and not endangered then
			begin
				endangered:=true;
				blitfix;
				Super_Off;
			end;

		//_vm->_lucerna->mblit(r.x1, r.y1, r.x2, r.y2, 3, 1 - _vm->_gyro->cp);
	}

	if endangered then
		Super_On;
#endif
}

// Eachstep procedures:
void Animation::follow_avvy_y(byte tripnum) {
	if (tr[0].face == kDirLeft)
		return;
	if (tr[tripnum].homing)
		tr[tripnum].hy = tr[1]._y;
	else {
		if (tr[tripnum]._y < tr[1]._y)
			tr[tripnum]._y += 1;
		else if (tr[tripnum]._y > tr[1]._y)
			tr[tripnum]._y -= 1;
		else
			return;
		if (tr[tripnum].ix == 0)  {
			tr[tripnum].step += 1;
			if (tr[tripnum].step == tr[tripnum]._stat.seq)
				tr[tripnum].step = 0;
			tr[tripnum].count = 0;
		}
	}
}

void Animation::back_and_forth(byte tripnum) {
	if (!tr[tripnum].homing) {
		if (tr[tripnum].face == kDirRight)
			tr[tripnum].walkto(4);
		else
			tr[tripnum].walkto(5);
	}
}

void Animation::face_avvy(byte tripnum) {
	if (!tr[tripnum].homing) {
		if (tr[0]._x >= tr[tripnum]._x)
			tr[tripnum].face = kDirRight;
		else
			tr[tripnum].face = kDirLeft;
	}
}

void Animation::arrow_procs(byte tripnum) {
	if (tr[tripnum].homing) {
		// Arrow is still in flight.
		// We must check whether or not the arrow has collided tr[tripnum] Avvy's head.
		// This is so if: a) the bottom of the arrow is below Avvy's head,
		// b) the left of the arrow is left of the right of Avvy's head, and
		// c) the right of the arrow is right of the left of Avvy's head.
		if (((tr[tripnum]._y + tr[tripnum]._info._yLength) >= tr[0]._y) // A
				&& (tr[tripnum]._x <= (tr[0]._x + tr[0]._info._xLength)) // B
				&& ((tr[tripnum]._x + tr[tripnum]._info._xLength) >= tr[0]._x)) { // C
			// OK, it's hit him... what now?

			tr[1].call_eachstep = false; // prevent recursion.
			_vm->_visa->dixi('Q', 47); // Complaint!
			tr[tripnum].done(); // Deallocate the arrow.
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

			_vm->_gyro->_dna._userMovesAvvy = false; // Stop the user from moving him.
			_vm->_timeout->set_up_timer(55, _vm->_timeout->procnaughty_duke, _vm->_timeout->reason_naughty_duke);
		}
	} else { // Arrow has hit the wall!
		tr[tripnum].done(); // Deallocate the arrow.
		_vm->_celer->drawBackgroundSprite(-1, -1, 3); // Show pic of arrow stuck into the door.
		_vm->_gyro->_dna._arrowInTheDoor = true; // So that we can pick it up.
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

void Animation::grab_avvy(byte tripnum) {     // For Friar Tuck, in Nottingham.
	int16 tox = tr[0]._x + 17;
	int16 toy = tr[0]._y - 1;
	if ((tr[tripnum]._x == tox) && (tr[tripnum]._y == toy)) {
		tr[tripnum].call_eachstep = false;
		tr[tripnum].face = kDirLeft;
		tr[tripnum].stopwalk();
		// ... whatever ...
	} else {
		// Still some way to go.
		if (tr[tripnum]._x < tox) {
			tr[tripnum]._x += 5;
			if (tr[tripnum]._x > tox)
				tr[tripnum]._x = tox;
		}
		if (tr[tripnum]._y < toy)
			tr[tripnum]._y++;
		tr[tripnum].step++;
		if (tr[tripnum].step == tr[tripnum]._stat.seq)
			tr[tripnum].step = 0;
	}
}

void Animation::take_a_step(byte &tripnum) {
	if (tr[tripnum].ix == 0) {
		tr[tripnum].step++;
		if (tr[tripnum].step == tr[tripnum]._stat.seq)
			tr[tripnum].step = 0;
		tr[tripnum].count = 0;
	}
}

void Animation::spin(byte whichway, byte &tripnum) {
	if (tr[tripnum].face != whichway) {
		tr[tripnum].face = whichway;
		if (tr[tripnum].whichsprite == 2)
			return; // Not for Spludwick

		_vm->_gyro->_dna._geidaSpin += 1;
		_vm->_gyro->_dna._geidaTime = 20;
		if (_vm->_gyro->_dna._geidaSpin == 5) {
			_vm->_scrolls->displayText("Steady on, Avvy, you'll make the poor girl dizzy!");
			_vm->_gyro->_dna._geidaSpin = 0;
			_vm->_gyro->_dna._geidaTime = 0; // knock out records
		}
	}
}

void Animation::geida_procs(byte tripnum) {
	if (_vm->_gyro->_dna._geidaTime > 0) {
		_vm->_gyro->_dna._geidaTime--;
		if (_vm->_gyro->_dna._geidaTime == 0)
			_vm->_gyro->_dna._geidaSpin = 0;
	}

	if (tr[tripnum]._y < (tr[0]._y - 2)) {
		// Geida is further from the screen than Avvy.
		spin(kDirDown, tripnum);
		tr[tripnum].iy = 1;
		tr[tripnum].ix = 0;
		take_a_step(tripnum);
		return;
	} else if (tr[tripnum]._y > (tr[0]._y + 2)) {
		// Avvy is further from the screen than Geida.
		spin(kDirUp, tripnum);
		tr[tripnum].iy = -1;
		tr[tripnum].ix = 0;
		take_a_step(tripnum);
		return;
	}

	tr[tripnum].iy = 0;
	// These 12-s are not in the original, I added them to make the following method more "smooth".
	// Now the NPC which is following Avvy won't block his way and will walk next to him properly.
	if (tr[tripnum]._x < tr[0]._x - tr[0].xs * 8 - 12) {
		tr[tripnum].ix = tr[0].xs;
		spin(kDirRight, tripnum);
		take_a_step(tripnum);
	} else if (tr[tripnum]._x > tr[0]._x + tr[0].xs * 8 + 12) {
		tr[tripnum].ix = -tr[0].xs;
		spin(kDirLeft, tripnum);
		take_a_step(tripnum);
	} else
		tr[tripnum].ix = 0;
}

// That's all...

void Animation::call_andexors() {
	int8 order[5];
	byte fv, temp;
	bool ok;

	for (int i = 0; i < 5; i++)
		order[i] = -1;

	for (fv = 0; fv < kSpriteNumbMax; fv++) {
		if (tr[fv].quick && tr[fv]._visible)
			order[fv] = fv;
	}

	do {
		ok = true;
		for (fv = 0; fv < 4; fv++) {
			if (((order[fv] != -1) && (order[fv + 1] != -1))
					&& (tr[order[fv]]._y > tr[order[fv + 1]]._y)) {
				// Swap them!
				temp = order[fv];
				order[fv] = order[fv + 1];
				order[fv + 1] = temp;
				ok = false;
			}
		}
	} while (!ok);


	_vm->_graphics->refreshBackground();

	for (fv = 0; fv < 5; fv++) {
		if (order[fv] > -1)
			tr[order[fv]].andexor();
	}
}

/**
 * Animation links
 * @remarks	Originally called 'trippancy_link'
 */
void Animation::animLink() {
	byte fv;

	if (_vm->_gyro->_dropdownActive | _vm->_gyro->_onToolbar | _vm->_gyro->_seeScroll)
		return;
	for (fv = 0; fv < kSpriteNumbMax; fv++) {
		if (tr[fv].quick && tr[fv]._visible)
			tr[fv].walk();
	}

	call_andexors();

	for (fv = 0; fv < kSpriteNumbMax; fv++) {
		if (tr[fv].quick && tr[fv].call_eachstep) {
			switch (tr[fv].eachstep) {
			case kProcFollowAvvyY :
				follow_avvy_y(fv);
				break;
			case kProcBackAndForth :
				back_and_forth(fv);
				break;
			case kProcFaceAvvy :
				face_avvy(fv);
				break;
			case kProcArrow :
				arrow_procs(fv);
				break;
				//    PROCSpludwick_procs : spludwick_procs(fv);
			case kProcGrabAvvy :
				grab_avvy(fv);
				break;
			case kProcGeida :
				geida_procs(fv);
				break;
			}
		}
	}

	if (mustexclaim) {
		mustexclaim = false;
		_vm->_visa->dixi('x', saywhat);
	}
}

void Animation::get_back_loretta() {
	byte fv;

	for (fv = 0; fv < kSpriteNumbMax; fv++) {
		if (tr[fv].quick) {
			getback();
			return;
		}
	}
	// for fv:=0 to 1 do begin cp:=1-cp; getback; end;
}

void Animation::stopwalking() {
	tr[0].stopwalk();
	_vm->_gyro->_dna._direction = kDirStopped;
	if (_vm->_gyro->_alive)
		tr[0].step = 1;
}

void Animation::tripkey(char dir) {
	warning("Replaced by Trip::handleMoveKey!");
}

void Animation::readstick() {
	warning("STUB: Trip::readstick()");
}

void Animation::getsetclear() {
	for (byte fv = 0; fv <= 1; fv++)
		getset[fv].init();
}

void Animation::hide_in_the_cupboard() {
	if (_vm->_gyro->_dna._avvysInTheCupboard) {
		if (_vm->_gyro->_dna._wearing == Acci::kNothing)
			_vm->_scrolls->displayText(Common::String(_vm->_scrolls->kControlItalic) + "AVVY!" + _vm->_scrolls->kControlRoman + "Get dressed first!");
		else {
			tr[0]._visible = true;
			_vm->_gyro->_dna._userMovesAvvy = true;
			apped(1, 3); // Walk out of the cupboard.
			_vm->_scrolls->displayText("You leave the cupboard. Nice to be out of there!");
			_vm->_gyro->_dna._avvysInTheCupboard = false;
			_vm->_sequence->first_show(8);
			_vm->_sequence->then_show(7);
			_vm->_sequence->start_to_close();
		}
	} else {
		// Not hiding in the cupboard
		tr[0]._visible = false;
		_vm->_gyro->_dna._userMovesAvvy = false;
		_vm->_scrolls->displayText(Common::String("You walk into the room...") + _vm->_scrolls->kControlParagraph
			+ "It seems to be an empty, but dusty, cupboard. Hmmmm... you leave the door slightly open to avoid suffocation.");
		_vm->_gyro->_dna._avvysInTheCupboard = true;
		_vm->_celer->drawBackgroundSprite(-1, -1, 8);
	}
}

void Animation::fliproom(byte room, byte ped) {
	byte fv;

	if (!_vm->_gyro->_alive) {
		// You can't leave the room if you're dead.
		tr[0].ix = 0;
		tr[0].iy = 0; // Stop him from moving.
		return;
	}

	if ((room == 177) && (_vm->_gyro->_dna._room == r__lusties)) {
		hide_in_the_cupboard();
		return;
	}

	if ((_vm->_gyro->_dna._jumpStatus > 0) && (_vm->_gyro->_dna._room == r__insidecardiffcastle)) {
		// You can't *jump* out of Cardiff Castle!
		tr[0].ix = 0;
		return;
	}

	_vm->_lucerna->exitRoom(_vm->_gyro->_dna._room);
	_vm->_lucerna->dusk();
	getsetclear();


	for (fv = 1; fv < kSpriteNumbMax; fv++) {
		if (tr[fv].quick)
			tr[fv].done();
	} // Deallocate sprite

	if (_vm->_gyro->_dna._room == r__lustiesroom)
		_vm->_gyro->_dna._enterCatacombsFromLustiesRoom = true;

	_vm->_lucerna->enterRoom(room, ped);
	apped(1, ped);
	_vm->_gyro->_dna._enterCatacombsFromLustiesRoom = false;
	_vm->_gyro->_oldDirection = _vm->_gyro->_dna._direction;
	_vm->_gyro->_dna._direction = tr[0].face;
	_vm->_lucerna->drawDirection();

	for (fv = 0; fv <= 1; fv++) {
		_vm->_gyro->_cp = 1 - _vm->_gyro->_cp;
		getback();
	}
	_vm->_lucerna->dawn();

	// Tidy up after mouse. I know it's a kludge...
	//  tidy_after_mouse;
}

bool Animation::infield(byte which) {
	which--; // Pascal -> C: different array indexes.

	int16 yy = tr[0]._y + tr[0]._info._yLength;

	return (tr[0]._x >= _vm->_gyro->_fields[which]._x1) && (tr[0]._x <= _vm->_gyro->_fields[which]._x2)
		&& (yy >= _vm->_gyro->_fields[which]._y1) && (yy <= _vm->_gyro->_fields[which]._y2);

}

bool Animation::neardoor() {
	if (_vm->_gyro->_fieldNum < 8) {
		// there ARE no doors here!
		return false;
	}

	int16 ux = tr[0]._x;
	int16 uy = tr[0]._y + tr[0]._info._yLength;
	bool nd = false;
	for (byte fv = 8; fv < _vm->_gyro->_fieldNum; fv++)
		if ((ux >= _vm->_gyro->_fields[fv]._x1) && (ux <= _vm->_gyro->_fields[fv]._x2)
			&& (uy >= _vm->_gyro->_fields[fv]._y1) && (uy <= _vm->_gyro->_fields[fv]._y2))
			nd = true;
	return nd;
}

void Animation::new_game_for_trippancy() {   // Called by gyro.newgame
	tr[0]._visible = false;
}

void Animation::handleMoveKey(const Common::Event &event) {
	if (!_vm->_gyro->_dna._userMovesAvvy)
		return;

	if (_vm->_dropdown->_activeMenuItem._activeNow)
		_vm->_parser->tryDropdown();
	else {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
			if (_vm->_gyro->_dna._direction != kDirUp) {
				_vm->_gyro->_dna._direction = kDirUp;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_DOWN:
			if (_vm->_gyro->_dna._direction != kDirDown) {
				_vm->_gyro->_dna._direction = kDirDown;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_LEFT:
			if (_vm->_gyro->_dna._direction != kDirLeft) {
				_vm->_gyro->_dna._direction = kDirLeft;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_RIGHT:
			if (_vm->_gyro->_dna._direction != kDirRight) {
				_vm->_gyro->_dna._direction = kDirRight;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_PAGEUP:
			if (_vm->_gyro->_dna._direction != kDirUpRight) {
				_vm->_gyro->_dna._direction = kDirUpRight;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_PAGEDOWN:
			if (_vm->_gyro->_dna._direction != kDirDownRight) {
				_vm->_gyro->_dna._direction = kDirDownRight;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_END:
			if (_vm->_gyro->_dna._direction != kDirDownLeft) {
				_vm->_gyro->_dna._direction = kDirDownLeft;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_HOME:
			if (_vm->_gyro->_dna._direction != kDirUpLeft) {
				_vm->_gyro->_dna._direction = kDirUpLeft;
				rwsp(0, _vm->_gyro->_dna._direction);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_KP5:
			stopwalking();
			break;
		default:
			break;
		}
	}
}


} // End of namespace Avalanche.
