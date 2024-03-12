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

#ifndef NUVIE_MISC_SDL_COMPAT_H
#define NUVIE_MISC_SDL_COMPAT_H

#include "common/events.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Nuvie {

#define SDL_TRUE 1

#define SDL_SWSURFACE 0

extern uint32 SDL_GetTicks();
extern int SDL_BlitSurface(const Graphics::ManagedSurface *src, const Common::Rect *srcrect,
	Graphics::ManagedSurface *dst, Common::Rect *dstrect);
extern int SDL_FillRect(Graphics::ManagedSurface *surf, const Common::Rect *rect, uint color);
extern Graphics::ManagedSurface *SDL_LoadBMP(const Common::Path &filename);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
