/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/md5.h"
#include "common/textconsole.h"
#include "mads/dragonsphere/sound/gsound.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

void GSoundChannel::reset(byte *startPtr) {
	_activeCount = 0;
	_pitchBendFadeStep = 0;
	_volumeFadeStep = 0;
	_panFadeStep = 0;
	_note = 0;
	_program = 0;
	_velocity = 0;
	_noteOffset = 0;
	_keyOnDelayOverride = 0;
	_keyOnDelay = 0;
	_volumeFadeCounter = 0;
	_volumeFadeReload = 0;
	_pitchBendFadeCounter = 0;
	_panFadeCounter = 0;
	_panFadeReload = 0;
	_pan = 0x40;
	_volume = 100;
	_pitchBend = 0x40;
	_pitchBendFadeReload = 0;
	_pitchBendFadeCount = 0;
	_loopStartPtr = startPtr;
	_pSrc = startPtr;
	_innerLoopPtr = startPtr;
	_outerLoopPtr = startPtr;
	_innerLoopCount = 0;
	_outerLoopCount = 0;
	_soundData = startPtr;
	_branchTarget = nullptr;
	_field24 = 0;
	_transpose = 0;
	_pendingStop = 0;
	_field27 = 0;
}

void GSoundChannel::load(byte *startPtr) {
	reset(startPtr);
	_activeCount = 1;
	_owner->sendPitchBend(_midiChannel, 0x40);
}

void GSoundChannel::enableFade(byte flag) {
	if (!_activeCount)
		return;
	_pendingStop = flag;
	_soundData = _owner->_silenceStream;
}

GSound::GSound(Audio::Mixer *mixer, const GSoundDriverData &driverData) :
		SoundDriver(mixer),
		_driverData(driverData), _midiDriver(nullptr),
		_driverCallbackDelta(0), _randomSeed(1234), _stateChanged(0),
		_callbackCounter(0), _callbackPeriod(0), _deferredCommand(-1),
		_musicIndex(0), _fadeCounter(0), _fadePeriod(0), _clockUnknown(0),
		_clockCoarseTarget(0), _clockMediumTarget(0), _clockFine(0),
		_clockCoarse(0), _clockMedium(0), _clockEnabled1(false),
		_clockEnabled2(false), _masterVolume(255), _isReady(false),
		_commandParam(0), _frameCounter(0), _tickCounter(0),
		_isDisabled(false), _pollResult(0) {
	if (!validateOverlay(driverData))
		return;

	Common::File file;
	if (!file.open(driverData.filename))
		return;
	_soundData.resize(driverData.dataSize);
	file.seek(0x200 + driverData.dataParagraph * 16);
	if (file.read(&_soundData[0], driverData.initializedDataSize) !=
			driverData.initializedDataSize) {
		_soundData.clear();
		return;
	}

	// SoundDriver reads the declared mutable image. The original file stores
	// only initialized bytes; reproduce DOS BSS deterministically.
	for (uint32 i = driverData.initializedDataSize; i < _soundData.size(); ++i)
		_soundData[i] = 0;

	_silenceStream[0] = 0;
	_silenceStream[1] = 0;
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i) {
		_channels[i]._owner = this;
		_channels[i]._midiChannel = i + 1;
		_channels[i].reset(_silenceStream);
	}
	for (int channel = 0; channel <= GSOUND_CHANNEL_COUNT; ++channel)
		for (int note = 0; note < GSOUND_HELD_NOTE_COUNT; ++note)
			_heldNotes[channel][note] = 0xFF;
	for (int i = 0; i < GSOUND_SCRIPT_VARIABLE_COUNT; ++i)
		_scriptVariables[i] = 0;

	_clockCoarse = 112;
	_clockMedium = 28;
	_clockFine = 7;

	_midiDriver = new MidiDriver_MT32GM(MusicType::MT_GM);
	const int result = _midiDriver->open();
	if (result) {
		warning("GSOUND failed to open the General MIDI driver (error %d)",
				result);
		delete _midiDriver;
		_midiDriver = nullptr;
		return;
	}

	_driverCallbackDelta = _midiDriver->getBaseTempo();
	resetMidiChannels();
	_midiDriver->setTimerCallback(this, &timerCallback);
	_isReady = true;
}

