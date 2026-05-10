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
	delete _childrenWithOverlaySurface;
	_childrenWithOverlaySurface = nullptr;
	_childrenWithOverlayContext._destImage = nullptr;
}

void CameraActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		_overlayImage = Common::SharedPtr<ImageAsset>(new ImageAsset);
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
		CameraActor *referencedCamera = static_cast<CameraActor *>(g_engine->getImtGod()->getActorByIdAndType(actorReference, kActorTypeCamera));
		_overlayImage = referencedCamera->_overlayImage;
		break;
	}

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

void CameraActor::readChunk(Chunk &chunk) {
	ImageInfo bitmapHeader(chunk);
	_overlayImage->bitmap = new PixMapImage(chunk, bitmapHeader);
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
		ARGCOUNTCHECK(0);
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
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_addedToStage);
		break;

	case kStartPanMethod: {
		ARGCOUNTCHECK(3);
		int16 deltaX = static_cast<int16>(args[0].asFloat());
		int16 deltaY = static_cast<int16>(args[1].asFloat());
		double duration = args[2].asFloatOrTime();
		_nextViewportOrigin = Common::Point(deltaX, deltaY) + _currentViewportOrigin;
		adjustCameraViewport(_nextViewportOrigin);
		startPan(deltaX, deltaY, duration);
		break;
	}

	case kStopPanMethod:
		ARGCOUNTCHECK(0);
		stopPan();
		break;

	case kIsPanningMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_panState);
		break;

	case kViewportMoveToMethod: {
		ARGCOUNTCHECK(2);
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
		ARGCOUNTCHECK(2);
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
		ARGCOUNTCHECK(2);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());

		// Apply centering adjustment, which is indeed based on the entire camera actor's
		// bounds, not just the current viewport bounds.
		int16 centeredX = x - (getBbox().width() / 2);
		int16 centeredY = y - (getBbox().height() / 2);
		_nextViewportOrigin = Common::Point(centeredX, centeredY);
		debugC(6, kDebugCamera, "%s: currentViewportOrigin: (%d, %d); nextViewportOrigin: (%d, %d)",
			__func__, _currentViewportOrigin.x, _currentViewportOrigin.y, _nextViewportOrigin.x, _nextViewportOrigin.y);

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
		ARGCOUNTCHECK(2);
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
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(getViewportOrigin().x);
		break;

	case kYViewportPositionMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(getViewportOrigin().y);
		break;

	case kPanToMethod: {
		ARGCOUNTRANGE(3, 4);
		int16 x = static_cast<int16>(args[0].asFloat());
		int16 y = static_cast<int16>(args[1].asFloat());

		if (args.size() == 4) {
			uint panSteps = static_cast<uint>(args[2].asFloat());
			double duration = args[3].asFloatOrTime();
			panToByStepCount(x, y, panSteps, duration);
		} else {
			double duration = args[2].asFloatOrTime();
			panToByTime(x, y, duration);
		}
		break;
	}

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}
	return returnValue;
}

void CameraActor::onEvent(const ActorEvent &event) {
	runScriptResponseIfExists(event.type);
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

	if (_overlayImage != nullptr) {
		// Create the intermediate surface where we'll draw the actors and the overlay.
		ImageInfo imageInfo;
		imageInfo._dimensions = Common::Point(getBbox().width(), getBbox().height());
		imageInfo._stride = getBbox().width();
		_childrenWithOverlaySurface = new PixMapImage(imageInfo);
		_childrenWithOverlayContext._destImage = &_childrenWithOverlaySurface->_image;
		_childrenWithOverlayContext.verifyClipSize();

		// Mark this whole region dirty.
		Region region;
		Common::Rect cameraRect(0, 0, getBbox().width(), getBbox().height());
		region.addRect(cameraRect);
		_childrenWithOverlayContext.addClip();
		_childrenWithOverlayContext.setClipTo(region);
	}
}

