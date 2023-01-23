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

#include "backends/events/sdl/sdl-events.h"

SDL_Joystick *SDL_JoystickOpen (int index) {
	return nullptr;
}

const char *SDL_JoystickName (int index) {
	return nullptr;
}

int SDL_NumJoysticks (void) {
	return 0;
}

void SDL_JoystickClose (SDL_Joystick *joystick) {
}

// SDL CDAudio is not used on kolibri as it's redirected away from
// in platform-specific code but since it's still linked-in it needs those stubs
CDstatus SDL_CDStatus (SDL_CD *cdrom) {
	return CD_ERROR;
}

int SDL_CDPlayTracks (SDL_CD *cdrom, int start_track, int start_frame, int ntracks, int nframes) {
	return -1;
}

int SDL_CDStop (SDL_CD *cdrom) {
	return -1;
}

SDL_CD *SDL_CDOpen (int drive) {
	return nullptr;
}

void SDL_CDClose (SDL_CD *cdrom) {
}
