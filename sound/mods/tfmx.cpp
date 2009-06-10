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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/debug.h"

#include "sound/mods/tfmx.h"

#include "tfmx/tfmxdebug.h"

namespace Audio {

const uint16 Tfmx::noteIntervalls[64] = {
	1710, 1614, 1524, 1438, 1357, 1281, 1209, 1141, 1077, 1017,  960,  908,
	 856,  810,  764,  720,  680,  642,  606,  571,  539,  509,  480,  454,
	 428,  404,  381,  360,  340,  320,  303,  286,  270,  254,  240,  227,
	 214,  202,  191,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 214,  202,  191,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 214,  202,  191,  180 };



Tfmx::Tfmx(int rate, bool stereo)
: Paula(stereo, rate), _resource()  {
	_playerCtx.enabled = false;
	_playerCtx.song = -1;

	for (int i = 0; i < kNumVoices; ++i) 
		_channelCtx[i].paulaChannel = (byte)i;
}

Tfmx::~Tfmx() {
}

void Tfmx::interrupt() {
	assert(!_end);
	for (int i = 0; i < kNumVoices; ++i) {
		ChannelContext &channel = _channelCtx[i];

		if (channel.dmaIntCount) {
			// wait for DMA Interupts to happen
			int doneDma = getChannelDmaCount(channel.paulaChannel);
			if (doneDma >= channel.dmaIntCount) {
				debug("channel %d, DMA done", i);
				channel.dmaIntCount = 0;
				channel.macroRun = true;
			}
		}

		// TODO: Sometimes a macro will be executed here

		// apply timebased effects on Parameters 
		effects(channel);

		// see if we have to run the macro-program
		if (channel.macroRun) {
			if (!channel.macroWait) {
				// run macro
				while (macroStep(channel))
					;
			} else
				--channel.macroWait;
		}

		// TODO: handling pending DMAOff?

		// set volume after macros were run.
		uint8 finVol = _playerCtx.volume * channel.volume >> 6;
		Paula::setChannelVolume(channel.paulaChannel, finVol);
	}

	// Patterns are only processed each _playerCtx.timerCount + 1 tick
	if (_playerCtx.song >= 0 && !_playerCtx.patternCount--) {
		_playerCtx.patternCount = _playerCtx.patternSkip;
		advancePatterns();
	}
}

void Tfmx::effects(ChannelContext &channel) {
	if (channel.sfxLockTime >= 0)
		--channel.sfxLockTime;
	else
		channel.sfxLocked = false;

	// addBegin

	// TODO: macroNote pending?
	if (0) {
		channel.sfxLocked = false;
		// TODO: macronote
	}

	// vibrato
	if (channel.vibLength) {
		channel.vibValue += channel.vibDelta;
		if (--channel.vibCount == 0) {
			channel.vibCount = channel.vibLength;
			channel.vibDelta = -channel.vibDelta;
		}
		if (!channel.portaDelta) {
			// 16x16 bit multiplication, casts needed for the right results
			channel.period = (uint16)(((uint32)channel.refPeriod * (uint16)((1 << 11) + channel.vibValue)) >> 11);
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
		}
	}

	// portamento
	if (channel.portaDelta && !(--channel.portaCount)) {
		channel.portaCount = channel.portaSkip;

		bool resetPorta = true;
		uint16 period = channel.refPeriod;
		const uint16 portaVal = channel.portaValue;

		if (period > portaVal) {
			period = ((uint32)period * (uint16)((1 << 8) - channel.portaDelta)) >> 8;
			resetPorta = (period <= portaVal);

		} else if (period < portaVal) {
			period = ((uint32)period * (uint16)((1 << 8) + channel.portaDelta)) >> 8;
			resetPorta = (period >= portaVal);
		}

		if (resetPorta) {
			channel.portaDelta = 0;
			channel.portaValue = channel.refPeriod & 0x7FF;
		} else {
			channel.period = period & 0x7FF;
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
		}
	}

	// envelope
	if (channel.envSkip && !channel.envCount--) {
		channel.envCount = channel.envSkip;

		const int8 endVol = channel.envEndVolume;
		int8 volume = channel.volume;
		bool resetEnv = true;

		if (endVol > volume) {
			volume += channel.envDelta;
			resetEnv = endVol <= volume;
		} else {
			volume -= channel.envDelta;
			resetEnv = volume <= 0 || endVol >= volume;
		}

		if (resetEnv) {
			channel.envSkip = 0;
			volume = endVol;
		}
		channel.volume = volume;
	}
	// Fade

	// Volume
}

static void warnMacroUnimplemented(const byte *macroPtr, int level) {
	if (level > 0)
		return;
	if (level == 0)
		debug("Warning - Macro not supported:");
	else
		debug("Warning - Macro not completely supported:");

	displayMacroStep(macroPtr);
}

FORCEINLINE bool Tfmx::macroStep(ChannelContext &channel) {
	const byte *const macroPtr = (byte *)(_resource.getMacroPtr(channel.macroOffset) + channel.macroStep);
	++channel.macroStep;
	//int channelNo = ((byte*)&channel-(byte*)_channelCtx)/sizeof(ChannelContext);

	//displayMacroStep(macroPtr, channel.paulaChannel, channel.macroIndex);

	int32 temp = 0;

	switch (macroPtr[0]) {
	case 0x00:	// Reset + DMA Off. Parameters: deferWait, addset, vol
		clearEffects(channel);
		// FT
	case 0x13:	// DMA Off. Parameters:  deferWait, addset, vol
		// TODO: implement PArameters
		Paula::disableChannel(channel.paulaChannel);
		channel.deferWait = macroPtr[1] >= 1;
		if (channel.deferWait) {
			// if set, then we expect a DMA On in the same tick.
			channel.period = 4;
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
			Paula::setChannelSampleLen(channel.paulaChannel, 1);
			// in this state we then need to allow some commands that normally
			// would halt the macroprogamm to continue instead.
			// those commands are: Wait, WaitDMA, AddPrevNote, AddNote, SetNote, <unknown Cmd>
			// DMA On is affected aswell
			// TODO remember time disabled, remember pending dmaoff?.
		} else {
			//TODO ?
			Paula::disableChannel(channel.paulaChannel);
		}

		if (macroPtr[2])
			channel.volume = macroPtr[3];
		else if (macroPtr[3])
			channel.volume = channel.relVol * 3 + macroPtr[3];
		debug("DMA Off: %02X %02X%02X%02X", macroPtr[0], macroPtr[1], macroPtr[2], macroPtr[3]);
		return true;

	case 0x01:	// DMA On
		channel.dmaIntCount = 0;
		if (channel.deferWait) {
			// TODO
			// there is actually a small delay in the player, but I think that
			// only allows to clear DMA-State on real Hardware
		}
		Paula::enableChannel(channel.paulaChannel);
		channel.deferWait = false;
		return true;

	case 0x02:	// SetBeginn. Parameters: SampleOffset(L)
		channel.sampleStart = READ_BE_UINT32(macroPtr) & 0xFFFFFF;
		Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(channel.sampleStart));
		return true;

	case 0x03:	// SetLength. Parameters: SampleLength(W)
		channel.sampleLen = READ_BE_UINT16(&macroPtr[2]);
		Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
		return true;

	case 0x04:	// Wait. Parameters: Ticks to wait(W).
		// TODO: some unkown Parameter? (macroPtr[1] & 1)
		channel.macroWait = READ_BE_UINT16(&macroPtr[2]);
		return false;

	case 0x10:	// Loop Key Up. Parameters: Loopcount, MacroStep(W)
		if (!channel.keyUp)
			return true;
		// FT
	case 0x05:	// Loop. Parameters: Loopcount, MacroStep(W)
		// debug("Step %d, Loopcount: %02X", channel.macroStep, channel.macroLoopCount);
		if (channel.macroLoopCount != 0) {
			if (channel.macroLoopCount == 0xFF)
				channel.macroLoopCount = macroPtr[1];
			channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
		}
		--channel.macroLoopCount;
		return true;

	case 0x06:	// Jump. Parameters: MacroIndex, MacroStep(W)
		channel.macroIndex = macroPtr[1] % kMaxMacroOffsets;
		channel.macroOffset = _macroOffset[macroPtr[1] % kMaxMacroOffsets];
		channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
		channel.macroLoopCount = 0xFF;
		return true;

	case 0x07:	// Stop Macro
		channel.macroRun = false;
		return false;

	case 0x08:	// AddNote. Parameters: Note, Finetune(W)
		setNoteMacro(channel, channel.note + macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
		return channel.deferWait;

	case 0x09:	// SetNote. Parameters: Note, Finetune(W)
		setNoteMacro(channel, macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
		return channel.deferWait;

	case 0x0A:	// Clear Effects
		clearEffects(channel);
		return true;

	case 0x0B:	// Portamento. Parameters: count, speed
		channel.portaSkip = macroPtr[1];
		channel.portaCount = 1;
		// if porta is already running, then keep using old value
		if (!channel.portaDelta)
			channel.portaValue = channel.refPeriod;
		channel.portaDelta = READ_BE_UINT16(&macroPtr[2]);
		return true;

	case 0x0C:	// Vibrato. Parameters: Speed, intensity
		channel.vibLength = macroPtr[1];
		channel.vibCount = macroPtr[1] / 2;
		channel.vibDelta = macroPtr[3];
		if (!channel.portaDelta) {
			channel.period = channel.refPeriod;
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
			channel.vibValue = 0;
		}
		return true;

	case 0x0D:	// Add Volume. Parameters: note, addNoteFlag, volume
		if (macroPtr[2] == 0xFE)
			setNoteMacro(channel, channel.note + macroPtr[1], 0);
		channel.volume = channel.relVol * 3 + macroPtr[3];
		return true;

	case 0x0E:	// Set Volume. Parameters: note, addNoteFlag, volume
		if (macroPtr[2] == 0xFE)
			setNoteMacro(channel, channel.note + macroPtr[1], 0);
		channel.volume = macroPtr[3];
		return true;

	case 0x0F:	// Envelope. Parameters: speed, count, endvol
		channel.envDelta = macroPtr[1];
		channel.envCount = channel.envSkip = macroPtr[2];
		channel.envEndVolume = macroPtr[3];
		return true;

	case 0x11:	// AddBegin. Parameters: times, Offset(W)
		// TODO: implement Parameter
		macroPtr[1];
//		debug("prev: %06X, after: %06X", channel.sampleStart, channel.sampleStart + (int16)READ_BE_UINT16(&macroPtr[2]));
		channel.sampleStart += (int16)READ_BE_UINT16(&macroPtr[2]);
		Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(channel.sampleStart));
		warnMacroUnimplemented(macroPtr, 1);
		return true;

	case 0x12:	// AddLen. Parameters: added Length(W)
		channel.sampleLen += READ_BE_UINT16(&macroPtr[2]);
		Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
		return true;

	case 0x14:	// Wait key up. Parameters: wait cycles
		if (!channel.keyUp || channel.macroLoopCount == 0) {
			channel.macroLoopCount = 0xFF;
			return true;
		} else if (channel.macroLoopCount == 0xFF)
			channel.macroLoopCount = macroPtr[3];
		--channel.macroLoopCount;
		--channel.macroStep;
		return false;

	case 0x15:	// Subroutine. Parameters: MacroIndex, Macrostep(W)
		channel.macroReturnOffset = channel.macroOffset;
		channel.macroReturnStep = channel.macroStep;

		channel.macroOffset = _macroOffset[macroPtr[1] % kMaxMacroOffsets];
		channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
		// TODO: MI does some weird stuff there. Figure out which varioables need to be set
		return true;

	case 0x16:	// Return from Sub.
		channel.macroOffset = channel.macroReturnOffset;
		channel.macroStep = channel.macroReturnStep;
		return true;

	case 0x17:	// set Period. Parameters: Period(W)
		channel.refPeriod = READ_BE_UINT16(&macroPtr[2]);
		if (!channel.portaDelta) {
			channel.period = channel.refPeriod;
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
		}
		return true;

	case 0x18:	// Sampleloop. Parameters: Offset from Samplestart(W)
		// TODO: MI loads 24 bit, but thats useless?
		temp = READ_BE_UINT16(&macroPtr[2]);
		assert(!(temp & 1));
		channel.sampleStart += temp & 0xFFFE;
		channel.sampleLen -= (uint16)(temp / 2);
		Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(channel.sampleStart));
		Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
		return true;

	case 0x19:	// set one-shot Sample
		channel.sampleStart = 0;
		channel.sampleLen = 1;
		Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(0));
		Paula::setChannelSampleLen(channel.paulaChannel, 1);
		return true;

