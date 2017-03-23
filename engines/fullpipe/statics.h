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

#ifndef FULLPIPE_STATICS_H
#define FULLPIPE_STATICS_H

#include "fullpipe/gfx.h"

namespace Fullpipe {

class ExCommand;

class StepArray : public CObject {
	int _currPointIndex;
	Common::Point **_points;
	int _maxPointIndex;
	int _pointsCount;
	int _isEos;

  public:
	StepArray();
	~StepArray();
	void clear();

	int getCurrPointIndex() { return _currPointIndex; }
	int getPointsCount() { return _maxPointIndex; }

	Common::Point *getCurrPoint(Common::Point *point);
	Common::Point *getPoint(Common::Point *point, int index, int offset);
	bool gotoNextPoint();
	void insertPoints(Common::Point **points, int pointsCount);
};

class StaticPhase : public Picture {
 public:
	int16 _initialCountdown;
	int16 _countdown;
	int16 _field_68;
	int16 _field_6A;
	ExCommand *_exCommand;

  public:
	StaticPhase();
	virtual ~StaticPhase();

	virtual bool load(MfcArchive &file);

	ExCommand *getExCommand() { return _exCommand; }
};

class DynamicPhase : public StaticPhase {
 public:
	int _someX;
	int _someY;
	Common::Rect *_rect;
	int16 _field_7C;
	int16 _field_7E;
	int _dynFlags;

  public:
	DynamicPhase();
	DynamicPhase(DynamicPhase *src, bool reverse);
	virtual ~DynamicPhase();

	virtual bool load(MfcArchive &file);

	int getDynFlags() { return _dynFlags; }
};

class Statics : public DynamicPhase {
 public:
 	int16 _staticsId;
	Common::String _staticsName;
	Picture *_picture;

  public:
	Statics();
	Statics(Statics *src, bool reverse);
	virtual ~Statics();

	virtual bool load(MfcArchive &file);
	virtual void init();
	Statics *getStaticsById(int itemId);

	Common::Point *getSomeXY(Common::Point &p);
	Common::Point *getCenter(Common::Point *p);
};

class StaticANIObject;

class Movement : public GameObject {
  public:
	Common::Point _somePoint;
	int _lastFrameSpecialFlag;
	int _flipFlag;
	int _updateFlag1;
	Statics *_staticsObj1;
	Statics *_staticsObj2;
	int _mx;
	int _my;
	int _m2x;
	int _m2y;
	int _field_50;
	int _counterMax;
	int _counter;
	Common::Array<DynamicPhase *> _dynamicPhases;
	int _field_78;
	Common::Point **_framePosOffsets;
	Movement *_currMovement;
	int _field_84;
	DynamicPhase *_currDynamicPhase;
	int _field_8C;
	int _currDynamicPhaseIndex;
	int _field_94;

  public:
	Movement();
	virtual ~Movement();

	Movement(Movement *src, StaticANIObject *ani);
	Movement(Movement *src, int *flag1, int flag2, StaticANIObject *ani);

	virtual bool load(MfcArchive &file);
	bool load(MfcArchive &file, StaticANIObject *ani);

	Common::Point *getCurrDynamicPhaseXY(Common::Point &p);
	Common::Point *getCenter(Common::Point *p);
	Common::Point *getDimensionsOfPhase(Common::Point *p, int phaseIndex);

	Common::Point *calcSomeXY(Common::Point &p, int idx, int dynidx);

	void initStatics(StaticANIObject *ani);
	void updateCurrDynamicPhase();

	void setAlpha(int alpha);

	void setDynamicPhaseIndex(int index);
	DynamicPhase *getDynamicPhaseByIndex(int idx);

	int calcDuration();
	int countPhasesWithFlag(int maxidx, int flag);

	void removeFirstPhase();
	bool gotoNextFrame(void (*_callback1)(int, Common::Point *point, int, int), void (*callback2)(int *));
	bool gotoPrevFrame();
	void gotoFirstFrame();
	void gotoLastFrame();

	void loadPixelData();
	void freePixelData();

	void draw(bool flipFlag, int angle);
};

class StaticANIObject : public GameObject {
 public:
	Movement *_movement;
	Statics *_statics;
	int _shadowsOn;
	int16 _field_30;
	int16 _field_32;
	int _field_34;
	int _initialCounter;
	void (*_callback1)(int, Common::Point *point, int, int);
	void (*_callback2)(int *);
	Common::Array<Movement *> _movements;
	Common::Array<Statics *> _staticsList;
	StepArray _stepArray;
	int16 _field_96;
	int _messageQueueId;
	int _messageNum;
	int _animExFlag;
	int _counter;
	int _someDynamicPhaseIndex;

public:
	int16 _sceneId;

public:
	StaticANIObject();
	virtual ~StaticANIObject();
	StaticANIObject(StaticANIObject *src);

	virtual bool load(MfcArchive &file);

	void setOXY(int x, int y);
	Statics *getStaticsById(int id);
	Statics *getStaticsByName(const Common::String &name);
	Movement *getMovementById(int id);
	int getMovementIdById(int itemId);
	Movement *getMovementByName(const Common::String &name);
	Common::Point *getCurrDimensions(Common::Point &p);

	Common::Point *getSomeXY(Common::Point &p);

	void clearFlags();
	void setFlags40(bool state);
	bool isIdle();
	void setAlpha(int alpha);

	void deleteFromGlobalMessageQueue();
	bool queueMessageQueue(MessageQueue *msg);
	void restartMessageQueue(MessageQueue *msg);
	MessageQueue *getMessageQueue();
	bool trySetMessageQueue(int msgNum, int qId);
	void startMQIfIdle(int qId, int flag);

	void initMovements();
	void loadMovementsPixelData();
	void freeMovementsPixelData();
	void preloadMovements(MovTable *mt);

	void setSomeDynamicPhaseIndex(int val) { _someDynamicPhaseIndex = val; }
	void adjustSomeXY();

	bool startAnim(int movementId, int messageQueueId, int dynPhaseIdx);
	bool startAnimEx(int movid, int parId, int flag1, int flag2);
	void startAnimSteps(int movementId, int messageQueueId, int x, int y, Common::Point **points, int pointsCount, int someDynamicPhaseIndex);

	void hide();
	void show1(int x, int y, int movementId, int mqId);
	void show2(int x, int y, int movementId, int mqId);
	void playIdle();
	void update(int counterdiff);

	Statics *addReverseStatics(Statics *ani);
	void draw();
	void draw2();

	MovTable *countMovements();
	Common::Point *calcStepLen(Common::Point *p);
	void setSpeed(int speed);

	void updateStepPos();
	void stopAnim_maybe();
	Common::Point *calcNextStep(Common::Point *point);

	MessageQueue *changeStatics1(int msgNum);
	void changeStatics2(int objId);

	bool getPixelAtPos(int x, int y, uint32 *pixel, bool hitOnly = false);
	bool isPixelHitAtPos(int x, int y);
};

struct MovTable {
	int count;
	int16 *movs;

	MovTable() { count = 0; movs = 0; }
	~MovTable() { free(movs); }
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_STATICS_H */
