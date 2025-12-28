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

#include "mediastation/actors/camera.h"
#include "mediastation/actors/stage.h"
#include "mediastation/actors/image.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

#include "common/util.h"

namespace MediaStation {

CameraActor::~CameraActor() {
	if (_parentStage != nullptr) {
		_parentStage->removeCamera(this);
		_parentStage->removeChildSpatialEntity(this);
	}
}

void CameraActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		_image = Common::SharedPtr<ImageAsset>(new ImageAsset);
		break;

	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderX:
		_offset.x = chunk.readTypedUint16();
		break;

	case kActorHeaderY:
		_offset.y = chunk.readTypedUint16();
		break;

	case kActorHeaderCameraViewportOrigin: {
		Common::Point origin = chunk.readTypedPoint();
		setViewportOrigin(origin);
		break;
	}

	case kActorHeaderCameraLensOpen:
		_lensOpen = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderCameraImageActor: {
		uint actorReference = chunk.readTypedUint16();
		Actor *referencedActor = g_engine->getActorById(actorReference);
		if (referencedActor == nullptr) {
			error("%s: Referenced actor %d doesn't exist or has not been read yet in this title", __func__, actorReference);
		}
		if (referencedActor->type() != kActorTypeCamera) {
			error("%s: Type mismatch of referenced actor %d", __func__, actorReference);
		}
		CameraActor *referencedImage = static_cast<CameraActor *>(referencedActor);
		_image = referencedImage->_image;
		break;
	}

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

void CameraActor::readChunk(Chunk &chunk) {
	BitmapHeader *bitmapHeader = new BitmapHeader(chunk);
	_image->bitmap = new Bitmap(chunk, bitmapHeader);
}

ScriptValue CameraActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kSpatialMoveToMethod:
	case kSpatialMoveToByOffsetMethod:
	case kSpatialCenterMoveToMethod:
		invalidateLocalBounds();
		returnValue = SpatialEntity::callMethod(methodId, args);
		invalidateLocalBounds();
		break;

	case kAddToStageMethod:
		assert(args.empty());
		addToStage();
		break;

	case kRemoveFromStageMethod: {
		bool stopPan = false;
		if (args.size() >= 1) {
			stopPan = args[0].asBool();
		}
		removeFromStage(stopPan);
		break;
	}

	case kAddedToStageMethod:
		assert(args.empty());
		returnValue.setToBool(_addedToStage);
		break;

	case kStartPanMethod: {
		assert(args.size() == 3);
		int16 deltaX = static_cast<uint16>(args[0].asFloat());
		int16 deltaY = static_cast<int16>(args[1].asFloat());
		double duration = args[2].asTime();
		_nextViewportOrigin = Common::Point(deltaX, deltaY) + _currentViewportOrigin;
		adjustCameraViewport(_nextViewportOrigin);
		startPan(deltaX, deltaY, duration);
		break;
	}

	case kStopPanMethod:
		assert(args.empty());
		stopPan();
		break;

	case kIsPanningMethod:
		assert(args.empty());
		returnValue.setToBool(_panState);
		break;

	case kViewportMoveToMethod: {
		assert(args.size() == 2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());
		_nextViewportOrigin = Common::Point(x, y);
		if (!_addedToStage) {
			_currentViewportOrigin = _nextViewportOrigin;
		} else {
			bool viewportMovedSuccessfully = processViewportMove();
			if (!viewportMovedSuccessfully) {
				startPan(0, 0, 0.0);
			}
		}
		break;
	}

	case kAdjustCameraViewportMethod: {
		assert(args.size() == 2);
		int16 xDiff = static_cast<int16>(args[0].asFloat());
		int16 yDiff = static_cast<int16>(args[1].asFloat());
		Common::Point viewportDelta(xDiff, yDiff);
		_nextViewportOrigin = getViewportOrigin() + viewportDelta;
		adjustCameraViewport(_nextViewportOrigin);
		if (!_addedToStage) {
			_currentViewportOrigin = _nextViewportOrigin;
		} else {
			bool viewportMovedSuccessfully = processViewportMove();
			if (!viewportMovedSuccessfully) {
				startPan(0, 0, 0.0);
			}
		}
		break;
	}