GSound::~GSound() {
	_isDisabled = true;
	if (!_midiDriver)
		return;

	_midiDriver->setTimerCallback(nullptr, nullptr);
	resetMidiChannels();
	_midiDriver->close();
	Common::StackLock lock(_driverMutex);
	delete _midiDriver;
	_midiDriver = nullptr;
}

bool GSound::validateOverlay(const GSoundDriverData &driverData) {
	Common::File file;
	if (!file.open(driverData.filename))
		return false;
	if ((uint32)file.size() != driverData.fileSize)
		return false;

	const Common::String md5 = Common::computeStreamMD5AsString(file, 8192);
	if (md5 != driverData.md5First8192)
		return false;

	file.seek(0x200 + 0x10);
	char identity[22];
	if (file.read(identity, 21) != 21)
		return false;
	identity[21] = 0;
	if (Common::String(identity) != "Dragon GM  N12-21-93")
		return false;

	file.seek(0x200 + 0x2A);
	if (file.readUint16LE() != driverData.dataParagraph ||
			file.readUint16LE() != driverData.dataSize ||
			file.readUint16LE() != 100 ||
			file.readUint16LE() != GSOUND_EXPORT_COUNT)
		return false;

	for (int i = 0; i < GSOUND_EXPORT_COUNT; ++i) {
		if (file.readUint16LE() != driverData.exports[i])
			return false;
	}

	const uint32 dataOffset = 0x200 + driverData.dataParagraph * 16;
	if (dataOffset + driverData.initializedDataSize != driverData.fileSize ||
			driverData.initializedDataSize > driverData.dataSize)
		return false;

	file.seek(0x200 + driverData.exports[4]);
	if (file.readByte() != 0xCB)
		return false;

	return true;
}

bool GSound::contains(const byte *ptr, uint32 count) const {
	if (_soundData.empty())
		return false;
	const byte *start = &_soundData[0];
	const byte *end = start + _soundData.size();
	return ptr >= start && ptr <= end && count <= (uint32)(end - ptr);
}

byte *GSound::dataAt(uint16 offset) {
	if (offset >= _soundData.size())
		error("GSOUND data offset 0x%04x is outside %s", offset,
				_driverData.filename);
	return &_soundData[offset];
}

int8 GSound::readSignedByte(byte *&pSrc) {
	return (int8)readByte(pSrc);
}

byte GSound::readByte(byte *&pSrc) {
	if (!contains(pSrc + 1))
		error("GSOUND bytecode read outside %s", _driverData.filename);
	return *++pSrc;
}

uint16 GSound::readWord(byte *&pSrc) {
	const byte low = readByte(pSrc);
	const byte high = readByte(pSrc);
	return low | (high << 8);
}

byte *GSound::readRoot(byte *&pSrc) {
	return dataAt(readWord(pSrc));
}

void GSound::sendNoteOn(int channel, int note, int velocity) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_NOTE_ON | (channel & 0x0F),
			note & 0x7F, velocity & 0x7F);
}

void GSound::sendProgramChange(int channel, int program) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PROGRAM_CHANGE |
			(channel & 0x0F), program & 0x7F, 0);
}

void GSound::sendControlChange(int channel, int controller, int value) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE |
			(channel & 0x0F), controller & 0x7F, value & 0x7F);
}

void GSound::sendPitchBend(int channel, int value) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PITCH_BEND |
			(channel & 0x0F), 0, value & 0x7F);
}

void GSound::sendVolume(GSoundChannel &channel) {
	if (!channel._pendingStop)
		sendControlChange(channel._midiChannel,
				MidiDriver::MIDI_CONTROLLER_VOLUME,
				scaleVolume(channel._volume));
}

int GSound::scaleVolume(int volume) const {
	return scaleMidiVolume(volume, _masterVolume);
}

void GSound::sendPan(GSoundChannel &channel) {
	sendControlChange(channel._midiChannel,
			MidiDriver::MIDI_CONTROLLER_PANNING, channel._pan);
}

void GSound::setPitchBendSensitivity(int channel, int semitones) {
	sendControlChange(channel, 101, 0);
	sendControlChange(channel, 100, 0);
	sendControlChange(channel, 6, semitones);
	sendControlChange(channel, 38, 0);
}

