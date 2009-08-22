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


#ifndef LINUXMOTO_SDL
#define LINUXMOTO_SDL

#include "backends/platform/sdl/sdl.h"

#include <SDL.h>

class OSystem_LINUXMOTO : public OSystem_SDL {
private:
	bool _audioSuspended;
public:
	virtual bool remapKey(SDL_Event &ev, Common::Event &event);
	virtual void preprocessEvents(SDL_Event *event);
	virtual void setupMixer();
	virtual Common::HardwareKeySet *getHardwareKeySet();
	void suspendAudio();
	int resumeAudio();
};

#endif
