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

#include "sound/mods/maxtrax.h"

// test for engines using this class.
#if defined(SOUND_MODS_MAXTRAX_H)

namespace {
int32 precalcNote(byte baseNote, int16 tune, byte octave) {
	return 0x9fd77 + 0x3C000 + (1 << 16) - ((baseNote << 14) + (tune << 11) / 3) / 3 - (octave << 16);
}
int32 calcVolumeDelta(int32 delta, uint16 time, uint16 vBlankFreq) {
	const int32 div = time * vBlankFreq;
	// div <= 1000 means time to small (or even 0)
	return (div <= 1000) ? delta : (1000 * delta) / div;
}
int32 calcTempo(const uint16 tempo, uint16 vBlankFreq) {
	return (int32)(((uint32)(tempo & 0xFFF0) << 8) / (uint16)(5 * vBlankFreq));
}
}

namespace Audio {

MaxTrax::MaxTrax(int rate, bool stereo)
	: Paula(stereo, rate, rate/50),
	  _voiceCtx(),
	  _patch(),
	  _scores(),
	  _numScores() {
	_playerCtx.maxScoreNum = 128;
	_playerCtx.vBlankFreq = 50;
	_playerCtx.frameUnit = (uint16)((1000 * (1<<8)) /  _playerCtx.vBlankFreq);
	_playerCtx.scoreIndex = -1;
	_playerCtx.nextEvent = 0;
	_playerCtx.volume = 0x40;

	_playerCtx.tempo = 120;
	_playerCtx.tempoTime = 0;
	_playerCtx.syncCallBack = 0;

	for (int i = 0; i < ARRAYSIZE(_channelCtx); ++i)
		resetChannel(_channelCtx[i], (i & 1) != 0);
}

MaxTrax::~MaxTrax() {
	stopMusic();
	freePatches();
	freeScores();
}

void MaxTrax::interrupt() {
	// a5 - maxtraxm a4 . globaldata

	// TODO
	// test for changes in shared struct and make changes
	// specifically all used channels get marked altered

	_playerCtx.ticks += _playerCtx.tickUnit;
	const int32 millis = _playerCtx.ticks >> 8; // d4
	for (int i = 0; i < ARRAYSIZE(_voiceCtx); ++i) {
		VoiceContext &voice = _voiceCtx[i]; // a3
		if (voice.channel && voice.stopEventCommand < 0x80) {
			const int channelNo = voice.stopEventParameter;
			assert(channelNo == voice.channel - _channelCtx); // TODO remove
			voice.stopEventTime -= (channelNo < kNumChannels) ? _playerCtx.tickUnit : _playerCtx.frameUnit;
			if (voice.stopEventTime <= 0) {
				noteOff(voice, voice.stopEventCommand);
				voice.stopEventCommand = 0xFF;
			}
		}
	}

	if (_playerCtx.musicPlaying) {
		const Event *curEvent = _playerCtx.nextEvent;
		int32 eventDelta = _playerCtx.nextEventTime - millis;
		for (; eventDelta <= 0; eventDelta += (++curEvent)->startTime) {
			const byte cmd = curEvent->command;
			ChannelContext &channel = _channelCtx[curEvent->parameter & 0x0F];

			// outPutEvent(*curEvent);
			// debug("CurTime, EventDelta, NextDelta: %d, %d, %d", millis, eventDelta, eventDelta + curEvent[1].startTime );

			if (cmd < 0x80) {	// Note
				const int8 voiceIndex = noteOn(channel, cmd, (curEvent->parameter & 0xF0) >> 1, kPriorityScore);
				if (voiceIndex >= 0) {
					VoiceContext &voice = _voiceCtx[voiceIndex];
					voice.stopEventCommand = cmd;
					voice.stopEventParameter = curEvent->parameter & 0x0F;
					voice.stopEventTime = (eventDelta + curEvent->stopTime) << 8;
				}

			} else {
				switch (cmd) {

				case 0x80:	// TEMPO
					if ((_playerCtx.tickUnit >> 8) > curEvent->stopTime) {
						_playerCtx.tickUnit = calcTempo(curEvent->parameter << 4, _playerCtx.vBlankFreq);
						_playerCtx.tempoTime = 0;
					} else {
						_playerCtx.tempoStart = _playerCtx.tempo;
						_playerCtx.tempoDelta = (curEvent->parameter << 4) - _playerCtx.tempoStart;
						_playerCtx.tempoTime  = (curEvent->stopTime << 8);
						_playerCtx.tempoTicks = 0;
					}
					break;
				
				case 0xC0:	// PROGRAM
					channel.patch = &_patch[curEvent->stopTime & (kNumPatches - 1)];
					break;

				case 0xE0:	// BEND
					channel.pitchBend = ((curEvent->stopTime & 0x7F00) >> 1) | (curEvent->stopTime & 0x7f);
					channel.pitchReal = (((int32)channel.pitchBendRange * channel.pitchBend) >> 5) - (channel.pitchBendRange << 8);
					channel.isAltered = true;
					break;

				case 0xFF:	// END
					if (_playerCtx.musicLoop) {
						curEvent = _scores[_playerCtx.scoreIndex].events;
						eventDelta = curEvent->startTime - millis;
						_playerCtx.ticks = 0;
					} else
						_playerCtx.musicPlaying = false;
					// stop processing for this tick
					goto endOfEventLoop;

				case 0xA0: 	// SPECIAL
					switch (curEvent->stopTime >> 8){
					case 0x01:	// SPECIAL_SYNC
						if (_playerCtx.syncCallBack)
							_playerCtx.syncCallBack(curEvent->stopTime & 0xFF);
						break;
					case 0x02:	// SPECIAL_BEGINREP
						// we allow a depth of 4 loops
						for (int i = 0; i < ARRAYSIZE(_playerCtx.repeatPoint); ++i) {
							if (!_playerCtx.repeatPoint[i]) {
								_playerCtx.repeatPoint[i] = curEvent;
								_playerCtx.repeatCount[i] = curEvent->stopTime & 0xFF;
								break;
							}
						}
						break;
					case 0x03:	// SPECIAL_ENDREP
						for (int i = ARRAYSIZE(_playerCtx.repeatPoint) - 1; i >= 0; --i) {
							if (_playerCtx.repeatPoint[i]) {
								if (_playerCtx.repeatCount[i]--)
									curEvent = _playerCtx.repeatPoint[i]; // gets incremented by 1 at end of loop
								else
									_playerCtx.repeatPoint[i] = 0;
								break;
							}
						}
						break;
					}
					break;

				case 0xB0:	// CONTROL
					// TODO: controlChange((byte)stopTime, (byte)(stopTime >> 8))
				default:
					debug("Unhandled Command");
					outPutEvent(*curEvent);
				}
			}
		}
endOfEventLoop:
		_playerCtx.nextEvent = curEvent;
		_playerCtx.nextEventTime = eventDelta + millis;

		// tempoEffect
		if (_playerCtx.tempoTime) {
			_playerCtx.tempoTicks += _playerCtx.tickUnit;
			uint16 newTempo = _playerCtx.tempoStart;
			if (_playerCtx.tempoTicks < _playerCtx.tempoTime) {
				newTempo += (uint16)((_playerCtx.tempoTicks * _playerCtx.tempoDelta) / _playerCtx.tempoTime);
			} else {
				_playerCtx.tempoTime = 0;
				newTempo += _playerCtx.tempoDelta;
			}
			_playerCtx.tickUnit = calcTempo(_playerCtx.tempoStart + newTempo, _playerCtx.vBlankFreq);
		}
	}

	// Handling of Envelopes and Portamento
	for (int i = 0; i < ARRAYSIZE(_voiceCtx); ++i) {
		VoiceContext &voice = _voiceCtx[i];
		if (!voice.channel)
			continue;
		const ChannelContext &channel = *voice.channel;
		const Patch &patch = *voice.patch;

		switch (voice.status) {
		case VoiceContext::kStatusSustain:
			if (!channel.isAltered && !voice.hasPortamento/* && !channel.modulation*/)
				continue;
			// Update Volume and Period
			break;

		case VoiceContext::kStatusHalt:
			killVoice((byte)i);
			continue;

		case VoiceContext::kStatusStart:
			if (patch.attackLen) {
				voice.envelope = patch.attackPtr;
				const uint16 duration = voice.envelope->duration;
				voice.envelopeLeft = patch.attackLen;
				voice.ticksLeft = duration << 8;
				voice.status = VoiceContext::kStatusAttack;
				voice.incrVolume = calcVolumeDelta((int32)voice.envelope->volume, duration, _playerCtx.vBlankFreq);
				// Process Envelope
			} else {
				voice.status = VoiceContext::kStatusSustain;
				voice.baseVolume = patch.volume;
				// Update Volume and Period
			}
			break;

		case VoiceContext::kStatusRelease:
			if (patch.releaseLen) {
				voice.envelope = patch.attackPtr + patch.attackLen;
				const uint16 duration = voice.envelope->duration;
				voice.envelopeLeft = patch.releaseLen;
				voice.ticksLeft = duration << 8;
				voice.status = VoiceContext::kStatusDecay;
				voice.incrVolume = calcVolumeDelta((int32)voice.envelope->volume - voice.baseVolume, duration, _playerCtx.vBlankFreq);
				// Process Envelope
			} else {
				voice.status = VoiceContext::kStatusHalt;
				voice.lastVolume = 0;
				// Send Audio Packet
			}
			break;
		}

		// Process Envelope
		const uint16 envUnit = _playerCtx.frameUnit;
		if (voice.envelope) {
			if (voice.ticksLeft > envUnit) {	// envelope still active
				voice.baseVolume = (uint16)MIN(MAX(0, voice.baseVolume + voice.incrVolume), 0x8000);
				voice.ticksLeft -= envUnit;
				// Update Volume and Period

			} else {	// next or last Envelope
				voice.baseVolume = voice.envelope->volume;
				assert(voice.envelopeLeft > 0);
				if (--voice.envelopeLeft) {
					++voice.envelope;
					const uint16 duration = voice.envelope->duration;
					voice.ticksLeft = duration << 8;
					voice.incrVolume = calcVolumeDelta((int32)voice.envelope->volume - voice.baseVolume, duration, _playerCtx.vBlankFreq);
					// Update Volume and Period
				} else if (voice.status == VoiceContext::kStatusDecay) {
					voice.status = VoiceContext::kStatusHalt;
					voice.envelope = 0;
					voice.lastVolume = 0;
					// Send Audio Packet
				} else {
					assert(voice.status == VoiceContext::kStatusAttack);
					voice.status = VoiceContext::kStatusSustain;
					voice.envelope = 0;
					// Update Volume and Period
				}
			}
		}

		// Update Volume and Period
		if (voice.status >= VoiceContext::kStatusDecay) {
			// Calc volume
			uint16 vol = (voice.noteVolume < (1 << 7)) ? (voice.noteVolume * _playerCtx.volume) >> 7 : _playerCtx.volume;
			if (voice.baseVolume < (1 << 15))
				vol = (uint16)(((uint32)vol * voice.baseVolume) >> 15);
			if (voice.channel->volume < (1 << 7))
				vol = (vol * voice.channel->volume) >> 7;
			voice.lastVolume = (byte)MIN(vol, (uint16)0x64);

			// Calc Period
			if (voice.hasPortamento) {
				voice.portaTicks += envUnit;
				if ((uint16)(voice.portaTicks >> 8) >= channel.portamentoTime) {
					voice.hasPortamento = false;
					voice.baseNote = voice.endNote;
					voice.preCalcNote = precalcNote(voice.baseNote, patch.tune, voice.octave);
				}
				voice.lastPeriod = calcNote(voice);
			} else if (channel.isAltered/* || channel.modulation*/)
				voice.lastPeriod = calcNote(voice);
		}

		// Send Audio Packet
		Paula::setChannelPeriod((byte)i, (voice.lastPeriod) ? voice.lastPeriod : 1000);
		Paula::setChannelVolume((byte)i, (voice.lastPeriod) ? voice.lastVolume : 0);
	}
	for (ChannelContext *c = _channelCtx; c != &_channelCtx[ARRAYSIZE(_channelCtx)]; ++c)
		c->isAltered = false;

	//modulation stuff,  sinevalue += tickunit

	// we need to check if some voices have no sustainSample.
	// in that case they are finished after the attackSample is done
	for (int i = 0; i < ARRAYSIZE(_voiceCtx); ++i) {
		VoiceContext &voice = _voiceCtx[i];
		if (voice.dmaOff && Paula::getChannelDmaCount((byte)i) >= voice.dmaOff ) {
			voice.isBlocked = false;
			voice.priority = 0;
			voice.dmaOff = 0;
			if (voice.status == VoiceContext::kStatusSustain)
				voice.status = VoiceContext::kStatusRelease;
		}
	}
}

void MaxTrax::setTempo(const uint16 tempo) {
	Common::StackLock lock(_mutex);
	_playerCtx.tickUnit = calcTempo(tempo, _playerCtx.vBlankFreq);
}

void MaxTrax::stopMusic() {
	Common::StackLock lock(_mutex);
	_playerCtx.musicPlaying = false;
	_playerCtx.scoreIndex = -1;
	_playerCtx.nextEvent = 0;
}

bool MaxTrax::playSong(int songIndex, bool loop) {
	if (songIndex < 0 || songIndex >= _numScores)
		return false;
	Common::StackLock lock(_mutex);
	_playerCtx.musicPlaying = false;
	_playerCtx.musicLoop = loop;

	setTempo(_playerCtx.tempoInitial << 4);
	Paula::setAudioFilter(_playerCtx.filterOn);
	_playerCtx.tempoTime = 0;
	_playerCtx.scoreIndex = songIndex;
	_playerCtx.ticks = 0;

	for (int i = 0; i < ARRAYSIZE(_playerCtx.repeatPoint); ++i)
		_playerCtx.repeatPoint[i] = 0;
	for (int i = 0; i < ARRAYSIZE(_voiceCtx); ++i)
		killVoice((byte)i);
	for (int i = 0; i < kNumChannels; ++i)
		resetChannel(_channelCtx[i], (i & 1) != 0);

	_playerCtx.nextEvent = _scores[songIndex].events;;
	_playerCtx.nextEventTime = _playerCtx.nextEvent->startTime;

	_playerCtx.musicPlaying = true;
	Paula::startPaula();
	return true;
}

void MaxTrax::advanceSong(int advance) {
	Common::StackLock lock(_mutex);
	const Event *cev = _playerCtx.nextEvent;
	if (cev) {
		for (; advance > 0; --advance) {
			// TODO - check for boundaries 
			for (; cev->command != 0xFF && (cev->command != 0xA0 || (cev->stopTime >> 8) != 0x00); ++cev)
				; // no end_command or special_command + end
		}
		_playerCtx.nextEvent = cev;
	}
}

void MaxTrax::killVoice(byte num) {
	VoiceContext &voice = _voiceCtx[num];
	if (voice.channel)
		--(voice.channel->voicesActive);
	voice.channel = 0;
	voice.envelope = 0;
	voice.status = VoiceContext::kStatusFree;
	voice.isBlocked = false;
	voice.hasDamper = false;
	voice.hasPortamento = false;
	voice.priority = 0;
	voice.dmaOff = 0;
	//voice.uinqueId = 0;

	// "stop" voice, set period to 1, vol to 0
	Paula::disableChannel(num);
	Paula::setChannelPeriod(num, 1);
	Paula::setChannelVolume(num, 0);
}

int8 MaxTrax::pickvoice(const VoiceContext voices[4], uint pick, int16 pri) {
	enum { kPrioFlagFixedSide = 1 << 3 };
	if ((pri & (kPrioFlagFixedSide)) == 0) {
		const bool leftSide = (uint)(pick - 1) > 1;
		const int leftBest = MIN(voices[0].status, voices[3].status);
		const int rightBest = MIN(voices[1].status, voices[2].status);
		const int sameSide = (leftSide) ? leftBest : rightBest;
		const int otherSide = leftBest + rightBest - sameSide;

		if (sameSide > VoiceContext::kStatusRelease && otherSide <= VoiceContext::kStatusRelease)
			pick ^= 1; // switches sides
	}
	pick &= 3;

	for (int i = 2; i > 0; --i) {
		const VoiceContext *voice = &voices[pick];
		const VoiceContext *alternate = &voices[pick ^ 3];

		if (voice->status > alternate->status 
			|| (voice->status == alternate->status && voice->lastVolume > alternate->lastVolume)) {
			// TODO: tiebreaking
			pick ^= 3; // switch channels
			const VoiceContext *tmp = voice;
			voice = alternate;
			alternate = tmp;
		}

		if (voice->isBlocked || voice->priority > pri) {
			pick ^= 3; // switch channels
			if (alternate->isBlocked || alternate->priority > pri) {
				// if not already done, switch sides and try again
				pick ^= 1;
				continue;
			}
		}
		// succeded
		return (int8)pick;
	}
	// failed
	debug("Nopick");
	return -1;
}

uint16 MaxTrax::calcNote(const VoiceContext &voice) {
	const ChannelContext &channel = *voice.channel;
	int16 bend = channel.pitchReal;
	if (voice.hasPortamento)
		bend += (int16)(((int8)(voice.endNote - voice.baseNote)) * voice.portaTicks) / channel.portamentoTime;

	// 0x9fd77 ~ log2(1017)  MIDI F5 ?
	// 0x8fd77 ~ log2(508.5) MIDI F4 ?
	// 0x6f73d ~ log2(125) ~ 5675Hz
	enum { K_VALUE = 0x9fd77, PREF_PERIOD = 0x8fd77, PERIOD_LIMIT = 0x6f73d };

	// tone = voice.baseNote << 8 + microtonal
	// bend = channelPitch + porta + modulation

	const int32 tone = voice.preCalcNote + (bend << 6) / 3;

	if (tone >= PERIOD_LIMIT + (1 << 16)) {
		// calculate 2^tone and round towards nearest integer 
		// 2*2^tone = exp((tone+1) * ln(2))
		const uint16 periodX2 = (uint16)expf((float)tone * (float)(0.69314718055994530942 / (1 << 16)));
		return (periodX2 + 1) / 2;
	}
	return 0;
}

int8 MaxTrax::noteOn(ChannelContext &channel, const byte note, uint16 volume, uint16 pri) {
//	if (channel.microtonal >= 0)
//		_microtonal[note % 127] = channel.microtonal;
	if (!volume)
		return -1;

	const Patch &patch = *channel.patch;
	if (!patch.samplePtr || patch.sampleTotalLen == 0)
		return -1;
	int8 voiceNum = -1;
	if ((channel.flags & ChannelContext::kFlagMono) != 0  && channel.voicesActive) {
		VoiceContext *voice = _voiceCtx + ARRAYSIZE(_voiceCtx) - 1;
		for (voiceNum = ARRAYSIZE(_voiceCtx) - 1; voiceNum >= 0 && voice->channel != &channel; --voiceNum, --voice)
			;
		if (voiceNum >= 0 && voice->status >= VoiceContext::kStatusSustain && (channel.flags & ChannelContext::kFlagPortamento) != 0) {
			// reset previous porta
			if (voice->hasPortamento)
				voice->baseNote = voice->endNote;
			voice->preCalcNote = precalcNote(voice->baseNote, patch.tune, voice->octave);
			voice->portaTicks = 0;
			voice->hasPortamento = true;
			voice->endNote = channel.lastNote = note;
			voice->noteVolume = (_playerCtx.handleVolume) ? volume + 1 : 128;
		}

	} else {
		voiceNum = pickvoice(_voiceCtx, (channel.flags & ChannelContext::kFlagRightChannel) != 0 ? 1 : 0, pri);
		if (voiceNum >= 0) {
			VoiceContext &voice = _voiceCtx[voiceNum];
			voice.hasDamper = false;
			voice.isBlocked = false;
			voice.hasPortamento = false;
			if (voice.channel)
				killVoice(voiceNum);
			voice.channel = &channel;
			voice.patch = &patch;
			voice.baseNote = note;

			const int32 plainNote = precalcNote(voice.baseNote, patch.tune, 0);
			const int32 PREF_PERIOD1 = 0x8fd77 + (1 << 16);
			// calculate which sample to use
			const int useOctave = (plainNote <= PREF_PERIOD1) ? 0 : MIN<int32>((plainNote + 0xFFFF - PREF_PERIOD1) >> 16, patch.sampleOctaves - 1);
			voice.octave = (byte)useOctave;
			voice.preCalcNote = plainNote - (useOctave << 16);
			
			voice.lastPeriod = calcNote(voice);

			voice.priority = (byte)pri;
			voice.status = VoiceContext::kStatusStart;

			voice.noteVolume = (_playerCtx.handleVolume) ? volume + 1 : 128;

			// ifeq HAS_FULLCHANVOL macro
			if (channel.volume < 128)
				voice.noteVolume = (voice.noteVolume * channel.volume) >> 7;

			voice.baseVolume = 0;

			const uint16 period = (voice.lastPeriod) ? voice.lastPeriod : 1000;

			// TODO: since the original player is using the OS-functions, more than 1 sample could be queued up already
			// get samplestart for the given octave
			const int8 *samplePtr = patch.samplePtr + (patch.sampleTotalLen << useOctave) - patch.sampleTotalLen;
			if (patch.sampleAttackLen) {
				Paula::setChannelSampleStart(voiceNum, samplePtr);
				Paula::setChannelSampleLen(voiceNum, (patch.sampleAttackLen << useOctave) / 2);
				Paula::setChannelPeriod(voiceNum, period);
				Paula::setChannelVolume(voiceNum, 0);

				Paula::enableChannel(voiceNum);
				// wait  for dma-clear
				// FIXME: this is a workaround to enable oneshot-samples and it currently might crash Paula
				if (patch.sampleTotalLen == patch.sampleAttackLen) {
					Paula::setChannelSampleStart(voiceNum, 0);
					Paula::setChannelSampleLen(voiceNum, 0);
					Paula::setChannelDmaCount(voiceNum);
					voice.dmaOff = 1;
				}
			}

			if (patch.sampleTotalLen > patch.sampleAttackLen) {
				Paula::setChannelSampleStart(voiceNum, samplePtr + (patch.sampleAttackLen << useOctave));
				Paula::setChannelSampleLen(voiceNum, ((patch.sampleTotalLen - patch.sampleAttackLen) << useOctave) / 2);
				if (!patch.sampleAttackLen) {
					// need to enable channel
					Paula::setChannelPeriod(voiceNum, period);
					Paula::setChannelVolume(voiceNum, 0);

					Paula::enableChannel(voiceNum);
				}
				// another pointless wait for DMA-Clear???
			}

			channel.voicesActive++;
			if (&channel < &_channelCtx[kNumChannels]) {
				if ((channel.flags & ChannelContext::kFlagPortamento) != 0) {
					if ((channel.flags & ChannelContext::kFlagMono) != 0 && channel.lastNote < 0x80 && channel.lastNote != voice.baseNote) {
						voice.portaTicks = 0;
						voice.endNote = voice.baseNote;
						voice.baseNote = channel.lastNote;
						voice.preCalcNote = precalcNote(voice.baseNote, patch.tune, voice.octave);
						voice.hasPortamento = true;
					}
					channel.lastNote = note;
				}
			}
		}
	}
	return voiceNum;
}

void MaxTrax::noteOff(VoiceContext &voice, const byte note) {
	ChannelContext &channel = *voice.channel;
	if (channel.voicesActive && voice.status != VoiceContext::kStatusRelease) {
		// TODO is this check really necessary?
		const byte refNote = (voice.hasPortamento) ? voice.endNote : voice.baseNote;
		assert(refNote == note);
		if (refNote == note) {
			if ((channel.flags & ChannelContext::kFlagDamper) != 0)
				voice.hasDamper = true;
			else
				voice.status = VoiceContext::kStatusRelease;
		}
	}
}

void MaxTrax::resetChannel(ChannelContext &chan, bool rightChannel) {
	chan.modulation = 0;
	chan.modulationTime = 1000;
	chan.microtonal = -1;
	chan.portamentoTime = 500;
	chan.pitchBend = 64 << 7;
	chan.pitchReal = 0;
	chan.pitchBendRange = 24;
	chan.volume = 128;
	// TODO: Not all flags sre (re)set, this might make a difference for the unimplemented commands
//	chan.flags &= ~ChannelContext::kFlagPortamento & ~ChannelContext::kFlagMicrotonal;
	chan.isAltered = true;
	if (rightChannel)
		chan.flags = ChannelContext::kFlagRightChannel;
	else
		chan.flags = 0; //~ChannelContext::kFlagRightChannel;
}

void MaxTrax::freeScores() {
	if (_scores) {
		for (int i = 0; i < _numScores; ++i)
			delete[] _scores[i].events;
		delete[] _scores;
		_scores = 0;
	}
	_numScores = 0;
//	memset(_microtonal, 0, sizeof(_microtonal));
}

void MaxTrax::freePatches() {
	for (int i = 0; i < ARRAYSIZE(_patch); ++i) {
		delete[] _patch[i].samplePtr;
		delete[] _patch[i].attackPtr;
	}
	memset(const_cast<Patch *>(_patch), 0, sizeof(_patch));
}

int MaxTrax::playNote(byte note, byte patch, uint16 duration, uint16 volume, bool rightSide) {
	Common::StackLock lock(_mutex);
	assert(patch < ARRAYSIZE(_patch));

	ChannelContext &channel = _channelCtx[kNumChannels];
	channel.flags = (rightSide) ? ChannelContext::kFlagRightChannel : 0;
	channel.isAltered = false;
	channel.patch = &_patch[patch];
	const int8 voiceIndex = noteOn(channel, note, (byte)volume, kPriorityNote);
	if (voiceIndex >= 0) {
		VoiceContext &voice = _voiceCtx[voiceIndex];
		voice.stopEventCommand = note;
		voice.stopEventParameter = kNumChannels;
		voice.stopEventTime = duration << 8;
	}
	return voiceIndex;
}

bool MaxTrax::load(Common::SeekableReadStream &musicData, bool loadScores, bool loadSamples) {
	Common::StackLock lock(_mutex);
	stopMusic();
	if (loadSamples)
		freePatches();
	if (loadScores)
		freeScores();
	// 0x0000: 4 Bytes Header "MXTX"
	// 0x0004: uint16 tempo
	// 0x0006: uint16 flags. bit0 = lowpassfilter, bit1 = attackvolume, bit15 = microtonal	
	if (musicData.readUint32BE() != 0x4D585458) {
		warning("Maxtrax: File is not a Maxtrax Module");
		return false;
	}
	const uint16 songTempo = musicData.readUint16BE();
	const uint16 flags = musicData.readUint16BE();
	if (loadScores) {
		_playerCtx.tempoInitial = songTempo;
		_playerCtx.filterOn = (flags & 1) != 0;
		_playerCtx.handleVolume = (flags & 2) != 0;
	}

	if (flags & (1 << 15)) {
		debug("Song has microtonal");
/*		if (loadScores) {
			for (int i = 0; i < ARRAYSIZE(_microtonal); ++i)
				_microtonal[i] = musicData.readUint16BE();
		} else*/
			musicData.skip(128 * 2);
	}

	int scoresLoaded = 0;
	// uint16 number of Scores
	const uint16 scoresInFile = musicData.readUint16BE();

	if (loadScores) {
		const uint16 tempScores = MIN(scoresInFile, _playerCtx.maxScoreNum);
		Score *curScore =_scores = new Score[tempScores];
		
		for (int i = tempScores; i > 0; --i, ++curScore) {
			const uint32 numEvents = musicData.readUint32BE();
			Event *curEvent = new Event[numEvents];
			curScore->events = curEvent;
			for (int j = numEvents; j > 0; --j, ++curEvent) {
				curEvent->command = musicData.readByte();
				curEvent->parameter = musicData.readByte();
				curEvent->startTime = musicData.readUint16BE();
				curEvent->stopTime = musicData.readUint16BE();
			}
			curScore->numEvents = numEvents;
		}
		_numScores = scoresLoaded = tempScores;
	}

	if (!loadSamples)
		return true;

	// skip over remaining scores in file
	for (int i = scoresInFile - scoresLoaded; i > 0; --i)
		musicData.skip(musicData.readUint32BE() * 6);

	// uint16 number of Samples
	const uint16 wavesInFile = musicData.readUint16BE();
	if (loadSamples) {
		for (int i = wavesInFile; i > 0; --i) {
			// load disksample structure
			const uint16 number = musicData.readUint16BE();
			assert(number < ARRAYSIZE(_patch));
			// pointer to samples needed?
			Patch &curPatch = const_cast<Patch &>(_patch[number]);

			curPatch.tune = musicData.readSint16BE();
			curPatch.volume = musicData.readUint16BE();
			curPatch.sampleOctaves = musicData.readUint16BE();
			curPatch.sampleAttackLen = musicData.readUint32BE();
			const uint32 sustainLen = musicData.readUint32BE();
			curPatch.sampleTotalLen = curPatch.sampleAttackLen + sustainLen;
			// each octave the number of samples doubles.
			const uint32 totalSamples = curPatch.sampleTotalLen * ((1 << curPatch.sampleOctaves) - 1);
			curPatch.attackLen = musicData.readUint16BE();
			curPatch.releaseLen = musicData.readUint16BE();
			const uint32 totalEnvs = curPatch.attackLen + curPatch.releaseLen;

			// Allocate space for both attack and release Segment.
			Envelope *envPtr = new Envelope[totalEnvs];
			// Attack Segment
			curPatch.attackPtr = envPtr;
			// Release Segment
			// curPatch.releasePtr = envPtr + curPatch.attackLen;

			// Read Attack and Release Segments
			for (int j = totalEnvs; j > 0; --j, ++envPtr) {
				envPtr->duration = musicData.readUint16BE();
				envPtr->volume = musicData.readUint16BE();
			}

			// read Samples
			int8 *allocSamples = new int8[totalSamples];
			curPatch.samplePtr = allocSamples;
			musicData.read(allocSamples, totalSamples);
		}
	} /* else if (wavesInFile > 0){ // only necessary if we need to consume the whole stream to point at end of data
		uint32 skipLen = 3 * 2;
		for (int i = wavesInFile; i > 0; --i) {
			musicData.skip(skipLen);
			const uint16 octaves = musicData.readUint16BE();
			const uint32 attackLen = musicData.readUint32BE();
			const uint32 sustainLen = musicData.readUint32BE();
			const uint16 attackCount = musicData.readUint16BE();
			const uint16 releaseCount = musicData.readUint16BE();
			
			skipLen = attackCount * 4 + releaseCount * 4 
				+ (attackLen + sustainLen) * ((1 << octaves) - 1)
				+ 3 * 2;
		}
		musicData.skip(skipLen - 3 * 2);
	} */
	return true;
}

#ifndef NDEBUG
void MaxTrax::outPutEvent(const Event &ev, int num) {
	struct {
		byte cmd;
		const char *name;
		const char *param;
	} COMMANDS[] = {
		{0x80, "TEMPO   ", "TEMPO, N/A      "},
		{0xa0, "SPECIAL ", "CHAN, SPEC # | VAL"},
		{0xb0, "CONTROL ", "CHAN, CTRL # | VAL"},
		{0xc0, "PROGRAM ", "CHANNEL, PROG # "},
		{0xe0, "BEND    ", "CHANNEL, BEND VALUE"},
		{0xf0, "SYSEX   ", "TYPE, SIZE      "},
		{0xf8, "REALTIME", "REALTIME, N/A   "},
		{0xff, "END     ", "N/A, N/A        "},
		{0xff, "NOTE    ", "VOL | CHAN, STOP"},
	};

	int i = 0;
	for (; i < ARRAYSIZE(COMMANDS) - 1 && ev.command != COMMANDS[i].cmd; ++i)
		;

	if (num == -1)
		debug("Event    : %02X %s %s %02X %04X %04X", ev.command, COMMANDS[i].name, COMMANDS[i].param, ev.parameter, ev.startTime, ev.stopTime);
	else
		debug("Event %3d: %02X %s %s %02X %04X %04X", num, ev.command, COMMANDS[i].name, COMMANDS[i].param, ev.parameter, ev.startTime, ev.stopTime);
}

void MaxTrax::outPutScore(const Score &sc, int num) {
	if (num == -1)
		debug("score   : %i Events", sc.numEvents);
	else
		debug("score %2d: %i Events", num, sc.numEvents);
	for (uint i = 0; i < sc.numEvents; ++i)
		outPutEvent(sc.events[i], i);
	debug("");
}
#else
void MaxTrax::outPutEvent(const Event &ev, int num) {}
void MaxTrax::outPutScore(const Score &sc, int num) {}
#endif	// #ifndef NDEBUG

}	// End of namespace Audio

#endif // #if defined(SOUND_MODS_MAXTRAX_H)