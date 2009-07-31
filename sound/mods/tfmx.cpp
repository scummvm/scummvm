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

// couple debug-functions
namespace {
	void displayPatternstep(const void *const vptr);
	void displayMacroStep(const void *const vptr);
}

namespace Audio {

const uint16 Tfmx::noteIntervalls[64] = {
	1710, 1614, 1524, 1438, 1357, 1281, 1209, 1141, 1077, 1017,  960,  908,
	 856,  810,  764,  720,  680,  642,  606,  571,  539,  509,  480,  454,
	 428,  404,  381,  360,  340,  320,  303,  286,  270,  254,  240,  227,
	 214,  202,  191,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 214,  202,  191,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 214,  202,  191,  180 };

Tfmx::Tfmx(int rate, bool stereo)
: Paula(stereo, rate), _resource(), _playerCtx() {
	_playerCtx.stopWithLastPattern = false;

	for (int i = 0; i < kNumVoices; ++i) 
		_channelCtx[i].paulaChannel = (byte)i;

	_playerCtx.song = -1;
	_playerCtx.volume = 0x40;
	_playerCtx.patternSkip = 6;
	stopPatternChannels();
	stopMacroChannels();

	setTimerBaseValue(kPalCiaClock);
	setInterruptFreqUnscaled(kPalDefaultCiaVal);
}

Tfmx::~Tfmx() {
}

void Tfmx::interrupt() {
	assert(!_end);
	++_playerCtx.tickCount;

	for (int i = 0; i < kNumVoices; ++i) {
		if (_channelCtx[i].dmaIntCount) {
			// wait for DMA Interupts to happen
			int doneDma = getChannelDmaCount(i);
			if (doneDma >= _channelCtx[i].dmaIntCount) {
				_channelCtx[i].dmaIntCount = 0;
				_channelCtx[i].macroRun = true;
			}
		}
	}

	for (int i = 0; i < kNumVoices; ++i) {
		ChannelContext &channel = _channelCtx[i];

		if (channel.sfxLockTime >= 0)
			--channel.sfxLockTime;
		else {
			channel.sfxLocked = false;
			channel.customMacroPrio = 0;
		}

		// externally queued macros
		if (channel.customMacro) {
			const byte *const noteCmd = (const byte *)&channel.customMacro;
			channel.sfxLocked = false;
			noteCommand(noteCmd[0], noteCmd[1], (noteCmd[2] & 0xF0) | (uint8)i, noteCmd[3]);
			channel.customMacro = 0;
			channel.sfxLocked = (channel.customMacroPrio != 0);
		}

		// apply timebased effects on Parameters
		if (channel.macroSfxRun > 0)
			effects(channel);

		// see if we have to run the macro-program
		if (channel.macroRun) {
			if (!channel.macroWait)
				macroRun(channel);
			else
				--channel.macroWait;
		}

		Paula::setChannelPeriod(channel.paulaChannel, channel.period);
		if (channel.macroSfxRun >= 0)
			channel.macroSfxRun = 1;

		// TODO: handling pending DMAOff?
	}

	// Patterns are only processed each _playerCtx.timerCount + 1 tick
	if (_playerCtx.song >= 0 && !_playerCtx.patternCount--) {
		_playerCtx.patternCount = _playerCtx.patternSkip;
		advancePatterns();
	}
}

void Tfmx::effects(ChannelContext &channel) {
	// addBegin
	if (channel.addBeginLength) {
		channel.sampleStart += channel.addBeginDelta;
		Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(channel.sampleStart));
		if (!(--channel.addBeginCount)) {
			channel.addBeginCount = channel.addBeginLength;
			channel.addBeginDelta = -channel.addBeginDelta;
		}
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
		}
	}

	// portamento
	if (channel.portaDelta && --channel.portaCount == 0) {
		channel.portaCount = channel.portaSkip;

		bool resetPorta = true;
		const uint16 period = channel.refPeriod;
		uint16 portaVal = channel.portaValue;

		if (period > portaVal) {
			portaVal = ((uint32)portaVal * (uint16)((1 << 8) + channel.portaDelta)) >> 8;
			resetPorta = (period <= portaVal);

		} else if (period < portaVal) {
			portaVal = ((uint32)portaVal * (uint16)((1 << 8) - channel.portaDelta)) >> 8;
			resetPorta = (period >= portaVal);
		}

		if (resetPorta) {
			channel.portaDelta = 0;
			channel.portaValue = period & 0x7FF;
		} else
			channel.period = channel.portaValue = portaVal & 0x7FF;
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
	if (_playerCtx.fadeDelta && !(--_playerCtx.fadeCount)) {
		_playerCtx.fadeCount = _playerCtx.fadeSkip;

		_playerCtx.volume += _playerCtx.fadeDelta;
		if (_playerCtx.volume == _playerCtx.fadeEndVolume)
			_playerCtx.fadeDelta = 0;
	}

	// Volume
	const uint8 finVol = _playerCtx.volume * channel.volume >> 6;
	Paula::setChannelVolume(channel.paulaChannel, finVol);
}

