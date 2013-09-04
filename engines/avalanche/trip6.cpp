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

#include "avalanche/trip6.h"
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

/*#include "Dropdown.h"*/


namespace Avalanche {

	
void triptype::init(byte spritenum, bool do_check, Trip *tr) {
	_tr = tr;

	const int32 idshould = -1317732048;
	int16 gd, gm;
	byte fv/*,nds*/;
	int32 id;
	Common::File inf;

	if (spritenum == 177)
		return; /* Already running! */

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
	
	if (!a.name.empty())
		a.name.clear();
	byte nameSize = inf.readByte();
	for (byte i = 0; i < nameSize; i++) 
		a.name += inf.readByte();
	inf.skip(12 - nameSize);
	
	//inf.skip(1); // Same as above.
	byte commentSize = inf.readByte();
	for (byte i = 0; i < commentSize; i++)
		a.comment += inf.readByte();
	inf.skip(16 - commentSize);

	a.num = inf.readByte();
	_info.xl = inf.readByte();
	_info.yl = inf.readByte();
	a.seq = inf.readByte();
	_info.size = inf.readUint16LE();
	a.fgc = inf.readByte();
	a.bgc = inf.readByte();
	a.accinum = inf.readByte();

	totalnum = 0; // = 1;
	_info.xw = _info.xl / 8;
	if ((_info.xl % 8) > 0)
		_info.xw++;
	for (byte aa = 0; aa < /*nds*seq*/a.num; aa++) {

		_info.sil[totalnum] = new siltype[11 * (_info.yl + 1)];
		//getmem(sil[totalnum-1], 11 * (a.yl + 1));
		_info.mani[totalnum] = new manitype[_info.size - 6];
		//getmem(mani[totalnum-1], a.size - 6);
		for (fv = 0; fv <= _info.yl; fv ++)
			inf.read((*_info.sil[totalnum])[fv], _info.xw);
			//blockread(inf, (*sil[totalnum-1])[fv], xw);
		inf.read(*_info.mani[totalnum], _info.size - 6);
		//blockread(inf, *mani[totalnum-1], a.size - 6);

		totalnum++;
	}
	totalnum++;

	/* on; */
	x = 0;
	y = 0;
	quick = true;
	visible = false;
	xs = 3;
	ys = 1;
	if (spritenum == 1)
		_tr->newspeed(); /* Just for the lights. */

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

void triptype::original() {
	quick = false;
	whichsprite = 177;
}

void triptype::andexor() {
	if ((vanishifstill) && (ix == 0) && (iy == 0))
		return;
	byte picnum = face * a.seq + step; // There'll maybe problem because of the different array indexes in Pascal (starting from 1). 

	_tr->_vm->_graphics->drawSprite(_info, picnum, x, y);	
}

void triptype::turn(byte whichway) {
	if (whichway == 8)
		face = 0;
	else
		face = whichway;
}

void triptype::appear(int16 wx, int16 wy, byte wf) {
	x = (wx / 8) * 8;
	y = wy;
	ox[_tr->_vm->_gyro->cp] = wx;
	oy[_tr->_vm->_gyro->cp] = wy;
	turn(wf);
	visible = true;
	ix = 0;
	iy = 0;
}

bool triptype::collision_check() {
	for (byte fv = 0; fv < _tr->numtr; fv++) 
		if (_tr->tr[fv].quick && (_tr->tr[fv].whichsprite != whichsprite) &&
			((x + _info.xl) > _tr->tr[fv].x) &&
			(x < (_tr->tr[fv].x + _tr->tr[fv]._info.xl)) &&
			(_tr->tr[fv].y == y)) 
				return true;

	return false;
}

void triptype::walk() {
	byte tc;
	bytefield r;


	if (visible) {
		r.x1 = (x / 8) - 1;
		if (r.x1 == 255)
			r.x1 = 0;
		r.y1 = y - 2;
		r.x2 = ((x + _info.xl) / 8) + 1;
		r.y2 = y + _info.yl + 2;
		
		_tr->getset[1 - _tr->_vm->_gyro->cp].remember(r);
	}

	if (!_tr->_vm->_gyro->doing_sprite_run) {
		ox[_tr->_vm->_gyro->cp] = x;
		oy[_tr->_vm->_gyro->cp] = y;
		if (homing)
			homestep();
		x = x + ix;
		y = y + iy;
	}

	if (check_me) {
		if (collision_check()) {
			bounce();
			return;
		}

		tc = _tr->checkfeet(x, x + _info.xl, oy[_tr->_vm->_gyro->cp], y, _info.yl) - 1;
		// -1  is becouse the modified array indexes of magics[] compared to Pascal .

		if ((tc != 255) & (!_tr->_vm->_gyro->doing_sprite_run)) {
			switch (_tr->_vm->_gyro->magics[tc].op) {
			case Gyro::exclaim: {
				bounce();
				_tr->mustexclaim = true;
				_tr->saywhat = _tr->_vm->_gyro->magics[tc].data;
				}
				break;
			case Gyro::bounces:
				bounce();
				break;
			case Gyro::transport:
				_tr->fliproom(_tr->_vm->_gyro->magics[tc].data >> 8, _tr->_vm->_gyro->magics[tc].data & 0xff);
				break;
			case Gyro::unfinished: {
				bounce();
				_tr->_vm->_scrolls->display("\7Sorry.\3\rThis place is not available yet!");
				}
				break;
			case Gyro::special:
				_tr->call_special(_tr->_vm->_gyro->magics[tc].data);
				break;
			case Gyro::mopendoor:
				_tr->open_the_door(_tr->_vm->_gyro->magics[tc].data >> 8, _tr->_vm->_gyro->magics[tc].data & 0xff, tc);
				break;
			}
		}
	}

	if (!_tr->_vm->_gyro->doing_sprite_run) {
		count++;
		if (((ix != 0) || (iy != 0)) && (count > 1)) {
			step++;
			if (step == a.seq)
				step = 0;
			count = 0;
		}
	}
}

void triptype::bounce() {
	x = ox[_tr->_vm->_gyro->cp];
	y = oy[_tr->_vm->_gyro->cp];
	if (check_me)
		_tr->stopwalking();
	else
		stopwalk();
	_tr->_vm->_gyro->oncandopageswap = false;
	_tr->_vm->_lucerna->showrw();
	_tr->_vm->_gyro->oncandopageswap = true;
}

int8 triptype::sgn(int16 x) {
	if (x > 0)
		return 1;
	else if (x < 0)
		return -1;
	else
		return 0;
}

void triptype::walkto(byte pednum) {
	pednum--; // Pascal -> C conversion: different array indexes.
	speed(sgn(_tr->_vm->_gyro->peds[pednum].x - x) * 4, sgn(_tr->_vm->_gyro->peds[pednum].y - y));
	hx = _tr->_vm->_gyro->peds[pednum].x - _info.xl / 2;
	hy = _tr->_vm->_gyro->peds[pednum].y - _info.yl;
	homing = true;
}

void triptype::stophoming() {
	homing = false;
}

void triptype::homestep() {
	int16 temp;

	if ((hx == x) && (hy == y)) {
		/* touching the target */
		stopwalk();
		return;
	}
	ix = 0;
	iy = 0;
	if (hy != y) {
		temp = hy - y;
		if (temp > 4)
			iy = 4;
		else if (temp < -4)
			iy = -4;
		else
			iy = temp;
	}
	if (hx != x) {
		temp = hx - x;
		if (temp > 4)
			ix = 4;
		else if (temp < -4)
			ix = -4;
		else
			ix = temp;
	}
}

void triptype::speed(int8 xx, int8 yy) {
	ix = xx;
	iy = yy;
	if ((ix == 0) && (iy == 0))
		return; /* no movement */
	if (ix == 0) {
		/* No horz movement */
		if (iy < 0)
			turn(_tr->up);
		else
			turn(_tr->down);
	} else {
		if (ix < 0)
			turn(_tr->left);
		else
			turn(_tr->right);
	}
}

void triptype::stopwalk() {
	ix = 0;
	iy = 0;
	homing = false;
}

void triptype::chatter() {
	_tr->_vm->_gyro->talkx = x + _info.xl / 2;
	_tr->_vm->_gyro->talky = y;
	_tr->_vm->_gyro->talkf = a.fgc;
	_tr->_vm->_gyro->talkb = a.bgc;
}

void triptype::set_up_saver(trip_saver_type &v) {
	v.whichsprite = whichsprite;
	v.face = face;
	v.step = step;
	v.x = x;
	v.y = y;
	v.ix = ix;
	v.iy = iy;
	v.visible = visible;
	v.homing = homing;
	v.check_me = check_me;
	v.count = count;
	v.xw = _info.xw;
	v.xs = xs;
	v.ys = ys;
	v.totalnum = totalnum;
	v.hx = hx;
	v.hy = hy;
	v.call_eachstep = call_eachstep;
	v.eachstep = eachstep;
	v.vanishifstill = vanishifstill;
}

void triptype::unload_saver(trip_saver_type v) {
	whichsprite = v.whichsprite;
	face = v.face;
	step = v.step;
	x = v.x;
	y = v.y;
	ix = v.ix;
	iy = v.iy;
	visible = v.visible;
	homing = v.homing;
	check_me = v.check_me;
	count = v.count;
	_info.xw = v.xw;
	xs = v.xs;
	ys = v.ys;
	totalnum = v.totalnum;
	hx = v.hx;
	hy = v.hy;
	call_eachstep = v.call_eachstep;
	eachstep = v.eachstep;
	vanishifstill = v.vanishifstill;
}

void triptype::savedata(Common::File &f) {
	warning("STUB: triptype::savedata()");
}

void triptype::loaddata(Common::File &f) {
	warning("STUB: triptype::loaddata()");
}

void triptype::save_data_to_mem(uint16 &where) {
	warning("STUB: triptype::save_data_to_mem()");
}

void triptype::load_data_from_mem(uint16 &where) {
	warning("STUB: triptype::load_data_from_mem()");
}

triptype *triptype::done() {
	int16 gd, gm;
	Common::String xx;
	byte fv/*,nds*/;
	byte aa, bb;
	int32 id;
	uint16 soa;

	/*  nds:=num div seq;*/
	totalnum--;
	_info.xw = _info.xl / 8;
	if ((_info.xl % 8) > 0)
		_info.xw++;
	for (aa = 0; aa < /*nds*seq*/ a.num; aa++) {
		totalnum--;
		delete _info.mani[totalnum];
		delete _info.sil[totalnum];
	}

	quick = false;
	whichsprite = 177;
	return this;
}






getsettype *getsettype::init() {
	numleft = 0; /* initialise array pointer */
	return this;
}

void getsettype::remember(bytefield r) {
	numleft++;
	//if (numleft > maxgetset)
	//	error("Trip::remember() : runerr_Getset_Overflow");
	gs[numleft] = r;
}

void getsettype::recall(bytefield &r) {
	r = gs[numleft];
	numleft--;
}






Trip::Trip(AvalancheEngine *vm) {
	_vm = vm;

	getsetclear();
	mustexclaim = false;
}

void Trip::loadtrip() {
	byte gm;

	for (gm = 0; gm < numtr; gm++)
		tr[gm].original();
	
	for (int i = 0; i < sizeof(aa); i++)
		aa[i] = 0;
}

byte Trip::checkfeet(int16 x1, int16 x2, int16 oy, int16 y, byte yl) {
	byte a, c;
	int16 fv, ff;

	/* if not alive then begin checkfeet:=0; exit; end;*/
	a = 0;

	//setactivepage(2);
	if (x1 < 0)
		x1 = 0;
	if (x2 > 639)
		x2 = 639;
	if (oy < y)
		for (fv = x1; fv <= x2; fv++)
			for (ff = oy + yl; ff <= y + yl; ff++) {
				c = *(byte *)_vm->_graphics->_magics.getBasePtr(fv, ff);
				if (c > a) 
					a = c;
			}
	else
		for (fv = x1; fv <= x2; fv++)
			for (ff = y + yl; ff <= oy + yl; ff++) {
				c = *(byte *)_vm->_graphics->_magics.getBasePtr(fv, ff);
				if (c > a)
					a = c;
			}

	//setactivepage(1 - cp);
	return a;
}

byte Trip::geida_ped(byte which) {
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
	}
}

void Trip::catamove(byte ped) {
/* When you enter a new position in the catacombs, this procedure should
	be called. It changes the Also codes so that they may match the picture
	on the screen. (Coming soon: It draws up the screen, too.) */

	int32 here;
	uint16 xy_uint16;
	byte fv, ff;

	/* XY_uint16 is cat_x+cat_y*256. Thus, every room in the
		catacombs has a different number for it. */


		
	xy_uint16 = _vm->_gyro->dna.cat_x + _vm->_gyro->dna.cat_y * 256;
	_vm->_gyro->dna.geida_spin = 0;
		
	switch (xy_uint16) {
	case 1801: /* Exit catacombs */
		fliproom(r__lustiesroom, 4);
		_vm->_scrolls->display("Phew! Nice to be out of there!");
		return;
	case 1033: /* Oubliette */
		fliproom(r__oubliette, 1);
		_vm->_scrolls->display(Common::String("Oh, NO!") + _vm->_scrolls->kControlRegister + '1' + _vm->_scrolls->kControlSpeechBubble);
		return;
	case 4:
		fliproom(r__geidas, 1);
		return;
	case 2307:
		fliproom(r__lusties, 5);
		_vm->_scrolls->display("Oh no... here we go again...");
		_vm->_gyro->dna.user_moves_avvy = false;
		tr[0].iy = 1;
		tr[0].ix = 0;
		return;
	}

	if (!_vm->_gyro->dna.enter_catacombs_from_lusties_room)
		_vm->_lucerna->load(29);
	here = _vm->_gyro->catamap[_vm->_gyro->dna.cat_y - 1][_vm->_gyro->dna.cat_x - 1];

	switch (here & 0xf) { /* West. */
	case 0: /* no connection (wall) */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->nix; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28);
		break;
	case 0x1: /* no connection (wall + shield), */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->nix; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 29); /* ...shield. */
		break;
	case 0x2: /* wall with door */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->special; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); /* ...door. */
		break;
	case 0x3: /* wall with door and shield */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->special; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); /* ...door, and... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 29); /* ...shield. */
		break;
	case 0x4: /* no connection (wall + window), */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->nix; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 5);  /* ...window. */
		break;
	case 0x5: /* wall with door and window */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->special; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); /* ...door, and... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 5); /* ...window. */
		break;
	case 0x6: /* no connection (wall + torches), */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->nix; /* No door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 7); /* ...torches. */
		break;
	case 0x7: /* wall with door and torches */
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[4].op = _vm->_gyro->special; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 28); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 30); /* ...door, and... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 7); /* ...torches. */
		break;
	case 0xf: /* straight-through corridor. */
		_vm->_gyro->magics[1].op = _vm->_gyro->nix; /* Sloping wall. */
		_vm->_gyro->magics[2].op = _vm->_gyro->special; /* Straight wall. */
		break;
	}

	/*  ---- */

	switch ((here & 0xf0) >> 4) { /* East */
	case 0: /* no connection (wall) */
		_vm->_gyro->magics[4].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[5].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[6].op = _vm->_gyro->nix; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 19);
		break;
	case 0x1: /* no connection (wall + window), */
		_vm->_gyro->magics[4].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[5].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[6].op = _vm->_gyro->nix; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 20); /* ...window. */
		break;
	case 0x2: /* wall with door */
		_vm->_gyro->magics[4].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[5].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[6].op = _vm->_gyro->special; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); /* ...door. */
		break;
	case 0x3: /* wall with door and window */
		_vm->_gyro->magics[4].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[5].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[6].op = _vm->_gyro->special; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 20); /* ...door, and... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); /* ...window. */
		break;
	case 0x6: /* no connection (wall + torches), */
		_vm->_gyro->magics[4].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[5].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[6].op = _vm->_gyro->nix; /* No door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 18); /* ...torches. */
		break;
	case 0x7: /* wall with door and torches */
		_vm->_gyro->magics[4].op = _vm->_gyro->bounces; /* Sloping wall. */
		_vm->_gyro->magics[5].op = _vm->_gyro->nix; /* Straight wall. */
		_vm->_gyro->portals[6].op = _vm->_gyro->special; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 19); /* Wall, plus... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 21); /* ...door, and... */
		_vm->_celer->drawBackgroundSprite(-1, -1, 18); /* ...torches. */
		break;
	case 0xf: /* straight-through corridor. */
		_vm->_gyro->magics[4].op = _vm->_gyro->nix; /* Sloping wall. */
		_vm->_gyro->magics[5].op = _vm->_gyro->special; /* Straight wall. */
		_vm->_gyro->portals[6].op = _vm->_gyro->nix; /* Door. */
		break;
	}

	/*  ---- */

	switch ((here & 0xf00) >> 8) { /* South */
	case 0: /* No connection. */
		_vm->_gyro->magics[6].op = _vm->_gyro->bounces;
		_vm->_gyro->magics[11].op = _vm->_gyro->bounces;
		_vm->_gyro->magics[12].op = _vm->_gyro->bounces;
		break;
	case 0x1:
		_vm->_celer->drawBackgroundSprite(-1, -1, 22);
			
		if ((xy_uint16 == 2051) && (_vm->_gyro->dna.geida_follows))
			_vm->_gyro->magics[12].op = _vm->_gyro->exclaim;
		else
			_vm->_gyro->magics[12].op = _vm->_gyro->special; /* Right exit south. */

		_vm->_gyro->magics[6].op = _vm->_gyro->bounces;
		_vm->_gyro->magics[11].op = _vm->_gyro->bounces;
		break;
	case 0x2:
		_vm->_celer->drawBackgroundSprite(-1, -1, 23);
		_vm->_gyro->magics[6].op = _vm->_gyro->special; /* Middle exit south. */
		_vm->_gyro->magics[11].op = _vm->_gyro->bounces;
		_vm->_gyro->magics[12].op = _vm->_gyro->bounces;
		break;
	case 0x3:
		_vm->_celer->drawBackgroundSprite(-1, -1, 24);
		_vm->_gyro->magics[11].op = _vm->_gyro->special; /* Left exit south. */
		_vm->_gyro->magics[6].op = _vm->_gyro->bounces;
		_vm->_gyro->magics[12].op = _vm->_gyro->bounces;
		break;
	}

	switch ((here & 0xf000) >> 12) { /* North */
	case 0: /* No connection */
		_vm->_gyro->magics[0].op = _vm->_gyro->bounces;
		_vm->_gyro->portals[3].op = _vm->_gyro->nix; /* Door. */
		break;
	// LEFT handles: 
/*
	case 0x1:
		_vm->_celer->show_one(-1, -1, 4);
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; // { Left exit north. } { Change magic number! }
		_vm->_gyro->portals[12].op = _vm->_gyro->special; // { Door. }
		break;
*/
	case 0x2:
		_vm->_celer->drawBackgroundSprite(-1, -1, 4);
		_vm->_gyro->magics[0].op = _vm->_gyro->bounces; // Middle exit north.
		_vm->_gyro->portals[3].op = _vm->_gyro->special; // Door.
		break;
/*	case 0x3:
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
*/
	case 0x5:
		_vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_gyro->magics[0].op = _vm->_gyro->bounces; /* Middle exit north. */
		_vm->_gyro->portals[3].op = _vm->_gyro->special; /* Door. */
		break;
/*
	case 0x6:
		_vm->_celer->show_one(-1, -1, 3);
		_vm->_gyro->magics[1].op = _vm->_gyro->bounces; // { Right exit north. }
		_vm->_gyro->portals[12].op = _vm->_gyro->special; // { Door. }
		break;
*/
	// ARCHWAYS:
	case 0x7:
	case 0x8:
	case 0x9: {
		_vm->_celer->drawBackgroundSprite(-1, -1, 6);

		if (((here & 0xf000) >> 12) > 0x7)
			_vm->_celer->drawBackgroundSprite(-1, -1, 31);
		if (((here & 0xf000) >> 12) == 0x9)
			_vm->_celer->drawBackgroundSprite(-1, -1, 32);

		_vm->_gyro->magics[0].op = _vm->_gyro->special; /* Middle arch north. */
		_vm->_gyro->portals[3].op = _vm->_gyro->nix; /* Door. */
	}
	break;
	/* DECORATIONS: */
	case 0xd: /* No connection + WINDOW */
		_vm->_gyro->magics[0].op = _vm->_gyro->bounces;
		_vm->_gyro->portals[3].op = _vm->_gyro->nix; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 14);
		break;
	case 0xe: /* No connection + TORCH */
		_vm->_gyro->magics[0].op = _vm->_gyro->bounces;
		_vm->_gyro->portals[3].op = _vm->_gyro->nix; /* Door. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 8);
		break;
	/* Recessed door: */
	case 0xf:
		_vm->_gyro->magics[0].op = _vm->_gyro->nix; /* Door to Geida's room. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 1);
		_vm->_gyro->portals[3].op = _vm->_gyro->special; /* Door. */
		break;
	}

	switch (xy_uint16) {
	case 514:
		_vm->_celer->drawBackgroundSprite(-1, -1, 17);
		break;     /* [2,2] : "Art Gallery" sign over door. */
	case 264:
		_vm->_celer->drawBackgroundSprite(-1, -1, 9);
		break;      /* [8,1] : "The Wrong Way!" sign. */
	case 1797:
		_vm->_celer->drawBackgroundSprite(-1, -1, 2);
		break;      /* [5,7] : "Ite Mingite" sign. */
	case 258:
		for (fv = 0; fv <= 2; fv++) { /* [2,1] : Art gallery - pictures */
			_vm->_celer->drawBackgroundSprite(130 + fv * 120, 70, 15);
			_vm->_celer->drawBackgroundSprite(184 + fv * 120, 78, 16);
		}
		break;
	case 1287:
		for (fv = 10; fv <= 13; fv++)
			_vm->_celer->drawBackgroundSprite(-1, -1, fv);
		break; /* [7,5] : 4 candles. */
	case 776:
		_vm->_celer->drawBackgroundSprite(-1, -1, 10);
		break;     /* [8,3] : 1 candle. */
	case 2049:
		_vm->_celer->drawBackgroundSprite(-1, -1, 11);
		break;     /* [1,8] : another candle. */
	case 257:
		_vm->_celer->drawBackgroundSprite(-1, -1, 12);
		_vm->_celer->drawBackgroundSprite(-1, -1, 13);
		break; /* [1,1] : the other two. */
	}

	if ((_vm->_gyro->dna.geida_follows) && (ped > 0)) {
		if (!tr[1].quick)  /* If we don't already have her... */
			tr[1].init(5, true, this); /* ...Load Geida. */
		apped(2, geida_ped(ped));
		tr[1].call_eachstep = true;
		tr[1].eachstep = procgeida_procs;
	}
}

	

