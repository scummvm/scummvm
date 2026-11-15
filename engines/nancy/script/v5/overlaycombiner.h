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

#ifndef NANCY_SCRIPT_V5_OVERLAYCOMBINER_H
#define NANCY_SCRIPT_V5_OVERLAYCOMBINER_H

#include "engines/nancy/script/records.h"

namespace Nancy {
namespace Script {

// Nancy29: shows the colors of a part of one image with the transparency of a
// part of another, e.g. a photo cut to the shape of a frame:
//
//   AR:OverlayCombiner({ rgbOvl = "Photo_OVL", rgbSource = Rect:New(0, 0, 100, 80),
//       alphaOvl = "Frame_OVL", alphaSource = Rect:New(0, 0, 100, 80),
//       onScreen = Rect:New(200, 100, 300, 180), invertAlpha = false })
//
// The alpha of each pixel is the alpha of the first image times the alpha of
// the second one, or times its inverse when invertAlpha is set. Both source
// parts and onScreen must have the same size, otherwise nothing is shown.
class OverlayCombinerRecord : public Record {
public:
	OverlayCombinerRecord(SceneContext &context) : Record(context, "ND.OverlayCombiner") {}

	RecordType getType() const override { return kRecordOverlayCombiner; }

	void updateGraphics() override;
	void registerGraphics() override;

protected:
	void readTable(int tableIndex) override;
	void onZChanged() override;
	bool hasTransform() const override { return true; }
	void onTransformChanged() override;
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	// Combines the two images into the picture, after a field has changed
	void combine();
	void place();

	Common::String _rgbImageName;
	Common::String _rgbTreeName;
	ScriptRect _rgbSource;
	Common::String _alphaImageName;
	Common::String _alphaTreeName;
	ScriptRect _alphaSource;
	ScriptRect _dest;
	bool _isAlphaInverted = false;
	uint32 _modColor = 0xFFFFFFFF;	// its alpha is always opaque

	Graphics::ManagedSurface _image;	// the combined picture
	bool _hasPicture = false;	// the images were combined successfully
	bool _isDrawn = false;		// _hasPicture, and onScreen has the size of the picture
	bool _isCombined = false;
	bool _isPlacementStale = false;
	RecordGraphic _graphic;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_OVERLAYCOMBINER_H