	case kAdjustCameraViewportSpatialCenterMethod: {
		assert(args.size() == 2);
		int16 xDiff = static_cast<int16>(args[0].asFloat());
		int16 yDiff = static_cast<int16>(args[1].asFloat());

		// Apply centering adjustment, which is indeed based on the entire camera actor's
		// bounds, not just the current viewport bounds.
		int16 centeredXDiff = xDiff - (getBbox().width() / 2);
		int16 centeredYDiff = yDiff - (getBbox().height() / 2);
		Common::Point viewportDelta(centeredXDiff, centeredYDiff);
		_nextViewportOrigin = getViewportOrigin() + viewportDelta;
		adjustCameraViewport(_nextViewportOrigin);
		if (!_addedToStage) {
			_currentViewportOrigin = _nextViewportOrigin;
		} else {
			bool viewportMovedSuccessfully = processViewportMove();
			if (!viewportMovedSuccessfully) {
				startPan(0, 0, 0.0);
			}
		}
		break;
	}

	case kSetCameraBoundsMethod: {
		assert(args.size() == 2);
		int16 width = static_cast<int16>(args[0].asFloat());
		int16 height = static_cast<int16>(args[1].asFloat());
		Common::Rect newBounds(_originalBoundingBox.origin(), width, height);

		// invalidateLocalBounds is already called in the setBounds call, but these extra calls are
		// in the original, so they are kept.
		invalidateLocalBounds();
		setBounds(newBounds);
		invalidateLocalBounds();
		break;
	}

	case kXViewportPositionMethod:
		assert(args.size() == 0);
		returnValue.setToFloat(getViewportOrigin().x);
		break;

	case kYViewportPositionMethod:
		assert(args.size() == 0);
		returnValue.setToFloat(getViewportOrigin().y);
		break;

	case kPanToMethod: {
		assert(args.size() >= 3);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());

		if (args.size() == 4) {
			uint panSteps = static_cast<uint>(args[2].asFloat());
			double duration = args[3].asFloat();
			panToByStepCount(x, y, panSteps, duration);
		} else if (args.size() == 3) {
			double duration = args[2].asFloat();
			panToByTime(x, y, duration);
		} else {
			error("%s: Incorrect number of args for method %s", __func__, builtInMethodToStr(methodId));
		}
		break;
	}

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
		break;
	}
	return returnValue;
}

void CameraActor::invalidateLocalBounds() {
	if (_parentStage != nullptr) {
		_parentStage->invalidateLocalBounds();
	}
}

void CameraActor::loadIsComplete() {
	SpatialEntity::loadIsComplete();
	if (_lensOpen) {
		addToStage();
	}

	if (_image != nullptr) {
		warning("%s: STUB: Camera image asset not handled yet", __func__);
	}
}

void CameraActor::addToStage() {
	if (_parentStage != nullptr) {
		_parentStage->addCamera(this);
		invalidateLocalBounds();
	}
}

void CameraActor::removeFromStage(bool shouldStopPan) {
	if (_parentStage != nullptr) {
		_parentStage->removeCamera(this);
		invalidateLocalBounds();
		_addedToStage = false;
		if (shouldStopPan && _panState != kCameraNotPanning) {
			stopPan();
		}
	}
}

void CameraActor::setViewportOrigin(const Common::Point &newViewpointOrigin) {
	_currentViewportOrigin = newViewpointOrigin;
}

Common::Point CameraActor::getViewportOrigin() {
	return _currentViewportOrigin;
}

Common::Rect CameraActor::getViewportBounds() {
	Common::Rect viewportBounds(getBbox());
	viewportBounds.moveTo(_currentViewportOrigin);
	return viewportBounds;
}

