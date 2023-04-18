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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_RIDDLEPUZZLE_H
#define NANCY_ACTION_RIDDLEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class RiddlePuzzle : public RenderActionRecord {
public:
	enum SolveState { kWaitForSound, kNotSolved, kFailed, kSolvedOne, kSolvedAll };
	RiddlePuzzle() : RenderActionRecord(7) {}
	virtual ~RiddlePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	struct Riddle {
		Common::String text;
		SoundDescription sound;
		Common::Array<Common::String> answers;
		SceneChangeWithFlag sceneIncorrect;
		SoundDescription soundIncorrect;
		SceneChangeWithFlag sceneCorrect;
		SoundDescription soundCorrect;
	};

	Common::String getRecordTypeName() const override { return "RiddlePuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawText();

	uint16 _viewportTextFontID;
	uint16 _textboxTextFontID;
	Time _cursorBlinkTime;
	SoundDescription _typeSound;
	SoundDescription _eraseSound;
	SoundDescription _enterSound;
	SceneChangeWithFlag _successSceneChange;
	SoundDescription _successSound;
	SceneChangeWithFlag _exitSceneChange;
	SoundDescription _exitSound;
	Common::Rect _exitHotspot;
	Common::Array<Riddle> _riddles;

	Time _nextBlinkTime;
	SolveState _solveState = kWaitForSound;
	bool _playerHasHitReturn = false;
	Common::String _playerInput;
	uint _riddleID = 0;
	RiddlePuzzleState *_puzzleState = nullptr;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RIDDLEPUZZLE_H
