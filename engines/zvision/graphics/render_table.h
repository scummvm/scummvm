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
	RenderTable(uint numRows, uint numColumns);
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
	bool highQuality = false;
	const uint8 filterPasses = 2;

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

	void mutateImage(uint16 *sourceBuffer, uint16 *destBuffer, uint32 destWidth, const Common::Rect &subRect);
	void mutateImage(Graphics::Surface *dstBuf, Graphics::Surface *srcBuf);
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
