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

#ifndef ULTIMA6_SOUND_SOUND_H
#define ULTIMA6_SOUND_SOUND_H

//disable some annoying MSC warnings
#if defined(_MSC_VER)
#pragma warning( disable : 4503 ) // warning: decorated name length exceeded
#if _MSC_VER <= 1200
#pragma warning( disable : 4786 ) // ident trunc to '255' chars in debug info
#endif
#endif

#include "SDL.h"

#include "ultima/shared/std/string.h"
#include <stdlib.h>
#include <list>
#include "ultima/shared/std/containers.h"
#include <map>
#include <iterator>

namespace Ultima {
namespace Ultima6 {

using std::string;
using std::list;
using std::iterator;
using std::map;
using std::vector;

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

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
