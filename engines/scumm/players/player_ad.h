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

#include "common/mutex.h"
#include "common/serializer.h"

namespace OPL {
class OPL;
}

namespace Scumm {

class ScummEngine;

/**
 * Sound output for v3/v4 AdLib data.
 */
class Player_AD : public MusicEngine {
public:
	Player_AD(ScummEngine *scumm);
	~Player_AD() override;

	// MusicEngine API
	void setMusicVolume(int vol) override;
	void startSound(int sound) override;
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int  getMusicTimer() override;
	int  getSoundStatus(int sound) const override;

	void saveLoadWithSerializer(Common::Serializer &ser) override;

	// Timer callback
	void onTimer();

private:
	ScummEngine *const _vm;
	Common::Mutex _mutex;

	void setupVolume();
	int _musicVolume;
	int _sfxVolume;

	OPL::OPL *_opl2;

	int _musicResource;
	int32 _engineMusicTimer;

	struct SfxSlot;

	struct HardwareChannel {
		bool allocated;
		int priority;
		SfxSlot *sfxOwner;
	} _hwChannels[9];
	int _numHWChannels;
	static const int _operatorOffsetToChannel[22];

	int allocateHWChannel(int priority, SfxSlot *owner = nullptr);
	void freeHWChannel(int channel);
	void limitHWChannels(int newCount);

	// AdLib register utilities
	uint8 _registerBackUpTable[256];
	void writeReg(int r, int v);
	uint8 readReg(int r) const;

	// Instrument setup
	void setupChannel(const uint channel, const byte *instrOffset);
	void setupOperator(const uint opr, const byte *&instrOffset);
	static const int _operatorOffsetTable[18];

	// Music handling
	void startMusic();
	void stopMusic();
	void updateMusic();
	bool parseCommand();
	uint parseVLQ();
	void noteOff(uint channel);
	void setupFrequency(uint channel, int8 frequency);
	void setupRhythm(uint rhythmInstr, uint instrOffset);

	const byte *_musicData;
	uint _timerLimit;
	uint _musicTicks;
	uint32 _musicTimer;
	uint32 _internalMusicTimer;
	bool _loopFlag;
	uint _musicLoopStart;
	uint _instrumentOffset[16];

	struct VoiceChannel {
		uint lastEvent;
		uint frequency;
		uint b0Reg;
	} _voiceChannels[9];
	void freeVoiceChannel(uint channel);

	void musicSeekTo(const uint position);
	bool _isSeeking;

	uint _mdvdrState;

	uint32 _curOffset;
	uint32 _nextEventTimer;

	static const uint _noteFrequencies[12];
	static const uint _mdvdrTable[6];
	static const uint _rhythmOperatorTable[6];
	static const uint _rhythmChannelTable[6];

	// SFX handling
	enum {
		kNoteStatePreInit = -1,
		kNoteStateAttack = 0,
		kNoteStateDecay = 1,
		kNoteStateSustain = 2,
		kNoteStateRelease = 3,
		kNoteStateOff = 4
	};

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
	};

	enum {
		kChannelStateOff = 0,
		kChannelStateParse = 1,
		kChannelStatePlay = 2
	};

	struct Channel {
		int state;
		const byte *currentOffset;
		const byte *startOffset;
		uint8 instrumentData[7];

		Note notes[2];

		int hardwareChannel;
	};

	struct SfxSlot {
		int resource;
		int priority;

		Channel channels[3];
	} _sfx[3];

	SfxSlot *allocateSfxSlot(int priority);
	bool startSfx(SfxSlot *sfx, const byte *resource);
	void stopSfx(SfxSlot *sfx);

	void updateSfx();
	void clearChannel(const Channel &channel);
	void updateChannel(Channel *channel);
	void parseSlot(Channel *channel);
	void updateSlot(Channel *channel);
	void parseNote(Note *note, const Channel &channel, const byte *offset);
	bool processNote(Note *note, const Channel &channel, const byte *offset);
	void noteOffOn(int channel);
	void writeRegisterSpecial(int channel, uint8 value, int offset);
	uint8 readRegisterSpecial(int channel, uint8 defaultValue, int offset);
	void setupNoteEnvelopeState(Note *note, int steps, int adjust);
	bool processNoteEnvelope(Note *note);

	int _sfxTimer;

	uint8 _rndSeed;
	uint8 getRnd();

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