	case 0x1A:	// Wait on DMA. Parameters: Cycles-1(W) to wait
		channel.dmaIntCount = READ_BE_UINT16(&macroPtr[2]) + 1;
		channel.macroRun = false;
		Paula::setChannelDmaCount(channel.paulaChannel);
		return channel.deferWait;

	case 0x1B:	// Random play. Parameters: macro/speed/mode
		macroPtr[1];
		macroPtr[2];
		macroPtr[3];
		warnMacroUnimplemented(macroPtr, 0);
		return true;

	case 0x1C:	// Branch on Note. Parameters: note/macrostep(W)
		if (channel.note > macroPtr[1])
			channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
		return true;

	case 0x1D:	// Branch on Volume. Parameters: volume/macrostep(W)
		if (channel.volume > macroPtr[1])
			channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
		return true;

	case 0x1E:	// Addvol+note. Parameters: note/CONST./volume
		warnMacroUnimplemented(macroPtr, 0);
		return true;

	case 0x1F:	// AddPrevNote. Parameters: Note, Finetune(W)
		setNoteMacro(channel, channel.prevNote + macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
		return channel.deferWait;

	case 0x20:	// Signal. Parameters: signalnumber/value
		warnMacroUnimplemented(macroPtr, 0);
		return true;

	case 0x21:	// Play macro. Parameters: macro/chan/detune
		warnMacroUnimplemented(macroPtr, 0);
		return true;
#if defined(TFMX_NOT_IMPLEMENTED)
	// used by Gem`X according to the docs
	case 0x22:	// SID setbeg. Parameters: sample-startadress
		return true;
	case 0x23:	// SID setlen. Parameters: buflen/sourcelen 
		return true;
	case 0x24:	// SID op3 ofs. Parameters: offset
		return true;
	case 0x25:	// SID op3 frq. Parameters: speed/amplitude
		return true;
	case 0x26:	// SID op2 ofs. Parameters: offset
		return true;
	case 0x27:	// SID op2 frq. Parameters: speed/amplitude
		return true;
	case 0x28:	// ID op1. Parameters: speed/amplitude/TC
		return true;
	case 0x29:	// SID stop. Parameters: flag (1=clear all)
		return true;
	// 30-34 used by Carribean Disaster
#endif
	default:
		warnMacroUnimplemented(macroPtr, 0);
		return channel.deferWait;
	}
}

