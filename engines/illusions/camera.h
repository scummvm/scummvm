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

#ifndef ILLUSIONS_CAMERA_H
#define ILLUSIONS_CAMERA_H

#include "illusions/graphics.h"
#include "common/rect.h"
#include "common/stack.h"

namespace Illusions {

class BackgroundInstance;

struct CameraState {
	int _cameraMode;
	//field_2 dw
	bool _paused;
	int16 _panSpeed;
	int _someX, _someY;
	Common::Point _currPan;
	int _panXShl, _panYShl;
	WRect _bounds;
	uint32 _panNotifyId;
	uint32 _time28;
	uint32 _panStartTime;
	uint32 _pauseStartTime;
	uint32 _time2E;
	Common::Point _currPan2;
	Common::Point _panTargetPoint;
	Common::Point _trackingLimits;
	Common::Point _centerPt;
	uint32 _panObjectId;
	Common::Point *_panToPositionPtr;
	uint _pointFlags;
	//field_4A dw
};

struct CameraModeStackItem {
	int _cameraMode;
	uint32 _panObjectId;
	int16 _panSpeed;
	Common::Point _panTargetPoint;
	uint32 _panNotifyId;
};

class Camera {
public:
	Camera(IllusionsEngine *vm);
	void clearStack();
	void set(Common::Point &panPoint, WidthHeight &dimensions);
	void panCenterObject(uint32 objectId, int16 panSpeed);
	void panTrackObject(uint32 objectId);
	void panToPoint(Common::Point pt, int16 panSpeed, uint32 panNotifyId);
	void panEdgeFollow(uint32 objectId, int16 panSpeed);
	void stopPan();
	void pause();
	void unpause();
	void pushCameraMode();
	void popCameraMode();
	void clearCameraModeStack();
	void update(uint32 currTime);
	void setBounds(Common::Point minPt, Common::Point maxPt);
	void setBoundsToDimensions(WidthHeight &dimensions);
	Common::Point getCurrentPan();
	Common::Point getScreenOffset();
	Common::Point getTrackingLimits();
	bool isAtPanLimit(int limitNum);
	void setActiveState(CameraState &state);
	void getActiveState(CameraState &state);
	void refreshPan(BackgroundInstance *backgroundItem, WidthHeight &dimensions);
protected:
	IllusionsEngine *_vm;
	CameraState _activeState;
	Common::FixedStack<CameraModeStackItem, 8> _stack;

	int16 _screenWidth, _screenHeight;
	int16 _screenMidX, _screenMidY;
	Common::Point _centerObjectTrackingLimits;
	Common::Point _trackObjectTrackingLimits;
	int16 _trackObjectTrackingLimitsPanSpeed;

	void updateMode1(uint32 currTime);
	void updateMode2(uint32 currTime);
	void updateMode3(uint32 currTime);
	bool updatePan(uint32 currTime);
	bool isPanFinished();
	Common::Point getPtOffset(Common::Point pt);
	void recalcPan(uint32 currTime);
	bool calcPointFlags(Common::Point &pt, WRect &rect, uint &outFlags);
	void clipPanTargetPoint();
	void init();
	void initDuckman();
	void initBBDOU();
};

} // End of namespace Illusions

#endif // ILLUSIONS_CAMERA_H