void CameraActor::drawUsingCamera(DisplayContext &displayContext, const Common::Array<SpatialEntity *> &entitiesToDraw) {
	Clip *currentClip = displayContext.currentClip();
	if (currentClip != nullptr) {
		Clip *previousClip = displayContext.previousClip();
		if (previousClip == nullptr) {
			currentClip->addToRegion(currentClip->_bounds);
		} else {
			*currentClip = *previousClip;
		}
	}

	Common::Rect cameraBounds = getBbox();
	displayContext.intersectClipWith(cameraBounds);
	displayContext.pushOrigin();

	Common::Point viewportOrigin = getViewportOrigin();
	Common::Point viewportOffset(
		-viewportOrigin.x + cameraBounds.left,
		-viewportOrigin.y + cameraBounds.top
	);
	displayContext._origin.x += viewportOffset.x;
	displayContext._origin.y += viewportOffset.y;

	if (_image != nullptr) {
		// TODO: Handle image asset stuff.
		warning("%s: Camera image asset not handled yet", __func__);
	}

	for (SpatialEntity *entityToDraw : entitiesToDraw) {
		if (entityToDraw->isVisible()) {
			drawObject(displayContext, displayContext, entityToDraw);
		}
	}

	displayContext.popOrigin();
	displayContext.emptyCurrentClip();
}

void CameraActor::drawObject(DisplayContext &sourceContext, DisplayContext &destContext, SpatialEntity *objectToDraw) {
	if (_parentStage == nullptr) {
		warning("%s: No parent stage", __func__);
		return;
	}

	objectToDraw->setAdjustedBounds(kWrapNone);
	Common::Rect visibleBounds = objectToDraw->getBbox();
	if (sourceContext.rectIsInClip(visibleBounds)) {
		objectToDraw->draw(destContext);
	}

	if (_parentStage->cylindricalX()) {
		warning("%s: CylindricalX not handled yet", __func__);
	}

	if (_parentStage->cylindricalY()) {
		warning("%s: CylindricalY not handled yet", __func__);
	}
	objectToDraw->setAdjustedBounds(kWrapNone);
}

void CameraActor::setXYDelta(uint xDelta, uint yDelta) {
	_panDelta.x = xDelta;
	_panDelta.y = yDelta;
	debugC(6, kDebugCamera, "%s: (%d, %d)", __func__, _panDelta.x, _panDelta.y);
}

void CameraActor::setXYDelta() {
	// If we have no parameters for setting the delta,
	// just set the delta to 1 in whatever direction we are going.
	if (_panStart.x < _panDest.x) {
		_panDelta.x = 1;
	} else if (_panDest.x < _panStart.x) {
		_panDelta.x = -1;
	}

	if (_panStart.y < _panDest.y) {
		_panDelta.y = 1;
	} else if (_panDest.y < _panStart.y) {
		_panDelta.y = -1;
	}
	debugC(6, kDebugCamera, "%s: (%d, %d)", __func__, _panDelta.x, _panDelta.y);
}

void CameraActor::panToByTime(int16 x, int16 y, double duration) {
	_panState = kCameraPanToByTime;
	_panStart = _currentViewportOrigin;
	_panDest = Common::Point(x, y);
	_panDuration = duration;
	_currentPanStep = 1;
	_startTime = g_system->getMillis();
	_nextPanStepTime = 0;
	debugC(6, kDebugCamera, "%s: panStart: (%d, %d); panDest: (%d, %d); panDuration: %f",
		__func__, _panStart.x, _panStart.y, _panDest.x, _panDest.y, _panDuration);
	setXYDelta();
	calcNewViewportOrigin();
}

void CameraActor::panToByStepCount(int16 x, int16 y, uint panSteps, double duration) {
	_panState = kCameraPanByStepCount;
	_panStart = _currentViewportOrigin;
	_panDest = Common::Point(x, y);
	_panDuration = duration;
	_currentPanStep = 1;
	_maxPanStep = panSteps;
	_startTime = g_system->getMillis();
	_nextPanStepTime = 0;
	debugC(6, kDebugCamera, "%s: panStart: (%d, %d); panDest: (%d, %d); panDuration: %f; maxPanStep: %d",
		__func__, _panStart.x, _panStart.y, _panDest.x, _panDest.y, _panDuration, _maxPanStep);
	setXYDelta();
	calcNewViewportOrigin();
}

void CameraActor::startPan(uint xOffset, uint yOffset, double duration) {
	_panState = kCameraPanningStarted;
	_panDuration = duration;
	_startTime = g_system->getMillis();
	_nextPanStepTime = 0;
	_currentPanStep = 0;
	_maxPanStep = 0;
	setXYDelta(xOffset, yOffset);
	debugC(6, kDebugCamera, "%s: xOffset: %u, yOffset: %u, duration: %f", __func__, xOffset, yOffset, duration);
}

