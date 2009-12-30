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

#ifndef MOHAWK_SOUND_H
#define MOHAWK_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"

#include "sound/audiostream.h"
#include "sound/adpcm.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/mixer.h"

#include "mohawk/mohawk.h"
#include "mohawk/file.h"

namespace Mohawk {

#define MAX_CHANNELS 2         // Can there be more than 2?
#define CUE_MAX 256

struct SLSTRecord {
	uint16 index;
	uint16 sound_count;
	uint16* sound_ids;
	uint16 fade_flags;
	uint16 loop;
	uint16 global_volume;
	uint16 u0;
	uint16 u1;
	uint16* volumes;
	int16* balances;
	uint16* u2;
};

enum SndHandleType {
	kFreeHandle,
	kUsedHandle
};

struct SndHandle {
	Audio::SoundHandle handle;
	SndHandleType type;
};

struct SLSTSndHandle {
	Audio::SoundHandle *handle;
	uint16 id;
};

struct ADPC_Chunk {            // Appears to only exist if there isn't MPEG-2 Audio
	uint32 size;
	uint16 u0;                 // Unknown (2 when there's a Cue# Chunk, 1 when there's not)
	uint16 channels;
	uint32 u1;                 // Unknown (always 0)
	uint32 u2[MAX_CHANNELS];   // Unknown (0x00400000 for both channels)

	// If there is a Cue# chunk, there can be two more variables:
	uint32 u3;
	uint32 u4[MAX_CHANNELS];
};

struct Cue_Chunk {
	uint32 size;
	uint16 point_count;
	struct {
		uint32 position;
		byte length;
		Common::String name;
	} cueList[CUE_MAX];
};

enum {
	kCodecRaw = 0,
	kCodecADPCM = 1,
	kCodecMPEG2 = 2
};

struct Data_Chunk {
	uint32 size;
	uint16 sample_rate;
	uint32 sample_count;
	byte bitsPerSample;
	byte channels;
	uint16 encoding;
	uint16 loop;
	uint32 loopStart;
	uint32 loopEnd;
	byte* audio_data;
};

class MohawkEngine;

class Sound {
public:
	Sound(MohawkEngine*);
	~Sound();
	
	void loadRivenSounds(uint16 stack);
	Audio::SoundHandle *playSound(uint16 id, bool mainSoundFile = true, byte volume = Audio::Mixer::kMaxChannelVolume);
	void playMidi(uint16 id);
	void stopSound();
	void pauseSound();
	void resumeSound();
	void playSLST(uint16 index, uint16 card);
	void playSLST(SLSTRecord slstRecord);
	void pauseSLST();
	void resumeSLST();
	void stopAllSLST();

private:
	MohawkEngine *_vm;
	MohawkFile *_rivenSoundFile;
	MidiDriver *_midiDriver;
	MidiParser *_midiParser;

	static Audio::AudioStream *getCSAmtrakMusic(uint16 id);
	static Audio::AudioStream *makeMohawkWaveStream(Common::SeekableReadStream *stream, bool loop = false);
	static Audio::AudioStream *makeOldMohawkWaveStream(Common::SeekableReadStream *stream, bool loop = false);
	void initMidi();

	Common::Array<SndHandle> _handles;
	SndHandle *getHandle();
	
	// Riven specific
	void playSLSTSound(uint16 index, bool fade, bool loop, uint16 volume, int16 balance);
	void stopSLSTSound(uint16 id, bool fade);
	Common::Array<SLSTSndHandle> _currentSLSTSounds;
};

} // End of namespace Mohawk

#endif
