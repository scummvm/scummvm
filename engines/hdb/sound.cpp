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

#include "hdb/hdb.h"

namespace HDB {

bool Sound::init() {
	warning("STUB: Sound::init()");
	return true;
}

void Sound::save(Common::OutSaveFile *out) {
	warning("STUB: Sound::save()");
}

void Sound::loadSaveFile(Common::InSaveFile *in) {
	warning("STUB: Sound::loadSaveFile()");
}

void Sound::clearPersistent() {
	warning("STUB: Sound::clearPersistent()");
}

bool Sound::playSound(int index) {
	debug(9, "STUB: Play Sound");
	return true;
}

bool Sound::playSoundEx(int index, int channel, bool loop) {
	debug(9, "STUB: Play SoundEx");
	return true;
}

bool Sound::playVoice(int index, int actor) {
	warning("STUB: Play Voice");
	return true;
}

bool Sound::stopChannel(int channel) {
	debug(9, "STUB: Stop Channel");
	return true;
}

void Sound::stopMusic() {
	debug(9, "STUB: Stop Music");
}

void Sound::markSoundCacheFreeable() {
	warning("STUB: Sound::markSoundCacheFreeable() ");
}

} // End of Namespace
