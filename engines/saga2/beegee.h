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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_BEEGEE_H
#define SAGA2_BEEGEE_H

namespace Saga2 {

struct audioEnvironment {
private:
	static uint8 musicPlaying;
	static uint8 loopPlaying;
	static uint8 soundPlaying;
	static uint32 lastSoundAt;

public:
	uint8   musicID;
	uint8   loopID;
	uint8   sounds;
	uint8   sound[4];
	uint32  maxRandomSoundTime;
	uint32  minRandomSoundTime;

	audioEnvironment() {
		musicID = 0;
		loopID = 0;
		sounds = 0;
		sound[0] = 0;
		sound[1] = 0;
		sound[2] = 0;
		sound[3] = 0;
		minRandomSoundTime = 0;
		maxRandomSoundTime = 0;
	}

	void activate(void);
	void check(uint32 deltaT);

private:
	void playMusic(uint8 musicID);
	void playLoop(uint8 loopID);
	void playIntermittent(int soundNo);
};

} // end of namespace Saga2

#endif
