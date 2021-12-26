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

#ifndef DS_BACKGROUND_H
#define DS_BACKGROUND_H

#include "graphics/surface.h"
#include "common/rect.h"

namespace DS {

class Background : public Graphics::Surface {
public:
	Background();

	size_t getRequiredVRAM(uint16 width, uint16 height, bool isRGB, bool swScale);
	void create(uint16 width, uint16 height, bool isRGB);
	void create(uint16 width, uint16 height, bool isRGB, int layer, bool isSub, int mapBase, bool swScale);
	void init(Background *surface);
	void init(Background *surface, int layer, bool isSub, int mapBase, bool swScale);

	void update();
	void reset();

	void show();
	void hide();
	inline bool isVisible() const { return _visible; }

	void setScalef(int32 sx, int32 sy);
	inline void setScale(int sx, int sy) { setScalef(sx << 8, sy << 8); }
	void setScrollf(int32 x, int32 y);
	inline void setScroll(int x, int y) { setScrollf(x << 8, y << 8); }

	Common::Point realToScaled(int16 x, int16 y);
	Common::Point scaledToReal(int16 x, int16 y);

	inline void clear() {
		memset(getPixels(), 0, pitch * h);
	}

	inline void grab(byte *dst, int dstPitch) {
		for (int y = 0; y < h; ++y) {
			memcpy(dst, getBasePtr(0, y), w * format.bytesPerPixel);
			dst += dstPitch;
		}
	}

protected:
	int _bg;
	bool _visible, _swScale;
	uint16 _realPitch, _realHeight;
	const Graphics::PixelFormat _pfCLUT8, _pfABGR1555;
	int32 _scrollX, _scrollY, _scaleX, _scaleY;
};

} // End of namespace DS

#endif // #ifndef DS_BACKGROUND_H
