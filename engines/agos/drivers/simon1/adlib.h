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

#ifndef AGOS_SIMON1_ADLIB_H
#define AGOS_SIMON1_ADLIB_H

#include "audio/mididrv.h"
#include "audio/fmopl.h"

namespace AGOS {

class MidiDriver_Simon1_AdLib : public MidiDriver {
public:
	MidiDriver_Simon1_AdLib(const byte *instrumentData);
	~MidiDriver_Simon1_AdLib() override;

	// MidiDriver API
	int open() override;
	bool isOpen() const override;
	void close() override;

	void send(uint32 b) override;

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override;
	uint32 getBaseTempo() override;

	MidiChannel *allocateChannel() override { return 0; }
	MidiChannel *getPercussionChannel() override { return 0; }
private:
	bool _isOpen;

	OPL::OPL *_opl;

	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	void onTimer();

	void reset();
	void resetOPLVoices();

	void resetRhythm();
	int _melodyVoices;
	uint8 _amvdrBits;
	bool _rhythmEnabled;

	enum {
		kNumberOfVoices = 11,
		kNumberOfMidiChannels = 16
	};

	struct Voice {
		Voice();

		uint channel;
		uint note;
		uint instrTotalLevel;
		uint instrScalingLevel;
		uint frequency;
	};

	void resetVoices();
	int allocateVoice(uint channel);

	Voice _voices[kNumberOfVoices];
	uint _midiPrograms[kNumberOfMidiChannels];

	void noteOff(uint channel, uint note);
	void noteOn(uint channel, uint note, uint velocity);
	void noteOnRhythm(uint channel, uint note, uint velocity);
	void controlChange(uint channel, uint controller, uint value);
	void programChange(uint channel, uint program);

	void setupInstrument(uint voice, uint instrument);
	const byte *_instruments;

	static const int _operatorMap[9];
	static const int _operatorDefaults[8];

	static const int _rhythmOperatorMap[5];
	static const uint _rhythmInstrumentMask[5];
	static const int _rhythmVoiceMap[5];

	static const int _frequencyIndexAndOctaveTable[128];
	static const int _frequencyTable[16];

	struct RhythmMap {
		int channel;
		int program;
		int note;
	};

	static const RhythmMap _rhythmMap[39];
};

MidiDriver *createMidiDriverSimon1AdLib(const char *instrumentFilename);

} // End of namespace AGOS

#endif
