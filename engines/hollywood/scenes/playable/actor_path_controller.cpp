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

#include "hollywood/scenes/playable/actor_path_controller.h"

#include "common/debug.h"
#include "common/util.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

#include <math.h>

namespace Hollywood {

const uint kPathActorFacingCount = 6;
const byte kActorPathStepDeltaTableSet00[kActorPathStepDeltaTableSize] = {
	2, 2, 2, 3, 3, 0, 2, 2, 2, 3, 3, 0,
	6, 7, 7, 5, 5, 5, 4, 6, 6, 5, 3, 4,
	5, 5, 5, 6, 2, 3, 4, 8, 10, 6, 3, 5,
	3, 0, 0, 3, 3, 3, 3, 0, 0, 3, 3, 3,
	4, 8, 10, 6, 3, 5, 5, 5, 5, 6, 2, 3,
	4, 6, 6, 5, 3, 4, 6, 7, 7, 5, 5, 5
};
const byte kActorPathStepDeltaTableSetB4[kActorPathStepDeltaTableSize] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};
const byte kActorPathStepDeltaTableSet87[kActorPathStepDeltaTableSize] = {
	6, 1, 1, 3, 3, 3, 7, 1, 0, 0, 4, 3,
	3, 2, 8, 6, 6, 7, 6, 4, 10, 3, 2, 9,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	4, 3, 3, 4, 0, 4, 4, 2, 0, 4, 2, 5,
	6, 10, 10, 4, 6, 4, 10, 8, 8, 7, 5, 10,
	6, 4, 10, 3, 2, 9, 3, 2, 8, 6, 6, 7
};
const byte kActorPathStepDeltaTableSet5A[kActorPathStepDeltaTableSize] = {
	4, 1, 1, 2, 2, 2, 5, 1, 0, 0, 3, 2,
	2, 1, 6, 4, 4, 5, 4, 3, 7, 2, 1, 6,
	6, 6, 5, 4, 7, 4, 7, 7, 3, 4, 3, 7,
	3, 2, 2, 3, 0, 3, 3, 1, 0, 3, 1, 4,
	4, 7, 7, 3, 4, 3, 7, 6, 6, 5, 4, 7,
	4, 3, 7, 2, 1, 6, 2, 1, 6, 4, 4, 5
};
const byte kActorFacingTurnTable[] = {
	0, 0, 0, 1, 0, 0, 1, 2, 0, 1, 2, 3, 5, 4, 0, 5, 0, 0,
	0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 3, 0, 0, 5, 4, 0, 5, 0,
	1, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 3, 4, 0, 1, 0, 5,
	4, 5, 0, 2, 1, 0, 2, 0, 0, 0, 0, 0, 4, 0, 0, 4, 5, 0,
	5, 0, 0, 5, 0, 1, 3, 2, 0, 3, 0, 0, 0, 0, 0, 5, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 1, 2, 4, 3, 0, 4, 0, 0, 0, 0, 0
};
const int8 kActorPathAxisDirectionByFacing[] = { -1, 1, 1, 1, -1, -1 };
const byte kActorInitialCelByFacing[] = { 0, 12, 12, 12, 1, 12 };
const float kActorPathDiagonalSlopeThreshold = 0.087488f;
const float kActorFacingSteepSlopeThreshold = 3.732051f;
const float kActorFacingDiagonalSlopeThreshold = 0.267949f;

int roundToNearestEven(float value) {
	const float lower = floorf(value);
	const float fraction = value - lower;
	const int lowerInt = (int)lower;
	if (fraction > 0.5f)
		return lowerInt + 1;
	if (fraction < 0.5f)
		return lowerInt;
	return (lowerInt & 1) ? lowerInt + 1 : lowerInt;
}

const byte *defaultActorPathStepDeltaTable() {
	return kActorPathStepDeltaTableSet00;
}

uint defaultActorPathStepDeltaTableSize() {
	return ARRAYSIZE(kActorPathStepDeltaTableSet00);
}

ActorPathController::ActorPathController() :
		_paletteRegionCount(0),
		_boundaryCandidateCount(0),
		_routeStepCountPerRegionPair(0) {
}

