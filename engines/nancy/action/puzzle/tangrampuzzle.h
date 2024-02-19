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

#ifndef NANCY_ACTION_TANGRAMPUZZLE_H
#define NANCY_ACTION_TANGRAMPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/misc/mousefollow.h"

namespace Nancy {
namespace Action {

// Handles a specific type of puzzle where clicking an object rotates it,
// as well as several other objects linked to it. Examples are the sun/moon
// and staircase spindle puzzles in nancy3
class TangramPuzzle : public RenderActionRecord {
public:
	TangramPuzzle() : RenderActionRecord(7) {}
	virtual ~TangramPuzzle();

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "TangramPuzzle"; }
	bool isViewportRelative() const override { return true; }

	class Tile : public Misc::MouseFollowObject {
		friend class TangramPuzzle;
	public:
		Tile();
		virtual ~Tile();

		void drawMask();
		void setHighlighted(bool highlighted);

		Graphics::ManagedSurface _srcImage;
		Graphics::ManagedSurface _highlightedSrcImage;
		byte *_mask;
		byte _id;
		byte _rotation;
		bool _isHighlighted;

	protected:
		bool isViewportRelative() const override { return true; }
	};

	void drawToBuffer(const Tile &tile, Common::Rect subRect = Common::Rect());

	void pickUpTile(uint id);
	void putDownTile(uint id);
	void rotateTile(uint id);

	void moveToTop(uint id);
	void redrawBuffer(const Common::Rect &rect);

	bool checkBuffer(const Tile &tile) const;

	Common::Path _tileImageName;
	Common::Path _maskImageName;

	Common::Array<Common::Rect> _tileSrcs;
	Common::Array<Common::Rect> _tileDests;

	Common::Rect _maskSolveBounds;

	SoundDescription _pickUpSound;
	SoundDescription _putDownSound;
	SoundDescription _rotateSound;

	SceneChangeWithFlag _solveScene;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _tileImage;
	Graphics::ManagedSurface _maskImage;
	byte *_zBuffer = nullptr;

	Common::Array<Tile> _tiles;

	int16 _pickedUpTile = -1;
	bool _shouldCheck = false;
	bool _solved = false;

	uint _pixelAdjustment = 5;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TANGRAMPUZZLE_H
