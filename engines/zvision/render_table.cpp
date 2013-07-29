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
#include "zvision/point.h"

namespace ZVision {

RenderTable::RenderTable(uint32 numColumns, uint32 numRows)
		: _numRows(numRows),
		  _numColumns(numColumns),
		  _renderState(RenderState::FLAT) {
	assert(numRows != 0 && numColumns != 0);

	_internalBuffer = new Point<int16>[numRows * numColumns];
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

void RenderTable::mutateImage(uint16 *sourceBuffer, uint16* destBuffer, uint32 imageWidth, uint32 imageHeight, Common::Rect subRectangle, Common::Rect destRectangle) {
	bool isTransposed = _renderState == RenderTable::PANORAMA || _renderState == RenderTable::TILT;

	for (int y = subRectangle.top; y < subRectangle.bottom; y++) {
		uint32 normalizedY = y - subRectangle.top;

		for (int x = subRectangle.left; x < subRectangle.right; x++) {
			uint32 normalizedX = x - subRectangle.left;

			uint32 index = (y + destRectangle.top) * _numColumns + (x + destRectangle.left);

			// RenderTable only stores offsets from the original coordinates
			uint32 sourceYIndex = y + _internalBuffer[index].y;
			uint32 sourceXIndex = x + _internalBuffer[index].x;

			// Clamp the yIndex to the size of the image
			sourceYIndex = CLIP<uint32>(sourceYIndex, 0, imageHeight - 1);

			// Clamp the xIndex to the size of the image
			sourceXIndex = CLIP<uint32>(sourceXIndex, 0, imageWidth - 1);

			// TODO: Figure out a way to not have branching every loop. The only way that comes to mind is to have a whole separate set of for loops for isTransposed, but that's ugly. The compiler might do this anyway in the end
			if (isTransposed) {
				destBuffer[normalizedY * subRectangle.width() + normalizedX] = sourceBuffer[sourceXIndex * imageHeight + sourceYIndex];
			} else {
				destBuffer[normalizedY * subRectangle.width() + normalizedX] = sourceBuffer[sourceYIndex * imageWidth + sourceXIndex];
			}
		}
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

	// TODO: Change the algorithm to write a whole row at a time instead of a whole column at a time. AKA: for(y) { for(x) {}} instead of for(x) { for(y) {}}
	for (uint32 x = 0; x < _numColumns; x++) {
		// Add an offset of 0.01 to overcome zero tan/atan issue (vertical line on half of screen)
		float temp = atan(tanOverHalfHeight * ((float)x - halfWidth + 0.01f));

		int32 newX = floor((halfHeightOverTan * _panoramaOptions.linearScale * temp) + halfWidth);
		float cosX = cos(temp);

		for (uint32 y = 0; y < _numRows; y++) {
			int32 newY = floor(halfHeight + (y - halfHeight) * cosX);

			uint32 index = y * _numColumns + x;

			// Only store the x,y offsets instead of the absolute positions
			_internalBuffer[index].x = newX - x; //pixel index
			_internalBuffer[index].y = newY - y; //pixel index
		}
	}
}

void RenderTable::generateTiltLookupTable() {

}

} // End of namespace ZVision
