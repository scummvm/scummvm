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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

#include "backends/modular-backend.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/mixer/sdl/sdl-mixer.h"

class OSystem_SDL : public ModularBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	/** Pre-initialize backend, it should be called after
	 *  instantiating the backend. Early needed managers
	 *  are created here.
	 */
	virtual void init();

	virtual void initBackend();

	virtual Common::HardwareKeySet *getHardwareKeySet();

	virtual void quit();
	virtual void deinit();

	virtual void setWindowCaption(const char *caption);

	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);
	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();

	virtual SdlGraphicsManager *getGraphicsManager();

	virtual bool pollEvent(Common::Event &event);

	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &td) const;

	virtual Audio::Mixer *getMixer();

protected:
	bool _inited;
	bool _initedSDL;
	SdlMixerManager *_mixerManager;

	virtual void initSDL();

	virtual void setupIcon();

	virtual const char *getConfigFileNameString();
	virtual Common::String getDefaultConfigFileName();
};

#endif