void Tfmx::macroRun(ChannelContext &channel) {
	bool deferWait = false;
	for (;;) {
		const byte *const macroPtr = (byte *)(_resource.getMacroPtr(channel.macroOffset) + channel.macroStep);
		++channel.macroStep;

		switch (macroPtr[0]) {
		case 0x00:	// Reset + DMA Off. Parameters: deferWait, addset, vol
			clearEffects(channel);
			// FT
		case 0x13:	// DMA Off. Parameters:  deferWait, addset, vol
			// TODO: implement PArameters
			Paula::disableChannel(channel.paulaChannel);
			channel.deferWait = deferWait = (macroPtr[1] != 0);
			if (deferWait) {
				// if set, then we expect a DMA On in the same tick.
				channel.period = 4;
				//Paula::setChannelPeriod(channel.paulaChannel, channel.period);
				Paula::setChannelSampleLen(channel.paulaChannel, 1);
				// in this state we then need to allow some commands that normally
				// would halt the macroprogamm to continue instead.
				// those commands are: Wait, WaitDMA, AddPrevNote, AddNote, SetNote, <unknown Cmd>
				// DMA On is affected aswell
				// TODO remember time disabled, remember pending dmaoff?.
			} else {
				//TODO ?
			}

			if (macroPtr[2])
				channel.volume = macroPtr[3];
			else if (macroPtr[3])
				channel.volume = channel.relVol * 3 + macroPtr[3];
			else
				continue;
			Paula::setChannelVolume(channel.paulaChannel, channel.volume);
			continue;

		case 0x01:	// DMA On
			// TODO: Parameter macroPtr[1] - en-/disable effects
			channel.dmaIntCount = 0;
			if (deferWait) {
				// TODO
				// there is actually a small delay in the player, but I think that
				// only allows to clear DMA-State on real Hardware
			}
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
			Paula::enableChannel(channel.paulaChannel);
			channel.deferWait = deferWait = false;
			continue;

		case 0x02:	// SetBeginn. Parameters: SampleOffset(L)
			channel.addBeginLength = 0;
			channel.sampleStart = READ_BE_UINT32(macroPtr) & 0xFFFFFF;
			Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(channel.sampleStart));
			continue;

		case 0x03:	// SetLength. Parameters: SampleLength(W)
			channel.sampleLen = READ_BE_UINT16(&macroPtr[2]);
			Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
			continue;

		case 0x04:	// Wait. Parameters: Ticks to wait(W).
			// TODO: some unkown Parameter? (macroPtr[1] & 1)
			channel.macroWait = READ_BE_UINT16(&macroPtr[2]);
			return;

		case 0x10:	// Loop Key Up. Parameters: Loopcount, MacroStep(W)
			if (channel.keyUp)
				continue;
			// FT
		case 0x05:	// Loop. Parameters: Loopcount, MacroStep(W)
			if (channel.macroLoopCount != 0) {
				if (channel.macroLoopCount == 0xFF)
					channel.macroLoopCount = macroPtr[1];
				channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			}
			--channel.macroLoopCount;
			continue;

		case 0x06:	// Jump. Parameters: MacroIndex, MacroStep(W)
			channel.macroIndex = macroPtr[1] & (kMaxMacroOffsets - 1);
			channel.macroOffset = _macroOffset[macroPtr[1] & (kMaxMacroOffsets - 1)];
			channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			channel.macroLoopCount = 0xFF;
			continue;

		case 0x07:	// Stop Macro
			channel.macroRun = false;
			--channel.macroStep;
			return;

		case 0x08:	// AddNote. Parameters: Note, Finetune(W)
			setNoteMacro(channel, channel.note + macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
			break;

		case 0x09:	// SetNote. Parameters: Note, Finetune(W)
			setNoteMacro(channel, macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
			break;

		case 0x0A:	// Clear Effects
			clearEffects(channel);
			continue;

		case 0x0B:	// Portamento. Parameters: count, speed
			channel.portaSkip = macroPtr[1];
			channel.portaCount = 1;
			// if porta is already running, then keep using old value
			if (!channel.portaDelta)
				channel.portaValue = channel.refPeriod;
			channel.portaDelta = READ_BE_UINT16(&macroPtr[2]);
			continue;

		case 0x0C:	// Vibrato. Parameters: Speed, intensity
			channel.vibLength = macroPtr[1];
			channel.vibCount = macroPtr[1] / 2;
			channel.vibDelta = macroPtr[3];
			if (!channel.portaDelta) {
				channel.period = channel.refPeriod;
				//Paula::setChannelPeriod(channel.paulaChannel, channel.period);
				channel.vibValue = 0;
			}
			continue;

		case 0x0D:	// Add Volume. Parameters: note, addNoteFlag, volume
			if (macroPtr[2] == 0xFE)
				setNoteMacro(channel, channel.note + macroPtr[1], 0);
			channel.volume = channel.relVol * 3 + macroPtr[3];
			continue;

		case 0x0E:	// Set Volume. Parameters: note, addNoteFlag, volume
			if (macroPtr[2] == 0xFE)
				setNoteMacro(channel, channel.note + macroPtr[1], 0);
			channel.volume = macroPtr[3];
			continue;

		case 0x0F:	// Envelope. Parameters: speed, count, endvol
			channel.envDelta = macroPtr[1];
			channel.envCount = channel.envSkip = macroPtr[2];
			channel.envEndVolume = macroPtr[3];
			continue;

		case 0x11:	// AddBegin. Parameters: times, Offset(W)
			channel.addBeginLength = channel.addBeginCount = macroPtr[1];
			channel.addBeginDelta = (int16)READ_BE_UINT16(&macroPtr[2]);
			channel.sampleStart += channel.addBeginDelta;
			Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(channel.sampleStart));
			continue;

		case 0x12:	// AddLen. Parameters: added Length(W)
			channel.sampleLen += (int16)READ_BE_UINT16(&macroPtr[2]);
			Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
			continue;

		case 0x14:	// Wait key up. Parameters: wait cycles
			if (channel.keyUp || channel.macroLoopCount == 0) {
				channel.macroLoopCount = 0xFF;
				continue;
			} else if (channel.macroLoopCount == 0xFF)
				channel.macroLoopCount = macroPtr[3];
			--channel.macroLoopCount;
			--channel.macroStep;
			return;

		case 0x15:	// Subroutine. Parameters: MacroIndex, Macrostep(W)
			channel.macroReturnOffset = channel.macroOffset;
			channel.macroReturnStep = channel.macroStep;

			channel.macroOffset = _macroOffset[macroPtr[1] & (kMaxMacroOffsets - 1)];
			channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			// TODO: MI does some weird stuff there. Figure out which varioables need to be set
			continue;

		case 0x16:	// Return from Sub.
			channel.macroOffset = channel.macroReturnOffset;
			channel.macroStep = channel.macroReturnStep;
			continue;

		case 0x17:	// set Period. Parameters: Period(W)
			channel.refPeriod = READ_BE_UINT16(&macroPtr[2]);
			if (!channel.portaDelta) {
				channel.period = channel.refPeriod;
				//Paula::setChannelPeriod(channel.paulaChannel, channel.period);
			}
			continue;

		case 0x18: {	// Sampleloop. Parameters: Offset from Samplestart(W)
			// TODO: MI loads 24 bit, but thats useless?
			const uint16 temp = /* ((int8)macroPtr[1] << 16) | */ READ_BE_UINT16(&macroPtr[2]);
			if (macroPtr[1] || (temp & 1))
				warning("Tfmx: Problematic value for sampleloop: %i", (macroPtr[1] << 16) | temp);
			channel.sampleStart += temp & 0xFFFE;
			channel.sampleLen -= (temp / 2) /* & 0x7FFF */;
			Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(channel.sampleStart));
			Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
			continue;
		}
		case 0x19:	// set one-shot Sample
			channel.addBeginLength = 0;
			channel.sampleStart = 0;
			channel.sampleLen = 1;
			Paula::setChannelSampleStart(channel.paulaChannel, _resource.getSamplePtr(0));
			Paula::setChannelSampleLen(channel.paulaChannel, 1);
			continue;

		case 0x1A:	// Wait on DMA. Parameters: Cycles-1(W) to wait
			channel.dmaIntCount = READ_BE_UINT16(&macroPtr[2]) + 1;
			channel.macroRun = false;
			Paula::setChannelDmaCount(channel.paulaChannel);
			break;

/*		case 0x1B:	// Random play. Parameters: macro/speed/mode
			warnMacroUnimplemented(macroPtr, 0);
			continue;*/

		case 0x1C:	// Branch on Note. Parameters: note/macrostep(W)
			if (channel.note > macroPtr[1])
				channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			continue;

		case 0x1D:	// Branch on Volume. Parameters: volume/macrostep(W)
			if (channel.volume > macroPtr[1])
				channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			continue;

/*		case 0x1E:	// Addvol+note. Parameters: note/CONST./volume
			warnMacroUnimplemented(macroPtr, 0);
			continue;*/

		case 0x1F:	// AddPrevNote. Parameters: Note, Finetune(W)
			setNoteMacro(channel, channel.prevNote + macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
			break;

		case 0x20:	// Signal. Parameters: signalnumber/value
			if (_playerCtx.numSignals > macroPtr[1])
				_playerCtx.signal[macroPtr[1]] = READ_BE_UINT16(&macroPtr[2]);
			continue;

		case 0x21:	// Play macro. Parameters: macro/chan/detune
			noteCommand(channel.note, (channel.relVol << 4) | macroPtr[1], macroPtr[2], macroPtr[3]);
			continue;

		// 0x22 - 0x29 are used by Gem`X
		// 0x30 - 0x34 are used by Carribean Disaster

		default:
			debug(3, "TFMX: Macro %02X not supported", macroPtr[0]);
		}
		if (!deferWait)
			return;
	}
}

