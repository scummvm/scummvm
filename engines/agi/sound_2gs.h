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

#ifndef AGI_SOUND_2GS_H
#define AGI_SOUND_2GS_H

#include "common/frac.h"
#include "audio/audiostream.h"

namespace Agi {

#define BUFFER_SIZE	410

// Apple IIGS MIDI uses 60 ticks per second (Based on tests with Apple IIGS
// KQ1 and SQ1 under MESS 0.124a). So we make the audio buffer size to be a
// 1/60th of a second in length. That should be getSampleRate() / 60 samples
// in length but as getSampleRate() is always 22050 at the moment we just use
// the hardcoded value of 368 (22050/60 = 367.5 which rounds up to 368).
// FIXME: Use getSampleRate() / 60 rather than a hardcoded value
#define IIGS_BUFFER_SIZE 368

// MIDI command values (Shifted right by 4 so they're in the lower nibble)
#define MIDI_CMD_NOTE_OFF        0x08
#define MIDI_CMD_NOTE_ON         0x09
#define MIDI_CMD_CONTROLLER      0x0B
#define MIDI_CMD_PROGRAM_CHANGE  0x0C
#define MIDI_CMD_PITCH_WHEEL     0x0E
// Whole MIDI byte values (Command and channel info together)
#define MIDI_BYTE_STOP_SEQUENCE  0xFC
#define MIDI_BYTE_TIMER_SYNC     0xF8

struct IIgsEnvelopeSegment {
	uint8 bp;
	uint16 inc; ///< 8b.8b fixed point, very probably little endian
};

#define ENVELOPE_SEGMENT_COUNT 8
struct IIgsEnvelope {
	IIgsEnvelopeSegment seg[ENVELOPE_SEGMENT_COUNT];

	/** Reads an Apple IIGS envelope from then given stream. */
	bool read(Common::SeekableReadStream &stream);
};

// 2**(1/12) i.e. the 12th root of 2
#define SEMITONE 1.059463094359295

// C6's frequency is A4's (440 Hz) frequency but one full octave and three semitones higher
// i.e. C6_FREQ = 440 * pow(2.0, 15/12.0)
#define C6_FREQ 1046.502261202395

// Size of the SIERRASTANDARD file (i.e. the wave file i.e. the sample data used by the instruments).
#define SIERRASTANDARD_SIZE 65536

// Maximum number of instruments in an Apple IIGS instrument set.
// Chosen empirically based on Apple IIGS AGI game data, increase if needed.
#define MAX_INSTRUMENTS 28

struct IIgsWaveInfo {
	uint8 top;
	uint addr;
	uint size;
// Oscillator channel
#define OSC_CHANNEL_RIGHT 0
#define OSC_CHANNEL_LEFT  1
	uint channel;
// Oscillator mode
#define OSC_MODE_LOOP     0
#define OSC_MODE_ONESHOT  1
#define OSC_MODE_SYNC_AM  2
#define OSC_MODE_SWAP     3
	uint mode;
	bool halt;
	int16 relPitch; ///< Relative pitch in semitones (Signed 8b.8b fixed point)

	/** Reads an Apple IIGS wave information structure from the given stream. */
	bool read(Common::SeekableReadStream &stream, bool ignoreAddr = false);
	bool finalize(Common::SeekableReadStream &uint8Wave);
};

// Number of waves per Apple IIGS sound oscillator
#define WAVES_PER_OSCILLATOR 2

/** An Apple IIGS sound oscillator. Consists always of two waves. */
struct IIgsOscillator {
	IIgsWaveInfo waves[WAVES_PER_OSCILLATOR];

	bool finalize(Common::SeekableReadStream &uint8Wave);
};

// Maximum number of oscillators in an Apple IIGS instrument.
// Chosen empirically based on Apple IIGS AGI game data, increase if needed.
#define MAX_OSCILLATORS 4

/** An Apple IIGS sound oscillator list. */
struct IIgsOscillatorList {
	uint count; ///< Oscillator count
	IIgsOscillator osc[MAX_OSCILLATORS]; ///< The oscillators

	/** Indexing operators for easier access to the oscillators. */
	const IIgsOscillator &operator()(uint index) const { return osc[index]; }
	IIgsOscillator &operator()(uint index) { return osc[index]; }

	/** Reads an Apple IIGS oscillator list from the given stream. */
	bool read(Common::SeekableReadStream &stream, uint oscillatorCount, bool ignoreAddr = false);
	bool finalize(Common::SeekableReadStream &uint8Wave);
};

struct IIgsInstrumentHeader {
	IIgsEnvelope env;
	uint8 relseg;
	uint8 bendrange;
	uint8 vibdepth;
	uint8 vibspeed;
	IIgsOscillatorList oscList;

