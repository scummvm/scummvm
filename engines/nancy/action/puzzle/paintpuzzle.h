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
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Paint-fill puzzle, new in Nancy14 (AR 181). The player picks a color from a
// palette (Vincent's brush) and clicks the regions of a picture to fill them.
// Each region has a target color; the puzzle is solved once every region holds
// its target color. A filled region is its shape recolored to the chosen color.
class PaintPuzzle : public RenderActionRecord {
public:
	PaintPuzzle() : RenderActionRecord(7) {}
	virtual ~PaintPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "PaintPuzzle"; }

	// A palette color: its RGB, the clickable swatch, and its fill sprite.
	struct PaintColor {
		byte r = 0;
		byte g = 0;
		byte b = 0;
		Common::Rect swatchRect;
		Common::Rect fillRect;
	};

	// A fillable region of the picture: an overlay shape drawn at a position,
	// its current color index, and the target it must hold to be solved.
	struct PaintRegion {
		Common::Path name;
		Common::Rect rect;
		int16 currentColor = -1;
		int16 targetColor = -1;
	};

	struct SceneOutcome {
		int16 field0 = 0;
		int16 sceneID = 0;
		byte flag = 0;
	};

	int colorSwatchAtCursor(const Common::Point &mousePos) const;
	int regionAtCursor(const Common::Point &mousePos) const;
	// Alpha of the region overlay's shape at (x,y): the pixel's alpha channel, or
	// 255/0 from the color key for alpha-less images. 0 means outside the shape.
	byte shapeAlpha(const Graphics::ManagedSurface &img, int x, int y) const;
	void paintRegion(uint regionIndex, int colorIndex);
	void drawRegion(uint regionIndex);
	void redraw();
	bool isSolved() const;
	void applyOutcome(const SceneOutcome &outcome);

	// -- File data --
	Common::Path _imageName;		// 0x3d
	int16 _field5e = 0;				// 0x5e
	Common::Point _offset;			// 0x60 (two int32)
	Common::Rect _canvasRect;		// 0x68

	Common::Array<PaintColor> _colors;		// 0x78
	Common::Array<PaintRegion> _regions;	// 0x94

	RandomSoundBlock _sounds[3];	// 0xa4/0xfa (before) + one after the outcome

	int16 _field1a6 = 0;		// 0x1a6
	SceneOutcome _outcome;		// 0x1a8

	// Give-up hotspot (count-prefixed 23-byte trailer): click to leave the puzzle.
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	Graphics::ManagedSurface _image;
	Common::Array<Graphics::ManagedSurface> _regionImages;
	int _heldColor = -1;
	int _hoverRegion = -1;
	int _hoverColor = -1;
	bool _solved = false;
	bool _outcomeApplied = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PAINTPUZZLE_H