void GSound::resetMidiChannels() {
	// The original loops from native MIDI channel 9 through channel 0.
	for (int channel = 9; channel >= 0; --channel) {
		sendControlChange(channel, MidiDriver::MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
		sendControlChange(channel,
				MidiDriver::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS, 0);
		sendControlChange(channel, MidiDriver::MIDI_CONTROLLER_VOLUME,
				scaleVolume(100));
		sendControlChange(channel, MidiDriver::MIDI_CONTROLLER_PANNING, 64);
		sendControlChange(channel, 91, 0);
		sendControlChange(channel, 93, 0);
		setPitchBendSensitivity(channel, 2);
	}
}

void GSound::flushHeldNotes(GSoundChannel &channel) {
	byte *notes = _heldNotes[channel._midiChannel];
	for (int i = 0; i < GSOUND_HELD_NOTE_COUNT; ++i) {
		if (notes[i] != 0xFF) {
			sendNoteOn(channel._midiChannel, notes[i], 0);
			notes[i] = 0xFF;
		}
	}
}

void GSound::resetChannels() {
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i)
		_channels[i].reset(_silenceStream);
	for (int channel = 0; channel <= GSOUND_CHANNEL_COUNT; ++channel)
		for (int note = 0; note < GSOUND_HELD_NOTE_COUNT; ++note)
			_heldNotes[channel][note] = 0xFF;
}

void GSound::resetMusicChannels() {
	// Native command 2 resets channels 1-6 and 9.
	for (int i = 0; i < 6; ++i)
		_channels[i].reset(_silenceStream);
	_channels[8].reset(_silenceStream);
}

void GSound::resetEffectChannels() {
	_channels[6].reset(_silenceStream);
	_channels[7].reset(_silenceStream);
}

void GSound::stopMusicChannels() {
	_fadePeriod = 1;
	for (int i = 0; i < 6; ++i)
		_channels[i].enableFade(0xFF);
	_channels[8].enableFade(0xFF);
}

void GSound::stopEffectChannels() {
	_fadePeriod = 1;
	_channels[6].enableFade(0xFF);
	_channels[7].enableFade(0xFF);
}

bool GSound::anyChannelActive() const {
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i) {
		if (_channels[i]._activeCount)
			return true;
	}
	return false;
}

bool GSound::isSoundActive(uint16 offset) const {
	if (offset >= _soundData.size())
		return false;
	const byte *root = &_soundData[offset];
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i) {
		if (_channels[i]._activeCount && _channels[i]._soundData == root)
			return true;
	}
	return false;
}

bool GSound::channelPlays(int channel, uint16 offset) const {
	if (channel < 1 || channel > GSOUND_CHANNEL_COUNT ||
			offset >= _soundData.size())
		return false;
	const GSoundChannel &state = _channels[channel - 1];
	return state._activeCount && state._soundData == &_soundData[offset];
}

GSoundChannel *GSound::allocateChannel(uint16 offset, int first, int last) {
	byte *root = dataAt(offset);
	for (int i = first; i <= last; ++i) {
		if (!_channels[i]._activeCount) {
			_channels[i].load(root);
			return &_channels[i];
		}
	}
	for (int i = last; i >= first; --i) {
		if (_channels[i]._pendingStop == 0xFF) {
			_channels[i].load(root);
			return &_channels[i];
		}
	}
	return nullptr;
}

GSoundChannel *GSound::playEffect78(uint16 offset) {
	return allocateChannel(offset, 6, 7);
}

GSoundChannel *GSound::playEffectAny(uint16 offset) {
	return allocateChannel(offset, 0, 7);
}

GSoundChannel *GSound::playEffectChannel8(uint16 offset) {
	return allocateChannel(offset, 7, 7);
}

void GSound::loadChannel(int channel, uint16 offset) {
	if (channel < 1 || channel > GSOUND_CHANNEL_COUNT)
		error("Invalid GSOUND channel %d", channel);
	_channels[channel - 1].load(dataAt(offset));
}

void GSound::loadRoots(const GSoundCommandSpec &spec) {
	for (int i = 0; i < spec.rootCount; ++i)
		loadChannel(spec.roots[i].channel, spec.roots[i].offset);
}

void GSound::armTimer(uint16 counter, uint16 period) {
	_deferredCommand = -1;
	_callbackCounter = counter;
	_callbackPeriod = period;
}

void GSound::deferCommand(int command) {
	_deferredCommand = command;
}

void GSound::scheduleSpecial(int command, uint16 counter, uint16 period) {
	armTimer(counter, period);
	deferCommand(command);
}

