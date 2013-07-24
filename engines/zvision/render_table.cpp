/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "common/scummsys.h"

#include "zvision/render_table.h"


namespace ZVision {

RenderTable::RenderTable(uint32 numRows, uint32 numColumns)
		: _numRows(numRows),
		  _numColumns(numColumns) {
	assert(numRows != 0 && numColumns != 0);

	_internalBuffer = new Common::Point[numRows * numColumns];
}

RenderTable::~RenderTable() {
	delete[] _internalBuffer;
}

void RenderTable::setRenderState(RenderState newState) {
	_renderState = newState;

	switch (newState) {
	case PANORAMA:
		_panoramaOptions.fieldOfView = 60;
		_panoramaOptions.linearScale = 1;
		generatePanoramaLookupTable();
		break;
	case TILT:
		generateTiltLookupTable();
		break;
	case FLAT:
		// Intentionally left empty
		break;
	}
}

void RenderTable::generatePanoramaLookupTable() {
	float fieldOfView = _panoramaOptions.fieldOfView;
	float scale = _panoramaOptions.linearScale;

	memset(_internalBuffer, 0, _numRows * _numColumns * sizeof(uint16));

	float halfWidth = (float)_numColumns / 2.0f;
	float halfHeight = (float)_numRows / 2.0f;

	float fovRadians = (fieldOfView * M_PI / 180.0f);
	float halfHeightOverTan = halfHeight / tan(fovRadians);
	float tanOverHalfHeight = tan(fovRadians) / halfHeight;

	for (uint32 x = 0; x < _numRows; x++) {
		// Add an offset of 0.01 to overcome zero tan/atan issue (vertical line on half of screen)
		float xPos = (float)x - halfWidth + 0.01;
		float tempX = atan(xPos*tanOverHalfHeight);

		int32 newX = floor(scale * halfHeightOverTan * tempX);// + half_w);
		float cosX = cos(tempX);

		for (uint32 y = 0; y < _numColumns; y++) {
			int32 newY = floor(halfHeight + (y - halfHeight) * cosX);

			uint32 index = x * _numColumns + y;
			_internalBuffer[index].x = newX; //pixel index
			_internalBuffer[index].y = newY; //pixel index
		}
	}
}

void RenderTable::generateTiltLookupTable() {

}

} // End of namespace ZVision
