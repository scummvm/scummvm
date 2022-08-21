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

#ifndef NUVIE_SOUND_SOUND_H
#define NUVIE_SOUND_SOUND_H

#include "ultima/nuvie/core/game.h"
#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Nuvie {

using Std::string;
using Std::list;
using Std::vector;

class Sound {
public:
	virtual ~Sound() {};
	virtual bool Play(bool looping = false) = 0;
	virtual bool Stop() = 0;
	virtual bool FadeOut(float seconds) = 0;
	virtual bool SetVolume(uint8 volume) = 0; //range 0..255
	string GetName() {
		return m_Filename;
	}
protected:
	string m_Filename;
//	static SoundManager *gpSM;
};

class SoundCollection {
public:
	Sound *Select() {
		int i = NUVIE_RAND()  % m_Sounds.size();
		return m_Sounds[i];
	}; //randomly select one from the list
	vector<Sound *> m_Sounds;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
