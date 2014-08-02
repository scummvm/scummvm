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

#include "common/algorithm.h"
#include "access/access.h"
#include "access/sound.h"

namespace Access {

SoundManager::SoundManager(AccessEngine *vm) : _vm(vm) {
	Common::fill(&_soundTable[0], &_soundTable[MAX_SOUNDS], (byte *)nullptr);
	Common::fill(&_soundPriority[0], &_soundPriority[MAX_SOUNDS], 0);
}

SoundManager::~SoundManager() {
	for (int i = 0; i < MAX_SOUNDS; ++i)
		delete _soundTable[i];
}

byte *SoundManager::loadSound(int fileNum, int subfile) {
	return _vm->_files->loadFile(fileNum, subfile);
}


} // End of namespace Access
