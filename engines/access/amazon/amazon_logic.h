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

#ifndef ACCESS_AMAZON_LOGIC_H
#define ACCESS_AMAZON_LOGIC_H

#include "common/scummsys.h"
#include "access/scripts.h"
#include "access/asurface.h"
#include "access/amazon/amazon_resources.h"

namespace Access {
namespace Amazon {

class AmazonEngine;

#define PAN_SIZE 32

class AmazonManager {
protected:
	AmazonEngine *_vm;
public:
	AmazonManager(AmazonEngine *vm) : _vm(vm) {}
};

class PannedScene : public AmazonManager {
	struct PanEntry {
		SpriteResource *_pObject;
		int _pImgNum;
		int _pObjX;
		int _pObjY;
		int _pObjZ;
		int _pObjXl;
		int _pObjYl;
	};
protected:
	int _xCount;
	int _xTrack;
	int _yTrack;
	int _zTrack;
	int _xCam;
	int _yCam;
	int _zCam;
	int _pNumObj;

	PanEntry _pan[PAN_SIZE];
public:
	PannedScene(AmazonEngine *vm);

	void pan();
};

class CampScene : public PannedScene {
protected:
	bool _skipStart;
public:
	CampScene(AmazonEngine *vm);

	void mWhileDoOpen();
};

class Opening : public CampScene {
private:
	int _pCount;

	void doTitle();
	void doCredit();
	void doCreditDemo();
	void scrollTitle();
	void doTent();
public:
	Opening(AmazonEngine *vm);

	void doIntroduction();
};

class Plane : public PannedScene {
public:
	int _pCount;
	Common::Point _position;
	int _planeCount;
	int _propCount;

	void doFlyCell();
	void doFallCell();
	void scrollFly();
	void scrollFall();
	void mWhileFly();
	void mWhileFall();
public:
	Plane(AmazonEngine *vm);
};

#define JUNGLE_SIZE 3
class Jungle : public CampScene {
private:
	void initJWalk2();
	void jungleMove();
	void scrollJWalk();

	int _jCnt[JUNGLE_SIZE];
	int _jungleX[JUNGLE_SIZE];
public:
	Jungle(AmazonEngine *vm);

	void mWhileJWalk();
	void mWhileJWalk2();
};

class Guard : public PannedScene {
private:
	int _guardCel;
	Common::Point _position;
	int _gCode1;
	int _gCode2;
	Common::Point _topLeft;
	Common::Point _bottomRight;
	int _xMid, _yMid;

	void chkVLine();
	void chkHLine();
	void setVerticalCode();
	void setHorizontalCode();
	void guardSee();
	void setGuardFrame();
public:
	Guard(AmazonEngine *vm);

	void doGuard();

	void setPosition(const Common::Point &pt);
};

class Cast : public PannedScene {
public:
	Cast(AmazonEngine *vm);

	void doCast(int param1);
};

class River : public PannedScene {
private:
	bool _chickenOutFl;
	const byte *_mapPtr;
	int _canoeVXPos;
	int _canoeMoveCount;
	int _canoeFrame;
	RiverStruct *_topList;
	RiverStruct *_botList;
	int _canoeDir;
	bool _saveRiver;
	bool _deathFlag;
	int _deathCount;
	int _deathType;
	int _maxHits;

	// Saved fields
	int _canoeLane;
	int _canoeYPos;
	int _hitCount;
	int _riverIndex;
	int _hitSafe;
	int _rScrollRow;
	int _rScrollCol;
	int _rScrollX;
	int _rScrollY;
	int _mapOffset;
	int _screenVertX;
	int _oldScrollCol;

	void initRiver();
	void resetPositions();
	void checkRiverPan();
	bool riverJumpTest();
	void riverSound();
	void moveCanoe();
	void moveCanoe2();
	void updateObstacles();
	void riverSetPhysX();
	bool checkRiverCollide();
	void plotRiver();
	void scrollRiver();
	void scrollRiver1();
	void setRiverPan();
public:
	River(AmazonEngine *vm);

	void doRiver();
	void mWhileDownRiver();

	void synchronize(Common::Serializer &s);
};

enum AntDirection { ANT_RIGHT = 0, ANT_LEFT = 1 };

class Ant : public AmazonManager {
private:
	AntDirection _antDirection;
	AntDirection _pitDirection;
	int _antCel;
	int _torchCel;
	int _pitCel;
	int _stabCel;
	Common::Point _antPos;
	bool _antDieFl;
	bool _antEatFl;
	bool _stabFl;
	Common::Point _pitPos;

	void plotTorchSpear(int indx, const int *&buf);
	void plotPit(int indx, const int *&buf);
	int antHandleRight(int indx, const int *&buf);
	int antHandleLeft(int indx, const int *&buf);
	int antHandleStab(int indx, const int *&buf);
public:
	Ant(AmazonEngine *vm);

	void doAnt();

	void synchronize(Common::Serializer &s);
};

class InactivePlayer : public ImageEntry {
public:
	SpriteResource *_altSpritesPtr;

	InactivePlayer() { _altSpritesPtr = nullptr; }
};

} // End of namespace Amazon

} // End of namespace Access

#endif /* ACCESS_AMAZON_LOGIC_H */
