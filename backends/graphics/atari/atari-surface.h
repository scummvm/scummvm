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

#ifndef BACKENDS_GRAPHICS_ATARI_SURFACE_H
#define BACKENDS_GRAPHICS_ATARI_SURFACE_H

#include "graphics/managed_surface.h"

#include "backends/graphics/atari/atari-supervidel.h"

constexpr Graphics::PixelFormat PIXELFORMAT_CLUT8  = Graphics::PixelFormat::createFormatCLUT8();
constexpr Graphics::PixelFormat PIXELFORMAT_RGB332 = Graphics::PixelFormat(1, 3, 3, 2, 0, 5, 2, 0, 0);
constexpr Graphics::PixelFormat PIXELFORMAT_RGB121 = Graphics::PixelFormat(1, 1, 2, 1, 0, 3, 1, 0, 0);

class AtariSurface : public Graphics::ManagedSurface {
public:
	AtariSurface() = default;
	AtariSurface(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat);
	~AtariSurface() override;

	using Graphics::ManagedSurface::create;
	void create(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat) final override;
	void free() final override;

	void addDirtyRect(const Common::Rect &r) final override {};

	// no override as ManagedSurface::copyRectToSurface is not a virtual function!
	virtual void copyRectToSurface(const void *buffer, int srcPitch, int destX, int destY, int width, int height);
	virtual void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect &subRect) {
		assert(subRect.left % 16 == 0);
		assert(srcSurface.format == format);

		copyRectToSurface(
			srcSurface.getBasePtr(subRect.left, subRect.top), srcSurface.pitch,
			destX, destY,
			subRect.width(), subRect.height());
	}

	virtual void drawMaskedSprite(const Graphics::Surface &srcSurface, const Graphics::Surface &srcMask,
								  const Graphics::Surface &boundingSurface,
								  int destX, int destY,
								  const Common::Rect &subRect);

	int getBitsPerPixel() const {
		return format == PIXELFORMAT_RGB121 ? 4 : 8;
	}

	static Common::Rect alignRect(int x1, int y1, int x2, int y2) {
		// make non-virtual for performance reasons
		return g_hasSuperVidel
		   ? Common::Rect(x1, y1, x2, y2)
		   : Common::Rect(x1 & (-16), y1, (x2 + 15) & (-16), y2);
	}
	static Common::Rect alignRect(const Common::Rect &rect) {
		// make non-virtual for performance reasons
		return g_hasSuperVidel
		   ? rect
		   : Common::Rect(rect.left & (-16), rect.top, (rect.right + 15) & (-16), rect.bottom);
	}
};

#ifdef USE_SUPERVIDEL
class SuperVidelSurface final : public AtariSurface {
public:
	SuperVidelSurface() = default;
	SuperVidelSurface(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat)
		: AtariSurface(width, height, pixelFormat) {
	}

	//using Graphics::ManagedSurface::copyRectToSurface;
	void copyRectToSurface(const void *buffer, int srcPitch, int destX, int destY, int width, int height) override {
		Graphics::ManagedSurface::copyRectToSurface(buffer, srcPitch, destX, destY, width, height);
	}
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect &subRect) override {
		Graphics::ManagedSurface::copyRectToSurface(srcSurface, destX, destY, subRect);
	}

	void drawMaskedSprite(const Graphics::Surface &srcSurface, const Graphics::Surface &srcMask,
						  const Graphics::Surface &boundingSurface,
						  int destX, int destY,
						  const Common::Rect &subRect) override {
#ifdef USE_SV_BLITTER
		g_blitMask = (const byte *)srcMask.getBasePtr(subRect.left, subRect.top);
#endif
		copyRectToSurfaceWithKey(srcSurface, destX, destY, subRect, 0);
#ifdef USE_SV_BLITTER
		g_blitMask = nullptr;
#endif
	}
};
#endif

void AtariSurfaceInit();
void AtariSurfaceDeinit();

#endif // BACKENDS_GRAPHICS_ATARI_SURFACE_H
