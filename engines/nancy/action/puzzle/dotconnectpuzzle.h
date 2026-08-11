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

#ifndef NANCY_ACTION_DOTCONNECTPUZZLE_H
#define NANCY_ACTION_DOTCONNECTPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Connect-the-dots puzzle. Player clicks dots to chain lines into a path;
// re-clicking the current tip pops the last line. Wins once kNumEdges lines
// have been drawn that match the solution either forward or fully reversed.
// Called from scene 6243 in Nancy10.
class DotConnectPuzzle : public RenderActionRecord {
public:
	DotConnectPuzzle() : RenderActionRecord(7) {}
	virtual ~DotConnectPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "DotConnectPuzzle"; }

	static const int kNumDots  = 17;
	static const int kNumEdges = 11;

	struct Edge {
		int32 a = 0;
		int32 b = 0;
	};

	// File data

	Common::Path _imageName;

	Common::Rect _dotSrcRects[kNumDots];
	Common::Rect _dotHighlightSrcRects[kNumDots];

	byte _lineColorR  = 0;
	byte _lineColorG  = 0;
	byte _lineColorB  = 0;
	int16 _lineThickness = 1;

	Edge _solution[kNumEdges];

	SoundDescription _clickSound;
	SoundDescription _firstLineHint;
	SoundDescription _startHint;
	SoundDescription _tooManyLinesSound;
	SoundDescription _allCoveredSound;

	SceneChangeDescription _winScene;
	FlagDescription _winFlag;
	uint16 _winDelaySec = 0;
	SoundDescription _winSound;

	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	Common::Rect _exitHotspot;

	// Runtime state

	enum SubState {
		kPlaying = 0,
		kWaitWinDelay,
		kPlayWinSound,
		kWaitWinSound,
		kExitToWin,
		kExitToCancel
	};

	SubState _subState = kPlaying;

	Common::Array<Edge> _drawn;
	int16 _currentTip = -1;

	bool _isActiveDot[kNumDots] = {};
	bool _firstLineHintPlayed = false;
	bool _startHintPlayed     = false;
	bool _allCoveredPlayed    = false;
	bool _tooManyPlayed       = false;

	uint32 _winDelayEndTime = 0;

	Graphics::ManagedSurface _image;

	void redraw();
	bool dotAlreadyUsed(int dot) const;
	void onDotClicked(int dot);
	void checkWin();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DOTCONNECTPUZZLE_H
