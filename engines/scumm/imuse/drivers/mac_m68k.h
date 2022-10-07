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

#ifndef SCUMM_IMUSE_MAC_M68K_H
#define SCUMM_IMUSE_MAC_M68K_H

#include "audio/softsynth/emumidi.h"

#include "common/hashmap.h"

namespace Common {
class SeekableReadStream;
}

namespace Scumm {

class IMuseDriver_MacM68k : public MidiDriver_Emulated {
	friend class MidiChannel_MacM68k;
public:
	IMuseDriver_MacM68k(Audio::Mixer *mixer);
	~IMuseDriver_MacM68k() override;

	int open() override;
	void close() override;

	void send(uint32 d) override;

	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override { return nullptr; }

	bool isStereo() const override { return false; }
	int getRate() const override {
		// The original is using a frequency of approx. 22254.54546 here.
		// To be precise it uses the 16.16 fixed point value 0x56EE8BA3.
		return 22254;
	}

protected:
	void generateSamples(int16 *buf, int len) override;
	void onTimer() override {}

private:
	int *_mixBuffer;
	int _mixBufferLength;

	struct Instrument {
		uint length;
		uint sampleRate;
		uint loopStart;
		uint loopEnd;
		int baseFrequency;

		byte *data;
	};

	enum {
		kDefaultInstrument = 0x3E7,
		kProgramChangeBase = 0x3E8,
		kSysExBase         = 0x7D0
	};

	Instrument getInstrument(int idx) const;
	typedef Common::HashMap<int, Instrument> InstrumentMap;
	InstrumentMap _instruments;
	Instrument _defaultInstrument;
	void loadAllInstruments();
	void addInstrument(int idx, Common::SeekableReadStream *data);

	struct OutputChannel {
		int pitchModifier;

		const byte *instrument;
		uint subPos;

		const byte *start;
		const byte *end;

		const byte *soundStart;
		const byte *soundEnd;
		const byte *loopStart;
		const byte *loopEnd;

		int frequency;
		int volume;

		bool isFinished;

		int baseFrequency;
	};

	void setPitch(OutputChannel *out, int frequency);
	int _pitchTable[128];

	byte *_volumeTable;
	static const int _volumeBaseTable[32];

	class MidiChannel_MacM68k;

	struct VoiceChannel {
		MidiChannel_MacM68k *part;
		VoiceChannel *prev, *next;
		int channel;
		int note;
		bool sustainNoteOff;
		OutputChannel out;

		void off();
	};

	class MidiChannel_MacM68k : public MidiChannel {
		friend class IMuseDriver_MacM68k;
	public:
		MidiChannel_MacM68k(IMuseDriver_MacM68k *driver, byte number) : MidiChannel(), _owner(driver), _number(number), _allocated(false),
			_priority(0), _sustain(0), _pitchBend(0), _pitchBendFactor(2), _transpose(0), _detune(0), _volume(0) {}
		MidiDriver *device() override { return _owner; }
		byte getNumber() override { return _number; }
		void release() override;

		void send(uint32 b) override;
		void noteOff(byte note) override;
		void noteOn(byte note, byte velocity) override;
		void programChange(byte program) override;
		void pitchBend(int16 bend) override;
		void controlChange(byte control, byte value) override;
		void pitchBendFactor(byte value) override;
		void transpose(int8 value) override;
		void detune(byte value) override;
		void priority(byte value) override;
		void sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize) override;

		bool allocate();

		void addVoice(VoiceChannel *voice);
		void removeVoice(VoiceChannel *voice);
	private:
		IMuseDriver_MacM68k *_owner;
		const byte _number;
		bool _allocated;

		VoiceChannel *_voice;
		int _priority;
		int _sustain;
		Instrument _instrument;
		int _pitchBend;
		int _pitchBendFactor;
		int8 _detune;
		int8 _transpose;
		int _volume;
	};

	MidiChannel_MacM68k *_channels[32];

	enum {
		kChannelCount = 8
	};
	VoiceChannel _voiceChannels[kChannelCount];
	int _lastUsedVoiceChannel;
	VoiceChannel *allocateVoice(int priority);
};

} // End of namespace Scumm

#endif
