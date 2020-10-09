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

#ifndef TOWNS_PC98_FMSYNTH_H
#define TOWNS_PC98_FMSYNTH_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/mutex.h"
#include "common/func.h"
#include "common/array.h"

#ifdef __DS__
/* This disables the rhythm channel when emulating the PC-98 type 86 sound card.
 * The only purpose is code size reduction for certain backends.
 * At the moment the only games which make use of the rhythm channel are the
 * (very rare) PC-98 versions of Legend of Kyrandia 2 and Lands of Lore. Music will
 * still be okay, just missing a couple of rhythm instruments.
 */
#define		DISABLE_PC98_RHYTHM_CHANNEL
#endif

/* Experimental code for emulation of the chip's busy flag wait cycle.
 * Explanation:
 * Before attempting a port write a client application would usually read the chip's
 * busy flag and remain in a loop until the flag is cleared. This does not work with
 * an emulator that is on the same thread as the client code (the busy flag will never
 * clear). Instead, I emulate a wait cycle by withholding (enqueueing) incoming register
 * writes for the duration of the wait cycle.
 * For now I have disabled this with an #ifdef since I haven't seen any impact on the
 * sound.
 */
//#define		ENABLE_SNDTOWNS98_WAITCYCLES

class TownsPC98_FmSynthOperator;
class TownsPC98_FmSynthSquareWaveSource;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
class TownsPC98_FmSynthPercussionSource;
#endif

enum EnvelopeState {
	kEnvReady = 0,
	kEnvAttacking,
	kEnvDecaying,
	kEnvSustaining,
	kEnvReleasing
};

class TownsPC98_FmSynth : public Audio::AudioStream {
public:
	enum EmuType {
		kTypeTowns = 0,
		kType26 = 1,
		kType86 = 2
	};

	TownsPC98_FmSynth(Audio::Mixer *mixer, EmuType type);
	virtual ~TownsPC98_FmSynth();

	virtual bool init();
	virtual void reset();

	void writeReg(uint8 part, uint8 regAddress, uint8 value);
	uint8 readReg(uint8 part, uint8 regAddress);

	// AudioStream interface
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const;
	bool endOfData() const;
	int getRate() const;

protected:
	void deinit();

	// Implement this in your inherited class if your driver generates
	// additional output that has to be inserted into the buffer.
	virtual void nextTickEx(int32 *buffer, uint32 bufferSize) {}

	virtual void timerCallbackA() = 0;
	virtual void timerCallbackB() = 0;

	// The audio driver can store and apply two different volume settings
	// (usually for music and sound effects). The channel mask will determine
	// which channels get effected by which setting. The first bits will be
	// the normal fm channels, the next bits the ssg channels and the final
	// bit the rhythm channel.
	void setVolumeIntern(int volA, int volB);
	void setVolumeChannelMasks(int channelMaskA, int channelMaskB);

	// This allows to balance out the fm/ssg levels.
	void setLevelSSG(int vol);

	const int _numChan;
	const int _numSSG;
	const bool _hasPercussion;

	Common::Mutex _mutex;
	int _mixerThreadLockCounter;

private:
	void generateTables();
	void writeRegInternal(uint8 part, uint8 regAddress, uint8 value);
	void nextTick(int32 *buffer, uint32 bufferSize);

#ifdef ENABLE_SNDTOWNS98_WAITCYCLES
	void startWaitCycle();
#endif

	struct ChanInternal {
		ChanInternal();
		~ChanInternal();

		void ampModSensitivity(uint32 value) {
			ampModSvty = (1 << (3 - value)) - (((value >> 1) & 1) | (value & 1));
		}
		void frqModSensitivity(uint32 value) {
			frqModSvty = value << 5;
		}
		void fbClear() {
			feedbuf[0] = feedbuf[1] = feedbuf[2] = 0;
		}

		bool enableLeft;
		bool enableRight;
		bool updateEnvelopeParameters;
		int32 feedbuf[3];
		uint8 algorithm;

		uint32 ampModSvty;
		uint32 frqModSvty;

		TownsPC98_FmSynthOperator *opr[4];
	};

	TownsPC98_FmSynthSquareWaveSource *_ssg;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	TownsPC98_FmSynthPercussionSource *_prc;
#endif
	ChanInternal *_chanInternal;

	uint8 *_oprRates;
	uint8 *_oprRateshift;
	uint8 *_oprAttackDecay;
	uint32 *_oprFrq;
	uint32 *_oprSinTbl;
	int32 *_oprLevelOut;
	int32 *_oprDetune;

	typedef Common::Functor0Mem<void, TownsPC98_FmSynth> ChipTimerProc;
	ChipTimerProc *_timerProcIdle;
	ChipTimerProc *_timerProcA;
	ChipTimerProc *_timerProcB;
	void idleTimerCallback() {}

	struct ChipTimer {
		bool enabled;
		uint16 value;

		int32 smpTillCb;
		uint32 smpTillCbRem;
		int32 smpPerCb;
		uint32 smpPerCbRem;

		ChipTimerProc *cb;
	};

	ChipTimer _timers[2];

	int _volMaskA, _volMaskB;
	uint16 _volumeA, _volumeB;

	int32 *_renderBuffer;
	int _renderBufferSize;
	int _numPending;
	int _offsPending;
	int _rateScale;
	int _outRateMult;
	int _rateConvCnt;
	float _predSmpCount;
	const int _internalRate;
	const int _outputRate;

#ifdef	ENABLE_SNDTOWNS98_WAITCYCLES
	int _waitCycleRemainder;
	const int _samplesPerWaitCycle;

	struct RegEntry {
		RegEntry(uint8 p, uint8 r, uint8 v) : part(p), reg(r), val(v) {}
		uint8 part;
		uint8 reg;
		uint8 val;
	};

	Common::Array<RegEntry> _waitCycleElapsedWrites;
#endif

	uint8 _registers[255][2];

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	static const uint8 _percussionData[];
#endif
	static const uint32 _adtStat[];
	static const uint8 _detSrc[];
	static const int _ssgTables[];

	bool _ready;
};

#endif
