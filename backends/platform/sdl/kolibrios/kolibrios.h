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

#ifndef PLATFORM_SDL_KOLIBRIOS_H
#define PLATFORM_SDL_KOLIBRIOS_H

#include "backends/platform/sdl/sdl.h"

class OSystem_KolibriOS : public OSystem_SDL {
public:
	OSystem_KolibriOS(const char *exeName);

	void init() override;
	void initBackend() override;

	// Default paths
	Common::String getDefaultIconsPath() override;
	Common::String getScreenshotsPath() override;
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority) override;
	const Common::Path& getExePath() const { return _exePath; }

protected:
	Common::String getDefaultConfigFileName() override;
	Common::String getDefaultLogFileName() override;

	AudioCDManager *createAudioCDManager() override;

private:
	Common::Path _exePath;
	Common::Path _writablePath;
	Common::String _exeName;
};

#endif