void Tfmx::advancePatterns() {
startPatterns:
	int runningPatterns = 0;

	for (int i = 0; i < kNumChannels; ++i) {
		const uint8 pattCmd = _patternCtx[i].command;
		if (pattCmd < 0x90) {	// execute Patternstep
			++runningPatterns;
			if (_patternCtx[i].wait == 0) {
				// issue all Steps for this tick
				const bool pendingTrackstep = patternRun(_patternCtx[i]);

				if (pendingTrackstep) {
					// we load the next Trackstep Command and then process all Channels again
					trackRun(true);
					goto startPatterns;
				}

			} else 
				--_patternCtx[i].wait;

		} else if (pattCmd == 0xFE) {	// Stop voice in pattern.expose
			_patternCtx[i].command = 0xFF;
			ChannelContext &channel = _channelCtx[_patternCtx[i].expose & (kNumVoices - 1)];
			if (!channel.sfxLocked) {
				clearMacroProgramm(channel);
				Paula::disableChannel(channel.paulaChannel);
			}
		} // else this pattern-Channel is stopped
	}
	if (_playerCtx.stopWithLastPattern && !runningPatterns) {
		stopPaula();
	}
}

bool Tfmx::patternRun(PatternContext &pattern) {
	for (;;) {
		const byte *const patternPtr = (byte *)(_resource.getPatternPtr(pattern.offset) + pattern.step);
		++pattern.step;
		const byte pattCmd = patternPtr[0];

		if (pattCmd < 0xF0) { // Playnote
			bool doWait = false;
			byte noteCmd = pattCmd + pattern.expose;
			byte param3  = patternPtr[3];
			if (pattCmd < 0xC0) {	// Note
				if (pattCmd >= 0x80) {	// Wait
					pattern.wait = param3;
					param3 = 0;
					doWait = true;
				}
				noteCmd &= 0x3F;
			}	// else Portamento 
			noteCommand(noteCmd, patternPtr[1], patternPtr[2], param3);
			if (doWait)
				return false;

		} else {	// Patterncommand
			switch (pattCmd & 0xF) {
			case 0: 	// End Pattern + Next Trackstep
				pattern.command = 0xFF;
				--pattern.step;
				return true;

			case 1: 	// Loop Pattern. Parameters: Loopcount, PatternStep(W)
				if (pattern.loopCount != 0) {
					if (pattern.loopCount == 0xFF)
						pattern.loopCount = patternPtr[1];
					pattern.step = READ_BE_UINT16(&patternPtr[2]);
				}
				--pattern.loopCount;
				continue;

			case 2: 	// Jump. Parameters: PatternIndex, PatternStep(W)
				pattern.offset = _patternOffset[patternPtr[1]];
				pattern.step = READ_BE_UINT16(&patternPtr[2]);
				continue;

			case 3: 	// Wait. Paramters: ticks to wait
				pattern.wait = patternPtr[1];
				return false;

			case 14: 	// Stop custompattern
				// TODO apparently toggles on/off pattern channel 7
				debug(3, "Tfmx: Encountered 'Stop custompattern' command");
				// FT
			case 4: 	// Stop this pattern
				pattern.command = 0xFF;
				--pattern.step;
				// TODO: try figuring out if this was the last Channel?
				return false;

			case 5: 	// Key Up Signal
				if (!_channelCtx[patternPtr[2] & (kNumVoices - 1)].sfxLocked)
					_channelCtx[patternPtr[2] & (kNumVoices - 1)].keyUp = true;
				continue;

			case 6: 	// Vibrato
			case 7: 	// Envelope
				noteCommand(pattCmd, patternPtr[1], patternPtr[2], patternPtr[3]);
				continue;

			case 8: 	// Subroutine
				pattern.savedOffset = pattern.offset;
				pattern.savedStep = pattern.step;

				pattern.offset = _patternOffset[patternPtr[1] & (kMaxPatternOffsets - 1)];
				pattern.step = READ_BE_UINT16(&patternPtr[2]);
				continue;

			case 9: 	// Return from Subroutine
				pattern.offset = pattern.savedOffset;
				pattern.step = pattern.savedStep;
				continue;

			case 10:	// fade master volume
				initFadeCommand((uint8)patternPtr[1], (int8)patternPtr[1]);
				++_trackCtx.posInd;
				continue;

			case 11: {	// play pattern. Parameters: patternCmd, channel, expose
				PatternContext &target = _patternCtx[patternPtr[2] & (kNumChannels - 1)];

				target.command = patternPtr[1];
				target.offset = _patternOffset[patternPtr[1] & (kMaxPatternOffsets - 1)];
				target.expose = patternPtr[3];
				target.step = 0;
				target.wait = 0;
				target.loopCount = 0xFF;
				}
				continue;

			case 12: 	// Lock
				_channelCtx[patternPtr[2] & (kNumVoices - 1)].sfxLocked = (patternPtr[1] != 0);
				_channelCtx[patternPtr[2] & (kNumVoices - 1)].sfxLockTime = patternPtr[3];
				continue;

			case 13: 	// Cue
				if (_playerCtx.numSignals > patternPtr[1])
					_playerCtx.signal[patternPtr[1]] = READ_BE_UINT16(&patternPtr[2]);
				continue;

			case 15: 	// NOP
				continue;
			}
		}
	}
}