void CameraActor::addToStage() {
	if (_parentStage != nullptr) {
		_parentStage->addCamera(this);
		invalidateLocalBounds();
		_addedToStage = true;
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

void CameraActor::drawUsingCamera(DisplayContext &destContext, const Common::Array<SpatialEntity *> &entitiesToDraw) {
	// Establish the initial clipping region.
	Clip *currentClip = destContext.currentClip();
	if (currentClip != nullptr) {
		Clip *previousClip = destContext.previousClip();
		if (previousClip == nullptr) {
			// Initialize the clip.
			currentClip->addToRegion(currentClip->_bounds);
		} else {
			// Copy the previous clip to the current clip.
			*currentClip = *previousClip;
		}
	}

	destContext.intersectClipWith(getBbox());
	destContext.pushOrigin();
	Common::Point viewportOrigin = getViewportOrigin();
	destContext._origin += (getBbox().origin() - viewportOrigin);

	if (_overlayImage != nullptr) {
		// Make sure we are ready to draw the overlay image.
		_childrenWithOverlayContext.pushOrigin();
		_childrenWithOverlayContext._origin -= _offset;
		_childrenWithOverlayContext._origin -= viewportOrigin;
	}

	for (SpatialEntity *entityToDraw : entitiesToDraw) {
		debugCN(6, kDebugGraphics, "[%s] %s: %s (viewport: %d, %d) (bounds: %d, %d, %d, %d) ", debugName(), __func__, entityToDraw->debugName(),
			_currentViewportOrigin.x, _currentViewportOrigin.y, PRINT_RECT(entityToDraw->getBbox()));

		if (entityToDraw->isVisible()) {
			if (_overlayImage == nullptr) {
				// Draw this image directly to the provided display context.
				debugC(6, kDebugGraphics, "(no overlay)");
				invalidateLocalBounds();
				drawObject(destContext, destContext, entityToDraw);
			} else {
				// Draw this image to our internal display context, so we can apply the
				// overlay to the drawn items afterward.
				debugC(6, kDebugGraphics, "(overlay)");
				drawObject(destContext, _childrenWithOverlayContext, entityToDraw);
			}
		} else {
			debugC(6, kDebugGraphics, "(not visible)");
		}
	}

	if (_overlayImage != nullptr) {
		// Now actually apply the overlay.
		destContext._origin += _offset;
		g_engine->getDisplayManager()->imageDeltaBlit(
			getViewportOrigin(), Common::Point(0, 0), _overlayImage->bitmap, _childrenWithOverlaySurface, 1.0, &destContext
		);

		_childrenWithOverlayContext.popOrigin();
	}

	destContext.popOrigin();
	destContext.emptyCurrentClip();
}

void CameraActor::drawObject(DisplayContext &sourceContext, DisplayContext &destContext, SpatialEntity *objectToDraw) {
	if (_parentStage == nullptr) {
		return;
	}

	// Draw object without any wrapping.
	objectToDraw->setAdjustedBounds(kWrapNone);
	Common::Rect visibleBounds = objectToDraw->getBbox();
	if (sourceContext.rectIsInClip(visibleBounds)) {
		objectToDraw->draw(destContext);
	}

	// Draw object with only cylindrical X wrapping if required.
	if (_parentStage->cylindricalX()) {
		objectToDraw->setAdjustedBounds(kWrapRight);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}

		objectToDraw->setAdjustedBounds(kWrapLeft);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}
	}

	// Draw object with only cylindrical Y wrapping if required.
	if (_parentStage->cylindricalY()) {
		objectToDraw->setAdjustedBounds(kWrapDown);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}

		objectToDraw->setAdjustedBounds(kWrapUp);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}
	}

	// Draw object with both cylindrical X and cylindrical Y wrapping if required.
	if (_parentStage->cylindricalX() && _parentStage->cylindricalY()) {
		objectToDraw->setAdjustedBounds(kWrapRightDown);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}

		objectToDraw->setAdjustedBounds(kWrapLeftUp);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}

		objectToDraw->setAdjustedBounds(kWrapLeftDown);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}

		objectToDraw->setAdjustedBounds(kWrapRightUp);
		visibleBounds = objectToDraw->getBbox();
		if (sourceContext.rectIsInClip(visibleBounds)) {
			objectToDraw->draw(destContext);
		}
	}

	objectToDraw->setAdjustedBounds(kWrapNone);
}

