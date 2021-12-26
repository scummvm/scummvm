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

#ifndef PLATFORM_SDL_ANDROIDSDL_H
#define PLATFORM_SDL_ANDROIDSDL_H

#include "backends/platform/sdl/posix/posix.h"

class OSystem_ANDROIDSDL : public OSystem_POSIX {
public:
	virtual void initBackend();
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual bool hasFeature(Feature f);
	void touchpadMode(bool enable);
	void swapMenuAndBackButtons(bool enable);
	void switchToDirectMouseMode();
	void switchToRelativeMouseMode();
	void showOnScreenControl(bool enable);
};

#endif