bool Tfmx::trackRun(const bool incStep) {
	assert(_playerCtx.song >= 0);
	if (incStep) {
		// TODO Optionally disable looping
		if (_trackCtx.posInd == _trackCtx.stopInd)
			_trackCtx.posInd = _trackCtx.startInd;
		else
			++_trackCtx.posInd;
	}
	for (;;) {
		const uint16 *const trackData = _resource.getTrackPtr(_trackCtx.posInd);

		if (trackData[0] != FROM_BE_16(0xEFFE)) {
			// 8 commands for Patterns
			for (int i = 0; i < 8; ++i) {
				const uint patCmd = READ_BE_UINT16(&trackData[i]);
				// First byte is pattern number
				const uint patNum = (patCmd >> 8);
				// if highest bit is set then keep previous pattern
				if (patNum < 0x80) {
					_patternCtx[i].step = 0;
					_patternCtx[i].wait = 0;
					_patternCtx[i].loopCount = 0xFF;
					_patternCtx[i].offset = _patternOffset[patNum];
				}
				_patternCtx[i].command = (uint8)patNum;
				_patternCtx[i].expose = patCmd & 0xFF;
			}
			return true;

		} else {
			// 16 byte Trackstep Command
			switch (READ_BE_UINT16(&trackData[1])) {
			case 0:	// Stop Player. No Parameters
				stopPaula();
				return false;

			case 1:	// Branch/Loop section of tracksteps. Parameters: branch target, loopcount
				if (_trackCtx.loopCount != 0) {
					if (_trackCtx.loopCount < 0)
						_trackCtx.loopCount = READ_BE_UINT16(&trackData[3]);
					_trackCtx.posInd    = READ_BE_UINT16(&trackData[2]);
					continue;
				}
				--_trackCtx.loopCount;
				break;

			case 2:	{ // Set Tempo. Parameters: tempo, divisor
				_playerCtx.patternCount = _playerCtx.patternSkip = READ_BE_UINT16(&trackData[2]); // tempo
				const uint16 temp = READ_BE_UINT16(&trackData[3]); // divisor

				if (!(temp & 0x8000) && (temp & 0x1FF))
					setInterruptFreqUnscaled(temp & 0x1FF);
				break;
			}
			case 4:	// Fade
				// load the LSB of the 16bit words
				initFadeCommand(((uint8 *)&trackData[2])[1], ((int8 *)&trackData[3])[1]);
				break;

			case 3:	// Unknown, stops player aswell
			default:
				debug(3, "Tfmx: Unknown Trackstep Command: %02X", READ_BE_UINT16(&trackData[1]));
				// MI-Player handles this by stopping the player, we just continue
			}
		}

		if (_trackCtx.posInd == _trackCtx.stopInd) {
			warning("Tfmx: Reached invalid Song-Position");
			return false;
		}
		++_trackCtx.posInd;
	}
}

