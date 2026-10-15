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
#ifndef IGOR_MIDI_H
#define IGOR_MIDI_H

#include "common/util.h"
#include "common/mutex.h"

#include "audio/fmopl.h"
#include "audio/softsynth/emumidi.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"

namespace Igor {

enum {
	kAdlibCarrier = 0,
	kAdlibModulator
};

struct AdlibInstrument {
	uint8 chr[2];
	uint8 scale[2];
	uint8 attack[2];
	uint8 sustain[2];
	uint8 waveSel[2];
	uint8 feedback;
};

class MidiParser_CTMF : public MidiParser {
public:

	enum {
		kMaxInstruments = 16
	};

	MidiParser_CTMF();
	~MidiParser_CTMF() {}

	bool loadMusic(const byte *data, uint32 size) override;

	AdlibInstrument _instruments[kMaxInstruments];
	int _instrumentsCount;

protected:

	void decodeHeader(const uint8 *p);
	void decodeAdlibInstrument(struct AdlibInstrument *ins, const uint8 *p);

	void parseNextEvent(EventInfo &info) override;

private:

	int _instrumentsDataOffset;
	int _midiDataOffset;
	int _ticksPerQuarter;
	int _ticksPerSecond;
	int _basicTempo;
};

class AdlibMidiDriver : public MidiDriver_Emulated {
public:

	enum {
		kAdlibChannelsCount = 18
	};

	AdlibMidiDriver(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer), _opl(nullptr), _oplStream(nullptr), _oplStereo(false) {}
	~AdlibMidiDriver() {}

	// MidiDriver
	int open() override;
	void close() override;
	void send(uint32 b) override;
	void metaEvent(byte type, const byte *data, uint16 length) override {}
	void sysEx(const byte *msg, uint16 length) override {}
	MidiChannel *allocateChannel() override { return 0; }
	MidiChannel *getPercussionChannel() override { return 0; }

	bool isStereo() const override { return _oplStereo; }
	int getRate() const override { return _mixer->getOutputRate(); }
	void generateSamples(int16 *buf, int len) override;

	void setInstruments(AdlibInstrument *i) { _adlibInstruments = i; }

private:

	void adlibWrite(int port, int value);
	void adlibSetupCard();
	void adlibTurnNoteOff(int channel, int note);
	void adlibTurnNoteOn(int channel, int note, int velocity);
	void adlibSetupInstrument(int channel, const AdlibInstrument &ins);
	void adlibSetupPercussion(int channel, const AdlibInstrument &ins);
	void adlibSetupNote(int channel, int note, int velocity);
	void adlibEndNote(int channel);
	void adlibSetVolume(int channel, int volume);
	void adlibControlChange(int channel, int control, int param);
	void adlibProgramChange(int channel, int num);

	OPL::OPL *_opl;
	Audio::EmulatedChip *_oplStream;
	bool _oplStereo;
	uint8 _adlibData[256];
	bool _adlibRhythmMode;
	struct {
		int ch;
		int note;
		int lt;
	} _adlibChannels[kAdlibChannelsCount];
	int _adlibInstrumentsMappingTable[kAdlibChannelsCount];
	AdlibInstrument *_adlibInstruments;

	static const uint8 _adlibOperatorsTable[];
	static const uint8 _adlibChannelsMappingTable[];
	static const int16 _adlibNoteFreqTable[];
	static const uint8 _adlibPercussionsMappingTable[];
};

class MidiPlayer {
public:

	MidiPlayer(Audio::Mixer *mixer);
	~MidiPlayer();

	void playMusic(uint8 *data, uint32 size);
	void stopMusic();
	bool isPlaying();

private:

	void updateTimer();
	static void updateTimerCallback(void *p) { ((MidiPlayer *)p)->updateTimer(); }

	MidiParser_CTMF *_parser;
	AdlibMidiDriver *_driver;
	Common::Mutex _mutex;
	bool _isPlaying;
};

} // namespace Igor

#endif