void CameraActor::stopPan() {
	_panState = kCameraNotPanning;
	_panDuration = 0.0;
	_startTime = 0;
	_nextPanStepTime = 0;
	_currentPanStep = 0;
	_maxPanStep = 0;
	debugC(6, kDebugCamera, "%s: nextViewportOrigin: (%d, %d); actualViewportOrigin: (%d, %d)",
		__func__,  _nextViewportOrigin.x, _nextViewportOrigin.y, _currentViewportOrigin.x, _currentViewportOrigin.y);
}

bool CameraActor::continuePan() {
	bool panShouldContinue = true;
	if (_panState == kCameraPanningStarted) {
		if (_panDelta == Common::Point(0, 0)) {
			panShouldContinue = false;
		}
	} else {
		if (percentComplete() >= 1.0) {
			panShouldContinue = false;
		}
	}
	debugC(6, kDebugCamera, "%s: %s", __func__, panShouldContinue ? "true": "false");
	return panShouldContinue;
}

void CameraActor::process() {
	// Only process panning if we're actively panning.
	if (_panState == kCameraNotPanning) {
		return;
	}

	// Check if it's time for the next pan step.
	uint currentTime = g_system->getMillis() - _startTime;
	if (currentTime < _nextPanStepTime) {
		return;
	}

	debugC(7, kDebugCamera, "*** START PAN STEP ***");
	timerEvent();
	debugC(7, kDebugCamera, "*** END PAN STEP ***");
}

void CameraActor::timerEvent() {
	if (_parentStage != nullptr) {
		if (processViewportMove()) {
			processNextPanStep();
			if (continuePan()) {
				if (cameraWithinStage(_nextViewportOrigin)) {
					adjustCameraViewport(_nextViewportOrigin);

					// The original had logic to pre-load the items that were going to be scrolled
					// into view next, but since we load actors more all-at-once, we don't actually need this.
					// The calls that would be made are kept commented out.
					// Common::Rect advanceRect = getAdvanceRect();
					// _parentStage->preload(advanceRect);
				} else {
					runEventHandlerIfExists(kCameraPanAbortEvent);
					stopPan();
				}
			} else {
				bool success = true;
				if (_panState == kCameraPanToByTime) {
					_nextViewportOrigin = _panDest;
					adjustCameraViewport(_nextViewportOrigin);
					success = processViewportMove();
				}
				if (success) {
					runEventHandlerIfExists(kCameraPanEndEvent);
					stopPan();
				} else {
					Common::Rect currentBounds = getBbox();
					Common::Rect preloadBounds(_nextViewportOrigin, currentBounds.width(), currentBounds.height());
					_parentStage->preload(preloadBounds);
				}
			}
		} else {
			Common::Rect currentBounds = getBbox();
			Common::Rect preloadBounds(_nextViewportOrigin, currentBounds.width(), currentBounds.height());
			_parentStage->preload(preloadBounds);
		}
	}
}

bool CameraActor::processViewportMove() {
	bool isRectInMemory = true;
	if (_parentStage != nullptr) {
		Common::Rect boundsInViewport = getBbox();
		boundsInViewport.moveTo(_nextViewportOrigin);
		_parentStage->setCurrentCamera(this);
		isRectInMemory = _parentStage->isRectInMemory(boundsInViewport);
		if (isRectInMemory) {
			invalidateLocalBounds();
			setViewportOrigin(_nextViewportOrigin);
			invalidateLocalBounds();
		}
	}
	return isRectInMemory;
}

void CameraActor::processNextPanStep() {
	// If pan type includes per-step updates (4-arg pan in original engine),
	// advance the pan step counter. Then compute the new viewport origin
	// and notify any script handlers registered for the pan-step event.
	if (_panState == kCameraPanByStepCount) {
		_currentPanStep += 1;
	}

	calcNewViewportOrigin();
	runEventHandlerIfExists(kCameraPanStepEvent);

	uint stepDurationInMilliseconds = 20; // Visually smooth.
	_nextPanStepTime += stepDurationInMilliseconds;
}

