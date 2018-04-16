/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NEVERHOOD_SCREEN_H
#define NEVERHOOD_SCREEN_H

#include "common/array.h"
#include "graphics/surface.h"
#include "neverhood/neverhood.h"
#include "neverhood/microtiles.h"
#include "neverhood/graphics.h"

namespace Video {
	class SmackerDecoder;
}

namespace Neverhood {

struct RenderItem {
	const Graphics::Surface *_surface;
	const Graphics::Surface *_shadowSurface;
	int16 _destX, _destY;
	int16 _srcX, _srcY, _width, _height;
	bool _transparent;
	byte _version;
	bool _refresh;
	bool operator==(const RenderItem &second) const {
		return
			_surface == second._surface &&
			_shadowSurface == second._shadowSurface &&
			_destX == second._destX &&
			_destY == second._destY &&
			_srcX == second._srcX &&
			_srcY == second._srcY &&
			_width == second._width &&
			_height == second._height &&
			_transparent == second._transparent &&
			_version == second._version;
	}
};

typedef Common::Array<RenderItem> RenderQueue;

class Screen {
public:
	Screen(NeverhoodEngine *vm);
	~Screen();
	void update();
	uint32 getNextFrameTime();
	void saveParams();
	void restoreParams();
	void setFps(int fps);
	int getFps();
	void setYOffset(int16 yOffset);
	int16 getYOffset();
	void setPaletteData(byte *paletteData);
	void unsetPaletteData(byte *paletteData);
	byte *getPaletteData() { return _paletteData; }
	void testPalette(byte *paletteData);
	void updatePalette();
	void clear();
	void clearRenderQueue();
	void drawSurface2(const Graphics::Surface *surface, NDrawRect &drawRect, NRect &clipRect, bool transparent, byte version,
		const Graphics::Surface *shadowSurface = NULL);
	void drawSurface3(const Graphics::Surface *surface, int16 x, int16 y, NDrawRect &drawRect, NRect &clipRect, bool transparent, byte version);
	void drawDoubleSurface2(const Graphics::Surface *surface, NDrawRect &drawRect);
	void drawUnk(const Graphics::Surface *surface, NDrawRect &drawRect, NDrawRect &sysRect, NRect &clipRect, bool transparent, byte version);
	void drawSurfaceClipRects(const Graphics::Surface *surface, NDrawRect &drawRect, NRect *clipRects, uint clipRectsCount, bool transparent, byte version);
	void setSmackerDecoder(Video::SmackerDecoder *smackerDecoder) { _smackerDecoder = smackerDecoder; }
	void queueBlit(const Graphics::Surface *surface, int16 destX, int16 destY, NRect &ddRect, bool transparent, byte version,
		const Graphics::Surface *shadowSurface = NULL);
	void blitRenderItem(const RenderItem &renderItem, const Common::Rect &clipRect);
protected:
	NeverhoodEngine *_vm;
	MicroTileArray *_microTiles;
	Graphics::Surface *_backScreen;
	Video::SmackerDecoder *_smackerDecoder, *_savedSmackerDecoder;
	int32 _ticks;
	int32 _frameDelay, _savedFrameDelay;
	byte *_paletteData;
	bool _paletteChanged;
	int16 _yOffset, _savedYOffset;
	bool _fullRefresh;
	RenderQueue *_renderQueue, *_prevRenderQueue;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SCREEN_H */
