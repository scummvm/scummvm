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

// see if all engines using this class are DISABLED
#if !defined(ENABLE_KYRA)

// normal Header Guard
#elif !defined SOUND_MODS_MAXTRAX_H
#define SOUND_MODS_MAXTRAX_H

#include "sound/mods/paula.h"

namespace Audio {

class MaxTrax : public Paula {
public:
	MaxTrax(int rate, bool stereo);
	virtual ~MaxTrax();

	bool load(Common::SeekableReadStream &musicData, bool loadScores = true, bool loadSamples = true);
	bool playSong(int songIndex, bool loop = false);
	void advanceSong(int advance = 1);
	int playNote(byte note, byte patch, uint16 duration, uint16 volume, bool rightSide);
	void setVolume(const byte volume) { _playerCtx.volume = volume; }
	void setTempo(const uint16 tempo) {
		_playerCtx.tickUnit = (int32)(((uint32)(tempo & 0xFFF0) << 8) / (uint16)(5 * _playerCtx.vBlankFreq));
	}
	void stopMusic();

protected:
	void interrupt();

private:
	enum { kNumPatches = 64, kNumVoices = 4, kNumChannels = 16, kNumExtraChannels = 1 };
	enum { kPriorityScore, kPriorityNote, kPrioritySound };
//	int16	_microtonal[128];

	struct Event {
		uint16	startTime;
		uint16	stopTime;
		byte	command;
		byte	parameter;
	};

	struct Score {
		const Event	*events;
		uint32	numEvents;
	} *_scores;

	int _numScores;

	struct {
		uint16	vBlankFreq;
		int32	ticks;
		int32	tickUnit;
		uint16	frameUnit;

		uint16	maxScoreNum;
		uint16	tempo;
		uint16	tempoInitial;
		uint16	tempoStart;
		int16	tempoDelta;
		int32	tempoTime;
		int32	tempoTicks;

		byte	volume;

		bool	filterOn;
		bool	handleVolume;
		bool	musicPlaying;
		bool	musicLoop;

		int		scoreIndex;
		const Event	*nextEvent;
		int32	nextEventTime;
	} _playerCtx;

	struct Envelope {
		uint16	duration;
		uint16	volume;
	};

	const struct Patch {
		const Envelope *attackPtr;
		//Envelope *releasePtr;
		uint16	attackLen;
		uint16	releaseLen;

		int16	tune;
		uint16	volume;

		// this was the SampleData struct in the assembler source
		const int8	*samplePtr;
		uint32	sampleTotalLen;
		uint32	sampleAttackLen;
		uint16	sampleOctaves;
	} _patch[kNumPatches];

	struct ChannelContext {
		const Patch	*patch;
		uint16	regParamNumber;

		uint16	modulation;
		uint16	modulationTime;

		int16	microtonal;

		uint16	portamentoTime;

		int16	pitchBend;
		int16	pitchReal;
		int8	pitchBendRange;

		uint8	volume;
		uint8	voicesActive;

		enum {
			kFlagRightChannel = 1 << 0,
			kFlagPortamento = 1 << 1,
			kFlagDamper = 1 << 2,
			kFlagMono = 1 << 3,
			kFlagMicrotonal = 1 << 4,
			kFlagModVolume = 1 << 5
		};
		byte	flags;
		bool	isAltered;

		uint8	lastNote;
//		uint8	program;

	} _channelCtx[kNumChannels + kNumExtraChannels];

	struct VoiceContext {
		ChannelContext *channel;
		const Patch	*patch;
		const Envelope *envelope;
//		uint32	uinqueId;
		int32	preCalcNote;
		uint32	ticksLeft;
		int32	portaTicks;
		int32	incrVolume;
//		int32	periodOffset;
		/*ifne FASTSOUND
			APTR	voice_CurFastIOB			; current fast iob playing
			APTR	voice_NextFastIOB			; next fast iob to play
			APTR	voice_FastBuffer			; pointer to buffer area
		endc*/
		uint16	envelopeLeft;
		uint16	noteVolume;
		uint16	baseVolume;
		uint16	lastPeriod;
		byte	baseNote;
		byte	endNote;
		byte	octave;
//		byte	number;
//		byte	link;
		byte	priority;
		enum {
			kStatusFree,
			kStatusHalt,
			kStatusDecay,
			kStatusRelease,
			kStatusSustain,
			kStatusAttack,
			kStatusStart
		};
		byte	status;
		bool	hasDamper;
		bool	isBlocked;
		byte	lastVolume;
		bool	hasPortamento;
		byte	dmaOff;

		int32	stopEventTime;
		byte	stopEventCommand;	// TODO: Remove?
		byte	stopEventParameter;	// TODO: Remove?
	} _voiceCtx[kNumVoices];

	void freePatches();
	void freeScores();
	void resetChannel(ChannelContext &chan, bool rightChannel);

	static int8 pickvoice(const VoiceContext voice[4], uint pick, int16 pri);
	static uint16 calcNote(const VoiceContext &voice);
	int8 noteOn(ChannelContext &channel, byte note, uint16 volume, uint16 pri);
	void noteOff(VoiceContext &voice, byte note);
	void killVoice(byte num);

	static void outPutEvent(const Event &ev, int num = -1);
	static void outPutScore(const Score &sc, int num = -1);
};
}	// End of namespace Audio

#endif // !defined SOUND_MODS_MAXTRAX_H
