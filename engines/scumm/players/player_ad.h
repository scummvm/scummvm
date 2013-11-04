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

#ifndef SCUMM_PLAYERS_PLAYER_AD_H
#define SCUMM_PLAYERS_PLAYER_AD_H

#include "scumm/music.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/mutex.h"

namespace OPL {
class OPL;
}

namespace Scumm {

class ScummEngine;

/**
 * Sound output for v3/v4 AdLib data.
 */
class Player_AD : public MusicEngine, public Audio::AudioStream {
public:
	Player_AD(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_AD();

	// MusicEngine API
	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer();
	virtual int  getSoundStatus(int sound) const;

	virtual void saveLoadWithSerializer(Serializer *ser);

	// AudioStream API
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _rate; }

private:
	ScummEngine *const _vm;
	Common::Mutex _mutex;
	Audio::Mixer *const _mixer;
	const int _rate;
	Audio::SoundHandle _soundHandle;
	void setupVolume();

	OPL::OPL *_opl2;

	int _samplesPerCallback;
	int _samplesPerCallbackRemainder;
	int _samplesTillCallback;
	int _samplesTillCallbackRemainder;

	int _soundPlaying;
	int _engineMusicTimer;

	// AdLib register utilities
	uint8 _registerBackUpTable[256];
	void writeReg(int r, int v);
	uint8 readReg(int r) const;

	// Instrument setup
	void setupChannel(const uint channel, uint instrOffset) {
		setupChannel(channel, _resource + instrOffset);
	}
	void setupChannel(const uint channel, const byte *instrOffset);
	void setupOperator(const uint opr, const byte *&instrOffset);
	static const int _operatorOffsetTable[18];

	// Sound data
	const byte *_resource;

	// Music handling
	void startMusic();
	void updateMusic();
	void noteOff(uint channel);
	int findFreeChannel();
	void setupFrequency(uint channel, int8 frequency);
	void setupRhythm(uint rhythmInstr, uint instrOffset);

	uint _timerLimit;
	uint _musicTicks;
	uint _musicTimer;
	uint _internalMusicTimer;
	bool _loopFlag;
	uint _musicLoopStart;
	uint _instrumentOffset[16];
	uint _channelLastEvent[9];
	uint _channelFrequency[9];
	uint _channelB0Reg[9];

	uint _mdvdrState;
	uint _voiceChannels;
	
	uint _curOffset;
	uint _nextEventTimer;

	static const uint _noteFrequencies[12];
	static const uint _mdvdrTable[6];
	static const uint _rhythmOperatorTable[6];
	static const uint _rhythmChannelTable[6];

	// SFX handling
	void startSfx();
	void updateSfx();
	void clearChannel(int channel);
	void updateChannel(int channel);
	void parseSlot(int channel);
	void updateSlot(int channel);
	void parseNote(int channel, int num, const byte *offset);
	bool processNote(int note, const byte *offset);
	void noteOffOn(int channel);
	void writeRegisterSpecial(int note, uint8 value, int offset);
	uint8 readRegisterSpecial(int note, uint8 defaultValue, int offset);
	void setupNoteEnvelopeState(int note, int steps, int adjust);
	bool processNoteEnvelope(int note, int &instrumentValue);

	int _sfxTimer;

	int _sfxResource[3];
	int _sfxPriority[3];

	struct Channel {
		int state;
		const byte *currentOffset;
		const byte *startOffset;
		uint8 instrumentData[7];
	} _channels[11];

	uint8 _rndSeed;
	uint8 getRnd();

	struct Note {
		int state;
		int playTime;
		int sustainTimer;
		int instrumentValue;
		int bias;
		int preIncrease;
		int adjust;
		
		struct Envelope {
			int stepIncrease;
			int step;
			int stepCounter;
			int timer;
		} envelope;
	} _notes[22];

	static const uint _noteBiasTable[7];
	static const uint _numStepsTable[16];
	static const uint _noteAdjustScaleTable[7];
	static const uint _noteAdjustTable[16];
	static const bool _useOperatorTable[7];
	static const uint _channelOffsetTable[11];
	static const uint _channelOperatorOffsetTable[7];
	static const uint _baseRegisterTable[7];
	static const uint _registerMaskTable[7];
	static const uint _registerShiftTable[7];
};

} // End of namespace Scumm

#endif
