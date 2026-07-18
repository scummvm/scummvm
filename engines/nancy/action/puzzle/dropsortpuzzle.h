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

#ifndef NANCY_ACTION_DROPSORTPUZZLE_H
#define NANCY_ACTION_DROPSORTPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Conveyor-belt candy-sorting puzzle, new in Nancy13 (AR 176). A hose on
// the left drops random candies onto a belt that carries them left to
// right; the player picks a candy up and drops it in a bin. Each sorting
// bin takes one candy type; the "Rejects" bin at the belt's end takes the
// types with no bin. A candy dropped in the wrong bin, or left to fall
// off into the Rejects bin, is a mistake unless the Rejects bin accepts
// it; three mistakes lose. Win once all candies are dispensed and the
// belt is empty. The scene ships several copies as a difficulty ramp
// (faster belt, shorter dispense interval).
class DropSortPuzzle : public RenderActionRecord {
public:
	DropSortPuzzle() : RenderActionRecord(7) {}
	virtual ~DropSortPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "DropSortPuzzle"; }

	static const int16 kNoItem = -1;

	struct Bin {
		Common::Rect region;			// the clickable drop zone
		byte enabled = 0;				// whether a wrong drop here counts as a strike
		Common::Array<int16> accepted;	// candy types this bin accepts
	};

	struct BeltItem {
		int16 type = 0;					// index into _itemSrcRects
		uint32 spawnTime = 0;			// dispense time, used to interpolate belt position
	};

	// Belt position (viewport space) at normalized progress: 0 = dispensed at the left,
	// 1 = about to fall off at the right.
	Common::Point beltPosition(float progress) const;
	Common::Rect itemDestAt(const Common::Point &pos, int16 type) const;
	int itemAtCursor(const Common::Point &mousePos) const;	// belt item under the cursor, or -1
	int binAtCursor(const Common::Point &mousePos) const;	// bin under the cursor, or -1
	Common::Point cursorToViewport(const Common::Point &mousePos) const;

	// Drops a candy into a bin: correct-bin sound if accepted, else a mistake (for an
	// enabled bin). Also handles candies falling off the belt into the reject bin.
	void applyDrop(int binIndex, int16 type);

	void redraw();
	void drawCounter();
	SoundDescription playSoundBlock(const RandomSoundBlock &block);

	// -- File data --
	Common::Path _imageName;			// 0x00 - overlay sprite sheet (candies, strikes)
	Common::Path _hoseMovieName;		// 0x21 - hose (dispenser) animation
	Common::Rect _hoseRect;				// 0x42
	uint32 _dispenseFrame = 0;			// 0x52 - unused in this port
	Common::Path _conveyorMovieName;	// 0x56 - belt (conveyor) animation
	Common::Rect _conveyorRect;			// 0x77
	uint32 _totalItems = 0;				// 0x87 - candies dispensed over the puzzle

	Common::Rect _beltLeft;				// belt left end (candies dispensed here)
	Common::Rect _beltRight;			// belt right end (candies fall off here)
	int32 _beltSpeed = 0;				// pixels per second
	int32 _dispenseInterval = 0;		// ms between candy dispenses
	uint16 _hoverCursorType = 0;		// raw Nancy13 cursor over a candy
	uint16 _dragCursorType = 0;			// raw Nancy13 cursor while carrying

	Common::Array<Common::Rect> _itemSrcRects;	// candy sprites in the overlay image
	int32 _unk1ad = 0;

	Common::Array<Bin> _bins;

	byte _showCounter = 0;
	int32 _counterX = 0;				// viewport position of the "Taffy to sort" number
	int32 _counterY = 0;
	uint16 _atlasId = 0;				// font id for the counter digits

	Common::Array<Common::Rect> _strikeSrcRects;	// strike marker sprites in the overlay image
	Common::Array<Common::Rect> _strikeDestRects;	// where the strike markers are drawn

	RandomSoundBlock _pickupSound;		// candy picked up
	RandomSoundBlock _dropSound;		// candy dropped in the correct bin
	RandomSoundBlock _hornSound;		// candy dropped in the wrong bin (a strike)

	SceneChangeDescription _winScene;
	FlagDescription _winFlag;
	RandomSoundBlock _winSound;

	SceneChangeDescription _loseScene;
	FlagDescription _loseFlag;
	RandomSoundBlock _loseSound;

	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;

	// -- Runtime state --
	Common::Array<BeltItem> _items;		// candies currently on the belt, oldest first
	uint _numDispensed = 0;
	uint32 _lastDispenseTime = 0;
	uint32 _travelDuration = 0;			// ms for a candy to cross the belt
	int _rejectBin = -1;				// bin at the end of the belt (unsorted candies land here)

	int16 _carriedType = kNoItem;
	Common::Point _dragPos;

	int _strikes = 0;
	bool _solved = false;
	bool _lost = false;
	bool _ended = false;
	bool _exitRequested = false;
	uint32 _endTime = 0;
	SoundDescription _endSound;

	// Decorative; both loop for the whole puzzle.
	MoviePlayer _conveyorMovie;
	MoviePlayer _hoseMovie;

	Graphics::ManagedSurface _image;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DROPSORTPUZZLE_H
