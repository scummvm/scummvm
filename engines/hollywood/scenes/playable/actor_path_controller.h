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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ACTOR_PATH_CONTROLLER_H
#define HOLLYWOOD_SCENES_PLAYABLE_ACTOR_PATH_CONTROLLER_H

#include "common/array.h"
#include "common/types.h"

#include "hollywood/resource.h"
#include "hollywood/scenes/playable/actor_types.h"

namespace Hollywood {

enum {
	kActorPathStepDeltaTableSize = 72
};

// Canonical six-facing, twelve-cel movement tables shared by scene resources.
extern const byte kActorPathStepDeltaTableSet00[kActorPathStepDeltaTableSize];
extern const byte kActorPathStepDeltaTableSetB4[kActorPathStepDeltaTableSize];
extern const byte kActorPathStepDeltaTableSet87[kActorPathStepDeltaTableSize];
extern const byte kActorPathStepDeltaTableSet5A[kActorPathStepDeltaTableSize];

/**
 * Supplies scene-dependent region lookup and route adjustments.
 *
 * Segment hooks may request a facing or temporarily alter the shared step-delta
 * table. Setting restoredStepDeltas asks the controller to restore the base table
 * after that segment.
 */
class ActorPathControllerDelegate {
public:
	virtual ~ActorPathControllerDelegate() {}

	virtual byte paletteRegionAt(int x, int y) const = 0;
	virtual bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) = 0;
	virtual bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) = 0;
};

/**
 * Expands resource-defined palette-region routes into actor animation frames.
 *
 * Resource route tables select intermediate regions and candidate boundary
 * points. The delegate maps coordinates to regions and may customize each
 * intermediate and final segment.
 */
class ActorPathController {
public:
	ActorPathController();

	void initialize(uint routeBoundaryPointCount, uint routeStepCount, uint paletteRegionCount,
		uint boundaryCandidateCount, uint routeStepCountPerRegionPair,
		const byte *defaultStepDeltas, uint defaultStepDeltaCount);
	bool loadRouteTables(const Common::Array<byte> &metadata, uint routeBoundaryOffset,
		uint routeStepsOffset, const char *archiveName);
	void resetStepDeltas(const byte *table, uint tableSize);
	void queueWithPaletteRegionRouting(ActorPathControllerDelegate &delegate, int startX, int startY,
		int targetX, int targetY, byte initialDrawOrderMode, byte initialFacing, byte initialCel,
		byte finalFacing, byte finalCel, byte invalidFacing, byte invalidCel,
		const byte *baseStepDeltas, uint baseStepDeltaCount);

	void buildFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
		byte finalFacing, byte finalCel, int requestedFacing, byte invalidFacing, byte invalidCel);
	void appendFrame(const ActorPathBuildState &state);
	ScenePoint nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const;
	ScenePoint bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
		byte currentRegion, byte targetRegion) const;
	byte calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const;
	uint calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const;
	byte nextCel(byte cel) const;
	uint stepDelta(byte facing, byte cel) const;
	byte calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const;

	Common::Array<ScenePoint> routeBoundaryPoints;
	Common::Array<byte> routeSteps;
	Common::Array<ActorPathFrame> frames;
	Common::Array<byte> stepDeltas;

private:
	uint _paletteRegionCount;
	uint _boundaryCandidateCount;
	uint _routeStepCountPerRegionPair;
};

const byte *defaultActorPathStepDeltaTable();
uint defaultActorPathStepDeltaTableSize();

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ACTOR_PATH_CONTROLLER_H