int GSound::executeSpec(const GSoundCommandSpec &spec, bool fromDeferred) {
	if (!fromDeferred && (spec.flags & kGSoundCheckFirstRoot)) {
		for (int i = 0; i < spec.guardCount; ++i) {
			if (isSoundActive(spec.guards[i]))
				return 0;
		}
	}

	if (!fromDeferred && (spec.flags & kGSoundDeferWhileActive) &&
			anyChannelActive()) {
		deferCommand(spec.command);
		return 0;
	}

	if (spec.timerCounter || spec.timerPeriod)
		armTimer(spec.timerCounter, spec.timerPeriod);
	if (spec.flags & kGSoundResetAll)
		command0();
	else if (spec.flags & kGSoundStopAll)
		command1();
	else if (spec.flags & kGSoundStopMusic)
		command3();
	if (spec.flags & kGSoundSetMusicIndex)
		_musicIndex = spec.musicIndex;

	switch (spec.mode) {
	case kGSoundNoOp:
		break;
	case kGSoundMusic:
	case kGSoundDirectMusic:
	case kGSoundDirectChannels:
		loadRoots(spec);
		break;
	case kGSoundEffect78:
		for (int i = 0; i < spec.rootCount; ++i)
			playEffect78(spec.roots[i].offset);
		break;
	case kGSoundEffectAny:
		for (int i = 0; i < spec.rootCount; ++i)
			playEffectAny(spec.roots[i].offset);
		break;
	case kGSoundEffectChannel8:
		for (int i = 0; i < spec.rootCount; ++i)
			playEffectChannel8(spec.roots[i].offset);
		break;
	case kGSoundSpecial:
		if (!executeSpecialCommand(spec.command, fromDeferred))
			error("Unknown special GSOUND command %d in section %d",
					spec.command, _driverData.section);
		break;
	}

	return 0;
}

int GSound::executeCommand(int command, int param) {
	_commandParam = param;
	switch (command) {
	case 0: return command0();
	case 1: return command1();
	case 2: return command2();
	case 3: return command3();
	case 4: return command4();
	case 5: return command5();
	case 6: return command6();
	case 7: return command7();
	case 8: return command8();
	case 18:
		// GSOUND.DR9 maps command 18 directly to a return stub. Unlike
		// DR1-DR6, its dispatcher does not retain a music selector either.
		if (_driverData.section == 9)
			return 0;
		command3();
		if (_musicIndex != 18)
			return executeCommand(_musicIndex, 0);
		return 0;
	default:
		break;
	}

	const GSoundCommandSpec *spec = findCommandSpec(command);
	return spec ? executeSpec(*spec, false) : 0;
}

int GSound::command0() {
	_deferredCommand = -1;
	_callbackCounter = 0;
	_callbackPeriod = 0;
	resetChannels();
	resetMidiChannels();
	return 0;
}

int GSound::command1() {
	command3();
	command5();
	return 0;
}

int GSound::command2() {
	resetMusicChannels();
	return 0;
}

int GSound::command3() {
	stopMusicChannels();
	return 0;
}

int GSound::command4() {
	resetEffectChannels();
	return 0;
}

int GSound::command5() {
	stopEffectChannels();
	return 0;
}

int GSound::command6() {
	_isDisabled = true;
	for (int channel = 1; channel <= GSOUND_CHANNEL_COUNT; ++channel)
		sendControlChange(channel, MidiDriver::MIDI_CONTROLLER_VOLUME, 0);
	return 0;
}

int GSound::command7() {
	_isDisabled = false;
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i)
		sendVolume(_channels[i]);
	return 0;
}

int GSound::command8() {
	int result = 0;
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i)
		result |= _channels[i]._activeCount;
	return result;
}

void GSound::tickDeferredCommand() {
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter)
		return;

	_callbackCounter = _callbackPeriod;
	if (_deferredCommand < 0)
		return;

	const int command = _deferredCommand;
	_deferredCommand = -1;
	const GSoundCommandSpec *spec = findCommandSpec(command);
	if (spec)
		executeSpec(*spec, true);
	else if (!executeSpecialCommand(command, true))
		error("Unknown deferred GSOUND command %d", command);
}

