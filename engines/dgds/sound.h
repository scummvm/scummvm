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

#ifndef DGDS_SOUND_H
#define DGDS_SOUND_H

#include "common/scummsys.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Dgds {

class ResourceManager;
class Decompressor;
class DgdsMidiPlayer;

struct Channel {
	Audio::AudioStream *stream;
	Audio::SoundHandle handle;
	byte volume;
};

class Sound {
public:
	Sound(Audio::Mixer *mixer, ResourceManager *resource, Decompressor *decompressor);
	~Sound();

	void playAmigaSfx(const Common::String &filename, byte channel, byte volume);
	void loadAmigaAiff(Common::SeekableReadStream &file);
	void loadMusic(Common::SeekableReadStream &file, Decompressor *decompressor);
	void playMusic(const Common::String &filename);

	void stopSfx(byte channel);

	bool playPCM(const byte *data, uint32 size);

private:
	struct Channel _channels[2];
	Common::SeekableReadStream *_soundData = nullptr;
	uint32 _musicSize = 0;
	byte *_musicData = nullptr;
	Audio::Mixer *_mixer;
	DgdsMidiPlayer *_midiPlayer;
	ResourceManager *_resource;
	Decompressor *_decompressor;
};

enum {
  DIGITAL_PCM   = 1 << 0,
  TRACK_ADLIB   = 1 << 1,
  TRACK_GM      = 1 << 2,
  TRACK_MT32    = 1 << 3
};

uint32 availableSndTracks(const byte *data, uint32 size);
byte loadSndTrack(uint32 track, const byte** trackPtr, uint16* trackSiz, const byte *data, uint32 size);

} // End of namespace Dgds

#endif // DGDS_SOUND_H