void CameraActor::adjustCameraViewport(Common::Point &viewportToAdjust) {
	if (_parentStage == nullptr) {
		warning("%s: No parent stage", __func__);
		return;
	}

	if (_parentStage->cylindricalX()) {
		warning("%s: CylindricalX not handled yet", __func__);
	}

	if (_parentStage->cylindricalY()) {
		warning("%s: CylindricalY not handled yet", __func__);
	}
}

void CameraActor::calcNewViewportOrigin() {
	if (_panState == kCameraPanningStarted) {
		_nextViewportOrigin = _currentViewportOrigin + _panDelta;
		debugC(6, kDebugCamera, "%s: (%d, %d) [panDelta: (%d, %d)]",
			__func__, _nextViewportOrigin.x, _nextViewportOrigin.y, _panDelta.x, _panDelta.y);
	} else {
		// Interpolate from the start to the dest based on percent complete.
		double progress = percentComplete();
		double startX = static_cast<double>(_panStart.x);
		double endX = static_cast<double>(_panDest.x);
		double interpolatedX = startX + (endX - startX) * progress + 0.5;
		_nextViewportOrigin.x = static_cast<int16>(interpolatedX);

		double startY = static_cast<double>(_panStart.y);
		double endY = static_cast<double>(_panDest.y);
		double interpolatedY = startY + (endY - startY) * progress + 0.5;
		_nextViewportOrigin.y = static_cast<int16>(interpolatedY);
		debugC(6, kDebugCamera, "%s: (%d, %d) [panStart: (%d, %d); panDest: (%d, %d); percentComplete: %f]",
			__func__, _nextViewportOrigin.x, _nextViewportOrigin.y, _panStart.x, _panStart.y, _panDest.x, _panDest.y, progress);
	}
}

bool CameraActor::cameraWithinStage(const Common::Point &candidate) {
	if (_parentStage == nullptr) {
		return true;
	}

	bool result = true;
	// We can only be out of horizontal bounds if we have a requested delta and
	// are not doing X axis wrapping.
	bool canBeOutOfHorizontalBounds = !_parentStage->cylindricalX() && _panDelta.x != 0;
	if (canBeOutOfHorizontalBounds) {
		int16 candidateRightBoundary = getBbox().width() + candidate.x;
		bool cameraPastRightBoundary = _parentStage->extent().x < candidateRightBoundary;
		if (cameraPastRightBoundary) {
			result = false;
		} else if (candidate.x < 0) {
			result = false;
		}
		debugC(6, kDebugCamera, "%s: %s [rightBoundary: %d, extent: %d]", __func__, result ? "true" : "false", candidateRightBoundary, _parentStage->extent().x);
	}

	// We can only be out of vertical bounds if we have a requested delta and
	// are not doing Y axis wrapping.
	bool canBeOutOfVerticalBounds = !_parentStage->cylindricalY() && _panDelta.y != 0;
	if (canBeOutOfVerticalBounds) {
		int16 candidateBottomBoundary = getBbox().height() + candidate.y;
		bool cameraPastBottomBoundary = _parentStage->extent().y < candidateBottomBoundary;
		if (cameraPastBottomBoundary) {
			result = false;
		} else if (candidate.y < 0) {
			result = false;
		}
		debugC(6, kDebugCamera, "%s: %s [bottomBoundary: %d, extent: %d]", __func__, result ? "true" : "false", candidateBottomBoundary, _parentStage->extent().y);
	}
	return result;
}

double CameraActor::percentComplete() {
	double percentValue = 0.0;
	switch (_panState) {
	case kCameraPanByStepCount: {
		percentValue = static_cast<double>(_maxPanStep - _currentPanStep) / static_cast<double>(_maxPanStep);
		percentValue = 1.0 - percentValue;
		break;
	}

	case kCameraPanToByTime: {
		const double MILLISECONDS_IN_ONE_SECOND = 1000.0;
		uint currentRuntime = g_system->getMillis();
		uint elapsedTime = currentRuntime - _startTime;
		double elapsedSeconds = elapsedTime / MILLISECONDS_IN_ONE_SECOND;
		percentValue = elapsedSeconds / _panDuration;
		break;
	}

	default:
		percentValue = 0.0;
		break;
	}

	percentValue = CLIP<double>(percentValue, 0.0, 1.0);
	return percentValue;
}

} // End of namespace MediaStation
