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
#include "zvision/zvision.h"

class OSystem;
namespace ZVision {

class FilterPixel {
public:
	// Bitfields representing sequential direction of contraction
	bool _xDir = false; // false left, true right
	bool _yDir = false; // false up, true down
	Common::Rect _src = Common::Rect(0, 0); // Coordinates of four panorama image pixels around actual working window pixel

	float _fX, _fY, _fTL, _fTR, _fBL, _fBR;
	
	FilterPixel() {}
	FilterPixel(float x, float y, bool highQuality = false) {
		_src.left = int16(floor(x));
		_src.right = int16(ceil(x));
		_src.top = int16(floor(y));
		_src.bottom = int16(ceil(y));
		if (highQuality) {
			_fX = x - (float)_src.left;
			_fY = y - (float)_src.top;
			_fTL = (1 - _fX) * (1 - _fY);
			_fTR = _fX * (1 - _fY);
			_fBL = (1 - _fX) * _fY;
			_fBR = _fX * _fY;
		} else {
			// Nearest neighbour
			_xDir = (x - _src.left) > 0.5f;
			_yDir = (y - _src.top) > 0.5f;
		}
	}
	~FilterPixel() {}
	inline void flipH() {
		_src.left = -_src.left;
		_src.right = -_src.right;
	}
	inline void flipV() {
		_src.top = -_src.top;
		_src.bottom = -_src.bottom;
	}
};

class RenderTable {
public:
	RenderTable(ZVision *engine, uint16 numRows, uint16 numColumns, const Graphics::PixelFormat &pixelFormat);
	~RenderTable();

// Common::Point testPixel = Common::Point(255,0);
public:
	enum RenderState {
		PANORAMA,
		TILT,
		FLAT
	};

private:
	ZVision *_engine;
	OSystem *_system;
	uint16 _numRows, _numColumns, _halfRows, _halfColumns; // Working area width, height; half width, half height, in whole pixels
	float _halfWidth, _halfHeight;  // Centre axis to midpoint of outermost pixel
	FilterPixel *_internalBuffer;
	RenderState _renderState;
	bool _highQuality = false;
	const Graphics::PixelFormat _pixelFormat;

	inline void splitColor(uint16 &color, uint32 &r, uint32 &g, uint32 &b) const {
		// NB Left & right shifting unnecessary for interpolating & recombining, so not bothering in order to save cycles
		r = color & 0x001f;
		g = color & 0x03e0;
		b = color & 0x7c00;
	}
	inline uint16 mergeColor(const uint32 &r, const uint32 &g, const uint32 &b) const {
		// NB Red uses the lowest bits in RGB555 and so doesn't need its fractional bits masked away after averaging
		return r | (g & 0x03e0) | (b & 0x7c00);
	}


	struct {
		float verticalFOV;  // Radians
		float linearScale;
		bool reverse;
		uint16 zeroPoint;
	} _panoramaOptions;

	struct {
		float verticalFOV;  // Radians
		float linearScale;
		bool reverse;
		float gap;
	} _tiltOptions;

public:
	RenderState getRenderState() {
		return _renderState;
	}
	void setRenderState(RenderState newState);

	const Common::Point convertWarpedCoordToFlatCoord(const Common::Point &point);  // input point in working area coordinates

// void mutateImage(uint16 *sourceBuffer, uint16 *destBuffer, uint32 destWidth, const Common::Rect &subRect);
	void mutateImage(Graphics::Surface *dstBuf, Graphics::Surface *srcBuf, bool filter = false);
	template <typename I>
	Common::String pixelToBinary(const I &pixel, bool splitColors = true) const {
		uint8 bits = sizeof(pixel) << 3;
		Common::String str("0b");
		I spaceMask = 0;
		for (uint8 i = 0; i < 3; i++)
			spaceMask = (spaceMask << 5) + 0x10;
		for (I mask = 0x01 << (bits - 1); mask; mask >>= 1) {
			if (splitColors && (spaceMask & mask))
				str += " ";
			str += mask & pixel ? "1" : "0";
		}
		return str;
	}
	void generateRenderTable();

	void setPanoramaFoV(float fov); // Degrees
	void setPanoramaScale(float scale);
	void setPanoramaReverse(bool reverse);
	void setPanoramaZeroPoint(uint16 point);
	uint16 getPanoramaZeroPoint();
	bool getPanoramaReverse();

	void setTiltFoV(float fov); // Degrees
	void setTiltScale(float scale);
	void setTiltReverse(bool reverse);

	float getTiltGap();
	float getAngle();
	float getLinscale();

private:
	void generateLookupTable(bool tilt = false);
	void generatePanoramaLookupTable();
	void generateTiltLookupTable();
};

} // End of namespace ZVision

#endif
