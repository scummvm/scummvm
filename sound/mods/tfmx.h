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

#ifndef SOUND_MODS_TFMX_H
#define SOUND_MODS_TFMX_H

#include "sound/mods/paula.h"

namespace {
#ifndef NDEBUG
inline void boundaryCheck(const void *bufStart, size_t bufLen, const void *address, size_t accessLen = 1) {
	assert(bufStart <= address && (const byte *)address + accessLen <= (const byte *)bufStart + bufLen);
}
#else
inline void boundaryCheck(const void *, size_t, void *, size_t = 1) {}
#endif
}

namespace Audio {

class Tfmx : public Paula {
public:
	Tfmx(int rate, bool stereo);
	virtual ~Tfmx();

	void interrupt();
	void doSong(int songPos);
	void doMacro(int macro, int note);
	bool load(Common::SeekableReadStream &musicData, Common::SeekableReadStream &sampleData);
	int getTicks() {return _playerCtx.tickCount;}

// Note: everythings public so the debug-Routines work.
// private:
	enum {kPalDefaultCiaVal = 11822, kNtscDefaultCiaVal = 14320, kCiaBaseInterval = 0x1B51F8};
	enum {kNumVoices = 4, kNumChannels = 8, kNumSubsongs = 32, kMaxPatternOffsets = 128, kMaxMacroOffsets = 128};

	static const uint16 noteIntervalls[64];

	struct Resource {
		uint32 _trackstepOffset;	//!< Offset in mdat

		byte *_mdatData;  	//!< Currently the whole mdat-File
		byte *_sampleData;	//!< Currently the whole sample-File
		
		uint32 _mdatLen;  
		uint32 _sampleLen;

		byte header[10];
		uint16 headerFlags;
		uint32 headerUnknown;
		char textField[6 * 40];

		const uint16 *getTrackPtr(uint16 trackstep = 0) {
			uint16 *trackData = (uint16 *)(_mdatData + _trackstepOffset + 16 * trackstep);

			boundaryCheck(_mdatData, _mdatLen, trackData, 16);
			return trackData;
		}

		const uint32 *getPatternPtr(uint32 offset) {
			uint32 *pattData = (uint32 *)(_mdatData + offset);

			boundaryCheck(_mdatData, _mdatLen, pattData, 4);
			return pattData;
		}

		const uint32 *getMacroPtr(uint32 offset) {
			uint32 *macroData = (uint32 *)(_mdatData + offset);

			boundaryCheck(_mdatData, _mdatLen, macroData, 4);
			return macroData;
		}

		const int8 *getSamplePtr(const uint32 offset) {
			int8 *sampleData = (int8 *)(_sampleData + offset);

			boundaryCheck(_sampleData, _sampleLen, sampleData, 2);
			return sampleData;
		}
		Resource() : _mdatData(), _mdatLen(), _sampleData(), _sampleLen() {}

		~Resource() {
			delete[] _mdatData;
			delete[] _sampleData;
		}
	} _resource;

	uint32 _patternOffset[kMaxPatternOffsets];	//!< Offset in mdat
	uint32 _macroOffset[kMaxMacroOffsets];	//!< Offset in mdat

	struct Subsong {
		uint16 songstart;	//!< Index in Trackstep-Table
		uint16 songend;		//!< Last index in Trackstep-Table
		uint16 tempo;
	} _subsong[kNumSubsongs];

	struct ChannelContext {
		byte	paulaChannel;

		byte	macroIndex;
		uint16	macroWait;
		uint32	macroOffset;
		uint32	macroReturnOffset;
		uint16	macroStep;
		uint16	macroReturnStep;
		uint8	macroLoopCount;
		bool	macroRun;

		bool	sfxLocked;
		int16	sfxLockTime;
		bool	keyUp;

		bool	deferWait;
		uint16	dmaIntCount;

		uint32	sampleStart;
		uint16	sampleLen;
		uint16	refPeriod;
		uint16	period;

		int8	volume;
		uint8	relVol;
		uint8	note;
		uint8	prevNote;
		int16	fineTune;

		uint8	portaSkip;
		uint8	portaCount;
		uint16	portaDelta;
		uint16	portaValue;

		uint8	envSkip;
		uint8	envCount;
		uint8	envDelta;
		int8	envEndVolume;

		uint8	vibLength;
		uint8	vibCount;
		int16	vibValue;
		int8	vibDelta;

		uint8	addBeginTime;
		uint8	addBeginReset;
		int16	addBeginDelta;
	} _channelCtx[kNumVoices];

	struct PatternContext {
		uint32	offset; // patternStart, Offset from mdat
		uint32	savedOffset;	// for subroutine calls
		uint16	step;	// distance from patternStart
		uint16	savedStep;

		uint8	command;
		int8	expose; 
		uint8	loopCount;
		uint8	wait;	//!< how many ticks to wait before next Command
	} _patternCtx[kNumChannels];

	struct TrackStepContext {
		uint16	startInd;
		uint16	stopInd;
		uint16	posInd;
		int16	loopCount;
	} _trackCtx;

	struct PlayerContext {
		bool	enabled;
//		bool	end;
		int8	song;	//!< >= 0 if Song is running (means process Patterns)

		bool	pendingTrackstep;

		uint16	patternCount;
		uint16	patternSkip;	//!< skip that amount of CIA-Interrupts

		int8	volume;	//!< Master Volume

/*		int8	fadeDest;
		int8	fadeTime;
		int8	fadeReset;
		int8	fadeSlope; */

		int		tickCount;
	} _playerCtx;

	void initMacroProgramm(ChannelContext &channel) {
		channel.macroStep = 0;
		channel.macroWait = 0;
		channel.macroRun = true;
		channel.macroLoopCount = 0xFF;
		channel.dmaIntCount = 0;
	}

	void clearEffects(ChannelContext &channel) {
		channel.envSkip = 0;

		channel.vibLength = 0;

		channel.portaDelta = 0;
	}

	void stopChannel(ChannelContext &channel) {
		if (!channel.sfxLocked) {
			channel.macroRun = false;
			channel.dmaIntCount = 0;
			Paula::disableChannel(channel.paulaChannel);
		} 
	}

	void setNoteMacro(ChannelContext &channel, uint note, int fineTune) {
		const uint16 noteInt = noteIntervalls[note & 0x3F];
		const uint16 finetune = (uint16)(fineTune + channel.fineTune + (1 << 8));
		channel.refPeriod = ((uint32)noteInt * finetune >> 8);
		if (!channel.portaDelta) {
			channel.period = channel.refPeriod;
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
		}
	}

	void effects(ChannelContext &channel);
	FORCEINLINE bool macroStep(ChannelContext &channel);
	void advancePatterns();
	FORCEINLINE bool patternStep(PatternContext &pattern);
	bool trackStep();
	void noteCommand(uint8 note, uint8 param1, uint8 param2, uint8 param3);
};

}

#endif
