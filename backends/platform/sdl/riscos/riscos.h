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

#ifndef PLATFORM_SDL_RISCOS_H
#define PLATFORM_SDL_RISCOS_H

#include "backends/platform/sdl/sdl.h"

class OSystem_RISCOS : public OSystem_SDL {
public:
	virtual void init();
	virtual void initBackend();

	virtual bool hasFeature(Feature f);

	virtual bool openUrl(const Common::String &url);

	virtual void logMessage(LogMessageType::Type type, const char *message);

protected:
	virtual Common::String getDefaultConfigFileName();
	virtual Common::String getDefaultLogFileName();
};

#endif
