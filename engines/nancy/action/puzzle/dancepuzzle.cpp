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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/dancepuzzle.h"

namespace Nancy {
namespace Action {

void DancePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _movieRecordName);
	_hoverCursor = stream.readUint16LE();
	_pointsTableIndex = stream.readSint16LE();
	readFilename(stream, _rootMovieName);
	readFilename(stream, _imageName);

	_moves.resize(stream.readUint16LE());
	for (Move &move : _moves) {
		readExclusiveRect(stream, move.hoverSrc);
		readExclusiveRect(stream, move.src);
		readExclusiveRect(stream, move.dest);
		readFilename(stream, move.movieName);
	}

	_music.readData(stream);

	_steps.resize(stream.readUint16LE());
	for (Step &step : _steps) {
		readFilename(stream, step.movieName);
		step.flag = stream.readSint16LE();
		step.startTime = stream.readSint32LE();
		step.endTime = stream.readSint32LE();

		// The data has the points and the time they start to count from;
		// they hold until the next ones do, the last ones to the end of the step
		const uint16 numPoints = stream.readUint16LE();
		int16 points = 0;
		for (uint i = 0; i < numPoints; ++i) {
			const int16 nextPoints = stream.readSint16LE();
			TimedPoints timed;
			timed.time = stream.readSint32LE();
			timed.points = i == 0 ? nextPoints : points;
			step.points.push_back(timed);
			points = nextPoints;
		}

		if (numPoints > 0) {
			TimedPoints last;
			last.time = step.endTime;
			last.points = points;
			step.points.push_back(last);
		}
	}

	_reactions.resize(stream.readUint16LE());
	for (Reaction &reaction : _reactions) {
		reaction.minPoints = stream.readSint16LE();
		reaction.maxPoints = stream.readSint16LE();
		reaction.sound.readData(stream);
	}
}

void DancePuzzle::init() {
	initViewportSurface();
	loadImage();

	playSoundBlock(_music);
	_currentMovieName = _rootMovieName;
	_startTime = g_nancy->getTotalPlayTime();

	redraw();
}

void DancePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (uint i = 0; i < _moves.size(); ++i) {
		const Move &move = _moves[i];
		_drawSurface.blitFrom(_image, (int)i == _hoveredMove ? move.hoverSrc : move.src, move.dest.origin());
	}

	_needsRedraw = true;
}

// The movie record plays what is left for it under its name
void DancePuzzle::playMovie(const Common::String &movieName) {
	Common::Array<Common::String> movies;
	movies.push_back(movieName);
	NancySceneState.setMoviePlaylist(_movieRecordName, movies);
	_currentMovieName = movieName;
}

void DancePuzzle::addPoints(int16 points) {
	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (table) {
		table->setSingleValue(_pointsTableIndex, table->getValueOrZero(_pointsTableIndex) + points);
	}
}

// A step is scored once with the right move. A wrong one costs what the latest
// right one would, also only once, and leaves nothing more to lose by being late.
int16 DancePuzzle::scoreMove(Step &step, int32 time, const Common::String &movieName) {
	if (step.hasScored) {
		return 0;
	}

	const bool isRightMove = step.movieName.equalsIgnoreCase(movieName);
	if (step.hasHadWrongMove && !isRightMove) {
		return 0;
	}

	int16 points = 0;
	if (isRightMove) {
		for (const TimedPoints &timed : step.points) {
			if (time <= timed.time) {
				step.hasScored = true;
				points = timed.points;
				break;
			}
		}

		if (step.hasScored && step.hasHadWrongMove && points < 0) {
			return 0;
		}
	} else {
		// A wrong move costs what being as late as possible does
		if (!step.points.empty()) {
			points = step.points.back().points;
		}

		step.hasHadWrongMove = true;
	}

	for (const Reaction &reaction : _reactions) {
		if (points >= reaction.minPoints && points <= reaction.maxPoints) {
			playSoundBlock(reaction.sound);
			break;
		}
	}

	return points;
}

void DancePuzzle::endStep(int32 time) {
	Step &step = _steps[_activeStep];
	addPoints(scoreMove(step, time, _currentMovieName));
	NancySceneState.setEventFlag(step.flag, g_nancy->_false);
	_activeStep = -1;
}

void DancePuzzle::startStep(uint stepID) {
	Step &step = _steps[stepID];
	_activeStep = stepID;

	// The move may be the one that is being danced already
	if (step.movieName.equalsIgnoreCase(_currentMovieName)) {
		addPoints(scoreMove(step, step.startTime, _currentMovieName));
	}

	NancySceneState.setEventFlag(step.flag, g_nancy->_true);
}

void DancePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		const int32 time = g_nancy->getTotalPlayTime() - _startTime;

		// Once the time of a step is up it is back to the root movie
		if (_activeStep != -1 && time >= _steps[_activeStep].endTime) {
			endStep(time);

			if (!_rootMovieName.empty()) {
				playMovie(_rootMovieName);
			}
		}

		while (_nextStep < _steps.size() && time >= _steps[_nextStep].startTime) {
			if (time < _steps[_nextStep].endTime) {
				if (_activeStep != -1) {
					endStep(time);
				}

				startStep(_nextStep);
			}

			++_nextStep;
		}

		break;
	}
	default:
		break;
	}
}

void DancePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	int hovered = -1;
	for (uint i = 0; i < _moves.size(); ++i) {
		if (_moves[i].dest.contains(pos)) {
			hovered = i;
		}
	}

	if (hovered != _hoveredMove) {
		_hoveredMove = hovered;
		redraw();
	}

	if (hovered == -1) {
		return;
	}

	setDataCursor(_hoverCursor);

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		playMovie(_moves[hovered].movieName);

		if (_activeStep != -1) {
			const int32 time = g_nancy->getTotalPlayTime() - _startTime;
			addPoints(scoreMove(_steps[_activeStep], time, _currentMovieName));
		}

		input.eatMouseInput();
	}
}

} // End of namespace Action
} // End of namespace Nancy