void ActorPathController::initialize(uint routeBoundaryPointCount, uint routeStepCount, uint paletteRegionCount,
		uint boundaryCandidateCount, uint routeStepCountPerRegionPair,
		const byte *defaultStepDeltas, uint defaultStepDeltaCount) {
	routeBoundaryPoints.resize(routeBoundaryPointCount);
	routeSteps.resize(routeStepCount);
	_paletteRegionCount = paletteRegionCount;
	_boundaryCandidateCount = boundaryCandidateCount;
	_routeStepCountPerRegionPair = routeStepCountPerRegionPair;
	resetStepDeltas(defaultStepDeltas, defaultStepDeltaCount);
}

bool ActorPathController::loadRouteTables(const Common::Array<byte> &metadata, uint routeBoundaryOffset,
		uint routeStepsOffset, const char *archiveName) {
	const uint boundaryBytes = routeBoundaryPoints.size() * 4;
	if (metadata.size() < routeBoundaryOffset + boundaryBytes ||
			metadata.size() < routeStepsOffset + routeSteps.size()) {
		warning("%s chunk 4 is too short for path route tables", archiveName);
		return false;
	}

	for (uint i = 0; i < routeBoundaryPoints.size(); ++i) {
		const uint offset = routeBoundaryOffset + i * 4;
		routeBoundaryPoints[i].x = readSint16LE(metadata, offset);
		routeBoundaryPoints[i].y = readSint16LE(metadata, offset + 2);
	}
	memcpy(routeSteps.data(), metadata.data() + routeStepsOffset, routeSteps.size());
	return true;
}

void ActorPathController::resetStepDeltas(const byte *table, uint tableSize) {
	stepDeltas.resize(tableSize);
	if (table != nullptr && tableSize != 0)
		memcpy(stepDeltas.data(), table, tableSize);
}

