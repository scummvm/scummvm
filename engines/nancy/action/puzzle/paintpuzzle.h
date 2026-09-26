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

#ifndef NANCY_ACTION_PAINTPUZZLE_H
#define NANCY_ACTION_PAINTPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/util.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Paint-fill puzzle, new in Nancy14 (AR 181). The player picks a color from a
// palette (Vincent's brush) and clicks the regions of a picture to fill them.
// Each region has a target color; the puzzle is solved once every region holds
// its target color. A filled region is its shape recolored to the chosen color.
// Nancy15 adds per-color objects (paint jars) drawn at the swatches, per-color
// event flags, order-independent solutions and single-paint regions.
class PaintPuzzle : public PuzzleRecord {
public:
	PaintPuzzle() : PuzzleRecord(7) {}
	virtual ~PaintPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

	// The painted picture stays on screen in the NO_ART_SCENE conversation
	// that follows the puzzle
	bool survivesSceneChange(bool nextSceneIsNoArt) const override { return nextSceneIsNoArt; }

protected:
	Common::String getRecordTypeName() const override { return "PaintPuzzle"; }

	// A palette color: its RGB, the clickable swatch, its brush sprite, and
	// (Nancy15) the object sprite drawn at the swatch plus a flag set while the
	// color is used on any region.
	struct PaintColor {
		byte r = 0;
		byte g = 0;
		byte b = 0;
		Common::Rect swatchRect;
		Common::Rect fillRect;
		Common::Rect objectRect;
		int16 flagLabel = -1;
	};

	// A fillable region of the picture: an overlay shape drawn at a position,
	// its current color, and the target it must hold to be solved. Colors are
	// 1-based palette indices; 0 means unpainted.
	struct PaintRegion {
		Common::Path name;
		Common::Rect rect;
		int16 currentColor = -1;
		int16 targetColor = -1;
	};

	int colorSwatchAtCursor(const Common::Point &mousePos) const;
	int regionAtCursor(const Common::Point &mousePos) const;
	// Alpha of the region overlay's shape at (x,y): the pixel's alpha channel, or
	// 255/0 from the color key for alpha-less images. 0 means outside the shape.
	byte shapeAlpha(const Graphics::ManagedSurface &img, int x, int y) const;
	void paintRegion(uint regionIndex, int colorIndex);
	void dropColor();
	void drawRegion(uint regionIndex);
	void blendPixel(int x, int y, byte a, byte r, byte g, byte b);
	void drawSprite(const Graphics::ManagedSurface &img, const Common::Rect &src, const Common::Point &dest);
	void drawObjects();
	void drawBrush();
	void updateColorFlags();
	void redraw();
	bool isSolved() const;

	// -- File data --
	Common::Path _objectsImageName;
	uint16 _hoverCursorType = 0;	// 0x5e - raw Nancy14 cursor type, shown over a color swatch
	Common::Point _offset;			// 0x60 (two int32) - brush sprite offset from the cursor
	Common::Rect _canvasRect;		// 0x68

	Common::Array<PaintColor> _colors;		// 0x78
	Common::Array<PaintRegion> _regions;	// 0x94

	bool _unorderedSolution = false;	// Region targets may be matched in any order
	bool _paintOnce = false;			// A painted region can't be repainted

	RandomSoundBlock _sounds[2];	// 0xa4/0xfa

	RandomSoundBlock _solveSoundBlock;		// 0x150, plays before the solve scene change

	Common::Array<ExitHotspot> _exitHotspots;

	// -- Runtime state --
	Graphics::ManagedSurface _objectsImage;
	Common::Array<Graphics::ManagedSurface> _regionImages;
	int _heldColor = -1;
	Common::Point _brushPos;
	int _hoverRegion = -1;
	int _hoverColor = -1;
	bool _solved = false;
	bool _solveHandled = false;
	int _takenExit = -1;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PAINTPUZZLE_H
