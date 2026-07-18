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

#include "graphics/paletteman.h"

#include "mediastation/minigames/dotgame.h"
#include "mediastation/datafile.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

void DotGameActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderDotGameMaxDots:
		// Read all dot positions from stream.
		_totalDots = chunk.readTypedUint16();
		for (uint16 i = 0; i < _totalDots; i++) {
			_dotPositions.push_back(chunk.readTypedPoint());
		}
		break;

	case kActorHeaderEditable:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderDotGameHelperSprite1:
		_startHotspotId = chunk.readTypedUint16();
		break;

	case kActorHeaderDotGameHelperSprite2:
		_endHotspotId = chunk.readTypedUint16();
		break;

	case kActorHeaderDotGameState:
		_markerActorId = chunk.readTypedUint16();
		break;

	case kActorHeaderDotGameSpeed:
		_speed = chunk.readTypedUint16();
		break;

	case kActorHeaderDotGameLineThickness:
		_lineThickness = chunk.readTypedByte();
		break;

	case kActorHeaderDotGameColor:
		_lineColorR = chunk.readTypedByte();
		_lineColorG = chunk.readTypedByte();
		_lineColorB = chunk.readTypedByte();
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue DotGameActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kDotGameResetMethod: {
		ARGCOUNTCHECK(1);
		int16 targetDot = static_cast<int16>(args[0].asFloat());
		doReset(targetDot);
		break;
	}

	case kDotGameShowMethod:
		ARGCOUNTCHECK(0);
		if (!_isVisible) {
			_isVisible = true;
			activateHelpers();
			updateHelpers();
			invalidateLocalBounds();
		}
		break;

	case kDotGameHideMethod:
		ARGCOUNTCHECK(0);
		if (_isVisible) {
			_isVisible = false;
			deActivateHelpers();
			invalidateLocalBounds();
		}
		break;

	case kDotGameHitMethod:
		ARGCOUNTCHECK(0);
		doHit();
		break;

	default:
		returnValue = Actor::callMethod(methodId, args);
	}

	return returnValue;
}

void DotGameActor::updateHelpers() {
	if (!_isVisible) {
		return;
	} else if (_currentDotIndex >= _totalDots) {
		warning("[%s] %s: Current dot count (%d) exceeds max dots (%d)", debugName(), __func__, _currentDotIndex, _totalDots);
		return;
	}

	// Determine next dot position in the sequence, wrapping around to close the shape.
	uint16 nextDotIndex = (_currentDotIndex + 1) % _totalDots;
	Common::Point nextDotPosition = _dotPositions[nextDotIndex] + getBbox().origin();

	Common::Array<ScriptValue> moveArgs;
	ScriptValue xArg, yArg;
	xArg.setToFloat(static_cast<double>(nextDotPosition.x));
	moveArgs.push_back(xArg);
	yArg.setToFloat(static_cast<double>(nextDotPosition.y));
	moveArgs.push_back(yArg);
	Actor *startHotspot = g_engine->getImtGod()->getActorById(_startHotspotId);
	if (startHotspot != nullptr) {
		startHotspot->callMethod(kSpatialCenterMoveToMethod, moveArgs);
	}

	Actor *endHotspot = g_engine->getImtGod()->getActorById(_endHotspotId);
	if (endHotspot != nullptr) {
		endHotspot->callMethod(kSpatialCenterMoveToMethod, moveArgs);
	}
}

void DotGameActor::draw(DisplayContext &displayContext) {
	if (!_isVisible || _currentDotIndex == 0) {
		return;
	}

	Graphics::ManagedSurface *targetSurface = displayContext._destImage;
	if (targetSurface == nullptr) {
		warning("[%s] %s: No target surface to draw", debugName(), __func__);
		return;
	}

	// Draw lines connecting consecutive dots that have been reached so far.
	Common::Point drawOrigin = getBbox().origin() + displayContext._origin;
	for (uint16 i = 0; i < _currentDotIndex - 1; i++) {
		Common::Point startPoint = drawOrigin + _dotPositions[i];
		Common::Point endPoint = drawOrigin + _dotPositions[i + 1];
		targetSurface->drawThickLine(
			startPoint.x, startPoint.y, endPoint.x, endPoint.y,
			_lineThickness, _lineThickness, _linePaletteIndex);
	}

	if (_animationProgress < 100) {
		// If animation not complete, draw line from last dot to current position.
		Common::Point lastDot = drawOrigin + _dotPositions[_currentDotIndex - 1];
		Common::Point currentPos = drawOrigin + _currentPosition;
		targetSurface->drawThickLine(
			lastDot.x, lastDot.y, currentPos.x, currentPos.y,
			_lineThickness, _lineThickness, _linePaletteIndex);

	} else if (_totalDots == _currentDotIndex) {
		// If all dots connected, close the loop from last dot to first dot.
		Common::Point firstDot = drawOrigin + _dotPositions[0];
		Common::Point lastDot = drawOrigin + _dotPositions[_currentDotIndex - 1];
		targetSurface->drawThickLine(
			lastDot.x, lastDot.y, firstDot.x, firstDot.y,
			_lineThickness, _lineThickness, _linePaletteIndex);

	} else {
		// Otherwise draw from last dot to next dot in the sequence.
		Common::Point lastDot = drawOrigin + _dotPositions[_currentDotIndex - 1];
		Common::Point nextDot = drawOrigin + _dotPositions[_currentDotIndex];
		targetSurface->drawThickLine(
			lastDot.x, lastDot.y, nextDot.x, nextDot.y,
			_lineThickness, _lineThickness, _linePaletteIndex);
	}
}

