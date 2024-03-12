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

#ifndef NUVIE_SOUND_SONG_FILENAME_H
#define NUVIE_SOUND_SONG_FILENAME_H

#include "ultima/nuvie/sound/song.h"

namespace Ultima {
namespace Nuvie {

class SongFilename : public Song {
public:
	~SongFilename() override;
	bool Init(const Common::Path &path, const char *fileId) override;
	bool Init(const Common::Path &path, const char *fileId, uint16 song_num) override;
	bool Play(bool looping = false) override;
	bool Stop() override;
	bool SetVolume(uint8 volume) override;
	bool FadeOut(float seconds) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