void Tfmx::advancePatterns() {
doTrackstep:
	if (_playerCtx.pendingTrackstep) {
		while (trackStep())
			;
		_playerCtx.pendingTrackstep = false;
	}

	for (int i = 0; i < kNumChannels; ++i) {
		assert(!_playerCtx.pendingTrackstep);

		const uint8 pattCmd = _patternCtx[i].command;
		if (pattCmd < 0x90) {	// execute Patternstep
			// FIXME: 0x90 is very likely a bug, 0x80 would make more sense
			assert(pattCmd < 0x80);

			if (_patternCtx[i].wait == 0) {
				// issue all Steps for this tick
				while (patternStep(_patternCtx[i]))
					;
			} else 
				--_patternCtx[i].wait;

		} else if (pattCmd == 0xFE) {	// Stop voice in pattern.expose
			_patternCtx[i].command = 0xFF;
			stopChannel(_channelCtx[_patternCtx[i].expose % kNumVoices]);
		} // else this pattern-Channel is stopped

		if (_playerCtx.pendingTrackstep) {
			// we load the next Trackstep Command and then process all Channels again
			// TODO Optionally disable looping
			if (_trackCtx.posInd == _trackCtx.stopInd)
				_trackCtx.posInd = _trackCtx.startInd;
			else
				++_trackCtx.posInd;
			goto doTrackstep;
		}
	}
}