void GSound::checkFade(GSoundChannel &channel) {
	if (!channel._activeCount || !channel._pendingStop)
		return;
	if (channel._volume <= 0) {
		flushHeldNotes(channel);
		channel._pSrc = _silenceStream;
		channel._pendingStop = 0;
		return;
	}
	--channel._volume;
	sendControlChange(channel._midiChannel,
			MidiDriver::MIDI_CONTROLLER_VOLUME,
			scaleVolume(channel._volume));
}

void GSound::checkFades() {
	if (!_fadePeriod || --_fadeCounter > 0)
		return;
	_fadeCounter = _fadePeriod;
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i)
		checkFade(_channels[i]);
}

void GSound::applyFades(GSoundChannel &channel) {
	if (channel._volumeFadeStep && --channel._volumeFadeCounter == 0) {
		channel._volumeFadeCounter = channel._volumeFadeReload;
		channel._volume += channel._volumeFadeStep;
		if ((byte)channel._volume > 0x7F) {
			channel._volumeFadeStep = 0;
			channel._volume = ((byte)channel._volume > 0xAF) ? 0 : 0x7F;
		}
		sendVolume(channel);
	}
	if (channel._pitchBendFadeStep) {
		if (--channel._pitchBendFadeCounter == 0) {
			channel._pitchBendFadeCounter = channel._pitchBendFadeReload;
			channel._pitchBend += channel._pitchBendFadeStep;
			sendPitchBend(channel._midiChannel, channel._pitchBend);
		}
		if (--channel._pitchBendFadeCount == 0)
			channel._pitchBendFadeStep = 0;
	}
	if (channel._panFadeStep && --channel._panFadeCounter == 0) {
		channel._panFadeCounter = channel._panFadeReload;
		channel._pan += channel._panFadeStep;
		sendPan(channel);
	}
}

