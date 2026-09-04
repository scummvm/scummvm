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

#ifndef MOHAWK_SOUND_H
#define MOHAWK_SOUND_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

#include "audio/mixer.h"
#include "audio/midiplayer.h"

class MidiDriver;
class MidiParser;

namespace Audio {
class SeekableAudioStream;
}

namespace Mohawk {

#define MAX_CHANNELS 2         // Can there be more than 2?

enum SndHandleType {
	kFreeHandle,
	kUsedHandle
};

/**
 * Tracks one mixer playback instance created by @ref Mohawk::Sound.
 *
 * The sound manager reuses an entry after its mixer handle becomes inactive.
 * Resource identity and sample rate are retained separately because the mixer
 * handle does not expose the Mohawk SND ID or a source-frame position.
 */
struct SndHandle {
	/** Mixer handle used to stop the instance and query its playback state. */
	Audio::SoundHandle handle;
	/** Allocation state of this entry in @ref Mohawk::Sound::_handles. */
	SndHandleType type;
	/** Source sample rate used to convert elapsed milliseconds to sample frames. */
	uint samplesPerSecond;
	/** Mohawk SND resource ID associated with the active mixer instance. */
	uint16 id;
};

/**
 * Parsed contents of an MHWK/WAVE ADPC seek-state chunk.
 *
 * Each item captures enough per-channel decoder state to resume ADPCM decoding
 * at a particular source sample frame. The current decoder parses and validates
 * these records but does not yet use them for seeking.
 */
struct ADPCMStatus {
	/** Serialized ADPC chunk payload size in bytes. */
	uint32 size;
	/** Number of seek-state records stored in @ref Mohawk::ADPCMStatus::statusItems. */
	uint16 itemCount;
	/** Number of channel states serialized in each seek-state record. */
	uint16 channels;

	/** Decoder state associated with one source sample-frame position. */
	struct StatusItem {
		/** Source sample frame at which this decoder state becomes valid. */
		uint32 sampleFrame;
		/** Predictor and step-table state needed to resume one ADPCM channel. */
		struct {
			/** Most recently decoded sample, used as the ADPCM predictor. */
			int16 last;
			/** Index into the ADPCM step-size table. */
			uint16 stepIndex;
		/**
		 * Per-channel decoder state.
		 *
		 * Only the first @ref Mohawk::ADPCMStatus::channels entries are serialized.
		 */
		} channelStatus[MAX_CHANNELS];
	} *statusItems; /**< Dynamically allocated array containing
	                * @ref Mohawk::ADPCMStatus::itemCount records.
	                * The MHWK decoder owns this array while parsing the ADPC
	                * chunk and releases it immediately because ADPCM seeking
	                * is not currently implemented.
	                */
};

/** One named synchronization point parsed from an MHWK/WAVE Cue# chunk. */
struct CueListPoint {
	/** Source sample frame at which the cue occurs. */
	uint32 sampleFrame;
	/** Serialized cue label used by consumers to identify the synchronization point. */
	Common::String name;
};

/**
 * Parsed synchronization-point list from an MHWK/WAVE Cue# chunk.
 *
 * Callers may use these points to synchronize animation or other game state
 * with sound playback.
 */
struct CueList {
	/** Serialized Cue# chunk payload size in bytes. */
	uint32 size;
	/** Number of cue records declared by the chunk. */
	uint16 pointCount;
	/** Cue records in their serialized order. */
	Common::Array<CueListPoint> points;
};

enum {
	kCodecRaw = 0,
	kCodecADPCM = 1,
	kCodecMPEG2 = 2
};

/**
 * Parsed state for the 20-byte MHWK/WAVE Data header and its payload.
 *
 * The loop positions are source sample-frame positions, not byte offsets.
 * @ref Mohawk::DataChunk::loopStart is inclusive.
 * @ref Mohawk::DataChunk::loopEnd is exclusive and must not exceed
 * @ref Mohawk::DataChunk::sampleCount.
 *
 * A loop count of zero disables the embedded loop.
 * A loop count of 0xFFFF requests an infinite loop.
 * Another nonzero value requests that many additional passes through the
 * loop range.
 *
 * Zoombini playback applies every supported embedded infinite loop automatically.
 * Finite loop counts remain parsed but are not yet applied by the ScummVM stream wrapper.
 */
struct DataChunk {
	/** Number of source sample frames played per second. */
	uint16 sampleRate;
	/** Total number of source sample frames declared by the Data header. */
	uint32 sampleCount;
	/** Number of encoded bits for each sample in one channel. */
	byte bitsPerSample;
	/** Number of interleaved audio channels in each source sample frame. */
	byte channels;
	/**
	 * Payload codec, using @ref Mohawk::kCodecRaw,
	 * @ref Mohawk::kCodecADPCM, or @ref Mohawk::kCodecMPEG2.
	 */
	uint16 encoding;
	/**
	 * Number of requested repeats for the embedded loop range.
	 *
	 * 0x0000 disables looping, 0xFFFF is an infinite loop.
	 */
	uint16 loopCount;
	/** Inclusive source sample frame at which an embedded loop begins. */
	uint32 loopStart;
	/** Exclusive source sample frame at which playback returns to @ref DataChunk::loopStart. */
	uint32 loopEnd;
	/**
	 * Stream containing only the encoded sample payload after the Data header.
	 *
	 * Ownership transfers to the codec-specific audio stream factory.
	 */
	Common::SeekableReadStream *audioData;
};

/**
 * Embedded-loop range exported by the MHWK decoder.
 *
 * Both positions use source sample frames.
 * @ref Mohawk::MohawkWaveLoopInfo::start is inclusive.
 * @ref Mohawk::MohawkWaveLoopInfo::end is exclusive.
 * An empty range means that the resource has no supported embedded loop.
 */
struct MohawkWaveLoopInfo {
	uint32 start = 0;
	uint32 end = 0;

