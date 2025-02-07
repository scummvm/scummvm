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

#include "zvision/graphics/render_table.h"

#include "common/rect.h"
#include "common/scummsys.h"
#include "math/utils.h"

namespace ZVision {

RenderTable::RenderTable(uint numColumns, uint numRows, const Graphics::PixelFormat pixelFormat)
	: _numRows(numRows),
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
	newPoint.x += (_internalBuffer[index].xDir & 0x01 ? _internalBuffer[index].Src.right : _internalBuffer[index].Src.left);
	newPoint.y += (_internalBuffer[index].yDir & 0x01 ? _internalBuffer[index].Src.bottom : _internalBuffer[index].Src.top);
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
	uint32 bufferTL = 0;
	uint32 bufferBL = 0;
	uint32 bufferTR = 0;
	uint32 bufferBR = 0;
	uint32 bufferTL_G = 0;
	uint32 bufferBL_G = 0;
	uint32 bufferTR_G = 0;
	uint32 bufferBR_G = 0;


	if(highQuality != _highQuality) {
	  _highQuality = highQuality;
	  generateRenderTable();
	}
	
  if(_highQuality) {
    //Apply bilinear interpolation
	  FilterPixel _curP;
	  uint8 xCount = 0;
	  uint8 yCount = 0;
	  uint8 xDir = 0;
	  uint8 yDir = 0;
	  
    //Deb
	  uint8 r = 0;
	  uint8 g = 0;
	  uint8 b = 0;
	  
	  for (int16 y = 0; y < srcBuf->h; ++y) {
		  uint32 sourceOffset = y * _numColumns;
		  for (int16 x = 0; x < srcBuf->w; ++x) {
		    xCount = 0;
		    yCount = 0;
			  uint32 index = sourceOffset + x;
			  // RenderTable only stores offsets from the original coordinates
			  _curP = _internalBuffer[index];
			  xDir = _curP.xDir;
			  yDir = _curP.yDir;

			  srcIndexYT = y + _curP.Src.top;
			  srcIndexYB = y + _curP.Src.bottom;
			  srcIndexXL = x + _curP.Src.left;
			  srcIndexXR = x + _curP.Src.right;
			  bufferTL = sourceBuffer[srcIndexYT * _numColumns + srcIndexXL];
			  bufferTR = sourceBuffer[srcIndexYT * _numColumns + srcIndexXR];
			  bufferBL = sourceBuffer[srcIndexYB * _numColumns + srcIndexXL];
			  bufferBR = sourceBuffer[srcIndexYB * _numColumns + srcIndexXR];
        if(_curP._printDebug) {
          _pixelFormat.colorToRGB(bufferTL,r,g,b);
          debug(2,"Original pixel value TL %s, RGB: %d,%d,%d", pixelToBinary(bufferTL).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferTR,r,g,b);
          debug(2,"Original pixel value TR %s, RGB: %d,%d,%d", pixelToBinary(bufferTR).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferBL,r,g,b);
          debug(2,"Original pixel value BL %s, RGB: %d,%d,%d", pixelToBinary(bufferBL).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferBR,r,g,b);
          debug(2,"Original pixel value BR %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
			  }
			  //Extract green values
			  bufferTL_G = bufferTL & 0x03e0;
			  bufferBL_G = bufferBL & 0x03e0;
			  bufferTR_G = bufferTR & 0x03e0;
			  bufferBR_G = bufferBR & 0x03e0;
		    //Mask for red & blue values
			  bufferTL &= 0x7c1f;
			  bufferBL &= 0x7c1f;
			  bufferTR &= 0x7c1f;
			  bufferBR &= 0x7c1f;
			  
			  if(_curP._printDebug) {
  			  debug(2,"\tTest pixel %d,%d", x, y);
  			  debug(2,"\tX byte fraction %d, Y byte fraction %d", _curP.fracX, _curP.fracY);
  			  debug(2,"\tX steps %d, Y steps %d", _curP.xSteps, _curP.ySteps);
  			  debug(2,"\tXdir 0x%X, Ydir 0x%X", xDir, _curP.yDir);
			  }
			  
        if(_curP._printDebug) {
          _pixelFormat.colorToRGB(bufferTL,r,g,b);
          debug(2,"Separated pixel value TL   %s, RGB: %d,%d,%d", pixelToBinary(bufferTL).c_str(), r,g,b);
          _pixelFormat.colorToRGB(bufferTR,r,g,b);
          debug(2,"Separated pixel value TR   %s, RGB: %d,%d,%d", pixelToBinary(bufferTR).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferBL,r,g,b);
          debug(2,"Separated pixel value BL   %s, RGB: %d,%d,%d", pixelToBinary(bufferBL).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferBR,r,g,b);
          debug(2,"Separated pixel value BR   %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferTL_G,r,g,b);
          debug(2,"Separated pixel value TL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTL_G).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferTR_G,r,g,b);
          debug(2,"Separated pixel value TR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTR_G).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferBL_G,r,g,b);
          debug(2,"Separated pixel value BL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBL_G).c_str(),r,g,b);
          _pixelFormat.colorToRGB(bufferBR_G,r,g,b);
          debug(2,"Separated pixel value BR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBR_G).c_str(),r,g,b);
		    }
			  
        //Horizontal contraction
		    if(_curP.ySteps > 1) {
          while(_curP.xSteps > xCount+1) {
            xCount++;
            if(_curP.xDir & 0x01) {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting right, T&B");
		          contractLeft(bufferTR, bufferTL);
		          contractLeft(bufferBR, bufferBL);
		          contractLeft(bufferTR_G, bufferTL_G);
		          contractLeft(bufferBR_G, bufferBL_G);
            }
            else {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting left, T&B");
		          contractLeft(bufferTL, bufferTR);
		          contractLeft(bufferBL, bufferBR);
		          contractLeft(bufferTL_G, bufferTR_G);
		          contractLeft(bufferBL_G, bufferBR_G);
            }
            _curP.xDir >>= 1;
            		          
            if(_curP._printDebug) {
		          debug(2,"\t\txDir 0x%X", _curP.xDir);	   
              _pixelFormat.colorToRGB(bufferTL,r,g,b);
              debug(2,"Contracted pixel value TL   %s, RGB: %d,%d,%d", pixelToBinary(bufferTL).c_str(), r,g,b);
              _pixelFormat.colorToRGB(bufferTR,r,g,b);
              debug(2,"Contracted pixel value TR   %s, RGB: %d,%d,%d", pixelToBinary(bufferTR).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBL,r,g,b);
              debug(2,"Contracted pixel value BL   %s, RGB: %d,%d,%d", pixelToBinary(bufferBL).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBR,r,g,b);
              debug(2,"Contracted pixel value BR   %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferTL_G,r,g,b);
              debug(2,"Contracted pixel value TL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTL_G).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferTR_G,r,g,b);
              debug(2,"Contracted pixel value TR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTR_G).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBL_G,r,g,b);
              debug(2,"Contracted pixel value BL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBL_G).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBR_G,r,g,b);
              debug(2,"Contracted pixel value BR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBR_G).c_str(),r,g,b);
			      }
          }
//*/
          if(_curP.xDir & 0x01) {
            bufferBR = (bufferBR >> xCount) & 0x7c1f;
            bufferBR_G = (bufferBR_G >> xCount) & 0x03e0;
            bufferTR = (bufferTR >> xCount) & 0x7c1f;
            bufferTR_G = (bufferTR_G >> xCount) & 0x03e0;
          }
          else {
            bufferBL = (bufferBL >> xCount) & 0x7c1f;
            bufferBL_G = (bufferBL_G >> xCount) & 0x03e0;
            bufferTL = (bufferTL >> xCount) & 0x7c1f;
            bufferTL_G = (bufferTL_G >> xCount) & 0x03e0;
          }
          if(_curP._printDebug) {
            debug(2,"\tShifted back %d places", xCount);
            _pixelFormat.colorToRGB(bufferTL,r,g,b);
            debug(2,"Shifted pixel value TL   %s, RGB: %d,%d,%d", pixelToBinary(bufferTL).c_str(), r,g,b);
            _pixelFormat.colorToRGB(bufferTR,r,g,b);
            debug(2,"Shifted pixel value TR   %s, RGB: %d,%d,%d", pixelToBinary(bufferTR).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferBL,r,g,b);
            debug(2,"Shifted pixel value BL   %s, RGB: %d,%d,%d", pixelToBinary(bufferBL).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferBR,r,g,b);
            debug(2,"Shifted pixel value BR   %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferTL_G,r,g,b);
            debug(2,"Shifted pixel value TL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTL_G).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferTR_G,r,g,b);
            debug(2,"Shifted pixel value TR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTR_G).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferBL_G,r,g,b);
            debug(2,"Shifted pixel value BL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBL_G).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferBR_G,r,g,b);
            debug(2,"Shifted pixel value BR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBR_G).c_str(),r,g,b);
			    }
//*/
        }
        else if (_curP.yDir) {
          while(_curP.xSteps > xCount+1) {
            xCount++;
            if(_curP.xDir & 0x01) {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting right, B");
		          contractLeft(bufferBR, bufferBL);
		          contractLeft(bufferBR_G, bufferBL_G);
            }
            else {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting left, B");
		          contractLeft(bufferBL, bufferBR);
		          contractLeft(bufferBL_G, bufferBR_G);
            }
            _curP.xDir >>= 1;
            
            if(_curP._printDebug) {
              _pixelFormat.colorToRGB(bufferBL,r,g,b);
              debug(2,"Contracted pixel value BL   %s, RGB: %d,%d,%d", pixelToBinary(bufferBL).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBR,r,g,b);
              debug(2,"Contracted pixel value BR   %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBL_G,r,g,b);
              debug(2,"Contracted pixel value BL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBL_G).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBR_G,r,g,b);
              debug(2,"Contracted pixel value BR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBR_G).c_str(),r,g,b);
			      }
          }
//*/
          if(_curP.xDir & 0x01) {
            bufferBR = (bufferBR >> xCount) & 0x7c1f;
            bufferBR_G = (bufferBR_G >> xCount) & 0x03e0;
          }
          else {
            bufferBL = (bufferBL >> xCount) & 0x7c1f;
            bufferBL_G = (bufferBL_G >> xCount) & 0x03e0;
          }
          if(_curP._printDebug) {
            debug(2,"\tShifted back %d places", xCount);
            _pixelFormat.colorToRGB(bufferBL,r,g,b);
            debug(2,"Shifted pixel value BL   %s, RGB: %d,%d,%d", pixelToBinary(bufferBL).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferBR,r,g,b);
            debug(2,"Shifted pixel value BR   %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferBL_G,r,g,b);
            debug(2,"Shifted pixel value BL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBL_G).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferBR_G,r,g,b);
            debug(2,"Shifted pixel value BR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBR_G).c_str(),r,g,b);
			    }
//*/
        }
        else {
          while(_curP.xSteps > xCount+1) {
            xCount++;
            if(_curP.xDir & 0x01) {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting right, T");
		          contractLeft(bufferTR, bufferTL);
		          contractLeft(bufferTR_G, bufferTL_G);
            }
            else {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting left, T");
		          contractLeft(bufferTL, bufferTR);
		          contractLeft(bufferTL_G, bufferTR_G);
            }
            _curP.xDir >>= 1;
            
            if(_curP._printDebug) {
              _pixelFormat.colorToRGB(bufferTL,r,g,b);
              debug(2,"Contracted pixel value TL   %s, RGB: %d,%d,%d", pixelToBinary(bufferTL).c_str(), r,g,b);
              _pixelFormat.colorToRGB(bufferTR,r,g,b);
              debug(2,"Contracted pixel value TR   %s, RGB: %d,%d,%d", pixelToBinary(bufferTR).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferTL_G,r,g,b);
              debug(2,"Contracted pixel value TL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTL_G).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferTR_G,r,g,b);
              debug(2,"Contracted pixel value TR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTR_G).c_str(),r,g,b);
			      }
          }
//*/
          if(_curP.xDir & 0x01) {
            bufferTR = (bufferTR >> xCount) & 0x7c1f;
            bufferTR_G = (bufferTR_G >> xCount) & 0x03e0;
          }
          else {
            bufferTL = (bufferTL >> xCount) & 0x7c1f;
            bufferTL_G = (bufferTL_G >> xCount) & 0x03e0;
          }
          if(_curP._printDebug) {
            debug(2,"\tShifted back %d places", xCount);
            _pixelFormat.colorToRGB(bufferTL,r,g,b);
            debug(2,"Shifted pixel value TL   %s, RGB: %d,%d,%d", pixelToBinary(bufferTL).c_str(), r,g,b);
            _pixelFormat.colorToRGB(bufferTR,r,g,b);
            debug(2,"Shifted pixel value TR   %s, RGB: %d,%d,%d", pixelToBinary(bufferTR).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferTL_G,r,g,b);
            debug(2,"Shifted pixel value TL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTL_G).c_str(),r,g,b);
            _pixelFormat.colorToRGB(bufferTR_G,r,g,b);
            debug(2,"Shifted pixel value TR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTR_G).c_str(),r,g,b);
			    }
//*/
        }
        
        //Vertical contraction
        if(_curP.xDir & 0x01) {
          while(_curP.ySteps > yCount+1) {
            yCount++;
            if(_curP.yDir & 0x01) {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting downward, R");
		          contractLeft(bufferBR, bufferTR);
		          contractLeft(bufferBR_G, bufferTR_G);
            }
            else {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting upward, R");
		          contractLeft(bufferTR, bufferBR);
		          contractLeft(bufferTR_G, bufferBR_G);
            }
            _curP.yDir >>= 1;
            
            if(_curP._printDebug) {
              _pixelFormat.colorToRGB(bufferTR,r,g,b);
              debug(2,"Contracted pixel value TR   %s, RGB: %d,%d,%d", pixelToBinary(bufferTR).c_str(), r,g,b);
              _pixelFormat.colorToRGB(bufferBR,r,g,b);
              debug(2,"Contracted pixel value BR   %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferTR_G,r,g,b);
              debug(2,"Contracted pixel value TR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTR_G).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBR_G,r,g,b);
              debug(2,"Contracted pixel value BR_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBR_G).c_str(),r,g,b);
			      }
          }
        }
        else {
          while(_curP.ySteps > yCount+1) {
            yCount++;
		        if(_curP._printDebug)
		          debug(2,"yDir %x", _curP.yDir);
            if(_curP.yDir & 0x01) {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting downward, L");
		          contractLeft(bufferBL, bufferTL);
		          contractLeft(bufferBL_G, bufferTL_G);
            }
            else {
			        if(_curP._printDebug)
			          debug(2,"\t\tContracting upward, L");
		          contractLeft(bufferTL, bufferBL);
		          contractLeft(bufferTL_G, bufferBL_G);
            }
            _curP.yDir >>= 1;
            
            if(_curP._printDebug) {
              _pixelFormat.colorToRGB(bufferTL,r,g,b);
              debug(2,"Contracted pixel value TL   %s, RGB: %d,%d,%d", pixelToBinary(bufferTL).c_str(), r,g,b);
              _pixelFormat.colorToRGB(bufferBL,r,g,b);
              debug(2,"Contracted pixel value BL   %s, RGB: %d,%d,%d", pixelToBinary(bufferBL).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferTL_G,r,g,b);
              debug(2,"Contracted pixel value TL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferTL_G).c_str(),r,g,b);
              _pixelFormat.colorToRGB(bufferBL_G,r,g,b);
              debug(2,"Contracted pixel value BL_G %s, RGB: %d,%d,%d", pixelToBinary(bufferBL_G).c_str(),r,g,b);
			      }
          }
        }
        if(_curP._printDebug)
          debug(2,"yCount %d", yCount);
        //Final value
        if(_curP.yDir & 0x01) {
          if(_curP.xDir & 0x01) {
		        if(_curP._printDebug)
		          debug(2,"\t\tPicking final value, BR");
            destBuffer[destOffset] = ((bufferBR >> yCount) & 0x7c1f) | ((bufferBR_G >> yCount) & 0x03e0);
          }
          else {
		        if(_curP._printDebug)
		          debug(2,"\t\tPicking final value, BL");
            destBuffer[destOffset] = ((bufferBL >> yCount) & 0x7c1f) | ((bufferBL_G >> yCount) & 0x03e0);
          }
        }
        else {
          if(_curP.xDir & 0x01) {
		        if(_curP._printDebug)
		          debug(2,"\t\tPicking final value, TR");
            destBuffer[destOffset] = ((bufferTR >> yCount) & 0x7c1f) | ((bufferTR_G >> yCount) & 0x03e0);
          }
          else {
		        if(_curP._printDebug)
		          debug(2,"\t\tPicking final value, TL");
            destBuffer[destOffset] = ((bufferTL >> yCount) & 0x7c1f) | ((bufferTL_G >> yCount) & 0x03e0);
          }
        }        
        if(_curP._printDebug) { 
          _pixelFormat.colorToRGB(destBuffer[destOffset],r,g,b);
          debug(2,"Final pixel value %s, RGB: %d,%d,%d", pixelToBinary(bufferBR).c_str(),r,g,b);
        }    
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
	
	uint pixelAverageCount = 0;
	
	//Transformation is both horizontally and vertically symmetrical about the camera axis,
	//We can thus save on trigonometric calculations by computing one quarter of the transformation matrix and then mirroring it in both X & Y
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
			_internalBuffer[indexBL] = FilterPixel(xOffset, -yOffset, _highQuality);
			_internalBuffer[indexTR] = FilterPixel(-xOffset, yOffset, _highQuality);
			_internalBuffer[indexBR] = FilterPixel(-xOffset, -yOffset, _highQuality);
			
			pixelAverageCount += _internalBuffer[indexTL].xSteps + _internalBuffer[indexTL].ySteps;
			
			//Increment indices
			rowIndexT += _numColumns;
			rowIndexB -= _numColumns;
		}
	}
	debug(1,"Render table generated, %s quality, filter tolerance %d, total pixel averaging operations per frame %e", _highQuality ? "high" : "low", _internalBuffer[0].tol, (float)pixelAverageCount*4);
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
        debug(2,"\tCylinder coordinates %f, %f", xInCylinderCoords, yInCylinderCoords);
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
