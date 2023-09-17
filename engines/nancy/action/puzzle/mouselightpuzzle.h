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

#ifndef NANCY_ACTION_MOUSELIGHTPUZZLE_H
#define NANCY_ACTION_MOUSELIGHTPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Shows a single image over the entire frame, with most of it blackened;
// a circle around that follows the cursor reveals parts of the image.
// Circle can have smooth or hard edges. Not actually a puzzle.
class MouseLightPuzzle : public RenderActionRecord {
public:
	MouseLightPuzzle() : RenderActionRecord(7) {}
	virtual ~MouseLightPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "MouseLightPuzzle"; };
	bool isViewportRelative() const override { return true; }

	Common::Path _imageName;
	byte _radius = 0;
	bool _smoothEdges = false;

	Graphics::ManagedSurface _baseImage;
	Graphics::ManagedSurface _maskCircle;
	Graphics::ManagedSurface _mask;

	Common::Point _lastMousePos;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MAZECHASEPUZZLE_H