	/**
	 * Read an Apple IIGS instrument header from the given stream.
	 * @param stream The source stream from which to read the data.
	 * @param ignoreAddr Should we ignore wave infos' wave address variable's value?
	 * @return True if successful, false otherwise.
	 */
	bool read(Common::SeekableReadStream &stream, bool ignoreAddr = false);
	bool finalize(Common::SeekableReadStream &uint8Wave);
};

struct IIgsSampleHeader {
	uint16 type;
	uint8  pitch; ///< Logarithmic, base is 2**(1/12), unknown multiplier (Possibly in range 1040-1080)
	uint8  unknownByte_Ofs3; // 0x7F in Gold Rush's sound resource 60, 0 in all others.
	uint8  volume; ///< Current guess: Logarithmic in 6 dB steps
	uint8  unknownByte_Ofs5; ///< 0 in all tested samples.
	uint16 instrumentSize; ///< Little endian. 44 in all tested samples. A guess.
	uint16 sampleSize; ///< Little endian. Accurate in all tested samples excluding Manhunter I's sound resource 16.
	IIgsInstrumentHeader instrument;

	/**
	 * Read an Apple IIGS AGI sample header from the given stream.
	 * @param stream The source stream from which to read the data.
	 * @return True if successful, false otherwise.
	 */
	bool read(Common::SeekableReadStream &stream);
	bool finalize(Common::SeekableReadStream &uint8Wave);
};

struct IIgsChannelInfo {
	const IIgsInstrumentHeader *ins; ///< Instrument info
	const int8 *relocatedSample; ///< Source sample data (8-bit signed format) using relocation
	const int8 *unrelocatedSample; ///< Source sample data (8-bit signed format) without relocation
	frac_t pos;     ///< Current sample position
	frac_t posAdd;  ///< Current sample position adder (Calculated using note, vibrato etc)
	uint8 origNote; ///< The original note without the added relative pitch
	frac_t note;    ///< Note (With the added relative pitch)
	frac_t vol;     ///< Current volume (Takes both channel volume and enveloping into account)
	frac_t chanVol; ///< Channel volume
	frac_t startEnvVol; ///< Starting envelope volume
	frac_t envVol;  ///< Current envelope volume
	uint   envSeg;  ///< Current envelope segment
	uint   size;    ///< Sample size
	bool   loop;    ///< Should we loop the sample?
	bool   end;     ///< Has the playing ended?

	void rewind(); ///< Rewinds the sound playing on this channel to its start
	void setChannelVolume(uint8 volume); ///< Sets the channel volume
	void setInstrument(const IIgsInstrumentHeader *instrument, const int8 *sample); ///< Sets the instrument to be used on this channel
	void noteOn(uint8 noteParam, uint8 velocity); ///< Starts playing a note on this channel
	void noteOff(uint8 velocity); ///< Releases the note on this channel
	void stop(); ///< Stops the note playing on this channel instantly
	bool playing(); ///< Is there a note playing on this channel?
};

class IIgsMidi : public AgiSound {
public:
	IIgsMidi(uint8 *data, uint32 len, int resnum, SoundMgr &manager);
	~IIgsMidi() { if (_data != NULL) free(_data); }
	virtual uint16 type() { return _type; }
	virtual const uint8 *getPtr() { return _ptr; }
	virtual void setPtr(const uint8 *ptr) { _ptr = ptr; }
	virtual void rewind() { _ptr = _data + 2; _midiTicks = _soundBufTicks = 0; }
protected:
	uint8 *_data; ///< Raw sound resource data
	const uint8 *_ptr; ///< Pointer to the current position in the MIDI data
	uint32 _len; ///< Length of the raw sound resource
	uint16 _type; ///< Sound resource type
public:
	uint _midiTicks; ///< MIDI song position in ticks (1/60ths of a second)
	uint _soundBufTicks; ///< Sound buffer position in ticks (1/60ths of a second)
};

class IIgsSample : public AgiSound {
public:
	IIgsSample(uint8 *data, uint32 len, int resnum, SoundMgr &manager);
	~IIgsSample() { delete[] _sample; }
	virtual uint16 type() { return _header.type; }
	const IIgsSampleHeader &getHeader() const { return _header; }
	const int8 *getSample() const { return _sample; }
protected:
	IIgsSampleHeader _header; ///< Apple IIGS AGI sample header
	int8 *_sample;           ///< Sample data (8-bit signed format)
};

/** Apple IIGS MIDI program change to instrument number mapping. */
struct MidiProgramChangeMapping {
	byte midiProgToInst[44]; ///< Lookup table for the MIDI program number to instrument number mapping
	byte undefinedInst; ///< The undefined instrument number

