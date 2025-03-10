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
	halfRows = floor((_numRows-1)/2);
	halfColumns = floor((_numColumns-1)/2);
	halfWidth = (float)_numColumns / 2.0f - 0.5f;
	halfHeight = (float)_numRows / 2.0f - 0.5f;
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
	index = point.y * _numColumns + point.x;
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
	destOffset = 0;
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
	  for (int16 y = 0; y < srcBuf->h; ++y) {
		  sourceOffset = y * _numColumns;
		  for (int16 x = 0; x < srcBuf->w; ++x) {
			  _curP = _internalBuffer[sourceOffset + x];
			  srcIndexYT = y + _curP.Src.top;
			  srcIndexYB = y + _curP.Src.bottom;
			  srcIndexXL = x + _curP.Src.left;
			  srcIndexXR = x + _curP.Src.right;
        splitColor(sourceBuffer[srcIndexYT * _numColumns + srcIndexXL], rTL, gTL, bTL);
        splitColor(sourceBuffer[srcIndexYT * _numColumns + srcIndexXR], rTR, gTR, bTR);
        splitColor(sourceBuffer[srcIndexYB * _numColumns + srcIndexXL], rBL, gBL, bBL);
        splitColor(sourceBuffer[srcIndexYB * _numColumns + srcIndexXR], rBR, gBR, bBR);
        rF = _curP.fTL*rTL + _curP.fTR*rTR + _curP.fBL*rBL + _curP.fBR*rBR;
        gF = _curP.fTL*gTL + _curP.fTR*gTR + _curP.fBL*gBL + _curP.fBR*gBR;
        bF = _curP.fTL*bTL + _curP.fTR*bTR + _curP.fBL*bBL + _curP.fBR*bBR;
        destBuffer[destOffset] = mergeColor(rF,gF,bF);
		    destOffset++;
	      /*/
        if(Common::Point(x,y)==testPixel) {
          debug(2,"\tMutated test pixel %d, %d", x, y);
          debug(2,"\tfX: %f, fY: %f", _curP.fX, _curP.fY);
          debug(2,"\tYT: %d, YB: %d, XL: %d XR: %d", srcIndexYT, srcIndexYB, srcIndexXL, srcIndexXR);
        } 
        //*/
      }
    }
	}
  else {
    //Apply nearest-neighbour interpolation
	  for (int16 y = 0; y < srcBuf->h; ++y) {
		  sourceOffset = y * _numColumns;
		  for (int16 x = 0; x < srcBuf->w; ++x) {
			  index = sourceOffset + x;
			  // RenderTable only stores offsets from the original coordinates
    		srcIndexXL = x + (_internalBuffer[index].xDir ? _internalBuffer[index].Src.right : _internalBuffer[index].Src.left);			  
			  srcIndexYT = y + (_internalBuffer[index].yDir ? _internalBuffer[index].Src.bottom : _internalBuffer[index].Src.top);
			  destBuffer[destOffset] = sourceBuffer[srcIndexYT * _numColumns + srcIndexXL];
			  destOffset++;
		  }
	  }
  }
  mutationTime = _system->getMillis() - mutationTime;
  debug(5,"\tPanorama mutation time %dms, %s quality", mutationTime, _highQuality? "high" : "low");
}

void RenderTable::generateRenderTable() {
	switch (_renderState) {
	  case RenderTable::PANORAMA: {
      generateLookupTable(false);
		  break;
	  }
	  case RenderTable::TILT:
	    generateLookupTable(true);
		  break;
	  case RenderTable::FLAT:
		  // Intentionally left empty
		  break;
	  default:
		  break;
	}
}