FORCEINLINE bool Tfmx::patternStep(PatternContext &pattern) {
	const byte *const patternPtr = (byte *)(_resource.getPatternPtr(pattern.offset) + pattern.step);
	++pattern.step;

	debug("Pattern %04X +%d", pattern.offset, pattern.step-1);
	displayPatternstep(patternPtr);

	const byte pattCmd = patternPtr[0];

	if (pattCmd < 0xF0) { // Playnote
		const byte flags = pattCmd >> 6; // 0-1 means note, 2 means wait, 3 means portamento
		byte noteCmd = pattCmd + pattern.expose;
		byte param3  = patternPtr[3];
		if (flags == 2) {
			// Store wait-value in context and delete it the (note)command
			pattern.wait = param3;
			param3 = 0;
		}
		if (flags != 3)
			noteCmd &= 0x3F;
		noteCommand(noteCmd, patternPtr[1], patternPtr[2], param3);
		return (flags != 2);

	} else {	// Patterncommand
		switch (pattCmd & 0xF) {
		case 0: 	// End Pattern + Next Trackstep
			pattern.command = 0xFF;
			_playerCtx.pendingTrackstep = true;
			return false;

		case 1: 	// Loop Pattern. Parameters: Loopcount, PatternStep(W)
			if (pattern.loopCount != 0) {
				if (pattern.loopCount == 0xFF)
					pattern.loopCount = patternPtr[1];
				pattern.step = READ_BE_UINT16(&patternPtr[2]);
			}
			--pattern.loopCount;
			return true;

		case 2: 	// Jump. Parameters: PatternIndex, PatternStep(W)
			pattern.offset = _patternOffset[patternPtr[1]];
			pattern.step = READ_BE_UINT16(&patternPtr[2]);
			return true;

		case 3: 	// Wait. Paramters: ticks to wait
			pattern.wait = patternPtr[1];
			return false;

		case 14: 	// Stop custompattern
			// TODO ?
			// FT
		case 4: 	// Stop this pattern
			pattern.command = 0xFF;
			// TODO: try figuring out if this was the last Channel?
			return false;

		case 5: 	// Kup^-Set key up
			// TODO: add expose?
		case 6: 	// Vibrato
		case 7: 	// Envelope
		case 12: 	// Lock
			noteCommand(pattCmd, patternPtr[1], patternPtr[2], patternPtr[3]);
			return true;

		case 8: 	// Subroutine
			return true;
		case 9: 	// Return from Subroutine
			return true;
		case 10:	// fade master volume
			return true;

		case 11: {	// play pattern. Parameters: patternCmd, channel, expose
			PatternContext &target = _patternCtx[patternPtr[2] % kNumChannels];

			target.command = patternPtr[1];
			target.offset = _patternOffset[patternPtr[1] % kMaxPatternOffsets];
			target.expose = patternPtr[3];
			target.step = 0;
			target.wait = 0;
			target.loopCount = 0xFF;
			}
			return true;
		case 13: 	// Cue
			return true;
		case 15: 	// NOP
			return true;
		}
	}

	return true;
}

