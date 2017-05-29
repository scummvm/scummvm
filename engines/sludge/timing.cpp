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
#if 0
#include <SDL/SDL.h>
#endif

namespace Sludge {

int desiredfps = 300;               //holds desired frames per second

#if 0
Uint32 starttime, endtime;
Uint32 desired_frame_time;

void Init_Timer(void) {
	desired_frame_time = 1000 / desiredfps;
	starttime = SDL_GetTicks();
}

void Init_Special_Timer(int t) {
	desired_frame_time = 1000 / t;
	starttime = SDL_GetTicks();
}

void Wait_Frame(void) {
	static Uint32 addNextTime = 0;
	Uint32 timetaken;

	for (;;) {
		endtime = SDL_GetTicks();
		timetaken = addNextTime + endtime - starttime;
		if (timetaken >= desired_frame_time) break;
		SDL_Delay(1);
	}

	addNextTime = timetaken - desired_frame_time;
	if (addNextTime > desired_frame_time) addNextTime = desired_frame_time;

	starttime = endtime;
}
#endif

}
               // End of namespace Sludge
