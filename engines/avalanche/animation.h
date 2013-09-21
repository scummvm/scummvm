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

/* Original name: TRIP5 / Trippancy V - the sprite animation subsystem */

#ifndef AVALANCHE_ANIMATION_H
#define AVALANCHE_ANIMATION_H

#include "avalanche/graphics.h"

#include "common/scummsys.h"
#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;
class Animation;

struct StatType {
	Common::String _name; // Name of character.
	Common::String _comment; // Comment.
	byte _frameNum; // Number of pictures.
	byte _seq; // How many in one stride.
	byte _fgBubbleCol, _bgBubbleCol; // Foreground & background bubble colors.
	byte _acciNum; // The number according to Acci. (1=Avvy, etc.)
};

class AnimationType {
public:
	SpriteInfo _info;
	StatType _stat; // Vital statistics.
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

	void init(byte spritenum, bool doCheck, Animation *anim); // Loads & sets up the sprite.
	void original(); // Just sets 'quick' to false.
	void draw(); // Drops sprite onto screen. Original: andexor().
	void turn(byte whichway); // Turns character round.
	void appear(int16 wx, int16 wy, byte wf); // Switches it on.
	void bounce(); // Bounces off walls.
	void walk(); // Prepares for andexor, etc.
	void walkTo(byte pednum); // Home in on a point.
	void stopHoming(); // Self-explanatory.
	void homeStep(); // Calculates ix & iy for one homing step.
	void setSpeed(int8 xx, int8 yy); // Sets ix & iy, non-homing, etc.
	void stopWalk(); // Stops the sprite from moving.
	void chatter();  // Sets up talk vars.
	void remove();

private:
	Animation *_anim;

	bool checkCollision();
	int8 getSign(int16 val);
};

class Animation {
public:
	friend class AnimationType;

	enum Direction {
		kDirUp, kDirRight, kDirDown, kDirLeft,
		kDirUpRight, kDirDownRight, kDirDownLeft, kDirUpLeft,
		kDirStopped
	};

	static const byte kSpriteNumbMax = 5; // current max no. of sprites

	enum Proc {
		kProcFollowAvvyY = 1,
		kProcBackAndForth,
		kProcFaceAvvy,
		kProcArrow,
		kProcSpludwick, // Unused
		kProcGrabAvvy,
		kProcGeida // Spludwick uses it as well for homing! TODO: Unify it with kProcSpludwick.
	};

	AnimationType _sprites[kSpriteNumbMax];
	bool _mustExclaim;
	uint16 _sayWhat;
	byte _direction; // The direction Avvy is currently facing.
	byte _oldDirection;

	Animation(AvalancheEngine *vm);
	~Animation();

	void animLink();
	void loadAnims(); // Original: loadtrip().
	void callSpecial(uint16 which);
	void openDoor(byte whither, byte ped, byte magicnum); // Handles slidey-open doors.
	void catacombMove(byte ped); // When you enter a new position in the catacombs, this procedure should be called. It changes the 'also' codes so that they may match the picture on the screen.
	void stopWalking();
	void changeDirection(byte t, byte dir);
	void appearPed(byte sprNum, byte pedNum);
	void flipRoom(byte room, byte ped);
	bool inField(byte which); // Returns true if you're within field "which".
	bool nearDoor(); // Returns True if you're near a door.
	void updateSpeed();
	void handleMoveKey(const Common::Event &event); // To replace tripkey().

private:
	AvalancheEngine *_vm;

	byte checkFeet(int16 x1, int16 x2, int16 oy, int16 y, byte yl);
	byte geidaPed(byte ped);
	void dawnDelay();

	void grabAvvy(byte tripnum);
	void arrowProcs(byte tripnum);
	void hideInCupboard();

	// Different movements for NPCs:
	void followAvalotY(byte tripnum); // Original: follow_avvy_y().
	void backAndForth(byte tripnum);
	void faceAvvy(byte tripnum);
	
	// Movements for Homing NPCs: Spludwick and Geida.
	void spin(byte whichway, byte &tripnum);
	void takeAStep(byte &tripnum);
	void geidaProcs(byte tripnum);

	void drawSprites(); // Original: call_andexors().
};

} // End of namespace Avalanche.

#endif // AVALANCHE_ANIMATION_H
