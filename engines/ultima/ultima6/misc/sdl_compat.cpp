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

#ifndef ULTIMA6_MISC_SDL_COMPAT_H
#define ULTIMA6_MISC_SDL_COMPAT_H

#include "common/system.h"
#include "graphics/managed_surface.h"

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

int SDL_BlitSurface(Graphics::ManagedSurface *src, const Common::Rect *srcrect,
		Graphics::ManagedSurface *dst, Common::Rect *dstrect) {
	dst->transBlitFrom(*src, *srcrect, *dstrect, (uint)-1);
	return 0;
}

int SDL_FillRect(Graphics::ManagedSurface *surf, Common::Rect *rect, uint color) {
	surf->fillRect(rect ? *rect : Common::Rect(0, 0, surf->w, surf->h), color);
	return 0;
}


#if 0
#define SDLK_UNKNOWN SDLK_LAST
#define Common::KeyCode SDLKey
#define SDL_Keymod SDLMod
#define Common::KeyState SDL_keysym
#define KMOD_GUI KMOD_META
#define SDL_JoystickNameForIndex SDL_JoystickName
#define SDL_Window void
#define SDL_Renderer void
#define SDL_Texture void

#define SDLK_KP_0 SDLK_KP0
#define SDLK_KP_1 SDLK_KP1
#define SDLK_KP_2 SDLK_KP2
#define SDLK_KP_3 SDLK_KP3
#define SDLK_KP_4 SDLK_KP4
#define SDLK_KP_5 SDLK_KP5
#define SDLK_KP_6 SDLK_KP6
#define SDLK_KP_7 SDLK_KP7
#define SDLK_KP_8 SDLK_KP8
#define SDLK_KP_9 SDLK_KP9

#define SDL_CreateThread(x, y, z) SDL_CreateThread(x, z)

#define SDL_TRUE SDL_SRCCOLORKEY
#else

#endif
#endif
