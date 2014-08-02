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
 */

#ifndef ACCESS_SOUND_H
#define ACCESS_SOUND_H

#include "common/scummsys.h"

#define MAX_SOUNDS 20

namespace Access {

class AccessEngine;

class SoundManager {
private:
	AccessEngine *_vm;
public:
	byte *_soundTable[MAX_SOUNDS];
	int _soundPriority[MAX_SOUNDS];
public:
	SoundManager(AccessEngine *vm);
	~SoundManager();

	byte *loadSound(int fileNum, int subfile);
};

} // End of namespace Access

#endif /* ACCESS_SOUND_H*/
