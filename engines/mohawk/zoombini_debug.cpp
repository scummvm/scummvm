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

#include "mohawk/zoombini_debug.h"

namespace Mohawk {

void ZoombiniDebugCommand::setDrawCursor(ZmbResource resource) {
	_type = kDrawCursor;
	_resource = resource;
}

void ZoombiniDebugCommand::setDrawImage(ZmbResource resource) {
	_type = kDrawImage;
	_resource = resource;
}

void ZoombiniDebugCommand::setDrawShape(ZmbResource resource, uint16 shapeIdx) {
	_type = kDrawShape;
	_resource = resource;
	_shapeIdx = shapeIdx;
}

void ZoombiniDebugCommand::setDrawShapes(ZmbResource resource, uint16 startShapeIdx) {
	_type = kDrawShapes;
	_resource = resource;
	_shapeIdx = startShapeIdx;
}

void ZoombiniDebugCommand::setDrawFeature(ZmbResource resource, int16 scrbId) {
	_type = kDrawFeature;
	_resource = resource;
	_scrbId = scrbId;
}

void ZoombiniDebugCommand::setDrawTerrain() {
	_type = kDrawTerrain;
}

void ZoombiniDebugCommand::setPlotPoint(int16 x, int16 y, uint32 color) {
	_type = kPlotPoint;
	_x1 = x;
	_y1 = y;
	_color = color;
}

void ZoombiniDebugCommand::setPlotLine(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color) {
	_type = kPlotLine;
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
	_color = color;
}

void ZoombiniDebugCommand::setPlotRect(int16 x, int16 y, int16 width, int16 height, uint32 color) {
	_type = kPlotRect;
	_x1 = x;
	_y1 = y;
	_x2 = width;
	_y2 = height;
	_color = color;
}

} // End of namespace Mohawk
