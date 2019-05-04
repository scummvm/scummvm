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
 * LGPL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef KYRA_SOUND_ADLIBDRIVER_H
#define KYRA_SOUND_ADLIBDRIVER_H

#include "kyra/resource/resource.h"
#include "common/mutex.h"

// Basic AdLib Programming:
// https://web.archive.org/web/20050322080425/http://www.gamedev.net/reference/articles/article446.asp


namespace Audio {
class Mixer;
}

namespace OPL {
class OPL;
}

namespace Kyra {

class AdLibDriver {
public:
	AdLibDriver(Audio::Mixer *mixer, int version);
	~AdLibDriver();

	void initDriver();
	void setSoundData(uint8 *data, uint32 size);
	void queueTrack(int track, int volume);
	bool isChannelPlaying(int channel) const;
	void stopAllChannels();
	int getSoundTrigger() const { return _soundTrigger; }
	void resetSoundTrigger() { _soundTrigger = 0; }

	void callback();

	void setSyncJumpMask(uint16 mask) { _syncJumpMask = mask; }

	void setMusicVolume(uint8 volume);
	void setSfxVolume(uint8 volume);

private:
	// These variables have not yet been named, but some of them are partly
	// known nevertheless:
	//
	// pitchBend - Sound-related. Possibly some sort of pitch bend.
	// unk18 - Sound-effect. Used for secondaryEffect1()
	// unk19 - Sound-effect. Used for secondaryEffect1()
	// unk20 - Sound-effect. Used for secondaryEffect1()
	// unk21 - Sound-effect. Used for secondaryEffect1()
	// unk22 - Sound-effect. Used for secondaryEffect1()
	// unk29 - Sound-effect. Used for primaryEffect1()
	// unk30 - Sound-effect. Used for primaryEffect1()
	// unk31 - Sound-effect. Used for primaryEffect1()
	// unk32 - Sound-effect. Used for primaryEffect2()
	// unk33 - Sound-effect. Used for primaryEffect2()
	// unk34 - Sound-effect. Used for primaryEffect2()
	// unk35 - Sound-effect. Used for primaryEffect2()
	// unk36 - Sound-effect. Used for primaryEffect2()
	// unk37 - Sound-effect. Used for primaryEffect2()
	// unk38 - Sound-effect. Used for primaryEffect2()
	// unk39 - Currently unused, except for updateCallback56()
	// unk40 - Currently unused, except for updateCallback56()
	// unk41 - Sound-effect. Used for primaryEffect2()

	struct Channel {
		bool lock;	// New to ScummVM
		uint8 opExtraLevel2;
		const uint8 *dataptr;
		uint8 duration;
		uint8 repeatCounter;
		int8 baseOctave;
		uint8 priority;
		uint8 dataptrStackPos;
		const uint8 *dataptrStack[4];
		int8 baseNote;
		uint8 unk29;
		uint8 unk31;
		uint16 unk30;
		uint16 unk37;
		uint8 unk33;
		uint8 unk34;
		uint8 unk35;
		uint8 unk36;
		uint8 unk32;
		uint8 unk41;
		uint8 unk38;
		uint8 opExtraLevel1;
		uint8 spacing2;
		uint8 baseFreq;
		uint8 tempo;
		uint8 position;
		uint8 regAx;
		uint8 regBx;
		typedef void (AdLibDriver::*Callback)(Channel&);
		Callback primaryEffect;
		Callback secondaryEffect;
		uint8 fractionalSpacing;
		uint8 opLevel1;
		uint8 opLevel2;
		uint8 opExtraLevel3;
		uint8 twoChan;
		uint8 unk39;
		uint8 unk40;
		uint8 spacing1;
		uint8 durationRandomness;
		uint8 unk19;
		uint8 unk18;
		int8 unk20;
		int8 unk21;
		uint8 unk22;
		uint16 offset;
		uint8 tempoReset;
		uint8 rawNote;
		int8 pitchBend;
		uint8 volumeModifier;
	};

	void primaryEffect1(Channel &channel);
	void primaryEffect2(Channel &channel);
	void secondaryEffect1(Channel &channel);

	void resetAdLibState();
	void writeOPL(byte reg, byte val);
	void initChannel(Channel &channel);
	void noteOff(Channel &channel);
	void unkOutput2(uint8 num);

	uint16 getRandomNr();
	void setupDuration(uint8 duration, Channel &channel);

	void setupNote(uint8 rawNote, Channel &channel, bool flag = false);
	void setupInstrument(uint8 regOffset, const uint8 *dataptr, Channel &channel);
	void noteOn(Channel &channel);

	void adjustVolume(Channel &channel);

	uint8 calculateOpLevel1(Channel &channel);
	uint8 calculateOpLevel2(Channel &channel);

	uint16 checkValue(int16 val) {
		if (val < 0)
			val = 0;
		else if (val > 0x3F)
			val = 0x3F;
		return val;
	}

	// The sound data has at least two lookup tables:
	//
	// * One for programs, starting at offset 0.
	// * One for instruments, starting at offset 500.

	uint8 *getProgram(int progId) {
		const uint16 offset = READ_LE_UINT16(_soundData + 2 * progId);

		// In case an invalid offset is specified we return nullptr to
		// indicate an error. 0xFFFF seems to indicate "this is not a valid
		// program/instrument". However, 0 is also invalid because it points
		// inside the offset table itself. We also ignore any offsets outside
		// of the actual data size.
		// The original does not contain any safety checks and will simply
		// read outside of the valid sound data in case an invalid offset is
		// encountered.
		if (offset == 0 || offset >= _soundDataSize) {
			return nullptr;
		} else {
			return _soundData + offset;
		}
	}