void Tfmx::noteCommand(const uint8 note, const uint8 param1, const uint8 param2, const uint8 param3) {
	ChannelContext &channel = _channelCtx[param2 & (kNumVoices - 1)];

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
		channel.macroIndex = param1 & (kMaxMacroOffsets - 1);
		channel.macroOffset = _macroOffset[param1 & (kMaxMacroOffsets - 1)];
		channel.relVol = (param2 >> 4) & 0xF;
		channel.fineTune = (int8)param3;

		initMacroProgramm(channel);
		channel.keyUp = false; // key down = playing a Note
		
	} else if (note < 0xF0) {	// Portamento
		channel.portaSkip = param1;
		channel.portaCount = 1;
		if (!channel.portaDelta)
			channel.portaValue = channel.refPeriod;
		channel.portaDelta = param3;

		channel.note = note & 0x3F;
		channel.refPeriod = noteIntervalls[channel.note];
	} else switch (note & 0xF) {	// Command
		case 5:	// Key Up Signal
			channel.keyUp = true;
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

	assert(0 == _resource.mdatData);
	assert(0 == _resource.sampleData);

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
	_resource.sfxTableOffset = 0x200;
	bool getSfxIndex = false;

	// This is how MI`s TFMX-Player tests for unpacked Modules.
	if (offTrackstep == 0) {
		offTrackstep	= 0x600 + 0x200;
		offPatternP		= 0x200 + 0x200;
		offMacroP		= 0x400 + 0x200;
		getSfxIndex = true;
		_resource.sfxTableOffset = 0x5FC;
	}

	_resource.trackstepOffset = offTrackstep;

	// Read in pattern starting offsets
	musicData.seek(offPatternP);
	for (int i = 0; i < kMaxPatternOffsets; ++i)
		_patternOffset[i] = musicData.readUint32BE();

	res = musicData.err();
	assert(!res);

	if (getSfxIndex)
		_resource.sfxTableOffset = _patternOffset[127];

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

	_resource.mdatData = new byte[size];
	assert(_resource.mdatData);
	_resource.mdatLen = size;
	musicData.seek(0);
	musicData.read(_resource.mdatData, size);
	
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

	_resource.sampleData = new byte[size];
	assert(_resource.sampleData);
	_resource.sampleLen = size;
	sampleData.seek(0);
	sampleData.read(_resource.sampleData, size);
	for (int i = 0; i < 4; ++i)
		_resource.sampleData[i] = 0;
	
	res = sampleData.err();
	assert(!res);
	sampleData.readByte();
	res = sampleData.eos();
	assert(res);

	return true;
}


