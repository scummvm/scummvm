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

#ifndef ULTIMA4_SOUND_P_H
#define ULTIMA4_SOUND_P_H

#include "ultima/ultima4/sound/sound.h"
#include "ultima/shared/std/containers.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

struct Mix_Chunk;
typedef Mix_Chunk OSSoundChunk;

class SoundManager {
public:
	~SoundManager();
	static SoundManager *getInstance();
	int init(void);
	void play(Sound sound, bool onlyOnce = true, int specificDurationInTicks = -1);
	void stop(int channel = 1);
private:
	bool load(Sound sound);
	int init_sys();
	void del()      {
		del_sys();
	}
	void del_sys();
	void play_sys(Sound sound, bool onlyOnce, int specificDurationInTicks);
	bool load_sys(Sound sound, const Common::String &soundPathName);
	void stop_sys(int channel);
	Std::vector<Common::String> _soundFilenames;
	Std::vector<OSSoundChunk *> _soundChunk;
	SoundManager();
	static SoundManager *_instance;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
