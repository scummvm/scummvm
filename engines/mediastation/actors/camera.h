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

#ifndef MEDIASTATION_CAMERA_H
#define MEDIASTATION_CAMERA_H

#include "mediastation/actor.h"
#include "mediastation/graphics.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

enum CameraPanState {
	kCameraNotPanning = 0,
	kCameraPanningStarted = 1,
	// We pan for a certain total amount of time.
	kCameraPanToByTime = 2,
	// We pan for a certain number of steps, waiting a given time between each step.
	kCameraPanByStepCount = 3
};

struct ImageAsset;

// A Camera's main purpose is panning around a stage that is too large to fit on screen all at once.
class CameraActor : public SpatialEntity, public ChannelClient {
public:
	CameraActor() : SpatialEntity(kActorTypeCamera) {};
	~CameraActor();

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual void readChunk(Chunk &chunk) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void loadIsComplete() override;
	virtual void process() override;

	Common::Point getViewportOrigin();
	Common::Rect getViewportBounds();
	virtual void invalidateLocalBounds() override;

	void drawUsingCamera(DisplayContext &displayContext, const Common::Array<SpatialEntity *> &entitiesToDraw);

private:
	bool _lensOpen = false;
	bool _addedToStage = false;
	double _panDuration = 0.0;
	uint _currentPanStep = 0;
	uint _maxPanStep = 0;
	uint _startTime = 0;
	uint _nextPanStepTime = 0;
	CameraPanState _panState = kCameraNotPanning;
	Common::Point _offset;
	Common::Point _currentViewportOrigin;
	Common::Point _nextViewportOrigin;
	Common::Point _panStart;
	Common::Point _panDest;
	Common::Point _panDelta;
	Common::SharedPtr<ImageAsset> _image;
	DisplayContext _displayContext;

	void addToStage();
	void removeFromStage(bool stopPan);
	void setViewportOrigin(const Common::Point &newViewportOrigin);
	void drawObject(DisplayContext &sourceContext, DisplayContext &destContext, SpatialEntity *objectToDraw);
	void setXYDelta(uint xDelta, uint yDelta);
	void setXYDelta();
	bool cameraWithinStage(const Common::Point &candidate);

	void panToByTime(int16 x, int16 y, double duration);
	void panToByStepCount(int16 x, int16 y, uint maxPanStep, double duration);
	void startPan(uint xOffset, uint yOffset, double duration);
	void stopPan();
	bool continuePan();

	void timerEvent();
	bool processViewportMove();
	void processNextPanStep();
	void adjustCameraViewport(Common::Point &viewportToAdjust);
	void calcNewViewportOrigin();
	double percentComplete();
};

} // End of namespace MediaStation

#endif
