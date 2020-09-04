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

#ifndef PLATFORM_SDL_SYMBIAN_H
#define PLATFORM_SDL_SYMBIAN_H

#include "backends/platform/sdl/sdl.h"

class RFs;

class OSystem_SDL_Symbian : public OSystem_SDL {
public:
	OSystem_SDL_Symbian();

	// Override from OSystem_SDL
	virtual void init();
	virtual void initBackend();
#ifdef GUI_ENABLE_KEYSDIALOG
	virtual void quit();
	virtual void engineInit();
	virtual void engineDone();
#endif
	virtual Common::String getDefaultConfigFileName();
	virtual bool hasFeature(Feature f);

	/**
	 * Returns reference to File session
	 */
	RFs& FsSession();

	void quitWithErrorMsg(const char *msg);

	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

protected:
#ifdef GUI_ENABLE_KEYSDIALOG
	/**
	 * Used to intialized special game mappings
	 */
	void checkMappings();
#endif

	RFs* _RFs;
};

#endif