void GSound::pollChannel(GSoundChannel &channel) {
	if (!channel._activeCount)
		return;

	if (channel._keyOnDelay && --channel._keyOnDelay == 0)
		flushHeldNotes(channel);
	if (--channel._activeCount)
		goto fades;

dispatch:
	if (!contains(channel._pSrc))
		error("GSOUND channel stream escaped %s", _driverData.filename);

	{
		byte *pSrc = channel._pSrc;
		const byte opcode = *pSrc;
		const int midiChannel = channel._midiChannel;

		if (opcode <= 0xBB) {
			if (!contains(pSrc, 2))
				error("Truncated GSOUND note event in %s", _driverData.filename);
			const int note = (int8)pSrc[0] + channel._transpose;
			const int duration = pSrc[1];
			channel._activeCount = duration;
			channel._pSrc += 2;

			if (note != channel._note)
				flushHeldNotes(channel);
			if (!duration)
				return;
			if (note) {
				channel._keyOnDelay = channel._keyOnDelayOverride ?
						channel._keyOnDelayOverride :
						channel._activeCount - channel._noteOffset;
				if (!(channel._noteOffset < 0 &&
						_heldNotes[midiChannel][0] == (byte)note)) {
					channel._note = note;
					_heldNotes[midiChannel][0] = note;
					sendNoteOn(midiChannel, note, channel._velocity);
				}
			}
			goto fades;
		}

		switch (opcode) {
		case 0xBC:
			setPitchBendSensitivity(midiChannel, readByte(pSrc));
			channel._pSrc += 2;
			goto dispatch;
		case 0xBD:
			_clockUnknown = readByte(pSrc);
			channel._pSrc += 2;
			goto dispatch;
		case 0xBE:
			_clockCoarseTarget = readWord(pSrc);
			if (!_tickCounter)
				_clockCoarse = _clockCoarseTarget;
			_clockEnabled1 = true;
			_clockEnabled2 = true;
			channel._pSrc += 3;
			goto dispatch;
		case 0xBF:
			_clockMediumTarget = readByte(pSrc);
			if (!_tickCounter)
				_clockMedium = _clockMediumTarget;
			channel._pSrc += 2;
			goto dispatch;
		case 0xC0:
			_clockFine = readByte(pSrc);
			channel._pSrc += 2;
			goto dispatch;
		case 0xC1:
			sendControlChange(midiChannel, 93, readByte(pSrc));
			channel._pSrc += 2;
			goto dispatch;
		case 0xC2:
			sendControlChange(midiChannel, 91, readByte(pSrc));
			channel._pSrc += 2;
			goto dispatch;
		case 0xC3:
			readByte(pSrc);
			channel._pSrc += 2;
			goto dispatch;
		case 0xC4: {
			const uint16 target = readWord(pSrc);
			if (!executeNativeCallback(target, channel))
				error("Unknown Dragonsphere GSOUND callback 0x%04x in %s",
						target, _driverData.filename);
			channel._pSrc += 3;
			goto dispatch;
		}
		case 0xC5: case 0xC6: case 0xC7: case 0xC8:
		case 0xC9: case 0xCA: case 0xCB: case 0xCC:
		case 0xCD: case 0xCE: case 0xCF: case 0xD0:
		case 0xD1: case 0xD2: case 0xD3: case 0xD4: {
			const bool saveReturn = opcode <= 0xCC;
			const bool variableRhs = (opcode <= 0xC8) ||
					(opcode >= 0xCD && opcode <= 0xD0);
			const byte lhsIndex = readByte(pSrc);
			const byte rhsValue = readByte(pSrc);
			const byte lhs = _scriptVariables[lhsIndex & 0x1F];
			const byte rhs = variableRhs ?
					_scriptVariables[rhsValue & 0x1F] : rhsValue;
			const int condition = (opcode -
					(saveReturn ? 0xC5 : 0xCD)) & 3;
			bool take = false;
			switch (condition) {
			case 0: take = lhs > rhs; break;
			case 1: take = lhs < rhs; break;
			case 2: take = lhs != rhs; break;
			case 3: take = lhs == rhs; break;
			}
			if (take) {
				if (saveReturn)
					channel._branchTarget = channel._pSrc + 5;
				channel._pSrc = readRoot(pSrc);
			} else {
				channel._pSrc += 5;
			}
			goto dispatch;
		}
		case 0xD5: case 0xD6: case 0xD7: case 0xD8:
		case 0xD9: case 0xDA: case 0xDB: case 0xDC:
		case 0xDD: case 0xDE: case 0xDF: case 0xE0:
		case 0xE1: case 0xE2: case 0xE3: case 0xE4: {
			const byte lhsIndex = readByte(pSrc) & 0x1F;
			const byte rhsByte = readByte(pSrc);
			const bool variableRhs = (opcode & 1) != 0;
			const byte rhs = variableRhs ?
					_scriptVariables[rhsByte & 0x1F] : rhsByte;
			byte &lhs = _scriptVariables[lhsIndex];
			lhs = applyArithmetic(opcode, lhs, rhs);
			channel._pSrc += 3;
			goto dispatch;
		}
		case 0xE5:
			--_scriptVariables[readByte(pSrc) & 0x1F];
			channel._pSrc += 2;
			goto dispatch;
		case 0xE6:
			++_scriptVariables[readByte(pSrc) & 0x1F];
			channel._pSrc += 2;
			goto dispatch;
		case 0xE7: {
			const byte variable = readByte(pSrc) & 0x1F;
			const int8 displacement = readSignedByte(pSrc);
			byte *target = pSrc + 1 + displacement;
			if (!contains(target))
				error("GSOUND E7 write outside %s", _driverData.filename);
			*target = _scriptVariables[variable];
			channel._pSrc += 3;
			goto dispatch;
		}
		case 0xE8: {
			const byte destination = readByte(pSrc) & 0x1F;
			const byte source = readByte(pSrc) & 0x1F;
			_scriptVariables[destination] = _scriptVariables[source];
			channel._pSrc += 3;
			goto dispatch;
		}
		case 0xE9: {
			const byte destination = readByte(pSrc) & 0x1F;
			_scriptVariables[destination] = readByte(pSrc);
			channel._pSrc += 3;
			goto dispatch;
		}
		case 0xEA: {
			const byte variable = readByte(pSrc) & 0x1F;
			const byte length = readByte(pSrc);
			byte *table = pSrc + 1;
			if (!contains(table, length + 2))
				error("Truncated GSOUND EA table in %s", _driverData.filename);
			const byte value = table[_scriptVariables[variable]];
			const int8 displacement = (int8)table[length];
			byte *target = table + length + 1 + displacement;
			if (!contains(target))
				error("GSOUND EA write outside %s", _driverData.filename);
			*target = value;
			channel._pSrc += length + 4;
			goto dispatch;
		}
		case 0xEB: {
			const byte low = readByte(pSrc);
			const byte high = readByte(pSrc);
			const int range = high - low + 1;
			const byte value = low + (range ? _randomSeed % range : 0);
			const int8 displacement = readSignedByte(pSrc);
			byte *target = pSrc + 1 + displacement;
			if (!contains(target))
				error("GSOUND EB write outside %s", _driverData.filename);
			*target = value;
			channel._pSrc += 4;
			goto dispatch;
		}
		case 0xEC: {
			const byte length = readByte(pSrc);
			byte *table = pSrc + 1;
			if (!contains(table, length + 1))
				error("Truncated GSOUND EC table in %s", _driverData.filename);
			const byte value = table[length ? _randomSeed % length : 0];
			const int8 displacement = (int8)table[length];
			byte *target = table + length + 1 + displacement;
			if (!contains(target))
				error("GSOUND EC write outside %s", _driverData.filename);
			*target = value;
			channel._pSrc += length + 3;
			goto dispatch;
		}
		case 0xED: {
			const int count = readByte(pSrc);
			if (count > GSOUND_HELD_NOTE_COUNT)
				error("GSOUND chord has %d notes", count);
			if (!contains(pSrc + 1, count + 1))
				error("Truncated GSOUND chord in %s", _driverData.filename);
			const int first = (int8)pSrc[1] + channel._transpose;
			if (first != _heldNotes[midiChannel][0])
				flushHeldNotes(channel);
			int i = 0;
			for (; i < count; ++i) {
				const int note = (int8)pSrc[1 + i] + channel._transpose;
				if (!(channel._noteOffset < 0 &&
						_heldNotes[midiChannel][i] == (byte)note)) {
					_heldNotes[midiChannel][i] = note;
					sendNoteOn(midiChannel, note, channel._velocity);
				}
			}
			for (; i < GSOUND_HELD_NOTE_COUNT; ++i)
				_heldNotes[midiChannel][i] = 0xFF;
			channel._activeCount = pSrc[1 + count];
			channel._keyOnDelay = channel._keyOnDelayOverride ?
					channel._keyOnDelayOverride :
					channel._activeCount - channel._noteOffset;
			channel._pSrc += count + 3;
			goto fades;
		}
		case 0xEE:
			channel._transpose = readSignedByte(pSrc);
			channel._pSrc += 2;
			goto dispatch;
		case 0xEF:
			channel._panFadeReload = readByte(pSrc);
			channel._panFadeStep = readSignedByte(pSrc);
			channel._panFadeCounter = 1;
			channel._pSrc += 3;
			goto dispatch;
		case 0xF0:
			channel._pan = readByte(pSrc);
			channel._pSrc += 2;
			sendPan(channel);
			goto dispatch;
		case 0xF1:
			channel._pitchBend = readByte(pSrc);
			channel._pSrc += 2;
			sendPitchBend(midiChannel, channel._pitchBend);
			goto dispatch;
		case 0xF2:
			channel._volumeFadeReload = readByte(pSrc);
			channel._volumeFadeStep = readSignedByte(pSrc);
			channel._volumeFadeCounter = 1;
			channel._pSrc += 3;
			goto dispatch;
		case 0xF3:
			channel._velocity = readByte(pSrc);
			channel._pSrc += 2;
			goto dispatch;
		case 0xF4:
			channel._volume = readByte(pSrc);
			channel._pSrc += 2;
			sendVolume(channel);
			goto dispatch;
		case 0xF5:
			channel._pitchBendFadeReload = readByte(pSrc);
			channel._pitchBendFadeStep = readSignedByte(pSrc);
			channel._pitchBendFadeCount = readByte(pSrc);
			channel._pitchBendFadeCounter = 1;
			channel._pSrc += 4;
			goto dispatch;
		case 0xF6:
			channel._keyOnDelayOverride = readByte(pSrc);
			channel._noteOffset = 0;
			channel._pSrc += 2;
			goto dispatch;
		case 0xF7:
			channel._noteOffset = readSignedByte(pSrc);
			channel._keyOnDelayOverride = 0;
			channel._pSrc += 2;
			goto dispatch;
		case 0xF8:
			channel._program = readByte(pSrc);
			channel._pSrc += 2;
			sendProgramChange(midiChannel, channel._program);
			goto dispatch;
		case 0xF9:
			if (channel._branchTarget) {
				channel._pSrc = channel._branchTarget;
				channel._branchTarget = nullptr;
			} else {
				++channel._pSrc;
			}
			goto dispatch;
		case 0xFA:
			channel._branchTarget = channel._pSrc + 3;
			channel._pSrc = readRoot(pSrc);
			goto dispatch;
		case 0xFB:
			channel._pSrc = readRoot(pSrc);
			goto dispatch;
		case 0xFC: {
			byte *root = readRoot(pSrc);
			channel._loopStartPtr = root;
			channel._pSrc = root;
			channel._innerLoopPtr = root;
			channel._outerLoopPtr = root;
			channel._soundData = root;
			goto fades;
		}
		case 0xFD:
			if (channel._soundData == nullptr) {
				channel._pSrc = channel._loopStartPtr;
			} else {
				channel._loopStartPtr = channel._soundData;
				channel._pSrc = channel._soundData;
				channel._innerLoopPtr = channel._soundData;
				channel._outerLoopPtr = channel._soundData;
			}
			goto fades;
		case 0xFE: {
			if (!channel._outerLoopCount) {
				channel._outerLoopCount = readSignedByte(pSrc);
				if (!channel._outerLoopCount) {
					channel._pSrc += 2;
					channel._outerLoopPtr = channel._pSrc;
					channel._innerLoopCount = 0;
					channel._outerLoopCount = 0;
				} else {
					channel._pSrc = channel._outerLoopPtr;
				}
			} else if (--channel._outerLoopCount == 0) {
				channel._pSrc += 2;
				channel._outerLoopPtr = channel._pSrc;
			} else {
				channel._pSrc = channel._outerLoopPtr;
			}
			channel._innerLoopPtr = channel._pSrc;
			goto dispatch;
		}
		case 0xFF: {
			if (!channel._innerLoopCount) {
				channel._innerLoopCount = readSignedByte(pSrc);
				if (!channel._innerLoopCount) {
					channel._pSrc += 2;
					channel._innerLoopPtr = channel._pSrc;
				} else {
					channel._pSrc = channel._innerLoopPtr;
				}
			} else if (--channel._innerLoopCount == 0) {
				channel._pSrc += 2;
				channel._innerLoopPtr = channel._pSrc;
			} else {
				channel._pSrc = channel._innerLoopPtr;
			}
			goto dispatch;
		}
		default:
			error("Unknown GSOUND opcode 0x%02x in %s", opcode,
					_driverData.filename);
		}
	}

fades:
	applyFades(channel);
}

