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

#ifndef NANCY_ACTION_SENTRYPUZZLE_H
#define NANCY_ACTION_SENTRYPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionzone.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy18. Tells the sentry puzzle of a room where the player comes in, by
// writing over the place that room has kept for them.
class SentryStartPosition : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "SentryStartPosition"; }

	Common::String _puzzleName;
	Common::Rect _rect;
	int32 _heading = 0;	// in degrees
};

// Nancy18 rooms full of robot sentries, seen from above. The player walks from
// cell to cell of a map, with the arrow keys or by holding the mouse button down
// where they want to go. Each sentry follows a path and sweeps a beam around,
// which walls stop; the beam touching the player sets an event flag. Zones on
// the map set flags as well, which is how a room is left.
class SentryPuzzle : public PuzzleRecord {
public:
	SentryPuzzle() : PuzzleRecord(7) {}
	virtual ~SentryPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "SentryPuzzle"; }

	enum PathMode {
		kPathStopsAtEnd		= 0,
		kPathStartsOver		= 1,
		kPathTurnsAround	= 2
	};

	struct Beam {
		int32 width = 0;
		Common::Rect src;
		Common::Rect endSrc;
	};

	struct Sentry {
		// -- File data --
		Common::Path imageName;
		Common::Array<Common::Rect> frameSrcs;
		Common::Array<Common::Rect> alertFrameSrcs;
		Beam beam;
		Beam alertBeam;
		byte pathMode = kPathStopsAtEnd;
		Common::Array<Common::Point> path;
		int32 speed = 0;			// in pixels per second
		double turnsPerSecond = 0;	// of the beam
		int32 numBeamSteps = 1;		// that a turn of the beam is made of

		// -- Runtime --
		Graphics::ManagedSurface image;
		double x = 0;
		double y = 0;
		uint nextPoint = 0;
		bool hasStopped = false;
		uint32 beamStepOffset = 0;
		double beamEndX = 0;
		double beamEndY = 0;
		bool hasSeenPlayer = false;
	};

	static void readBeam(Common::SeekableReadStream &stream, Beam &beam);

	Common::Rect getPlayerRect(double x, double y) const;
	bool isWalkable(int x, int y) const;
	bool canPlayerBeAt(double x, double y) const;
	int32 snapToCell(double position) const;

	void movePlayer(uint32 elapsed);
	void moveSentry(Sentry &sentry, uint32 elapsed);
	void castBeam(Sentry &sentry);
	bool doesBeamHit(const Sentry &sentry, const Common::Rect &rect) const;
	void checkZones();
	void storePlayerPosition();

	void drawPlayer();
	void drawSentry(const Sentry &sentry);
	void redraw();

	// -- File data --
	Common::String _name;
	Common::Path _mapName;
	int16 _seenFlag = -1;
	Common::Rect _startRect;
	int32 _startHeading = 0;	// in degrees
	int32 _playerSpeed = 0;		// in pixels per second
	int32 _cellSize = 1;
	Common::Array<Common::Rect> _playerFrameSrcs;
	Common::Array<Sentry> _sentries;
	Common::Array<ActionZone> _zones;

	// -- Runtime --
	Graphics::ManagedSurface _map;
	Common::Point _playerSize;
	double _playerX = 0;
	double _playerY = 0;
	double _playerHeading = 0;	// in radians, counterclockwise
	double _targetX = 0;
	double _targetY = 0;
	bool _isPlayerMoving = false;
	bool _isSeen = false;

	// What the player asks for
	bool _isMouseHeld = false;
	Common::Point _mousePos;
	uint16 _heldKeys = 0;

	uint32 _startTime = 0;
	uint32 _lastUpdateTime = 0;
	uint32 _walkStartTime = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SENTRYPUZZLE_H