bool Tfmx::trackStep() {
	const uint16 *const trackData = _resource.getTrackPtr(_trackCtx.posInd);

	debug( "TStep %04X", _trackCtx.posInd);
	displayTrackstep(trackData);

	if (trackData[0] != FROM_BE_16(0xEFFE)) {
		// 8 commands for Patterns
		for (int i = 0; i < 8; ++i) {
			const uint patCmd = READ_BE_UINT16(&trackData[i]);

			// First byte is pattern number
			const uint patNum = (patCmd >> 8);
			
			// if highest bit is set then keep previous pattern
			if (patNum < 0x80) {
				_patternCtx[i].command = (uint8)patNum;
				_patternCtx[i].step = 0;
				_patternCtx[i].wait = 0;
				_patternCtx[i].loopCount = 0xFF;
				_patternCtx[i].offset = _patternOffset[patNum];
			}

			// second byte expose is always set
			_patternCtx[i].expose = patCmd & 0xFF;
		}
		return false;

	} else {
		// 16 byte Trackstep Command
		int temp;
		switch (READ_BE_UINT16(&trackData[1])) {
		case 0:	// Stop Player. No Parameters
			_playerCtx.enabled = 0;
			stopPaula();
			return false;

		case 1:	// Branch/Loop section of tracksteps. Parameters: branch target, loopcount
			// this depends on the current loopCounter
			temp = _trackCtx.loopCount;
			if (temp > 0) {
				// if trackloop is positive, we decrease one loop and continue at start of loop
				--_trackCtx.loopCount;
				_trackCtx.posInd = READ_BE_UINT16(&trackData[2]);
			} else if (temp == 0) {
				// if trackloop is 0, we reached last iteration and continue with next trackstep
				_trackCtx.loopCount = (uint16)-1;
			} else /*if (_context.TrackLoop < 0)*/ {
				// if TrackLoop is negative then we reached the loop instruction the first time
				// and we setup the Loop
				_trackCtx.posInd    = READ_BE_UINT16(&trackData[2]);
				_trackCtx.loopCount = READ_BE_UINT16(&trackData[3]);
			}
			break;

		case 2:	// Set Tempo. Parameters: tempo, divisor
			_playerCtx.patternCount = _playerCtx.patternSkip = READ_BE_UINT16(&trackData[2]); // tempo
			temp = READ_BE_UINT16(&trackData[3]); // divisor

			if (!(temp & 0x8000) && (temp & 0x1FF))
				setInterruptFreqUnscaled(temp & 0x1FF);
			break;

		case 3:	// Unknown, stops player aswell
		case 4:	// Fade
		default:
			debug("Unknown Command: %02X", READ_BE_UINT16(&trackData[1]));
			// MI-Player handles this by stopping the player, we just continue
		}
		++_trackCtx.posInd;
		return true;
	}
}