void CameraActor::setXYDelta(uint xDelta, uint yDelta) {
	_panDelta.x = xDelta;
	_panDelta.y = yDelta;
	debugC(6, kDebugCamera, "[%s] %s: (%d, %d)", debugName(), __func__, _panDelta.x, _panDelta.y);
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
	debugC(6, kDebugCamera, "[%s] %s: (%d, %d)", debugName(), __func__, _panDelta.x, _panDelta.y);
}

void CameraActor::panToByTime(int16 x, int16 y, double duration) {
	_panState = kCameraPanToByTime;
	_panStart = _currentViewportOrigin;
	_panDest = Common::Point(x, y);
	_totalPanDuration = duration;
	_currentPanStep = 1;
	_startTime = g_engine->getTotalPlayTime();
	debugC(6, kDebugCamera, "[%s] %s: panStart: (%d, %d); panDest: (%d, %d); panDuration: %f",
		debugName(), __func__, _panStart.x, _panStart.y, _panDest.x, _panDest.y, _totalPanDuration);
	setXYDelta();
	calcNewViewportOrigin();

	g_engine->getTimerService()->startTimer(_timer, _durationBetweenStepEvents);
}

void CameraActor::panToByStepCount(int16 x, int16 y, uint panSteps, double duration) {
	_panState = kCameraPanByStepCount;
	_panStart = _currentViewportOrigin;
	_panDest = Common::Point(x, y);
	_durationBetweenStepEvents = duration;
	_currentPanStep = 1;
	_maxPanStep = panSteps;
	debugC(6, kDebugCamera, "[%s] %s: panStart: (%d, %d); panDest: (%d, %d); panDuration: %f; maxPanStep: %d",
		debugName(), __func__, _panStart.x, _panStart.y, _panDest.x, _panDest.y, _totalPanDuration, _maxPanStep);
	setXYDelta();
	calcNewViewportOrigin();
	g_engine->getTimerService()->startTimer(_timer, _durationBetweenStepEvents);
}

void CameraActor::startPan(uint xOffset, uint yOffset, double duration) {
	_panState = kCameraPanningStarted;
	g_engine->getTimerService()->startTimer(_timer, duration);
	_durationBetweenStepEvents = duration;
	setXYDelta(xOffset, yOffset);

	_currentPanStep = 0;
	_maxPanStep = 0;
	debugC(6, kDebugCamera, "[%s] %s: xOffset: %u, yOffset: %u, duration: %f", debugName(), __func__, xOffset, yOffset, duration);
}

void CameraActor::stopPan() {
	_panState = kCameraNotPanning;
	g_engine->getTimerService()->stopTimer(_timer);

	_totalPanDuration = 0.0;
	_startTime = 0;
	_currentPanStep = 0;
	_maxPanStep = 0;
	debugC(6, kDebugCamera, "[%s] %s: nextViewportOrigin: (%d, %d); actualViewportOrigin: (%d, %d)",
		debugName(), __func__,  _nextViewportOrigin.x, _nextViewportOrigin.y, _currentViewportOrigin.x, _currentViewportOrigin.y);
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
	debugC(6, kDebugCamera, "[%s] %s: %s", debugName(), __func__, panShouldContinue ? "true" : "false");
	return panShouldContinue;
}

