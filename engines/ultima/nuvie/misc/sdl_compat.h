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

#ifndef NUVIE_MISC_SDL_COMPAT_H
#define NUVIE_MISC_SDL_COMPAT_H

#include "common/events.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Nuvie {

#define SDL_TRUE 1
#define SDL_FALSE 0

#define SDL_SWSURFACE 0

typedef uint32 SDL_Color;
#define MAKE_COLOR(r, g, b) (((uint32)r) | (((uint32)g) << 8) | (((uint32)b) << 16) | (((uint32)0xff) << 24))

extern uint32 SDL_GetTicks();
extern void SDL_FreeSurface(Graphics::ManagedSurface *&s);
extern void SDL_ShowCursor(bool show);
extern uint32 SDL_MapRGB(Graphics::PixelFormat &format, byte r, byte g, byte b);
extern int SDL_BlitSurface(const Graphics::ManagedSurface *src, const Common::Rect *srcrect,
	Graphics::ManagedSurface *dst, Common::Rect *dstrect);
extern int SDL_FillRect(Graphics::ManagedSurface *surf, Common::Rect *rect, uint color);
extern void SDL_UpdateRect(Graphics::ManagedSurface *surf, int x, int y, int w, int h);
extern void SDL_UpdateRects(Graphics::ManagedSurface *surf, int count, Common::Rect *rects);
extern Graphics::ManagedSurface *SDL_LoadBMP(const char *filename);
extern int SDL_SetColorKey(Graphics::ManagedSurface *surface, int flag, uint32 key);
extern int SDL_SetColors(Graphics::ManagedSurface *surface, const SDL_Color *colors, int firstcolor, int ncolors);
extern int SDL_WaitEvent(Common::Event *event);
extern int SDL_PollEvent(Common::Event *event);
extern int SDL_LockSurface(Graphics::ManagedSurface *surface);
extern int SDL_UnlockSurface(Graphics::ManagedSurface *surface);
extern Graphics::ManagedSurface *SDL_ConvertSurface(Graphics::ManagedSurface *src,
	const Graphics::PixelFormat &fmt, uint32 flags);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