void Tfmx::doMacro(int note, int macro, int relVol, int finetune, int channelNo) {
	assert(0 <= macro && macro < kMaxMacroOffsets);
	assert(0 <= note && note < 0xC0);
	Common::StackLock lock(_mutex);

	channelNo &= (kNumVoices - 1);
	ChannelContext &channel = _channelCtx[channelNo];
	unlockMacroChannel(channel);

	noteCommand((uint8)note, (uint8)macro, (uint8)(relVol << 4) | channelNo, (uint8)finetune);
	startPaula();
}

void  Tfmx::stopMacroEffect(int channel) {
	assert(0 <= channel && channel < kNumVoices);
	Common::StackLock lock(_mutex);
	unlockMacroChannel(_channelCtx[channel]);
	clearMacroProgramm(_channelCtx[channel]);
	Paula::disableChannel(_channelCtx[channel].paulaChannel);
}

void Tfmx::stopSong(bool stopAudio) {
	Common::StackLock lock(_mutex);
	_playerCtx.song = -1;
	if (stopAudio) {
		stopMacroChannels();
		stopPaula();
	}
}

void Tfmx::doSong(int songPos, bool stopAudio) {
	assert(0 <= songPos && songPos < kNumSubsongs);
	Common::StackLock lock(_mutex);

	stopPatternChannels();
	if (stopAudio) {
		stopMacroChannels();
		stopPaula();
	}

	_playerCtx.song = (int8)songPos;

	_trackCtx.loopCount = -1;
	_trackCtx.startInd = _trackCtx.posInd = _subsong[songPos].songstart;
	_trackCtx.stopInd = _subsong[songPos].songend;

	const bool palFlag = (_resource.headerFlags & 2) != 0;
	const uint16 tempo = _subsong[songPos].tempo;
	uint16 ciaIntervall;
	if (tempo >= 0x10) {
		ciaIntervall = (uint16)(kCiaBaseInterval / tempo);
		_playerCtx.patternSkip = 0;
	} else {
		ciaIntervall = palFlag ? (uint16)kPalDefaultCiaVal : (uint16)kNtscDefaultCiaVal;
		_playerCtx.patternSkip = tempo;
	}
	setInterruptFreqUnscaled(ciaIntervall);

	_playerCtx.patternCount = 0;
	if (trackRun())
		startPaula();
}