	const uint8 *getInstrument(int instrumentId) {
		return getProgram(_numPrograms + instrumentId);
	}

	void setupPrograms();
	void executePrograms();

	struct ParserOpcode {
		typedef int (AdLibDriver::*POpcode)(const uint8 *&dataptr, Channel &channel, uint8 value);
		POpcode function;
		const char *name;
	};

	void setupParserOpcodeTable();
	const ParserOpcode *_parserOpcodeTable;
	int _parserOpcodeTableSize;

	int update_setRepeat(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_checkRepeat(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupProgram(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setNoteSpacing(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_jump(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_jumpToSubroutine(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_returnFromSubroutine(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseOctave(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_stopChannel(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playRest(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_writeAdLib(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupNoteAndDuration(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseNote(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupSecondaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_stopOtherChannel(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_waitForEndOfProgram(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupInstrument(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupPrimaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removePrimaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseFreq(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupPrimaryEffect2(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setPriority(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback23(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback24(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupDuration(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playNote(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setFractionalNoteSpacing(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removeSecondaryEffect1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setChannelTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel3(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel2(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeExtraLevel2(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setAMDepth(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setVibratoDepth(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeExtraLevel1(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback38(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback39(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removePrimaryEffect2(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_pitchBend(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_resetToGlobalTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_nop(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setDurationRandomness(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeChannelTempo(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback46(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removeRhythmSection(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback51(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback52(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback53(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setSoundTrigger(const uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setTempoReset(const uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback56(const uint8 *&dataptr, Channel &channel, uint8 value);
private:
	// These variables have not yet been named, but some of them are partly
	// known nevertheless:
	//
	// _unkValue1      - Unknown. Used for updating _unkValue2
	// _unkValue2      - Unknown. Used for updating _unkValue4
	// _unkValue4      - Unknown. Used for updating _unkValue5
	// _unkValue5      - Unknown. Used for controlling updateCallback24().
	// _unkValue6      - Unknown. Rhythm section volume?
	// _unkValue7      - Unknown. Rhythm section volume?
	// _unkValue8      - Unknown. Rhythm section volume?
	// _unkValue9      - Unknown. Rhythm section volume?
	// _unkValue10     - Unknown. Rhythm section volume?
	// _unkValue11     - Unknown. Rhythm section volume?
	// _unkValue12     - Unknown. Rhythm section volume?
	// _unkValue13     - Unknown. Rhythm section volume?
	// _unkValue14     - Unknown. Rhythm section volume?
	// _unkValue15     - Unknown. Rhythm section volume?
	// _unkValue16     - Unknown. Rhythm section volume?
	// _unkValue17     - Unknown. Rhythm section volume?
	// _unkValue18     - Unknown. Rhythm section volume?
	// _unkValue19     - Unknown. Rhythm section volume?
	// _unkValue20     - Unknown. Rhythm section volume?
	// _freqTable[]     - Probably frequences for the 12-tone scale.
	// _unkTable2[]    - Unknown. Currently only used by updateCallback46()
	// _unkTable2_1[]  - One of the tables in _unkTable2[]
	// _unkTable2_2[]  - One of the tables in _unkTable2[]
	// _unkTable2_3[]  - One of the tables in _unkTable2[]

	int _curChannel;
	uint8 _soundTrigger;

	uint16 _rnd;

	uint8 _unkValue1;
	uint8 _unkValue2;
	uint8 _callbackTimer;
	uint8 _unkValue4;
	uint8 _unkValue5;
	uint8 _unkValue6;
	uint8 _unkValue7;
	uint8 _unkValue8;
	uint8 _unkValue9;
	uint8 _unkValue10;
	uint8 _unkValue11;
	uint8 _unkValue12;
	uint8 _unkValue13;
	uint8 _unkValue14;
	uint8 _unkValue15;
	uint8 _unkValue16;
	uint8 _unkValue17;
	uint8 _unkValue18;
	uint8 _unkValue19;
	uint8 _unkValue20;

	OPL::OPL *_adlib;

	uint8 *_soundData;
	uint32 _soundDataSize;

	struct QueueEntry {
		QueueEntry() : data(0), id(0), volume(0) {}
		QueueEntry(uint8 *ptr, uint8 track, uint8 vol) : data(ptr), id(track), volume(vol) {}
		uint8 *data;
		uint8 id;
		uint8 volume;
	};

	QueueEntry _programQueue[16];
	int _programStartTimeout;
	int _programQueueStart, _programQueueEnd;
	bool _retrySounds;

	void adjustSfxData(uint8 *data, int volume);
	uint8 *_sfxPointer;
	int _sfxPriority;
	int _sfxVelocity;

	Channel _channels[10];

	uint8 _vibratoAndAMDepthBits;
	uint8 _rhythmSectionBits;

	uint8 _curRegOffset;
	uint8 _tempo;

	const uint8 *_tablePtr1;
	const uint8 *_tablePtr2;

	static const uint8 _regOffset[];
	static const uint16 _freqTable[];
	static const uint8 *const _unkTable2[];
	static const uint8 _unkTable2_1[];
	static const uint8 _unkTable2_2[];
	static const uint8 _unkTable2_3[];
	static const uint8 _pitchBendTables[][32];

	uint16 _syncJumpMask;

	Common::Mutex _mutex;
	Audio::Mixer *_mixer;

	uint8 _musicVolume, _sfxVolume;

	int _numPrograms;
	int _version;
};

} // End of namespace Kyra

#endif
