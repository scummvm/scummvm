/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NGI_STATICS_H
#define NGI_STATICS_H

#include "ngi/gfx.h"

namespace NGI {

class Command;

class StepArray : public CObject {
	int _currPointIndex;
	PointList _points;
	bool _isEos;

  public:
	StepArray();
	void clear();

	int getCurrPointIndex() const { return _currPointIndex; }
	int getPointsCount() const { return _points.size(); }

	Common::Point getCurrPoint() const;
	Common::Point getPoint(int index, int offset) const;
	bool gotoNextPoint();
	void insertPoints(const PointList &points);
};

class StaticPhase : public Texture {
 public:
	int16 _delay;     // Extra animation ticks in binary projects.
	int16 _countdown;
	int16 _messageNum;
	int16 _field_6A;
	Common::ScopedPtr<Command> _command;

  public:
	StaticPhase();

	bool load(MfcArchive &file) override;

	virtual Common::String toXML();

	Command *getCommand() { return _command.get(); }
};

class Phase : public StaticPhase {
 public:
	int _someX;
	int _someY;
	// XML phases can add both milliseconds and multiples of the movement's loop delay.
	int _delayMs = 0;
	int _delayRelative = 0;
	uint16 _soundId = 0;
	bool _loopSound = false;
	Common::Rect _rect;
	int16 _field_7C;
	int16 _field_7E;
	int _flags;

  public:
	Phase();
	Phase(Phase *src, bool reverse);
	void setPNG(const Common::Path &path, const Dims &size);
	void enter(int objectId);

	bool load(MfcArchive &file) override;

	Common::String toXML() override;

	int getDynFlags() { return _flags; }
};

class Statics : public Phase {
 public:
 	int16 _staticsId;
	Common::String _staticsName;
	Texture _picture;

  public:
	Statics();
	Statics(Statics *src, bool reverse);

	bool load(MfcArchive &file) override;
	void init() override;
	Statics *getStaticsById(int itemId);

	Common::Point getSomeXY() const;
	Common::Point getCenter() const;
};

class AniObject;

class Movement : public GameObject {
  public:
	Common::Point _somePoint;
	int _lastFrameSpecialFlag;
	int _flipFlag;
	int _updateFlag1;
	Statics *_prevStatic;
	Statics *_nextStatic;
	bool _mirrorPrev = false;
	bool _mirrorNext = false;
	int _prevStepX;
	int _prevStepY;
	int _nextStepX;
	int _nextStepY;
	int _link;
	int _loopDelay;
	int _counter;
	/** a confusing mix of owned and unowned items */
	Common::Array<Phase *> _phases;
	int _field_78;
	PointList _framePosOffsets;
	Movement *_currMovement;
	int _field_84;
	Phase *_curPhase;
	int _field_8C;
	int _curFrame;
	int _field_94;

  public:
	Movement();
	~Movement() override;

	Movement(Movement *src, AniObject *ani);
	Movement(Movement *src, int *flag1, int flag2, AniObject *ani);

	bool load(MfcArchive &file) override;
	bool load(MfcArchive &file, AniObject *ani);

	Common::Point getCurrDynamicPhaseXY() const;
	Common::Point getCenter() const;
	Dims getPhaseSize(int phaseIndex) const;

	Common::Point getMovementSize(int idx, int dynidx);

	void initStatics(AniObject *ani);
	void updateCurrDynamicPhase();

	void setAlpha(int alpha);

	void setCurFrame(int index);
	Phase *getPhase(int idx);

	int calcDuration();
	int countPhasesWithFlag(int maxidx, int flag);

	void removeFirstPhase();
	bool setNext(void (*_callback1)(int, Common::Point *point, int, int), void (*callback2)(int *), bool mirror = false);
	bool gotoPrevFrame();
	void gotoFirstFrame();
	void setLast();

	void load();
	void unload();

	void draw(bool flipFlag, int angle);
};

class AniObject : public GameObject {
 public:
	Movement *_movement;
	Statics *_statics;
	int _shadowsOn;
	int16 _encodedRotation;
	int16 _field_32;
	int _field_34;
	int _initialCounter;
	void (*_callback1)(int, Common::Point *point, int, int);
	void (*_callback2)(int *);
	/** list items are owned */
	Common::Array<Movement *> _movements;
	/** list items are owned */
	Common::Array<Statics *> _staticsList;
	StepArray _stepArray;
	int16 _field_96;
	int _messageQueueId;
	int _messageNum;
	int _animExFlag;
	int _counter;
	int _targetFrame;

public:
	int16 _sceneId;

public:
	AniObject();
	~AniObject() override;
	AniObject(AniObject *src);

	bool load(MfcArchive &file) override;

	void setPos(int x, int y);
	Statics *getStatic(int id);
	Statics *getStaticsByName(const Common::String &name);
	Movement *getMovement(int id);
	int getMovementIdById(int itemId) const;
	Movement *getMovementByName(const Common::String &name);
	Common::Point getFullSize() const;

	Common::Point getSomeXY() const;

	void clearFlags();
	void setFlags40(bool state);
	bool isIdle();
	void setAlpha(int alpha);

	void stopQueue();
	bool setQueue(MessageQueue *msg);
	void restartMessageQueue(MessageQueue *msg);
	MessageQueue *getMessageQueue();
	bool trySetMessageQueue(int msgNum, int qId);
	void startMQIfIdle(int qId, int flag);

	void initMovements();
	void loadMovements();
	void unloadMovements();
	void preloadMovements(MovTable *mt);

	void setTargetFrame(int val) { _targetFrame = val; }
	void adjustSomeXY();

	bool startMovement(int movementId, int messageQueueId, int dynPhaseIdx, bool mirror = false);
	bool startAnimEx(int movid, int parId, int flag1, int flag2);
	void startSliding(int movementId, int messageQueueId, int x, int y, const PointList &points, int someDynamicPhaseIndex);

	void hide();
	void show1(int x, int y, int movementId, int mqId);
	void show2(int x, int y, int movementId, int mqId);
	void playIdle();
	bool update(int counterdiff);

	Statics *addReverseStatics(Statics *ani);
	void render();
	void draw2();

	/** ownership of returned object is transferred to caller */
	MovTable *countMovements();
	Common::Point *calcStepLen(Common::Point *p);
	void setSpeed(int speed);

	void updateStepPos();
	void stopMovement();
	Common::Point *calcNextStep(Common::Point *point);

	MessageQueue *moveToStatic(int msgNum);
	void setStatic(int objId);

	bool getPixelAtPos(int x, int y, uint32 *pixel, bool hitOnly = false);
	bool hit(int x, int y);
};

} // End of namespace NGI

#endif /* NGI_STATICS_H */