void CameraActor::timerEvent(const TimerEvent &event) {
	if (_parentStage != nullptr) {
		if (processViewportMove()) {
			processNextPanStep();
			if (continuePan()) {
				if (cameraWithinStage(_nextViewportOrigin)) {
					adjustCameraViewport(_nextViewportOrigin);
					Common::Rect advanceRect = getAdvanceRect();
					_parentStage->preload(advanceRect, false);
					g_engine->getTimerService()->startTimer(_timer, _durationBetweenStepEvents);
				} else {
					ActorEvent actorEvent(_id, kCameraPanAbortEvent);
					g_engine->getEventLoop()->queueEvent(actorEvent);
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
					ActorEvent actorEvent(_id, kCameraPanEndEvent);
					g_engine->getEventLoop()->queueEvent(actorEvent);
					stopPan();
				} else {
					Common::Rect currentBounds = getBbox();
					Common::Rect preloadBounds(_nextViewportOrigin, currentBounds.width(), currentBounds.height());
					_parentStage->preload(preloadBounds, false);
					g_engine->getTimerService()->startTimer(_timer, _durationBetweenStepEvents);
				}
			}
		} else {
			Common::Rect currentBounds = getBbox();
			Common::Rect preloadBounds(_nextViewportOrigin, currentBounds.width(), currentBounds.height());
			_parentStage->preload(preloadBounds, false);
			g_engine->getTimerService()->startTimer(_timer, _durationBetweenStepEvents);
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
	// and notify any script responses registered for the pan-step event.
	if (_panState == kCameraPanByStepCount) {
		_currentPanStep += 1;
	}

	calcNewViewportOrigin();
	ActorEvent event(_id, kCameraPanStepEvent);
	g_engine->getEventLoop()->queueEvent(event);
}

void CameraActor::adjustCameraViewport(Common::Point &viewportToAdjust) {
	if (_parentStage == nullptr) {
		return;
	}

	Common::Point stageExtent = _parentStage->extent();

	// Normalize viewport position for cylindrical wrapping.
	// When the viewport scrolls beyond the stage boundaries,
	// wrap it back to the opposite edge to create seamless infinite scrolling.
	if (_parentStage->cylindricalX()) {
		if (viewportToAdjust.x >= stageExtent.x) {
			viewportToAdjust.x -= stageExtent.x;
		} else if (viewportToAdjust.x < 0) {
			viewportToAdjust.x += stageExtent.x;
		}
	}

	if (_parentStage->cylindricalY()) {
		if (viewportToAdjust.y >= stageExtent.y) {
			viewportToAdjust.y -= stageExtent.y;
		} else if (viewportToAdjust.y < 0) {
			viewportToAdjust.y += stageExtent.y;
		}
	}
}

void CameraActor::calcNewViewportOrigin() {
	if (_panState == kCameraPanningStarted) {
		_nextViewportOrigin = _currentViewportOrigin + _panDelta;
		debugC(6, kDebugCamera, "[%s] %s: (%d, %d) [panDelta: (%d, %d)]",
			debugName(), __func__, _nextViewportOrigin.x, _nextViewportOrigin.y, _panDelta.x, _panDelta.y);
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
		debugC(6, kDebugCamera, "[%s] %s: (%d, %d) [panStart: (%d, %d); panDest: (%d, %d); percentComplete: %f]",
			debugName(), __func__, _nextViewportOrigin.x, _nextViewportOrigin.y, _panStart.x, _panStart.y, _panDest.x, _panDest.y, progress);
	}
}

bool CameraActor::cameraWithinStage(const Common::Point &candidate) {
	bool result = true;
	if (_parentStage == nullptr) {
		return result;
	}

	// We can only be out of horizontal bounds if we have a requested delta and
	// are not doing X wrapping.
	bool canBeOutOfHorizontalBounds = !_parentStage->cylindricalX() && _panDelta.x != 0;
	if (canBeOutOfHorizontalBounds) {
		int16 candidateRightBoundary = getBbox().width() + candidate.x;
		bool cameraPastRightBoundary = _parentStage->extent().x < candidateRightBoundary;
		bool cameraPastLeftBoundary = candidate.x < 0;
		if (cameraPastRightBoundary || cameraPastLeftBoundary) {
			result = false;
		}
		debugC(6, kDebugCamera, "[%s] %s: %s [rightBoundary: %d, extent: %d]", debugName(), __func__, result ? "true" : "false", candidateRightBoundary, _parentStage->extent().x);
	}

	// We can only be out of vertical bounds if we have a requested delta and
	// are not doing Y wrapping.
	bool canBeOutOfVerticalBounds = !_parentStage->cylindricalY() && _panDelta.y != 0;
	if (canBeOutOfVerticalBounds) {
		int16 candidateBottomBoundary = getBbox().height() + candidate.y;
		bool cameraPastBottomBoundary = _parentStage->extent().y < candidateBottomBoundary;
		bool cameraPastTopBoundary = candidate.y < 0;
		if (cameraPastBottomBoundary || cameraPastTopBoundary) {
			result = false;
		}
		debugC(6, kDebugCamera, "[%s] %s: %s [bottomBoundary: %d, extent: %d]", debugName(), __func__, result ? "true" : "false", candidateBottomBoundary, _parentStage->extent().y);
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
		uint currentRuntime = g_engine->getTotalPlayTime();
		uint elapsedTime = currentRuntime - _startTime;
		double elapsedSeconds = elapsedTime / MILLISECONDS_IN_ONE_SECOND;
		percentValue = elapsedSeconds / _totalPanDuration;
		break;
	}

	default:
		percentValue = 0.0;
		break;
	}

	percentValue = CLIP<double>(percentValue, 0.0, 1.0);
	return percentValue;
}

Common::Rect CameraActor::getAdvanceRect() {
	Common::Rect viewportBounds = getViewportBounds();
	Common::Point viewportBoundsOrigin = viewportBounds.origin();
	int16 viewportWidth = viewportBounds.width();
	int16 viewportHeight = viewportBounds.height();

	// These constants seem to be set for smooth streaming from CD-ROM, but disk
	// image actors also rely on a properly large advance rect for strip loading.
	// Divisor for calculating horizontal preload expansion in the pan direction.
	const double HORIZONTAL_PRELOAD_EXPANSION_FACTOR = 2.25;
	// Multiplier for vertical preload expansion.
	const int16 VERTICAL_PRELOAD_EXPANSION_FACTOR = 2;

	// Handle horizontal panning.
	if (_panDelta.x < 0) {
		// We're panning left, so expand the advance rect leftward.
		int16 quotient = viewportWidth / HORIZONTAL_PRELOAD_EXPANSION_FACTOR;
		viewportBoundsOrigin.x -= quotient;
		viewportWidth += quotient;
	} else if (_panDelta.x > 0) {
		// We're panning right, so expand the advance rect rightward.
		int16 quotient = viewportWidth / HORIZONTAL_PRELOAD_EXPANSION_FACTOR;
		viewportWidth += quotient;
	}

	// Handle vertical panning.
	if (_panDelta.y < 0) {
		// We're panning up, so expand the advance rect upward.
		int16 quotient = viewportHeight / HORIZONTAL_PRELOAD_EXPANSION_FACTOR;
		viewportBoundsOrigin.y -= quotient;
		viewportHeight *= VERTICAL_PRELOAD_EXPANSION_FACTOR;
	} else if (_panDelta.y > 0) {
		// We're panning down, so expand the advance rect downward.
		viewportHeight *= VERTICAL_PRELOAD_EXPANSION_FACTOR;
	}

	// Construct and return the advance rect with the adjusted origin and dimensions.
	adjustCameraViewport(viewportBoundsOrigin);
	Common::Rect advanceRect(viewportBoundsOrigin, viewportWidth, viewportHeight);
	return advanceRect;
}

} // End of namespace MediaStation
