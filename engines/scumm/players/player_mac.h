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

#ifndef SCUMM_PLAYERS_PLAYER_MAC_H
#define SCUMM_PLAYERS_PLAYER_MAC_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/mutex.h"
#include "scumm/music.h"
#include "scumm/saveload.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

#define RES_SND MKTAG('s', 'n', 'd', ' ')

class Mixer;

namespace Scumm {

class ScummEngine;

/**
 * Scumm Macintosh music driver, base class.
 */
class Player_Mac : public Audio::AudioStream, public MusicEngine {
public:
	Player_Mac(ScummEngine *scumm, Audio::Mixer *mixer, int numberOfChannels, int channelMask, bool fadeNoteEnds);
	virtual ~Player_Mac();

	void init();

	// MusicEngine API
	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer();
	virtual int  getSoundStatus(int sound) const;

	// AudioStream API
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }

	virtual void saveLoadWithSerializer(Serializer *ser);

private:
	Common::Mutex _mutex;
	Audio::Mixer *const _mixer;
	Audio::SoundHandle _soundHandle;
	uint32 _sampleRate;
	int _soundPlaying;

	void stopAllSounds_Internal();

	struct Instrument {
		byte *_data;
		uint32 _size;
		uint32 _rate;
		uint32 _loopStart;
		uint32 _loopEnd;
		byte _baseFreq;

		uint _pos;
		uint _subPos;

		void newNote() {
			_pos = 0;
			_subPos = 0;
		}

		void generateSamples(int16 *data, int pitchModifier, int volume, int numSamples, int remainingSamplesOnNote, bool fadeNoteEnds);
	};

	int _pitchTable[128];
	int _numberOfChannels;
	int _channelMask;
	bool _fadeNoteEnds;

	virtual bool checkMusicAvailable() { return false; }
	virtual bool loadMusic(const byte *ptr) { return false; }
	virtual bool getNextNote(int ch, uint32 &samples, int &pitchModifier, byte &velocity) { return false; }

protected:
	struct Channel {
		virtual ~Channel() {}

		Instrument _instrument;
		bool _looped;
		uint32 _length;
		const byte *_data;

		uint _pos;
		int _pitchModifier;
		byte _velocity;
		uint32 _remaining;

		bool _notesLeft;

		bool loadInstrument(Common::SeekableReadStream *stream);
 	};

	ScummEngine *const _vm;
	Channel *_channel;

	uint32 durationToSamples(uint16 duration);
	int noteToPitchModifier(byte note, Instrument *instrument);
};

} // End of namespace Scumm

#endif