void ActorPathController::queueWithPaletteRegionRouting(ActorPathControllerDelegate &delegate,
		int startX, int startY, int targetX, int targetY, byte initialDrawOrderMode,
		byte initialFacing, byte initialCel, byte finalFacing, byte finalCel, byte invalidFacing,
		byte invalidCel, const byte *baseStepDeltas, uint baseStepDeltaCount) {
	frames.clear();
	resetStepDeltas(baseStepDeltas, baseStepDeltaCount);

	ActorPathBuildState state;
	state.drawOrderMode = initialDrawOrderMode;
	state.facing = initialFacing;
	state.cel = nextCel(initialCel);
	state.x = startX;
	state.y = startY;
	appendFrame(state);

	byte currentRegion = delegate.paletteRegionAt(startX, startY);
	if (currentRegion == 0)
		currentRegion = initialDrawOrderMode;

	byte targetRegion = delegate.paletteRegionAt(targetX, targetY);
	if (targetRegion == 0)
		targetRegion = currentRegion;

	debugC(2, kDebugPath,
		"ActorPath route begin: start=(%d,%d) rawStartRegion=%u currentRegion=%u target=(%d,%d) rawTargetRegion=%u targetRegion=%u initialFacing=%u initialCel=%u",
		startX, startY, delegate.paletteRegionAt(startX, startY), currentRegion, targetX, targetY,
		delegate.paletteRegionAt(targetX, targetY), targetRegion, initialFacing, initialCel);

	if (currentRegion != targetRegion) {
		const byte routeStartRegion = currentRegion;
		const byte routeTargetRegion = targetRegion;
		for (uint stepIndex = 0; stepIndex < _routeStepCountPerRegionPair &&
				currentRegion != targetRegion; ++stepIndex) {
			const uint routeOffset =
				((uint)routeStartRegion * _paletteRegionCount + routeTargetRegion) *
				_routeStepCountPerRegionPair + stepIndex;
			if (routeOffset >= routeSteps.size())
				break;

			const byte nextRegion = routeSteps[routeOffset];
			if (nextRegion == 0 || nextRegion >= _paletteRegionCount)
				break;

			state.drawOrderMode = currentRegion;
			const ScenePoint boundary = nextRegion == targetRegion ?
				bestPaletteRouteBoundaryPoint(state.x, state.y, targetX, targetY, currentRegion, nextRegion) :
				nearestPaletteRouteBoundaryPoint(state.x, state.y, currentRegion, nextRegion);
			debugC(2, kDebugPath,
				"ActorPath route step %u: routeOffset=%u currentRegion=%u nextRegion=%u targetRegion=%u state=(%d,%d f=%u c=%u d=%u) boundary=(%d,%d)",
				stepIndex, routeOffset, currentRegion, nextRegion, targetRegion, state.x, state.y,
				state.facing, state.cel, state.drawOrderMode, boundary.x, boundary.y);

			byte segmentFinalFacing = invalidFacing;
			byte segmentFinalCel = invalidCel;
			if (boundary.x == targetX && boundary.y == targetY) {
				segmentFinalFacing = finalFacing;
				segmentFinalCel = finalCel;
			}

			int requestedFacing = -1;
			bool restoredStepDeltas = false;
			delegate.customizeRouteSegment(currentRegion, nextRegion, state, boundary,
				requestedFacing, restoredStepDeltas);
			debugC(3, kDebugPath,
				"ActorPath route step %u customization: requestedFacing=%d restoredStepDeltas=%u segmentFinalFacing=%u segmentFinalCel=%u",
				stepIndex, requestedFacing, restoredStepDeltas, segmentFinalFacing, segmentFinalCel);
			buildFramesBetweenPoints(state, boundary.x, boundary.y,
				segmentFinalFacing, segmentFinalCel, requestedFacing, invalidFacing, invalidCel);
			if (restoredStepDeltas)
				resetStepDeltas(baseStepDeltas, baseStepDeltaCount);

			currentRegion = nextRegion;
		}
	}

	int requestedFacing = -1;
	bool restoredStepDeltas = false;
	delegate.customizeRouteFinal(currentRegion, targetRegion, state, targetX, targetY,
		requestedFacing, restoredStepDeltas);
	state.drawOrderMode = currentRegion;
	debugC(2, kDebugPath,
		"ActorPath route final: currentRegion=%u targetRegion=%u state=(%d,%d f=%u c=%u d=%u) target=(%d,%d) requestedFacing=%d finalFacing=%u finalCel=%u",
		currentRegion, targetRegion, state.x, state.y, state.facing, state.cel, state.drawOrderMode,
		targetX, targetY, requestedFacing, finalFacing, finalCel);
	buildFramesBetweenPoints(state, targetX, targetY, finalFacing, finalCel, requestedFacing,
		invalidFacing, invalidCel);
	if (restoredStepDeltas)
		resetStepDeltas(baseStepDeltas, baseStepDeltaCount);
	debugC(2, kDebugPath, "ActorPath route end: frames=%u", frames.size());
}

