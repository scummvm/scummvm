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

// NOT USED AT THE MOMENT
#ifndef ULTIMA6_SOUND_SAMPLE_H
#define ULTIMA6_SOUND_SAMPLE_H

#include "ultima/ultima6/sound/sound.h"

namespace Ultima {
namespace Ultima6 {

class Sample: public Sound {
public:
	Sample();
	~Sample();
	bool Init(const char *filename);
	bool Play(bool looping = false);
	bool Stop();
	bool FadeOut(float seconds);
	bool SetVolume(uint8 volume); //range 0..1
private:
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
