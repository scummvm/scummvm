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

#include "common/system.h"
#include "common/textconsole.h"
#include "hopkins/sound.h"

namespace Hopkins {

void SoundManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
	SPECIAL_SOUND = 0;
}

void SoundManager::WSOUND_INIT() {
	warning("TODO: WSOUND_INIT");
}

void SoundManager::VERIF_SOUND() {
	warning("TODO: VERIF_SOUND");
}

void SoundManager::LOAD_ANM_SOUND() {
	warning("TODO: LOAD_ANIM_SOUND");
}

void SoundManager::PLAY_ANM_SOUND(int soundNumber) {
	warning("TODO: PLAAY_ANIM_SOUND");
}

void SoundManager::WSOUND(int soundNumber) {
	warning("TODO: WSOUND");
}

void SoundManager::VOICE_MIX(int a1, int a2) {
	warning("TODO: VOICE_MIX");
}

} // End of namespace Hopkins
