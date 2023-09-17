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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/sound/adplug/emu_opl.h"
#include "ultima/nuvie/sound/adplug/u6m.h"
#include "ultima/nuvie/sound/song_filename.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/nuvie.h"

namespace Ultima {
namespace Nuvie {

SongFilename::~SongFilename() {
}

bool SongFilename::Init(const Common::Path &path, const char *fileId) {
	return Init(path, fileId, 0);
}

bool SongFilename::Init(const Common::Path &filename, const char *fileId, uint16 song_num) {
	if (filename.empty())
		return false;

	m_Filename = filename.toString(); // SB-X
	m_FileId = fileId;

	return true;
}

bool SongFilename::Play(bool looping) {
	return true;
}

bool SongFilename::Stop() {
	return true;
}

bool SongFilename::SetVolume(uint8 volume) {
	return true;
}

bool SongFilename::FadeOut(float seconds) {
	return false;
}

} // End of namespace Nuvie
} // End of namespace Ultima
