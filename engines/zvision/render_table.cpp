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

#include "graphics/colormasks.h"

#include "zvision/render_table.h"
#include "zvision/vector2.h"

namespace ZVision {

RenderTable::RenderTable(uint numColumns, uint numRows)
		: _numRows(numRows),
		  _numColumns(numColumns),
		  _renderState(FLAT) {
	assert(numRows != 0 && numColumns != 0);

	_internalBuffer = new Vector2[numRows * numColumns];
}

RenderTable::~RenderTable() {
	delete[] _internalBuffer;
}

void RenderTable::setRenderState(RenderState newState) {
	_renderState = newState;

	switch (newState) {
	case PANORAMA:
		_panoramaOptions.fieldOfView = 27.0f;
		_panoramaOptions.linearScale = 0.55f;
		_panoramaOptions.reverse = false;
		break;
	case TILT:

		break;
	case FLAT:
		// Intentionally left empty
		break;
	}
}

const Common::Point RenderTable::convertWarpedCoordToFlatCoord(const Common::Point &point) {
	uint32 index = point.y * _numColumns + point.x;

	Common::Point newPoint(point);
	newPoint.x += _internalBuffer[index].x;
	newPoint.y += _internalBuffer[index].y;

	return newPoint;
}

uint16 mixTwoRGB(uint16 colorOne, uint16 colorTwo, float percentColorOne) {
	assert(percentColorOne < 1.0f);

	float rOne = float((colorOne & Graphics::ColorMasks<555>::kRedMask) >> Graphics::ColorMasks<555>::kRedShift);
	float rTwo = float((colorTwo & Graphics::ColorMasks<555>::kRedMask) >> Graphics::ColorMasks<555>::kRedShift);
	float gOne = float((colorOne & Graphics::ColorMasks<555>::kGreenMask) >> Graphics::ColorMasks<555>::kGreenShift);
	float gTwo = float((colorTwo & Graphics::ColorMasks<555>::kGreenMask) >> Graphics::ColorMasks<555>::kGreenShift);
	float bOne = float((colorOne & Graphics::ColorMasks<555>::kBlueMask) >> Graphics::ColorMasks<555>::kBlueShift);
	float bTwo = float((colorTwo & Graphics::ColorMasks<555>::kBlueMask) >> Graphics::ColorMasks<555>::kBlueShift);

	float rFinal = rOne * percentColorOne + rTwo * (1.0f - percentColorOne);
	float gFinal = gOne * percentColorOne + gTwo * (1.0f - percentColorOne);
	float bFinal = bOne * percentColorOne + bTwo * (1.0f - percentColorOne);

	uint16 returnColor = (byte(rFinal + 0.5f) << Graphics::ColorMasks<555>::kRedShift) |
	                     (byte(gFinal + 0.5f) << Graphics::ColorMasks<555>::kGreenShift) |
						 (byte(bFinal + 0.5f) << Graphics::ColorMasks<555>::kBlueShift);

	return returnColor;
}

void RenderTable::mutateImage(uint16 *sourceBuffer, uint16* destBuffer, uint32 imageWidth, uint32 imageHeight, Common::Rect subRectangle, Common::Rect destRectangle) {
	bool isTransposed = _renderState == RenderTable::PANORAMA || _renderState == RenderTable::TILT;

	for (int y = subRectangle.top; y < subRectangle.bottom; y++) {
		uint normalizedY = y - subRectangle.top;

		for (int x = subRectangle.left; x < subRectangle.right; x++) {
			uint normalizedX = x - subRectangle.left;

			uint32 index = (normalizedY + destRectangle.top) * _numColumns + (normalizedX + destRectangle.left);

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

void RenderTable::generateRenderTable() {
	switch (_renderState) {
	case ZVision::RenderTable::PANORAMA:
		generatePanoramaLookupTable();
		break;
	case ZVision::RenderTable::TILT:
		generateTiltLookupTable();
		break;
	case ZVision::RenderTable::FLAT:
		// Intentionally left empty
		break;
	}
}

void RenderTable::generatePanoramaLookupTable() {
	memset(_internalBuffer, 0, _numRows * _numColumns * sizeof(uint16));

	float halfWidth = (float)_numColumns / 2.0f;
	float halfHeight = (float)_numRows / 2.0f;

	float fovRadians = (_panoramaOptions.fieldOfView * M_PI / 180.0f);
	float halfHeightOverTan = halfHeight / tan(fovRadians);
	float tanOverHalfHeight = tan(fovRadians) / halfHeight;

	// TODO: Change the algorithm to write a whole row at a time instead of a whole column at a time. AKA: for(y) { for(x) {}} instead of for(x) { for(y) {}}
	for (uint x = 0; x < _numColumns; x++) {
		// Add an offset of 0.01 to overcome zero tan/atan issue (vertical line on half of screen)
		float temp = atan(tanOverHalfHeight * ((float)x - halfWidth + 0.01f));

		int32 newX = int32(floor((halfHeightOverTan * _panoramaOptions.linearScale * temp) + halfWidth));
		float cosX = cos(temp);

		for (uint y = 0; y < _numRows; y++) {
			int32 newY = int32(floor(halfHeight + ((float)y - halfHeight) * cosX));

			uint32 index = y * _numColumns + x;

			// Only store the x,y offsets instead of the absolute positions
			_internalBuffer[index].x = newX - x;
			_internalBuffer[index].y = newY - y;
		}
	}
}

void RenderTable::generateTiltLookupTable() {
	for (uint x = 0; x < _numColumns; x++) {
		for (uint y = 0; y < _numRows; y++) {
			uint32 index = y * _numColumns + x;

			_internalBuffer[index].x = 0;
			_internalBuffer[index].y = 0;
		}
	}
}

void RenderTable::setPanoramaFoV(float fov) {
	assert(fov > 0.0f);

	_panoramaOptions.fieldOfView = fov;
}

void RenderTable::setPanoramaScale(float scale) {
	assert(scale > 0.0f);

	_panoramaOptions.linearScale = scale;
}

void RenderTable::setPanoramaReverse(bool reverse) {
	_panoramaOptions.reverse = reverse;
}

void RenderTable::setTiltFoV(float fov) {
	assert(fov > 0.0f);

	_tiltOptions.fieldOfView = fov;
}

void RenderTable::setTiltScale(float scale) {
	assert(scale > 0.0f);

	_tiltOptions.linearScale = scale;
}

void RenderTable::setTiltReverse(bool reverse) {
	_tiltOptions.reverse = reverse;
}

} // End of namespace ZVision
