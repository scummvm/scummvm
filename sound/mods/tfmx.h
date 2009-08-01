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

	void stopSong(bool stopAudio = true);
	void doSong(int songPos, bool stopAudio = false);
	int doSfx(uint16 sfxIndex, bool unlockChannel = false);
	void doMacro(int note, int macro, int relVol = 0, int finetune = 0, int channelNo = 0);
	bool load(Common::SeekableReadStream &musicData, Common::SeekableReadStream &sampleData);
	int getTicks() const { return _playerCtx.tickCount; } 
	int getSongIndex() const { return _playerCtx.song; }
	void setSignalPtr(uint16 *ptr, uint16 numSignals) { _playerCtx.signal = ptr; _playerCtx.numSignals = numSignals; }
	void stopMacroEffect(int channel);

protected:
	void interrupt();

private:
	enum { kPalDefaultCiaVal = 11822, kNtscDefaultCiaVal = 14320, kCiaBaseInterval = 0x1B51F8 };
	enum { kNumVoices = 4, kNumChannels = 8, kNumSubsongs = 32, kMaxPatternOffsets = 128, kMaxMacroOffsets = 128 };

	static const uint16 noteIntervalls[64];

	struct Resource {
		uint32 trackstepOffset;	//!< Offset in mdat
		uint32 sfxTableOffset;

		byte *mdatData;  	//!< Currently the whole mdat-File
		byte *sampleData;	//!< Currently the whole sample-File
		
		uint32 mdatLen;  
		uint32 sampleLen;

		byte header[10];
		uint16 headerFlags;
		uint32 headerUnknown;
		char textField[6 * 40];

		const byte *getSfxPtr(uint16 index = 0) {
			byte *sfxPtr = (byte *)(mdatData + sfxTableOffset + index * 8);

			boundaryCheck(mdatData, mdatLen, sfxPtr, 8);
			return sfxPtr;
		}

		const uint16 *getTrackPtr(uint16 trackstep = 0) {
			uint16 *trackData = (uint16 *)(mdatData + trackstepOffset + 16 * trackstep);

			boundaryCheck(mdatData, mdatLen, trackData, 16);
			return trackData;
		}

		const uint32 *getPatternPtr(uint32 offset) {
			uint32 *pattData = (uint32 *)(mdatData + offset);

			boundaryCheck(mdatData, mdatLen, pattData, 4);
			return pattData;
		}

		const uint32 *getMacroPtr(uint32 offset) {
			uint32 *macroData = (uint32 *)(mdatData + offset);

			boundaryCheck(mdatData, mdatLen, macroData, 4);
			return macroData;
		}

		const int8 *getSamplePtr(const uint32 offset) {
			int8 *sample = (int8 *)(sampleData + offset);

			boundaryCheck(sampleData, sampleLen, sample, 2);
			return sample;
		}
		Resource() : mdatData(), mdatLen(), sampleData(), sampleLen() {}

		~Resource() {
			delete[] mdatData;
			delete[] sampleData;
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
		int8	macroSfxRun;	//!< values are the folowing: -1 macro disabled, 0 macro init, 1 macro running

		uint32	customMacro;
		uint8	customMacroIndex;
		uint8	customMacroPrio;

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
		int16	fineTune; // always a signextended byte

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

		uint8	addBeginLength;
		uint8	addBeginCount;
		int32	addBeginDelta;
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
		int8	song;	//!< >= 0 if Song is running (means process Patterns)

		uint16	patternCount;
		uint16	patternSkip;	//!< skip that amount of CIA-Interrupts

		int8	volume;	//!< Master Volume

		uint8	fadeSkip;
		uint8	fadeCount;
		int8	fadeEndVolume;
		int8	fadeDelta;

		int		tickCount;

		uint16	*signal;
		uint16	numSignals;

		bool	stopWithLastPattern; //!< hack to automatically stop the whole player if no Pattern is running
	} _playerCtx;

	static void initMacroProgramm(ChannelContext &channel) {
		channel.macroStep = 0;
		channel.macroWait = 0;
		channel.macroRun = true;
		channel.macroSfxRun = 0;
		channel.macroLoopCount = 0xFF;
		channel.dmaIntCount = 0;
		channel.deferWait = false;
	}

	static void clearEffects(ChannelContext &channel) {
		channel.addBeginLength = 0;
		channel.envSkip = 0;
		channel.vibLength = 0;
		channel.portaDelta = 0;
	}

	static void clearMacroProgramm(ChannelContext &channel) {
		channel.macroRun = false;
		channel.macroSfxRun = 0;
		channel.dmaIntCount = 0;
	}

	static void unlockMacroChannel(ChannelContext &channel) {
		channel.customMacro = 0;
		channel.customMacroPrio = false;
		channel.sfxLocked = false;
		channel.sfxLockTime = -1;
	}

	void stopPatternChannels() {
		for (int i = 0; i < kNumChannels; ++i) {
			_patternCtx[i].command = 0xFF;
			_patternCtx[i].expose = 0;
		}
	}

	void stopMacroChannels() {
		for (int i = 0; i < kNumVoices; ++i) {
			clearEffects(_channelCtx[i]);
			unlockMacroChannel(_channelCtx[i]);
			clearMacroProgramm(_channelCtx[i]);
			_channelCtx[i].note = 0;
			_channelCtx[i].volume = 0;
		}
	}

	static void setNoteMacro(ChannelContext &channel, uint note, int fineTune) {
		const uint16 noteInt = noteIntervalls[note & 0x3F];
		const uint16 finetune = (uint16)(fineTune + channel.fineTune + (1 << 8));
		channel.refPeriod = ((uint32)noteInt * finetune >> 8);
		if (!channel.portaDelta)
			channel.period = channel.refPeriod;
	}

	void initFadeCommand(const uint8 fadeTempo, const int8 endVol) {
		_playerCtx.fadeCount = _playerCtx.fadeSkip = fadeTempo;
		_playerCtx.fadeEndVolume = endVol;

		if (fadeTempo) {
			const int diff = _playerCtx.fadeEndVolume - _playerCtx.volume;
			_playerCtx.fadeDelta = (diff != 0) ? ((diff > 0) ? 1 : -1) : 0;
		} else {
			_playerCtx.volume = endVol;
			_playerCtx.fadeDelta = 0;
		}
	}

	void effects(ChannelContext &channel);
	void macroRun(ChannelContext &channel);
	void advancePatterns();
	bool patternRun(PatternContext &pattern);
	bool trackRun(bool incStep = false);
	void noteCommand(uint8 note, uint8 param1, uint8 param2, uint8 param3);
};

}	// End of namespace Audio

#endif
