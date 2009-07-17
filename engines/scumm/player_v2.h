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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCUMM_PLAYER_V2_H
#define SCUMM_PLAYER_V2_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "scumm/music.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

class ScummEngine;

#include "common/pack-start.h"	// START STRUCT PACKING

struct channel_data {
	uint16 time_left;          // 00
	uint16 next_cmd;           // 02
	uint16 base_freq;          // 04
	uint16 freq_delta;         // 06
	uint16 freq;               // 08
	uint16 volume;             // 10
	uint16 volume_delta;       // 12
	uint16 tempo;              // 14
	uint16 inter_note_pause;   // 16
	uint16 transpose;          // 18
	uint16 note_length;        // 20
	uint16 hull_curve;         // 22
	uint16 hull_offset;        // 24
	uint16 hull_counter;       // 26
	uint16 freqmod_table;      // 28
	uint16 freqmod_offset;     // 30
	uint16 freqmod_incr;       // 32
	uint16 freqmod_multiplier; // 34
	uint16 freqmod_modulo;     // 36
	uint16 unknown[4];         // 38 - 44
	uint16 music_timer;        // 46
	uint16 music_script_nr;    // 48
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


/**
 * Scumm V2 PC-Speaker MIDI driver.
 * This simulates the pc speaker sound, which is driven  by the 8253 (square
 * wave generator) and a low-band filter.
 */
class Player_V2 : public Audio::AudioStream, public MusicEngine {
public:
	Player_V2(ScummEngine *scumm, Audio::Mixer *mixer, bool pcjr);
	virtual ~Player_V2();

	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer() const;
	virtual int  getSoundStatus(int sound) const;

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		do_mix(buffer, numSamples / 2);
		return numSamples;
	}
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sample_rate; }

protected:
	bool _isV3Game;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	ScummEngine *_vm;

	bool _pcjr;
	int _header_len;

	uint32 _sample_rate;
	uint32 _next_tick;
	uint32 _tick_len;
	unsigned int _update_step;
	unsigned int _decay;
	int _level;
	unsigned int _RNG;
	unsigned int _volumetable[16];

	int _timer_count[4];
	int _timer_output;

	int   _current_nr;
	byte *_current_data;
	int   _next_nr;
	byte *_next_data;
	byte *_retaddr;

private:
	union ChannelInfo {
		channel_data d;
		uint16 array[sizeof(channel_data)/2];
	};

	int _music_timer;
	int _music_timer_ctr;
	int _ticks_per_music_timer;

	const uint16 *_freqs_table;

	Common::Mutex _mutex;
	ChannelInfo _channels[5];

protected:
	void mutex_up();
	void mutex_down();

	virtual void nextTick();
	virtual void clear_channel(int i);
	virtual void chainSound(int nr, byte *data);
	virtual void chainNextSound();

	virtual void generateSpkSamples(int16 *data, uint len);
	virtual void generatePCjrSamples(int16 *data, uint len);

	void lowPassFilter(int16 *data, uint len);
	void squareGenerator(int channel, int freq, int vol,
						int noiseFeedback, int16 *sample, uint len);

private:
	void do_mix(int16 *buf, uint len);

	void set_pcjr(bool pcjr);
	void execute_cmd(ChannelInfo *channel);
	void next_freqs(ChannelInfo *channel);
};

/**
 * Scumm V2 CMS/Gameblaster MIDI driver.
 */
class Player_V2CMS : public Audio::AudioStream, public MusicEngine {
public:
	Player_V2CMS(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_V2CMS();

	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer() const;
	virtual int  getSoundStatus(int sound) const;

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sample_rate; }
	
protected:

#include "common/pack-start.h"	// START STRUCT PACKING
	struct Voice {
		byte attack;
		byte decay;
		byte sustain;
		byte release;
		byte octadd;
		int16 vibrato;
		int16 vibrato2;
		int16 noise;
	} PACKED_STRUCT;
	
	struct Voice2 {
		byte *amplitudeOutput;
		byte *freqOutput;
		byte *octaveOutput;
		
		uint8 channel;
		int8 sustainLevel;
		int8 attackRate;
		uint8 maxAmpl;
		int8 decayRate;
		int8 sustainRate;
		int8 releaseRate;
		int8 releaseTime;
		int8 vibratoRate;
		int8 vibratoDepth;
		
		int8 curVibratoRate;
		int8 curVibratoUnk;

		int8 unkVibratoRate;
		int8 unkVibratoDepth;

		int8 unkRate;
		int8 unkCount;

		int nextProcessState;
		int8 curVolume;
		int8 curOctave;
		int8 curFreq;

		int8 octaveAdd;

		int8 playingNote;
		Voice2 *nextVoice;

		byte chanNumber;
	} PACKED_STRUCT;
	
	struct MusicChip {
		byte ampl[4];
		byte freq[4];
		byte octave[2];
	} PACKED_STRUCT;
#include "common/pack-end.h"	// END STRUCT PACKING

	Voice _cmsVoicesBase[16];
	Voice2 _cmsVoices[8];
	MusicChip _cmsChips[2];
	
	int8 _tempo;
	int8 _tempoSum;
	byte _looping;
	byte _octaveMask;
	int16 _midiDelay;
	Voice2 *_midiChannel[16];
	byte _midiChannelUse[16];
	byte *_midiData;
	byte *_midiSongBegin;
	
	int _loadedMidiSong;
	
	byte _lastMidiCommand;
	uint _outputTableReady;
	byte _clkFrequenz;
	byte _restart;
	byte _curSno;
	
	void loadMidiData(byte *data, int sound);
	void play();
	
	void processChannel(Voice2 *channel);
	void processRelease(Voice2 *channel);
	void processAttack(Voice2 *channel);
	void processDecay(Voice2 *channel);
	void processSustain(Voice2 *channel);
	void processVibrato(Voice2 *channel);
	
	void playMusicChips(const MusicChip *table);
	void playNote(byte *&data);
	void clearNote(byte *&data);
	void offAllChannels();
	void playVoice();
	void processMidiData(uint ticks);
	
	Voice2 *getFreeVoice();
	Voice2 *getPlayVoice(byte param);
	
	// from Player_V2
protected:
	bool _isV3Game;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	ScummEngine *_vm;

	int _header_len;

	uint32 _sample_rate;
	uint32 _next_tick;
	uint32 _tick_len;

	int _timer_count[4];
	int _timer_output;

	int   _current_nr;
	byte *_current_data;
	int   _next_nr;
	byte *_next_data;
	byte *_retaddr;

private:
	union ChannelInfo {
		channel_data d;
		uint16 array[sizeof(channel_data)/2];
	};

	int _music_timer;
	int _music_timer_ctr;
	int _ticks_per_music_timer;

	Common::Mutex _mutex;
	ChannelInfo _channels[5];

protected:
	void mutex_up();
	void mutex_down();

	virtual void nextTick();
	virtual void clear_channel(int i);
	virtual void chainSound(int nr, byte *data);
	virtual void chainNextSound();

private:
	void do_mix(int16 *buf, uint len);

	void execute_cmd(ChannelInfo *channel);
	void next_freqs(ChannelInfo *channel);
};

} // End of namespace Scumm

#endif
