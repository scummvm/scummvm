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

#ifndef PLATFORM_SDL_SYMBIAN_H
#define PLATFORM_SDL_SYMBIAN_H

#include "backends/platform/sdl/sdl.h"

class RFs;

class OSystem_SDL_Symbian : public OSystem_SDL {
public:
	OSystem_SDL_Symbian();

	// Override from OSystem_SDL
	void init() override;
	void initBackend() override;
	Common::String getDefaultConfigFileName() override;
	bool hasFeature(Feature f) override;

	void quitWithErrorMsg(const char *msg);

	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

	Common::KeymapperDefaultBindings *getKeymapperDefaultBindings() override;

	bool openUrl(const Common::String &url) override;
protected:
	TFileName _localpath;
};

RFs &FsSession();
	
#endif