	// Maps the MIDI program number to an instrument number
	byte map(uint midiProg) const {
		return midiProg < ARRAYSIZE(midiProgToInst) ? midiProgToInst[midiProg] : undefinedInst;
	}
};

/** Apple IIGS AGI instrument set information. */
struct InstrumentSetInfo {
	uint byteCount;          ///< Length of the whole instrument set in bytes
	uint instCount;          ///< Amount of instrument in the set
	const char *md5;         ///< MD5 hex digest of the whole instrument set
	const char *waveFileMd5; ///< MD5 hex digest of the wave file (i.e. the sample data used by the instruments)
	const MidiProgramChangeMapping *progToInst; ///< Program change to instrument number mapping
};

/** Apple IIGS AGI executable file information. */
struct IIgsExeInfo {
	enum AgiGameID gameid;            ///< Game ID
	const char *exePrefix;            ///< Prefix of the Apple IIGS AGI executable (e.g. "SQ", "PQ", "KQ4" etc)
	uint agiVer;                      ///< Apple IIGS AGI version number, not strictly needed
	uint exeSize;                     ///< Size of the Apple IIGS AGI executable file in bytes
	uint instSetStart;                ///< Starting offset of the instrument set inside the executable file
	const InstrumentSetInfo *instSet; ///< Information about the used instrument set
};

class IIgsMidiChannel {
public:
	IIgsMidiChannel() : _instrument(0), _sample(0), _volume(0) {}
	uint activeSounds() const; ///< How many active sounds are playing?
	void setInstrument(const IIgsInstrumentHeader *instrument, const int8 *sample);
	void setVolume(uint8 volume);
	void noteOff(uint8 note, uint8 velocity);
	void noteOn(uint8 note, uint8 velocity);
	void stopSounds(); ///< Clears the channel of any sounds
	void removeStoppedSounds(); ///< Removes all stopped sounds from this MIDI channel
public:
	typedef Common::Array<IIgsChannelInfo>::const_iterator const_iterator;
	typedef Common::Array<IIgsChannelInfo>::iterator iterator;
	Common::Array<IIgsChannelInfo> _gsChannels;	///< Apple IIGS channels playing on this MIDI channel
protected:
	const IIgsInstrumentHeader *_instrument;	///< Instrument used on this MIDI channel
	const int8 *_sample;						///< Sample data used on this MIDI channel
	uint8 _volume;								///< MIDI controller number 7 (Volume)
};

class SoundGen2GS : public SoundGen, public Audio::AudioStream {
public:
	SoundGen2GS(AgiEngine *vm, Audio::Mixer *pMixer);
	~SoundGen2GS();

	void play(int resnum);
	void stop(void);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const {
		return false;
	}

	bool endOfData() const {
		return false;
	}

	int getRate() const {
		// FIXME: Ideally, we should use _sampleRate.
		return 22050;
	}

private:
	bool _disabledMidi;
	int _playingSound;
	bool _playing;

	int16 *_sndBuffer;

/**
 * Class for managing Apple IIGS sound channels.
 * TODO: Check what instruments are used by default on the MIDI channels
 * FIXME: Some instrument choices sound wrong
 */
private:
	typedef Common::Array<IIgsMidiChannel>::const_iterator const_iterator;
	typedef Common::Array<IIgsMidiChannel>::iterator iterator;
	static const uint kSfxMidiChannel = 0; ///< The MIDI channel used for playing sound effects

	bool loadInstruments();
	const IIgsExeInfo *getIIgsExeInfo(enum AgiGameID gameid) const;

	void setProgramChangeMapping(const MidiProgramChangeMapping *mapping);
	bool loadInstrumentHeaders(const Common::FSNode &exePath, const IIgsExeInfo &exeInfo);
	bool loadWaveFile(const Common::FSNode &wavePath, const IIgsExeInfo &exeInfo);

	// Miscellaneous methods
	void fillAudio(int16 *stream, uint len);
	uint32 mixSound();
	void playSound();
	uint activeSounds() const; ///< How many active sounds are playing?
	void stopSounds(); ///< Stops all sounds
	void removeStoppedSounds(); ///< Removes all stopped sounds from the MIDI channels

	// For playing Apple IIGS AGI samples (Sound effects etc)
	bool playSampleSound(const IIgsSampleHeader &sampleHeader, const int8 *sample);
	void playMidiSound();
	void playSampleSound();

	// MIDI commands
	void midiNoteOff(uint8 channel, uint8 note, uint8 velocity);
	void midiNoteOn(uint8 channel, uint8 note, uint8 velocity);
	void midiController(uint8 channel, uint8 controller, uint8 value);
	void midiProgramChange(uint8 channel, uint8 program);
	void midiPitchWheel(uint8 wheelPos);
	//protected:
	const IIgsInstrumentHeader* getInstrument(uint8 program) const;
	//public:
	Common::Array<IIgsMidiChannel> _midiChannels;		///< Information about each MIDI channel
	//protected:
	Common::Array<int8> _wave;							///< Sample data used by the Apple IIGS MIDI instruments
	const MidiProgramChangeMapping *_midiProgToInst;	///< MIDI program change to instrument number mapping
	Common::Array<IIgsInstrumentHeader> _instruments;	///< Instruments used by the Apple IIGS AGI
};

} // End of namespace Agi

#endif /* AGI_SOUND_2GS_H */
