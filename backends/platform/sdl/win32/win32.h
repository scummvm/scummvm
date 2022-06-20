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

#ifndef PLATFORM_SDL_WIN32_H
#define PLATFORM_SDL_WIN32_H

#include "backends/platform/sdl/sdl.h"
#include "backends/platform/sdl/win32/win32-window.h"

class OSystem_Win32 final : public OSystem_SDL {
public:
	OSystem_Win32();

	void init() override;
	void initBackend() override;

#ifdef USE_OPENGL
	GraphicsManagerType getDefaultGraphicsManager() const override;
#endif

	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;

	bool hasFeature(Feature f) override;

	bool displayLogFile() override;

	bool openUrl(const Common::String &url) override;

	void logMessage(LogMessageType::Type type, const char *message) override;

	Common::String getSystemLanguage() const override;

	Common::String getDefaultIconPath() override;
	Common::String getScreenshotsPath() override;

protected:
	Common::String getDefaultConfigFileName() override;
	Common::String getDefaultLogFileName() override;

	// Override createAudioCDManager() to get our Windows-specific
	// version.
	AudioCDManager *createAudioCDManager() override;

	HWND getHwnd() { return ((SdlWindow_Win32*)_window)->getHwnd(); }

private:
	bool _isPortable;
	bool detectPortableConfigFile();
};

#endif
