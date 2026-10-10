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

#ifndef NANCY_ACTION_DANCEPUZZLE_H
#define NANCY_ACTION_DANCEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy18 dance floor. A song plays, and at set times one of the dance moves is
// called for, which an event flag lets the scene show. The player has a button
// for every move; the sooner the right one is pressed the more points it is
// worth, and a wrong one, or none, costs some. The points go into the value
// table, and the audience reacts to them. The dancing itself is done by a movie
// record of the scene, which gets told what to play.
class DancePuzzle : public PuzzleRecord {
public:
	// Drawn above the stage light movies, which cover the buttons
	DancePuzzle() : PuzzleRecord(15) {}
	virtual ~DancePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "DancePuzzle"; }

	struct Move {
		Common::Rect hoverSrc;
		Common::Rect src;
		Common::Rect dest;
		Common::String movieName;
	};

	// What a move is worth up to a point in time
	struct TimedPoints {
		int32 time = 0;	// in milliseconds from the start of the song
		int16 points = 0;
	};

	struct Step {
		// -- File data --
		Common::String movieName;	// of the move that is called for
		int16 flag = -1;
		int32 startTime = 0;
		int32 endTime = 0;
		Common::Array<TimedPoints> points;

		// -- Runtime --
		bool hasScored = false;
		bool hasHadWrongMove = false;
	};

	struct Reaction {
		int16 minPoints = 0;
		int16 maxPoints = 0;
		RandomSoundBlock sound;
	};

	int16 scoreMove(Step &step, int32 time, const Common::String &movieName);
	void addPoints(int16 points);
	void endStep(int32 time);
	void startStep(uint stepID);
	void playMovie(const Common::String &movieName);
	void redraw();

	// -- File data --
	Common::String _movieRecordName;
	uint16 _hoverCursor = 0;
	int16 _pointsTableIndex = 0;
	Common::String _rootMovieName;	// played whenever no move is
	Common::Array<Move> _moves;
	RandomSoundBlock _music;
	Common::Array<Step> _steps;
	Common::Array<Reaction> _reactions;

	// -- Runtime --
	uint32 _startTime = 0;
	int _activeStep = -1;
	uint _nextStep = 0;
	Common::String _currentMovieName;
	int _hoveredMove = -1;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DANCEPUZZLE_H
