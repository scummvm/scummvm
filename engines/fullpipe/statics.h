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

#ifndef FULLPIPE_STATICS_H
#define FULLPIPE_STATICS_H

#include "fullpipe/gfx.h"

namespace Fullpipe {

class CStepArray : public CObject {
	int _currPointIndex;
	Common::Point **_points;
	int _maxPointIndex;
	int _pointsCount;
	int _isEos;

  public:
	CStepArray();
	~CStepArray();
	void clear();

	int getCurrPointIndex() { return _currPointIndex; }
};

class StaticPhase : public Picture {
	int16 _initialCountdown;
	int16 _countdown;
	int16 _field_68;
	int16 _field_6A;
	ExCommand *_exCommand;

  public:
	StaticPhase();
	virtual bool load(MfcArchive &file);
};

class DynamicPhase : public StaticPhase {
	friend class Movement;

	int _someX;
	int _someY;
	Common::Rect *_rect;
	int16 _field_7C;
	int16 _field_7E;
	int _flags;

  public:
	DynamicPhase();
	virtual bool load(MfcArchive &file);
};

class Statics : public DynamicPhase {
	friend class StaticANIObject;
	friend class Movement;

 	int16 _staticsId;
	int16 _field_86;
	char *_staticsName;
	Picture *_picture;

  public:
	Statics();
	virtual bool load(MfcArchive &file);
	Statics *getStaticsById(int itemId);
};

class StaticANIObject;

class Movement : public GameObject {
	int _field_24;
	int _field_28;
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
	CPtrList _dynamicPhases;
	int _field_78;
	Common::Point **_framePosOffsets;
	Movement *_currMovementObj;
	int _field_84;
	DynamicPhase *_currDynamicPhase;
	int _field_8C;
	int _currDynamicPhaseIndex;
	int _field_94;

  public:
	Movement();
	virtual bool load(MfcArchive &file);
	bool load(MfcArchive &file, StaticANIObject *ani);

	Common::Point *getCurrDynamicPhaseXY(Common::Point &p);

	void initStatics(StaticANIObject *ani);
	void updateCurrDynamicPhase();

	void loadPixelData();
};

class StaticANIObject : public GameObject {
	friend class FullpipeEngine;

 protected:
	Movement *_movementObj;
	Statics *_staticsObj;
	int _shadowsOn;
	int16 _field_30;
	int16 _field_32;
	int _field_34;
	int _initialCounter;
	int _callback1;
	int _callback2;
	CPtrList _movements;
	CPtrList _staticsList;
	CStepArray _stepArray;
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
	virtual bool load(MfcArchive &file);

	void setOXY(int x, int y);
	Statics *getStaticsById(int id);
	Movement *getMovementById(int id);
	Movement *getMovementByName(char *name);

	void clearFlags();
	bool isIdle();

	void deleteFromGlobalMessageQueue();

	void loadMovementsPixelData();

	Statics *addStatics(Statics *ani);
	void draw();
	void draw2();
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_STATICS_H */