	bool isValid() const { return start < end; }
};

/**
 * Decode an MHWK/WAVE resource as a seekable stream.
 *
 * This low-level decoder returns the linear sample stream and can optionally
 * export a validated embedded-loop range. Zoombini's @ref Mohawk::Sound
 * playback layer requests that range and wraps the decoded stream
 * automatically.
 *
 * @param stream The MHWK resource stream, which is consumed and deleted.
 * @param cueList Optional destination for parsed Cue# entries.
 * @param loopInfo Optional destination for a validated embedded-loop range.
 * @return The decoded stream, or nullptr when the resource cannot be decoded.
 */
Audio::SeekableAudioStream *makeMohawkWaveStream(Common::SeekableReadStream *stream, CueList *cueList = nullptr, MohawkWaveLoopInfo *loopInfo = nullptr);

class MohawkEngine;

class Sound {
public:
	/**
	 * Create a common Mohawk sound manager.
	 */
	explicit Sound(MohawkEngine *vm);
	virtual ~Sound();

	// Generic sound functions
	Audio::SoundHandle *playSound(uint16 id, byte volume = Audio::Mixer::kMaxChannelVolume, bool loop = false, CueList *cueList = NULL);
	Audio::SoundHandle *playSound(uint16 id, Audio::Mixer::SoundType soundType, byte volume = Audio::Mixer::kMaxChannelVolume, bool loop = false, CueList *cueList = NULL);
	void stopSound();
	void stopSound(uint16 id);
	bool isPlaying(uint16 id);
	bool isPlaying();
	uint getNumSamplesPlayed(uint16 id);

protected:
	/** Play a decoded audio stream and take ownership of it. */
	Audio::SoundHandle *playSoundStream(Audio::SeekableAudioStream *seekableStream, uint16 id, Audio::Mixer::SoundType soundType, byte volume, bool loop, const MohawkWaveLoopInfo &loopInfo);

private:
	MohawkEngine *_vm;

	static Audio::SeekableAudioStream *makeLivingBooksWaveStream_v1(Common::SeekableReadStream *stream);

	Common::Array<SndHandle> _handles;
	SndHandle *getHandle();
	Audio::SeekableAudioStream *makeAudioStream(uint16 id, CueList *cueList = nullptr, MohawkWaveLoopInfo *loopInfo = nullptr);
};

class MidiPlayer : public Audio::MidiPlayer {
public:
	MidiPlayer(MohawkEngine *vm);
	~MidiPlayer();

	void pause(bool p);
	void playMidi(uint16 id);

	// When enabled, a GM reset is sent before each song starts.
	// Needed for the MIDI tracks without the inline GM/GS setup.
	// e.g. Zoombini Macintosh MIDI profile
	void setResetChannelsOnPlay(bool reset) { _resetChannelsOnPlay = reset; }

	void pause() override { Audio::MidiPlayer::pause(); }

	void sendToChannel(byte channel, uint32 b) override;
	void onTimer() override;

protected:
	/** Play an already-opened Mohawk tMID stream and take ownership of it. */
	void playMidiStream(Common::SeekableReadStream *stream, uint16 id);

private:
	MohawkEngine *_vm;
	bool _paused;
	bool _resetChannelsOnPlay;

	static bool extractMohawkMidi(Common::SeekableReadStream *stream, Common::Array<byte> &standardMidi);
	Common::SeekableReadStream *makeMidiStream(uint16 id);
	void playMidiStreamLocked(Common::SeekableReadStream *stream, uint16 id);
};

} // End of namespace Mohawk

#endif
