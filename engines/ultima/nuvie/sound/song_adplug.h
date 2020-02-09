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

#ifndef NUVIE_SOUND_SONG_ADPLUG_H
#define NUVIE_SOUND_SONG_ADPLUG_H

#include "audio/mixer.h"
#include "ultima/nuvie/sound/song.h"
#include "ultima/nuvie/sound/decoder/u6_adplug_decoder_stream.h"

namespace Ultima {
namespace Nuvie {

class CEmuopl;

class SongAdPlug : public Song {
public:
	uint16 samples_left;

	SongAdPlug(Audio::Mixer *m, CEmuopl *o);
	~SongAdPlug() override;
	bool Init(const char *filename) override {
		return Init(filename, 0);
	}
	bool Init(const char *filename, uint16 song_num);
	bool Play(bool looping = false) override;
	bool Stop() override;
	bool SetVolume(uint8 volume) override;
	bool FadeOut(float seconds) override {
		return false;
	}
	CEmuopl *get_opl() {
		return opl;
	};

private:
	Audio::Mixer *mixer;
	CEmuopl *opl;
	U6AdPlugDecoderStream *stream;
	Audio::SoundHandle handle;


};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