void ActorPathController::buildFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
		byte finalFacing, byte finalCel, int requestedFacing, byte invalidFacing, byte invalidCel) {
	const uint firstFrame = frames.size();
	if (targetX == state.x && targetY == state.y) {
		debugC(3, kDebugPath,
			"ActorPath build zero-distance: pos=(%d,%d) facing=%u cel=%u finalFacing=%u finalCel=%u requestedFacing=%d",
			state.x, state.y, state.facing, state.cel, finalFacing, finalCel, requestedFacing);
		if (finalFacing != invalidFacing && state.facing != finalFacing) {
			for (uint turnStep = 0; turnStep < 3 && state.facing != finalFacing; ++turnStep) {
				const uint turnOffset = ((uint)state.facing * kPathActorFacingCount + finalFacing) * 3 + turnStep;
				state.facing = kActorFacingTurnTable[turnOffset];
				state.cel = kActorInitialCelByFacing[state.facing];
				appendFrame(state);
				state.cel = nextCel(state.cel);
			}
		}
		if (finalCel != invalidCel)
			state.cel = finalCel;
		appendFrame(state);
		state.cel = nextCel(state.cel);
		debugC(3, kDebugPath, "ActorPath build zero-distance end: addedFrames=%u", frames.size() - firstFrame);
		return;
	}

	const byte movementFacing = calculateMovementFacingForPath(state.x, state.y, targetX, targetY, requestedFacing);
	const int originalX = state.x;
	const int originalY = state.y;
	if (state.facing != movementFacing) {
		for (uint turnStep = 0; turnStep < 3 && state.facing != movementFacing; ++turnStep) {
			const uint turnOffset = ((uint)state.facing * kPathActorFacingCount + movementFacing) * 3 + turnStep;
			state.facing = kActorFacingTurnTable[turnOffset];
			state.cel = kActorInitialCelByFacing[state.facing];
			appendFrame(state);
			state.cel = nextCel(state.cel);
		}
	}

	const int startX = state.x;
	const int startY = state.y;
	const int principalStart = (movementFacing == 0 || movementFacing == 3) ? startY : startX;
	const int principalTarget = (movementFacing == 0 || movementFacing == 3) ? targetY : targetX;
	const uint stepCount = calculateWalkStepCountForAxisDelta(principalStart, principalTarget,
		movementFacing, state.cel);
	debugC(3, kDebugPath,
		"ActorPath build segment: from=(%d,%d) target=(%d,%d) movementFacing=%u requestedFacing=%d startCel=%u stepCount=%u",
		originalX, originalY, targetX, targetY, movementFacing, requestedFacing, state.cel, stepCount);

	if (stepCount != 0) {
		const bool verticalMovement = movementFacing == 0 || movementFacing == 3;
		const int secondaryDelta = verticalMovement ? ABS(startX - targetX) : ABS(startY - targetY);
		const float secondaryStep = (float)secondaryDelta / (float)stepCount;
		float secondaryAccumulator = secondaryStep;
		for (uint step = 1; step <= stepCount; ++step) {
			const int secondaryOffset = roundToNearestEven(secondaryAccumulator);
			const int delta = stepDelta(movementFacing, state.cel);
			if (verticalMovement) {
				state.y += (startY < targetY) ? delta : -delta;
				state.x = startX < targetX ? startX + secondaryOffset : startX - secondaryOffset;
			} else {
				state.x += (startX < targetX) ? delta : -delta;
				state.y = startY < targetY ? startY + secondaryOffset : startY - secondaryOffset;
			}
			state.facing = movementFacing;
			appendFrame(state);
			state.cel = nextCel(state.cel);
			secondaryAccumulator += secondaryStep;
		}
	}

	state.x = targetX;
	state.y = targetY;
	if (finalFacing != invalidFacing && state.facing != finalFacing) {
		for (uint turnStep = 0; turnStep < 3 && state.facing != finalFacing; ++turnStep) {
			const uint turnOffset = ((uint)state.facing * kPathActorFacingCount + finalFacing) * 3 + turnStep;
			state.facing = kActorFacingTurnTable[turnOffset];
			state.cel = kActorInitialCelByFacing[state.facing];
			appendFrame(state);
			state.cel = nextCel(state.cel);
		}
	}
	if (finalCel != invalidCel)
		state.cel = finalCel;
	appendFrame(state);
	state.cel = nextCel(state.cel);
	debugC(3, kDebugPath,
		"ActorPath build segment end: addedFrames=%u finalState=(%d,%d f=%u c=%u d=%u)",
		frames.size() - firstFrame, state.x, state.y, state.facing, state.cel, state.drawOrderMode);
}

void ActorPathController::appendFrame(const ActorPathBuildState &state) {
	ActorPathFrame frame;
	frame.drawOrderMode = state.drawOrderMode;
	frame.facing = state.facing;
	frame.cel = state.cel;
	frame.worldX = (int16)CLIP<int>(state.x, -32768, 32767);
	frame.worldY = (int16)CLIP<int>(state.y, -32768, 32767);
	frames.push_back(frame);
}

ScenePoint ActorPathController::nearestPaletteRouteBoundaryPoint(int startX, int startY,
		byte currentRegion, byte nextRegion) const {
	ScenePoint points[3];
	float scores[3];
	memset(points, 0, sizeof(points));
	memset(scores, 0, sizeof(scores));

	const uint baseIndex = ((uint)currentRegion * _paletteRegionCount + nextRegion) *
		_boundaryCandidateCount;
	for (uint candidate = 0; candidate < _boundaryCandidateCount && candidate < ARRAYSIZE(points); ++candidate) {
		const uint pointIndex = baseIndex + candidate;
		if (pointIndex >= routeBoundaryPoints.size())
			break;

		const ScenePoint point = routeBoundaryPoints[pointIndex];
		points[candidate] = point;
		scores[candidate] =
			sqrtf((float)ABS(startX - point.x)) +
			sqrtf((float)ABS(startY - point.y));
	}

	if (scores[1] <= scores[0])
		return scores[2] < scores[1] ? points[2] : points[1];
	return scores[0] < scores[2] ? points[0] : points[2];
}

