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

#include "common/rect.h"

namespace Graphics {
struct Surface;
}

namespace DS {

class Background {
public:
	Background(Graphics::Surface *surface, int layer, bool isSub, int mapBase, bool swScale);

	static size_t getRequiredVRAM(uint16 width, uint16 height, bool isRGB, bool swScale);

	void update();
	void reset();

	void show();
	void hide();
	inline bool isVisible() const { return _visible; }

	void setScalef(int32 sx, int32 sy);
	inline void setScale(int sx, int sy) { setScalef(sx << 8, sy << 8); }
	void setScrollf(int32 x, int32 y);
	inline void setScroll(int x, int y) { setScrollf(x << 8, y << 8); }

	Common::Point realToScaled(int16 x, int16 y) const;
	Common::Point scaledToReal(int16 x, int16 y) const;

protected:
	int _bg;
	bool _visible, _swScale;
	uint16 _pitch;
	int32 _scrollX, _scrollY, _scaleX, _scaleY;
	Graphics::Surface *_surface;
};

class TiledBackground {
public:
	TiledBackground(const unsigned int *tiles, size_t tilesLen, const unsigned short *map, size_t mapLen, int layer, bool isSub, int mapBase, int tileBase);

	void update();
	void reset();

	void show();
	void hide();
	inline bool isVisible() const { return _visible; }

protected:
	const unsigned int *_tiles;
	const unsigned short *_map;
	size_t _tilesLen, _mapLen;

	int _bg;
	bool _visible;
};

} // End of namespace DS

#endif // #ifndef DS_BACKGROUND_H
