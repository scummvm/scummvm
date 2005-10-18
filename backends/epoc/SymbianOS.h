/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 */

#ifndef SDLSYMBIANH
#define SDLSYMBIANH

#include "sdl-common.h"

#define TOTAL_ZONES 3

class OSystem_SDL_Symbian : public OSystem_SDL {
public:
	OSystem_SDL_Symbian();
	~OSystem_SDL_Symbian();
	int getDefaultGraphicsMode() const;
	const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	bool setGraphicsMode(const char *name);

	// Set function that generates samples
	/**
	 * This function is overridden by the symbian port in order to provide MONO audio
	 * downmix is done by supplying our own audiocallback
	 */
	virtual bool setSoundCallback(SoundProc proc, void *param); // overloaded by CE backend
protected:
	/**
	 * The mixer callback function, passed on to OSystem::setSoundCallback().
	 * This simply calls the mix() method.
	 * and then does downmixing for symbian if needed
	 */
	static void symbianMixCallback(void *s, byte *samples, int len);

	/**
	 * Actual mixing implementation
	 */
	void symbianMix(byte *samples, int len);

	/**
	 * This is an implementation by the remapKey function
	 * @param SDL_Event to remap
	 * @param ScumVM event to modify if special result is requested
	 * @return true if Event has a valid return status
	 */
	bool remapKey(SDL_Event &ev,Event &event);

	void setWindowCaption(const char *caption);

	/**
	 * Used to intialized special game mappings
	 */
	void check_mappings();

	void initZones();

	// Audio
	int _channels;

	SoundProc _sound_proc;
	void *_sound_proc_param;
	byte* _stereo_mix_buffer;

	// Used to handle joystick navi zones
	int _mouseXZone[TOTAL_ZONES];
	int _mouseYZone[TOTAL_ZONES];
	int _currentZone;

	typedef struct zoneDesc {
		int x;
		int y;
		int width;
		int height;
	} zoneDesc;

	static zoneDesc _zones[TOTAL_ZONES];
};

#endif
