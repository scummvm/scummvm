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

#ifndef ZVISION_RENDER_TABLE_H
#define ZVISION_RENDER_TABLE_H

#include "common/rect.h"
#include "graphics/surface.h"

namespace ZVision {

class FilterPixel {
public:
  const static uint8 colorBits = 5;
  const static uint8 tol = 1;
  const static uint8 pFrac = 0xff;
  //Bitfields representing sequential direction of contraction
  uint8 xDir = 0; //0 contract left, 1 contract right
  uint8 yDir = 0; //0 contract up, 1 contract down
  uint8 xSteps = 0; //Number of X-contractions carried out
  uint8 ySteps = 0; //Number of y-contractions carried out
  //TODO - make this a sequential list for recursive filtering.
  //TODO - Also include a recursion limit value so that we don't waste filtering operations on pixels that are already accurate enough.
  Common::Rect Src = Common::Rect(0,0);  //Coordinates of four panorama image pixels around actual working window pixel
  
  uint8 fracX = 0;
  uint8 fracY = 0;
  
  bool _printDebug = false;
  
  FilterPixel() {};
  FilterPixel(float x, float y, bool highQuality=false, bool printDebug=false) {
    Src.left = int16(floor(x));
    Src.right = int16(ceil(x));
	  Src.top = int16(floor(y));
	  Src.bottom = int16(ceil(y));

	  _printDebug = printDebug;

    if(_printDebug)
      debug(1,"\tTarget pixel offset: %f, %f", x, y);

		//Bilinear
	  if(highQuality) {
      fracX = uint8((x-Src.left)*pFrac);  //Fraction of horizontal pixel position, 0 = left, pFrac = right
      fracY = uint8((y-Src.top)*pFrac); //Fraction of vertical pixel position, 0 = top, pFrac = bottom
	    uint xBound = pFrac >> 1;
	    uint yBound = pFrac >> 1;
      if(_printDebug) {
        debug(1,"\tBilinear, xBound: %d, yBound: %d, tol: %d", xBound, yBound, tol);
        debug(1,"\tByte fractions, fracX: %d, fracY: %d", fracX, fracY);
      }
	    for(uint8 i = (pFrac >> 2); i > tol; i >>= 1) {
        if(_printDebug)
          debug(1,"\txBound %d, yBound %d, i %d", xBound, yBound, i);
	      if(fracX >= xBound) {
  	      if(fracX-xBound > tol) {
    	      xDir += 0x80;
    	      xDir >>= 1;
    	      xBound += i;
    	      xSteps++;
            if(_printDebug)
              debug(1,"		Contract right, xDir: 0x%X, xSteps: %d", xDir, xSteps);
  	      }
	      }
	      else {
  	      if(xBound-fracX > tol) {
    	      xDir >>= 1;
    	      xBound -= i;
    	      xSteps++;
            if(_printDebug)
              debug(1,"		Contract left, xDir: 0x%X, xSteps: %d", xDir, xSteps);
  	      }
	      }
	      if(fracY >= yBound) {
  	      if(fracY-yBound > tol) {
    	      yDir += 0x80;
    	      yDir >>= 1;
    	      yBound += i;
    	      ySteps++;
            if(_printDebug)
              debug(1,"		Contract downward, yDir: 0x%X, ySteps: %d", yDir, ySteps);
  	      }  
	      }
	      else {
  	      if(yBound-fracY > tol) {
    	      yDir >>= 1;
    	      yBound -= i;
    	      ySteps++;
            if(_printDebug)
              debug(1,"		Contract upward, yDir: 0x%X, ySteps: %d", yDir, ySteps);
  	      }
	      }
	    }
	    xDir >>= (7-xSteps);
	    yDir >>= (7-ySteps);
      if(_printDebug)
		    debug(2,"Xdir 0x%X, Ydir 0x%X, xSteps %d, ySteps %d", xDir, yDir, xSteps, ySteps);
	  }
	  else {
    //Nearest neighbour
		xDir = (x-Src.left) > 0.5f ? 0x01 : 0x00;
		yDir = (y-Src.top) > 0.5f ? 0x01 : 0x00;
    if(_printDebug)
      debug(1,"\tNearest neighbour, xDir: 0x%X, yDir: 0x%X", xDir, yDir);
	  }
  };
  ~FilterPixel() {};
};

class RenderTable {
public:
	RenderTable(uint numRows, uint numColumns, const Graphics::PixelFormat pixelFormat);
	~RenderTable();
	
