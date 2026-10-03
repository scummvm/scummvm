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

#ifndef MOHAWK_ZOOMBINI_DEBUG_H
#define MOHAWK_ZOOMBINI_DEBUG_H

#include "common/scummsys.h"
#include "mohawk/zoombini_resource.h"

namespace Mohawk {

struct ZoombiniDebugCommand {
public:
	enum DebugCommandType {
		kNone = 0,
		kDrawCursor,
		kDrawImage,
		kDrawShape,
		kDrawShapes,
		kDrawFeature,
		kDrawTerrain,
		kPlotPoint,
		kPlotLine,
		kPlotRect,
	};

	/** Set a command that draws one cursor resource. */
	void setDrawCursor(ZmbResource resource);
	/** Set a command that draws one image resource. */
	void setDrawImage(ZmbResource resource);
	/** Set a command that draws one shape from an image resource. */
	void setDrawShape(ZmbResource resource, uint16 shapeIdx);
	/** Set a command that draws a sequence of shapes from an image resource. */
	void setDrawShapes(ZmbResource resource, uint16 startShapeIdx);
	/** Set a command that draws one SCRB feature. */
	void setDrawFeature(ZmbResource resource, int16 scrbId);
	/** Set a command that shows the active page through its terrain mask. */
	void setDrawTerrain();
	/** Set a command that plots one diagnostic pixel. */
	void setPlotPoint(int16 x, int16 y, uint32 color);
	/** Set a command that plots a diagnostic line. */
	void setPlotLine(int16 x0, int16 y0, int16 x1, int16 y1, uint32 color);
	/** Set a command that plots a diagnostic rectangle. */
	void setPlotRect(int16 x, int16 y, int16 width, int16 height, uint32 color);

	/** Selected diagnostic command kind. */
	DebugCommandType _type = kNone;
	/** Resource used by a draw command. */
	ZmbResource _resource;
	/** Shape index used by a shape draw command. */
	uint16 _shapeIdx = 1;
	/** SCRB resource ID used by a feature draw command. */
	int16 _scrbId = 0;
	/** First X coordinate for a plot command. */
	int16 _x1 = 0;
	/** First Y coordinate for a plot command. */
	int16 _y1 = 0;
	/** Second X coordinate or width for a plot command. */
	int16 _x2 = 0;
	/** Second Y coordinate or height for a plot command. */
	int16 _y2 = 0;
	/** Packed diagnostic color value. */
	uint32 _color = 0;
};

} // End of namespace Mohawk

#endif // MOHAWK_ZOOMBINI_DEBUG_H
