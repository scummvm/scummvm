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

#include "audio/fmopl.h"
#include "audio/mididrv.h"

namespace Queen {

class AdLibMidiDriver : public MidiDriver {
public:

	AdLibMidiDriver() {
		_adlibWaveformSelect = 0;
		_isOpen = false;
	}

	~AdLibMidiDriver() override {}

	// MidiDriver
	int open() override;
	void close() override;
	void send(uint32 b) override;
	void metaEvent(byte type, byte *data, uint16 length) override;
	MidiChannel *allocateChannel() override { return 0; }
	MidiChannel *getPercussionChannel() override { return 0; }
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override;
	bool isOpen() const override { return _isOpen; }
	uint32 getBaseTempo() override { return 1000000 / OPL::OPL::kDefaultCallbackFrequency; }

	void setVolume(uint32 volume);

private:

	void handleMidiEvent0x90_NoteOn(int channel, int param1, int param2);
	void handleSequencerSpecificMetaEvent1(int channel, const uint8 *data);
	void handleSequencerSpecificMetaEvent2(uint8 value);
	void handleSequencerSpecificMetaEvent3(uint8 value);

	void adlibWrite(uint8 port, uint8 value);
	void adlibSetupCard();
	void adlibSetupChannels(int fl);
	void adlibResetAmpVibratoRhythm(int am, int vib, int kso);
	void adlibResetChannels();
	void adlibSetAmpVibratoRhythm();
	void adlibSetCSMKeyboardSplit();
	void adlibSetNoteMul(int mul);
	void adlibSetWaveformSelect(int fl);
	void adlibSetPitchBend(int channel, int range);
	void adlibPlayNote(int channel);
	uint8 adlibPlayNoteHelper(int channel, int note1, int note2, int oct);
	void adlibTurnNoteOff(int channel);
	void adlibTurnNoteOn(int channel, int note);
	void adlibSetupChannelFromSequence(int channel, const uint8 *src, int fl);
	void adlibSetupChannel(int channel, const uint16 *src, int fl);
	void adlibSetNoteVolume(int channel, int volume);
	void adlibSetChannelVolume(int channel, uint8 volume);
	void adlibSetupChannelHelper(int channel);
	void adlibSetChannel0x40(int channel);
	void adlibSetChannel0xC0(int channel);
	void adlibSetChannel0x60(int channel);
	void adlibSetChannel0x80(int channel);
	void adlibSetChannel0x20(int channel);
	void adlibSetChannel0xE0(int channel);

	void onTimer();

	OPL::OPL *_opl;
	int _midiNumberOfChannels;
	int _adlibNoteMul;
	int _adlibWaveformSelect;
	int _adlibAMDepthEq48;
	int _adlibVibratoDepthEq14;
	int _adlibRhythmEnabled;
	int _adlibKeyboardSplitOn;
	int _adlibVibratoRhythm;
	uint8 _midiChannelsFreqTable[9];
	uint8 _adlibChannelsLevelKeyScalingTable[11];
	uint8 _adlibSetupChannelSequence1[14 * 18];
	uint16 _adlibSetupChannelSequence2[14];
	int16 _midiChannelsNote2Table[9];
	uint8 _midiChannelsNote1Table[9];
	uint8 _midiChannelsOctTable[9];
	uint16 _adlibChannelsVolume[11];
	uint16 _adlibMetaSequenceData[28];
	uint8 _adlibChannelsVolumeTable[11];

	bool _isOpen;
	Common::TimerManager::TimerProc _adlibTimerProc;
	void *_adlibTimerParam;

	static const uint8 _adlibChannelsMappingTable1[];
	static const uint8 _adlibChannelsNoFeedback[];
	static const uint8 _adlibChannelsMappingTable2[];
	static const uint8 _adlibChannelsMappingTable3[];
	static const uint8 _adlibChannelsKeyScalingTable1[];
	static const uint8 _adlibChannelsKeyScalingTable2[];
	static const uint8 _adlibInitSequenceData1[];
	static const uint8 _adlibInitSequenceData2[];
	static const uint8 _adlibInitSequenceData3[];
	static const uint8 _adlibInitSequenceData4[];
	static const uint8 _adlibInitSequenceData5[];
	static const uint8 _adlibInitSequenceData6[];
	static const uint8 _adlibInitSequenceData7[];
	static const uint8 _adlibInitSequenceData8[];
	static const int16 _midiChannelsNoteTable[];
	static const int16 _midiNoteFreqTable[];
};

} // End of namespace Queen
