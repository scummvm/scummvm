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

struct AnimationSaver {
	byte _id;
	byte _facingDir;
	byte _stepNum;
	int16 _x;
	int16 _y;
	int8 _moveX;
	int8 _moveY;
	bool _visible;
	bool _homing;
	bool _doCheck;
	byte _count;
	byte _xWidth, _speedX, _speedY;
	byte _animCount;
	int16 _homingX;
	int16 _homingY;
	bool _callEachStepFl;
	byte _eachStepProc;
	bool _vanishIfStill;
};

class Animation;

class AnimationType {
public:
	SpriteInfo _info;

	adxtype _stat; // Vital statistics.
	byte _facingDir, _stepNum;
	int16 _x, _y; // Current xy coords.
	int16 _oldX[2], _oldY[2];  // Last xy coords.
	int8 _moveX, _moveY; // Amount to move sprite by, each step.
	byte _id;
	bool _quick, _visible, _homing, _doCheck;
	int16 _homingX, _homingY; // Homing x & y coords.
	byte _count; // Counts before changing step.
	byte _speedX, _speedY; // x & y speed.
	byte _animCount; // Total number of sprites.
	bool _vanishIfStill; // Do we show this sprite if it's still?
	bool _callEachStepFl; // Do we call the eachstep procedure?
	byte _eachStepProc;

	void init(byte spritenum, bool doCheck, Animation *tr); // Loads & sets up the sprite.
	void original(); // Just sets 'quick' to false.
	void andexor(); // Drops sprite onto screen.
	void turn(byte whichway); // Turns character round.
	void appear(int16 wx, int16 wy, byte wf); // Switches it on.
	void bounce(); // Bounces off walls.
	void walk(); // Prepares for andexor, etc.
	void walkTo(byte pednum); // Home in on a point.
	void stophoming(); // Self-explanatory.
	void homeStep(); // Calculates ix & iy for one homing step.
	void speed(int8 xx, int8 yy); // Sets ix & iy, non-homing, etc.
	void stopWalk(); // Stops the sprite from moving.
	void chatter();  // Sets up talk vars.
	void done();

private:
	Animation *_tr;

	bool checkCollision();
	int8 sgn(int16 val);
};

class Animation {
public:
	friend class AnimationType;
	friend class getsettype;

	static const byte kDirUp = 0;
	static const byte kDirRight = 1;
	static const byte kDirDown = 2;
	static const byte kDirLeft = 3;
	static const byte kDirUpRight = 4;
	static const byte kDirDownRight = 5;
	static const byte kDirDownLeft = 6;
	static const byte kDirUpLeft = 7;
	static const byte kDirStopped = 8;

	static const int16 kSpriteNumbMax = 5; // current max no. of sprites

	static const byte kProcFollowAvvyY = 1;
	static const byte kProcBackAndForth = 2;
	static const byte kProcFaceAvvy = 3;
	static const byte kProcArrow = 4;
	static const byte kProcsPludwick = 5; // Unused
	static const byte kProcGrabAvvy = 6;
	static const byte kProcGeida = 7;

	Animation(AvalancheEngine *vm);
	~Animation();

	void animLink();
	void get_back_loretta();
	void loadtrip();
	void call_special(uint16 which);
	void openDoor(byte whither, byte ped, byte magicnum); // Handles slidey-open doors.
	void catamove(byte ped);
	void stopWalking();
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
	void hideInCupboard();
	void follow_avvy_y(byte tripnum);
	void back_and_forth(byte tripnum);
	void face_avvy(byte tripnum);
	void arrow_procs(byte tripnum);
	void grab_avvy(byte tripnum);
	void spin(byte whichway, byte &tripnum);
	void geida_procs(byte tripnum);
	void call_andexors();
};

} // End of namespace Avalanche.

#endif // AVALANCHE_ANIMATION_H
