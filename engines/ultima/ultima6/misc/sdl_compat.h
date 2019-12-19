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

#include "graphics/managed_surface.h"

enum MouseButton { BUTTON_LEFT = 1, BUTTON_RIGHT = 2, BUTTON_MIDDLE = 3 };

extern uint32 SDL_GetTicks();
extern void SDL_FreeSurface(Graphics::ManagedSurface *&s);
extern void SDL_ShowCursor(bool show);
extern uint32 SDL_MapRGB(Graphics::PixelFormat &format, byte r, byte g, byte b);
extern int SDL_BlitSurface(Graphics::ManagedSurface *src, const Common::Rect *srcrect,
	Graphics::ManagedSurface *dst, Common::Rect *dstrect);
extern int SDL_FillRect(Graphics::ManagedSurface *surf, Common::Rect *rect, uint color);
extern void SDL_UpdateRect(Graphics::ManagedSurface *surf, int x, int y, int w, int h);
extern void SDL_UpdateRects(Graphics::ManagedSurface *surf, int count, Common::Rect *rects);
extern Graphics::ManagedSurface *SDL_LoadBMP(const char *filename);

#if 0
#define Common::KEYCODE_UNKNOWN Common::KEYCODE_LAST
#define Common::KeyCode SDLKey
#define SDL_Keymod SDLMod
#define Common::KeyState SDL_keysym
#define Common::KBD_GUI Common::KBD_META
#define SDL_JoystickNameForIndex SDL_JoystickName
#define SDL_Window void
#define SDL_Renderer void
#define SDL_Texture void

#define Common::KEYCODE_KP_0 Common::KEYCODE_KP0
#define Common::KEYCODE_KP_1 Common::KEYCODE_KP1
#define Common::KEYCODE_KP_2 Common::KEYCODE_KP2
#define Common::KEYCODE_KP_3 Common::KEYCODE_KP3
#define Common::KEYCODE_KP_4 Common::KEYCODE_KP4
#define Common::KEYCODE_KP_5 Common::KEYCODE_KP5
#define Common::KEYCODE_KP_6 Common::KEYCODE_KP6
#define Common::KEYCODE_KP_7 Common::KEYCODE_KP7
#define Common::KEYCODE_KP_8 Common::KEYCODE_KP8
#define Common::KEYCODE_KP_9 Common::KEYCODE_KP9

#define SDL_CreateThread(x, y, z) SDL_CreateThread(x, z)

#define SDL_TRUE SDL_SRCCOLORKEY
#else

#endif
#endif