void DotGameActor::doHit() {
	if (!_isVisible) {
		return;
	} else if (_currentDotIndex >= _totalDots) {
		warning("[%s] %s: Current dot count (%d) exceeds max dots (%d)", debugName(), __func__, _currentDotIndex, _totalDots);
		return;
	}

	_currentDotIndex++;

	// Show the marker actor to show "tracing" the path to the next dot.
	Actor *markerActor = g_engine->getImtGod()->getActorById(_markerActorId);
	Common::Array<ScriptValue> emptyArgs;
	if (markerActor != nullptr) {
		markerActor->callMethod(kSpatialShowMethod, emptyArgs);
	}

	deActivateHelpers();
	_animationProgress = 0;
	while (_animationProgress < 100) {
		_animationProgress += _speed;

		Common::Point startPoint = _dotPositions[_currentDotIndex - 1];
		Common::Point endPoint;
		if (_totalDots == _currentDotIndex) {
			// The next dot is the first dot again to close the loop.
			endPoint = _dotPositions[0];
		} else {
			// Move to next dot.
			endPoint = _dotPositions[_currentDotIndex];
		}

		// Interpolate position between start and end.
		Common::Point delta = endPoint - startPoint;
		Common::Point interpolated = startPoint + (delta * _animationProgress / 100);
		_currentPosition = interpolated;

		// Update the marker actor position.
		if (markerActor != nullptr) {
			Common::Array<ScriptValue> moveArgs;
			ScriptValue xArg, yArg;
			Common::Point originOnScreen = getBbox().origin() + _currentPosition;
			xArg.setToFloat(static_cast<double>(originOnScreen.x));
			yArg.setToFloat(static_cast<double>(originOnScreen.y));
			moveArgs.push_back(xArg);
			moveArgs.push_back(yArg);
			markerActor->callMethod(kSpatialCenterMoveToMethod, moveArgs);

			Common::Array<ScriptValue> frameArgs;
			ScriptValue trueArg;
			trueArg.setToBool(true);
			frameArgs.push_back(trueArg);
			markerActor->callMethod(kIncrementFrameMethod, frameArgs);
		}

		invalidateLocalBounds();
		g_engine->getDisplayUpdateManager()->performUpdateDirty();
	}

	_animationProgress = 100;

	// We traced the path to the next dot, so we can now hide the marker actor again.
	if (markerActor != nullptr) {
		markerActor->callMethod(kSpatialHideMethod, emptyArgs);
	}
	invalidateLocalBounds();

	bool allDotsCompleted = (_totalDots - 1 == _currentDotIndex);
	if (allDotsCompleted) {
		callMethod(kDotGameHideMethod, emptyArgs);
		_currentDotIndex = 0;
		_animationProgress = 100;
		runScriptResponseIfExists(kDotGameCompleteEvent);
	} else {
		updateHelpers();
		activateHelpers();
	}
}

void DotGameActor::doReset(int16 targetDotIndex) {
	if (_totalDots == 0) {
		_currentDotIndex = 0;
		return;
	}

	const int16 maxDotIndex = static_cast<int16>(_totalDots - 1);
	targetDotIndex = CLIP<int16>(targetDotIndex, 0, maxDotIndex);
	_currentDotIndex = targetDotIndex;

	if (_isVisible) {
		updateHelpers();
		invalidateLocalBounds();
	}

	// The original apparently supported arbitrary colors here, but
	// ScummVM forces us to choose a palette index. The only known
	// use of this actor (for the Lion King constellations minigame)
	// only uses a white line anyway. So this shouldn't be a problem.
	// This must be done here because at load time we don't have an active palette yet.
	Graphics::Palette currentPalette = g_system->getPaletteManager()->grabPalette(0, Graphics::PALETTE_COUNT);
	_linePaletteIndex = currentPalette.findBestColor(_lineColorR, _lineColorG, _lineColorB);
}

void DotGameActor::activateHelpers() {
	Common::Array<ScriptValue> emptyArgs;

	Actor *startHotspot = g_engine->getImtGod()->getActorById(_startHotspotId);
	if (startHotspot != nullptr) {
		startHotspot->callMethod(kMouseActivateMethod, emptyArgs);
	}

	Actor *endHotspot = g_engine->getImtGod()->getActorById(_endHotspotId);
	if (endHotspot != nullptr) {
		endHotspot->callMethod(kSpatialShowMethod, emptyArgs);
		endHotspot->callMethod(kTimePlayMethod, emptyArgs);
	}
}

void DotGameActor::deActivateHelpers() {
	Common::Array<ScriptValue> emptyArgs;

	Actor *startHotspot = g_engine->getImtGod()->getActorById(_startHotspotId);
	if (startHotspot != nullptr) {
		startHotspot->callMethod(kMouseDeactivateMethod, emptyArgs);
	}

	Actor *endHotspot = g_engine->getImtGod()->getActorById(_endHotspotId);
	if (endHotspot != nullptr) {
		endHotspot->callMethod(kSpatialHideMethod, emptyArgs);
		endHotspot->callMethod(kTimeStopMethod, emptyArgs);
	}
}

void DotGameActor::loadIsComplete() {
	// If the actor is initially visible, hide it on load completion.
	if (_isVisible) {
		_isVisible = false;
		Common::Array<ScriptValue> emptyArgs;
		callMethod(kDotGameHideMethod, emptyArgs);
	}

	SpatialEntity::loadIsComplete();
}

} // End namespace MediaStation
