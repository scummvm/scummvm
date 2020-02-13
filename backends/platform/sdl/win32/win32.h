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

#ifndef PLATFORM_SDL_WIN32_H
#define PLATFORM_SDL_WIN32_H

#include "backends/platform/sdl/sdl.h"
#include "backends/platform/sdl/win32/win32-window.h"

class OSystem_Win32 : public OSystem_SDL {
public:
	virtual void init();
	virtual void initBackend();

	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

	virtual bool hasFeature(Feature f);

	virtual bool displayLogFile();

	virtual bool openUrl(const Common::String &url);

	virtual void logMessage(LogMessageType::Type type, const char *message);

	virtual Common::String getSystemLanguage() const;

	virtual Common::String getScreenshotsPath();

protected:
	virtual Common::String getDefaultConfigFileName();
	virtual Common::String getDefaultLogFileName();

	// Override createAudioCDManager() to get our Windows-specific
	// version.
	virtual AudioCDManager *createAudioCDManager();
	
	HWND getHwnd() { return ((SdlWindow_Win32*)_window)->getHwnd(); }

	virtual char *convertEncoding(const char *to, const char *from, const char *string, size_t length);
};

#endif
