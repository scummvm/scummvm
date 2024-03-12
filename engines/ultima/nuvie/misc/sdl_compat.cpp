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

#include "ultima/nuvie/misc/sdl_compat.h"
#include "ultima/nuvie/core/events.h"
#include "common/system.h"
#include "common/events.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "graphics/managed_surface.h"
#include "image/bmp.h"
#include "ultima/nuvie/screen/screen.h"

namespace Ultima {
namespace Nuvie {

uint32 SDL_GetTicks() {
	return g_system->getMillis();
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

int SDL_FillRect(Graphics::ManagedSurface *surf, const Common::Rect *rect, uint color) {
	surf->fillRect(rect ? *rect : Common::Rect(0, 0, surf->w, surf->h), color);
	return 0;
}

Graphics::ManagedSurface *SDL_LoadBMP(const Common::Path &filename) {
	Common::File f;
	Image::BitmapDecoder decoder;

	if (!f.open(filename))
		error("Could not open file - %s", filename.toString().c_str());
	if (!decoder.loadStream(f))
		error("Could not load bitmap - %s", filename.toString().c_str());

	const Graphics::Surface *src = decoder.getSurface();
	Screen *const screen = Game::get_game()->get_screen();
	assert(screen);
	Graphics::ManagedSurface *const screenSurface = screen->get_sdl_surface();
	assert(screenSurface);
	Graphics::ManagedSurface *dest = new Graphics::ManagedSurface(src->w, src->h, screenSurface->format);
	dest->blitFrom(*src, decoder.getPalette());

	return dest;
}

} // End of namespace Nuvie
} // End of namespace Ultima