int Tfmx::doSfx(uint16 sfxIndex, bool unlockChannel) {
	assert(0 <= sfxIndex && sfxIndex < 128);
	Common::StackLock lock(_mutex);

	const byte *sfxEntry = _resource.getSfxPtr(sfxIndex);
	if (sfxEntry[0] == 0xFB) {
		// custompattern
		const uint8 patCmd = sfxEntry[2];
		const int8 patExp = (int8)sfxEntry[3];
	} else {
		// custommacro
		const byte channelNo = ((_playerCtx.song >= 0) ? sfxEntry[2] : sfxEntry[4]) & (kNumVoices - 1);
		const byte priority = sfxEntry[5] & 0x7F;

		ChannelContext &channel = _channelCtx[channelNo];
		if (unlockChannel)
			unlockMacroChannel(channel);

		const int16 sfxLocktime = channel.sfxLockTime;
		if (priority >= channel.customMacroPrio || sfxLocktime < 0) {
			if (sfxIndex != channel.customMacroIndex || sfxLocktime < 0 || (sfxEntry[5] < 0x80)) {
				channel.customMacro = READ_UINT32(sfxEntry); // intentionally not "endian-correct"
				channel.customMacroPrio = priority;
				channel.customMacroIndex = (uint8)sfxIndex;
				debug(3, "Tfmx: running Macro %08X on channel %i - priority: %02X", TO_BE_32(channel.customMacro), channelNo, priority);
				return channelNo;
			}
		}
	}
	return -1;
}

}	// End of namespace Audio

