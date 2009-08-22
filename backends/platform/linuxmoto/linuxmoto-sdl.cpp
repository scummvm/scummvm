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


#include "backends/platform/linuxmoto/linuxmoto-sdl.h"

void OSystem_LINUXMOTO::preprocessEvents(SDL_Event *event) {
	if (event->type == SDL_ACTIVEEVENT) {
		if (event->active.state == SDL_APPINPUTFOCUS && !event->active.gain) {
			suspendAudio(); 
			for (;;) {
				if (!SDL_WaitEvent(event)) {
					SDL_Delay(10);
					continue;
				}
				if (event->type == SDL_QUIT)
					return;
				if (event->type != SDL_ACTIVEEVENT)
					continue;
				if (event->active.state == SDL_APPINPUTFOCUS && event->active.gain) {
					resumeAudio();
						return;
				}
			}
		}
	}
}

void OSystem_LINUXMOTO::suspendAudio() {
	SDL_CloseAudio();
	_audioSuspended = true;
}

int OSystem_LINUXMOTO::resumeAudio() {
	if (!_audioSuspended)
		return -2;
	if (SDL_OpenAudio(&_obtainedRate, NULL) < 0){
		return -1;
	}
	SDL_PauseAudio(0);
	_audioSuspended = false;
	return 0;
} 

void OSystem_LINUXMOTO::setupMixer() {
	OSystem_SDL::setupMixer();
}