void RenderTable::generateLookupTable(bool tilt) {
  debug(1,"Generating %s lookup table.", tilt ? "tilt" : "panorama");
	debug(1,"halfWidth %f, halfHeight %f", halfWidth, halfHeight);
	debug(1,"halfRows %d, halfColumns %d", halfRows, halfColumns);
  uint32 generationTime = _system->getMillis();
	float alpha, cosAlpha, polarCoordInCylinderCoords, linearCoordInCylinderCoords, cylinderRadius, xOffset, yOffset;
	uint32 indexTL, indexBL, indexTR, indexBR;
	uint x=0;
	uint y=0;
	auto outerLoop = [&](uint &polarCoord, float &halfPolarSize, float &scale) {
    //polarCoord is the coordinate of the working window pixel parallel to the direction of camera rotation
    //halfPolarSize is the distance from the central axis to the outermost working window pixel in the direction of camera rotation
		//alpha represents the angle in the direction of camera rotation between the view axis and the centre of a pixel at the given polar coordinate
		alpha = atan(((float)polarCoord - halfPolarSize) / cylinderRadius);
		// To map the polar coordinate to the cylinder surface coordinates, we just need to calculate the arc length
		// We also scale it by linearScale
		polarCoordInCylinderCoords = (cylinderRadius * scale * alpha) + halfPolarSize;
		cosAlpha = cos(alpha);
  };	  
  auto innerLoop = [&](uint &polarCoord, uint &linearCoord, float &halfLinearSize,  float &polarOffset, float &linearOffset) {
		// To calculate linear coordinate in cylinder coordinates, we can do similar triangles comparison,
		// comparing the triangle from the center to the screen and from the center to the edge of the cylinder
		linearCoordInCylinderCoords = halfLinearSize + ((float)linearCoord - halfLinearSize) * cosAlpha;
		linearOffset = linearCoordInCylinderCoords - linearCoord;
    polarOffset = polarCoordInCylinderCoords - polarCoord;
    bool _printDebug = (Common::Point(x,y)==testPixel);
    if(_printDebug) {
      debug(2,"\tGenerating test pixel %d, %d", x, y);
      debug(2,"\tOffsets %f,%f", xOffset, yOffset);
    } 
		// Only store the (x,y) offsets instead of the absolute positions
		_internalBuffer[indexTL] = FilterPixel(xOffset, yOffset, _highQuality, _printDebug);
    //Transformation is both horizontally and vertically symmetrical about the camera axis,
    //We can thus save on trigonometric calculations by computing one quarter of the transformation matrix and then mirroring it in both X & Y:
		_internalBuffer[indexBL] = _internalBuffer[indexTL];
		_internalBuffer[indexBL].flipV();
		_internalBuffer[indexTR] = _internalBuffer[indexTL];
		_internalBuffer[indexTR].flipH();
		_internalBuffer[indexBR] = _internalBuffer[indexBL];
		_internalBuffer[indexBR].flipH();
  };
  if(tilt) {
    uint32 columnIndexTL, columnIndexBL, columnIndexTR, columnIndexBR;
	  cylinderRadius = (halfWidth + 0.5f) / tan(_tiltOptions.verticalFOV);
	  _tiltOptions.gap = cylinderRadius * atan2((float)(halfHeight / cylinderRadius), 1.0f) * _tiltOptions.linearScale;
	  for (y = 0; y <= halfRows; ++y) {
	    outerLoop(y, halfHeight, _tiltOptions.linearScale);
		  columnIndexTL = y * _numColumns;
		  columnIndexBL = (_numRows-(y+1)) * _numColumns;
		  columnIndexTR = columnIndexTL + (_numColumns - 1);
		  columnIndexBR = columnIndexBL + (_numColumns - 1);
	    for (x = 0; x <= halfColumns; ++x) {
			  indexTL = columnIndexTL + x;
			  indexBL = columnIndexBL + x;
			  indexTR = columnIndexTR - x;
			  indexBR = columnIndexBR - x;
	      innerLoop(y, x, halfWidth, yOffset, xOffset);
      }
    }
	}
  else {
    uint32 rowIndexT, rowIndexB, columnIndexL, columnIndexR;
    cylinderRadius = (halfHeight + 0.5f) / tan(_panoramaOptions.verticalFOV);
	  for (x = 0; x <= halfColumns; ++x) {
  		columnIndexL = x;
		  columnIndexR = (_numColumns - 1) - x;
		  rowIndexT = 0;
		  rowIndexB = _numColumns * (_numRows - 1);
	    outerLoop(x, halfWidth, _panoramaOptions.linearScale);
		  for (y = 0; y <= halfRows; ++y) {
		    indexTL = rowIndexT + columnIndexL;
		    indexBL = rowIndexB + columnIndexL;
		    indexTR = rowIndexT + columnIndexR;
		    indexBR = rowIndexB + columnIndexR;
		    innerLoop(x, y, halfHeight, xOffset, yOffset);
		    rowIndexT += _numColumns;
		    rowIndexB -= _numColumns;
	    }
	  }
  }
  generationTime = _system->getMillis() - generationTime;		  
	debug(1,"Render table generated, %s quality", _highQuality ? "high" : "low");
  debug(1,"\tRender table generation time %dms", generationTime);
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
