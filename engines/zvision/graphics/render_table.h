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
  uint8 fracX = 0;  //Byte fraction of horizontal pixel position, 0 = left, 255 = right
  uint8 fracY = 0;  //Byte fraction of vertical pixel position, 0 = top, 255 = bottom
  //TODO - make this a sequential list for recursive filtering.  
  //TODO - Also include a recursion limit value so that we don't waste filtering operations on pixels that are already accurate enough.
  Common::Rect Src = Common::Rect(0,0);  //Coordinates of four panorama image pixels around actual working window pixel
  
  FilterPixel() {};
  FilterPixel(float x, float y, bool highQuality=false) {
		if(highQuality) {
	    Src.left = int16(floor(x));
	    Src.right = int16(ceil(x));
	    fracX = uint8((x-Src.left)*255);
		  Src.top = int16(floor(y));
		  Src.bottom = int16(ceil(y));
		  fracY = uint8((y-Src.top)*255);
	  }
	  else {
		  Src.left = int16(round(x));
  		Src.top = int16(round(y));
		}
  };
  ~FilterPixel() {};
};

class RenderTable {
public:
	RenderTable(uint numRows, uint numColumns, const Graphics::PixelFormat pixelFormat);
	~RenderTable();

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
	bool highQuality = true;
	const uint8 filterPasses = 2;
	const Graphics::PixelFormat _pixelFormat;
	uint16 avgL, avgH;

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
  void mutateImage(Graphics::Surface *dstBuf, Graphics::Surface *srcBuf);
	
	inline uint16 avgPixels(uint16 &PixelA, uint16 &PixelB) {
	  //NB Optimised & valid for RGB555 only!
  	avgL = (PixelA & 0x3def) + (PixelB & 0x3def);  //Add first 4 respective bits of eagh 5-bit R, G & B value
	  avgH = (PixelA & 0x4210) + (PixelB & 0x4210);  //Add 5th respective bit of each 5-bit R, G & B value
	  return (avgH + (avgL & 0x7bde)) >> 1; //Combine upper & lower bits, dropping 1st respective bit of each 5-bit R, G & B value, & then halve to get averages.
	};
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