void Tfmx::noteCommand(const uint8 note, const uint8 param1, const uint8 param2, const uint8 param3) {
	ChannelContext &channel = _channelCtx[param2 % kNumVoices];

	if (note == 0xFC) { // Lock
		channel.sfxLocked = (param1 != 0);
		channel.sfxLockTime = param3; // only 1 byte read! 
		return;
	}
	if (channel.sfxLocked)
		return;

	if (note < 0xC0) {	// Play Note
		channel.prevNote = channel.note;
		channel.note = note;
		channel.macroIndex = param1 % kMaxMacroOffsets;
		channel.macroOffset = _macroOffset[param1 % kMaxMacroOffsets];
		channel.relVol = (param2 >> 4) & 0xF;
		channel.fineTune = (int8)param3;

		initMacroProgramm(channel);
		channel.keyUp = true;
		
	} else if (note < 0xF0) {	// Portamento
		channel.portaSkip = param1;
		channel.portaCount = 1;
		if (!channel.portaDelta)
			channel.portaValue = channel.refPeriod;
		channel.portaDelta = param3;

		channel.note = note & 0x3F;
		channel.refPeriod = noteIntervalls[channel.note];
	} else switch (note & 0xF) {	// Command
		case 5:	// Key Up Release
			channel.keyUp = false;
			break;
		case 6:	// Vibratio
			channel.vibLength = param1 & 0xFE;
			channel.vibCount = param1 / 2;
			channel.vibValue = 0;
			break;
		case 7:	// Envelope
			channel.envDelta = param1;
			channel.envSkip = channel.envCount = (param2 >> 4) + 1;
			channel.envEndVolume = param3;
			break;
	}
}

bool Tfmx::load(Common::SeekableReadStream &musicData, Common::SeekableReadStream &sampleData) {
	bool res;

	assert(0 == _resource._mdatData);
	assert(0 == _resource._sampleData);

	// TODO: Sanity checks if we have a valid TFMX-Module
	// TODO: check for Stream-Errors (other than using asserts)

	// 0x0000: 10 Bytes Header "TFMX-SONG "
	// 0x000A: int16 ?
	// 0x000C: int32 ?
	musicData.read(_resource.header, 10);
	_resource.headerFlags = musicData.readUint16BE();
	_resource.headerUnknown = musicData.readUint32BE();

	// This might affect timing
	// bool isPalModule = (_resource.headerFlags & 2) != 0;

	// 0x0010: 6*40 Textfield
	musicData.read(_resource.textField, 6 * 40);

	/* 0x0100: Songstart x 32*/
	for (int i = 0; i < kNumSubsongs; ++i)
		_subsong[i].songstart = musicData.readUint16BE();

	/* 0x0140: Songend x 32*/
	for (int i = 0; i < kNumSubsongs; ++i)
		_subsong[i].songend = musicData.readUint16BE();

	/* 0x0180: Tempo x 32*/
	for (int i = 0; i < kNumSubsongs; ++i)
		_subsong[i].tempo  = musicData.readUint16BE();

	/* 0x01c0: unused ? */
	musicData.skip(16);

	/* 0x01d0: trackstep, pattern data p, macro data p */
	uint32 offTrackstep = musicData.readUint32BE();
	uint32 offPatternP = musicData.readUint32BE();
	uint32 offMacroP = musicData.readUint32BE();

	// This is how MI`s TFMX-Player tests for unpacked Modules.
	if (offTrackstep == 0) {
		offTrackstep	= 0x600 + 0x200;
		offPatternP		= 0x200 + 0x200;
		offMacroP		= 0x400 + 0x200;
	}

	_resource._trackstepOffset = offTrackstep;

	// Read in pattern starting offsets
	musicData.seek(offPatternP);
	for (int i = 0; i < kMaxPatternOffsets; ++i)
		_patternOffset[i] = musicData.readUint32BE();

	res = musicData.err();
	assert(!res);

	// Read in macro starting offsets
	musicData.seek(offMacroP);
	for (int i = 0; i < kMaxMacroOffsets; ++i)
		_macroOffset[i] = musicData.readUint32BE();

	res = musicData.err();
	assert(!res);

	// Read in whole mdat-file
	int32 size = musicData.size();
	assert(size != -1);
	// TODO: special routine if size = -1?

	_resource._mdatData = new byte[size];
	assert(_resource._mdatData);
	_resource._mdatLen = size;
	musicData.seek(0);
	musicData.read(_resource._mdatData, size);
	
	res = musicData.err();
	assert(!res);
	musicData.readByte();
	res = musicData.eos();
	assert(res);


	// TODO: It would be possible to analyze the pointers and be able to
	// seperate the file in trackstep, patterns and macro blocks
	// Modules could do weird stuff like having those blocks mixed though.
	// TODO: Analyze pointers if they are correct offsets,
	// so that accesses can be unchecked later

	// Read in whole sample-file
	size = sampleData.size();
	assert(size >= 4);
	assert(size != -1);
	// TODO: special routine if size = -1?

	_resource._sampleData = new byte[size];
	assert(_resource._sampleData);
	_resource._sampleLen = size;
	sampleData.seek(0);
	sampleData.read(_resource._sampleData, size);
	for (int i = 0; i < 4; ++i)
		_resource._sampleData[i] = 0;
	
	res = sampleData.err();
	assert(!res);
	sampleData.readByte();
	res = sampleData.eos();
	assert(res);

	return true;
}


