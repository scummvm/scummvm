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

#include "common/rect.h"
#include "common/scummsys.h"
#include "math/utils.h"
#include "zvision/zvision.h"
#include "common/system.h"

#include "zvision/graphics/render_table.h"
#include "zvision/scripting/script_manager.h"

namespace ZVision {

RenderTable::RenderTable(ZVision *engine, uint numColumns, uint numRows, const Graphics::PixelFormat pixelFormat)
	: _engine(engine),
	  _system(engine->_system),
	  _numRows(numRows),
    _numColumns(numColumns),
	  _renderState(FLAT),
	  _pixelFormat(pixelFormat) {
	assert(numRows != 0 && numColumns != 0);
	_internalBuffer = new FilterPixel[numRows * numColumns];
	memset(&_panoramaOptions, 0, sizeof(_panoramaOptions));
	memset(&_tiltOptions, 0, sizeof(_tiltOptions));
}

RenderTable::~RenderTable() {
	delete[] _internalBuffer;
}

void RenderTable::setRenderState(RenderState newState) {
	_renderState = newState;
	switch (newState) {
	  case PANORAMA:
		  _panoramaOptions.verticalFOV = Math::deg2rad<float>(27.0f);
		  _panoramaOptions.linearScale = 0.55f;
		  _panoramaOptions.reverse = false;
		  _panoramaOptions.zeroPoint = 0;
		  break;
	  case TILT:
		  _tiltOptions.verticalFOV = Math::deg2rad<float>(27.0f);
		  _tiltOptions.linearScale = 0.65f;
		  _tiltOptions.reverse = false;
		  break;
	  case FLAT:
		  // Intentionally left empty
		  break;
	  default:
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
	newPoint.x += (_internalBuffer[index].xDir ? _internalBuffer[index].Src.right : _internalBuffer[index].Src.left);
	newPoint.y += (_internalBuffer[index].yDir ? _internalBuffer[index].Src.bottom : _internalBuffer[index].Src.top);
	return newPoint;
}

/*/
void RenderTable::mutateImage(uint16 *sourceBuffer, uint16 *destBuffer, uint32 destWidth, const Common::Rect &subRect) {
	uint32 destOffset = 0;
  uint32 sourceXIndex = 0;
  uint32 sourceYIndex = 0;
  if(highQuality) {
    //TODO - convert to high quality pixel filtering
	  for (int16 y = subRect.top; y < subRect.bottom; ++y) {
		  uint32 sourceOffset = y * _numColumns;
		  for (int16 x = subRect.left; x < subRect.right; ++x) {
			  uint32 normalizedX = x - subRect.left;
			  uint32 index = sourceOffset + x;
			  // RenderTable only stores offsets from the original coordinates
			  sourceYIndex = y + _internalBuffer[index].Src.top;
			  sourceXIndex = x + _internalBuffer[index].Src.left;
			  destBuffer[destOffset + normalizedX] = sourceBuffer[sourceYIndex * _numColumns + sourceXIndex];
		  }
		  destOffset += destWidth;
	  }
  }
  else {
	  for (int16 y = subRect.top; y < subRect.bottom; ++y) {
		  uint32 sourceOffset = y * _numColumns;
		  for (int16 x = subRect.left; x < subRect.right; ++x) {
			  uint32 normalizedX = x - subRect.left;
			  uint32 index = sourceOffset + x;
			  // RenderTable only stores offsets from the original coordinates
			  sourceYIndex = y + _internalBuffer[index].Src.top;
			  sourceXIndex = x + _internalBuffer[index].Src.left;
			  destBuffer[destOffset + normalizedX] = sourceBuffer[sourceYIndex * _numColumns + sourceXIndex];
		  }
		  destOffset += destWidth;
	  }
  }
}
//*/

void RenderTable::mutateImage(Graphics::Surface *dstBuf, Graphics::Surface *srcBuf, bool highQuality) {
	uint32 destOffset = 0;
  uint32 srcIndexXL = 0;
  uint32 srcIndexXR = 0;
  uint32 srcIndexYT = 0;
  uint32 srcIndexYB = 0;
	uint16 *sourceBuffer = (uint16 *)srcBuf->getPixels();
	uint16 *destBuffer = (uint16 *)dstBuf->getPixels();
	if(highQuality != _highQuality) {
	  _highQuality = highQuality;
	  generateRenderTable();
	}
  uint32 mutationTime = _system->getMillis();
  if(_highQuality) {
    //Apply bilinear interpolation
	  FilterPixel _curP;
	  uint32 index;
	  uint32 rTL,rTR,rBL,rBR,rF;
	  uint32 gTL,gTR,gBL,gBR,gF;
	  uint32 bTL,bTR,bBL,bBR,bF;
	  for (int16 y = 0; y < srcBuf->h; ++y) {
		  uint32 sourceOffset = y * _numColumns;
		  for (int16 x = 0; x < srcBuf->w; ++x) {
			  index = sourceOffset + x;
			  _curP = _internalBuffer[index];
			  srcIndexYT = y + _curP.Src.top;
			  srcIndexYB = y + _curP.Src.bottom;
			  srcIndexXL = x + _curP.Src.left;
			  srcIndexXR = x + _curP.Src.right;
        splitColor(sourceBuffer[srcIndexYT * _numColumns + srcIndexXL], rTL, gTL, bTL);
        splitColor(sourceBuffer[srcIndexYT * _numColumns + srcIndexXR], rTR, gTR, bTR);
        splitColor(sourceBuffer[srcIndexYB * _numColumns + srcIndexXL], rBL, gBL, bBL);
        splitColor(sourceBuffer[srcIndexYB * _numColumns + srcIndexXR], rBR, gBR, bBR);      
        rF = round(_curP.fTL*rTL + _curP.fTR*rTR + _curP.fBL*rBL + _curP.fBR*rBR);
        gF = round(_curP.fTL*gTL + _curP.fTR*gTR + _curP.fBL*gBL + _curP.fBR*gBR);
        bF = round(_curP.fTL*bTL + _curP.fTR*bTR + _curP.fBL*bBL + _curP.fBR*bBR);
        destBuffer[destOffset] = mergeColor(rF,gF,bF);
		    destOffset++;
      }
    }
	}
  else {
    //Apply nearest-neighbour interpolation
	  for (int16 y = 0; y < srcBuf->h; ++y) {
		  uint32 sourceOffset = y * _numColumns;
		  for (int16 x = 0; x < srcBuf->w; ++x) {
			  uint32 index = sourceOffset + x;
			  // RenderTable only stores offsets from the original coordinates
    		srcIndexXL = x + (_internalBuffer[index].xDir ? _internalBuffer[index].Src.right : _internalBuffer[index].Src.left);			  
			  srcIndexYT = y + (_internalBuffer[index].yDir ? _internalBuffer[index].Src.bottom : _internalBuffer[index].Src.top);
			  destBuffer[destOffset] = sourceBuffer[srcIndexYT * _numColumns + srcIndexXL];
			  destOffset++;
		  }
	  }
  }
  mutationTime = _system->getMillis() - mutationTime;
  debug(1,"\tPanorama mutation time %dms, %s quality", mutationTime, _engine->getScriptManager()->getStateValue(StateKey_HighQuality) ? "high" : "low");
}

void RenderTable::generateRenderTable() {
	switch (_renderState) {
	  case RenderTable::PANORAMA: {
      uint32 generationTime = _system->getMillis();
		  generatePanoramaLookupTable();
      generationTime = _system->getMillis() - generationTime;		  
      debug(1,"\tRender table generation time %dms, %s quality", generationTime, _engine->getScriptManager()->getStateValue(StateKey_HighQuality) ? "high" : "low");
		  break;
	  }
	  case RenderTable::TILT:
		  generateTiltLookupTable();
		  break;
	  case RenderTable::FLAT:
		  // Intentionally left empty
		  break;
	  default:
		  break;
	}
}

void RenderTable::generatePanoramaLookupTable() {
  debug(1,"Generating panorama lookup table.");
	uint halfRows = ceil(_numRows/2);
	uint halfColumns = ceil(_numColumns/2);
	float halfWidth = (float)_numColumns / 2.0f;
	float halfHeight = (float)_numRows / 2.0f;
	float cylinderRadius = halfHeight / tan(_panoramaOptions.verticalFOV);
	float xOffset = 0.0f;
	float yOffset = 0.0f;
	
	debug(1,"halfWidth %f, halfHeight %f", halfWidth, halfHeight);
	debug(1,"halfRows %d, halfColumns %d", halfRows, halfColumns);
	
	//Transformation is both horizontally and vertically symmetrical about the camera axis,
	//We can thus save on trigonometric calculations by computing one quarter of the transformation matrix and then mirroring it in both X & Y
	//TODO - find & fix cause of vertial "seam" in Nemesis.
	for (uint x = 0; x < halfColumns; ++x) {
		// Add an offset of 0.01 to overcome zero tan/atan issue (vertical line on half of screen)
		// Alpha represents the horizontal angle between the viewer at the center of a cylinder and x
		float alpha = atan(((float)x - halfWidth + 0.01f) / cylinderRadius);

		// To get x in cylinder coordinates, we just need to calculate the arc length
		// We also scale it by _panoramaOptions.linearScale
		float xInCylinderCoords = (cylinderRadius * _panoramaOptions.linearScale * alpha) + halfWidth;
		float cosAlpha = cos(alpha);
		uint32 columnIndexL = x;
		uint32 columnIndexR = (_numColumns - 1) - x;
		uint32 rowIndexT = 0;
		uint32 rowIndexB = _numColumns * (_numRows - 1);

		for (uint y = 0; y < halfRows; ++y) {
			// To calculate y in cylinder coordinates, we can do similar triangles comparison,
			// comparing the triangle from the center to the screen and from the center to the edge of the cylinder
			float yInCylinderCoords = halfHeight + ((float)y - halfHeight) * cosAlpha;
			
			uint32 indexTL = rowIndexT + columnIndexL;
			uint32 indexBL = rowIndexB + columnIndexL;
			uint32 indexTR = rowIndexT + columnIndexR;
			uint32 indexBR = rowIndexB + columnIndexR;
			
			xOffset = xInCylinderCoords - x; 
      yOffset = yInCylinderCoords - y;
      
      bool _printDebug = (Common::Point(x,y)==testPixel);
      if(_printDebug) {
        debug(2,"\tGenerating test pixel %d, %d", x, y);
        debug(2,"\tCylinder coordinates %f, %f", xInCylinderCoords, yInCylinderCoords);
        debug(2,"\tOffsets %f,%f", xOffset, yOffset);
      } 
			// Only store the (x,y) offsets instead of the absolute positions
			_internalBuffer[indexTL] = FilterPixel(xOffset, yOffset, _highQuality, _printDebug);
			//Store mirrored offset values
			_internalBuffer[indexBL] = _internalBuffer[indexTL];
			_internalBuffer[indexBL].flipV();
			_internalBuffer[indexTR] = _internalBuffer[indexTL];
			_internalBuffer[indexTR].flipH();
			_internalBuffer[indexBR] = _internalBuffer[indexBL];
			_internalBuffer[indexBR].flipH();
			//Increment indices
			rowIndexT += _numColumns;
			rowIndexB -= _numColumns;
		}
	}
	debug(1,"Render table generated, %s quality", _highQuality ? "high" : "low");
}

void RenderTable::generateTiltLookupTable() {
	uint halfRows = ceil(_numRows/2);
	uint halfColumns = ceil(_numColumns/2);
	float halfWidth = (float)_numColumns / 2.0f;
	float halfHeight = (float)_numRows / 2.0f;
	float cylinderRadius = halfWidth / tan(_tiltOptions.verticalFOV);
	float xOffset = 0.0f;
	float yOffset = 0.0f;
	_tiltOptions.gap = cylinderRadius * atan2((float)(halfHeight / cylinderRadius), 1.0f) * _tiltOptions.linearScale;
	
	//Transformation is both horizontally and vertically symmetrical about the camera axis,
	//We can thus save on trigonometric calculations by computing one quarter of the transformation matrix and then mirroring it in both X & Y
	for (uint y = 0; y < halfRows; ++y) {
		// Add an offset of 0.01 to overcome zero tan/atan issue (horizontal line on half of screen)
		// Alpha represents the vertical angle between the viewer at the center of a cylinder and y
		float alpha = atan(((float)y - halfHeight + 0.01f) / cylinderRadius);

		// To get y in cylinder coordinates, we just need to calculate the arc length
		// We also scale it by _tiltOptions.linearScale
		int32 yInCylinderCoords = int32(round((cylinderRadius * _tiltOptions.linearScale * alpha) + halfHeight));

		float cosAlpha = cos(alpha);
		uint32 columnIndexTL = y * _numColumns;
		uint32 columnIndexBL = (_numRows-(y+1)) * _numColumns;
		uint32 columnIndexTR = columnIndexTL + (_numColumns - 1);
		uint32 columnIndexBR = columnIndexBL + (_numColumns - 1);

  //TODO - all four pixel values are always the same; find out why & fix.

		for (uint x = 0; x < halfColumns; ++x) {
			// To calculate x in cylinder coordinates, we can do similar triangles comparison,
			// comparing the triangle from the center to the screen and from the center to the edge of the cylinder
			int32 xInCylinderCoords = int32(round(halfWidth + ((float)x - halfWidth) * cosAlpha));

			uint32 indexTL = columnIndexTL + x;
			uint32 indexBL = columnIndexBL + x;
			uint32 indexTR = columnIndexTR - x;
			uint32 indexBR = columnIndexBR - x;
			
			xOffset = xInCylinderCoords - x;
      yOffset = yInCylinderCoords - y;
      
      bool _printDebug = (Common::Point(x,y)==testPixel);
      if(_printDebug) {
        debug(2,"\tGenerating test pixel %d, %d", x, y);
        debug(2,"\tCylinder coordinates %d, %d", xInCylinderCoords, yInCylinderCoords);
        debug(2,"\tOffsets %f,%f", xOffset, yOffset);
      }
			// Only store the (x,y) offsets instead of the absolute positions
			_internalBuffer[indexTL] = FilterPixel(xOffset, yOffset, _highQuality, _printDebug);			
			//Store mirrored offset values
			_internalBuffer[indexBL] = FilterPixel(xOffset, -yOffset, _highQuality);
			_internalBuffer[indexTR] = FilterPixel(-xOffset, yOffset, _highQuality);
			_internalBuffer[indexBR] = FilterPixel(-xOffset, -yOffset, _highQuality);
		}
	}
}

void RenderTable::setPanoramaFoV(float fov) {
	assert(fov > 0.0f);
	_panoramaOptions.verticalFOV = Math::deg2rad<float>(fov);
}

void RenderTable::setPanoramaScale(float scale) {
	assert(scale > 0.0f);
	_panoramaOptions.linearScale = scale;
}

void RenderTable::setPanoramaReverse(bool reverse) {
	_panoramaOptions.reverse = reverse;
}

bool RenderTable::getPanoramaReverse() {
	return _panoramaOptions.reverse;
}

void RenderTable::setPanoramaZeroPoint(uint16 point) {
	_panoramaOptions.zeroPoint = point;
}

uint16 RenderTable::getPanoramaZeroPoint() {
	return _panoramaOptions.zeroPoint;
}

void RenderTable::setTiltFoV(float fov) {
	assert(fov > 0.0f);
	_tiltOptions.verticalFOV = Math::deg2rad<float>(fov);
}

void RenderTable::setTiltScale(float scale) {
	assert(scale > 0.0f);
	_tiltOptions.linearScale = scale;
}

void RenderTable::setTiltReverse(bool reverse) {
	_tiltOptions.reverse = reverse;
}

float RenderTable::getTiltGap() {
	return _tiltOptions.gap;
}

float RenderTable::getAngle() {
	switch(_renderState) {
	  case TILT:
  		return Math::rad2deg<float>(_tiltOptions.verticalFOV);
		case PANORAMA:
		  return Math::rad2deg<float>(_panoramaOptions.verticalFOV);
	  default:
  		return 1.0;
	}
}

float RenderTable::getLinscale() {
	switch(_renderState) {
	  case TILT:
  		return _tiltOptions.linearScale;
	  case PANORAMA:
  		return _panoramaOptions.linearScale;
	  default:
  		return 1.0;
	}
}

} // End of namespace ZVision