void GSound::update() {
	uint16 value = _randomSeed + 0x9249;
	_randomSeed = (value >> 3) | (value << 13);
	if (_isDisabled)
		return;

	++_frameCounter;
	++_tickCounter;
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i)
		pollChannel(_channels[i]);
	tickDeferredCommand();
	checkFades();
}

void GSound::onTimer() {
	Common::StackLock lock(_driverMutex);
	uint32 serviceTicks = _hostTimer.advance(_driverCallbackDelta, 1000000);
	while (serviceTicks--) {
		// Export 4 is an immediate RETF in every validated GSOUND overlay.
		if (_hostTimer.pollDue())
			poll();
	}
}

void GSound::timerCallback(void *data) {
	static_cast<GSound *>(data)->onTimer();
}

int GSound::stop() {
	return command0();
}

int GSound::poll() {
	update();
	const int result = _pollResult;
	_pollResult = 0;
	return result;
}

void GSound::setVolume(int volume) {
	_masterVolume = CLIP(volume, 0, 255);
	for (int i = 0; i < GSOUND_CHANNEL_COUNT; ++i) {
		if (_isDisabled)
			sendControlChange(_channels[i]._midiChannel,
					MidiDriver::MIDI_CONTROLLER_VOLUME, 0);
		else
			sendVolume(_channels[i]);
	}
}

void GSound::setDataByte(uint16 offset, byte value) {
	*dataAt(offset) = value;
}

byte GSound::getDataByte(uint16 offset) const {
	if (offset >= _soundData.size())
		error("GSOUND data offset 0x%04x is outside %s", offset,
				_driverData.filename);
	return _soundData[offset];
}

void GSound::setScriptVariable(byte index, byte value) {
	_scriptVariables[index & 0x1F] = value;
}

byte GSound::getScriptVariable(byte index) const {
	return _scriptVariables[index & 0x1F];
}

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS
