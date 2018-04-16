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

#ifndef MADS_AUDIO_H
#define MADS_AUDIO_H

#include "mads/resources.h"

#include "common/array.h"
#include "audio/mixer.h"

namespace MADS {

struct DSREntry {
	int16 frequency;
	int channels;
	int32 compSize;
	int32 uncompSize;
	int32 offset;
};

class AudioPlayer {
public:
	AudioPlayer(Audio::Mixer *mixer, uint32 gameID);
	~AudioPlayer();

	void setSoundGroup(const Common::String &filename);
	void setDefaultSoundGroup();
	void playSound(int soundIndex, bool loop = false);
	void stop();
	void setVolume(int volume);
	bool isPlaying() const;

 private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	uint32 _gameID;

	File _dsrFile;
	Common::String _filename;
	Common::Array<DSREntry> _dsrEntries;
};

} // End of namespace MADS

#endif
