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

//Mix_HookMusicFinished

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/sound/adplug/emu_opl.h"
#include "ultima/nuvie/sound/adplug/u6m.h"
#include "ultima/nuvie/sound/song_adplug.h"
#include "ultima/nuvie/sound/sound_manager.h"

namespace Ultima {
namespace Nuvie {

SongAdPlug::SongAdPlug(Audio::Mixer *m, CEmuopl *o) {
	mixer = m;
	opl = o;
	samples_left = 0;
	stream = NULL;
}

SongAdPlug::~SongAdPlug() {
//delete player;
}

bool SongAdPlug::Init(const char *filename, uint16 song_num) {
	if (filename == NULL)
		return false;

	m_Filename = filename; // SB-X

	stream = new U6AdPlugDecoderStream(opl, string(filename), song_num);

	return true;
}

bool SongAdPlug::Play(bool looping) {
	if (stream) {
		mixer->playStream(Audio::Mixer::kMusicSoundType, &handle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}
	return true;
}

bool SongAdPlug::Stop() {

	mixer->stopHandle(handle);
	stream->rewind();
	return true;
}

bool SongAdPlug::SetVolume(uint8 volume) {
	mixer->setChannelVolume(handle, volume);
	return true;
}

/*
bool SongAdPlug::Pause() {
    if (!Mix_PlayingMusic()) return false;
    return true;
}

bool SongAdPlug::Resume() {
    if (Mix_PlayingMusic()) return false;
    return true;
}
*/

} // End of namespace Nuvie
} // End of namespace Ultima