/* This proc gets called whenever you touch a line defined as _vm->_gyro->special. */
void Trip::dawndelay() {
	_vm->_timeout->set_up_timer(2, _vm->_timeout->procdawn_delay, _vm->_timeout->reason_dawndelay);
}

void Trip::call_special(uint16 which) {
	switch (which) {
	case 1: /* _vm->_gyro->special 1: Room 22: top of stairs. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 1);
		_vm->_gyro->dna.brummie_stairs = 1;
		_vm->_gyro->magics[9].op = _vm->_gyro->nix;
		_vm->_timeout->set_up_timer(10, _vm->_timeout->procstairs, _vm->_timeout->reason_brummiestairs);
		stopwalking();
		_vm->_gyro->dna.user_moves_avvy = false;
		break;
	case 2: /* _vm->_gyro->special 2: Room 22: bottom of stairs. */
		_vm->_gyro->dna.brummie_stairs = 3;
		_vm->_gyro->magics[10].op = _vm->_gyro->nix;
		_vm->_gyro->magics[11].op = _vm->_gyro->exclaim;
		_vm->_gyro->magics[11].data = 5;
		_vm->_gyro->magics[3].op = _vm->_gyro->bounces; /* Now works as planned! */
		stopwalking();
		_vm->_visa->dixi('q', 26);
		_vm->_gyro->dna.user_moves_avvy = true;
		break;
	case 3: /* _vm->_gyro->special 3: Room 71: triggers dart. */
		tr[0].bounce(); /* Must include that. */

		if (!_vm->_gyro->dna.arrow_triggered) {
			_vm->_gyro->dna.arrow_triggered = true;
			apped(2, 4); /* The dart starts at ped 4, and... */
			tr[1].walkto(5); /* flies to ped 5. */
			tr[1].face = 0; /* Only face. */
			/* Should call some kind of Eachstep procedure which will deallocate
				the sprite when it hits the wall, and replace it with the chunk
				graphic of the arrow buried in the plaster. */
			/* OK! */
			tr[1].call_eachstep = true;
			tr[1].eachstep = procarrow_procs;
		}
		break;
	case 4: /* This is the ghost room link. */
		_vm->_lucerna->dusk();
		tr[0].turn(right); /* you'll see this after we get back from bootstrap */
		_vm->_timeout->set_up_timer(1, _vm->_timeout->procghost_room_phew, _vm->_timeout->reason_ghost_room_phew);
		_vm->_enid->back_to_bootstrap(3);
		break;
	case 5:
		if (_vm->_gyro->dna.friar_will_tie_you_up) {
			/* _vm->_gyro->special 5: Room 42: touched tree, and get tied up. */
			_vm->_gyro->magics[4].op = _vm->_gyro->bounces; /* Boundary effect is now working again. */
			_vm->_visa->dixi('q', 35);
			tr[0].done();
			/*tr[1].vanishifstill:=true;*/
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_visa->dixi('q', 36);
			_vm->_gyro->dna.tied_up = true;
			_vm->_gyro->dna.friar_will_tie_you_up = false;
			tr[1].walkto(3);
			tr[1].vanishifstill = true;
			tr[1].check_me = true; /* One of them must have Check_Me switched on. */
			_vm->_gyro->whereis[_vm->_gyro->pfriartuck - 150] = 177; /* Not here, then. */
			_vm->_timeout->set_up_timer(364, _vm->_timeout->prochang_around, _vm->_timeout->reason_hanging_around);
		}
		break;
	case 6: /* _vm->_gyro->special 6: fall down oubliette. */
		_vm->_gyro->dna.user_moves_avvy = false;
		tr[0].ix = 3;
		tr[0].iy = 0;
		tr[0].face = right;
		_vm->_timeout->set_up_timer(1, _vm->_timeout->procfall_down_oubliette, _vm->_timeout->reason_falling_down_oubliette);
		break;
	case 7: /* _vm->_gyro->special 7: stop falling down oubliette. */
		tr[0].visible = false;
		_vm->_gyro->magics[9].op = _vm->_gyro->nix;
		stopwalking();
		_vm->_timeout->lose_timer(_vm->_timeout->reason_falling_down_oubliette);
		/*_vm->_lucerna->mblit(12, 80, 38, 160, 3, 0);
		_vm->_lucerna->mblit(12, 80, 38, 160, 3, 1);*/
		_vm->_scrolls->display("Oh dear, you seem to be down the bottom of an oubliette.");
		_vm->_timeout->set_up_timer(200, _vm->_timeout->procmeet_avaroid, _vm->_timeout->reason_meeting_avaroid);
		break;
	case 8:        /* _vm->_gyro->special 8: leave du Lustie's room. */
		if ((_vm->_gyro->dna.geida_follows) && (!_vm->_gyro->dna.lustie_is_asleep)) {
			_vm->_visa->dixi('q', 63);
			tr[1].turn(down);
			tr[1].stopwalk();
			tr[1].call_eachstep = false; /* Geida */
			_vm->_lucerna->gameover();
		}
		break;
	case 9: /* _vm->_gyro->special 9: lose Geida to Robin Hood... */
		if (!_vm->_gyro->dna.geida_follows)
			return;   /* DOESN'T COUNT: no Geida. */
		tr[1].call_eachstep = false; /* She no longer follows Avvy around. */
		tr[1].walkto(4); /* She walks to somewhere... */
		tr[0].done();     /* Lose Avvy. */
		_vm->_gyro->dna.user_moves_avvy = false;
		_vm->_timeout->set_up_timer(40, _vm->_timeout->procrobin_hood_and_geida, _vm->_timeout->reason_robin_hood_and_geida);
		break;
	case 10: /* _vm->_gyro->special 10: transfer north in catacombs. */
		if ((_vm->_gyro->dna.cat_x == 4) && (_vm->_gyro->dna.cat_y == 1)) {
			/* Into Geida's room. */
			if (_vm->_gyro->dna.obj[_vm->_gyro->key - 1])
				_vm->_visa->dixi('q', 62);
			else {
				_vm->_visa->dixi('q', 61);
				return;
			}
		}
		_vm->_lucerna->dusk();
		_vm->_gyro->dna.cat_y --;
		catamove(4);
		if (_vm->_gyro->dna.room != r__catacombs)
			return;
		_vm->_lucerna->delavvy();
		switch ((_vm->_gyro->catamap[_vm->_gyro->dna.cat_y - 1][_vm->_gyro->dna.cat_x - 1] & 0xf00) >> 8) {
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
	case 11: /* _vm->_gyro->special 11: transfer east in catacombs. */
		_vm->_lucerna->dusk();
		_vm->_gyro->dna.cat_x++;
		catamove(1);
		if (_vm->_gyro->dna.room != r__catacombs)
			return;
		_vm->_lucerna->delavvy();
		apped(1, 1);
		getback();
		dawndelay();
		break;
	case 12: /* _vm->_gyro->special 12: transfer south in catacombs. */
		_vm->_lucerna->dusk();
		_vm->_gyro->dna.cat_y += 1;
		catamove(2);
		if (_vm->_gyro->dna.room != r__catacombs)
			return;
		_vm->_lucerna->delavvy();
		apped(1, 2);
		getback();
		dawndelay();
		break;
	case 13: /* _vm->_gyro->special 13: transfer west in catacombs. */
		_vm->_lucerna->dusk();
		_vm->_gyro->dna.cat_x--;
		catamove(3);
		if (_vm->_gyro->dna.room != r__catacombs)
			return;
		_vm->_lucerna->delavvy();
		apped(1, 3);
		getback();
		dawndelay();
		break;
	}
}

	

void Trip::open_the_door(byte whither, byte ped, byte magicnum) {
/* This slides the door open. (The data really ought to be saved in
	the Also file, and will be next time. However, for now, they're
	here.) */
	switch (_vm->_gyro->dna.room) {
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
			if (_vm->_gyro->dna.avvys_in_the_cupboard) {
				hide_in_the_cupboard();
				_vm->_sequence->first_show(8);
				_vm->_sequence->then_show(7);
				_vm->_sequence->start_to_close();
				return;
			} else {
				apped(1, 6);
				tr[0].face = right; /* added by TT 12/3/1995 */
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

void Trip::newspeed() {
	// Given that you've just changed the speed in triptype.xs, this adjusts ix.
	const bytefield lightspace = {40, 199, 47, 199};
	byte page_;

	tr[0].ix = (tr[0].ix / 3) * tr[0].xs;

	//setactivepage(3);

	if (tr[0].xs == _vm->_gyro->run)
		_vm->_graphics->_surface.drawLine(371, 199, 373, 199, yellow);
	else
		_vm->_graphics->_surface.drawLine(336, 199, 338, 199, yellow);

	if (tr[0].xs == _vm->_gyro->run)
		_vm->_graphics->_surface.drawLine(336, 199, 338, 199, lightblue);
	else
		_vm->_graphics->_surface.drawLine(371, 199, 373, 199, lightblue);

	//setactivepage(1 - cp);

	for (page_ = 0; page_ <= 1; page_ ++)
		getset[page_].remember(lightspace);
	
}

void Trip::rwsp(byte t, byte r) {
	switch (r) {
	case up:
		tr[t].speed(0, -tr[t].ys);
		break;
	case down:
		tr[t].speed(0, tr[t].ys);
		break;
	case left:
		tr[t].speed(-tr[t].xs,  0);
		break;
	case right:
		tr[t].speed(tr[t].xs,  0);
		break;
	case ul:
		tr[t].speed(-tr[t].xs, -tr[t].ys);
		break;
	case ur:
		tr[t].speed(tr[t].xs, -tr[t].ys);
		break;
	case dl:
		tr[t].speed(-tr[t].xs, tr[t].ys);
		break;
	case dr:
		tr[t].speed(tr[t].xs, tr[t].ys);
		break;
	}
}

void Trip::apped(byte trn, byte np) {
	trn--;
	np--;
	tr[trn].appear(_vm->_gyro->peds[np].x - tr[trn]._info.xl / 2, _vm->_gyro->peds[np].y - tr[trn]._info.yl, _vm->_gyro->peds[np].dir);
	rwsp(trn, _vm->_gyro->peds[np].dir);
}



/*   function overlap(x1,y1,x2,y2,x3,y3,x4,y4:uint16):bool;
	begin { By De Morgan's law: }
	overlap:=(x2>=x3) and (x4>=x1) and (y2>=y3) and (y4>=y1);
	end;*/
/* x1,x2 - as _vm->_gyro->bytefield, but *8. y1,y2 - as _vm->_gyro->bytefield.
	x3,y3 = mx,my. x4,y4 = mx+16,my+16. */
bool Trip::overlaps_with_mouse() {
	bool overlaps_with_mouse_result;
	overlaps_with_mouse_result =
		(r.x2 * 8 >= _vm->_gyro->mx) && (_vm->_gyro->mx + 16 >= r.x1 * 8) && (r.y2 >= _vm->_gyro->my) && (_vm->_gyro->my + 16 >= r.y1);
	return overlaps_with_mouse_result;
}

void Trip::getback() {
	byte fv;
	bool endangered;


	endangered = false;
	/* Super_Off;*/

	while (getset[1 - _vm->_gyro->cp].numleft > 0) {
		getset[1 - _vm->_gyro->cp].recall(r);

		/*
		if overlaps_with_mouse and not endangered then
			begin
				endangered:=true;
				blitfix;
				Super_Off;
			end;
		*/

		//_vm->_lucerna->mblit(r.x1, r.y1, r.x2, r.y2, 3, 1 - _vm->_gyro->cp);
	}
	/*
	if endangered then
		Super_On;
	*/
}

/* Eachstep procedures: */
void Trip::follow_avvy_y(byte tripnum) {
	if (tr[0].face == left)
		return;
	if (tr[tripnum].homing)
		tr[tripnum].hy = tr[1].y;
	else {
		if (tr[tripnum].y < tr[1].y)
			tr[tripnum].y += 1;
		else if (tr[tripnum].y > tr[1].y)
			tr[tripnum].y -= 1;
		else
			return;
		if (tr[tripnum].ix == 0)  {
			tr[tripnum].step += 1;
			if (tr[tripnum].step == tr[tripnum].a.seq)
				tr[tripnum].step = 0;
			tr[tripnum].count = 0;
		}
	}
}

void Trip::back_and_forth(byte tripnum) {
	if (!tr[tripnum].homing) {
		if (tr[tripnum].face == right)
			tr[tripnum].walkto(4);
		else
			tr[tripnum].walkto(5);
	}
}

void Trip::face_avvy(byte tripnum) {
	if (!tr[tripnum].homing) {
		if (tr[0].x >= tr[tripnum].x)
			tr[tripnum].face = right;
		else
			tr[tripnum].face = left;
	}
}

void Trip::arrow_procs(byte tripnum) {
	byte fv;
			
	if (tr[tripnum].homing) {
		/* Arrow is still in flight. */
		/* We must check whether or not the arrow has collided tr[tripnum] Avvy's head.
			This is so if: a) the bottom of the arrow is below Avvy's head,
			b) the left of the arrow is left of the right of Avvy's head, and
			c) the right of the arrow is right of the left of Avvy's head. */
		if (((tr[tripnum].y + tr[tripnum]._info.yl) >= tr[0].y) /* A */
				&& (tr[tripnum].x <= (tr[0].x + tr[0]._info.xl)) /* B */
				&& ((tr[tripnum].x + tr[tripnum]._info.xl) >= tr[0].x)) { /* C */
			/* OK, it's hit him... what now? */

			tr[1].call_eachstep = false; /* prevent recursion. */
			_vm->_visa->dixi('Q', 47); /* Complaint! */
			tr[tripnum].done(); /* Deallocate the arrow. */
			/*     tr[1].done; { Deallocate normal pic of Avvy. }

					off;
					for fv:=0 to 1 do
					begin
					cp:=1-cp;
					getback;
					end;
					on;*/

			_vm->_lucerna->gameover();

			_vm->_gyro->dna.user_moves_avvy = false; /* Stop the user from moving him. */
			_vm->_timeout->set_up_timer(55, _vm->_timeout->procnaughty_duke, _vm->_timeout->reason_naughty_duke);
		}
	} else { /* Arrow has hit the wall! */
		tr[tripnum].done(); /* Deallocate the arrow. */
		_vm->_celer->drawBackgroundSprite(-1, -1, 3); /* Show pic of arrow stuck into the door. */
		_vm->_gyro->dna.arrow_in_the_door = true; /* So that we can pick it up. */
	}
		
}

/*procedure Spludwick_procs(tripnum:byte);
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
end;*/

void Trip::grab_avvy(byte tripnum) {     /* For Friar Tuck, in Nottingham. */
	byte fv;
	int16 tox, toy;
			
	tox = tr[0].x + 17;
	toy = tr[0].y - 1;
	if ((tr[tripnum].x == tox) && (tr[tripnum].y == toy)) {
		tr[tripnum].call_eachstep = false;
		tr[tripnum].face = left;
		tr[tripnum].stopwalk();
		/* ... whatever ... */
	} else {
		/* Still some way to go. */
		if (tr[tripnum].x < tox) {
			tr[tripnum].x += 5;
			if (tr[tripnum].x > tox)
				tr[tripnum].x = tox;
		}
		if (tr[tripnum].y < toy)
			tr[tripnum].y++;
		tr[tripnum].step++;
		if (tr[tripnum].step == tr[tripnum].a.seq)
			tr[tripnum].step = 0;
	}
}

void Trip::take_a_step(byte &tripnum) {
	if (tr[tripnum].ix == 0) {
		tr[tripnum].step++;
		if (tr[tripnum].step == tr[tripnum].a.seq)
			tr[tripnum].step = 0;
		tr[tripnum].count = 0;
	}
}

void Trip::spin(byte whichway, byte &tripnum) {
	if (tr[tripnum].face != whichway) {
		tr[tripnum].face = whichway;
		if (tr[tripnum].whichsprite == 2)
			return; /* Not for Spludwick */

		_vm->_gyro->dna.geida_spin += 1;
		_vm->_gyro->dna.geida_time = 20;
		if (_vm->_gyro->dna.geida_spin == 5) {
			_vm->_scrolls->display("Steady on, Avvy, you'll make the poor girl dizzy!");
			_vm->_gyro->dna.geida_spin = 0;
			_vm->_gyro->dna.geida_time = 0; /* knock out records */
		}
	}
}

void Trip::geida_procs(byte tripnum) {
	if (_vm->_gyro->dna.geida_time > 0) {
		_vm->_gyro->dna.geida_time --;
		if (_vm->_gyro->dna.geida_time == 0)
			_vm->_gyro->dna.geida_spin = 0;
	}

	if (tr[tripnum].y < (tr[0].y - 2)) {
		/* Geida is further from the screen than Avvy. */
		spin(down, tripnum);
		tr[tripnum].iy = 1;
		tr[tripnum].ix = 0;
		take_a_step(tripnum);
		return;
	} else if (tr[tripnum].y > (tr[0].y + 2)) {
		/* Avvy is further from the screen than Geida. */
		spin(up, tripnum);
		tr[tripnum].iy = -1;
		tr[tripnum].ix = 0;
		take_a_step(tripnum);
		return;
	}

	tr[tripnum].iy = 0;
	// These 12-s are not in the original, I added them to make the following method more "smooth".
	// Now the NPC which is following Avvy won't block his way and will walk next to him properly.
	if (tr[tripnum].x < tr[0].x - tr[0].xs * 8 - 12) {
		tr[tripnum].ix = tr[0].xs;
		spin(right, tripnum);
		take_a_step(tripnum);
	} else if (tr[tripnum].x > tr[0].x + tr[0].xs * 8 + 12) {
		tr[tripnum].ix = -tr[0].xs;
		spin(left, tripnum);
		take_a_step(tripnum);
	} else
		tr[tripnum].ix = 0;
}

/* That's all... */

void Trip::call_andexors() {
	int8 order[5];
	byte fv, temp;
	bool ok;

	for (int i = 0; i < 5; i++)
		order[i] = -1;

	for (fv = 0; fv < numtr; fv++) {
		if (tr[fv].quick && tr[fv].visible)
			order[fv] = fv;
	}

	do {
		ok = true;
		for (fv = 0; fv < 4; fv++) {
			if (((order[fv] != -1) && (order[fv + 1] != -1))
					&& (tr[order[fv]].y > tr[order[fv + 1]].y)) {
				/* Swap them! */
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

void Trip::trippancy_link() {
	byte fv;

	if (_vm->_gyro->ddmnow | _vm->_gyro->ontoolbar | _vm->_gyro->seescroll)
		return;
	for (fv = 0; fv < numtr; fv++) {
		if (tr[fv].quick && tr[fv].visible)
			tr[fv].walk();
	}

	call_andexors();

	for (fv = 0; fv < numtr; fv++) {
		if (tr[fv].quick && tr[fv].call_eachstep) {
			switch (tr[fv].eachstep) {
			case procfollow_avvy_y :
				follow_avvy_y(fv);
				break;
			case procback_and_forth :
				back_and_forth(fv);
				break;
			case procface_avvy :
				face_avvy(fv);
				break;
			case procarrow_procs :
				arrow_procs(fv);
				break;
				/*    PROCSpludwick_procs : spludwick_procs(fv);*/
			case procgrab_avvy :
				grab_avvy(fv);
				break;
			case procgeida_procs :
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

void Trip::get_back_loretta() {
	byte fv;

	/* for fv:=1 to numtr do with tr[fv] do if quick then getback;*/
	for (fv = 0; fv < numtr; fv++) {
		if (tr[fv].quick) {
			getback();
			return;
		}
	}
	/* for fv:=0 to 1 do begin cp:=1-cp; getback; end;*/
}

void Trip::stopwalking() {
	tr[0].stopwalk();
	_vm->_gyro->dna.rw = stopped;
	if (_vm->_gyro->alive)
		tr[0].step = 1;
}

void Trip::tripkey(char dir) {
	warning("Replaced by Trip::handleMoveKey!");
}

void Trip::readstick() {
	warning("STUB: Trip::readstick()");
}

void Trip::getsetclear() {
	for (byte fv = 0; fv <= 1; fv++)
		getset[fv].init();
}

void Trip::hide_in_the_cupboard() {
	const char nowt = 250; /* As in Acci. */

	if (_vm->_gyro->dna.avvys_in_the_cupboard) {
		if (_vm->_gyro->dna.wearing == nowt)
			_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlItalic) + "AVVY!" + _vm->_scrolls->kControlRoman + "Get dressed first!");
		else {
			tr[0].visible = true;
			_vm->_gyro->dna.user_moves_avvy = true;
			apped(1, 3); /* Walk out of the cupboard. */
			_vm->_scrolls->display("You leave the cupboard. Nice to be out of there!");
			_vm->_gyro->dna.avvys_in_the_cupboard = false;
			_vm->_sequence->first_show(8);
			_vm->_sequence->then_show(7);
			_vm->_sequence->start_to_close();
		}
	} else {
		/* Not hiding in the cupboard */
		tr[0].visible = false;
		_vm->_gyro->dna.user_moves_avvy = false;
		_vm->_scrolls->display(Common::String("You walk into the room...") + _vm->_scrolls->kControlParagraph
			+ "It seems to be an empty, but dusty, cupboard. Hmmmm... you leave the door slightly open to avoid suffocation.");
		_vm->_gyro->dna.avvys_in_the_cupboard = true;
		_vm->_celer->drawBackgroundSprite(-1, -1, 8);
	}
}

void Trip::tidy_up(int16 a, int16 b, int16 c, int16 d) {
	warning("STUB: Trip::tidy_up()");
}

void Trip::tidy_after_mouse() {
	tidy_up(beforex, beforey, beforex + 15, beforey + 15);
	_vm->_gyro->xycheck();
	tidy_up(_vm->_gyro->mx, _vm->_gyro->my, _vm->_gyro->mx + 15, _vm->_gyro->my + 15);
}

void Trip::fliproom(byte room, byte ped) {
	byte fv;

	if (!_vm->_gyro->alive) {
		/* You can't leave the room if you're dead. */
		tr[0].ix = 0;
		tr[0].iy = 0; /* Stop him from moving. */
		return;
	}

	if ((room == 177) && (_vm->_gyro->dna.room == r__lusties)) {
		hide_in_the_cupboard();
		return;
	}

	if ((_vm->_gyro->dna.jumpstatus > 0) && (_vm->_gyro->dna.room == r__insidecardiffcastle)) {
		/* You can't *jump* out of Cardiff Castle! */
		tr[0].ix = 0;
		return;
	}

	_vm->_gyro->xycheck();
	beforex = _vm->_gyro->mx;
	beforey = _vm->_gyro->my;

	_vm->_lucerna->exitroom(_vm->_gyro->dna.room);
	_vm->_lucerna->dusk();
	getsetclear();


	for (fv = 1; fv < numtr; fv++) {
		if (tr[fv].quick)
			tr[fv].done();
	} /* Deallocate sprite */

	if (_vm->_gyro->dna.room == r__lustiesroom)
		_vm->_gyro->dna.enter_catacombs_from_lusties_room = true;

	_vm->_lucerna->enterroom(room, ped);
	apped(1, ped);
	_vm->_gyro->dna.enter_catacombs_from_lusties_room = false;
	_vm->_gyro->oldrw = _vm->_gyro->dna.rw;
	_vm->_gyro->dna.rw = tr[0].face;
	_vm->_lucerna->showrw();

	for (fv = 0; fv <= 1; fv++) {
		_vm->_gyro->cp = 1 - _vm->_gyro->cp;
		getback();
	}
	_vm->_lucerna->dawn();

	/* Tidy up after mouse. I know it's a kludge... */
	/*  tidy_after_mouse;*/
}

bool Trip::infield(byte which) {
	which--; // Pascal -> C: different array indexes.

	int16 yy = tr[0].y + tr[0]._info.yl;

	return (tr[0].x >= _vm->_gyro->fields[which].x1) && (tr[0].x <= _vm->_gyro->fields[which].x2)
		&& (yy >= _vm->_gyro->fields[which].y1) && (yy <= _vm->_gyro->fields[which].y2);

}

bool Trip::neardoor() {
	if (_vm->_gyro->numfields < 8) {
		/* there ARE no doors here! */
		return false;
	}
		
	int16 ux = tr[0].x;
	int16 uy = tr[0].y + tr[0]._info.yl;
	bool nd = false;
	for (byte fv = 8; fv < _vm->_gyro->numfields; fv++)
		if ((ux >= _vm->_gyro->fields[fv].x1) && (ux <= _vm->_gyro->fields[fv].x2)
			&& (uy >= _vm->_gyro->fields[fv].y1) && (uy <= _vm->_gyro->fields[fv].y2)) 
			nd = true;
	return nd;
}

void Trip::new_game_for_trippancy() {   /* Called by gyro.newgame */
	tr[0].visible = false;
}



void Trip::handleMoveKey(const Common::Event &event) { 
	if (!_vm->_gyro->dna.user_moves_avvy)
		return;

	if (_vm->_dropdown->_activeMenuItem._activeNow)
		_vm->_parser->tryDropdown();
	else
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
			if (_vm->_gyro->dna.rw != up) {
				_vm->_gyro->dna.rw = up;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_DOWN:
			if (_vm->_gyro->dna.rw != down) {
				_vm->_gyro->dna.rw = down;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_LEFT:
			if (_vm->_gyro->dna.rw != left) {
				_vm->_gyro->dna.rw = left;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_RIGHT:
			if (_vm->_gyro->dna.rw != right) {
				_vm->_gyro->dna.rw = right;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_PAGEUP:
			if (_vm->_gyro->dna.rw != ur) {
				_vm->_gyro->dna.rw = ur;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_PAGEDOWN:
			if (_vm->_gyro->dna.rw != dr) {
				_vm->_gyro->dna.rw = dr;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_END:
			if (_vm->_gyro->dna.rw != dl) {
				_vm->_gyro->dna.rw = dl;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_HOME:
			if (_vm->_gyro->dna.rw != ul) {
				_vm->_gyro->dna.rw = ul;
				rwsp(0, _vm->_gyro->dna.rw);
			} else
				stopwalking();
			break;
		case Common::KEYCODE_KP5:
			stopwalking();
			break;
		}
}


} // End of namespace Avalanche.
