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

/* Original name TRIP5 / Trippancy V - the sprite animation subsystem */

#ifndef AVALANCHE_ANIMATION_H
#define AVALANCHE_ANIMATION_H

#include "avalanche/graphics.h"

#include "common/scummsys.h"
#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

struct adxtype { // Second revision of ADX type
	// CHECKME: Useless?
	Common::String _name; // name of character
	Common::String _comment; // comment
	//
	byte _frameNum; // number of pictures
	byte seq; // how many in one stride
	byte _fgBubbleCol, _bgBubbleCol; // foreground & background bubble colors
	byte accinum; // the number according to Acci (1=Avvy, etc.)
};

struct trip_saver_type {
	byte whichsprite;
	byte face;
	byte step;
	int16 x;
	int16 y;
	int8 ix;
	int8 iy;
	bool visible;
	bool homing;
	bool check_me;
	byte count;
	byte xw, xs, ys;
	byte totalnum;
	int16 hx;
	int16 hy;
	bool call_eachstep;
	byte eachstep;
	bool vanishifstill;
};

class Animation;

class AnimationType {
public:
	SpriteInfo _info;

	adxtype _stat; // vital statistics
	byte face, step;
	int16 _x, _y; // current xy coords
	int16 _oldX[2], _oldY[2];  // last xy coords
	int8 ix, iy; // amount to move sprite by, each step
	byte whichsprite;
	bool quick, _visible, homing, check_me;
	int16 hx, hy; // homing x & y coords
	byte count; // counts before changing step
	byte xs, ys; // x & y speed
	byte totalnum; // total number of sprites
	bool vanishifstill; // Do we show this sprite if it's still?

	bool call_eachstep; // Do we call the eachstep procedure?
	byte eachstep;

	void init(byte spritenum, bool do_check, Animation *tr);
	// loads & sets up the sprite
	void original();    // just sets Quick to false
	void andexor();    // drops sprite onto screen
	void turn(byte whichway);      // turns him round
	void appear(int16 wx, int16 wy, byte wf); // switches him on
	void bounce();    // bounces off walls.
	void walk();    // prepares for andexor, etc.
	void walkto(byte pednum);      // home in on a point
	void stophoming();    // self-explanatory
	void homestep();    // calculates ix & iy for one homing step
	void speed(int8 xx, int8 yy); // sets ix & iy, non-homing, etc
	void stopwalk();    // Stops the sprite from moving
	void chatter();    // Sets up talk vars
	void set_up_saver(trip_saver_type &v);
	void unload_saver(trip_saver_type v);

	void savedata(Common::File &f); // Self-explanatory,
	void loaddata(Common::File &f);  // really.

	void save_data_to_mem(uint16 &where);
	void load_data_from_mem(uint16 &where);
	AnimationType *done();

private:
	Animation *_tr;

	bool checkCollision();
	int8 sgn(int16 val);
};

const int16 maxgetset = 35;

class getsettype {
public:
	ByteField gs[maxgetset];
	byte numleft;

	getsettype *init();
	void remember(ByteField r);
	void recall(ByteField &r);
};

class Animation {
public:
	friend class AnimationType;
	friend class getsettype;

	static const int16 kDirUp = 0;
	static const int16 kDirRight = 1;
	static const int16 kDirDown = 2;
	static const int16 kDirLeft = 3;
	static const int16 kDirUpRight = 4;
	static const int16 kDirDownRight = 5;
	static const int16 kDirDownLeft = 6;
	static const int16 kDirUpLeft = 7;
	static const int16 kDirStopped = 8;

	static const int16 kSpriteNumbMax = 5; // current max no. of sprites

	static const int16 kProcFollowAvvyY = 1;
	static const int16 kProcBackAndForth = 2;
	static const int16 kProcFaceAvvy = 3;
	static const int16 kProcArrow = 4;
	static const int16 kProcsPludwick = 5; // Unused
	static const int16 kProcGrabAvvy = 6;
	static const int16 kProcGeida = 7;

	Animation(AvalancheEngine *vm);
	~Animation();

	void animLink();
	void get_back_loretta();
	void loadtrip();
	void call_special(uint16 which);
	void open_the_door(byte whither, byte ped, byte magicnum); // Handles slidey-open doors.
	void catamove(byte ped);
	void stopwalking();
	void tripkey(char dir);
	void rwsp(byte t, byte dir);
	void apped(byte trn, byte np);
	void getback();
	void fliproom(byte room, byte ped);
	bool infield(byte which); // Returns true if you're within field "which".
	bool neardoor(); // Returns True if you're near a door.
	void readstick();
	void newspeed();
	void new_game_for_trippancy();
	void take_a_step(byte &tripnum);
	void handleMoveKey(const Common::Event &event); // To replace tripkey().

	AnimationType tr[kSpriteNumbMax];
	getsettype getset[2];
	byte aa[1600];

	bool mustexclaim;
	uint16 saywhat;

private:
	AvalancheEngine *_vm;

	// CHECKME: Useless?
	// ByteField r;
	// int16 beforex, beforey;

	byte checkfeet(int16 x1, int16 x2, int16 oy, int16 y, byte yl);
	byte geida_ped(byte which);
	void dawndelay();
	void hide_in_the_cupboard();
	void follow_avvy_y(byte tripnum);
	void back_and_forth(byte tripnum);
	void face_avvy(byte tripnum);
	void arrow_procs(byte tripnum);
	void grab_avvy(byte tripnum);
	void spin(byte whichway, byte &tripnum);
	void geida_procs(byte tripnum);
	void call_andexors();
	void getsetclear();
};

} // End of namespace Avalanche.

#endif // AVALANCHE_ANIMATION_H
