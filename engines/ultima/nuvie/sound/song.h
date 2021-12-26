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

#ifndef NUVIE_SOUND_SONG_H
#define NUVIE_SOUND_SONG_H

#include "ultima/nuvie/sound/sound.h"

namespace Ultima {
namespace Nuvie {

class Song : public Sound {
public:

	virtual bool Init(const char *filename) {
		return false;
	}
	bool Play(bool looping = false) override {
		return false;
	}
	bool Stop() override {
		return false;
	}
	bool SetVolume(uint8 volume) override {
		return false;
	}
	bool FadeOut(float seconds) override {
		return false;
	}

	void SetName(const char *name) {
		if (name) m_Filename = name;    // SB-X
	}
private:

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
