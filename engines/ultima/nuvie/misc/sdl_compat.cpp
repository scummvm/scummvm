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

#include "ultima/nuvie/misc/sdl_compat.h"
#include "ultima/nuvie/core/events.h"
#include "common/system.h"
#include "common/events.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "graphics/managed_surface.h"
#include "image/bmp.h"

namespace Ultima {
namespace Nuvie {

uint32 SDL_GetTicks() {
	return g_system->getMillis();
}

void SDL_FreeSurface(Graphics::ManagedSurface *&s) {
	delete s;
	s = nullptr;
}

void SDL_ShowCursor(bool show) {
	g_system->showMouse(show);
}

uint32 SDL_MapRGB(Graphics::PixelFormat &format, byte r, byte g, byte b) {
	return format.RGBToColor(r, g, b);
}

int SDL_BlitSurface(const Graphics::ManagedSurface *src, const Common::Rect *srcrect,
		Graphics::ManagedSurface *dst, Common::Rect *dstrect) {
	Common::Rect srcRect = srcrect ? *srcrect : Common::Rect(0, 0, src->w, src->h);
	Common::Point destPos = dstrect ? Common::Point(dstrect->left, dstrect->top) : Common::Point();

	dst->transBlitFrom(*src, srcRect, destPos, (uint)-1);

	if (dstrect) {
		dstrect->setWidth(srcRect.width());
		dstrect->setHeight(srcRect.height());
	}

	return 0;
}

int SDL_FillRect(Graphics::ManagedSurface *surf, Common::Rect *rect, uint color) {
	surf->fillRect(rect ? *rect : Common::Rect(0, 0, surf->w, surf->h), color);
	return 0;
}

void SDL_UpdateRect(Graphics::ManagedSurface *surf, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w, y + h);
	if (r.isEmpty())
		r = Common::Rect(0, 0, surf->w, surf->h);

	g_system->copyRectToScreen(surf->getPixels(), surf->pitch, r.left, r.top, r.width(), r.height());
}

void SDL_UpdateRects(Graphics::ManagedSurface *surf, int count, Common::Rect *rects) {
	while (count-- > 0)
		g_system->copyRectToScreen(surf->getPixels(), surf->pitch, rects->left, rects->top,
			rects->width(), rects->height());
}

Graphics::ManagedSurface *SDL_LoadBMP(const char *filename) {
	Common::File f;
	Image::BitmapDecoder decoder;
	
	if (!f.open(filename))
		error("Could not open file - %s", filename);
	if (!decoder.loadStream(f))
		error("Could not load bitmap - %s", filename);
	
	const Graphics::Surface *src = decoder.getSurface();
	Graphics::ManagedSurface *dest = new Graphics::ManagedSurface(src->w, src->h, src->format);
	dest->blitFrom(*src);

	return dest;
}

int SDL_SetColorKey(Graphics::ManagedSurface *surface, int flag, uint32 key) {
	if (flag)
		surface->setTransparentColor(key);
	else
		surface->clearTransparentColor();

	return 0;
}

int SDL_SetColors(Graphics::ManagedSurface *surface, const SDL_Color *colors, int firstcolor, int ncolors) {
	surface->setPalette(colors, firstcolor, ncolors);
	return 0;
}

int SDL_WaitEvent(Common::Event *event) {
	while (!Events::get()->pollEvent(*event))
		g_system->delayMillis(5);
	return 0;
}

int SDL_PollEvent(Common::Event *event) {
	return Events::get()->pollEvent(*event);
}

int SDL_LockSurface(Graphics::ManagedSurface *surface) {
	return 0;
}

int SDL_UnlockSurface(Graphics::ManagedSurface *surface) {
	return 0;
}

Graphics::ManagedSurface *SDL_ConvertSurface(Graphics::ManagedSurface *src,
		const Graphics::PixelFormat &fmt, uint32 flags) {
	Graphics::ManagedSurface *dest = new Graphics::ManagedSurface(src->w, src->h, fmt);
	dest->blitFrom(*src);

	return dest;
}

} // End of namespace Nuvie
} // End of namespace Ultima