void Tfmx::doMacro(int macro, int note) {
	assert(0 <= macro && macro < kMaxMacroOffsets);
	assert(0 <= note && note < 0xC0);

	_playerCtx.song = -1;
	_playerCtx.volume = 0x40;

	const int channel = 0;
	_channelCtx[channel].sfxLocked = false;
	_channelCtx[channel].note = 0;

	for (int i = 0; i < kNumVoices; ++i) {
		_channelCtx[i].sfxLocked = false;
		_channelCtx[i].sfxLockTime = -1;
		clearEffects(_channelCtx[i]);
		_channelCtx[i].vibValue = 0;
		stopChannel(_channelCtx[i]);
		_channelCtx[i].volume = 0;
	}

	noteCommand((uint8)note, (uint8)macro, (uint8)channel, 0);

	setTimerBaseValue(kPalCiaClock);
	setInterruptFreqUnscaled(kPalDefaultCiaVal);
	startPaula();
}

void Tfmx::doSong(int songPos) {
	assert(0 <= songPos && songPos < kNumSubsongs);

	_playerCtx.song = (int8)songPos;
	_playerCtx.volume = 0x40;

	_trackCtx.loopCount = -1;
	_trackCtx.startInd = _trackCtx.posInd = _subsong[songPos].songstart;
	_trackCtx.stopInd = _subsong[songPos].songend;

	const uint16 tempo = _subsong[songPos].tempo;
	uint16 ciaIntervall;
	
	if (tempo >= 0x10) {
		ciaIntervall = (uint16)(kCiaBaseInterval / tempo);
		_playerCtx.patternSkip = 0;
	} else {
		ciaIntervall = kPalDefaultCiaVal;
		_playerCtx.patternSkip = tempo;
	}

	_playerCtx.patternCount = 0;

	_playerCtx.pendingTrackstep = true;

	for (int i = 0; i < kNumChannels; ++i) {
		_patternCtx[i].command = 0xFF;
		_patternCtx[i].expose = 0;
	}

	for (int i = 0; i < kNumVoices; ++i) {
		_channelCtx[i].sfxLocked = false;
		_channelCtx[i].sfxLockTime = -1;
		clearEffects(_channelCtx[i]);
		_channelCtx[i].vibValue = 0;
		stopChannel(_channelCtx[i]);
		_channelCtx[i].volume = 0;
	}

	setTimerBaseValue(kPalCiaClock);
	setInterruptFreqUnscaled(ciaIntervall);
	startPaula();
}

}
