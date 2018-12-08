/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/sound.h"

namespace Glk {

Sounds::~Sounds() {
	for (int idx = (int)_sounds.size() - 1; idx >= 0; --idx)
		delete _sounds[idx];
}

void Sounds::removeSound(schanid_t snd) {
	for (uint idx = 0; idx < _sounds.size(); ++idx) {
		if (_sounds[idx] == snd) {
			_sounds.remove_at(idx);
			break;
		}
	}
}

schanid_t Sounds::create(glui32 rock) {
	schanid_t snd = new SoundChannel();
	_sounds.push_back(snd);
	return snd;
}

schanid_t Sounds::iterate(schanid_t chan, glui32 *rockptr) {
	for (int idx = 0; idx < (int)_sounds.size() - 1; ++idx) {
		if (_sounds[idx] == chan) {
			schanid_t next = _sounds[idx + 1];
			if (*rockptr)
				*rockptr = next->_rock;

			return next;
		}
	}

	return nullptr;
}

/*--------------------------------------------------------------------------*/

SoundChannel::~SoundChannel() {
	_owner->removeSound(this);
	delete _stream;
}

void SoundChannel::play(uint soundNum) {
	// TODO
}

} // End of namespace Glk