ScenePoint ActorPathController::bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
		byte currentRegion, byte targetRegion) const {
	ScenePoint points[3];
	float scores[3];
	memset(points, 0, sizeof(points));
	memset(scores, 0, sizeof(scores));

	const uint baseIndex = ((uint)currentRegion * _paletteRegionCount + targetRegion) *
		_boundaryCandidateCount;
	for (uint candidate = 0; candidate < _boundaryCandidateCount && candidate < ARRAYSIZE(points); ++candidate) {
		const uint pointIndex = baseIndex + candidate;
		if (pointIndex >= routeBoundaryPoints.size())
			break;

		const ScenePoint point = routeBoundaryPoints[pointIndex];
		points[candidate] = point;
		scores[candidate] =
			sqrtf((float)ABS(startX - point.x)) +
			sqrtf((float)ABS(startY - point.y)) +
			sqrtf((float)ABS(targetX - point.x)) +
			sqrtf((float)ABS(targetY - point.y));
	}

	if (scores[1] <= scores[0])
		return scores[2] < scores[1] ? points[2] : points[1];
	return scores[0] < scores[2] ? points[0] : points[2];
}

byte ActorPathController::calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY,
		int requestedFacing) const {
	if (requestedFacing >= 0)
		return (byte)requestedFacing;

	if (toX == fromX)
		return fromY < toY ? 3 : 0;

	const float slope = (float)ABS(toY - fromY) / (float)MAX<int>(1, ABS(toX - fromX));
	if (fromX < toX) {
		if (toY < fromY) {
			if (slope < 1.0f)
				return slope <= kActorPathDiagonalSlopeThreshold ? 2 : 1;
			return 0;
		}
		return slope < 1.0f ? 2 : 3;
	}

	if (toY < fromY) {
		if (slope > 1.0f)
			return 0;
		return slope > kActorPathDiagonalSlopeThreshold ? 5 : 4;
	}
	return slope > 1.0f ? 3 : 4;
}

uint ActorPathController::calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis,
		byte facing, byte cel) const {
	if (facing >= kPathActorFacingCount)
		return 0;

	const int direction = kActorPathAxisDirectionByFacing[facing];
	int remaining = (targetAxis - startAxis) * direction;
	if (remaining <= 0)
		return 0;

	uint steps = 0;
	byte nextStepCel = cel;
	while (stepDelta(facing, nextStepCel) < (uint)remaining) {
		remaining -= (int)stepDelta(facing, nextStepCel);
		nextStepCel = nextCel(nextStepCel);
		++steps;
		if (steps > 300)
			break;
	}

	return steps;
}

byte ActorPathController::nextCel(byte cel) const {
	return cel == 12 ? 1 : (byte)(cel + 1);
}

uint ActorPathController::stepDelta(byte facing, byte cel) const {
	if (facing >= kPathActorFacingCount || cel == 0 || cel > 12)
		return 0;

	const uint offset = (uint)facing * 12 + cel - 1;
	if (offset >= stepDeltas.size())
		return 0;

	return stepDeltas[offset];
}

byte ActorPathController::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
	if (toX == fromX)
		return fromY < toY ? 3 : 0;

	const float slope = (float)ABS(toY - fromY) / (float)MAX<int>(1, ABS(toX - fromX));
	if (toX > fromX) {
		if (toY < fromY)
			return slope > kActorFacingSteepSlopeThreshold ? 0 :
				(slope > kActorFacingDiagonalSlopeThreshold ? 1 : 2);
		return slope > kActorFacingSteepSlopeThreshold ? 3 : 2;
	}

	if (toY < fromY)
		return slope > kActorFacingSteepSlopeThreshold ? 0 :
			(slope > kActorFacingDiagonalSlopeThreshold ? 5 : 4);
	return slope > kActorFacingSteepSlopeThreshold ? 3 : 4;
}

} // End of namespace Hollywood
