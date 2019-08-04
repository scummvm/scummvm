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

#ifndef PLATFORM_SDL_POSIX_H
#define PLATFORM_SDL_POSIX_H

#include "backends/platform/sdl/sdl.h"

class OSystem_POSIX : public OSystem_SDL {
public:
	// Let the subclasses be able to change _baseConfigName in the constructor
	OSystem_POSIX(Common::String baseConfigName = "scummvm.ini");
	virtual ~OSystem_POSIX() {}

	virtual bool hasFeature(Feature f);

	virtual bool displayLogFile();

	virtual bool openUrl(const Common::String &url);

	virtual void init();
	virtual void initBackend();

	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

	Common::String getScreenshotsPath() override;

protected:
	/**
	 * Base string for creating the default path and filename for the
	 * configuration file. This allows the Mac OS X subclass to override
	 * the config file path and name.
	 */
	Common::String _baseConfigName;

	virtual Common::String getDefaultConfigFileName();
	virtual Common::String getDefaultLogFileName();

	Common::String getXdgUserDir(const char *name);

	virtual AudioCDManager *createAudioCDManager();

	bool launchBrowser(const Common::String& client, const Common::String &url);
};

#endif
