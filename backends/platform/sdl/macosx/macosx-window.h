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

#ifndef BACKENDS_PLATFORM_SDL_MACOSX_MACOSX_WINDOW_H
#define BACKENDS_PLATFORM_SDL_MACOSX_MACOSX_WINDOW_H

#ifdef MACOSX

#include "backends/platform/sdl/sdl-window.h"

class SdlWindow_MacOSX final : public SdlWindow {
public:
	// Use an iconless window on macOS, as we use a nicer external icon there.
	void setupIcon() override {}
	float getDpiScalingFactor() const override;
};

#endif

#endif
