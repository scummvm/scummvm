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

#ifndef AGS_MUSIC_H
#define AGS_MUSIC_H

#include "audio/audiostream.h"
#include "audio/midiplayer.h"
#include "audio/mixer.h"
#include "common/array.h"

namespace AGS {

class Music : public Audio::MidiPlayer {
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	Common::Array<byte> _midiData;
	bool _isFirstTime;
protected:
	// Overload Audio::MidiPlayer method
	void sendToChannel(byte channel, uint32 b) override;
public:
	Music(Audio::Mixer *mixer);
	~Music() override;

	/**
	 * Play music
	 */
	void playMusic(Common::SeekableReadStream *midi, bool repeat = false);

	/**
	 * Seek within the MIDI
	 */
	void seek(size_t offset);
};

extern Music *g_music;

} // End of namespace AGS

#endif
