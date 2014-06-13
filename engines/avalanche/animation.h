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

/* Original name: TRIP5 / Trippancy V - the sprite animation subsystem */

#ifndef AVALANCHE_ANIMATION_H
#define AVALANCHE_ANIMATION_H

namespace Avalanche {
class AvalancheEngine;
class Animation;

enum Direction {
	kDirUp = 0, kDirRight, kDirDown, kDirLeft,
	kDirUpRight, kDirDownRight, kDirDownLeft, kDirUpLeft,
	kDirStopped, kDirNone = 177
};

class AnimationType {
public:
	byte _id;

	byte _xLength, _yLength;
	ManiType *_mani[24];
	SilType *_sil[24];
	byte _frameNum; // Number of pictures.
	byte _seq; // How many in one stride.
	byte _characterId; // The number according to Acci. (1=Avvy, etc.)
	byte _count; // Counts before changing step.

	Direction _facingDir;
	byte _stepNum;
	int16 _x, _y; // Current xy coords.
	int8 _moveX, _moveY; // Amount to move sprite by, each step.
	bool _quick, _visible, _homing, _doCheck;
	int16 _homingX, _homingY; // Homing x & y coords.
	byte _speedX, _speedY;
	bool _vanishIfStill;
	bool _callEachStepFl;
	byte _eachStepProc;

	AnimationType(Animation *anim);

	void init(byte spritenum, bool doCheck);
	void reset();
	void draw();
	void turn(Direction whichway);
	void appear(int16 wx, int16 wy, Direction wf);
	void bounce();
	void walk();
	void walkTo(byte pednum);
	void stopHoming();
	void setSpeed(int8 xx, int8 yy);
	void stopWalk();
	void chatter();
	void remove();

private:
	Animation *_anim;

	int16 _oldX[2], _oldY[2];  // Last xy coords.
	Color _fgBubbleCol, _bgBubbleCol; // Foreground & background bubble colors.

	bool checkCollision();
	int8 getSign(int16 val);
	void homeStep();
};

class Animation {
public:
	friend class AnimationType;

	static const byte kSpriteNumbMax = 5; // current max no. of sprites

	enum Proc {
		kProcNone = 0,
		kProcFollowAvvyY,
		kProcBackAndForth,
		kProcFaceAvvy,
		kProcArrow,
		kProcGrabAvvy,
		kProcFollowAvvy
	};

	AnimationType *_sprites[kSpriteNumbMax];

	Animation(AvalancheEngine *vm);
	~Animation();

	void animLink();
	void resetAnims();
	void callSpecial(uint16 which);
	void catacombMove(byte ped);
	void stopWalking();
	void setMoveSpeed(byte t, Direction dir);
	void appearPed(byte sprNum, byte pedNum);
	bool inField(byte which);
	bool nearDoor();
	void updateSpeed();
	void handleMoveKey(const Common::Event &event);
	void hideInCupboard();

	// These 2 functions are responsible for playing the thunder animation when the player swears too much.
	void drawLightning(int16 x1, int16 y1, int16 x2, int16 y2);
	void thunder();

	void wobble();

	void setDirection(Direction dir);
	void setOldDirection(Direction dir);
	Direction getDirection();
	Direction getOldDirection();

	void setAvvyClothes(int id);
	int getAvvyClothes();

	void resetVariables();
	void synchronize(Common::Serializer &sz);
private:
	Direction _direction; // The direction Avvy is currently facing.
	Direction _oldDirection;
	static const int32 kCatacombMap[8][8];
	bool _arrowTriggered; // And has the arrow been triggered?
	bool _mustExclaim;
	byte _geidaSpin, _geidaTime; // For the making "Geida dizzy" joke.
	uint16 _sayWhat;

	AvalancheEngine *_vm;

	byte checkFeet(int16 x1, int16 x2, int16 oy, int16 y, byte yl);
	byte geidaPed(byte ped);
	void dawnDelay();

	void grabAvvy(byte tripnum);
	void arrowProcs(byte tripnum);

	// Different movements for NPCs:
	void followAvalotY(byte tripnum);
	void backAndForth(byte tripnum);
	void faceAvvy(byte tripnum);

	// Movements for Homing NPCs: Spludwick and Geida.
	void spin(Direction dir, byte &tripnum);
	void takeAStep(byte &tripnum);
	void follow(byte tripnum);

	void drawSprites();
};

} // End of namespace Avalanche.

#endif // AVALANCHE_ANIMATION_H
