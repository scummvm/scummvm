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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/graphics/render_table.h"

#include "common/rect.h"

#include "graphics/colormasks.h"


namespace ZVision {

RenderTable::RenderTable(uint numColumns, uint numRows)
		: _numRows(numRows),
		  _numColumns(numColumns),
		  _renderState(FLAT) {
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
		_panoramaOptions.fieldOfView = 27.0f;
		_panoramaOptions.linearScale = 0.55f;
		_panoramaOptions.reverse = false;
		break;
	case TILT:
		_tiltOptions.fieldOfView = 27.0f;
		_tiltOptions.linearScale = 0.55f;
		_tiltOptions.reverse = false;
		break;
	case FLAT:
		// Intentionally left empty
		break;
	}
}

const Common::Point RenderTable::convertWarpedCoordToFlatCoord(const Common::Point &point) {
	// If we're outside the range of the RenderTable, no warping is happening. Return the maximum image coords
	if (point.x >= (int16)_numColumns || point.y >= (int16)_numRows || point.x < 0 || point.y < 0) {
		int16 x = CLIP<int16>(point.x, 0, (int16)_numColumns);
		int16 y = CLIP<int16>(point.y, 0, (int16)_numRows);
		return Common::Point(x, y);
	}

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

void RenderTable::mutateImage(uint16 *sourceBuffer, uint16* destBuffer, uint32 destWidth, const Common::Rect &subRect) {
	uint32 destOffset = 0;

	for (int16 y = subRect.top; y < subRect.bottom; ++y) {
		uint32 sourceOffset = y * _numColumns;

		for (int16 x = subRect.left; x < subRect.right; ++x) {
			uint32 normalizedX = x - subRect.left;
			uint32 index = sourceOffset + x;

			// RenderTable only stores offsets from the original coordinates
			uint32 sourceYIndex = y + _internalBuffer[index].y;
			uint32 sourceXIndex = x + _internalBuffer[index].x;

			destBuffer[destOffset + normalizedX] = sourceBuffer[sourceYIndex * _numColumns + sourceXIndex];
		}

		destOffset += destWidth;
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

	float fovInRadians = (_panoramaOptions.fieldOfView * M_PI / 180.0f);
	float cylinderRadius = halfHeight / tan(fovInRadians);

	for (uint x = 0; x < _numColumns; ++x) {
		// Add an offset of 0.01 to overcome zero tan/atan issue (vertical line on half of screen)
		// Alpha represents the horizontal angle between the viewer at the center of a cylinder and x
		float alpha = atan(((float)x - halfWidth + 0.01f) / cylinderRadius);

		// To get x in cylinder coordinates, we just need to calculate the arc length
		// We also scale it by _panoramaOptions.linearScale
		int32 xInCylinderCoords = int32(floor((cylinderRadius * _panoramaOptions.linearScale * alpha) + halfWidth));

		float cosAlpha = cos(alpha);

		for (uint y = 0; y < _numRows; ++y) {
			// To calculate y in cylinder coordinates, we can do similar triangles comparison,
			// comparing the triangle from the center to the screen and from the center to the edge of the cylinder
			int32 yInCylinderCoords = int32(floor(halfHeight + ((float)y - halfHeight) * cosAlpha));

			uint32 index = y * _numColumns + x;

			// Only store the (x,y) offsets instead of the absolute positions
			_internalBuffer[index].x = xInCylinderCoords - x;
			_internalBuffer[index].y = yInCylinderCoords - y;
		}
	}
}

void RenderTable::generateTiltLookupTable() {
	float halfWidth = (float)_numColumns / 2.0f;
	float halfHeight = (float)_numRows / 2.0f;

	float fovInRadians = (_tiltOptions.fieldOfView * M_PI / 180.0f);
	float cylinderRadius = halfWidth / tan(fovInRadians);

	for (uint y = 0; y < _numRows; ++y) {

		// Add an offset of 0.01 to overcome zero tan/atan issue (horizontal line on half of screen)
		// Alpha represents the vertical angle between the viewer at the center of a cylinder and y
		float alpha = atan(((float)y - halfHeight + 0.01f) / cylinderRadius);

		// To get y in cylinder coordinates, we just need to calculate the arc length
		// We also scale it by _tiltOptions.linearScale
		int32 yInCylinderCoords = int32(floor((cylinderRadius * _tiltOptions.linearScale * alpha) + halfHeight));

		float cosAlpha = cos(alpha);
		uint32 columnIndex = y * _numColumns;

		for (uint x = 0; x < _numColumns; ++x) {
			// To calculate x in cylinder coordinates, we can do similar triangles comparison,
			// comparing the triangle from the center to the screen and from the center to the edge of the cylinder
			int32 xInCylinderCoords = int32(floor(halfWidth + ((float)x - halfWidth) * cosAlpha));

			uint32 index = columnIndex + x;

			// Only store the (x,y) offsets instead of the absolute positions
			_internalBuffer[index].x = xInCylinderCoords - x;
			_internalBuffer[index].y = yInCylinderCoords - y;
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