	Common::Point testPixel = Common::Point(30,120);

public:
	enum RenderState {
		PANORAMA,
		TILT,
		FLAT
	};

private:
	uint _numColumns, _numRows; //Working area width, height
  FilterPixel *_internalBuffer;
	RenderState _renderState;
	bool _highQuality = false;
	const uint8 filterPasses = 2;
	const Graphics::PixelFormat _pixelFormat;
	uint32 avgRB, avgG;
	uint8 _tol = 10;  //TODO - allow different optimised values for ZGI & Nemesis

	struct {
		float verticalFOV;  //Radians
		float linearScale;
		bool reverse;
		uint16 zeroPoint;
	} _panoramaOptions;

	// TODO: See if tilt and panorama need to have separate options
	struct {
		float verticalFOV;  //Radians
		float linearScale;
		bool reverse;
		float gap;
	} _tiltOptions;

public:
	RenderState getRenderState() {
		return _renderState;
	}
	void setRenderState(RenderState newState);

	const Common::Point convertWarpedCoordToFlatCoord(const Common::Point &point);  //input point in working area coordinates

//	void mutateImage(uint16 *sourceBuffer, uint16 *destBuffer, uint32 destWidth, const Common::Rect &subRect);
  void mutateImage(Graphics::Surface *dstBuf, Graphics::Surface *srcBuf, bool filter=false);
	
	template <typename I>
	Common::String pixelToBinary(I &pixel, bool splitColors=true) {
	  uint8 bits = sizeof(pixel) << 3;
    Common::String str("0b");
    I spaceMask = 0;
    for(uint8 i = 0; i < 3; i++)
      spaceMask = (spaceMask << 5) + 0x10;
	  for(I mask = 0x01 << (bits-1); mask; mask >>= 1) {
	    if(splitColors && (spaceMask & mask))
	      str += " ";
      str += mask & pixel ? "1" : "0";
    }
    return str;
	}
	
//Old version
/*/
	inline void contractLeft(uint32 &LeftPixel, uint32 &RightPixel) {
	  //NB Optimised & valid for RGB555 only; ALWAYS ROUNDS DOWN, WILL CAUSE CUMULATIVE ERRORS
  	avgG = ((LeftPixel & 0x03e0) + (RightPixel & 0x03e0)) & 0x07c0;
	  RightPixel = ((LeftPixel & 0x7c1f) + (RightPixel & 0x7c1f)) & 0xf83e;
    RightPixel = (RightPixel | avgG) >> 1;
	};
	
//New version
/*/
	inline void contractLeft(uint32 &LeftPixel, uint32 &RightPixel) {
	  RightPixel = LeftPixel + RightPixel;
	  LeftPixel <<= 1;
	};
//*/
	
	void generateRenderTable();

	void setPanoramaFoV(float fov); //Degrees
	void setPanoramaScale(float scale);
	void setPanoramaReverse(bool reverse);
	void setPanoramaZeroPoint(uint16 point);
	uint16 getPanoramaZeroPoint();
	bool getPanoramaReverse();

	void setTiltFoV(float fov); //Degrees
	void setTiltScale(float scale);
	void setTiltReverse(bool reverse);

	float getTiltGap();
	float getAngle();
	float getLinscale();

private:
	void generatePanoramaLookupTable();
//	Common::Point generatePanoramaLookupPoint();
	void generateTiltLookupTable();
};

} // End of namespace ZVision

#endif
