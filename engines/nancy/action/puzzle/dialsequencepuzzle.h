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

#ifndef NANCY_ACTION_DIALSEQUENCEPUZZLE_H
#define NANCY_ACTION_DIALSEQUENCEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy17 grandfather clock. Each named dial is turned to a position, and a button
// enters the positions of all of them. The positions entered last make up the
// combination. Every solution has a scene of its own, and can ask for an event flag.
class DialSequencePuzzle : public PuzzleRecord {
public:
	DialSequencePuzzle() : PuzzleRecord(7) {}
	virtual ~DialSequencePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "DialSequencePuzzle"; }

	struct Dial {
		Common::String name;
		bool canWrapAround = false;
		Common::Rect forwardHotspot;
		Common::Rect backHotspot;
		Common::Array<Common::Rect> srcs;	// one per position

		uint position = 0;
		Common::String entered;	// the positions entered so far, separated by commas
	};

	struct Sequence {
		Common::String dialName;
		Common::String positions;	// separated by commas
	};

	struct Solution {
		FlagDescription condition;	// has to hold for the solution to count; label -1 = always
		Common::Array<Sequence> sequences;
		SceneChangeWithFlag scene;
		RandomSoundBlock sound;
	};

	static uint countPositions(const Common::String &positions);

	bool turnDial(Dial &dial, const Common::Point &mousePos);
	void enterPositions();
	bool isSolution(const Solution &solution) const;
	void redraw();

	// -- File data --
	uint16 _setCursor = 0;
	uint16 _forwardCursor = 0;
	uint16 _backCursor = 0;

	Common::Array<Dial> _dials;
	Common::Rect _dialDest;

	// The button that enters the positions is a movie, played on every press
	Common::Path _setMovieName;
	Common::Rect _setDest;

	RandomSoundBlock _setSound;
	RandomSoundBlock _turnSound;

	Common::Array<Solution> _solutions;

	// -- Runtime --
	MoviePlayer _setMovie;
	bool _isSetMovieShown = false;
	uint _maxPositions = 0;	// the length of the longest solution
	int _solvedSolution = -1;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DIALSEQUENCEPUZZLE_H
