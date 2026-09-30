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

#ifndef NANCY_ACTION_MINESWEEPERPUZZLE_H
#define NANCY_ACTION_MINESWEEPERPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 snow shovel puzzle (AR 195), a take on Minesweeper. The frozen pond is cleared
// with a shovel, working inwards from its edges, and the thin patches of ice have to be
// marked with cones. Cracking the thin ice too often sends the player into the water.
class MinesweeperPuzzle : public PuzzleRecord {
public:
	MinesweeperPuzzle() : PuzzleRecord(7) {}
	virtual ~MinesweeperPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MinesweeperPuzzle"; }

	// Tile types; the ones past kClear are for clear ice with 0-8 thin neighbors
	enum TileType {
		kCovered	= 0,
		kThinIce	= 1,
		kClear		= 2
	};

	enum DigResult {
		kDug,
		kCracked,
		kNotAllowed,
		kFellThrough,
		kOffBoard
	};

	enum ConeResult {
		kConePlaced,
		kConeRemoved,
		kConeNotAllowed,
		kConeOffBoard
	};

	struct Tool {
		Common::Rect idleSrc;
		Common::Rect activeSrc;		// shown while the tool can be used where it points
		uint16 cursor = 0;
		Common::Point hotspot;			// moves the held tool away from the cursor's center
		RandomSoundBlock pickUpSound;
		RandomSoundBlock putDownSound;
		bool isHeld = false;
	};

	struct Cell {
		byte type = kCovered;
		byte shownType = kCovered;
		bool hasCone = false;
		Common::Rect dest;
	};

	struct Ending {
		SceneChangeWithFlag scene;
		RandomSoundBlock sound;
	};

	static void readTool(Common::SeekableReadStream &stream, Tool &tool);
	static void readEnding(Common::SeekableReadStream &stream, Ending &ending);

	int cellAt(const Common::Point &pos) const;
	void getNeighbors(int cell, Common::Array<int> &neighbors) const;
	bool isReachable(int cell) const;
	bool isRevealed(int cell) const { return _cells[cell].shownType == _cells[cell].type; }

	DigResult dig(const Common::Point &pos);
	ConeResult toggleCone(const Common::Point &pos);
	bool removeCone(const Common::Point &pos);
	bool isSolved();

	void handleDigResult(DigResult result);
	void handleConeResult(ConeResult result);
	void pickUpTool(Tool &tool);
	void putDownTool(Tool &tool);
	void setCursor(const Common::Point &pos);
	void startEnding(const Ending &ending);

	void drawTile(const Common::Rect &src, const Common::Point &dest);
	void redraw();

	// -- File data --
	uint32 _maxCracks = 0;
	Common::Path _toolImageName;
	Tool _shovel;
	Tool _cone;
	uint16 _hoverCursor = 0;
	Common::Rect _shovelHotspot;
	Common::Rect _coneHotspot;
	Common::Rect _coneCountDest;
	uint32 _coneCountColor = 0;		// ARGB
	Common::String _coneCountFontName;
	RandomSoundBlock _digSound;
	RandomSoundBlock _crackSound;
	RandomSoundBlock _digNotAllowedSound;
	RandomSoundBlock _coneNotAllowedSound;
	Ending _solveEnding;
	Ending _failEnding;

	Common::Rect _tileSrcs[11];
	Common::Rect _coneSrc;
	Common::Path _boardImageName;
	uint _width = 0;
	uint _height = 0;
	Common::Array<Cell> _cells;

	// -- Runtime state --
	int _conesLeft = 0;
	uint _cracks = 0;
	bool _isEnding = false;
	bool _exitRequested = false;
	SceneChangeWithFlag _endScene;
	SoundDescription _endSound;
	Common::Point _mousePos;

	Graphics::ManagedSurface _toolImage;
	Graphics::ManagedSurface _boardImage;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MINESWEEPERPUZZLE_H
