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

class OSystem;

namespace ZVision {

class FilterPixel {
public:
  //Bitfields representing sequential direction of contraction
  bool xDir = 0; //0 left, 1 right
  bool yDir = 0; //0 up, 1 down
  Common::Rect Src = Common::Rect(0,0);  //Coordinates of four panorama image pixels around actual working window pixel
  
  float fX, fY, fTL, fTR, fBL, fBR;
  
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
    if(highQuality) {
      fX = x-(float)Src.left;
      fY = y-(float)Src.top;
      fTL = (1-fX)*(1-fY);
      fTR = fX*(1-fY);
      fBL = (1-fX)*fY;
      fBR = fX*fY;
      if(_printDebug)
        debug(1,"fX: %f, fY: %f, fTL:%f, fTR:%f, fBL:%f, fBR:%f", fX, fY, fTL, fTR, fBL, fBR);
    }
	  else {
      //Nearest neighbour
		  xDir = (x-Src.left) > 0.5f;
		  yDir = (y-Src.top) > 0.5f;
      if(_printDebug)
        debug(1,"\tNearest neighbour, xDir: 0x%X, yDir: 0x%X", xDir, yDir);
	  }
  };
  ~FilterPixel() {};
};

class RenderTable {
public:
	RenderTable(ZVision *engine, uint numRows, uint numColumns, const Graphics::PixelFormat pixelFormat);
	~RenderTable();
	
	Common::Point testPixel = Common::Point(30,120);

public:
	enum RenderState {
		PANORAMA,
		TILT,
		FLAT
	};

private:
	ZVision *_engine;
	OSystem *_system;
	uint _numColumns, _numRows; //Working area width, height
  FilterPixel *_internalBuffer;
	RenderState _renderState;
	bool _highQuality = false;
	const uint8 filterPasses = 2;
	const Graphics::PixelFormat _pixelFormat;
	
  inline void splitColor(uint16 &color, uint32 &r, uint32 &g, uint32 &b) {
    //NB Left & right shifting unnecessary for interpolating & recombining, so not bothering in order to save cycles
    r = color & 0x001f;
    g = color & 0x03e0;
    b = color & 0x7c00;
  };
  inline uint16 mergeColor(uint32 &r, uint32 &g, uint32 &b) const {
    return (r & 0x001f) | (g & 0x03e0) | (b & 0x7c00);
  };


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
