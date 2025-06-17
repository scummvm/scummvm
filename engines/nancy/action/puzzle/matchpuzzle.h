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

#ifndef NANCY_ACTION_MATCHPUZZLE_H
#define NANCY_ACTION_MATCHPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Flag puzzle in Nancy 8

class MatchPuzzle : public RenderActionRecord {
public:
	MatchPuzzle() : RenderActionRecord(7) {}
	virtual ~MatchPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "MatchPuzzle"; }
	bool isViewportRelative() const override { return true; }

	Graphics::ManagedSurface _image;
	//Common::Rect _displayBounds;

	Common::Rect _shuffleButtonRect;
	Common::Array<Common::Rect> _flagRects;

	Common::Path _overlayName;
	Common::Path _flagPointBackgroundName;

	Common::StringArray _flagNames;

	SoundDescription _slotWinSound;
	SoundDescription _shuffleSound;
	SoundDescription _cardPlaceSound;
	SoundDescription _matchSuccessSound;

	SceneChangeWithFlag _solveSceneChange;
	SceneChangeWithFlag _exitSceneChange;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MATCHPUZZLE_H