// some debugging functions
namespace {
#ifndef NDEBUG
void displayMacroStep(const void *const vptr) {
	const char *tableMacros[] = {
		"DMAoff+Resetxx/xx/xx flag/addset/vol   ",
		"DMAon (start sample at selected begin) ",
		"SetBegin    xxxxxx   sample-startadress",
		"SetLen      ..xxxx   sample-length     ",
		"Wait        ..xxxx   count (VBI''s)    ",
		"Loop        xx/xxxx  count/step        ",
		"Cont        xx/xxxx  macro-number/step ",
		"-------------STOP----------------------",
		"AddNote     xx/xxxx  note/detune       ",
		"SetNote     xx/xxxx  note/detune       ",
		"Reset   Vibrato-Portamento-Envelope    ",
		"Portamento  xx/../xx count/speed       ",
		"Vibrato     xx/../xx speed/intensity   ",
		"AddVolume   ....xx   volume 00-3F      ",
		"SetVolume   ....xx   volume 00-3F      ",
		"Envelope    xx/xx/xx speed/count/endvol",
		"Loop key up xx/xxxx  count/step        ",
		"AddBegin    xx/xxxx  count/add to start",
		"AddLen      ..xxxx   add to sample-len ",
		"DMAoff stop sample but no clear        ",
		"Wait key up ....xx   count (VBI''s)    ",
		"Go submacro xx/xxxx  macro-number/step ",
		"--------Return to old macro------------",
		"Setperiod   ..xxxx   DMA period        ",
		"Sampleloop  ..xxxx   relative adress   ",
		"-------Set one shot sample-------------",
		"Wait on DMA ..xxxx   count (Wavecycles)",
		"Random play xx/xx/xx macro/speed/mode  ",
		"Splitkey    xx/xxxx  key/macrostep     ",
		"Splitvolume xx/xxxx  volume/macrostep  ",
		"Addvol+note xx/fe/xx note/CONST./volume",
		"SetPrevNote xx/xxxx  note/detune       ",
		"Signal      xx/xxxx  signalnumber/value",
		"Play macro  xx/.x/xx macro/chan/detune ",
		"SID setbeg  xxxxxx   sample-startadress",
		"SID setlen  xx/xxxx  buflen/sourcelen  ",
		"SID op3 ofs xxxxxx   offset            ",
		"SID op3 frq xx/xxxx  speed/amplitude   ",
		"SID op2 ofs xxxxxx   offset            ",
		"SID op2 frq xx/xxxx  speed/amplitude   ",
		"SID op1     xx/xx/xx speed/amplitude/TC",
		"SID stop    xx....   flag (1=clear all)"
	};

	const byte *const macroData = (const byte *const)vptr;
	if (macroData[0] < ARRAYSIZE(tableMacros))
		debug("%s %02X%02X%02X", tableMacros[macroData[0]], macroData[1], macroData[2], macroData[3]);
	else
		debug("Unkown Macro #%02X %02X%02X%02X", macroData[0], macroData[1], macroData[2], macroData[3]);
}

void displayPatternstep(const void *const vptr) {
	const char *tablePatterns[] = {
		"End --Next track  step--",
		"Loop[count     / step.w]",
		"Cont[patternno./ step.w]",
		"Wait[count 00-FF--------",
		"Stop--Stop this pattern-",
		"Kup^-Set key up/channel]",
		"Vibr[speed     / rate.b]",
		"Enve[speed /endvolume.b]",
		"GsPt[patternno./ step.w]",
		"RoPt-Return old pattern-",
		"Fade[speed /endvolume.b]",
		"PPat[patt./track+transp]",
		"Lock---------ch./time.b]",
		"Cue [number.b/  value.w]",
		"Stop-Stop custompattern-",
		"NOP!-no operation-------"
	};

	const byte *const patData = (const byte *const)vptr;
	const byte command = patData[0];
	if (command < 0xF0) { // Playnote
		const byte flags = command >> 6; // 0-1 means note+detune, 2 means wait, 3 means portamento?
		char *flagsSt[] = { "Note ", "Note ", "Wait ", "Porta" };
		debug("%s %02X%02X%02X%02X", flagsSt[flags], patData[0], patData[1], patData[2], patData[3]);
	} else
		debug("%s %02X%02X%02X",tablePatterns[command & 0xF], patData[1], patData[2], patData[3]);
}
#else
void displayMacroStep(const void *const vptr, int chan, int index) {}
void displayPatternstep(const void *const vptr) {}
#endif
}	// End of namespace
