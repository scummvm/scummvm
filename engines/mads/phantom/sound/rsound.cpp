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
#include "mads/phantom/sound/rsound.h"

namespace MADS {
namespace Phantom {
namespace Sound {

/*-----------------------------------------------------------------------*/

void Channel::reset(byte *startPtr) {
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
	_pitchBendFadeReload = 0;
	_pitchBendFadeCount = 0;
	_innerLoopCount = 0;
	_outerLoopCount = 0;
	_branchTarget = nullptr;
	_transpose = 0;
	_pendingStop = 0;
	_volume = 0;

	_loopStartPtr = startPtr;
	_pSrc = startPtr;
	_innerLoopPtr = startPtr;
	_outerLoopPtr = startPtr;
	_soundData = startPtr;
	_pan = 0x40;
	_pitchBend = 0x40;
}

void Channel::enable(int flag) {
	if (_activeCount) {
		_pendingStop = flag;
		// Matches Channel_enable's "mov [bx+20h], ax" - invalidates the
		// identity pointer so isSoundActive() no longer matches this
		// channel. The original writes register garbage (AH was never
		// explicitly cleared); nullptr is the correct semantic
		// equivalent without inheriting undefined register state.
		_soundData = nullptr;
	}
}

void Channel::load(byte *pData) {
	reset(pData);
	_activeCount = 1;
	_owner->resetPitchBend(_midiChannel);
}

/*-----------------------------------------------------------------------*/

RSound::RSound(Audio::Mixer *mixer, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset) : SoundDriver(mixer, filename, dataOffset, dataSize) {
	_commandParam = 0;
	_frameCounter = 0;
	_isDisabled = false;
	_randomSeed = 1234;
	_stateChangedFlag = 0;
	_pollResult = 0;
	_sysExOffset = sysExOffset;
	_fadeCheckCounter = 0;
	_fadeCheckPeriod = 0;

	_tickCounter = 0;
	_clockMedTarget = 0;
	_clockCoarseTarget = 0;
	_clockUnknown = 0;
	_clockCoarse = 112;
	_clockMed = 28;
	_clockFine = 7;
	_clockEnabled1 = 0;
	_clockEnabled2 = 0;
	_randomAmbianceTriggerFlag = 0;

	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i) {
		_channels[i]._owner = this;
		_channels[i]._midiChannel = i + 1;
	}

	for (int i = 0; i <= RSOUND_CHANNEL_COUNT; ++i)
		for (int j = 0; j < 4; ++j)
			_heldNotes[i][j] = 0xFF;

	for (int i = 0; i < ARRAYSIZE(_scriptVariables); ++i)
		_scriptVariables[i] = 0;

	_midiDriver = new MidiDriver_MT32GM(MusicType::MT_MT32);
	const int returnCode = _midiDriver->open();
	if (returnCode != 0)
		error("RSound - Failed to open MIDI music driver - error code %d.", returnCode);

	_driverCallbackDelta = _midiDriver->getBaseTempo();

	// Matches initDeviceOnce: command0() then sendSysExSequence(). The
	// disassembly's _deviceInitialized guard flag is omitted - this
	// constructor only ever runs once per driver instance, so there's
	// nothing to guard against.
	command0();
	sendSysExSequence();

	_midiDriver->setTimerCallback(this, &timerCallback);
}

RSound::~RSound() {
	_isDisabled = true;
	if (_midiDriver) {
		_midiDriver->setTimerCallback(nullptr, nullptr);
		_midiDriver->close();

		Common::StackLock lock(_driverMutex);
		delete _midiDriver;
		_midiDriver = nullptr;
	}
}

void RSound::validate() {
	Common::File f;
	static const char *const MD5[] = {
		"d6a64de63e58d9aceadc4a0ad6bedff7",
		"8a89af7bc6086a99c84455305e8659b3",
		"8f68ee5787d21764d6c565f79eca2505",
		"95de8817e92edc61dbb40a16adc0baa2",
		"9cf8f6e744eaa43a1e1d4855da77e7c1",
		nullptr,
		nullptr,
		nullptr,
		"97a4e9b701d8de4322b9ef3687497b17"
	};

	for (int i = 1; i <= 9; ++i) {
		if (i >= 6 && i <= 8)
			continue;
		Common::Path filename(Common::String::format("rsound.ph%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != MD5[i - 1])
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

int RSound::stop() {
	command0();
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

int RSound::poll() {
	update();
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

void RSound::onTimer() {
	Common::StackLock lock(_driverMutex);

	uint32 serviceTicks = _hostTimer.advance(_driverCallbackDelta, 1000000);
	while (serviceTicks--) {
		// Export 4 is a return stub in every audited Phantom RSOUND overlay.
		if (_hostTimer.pollDue())
			poll();
	}
}

void RSound::timerCallback(void *data) {
	static_cast<RSound *>(data)->onTimer();
}

void RSound::setVolume(int volume) {
	_masterVolume = CLIP(volume, 0, 255);
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		sendVolumeCC(i + 1, _isDisabled ? 0 : _channels[i]._volume);
}

void RSound::resultCheck() {
	// Matches the state-changed-flag latch right after rsound_update.
	if (_stateChangedFlag != 0xFFFF) {
		_stateChangedFlag = 0xFFFF;
		_pollResult = 0xFFFF;
	}
}

/*-----------------------------------------------------------------------*/

int RSound::getRandomNumber() {
	uint16 val = _randomSeed + 0x9249;
	val = (val >> 3) | (val << 13); // matches three "ror ax,1" in a row
	_randomSeed = val;
	return val;
}

Channel *RSound::playSoundData(byte *pData, int startingChannel, int freeScanEnd, int fallbackScanEnd) {
	for (int i = startingChannel; i <= freeScanEnd; ++i) {
		if (!_channels[i]._activeCount) {
			_channels[i].load(pData);
			return &_channels[i];
		}
	}

	for (int i = fallbackScanEnd; i >= startingChannel; --i) {
		if (_channels[i]._pendingStop == 0xFF) {
			_channels[i].load(pData);
			return &_channels[i];
		}
	}

	return nullptr;
}

Channel *RSound::playSound(int offset) {
	// Channels 6-8 (0-based 5-7), symmetric free/fallback scan.
	return playSoundData(loadData(offset), 5, 7, 7);
}

Channel *RSound::playSoundChannels1To5(int offset) {
	// Channels 1-5 (0-based 0-4) for the free scan, but the pending-stop
	// fallback only reaches down to channel 4 (0-based 3) - channel 5
	// can never be pre-empted here, confirmed from the disassembly.
	return playSoundData(loadData(offset), 0, 4, 3);
}

Channel *RSound::playSoundAny(int offset) {
	// Channels 1-8 (0-based 0-7), symmetric free/fallback scan.
	return playSoundData(loadData(offset), 0, 7, 7);
}

Channel *RSound::playSoundChannels5To8(int offset) {
	// Channels 5-8 (0-based 4-7), symmetric free/fallback scan.
	return playSoundData(loadData(offset), 4, 7, 7);
}

Channel *RSound::playSoundChannels1To6(int offset) {
	// Channels 1-6 (0-based 0-5), symmetric free/fallback scan.
	return playSoundData(loadData(offset), 0, 5, 5);
}

bool RSound::isSoundActive(byte *pData) {
	// Matches the disassembly exactly: only channels 1-8 are checked,
	// channel 9 is never included.
	for (int i = 0; i < RSOUND_CHANNEL_COUNT - 1; ++i) {
		if (_channels[i]._activeCount && _channels[i]._soundData == pData)
			return true;
	}
	return false;
}

/*-----------------------------------------------------------------------*/

void RSound::sendNoteOn(int midiChannel, int note, int velocity) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_NOTE_ON | midiChannel,
		note, velocity);
}

void RSound::sendProgramChange(int midiChannel, int program) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PROGRAM_CHANGE | midiChannel,
		program, 0);
}

void RSound::sendVolume(int midiChannel, int volume) {
	const int scaledVolume = CLIP(volume, 0, 127) * _masterVolume / 255;
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
		MidiDriver::MIDI_CONTROLLER_VOLUME, scaledVolume);
}

void RSound::sendVolumeCC(int midiChannel, int volume) {
	// The original emits an unconditional status byte here. Structured MIDI
	// messages do not retain running status, so this is equivalent to the
	// ordinary volume helper.
	sendVolume(midiChannel, volume);
}

void RSound::sendPitchBend(int midiChannel, int value) {
	// The original only uses the coarse (MSB) component.
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PITCH_BEND | midiChannel,
		0, value);
}

void RSound::resetPitchBend(int midiChannel) {
	sendPitchBend(midiChannel, 0x40);
}

void RSound::sendPan(int midiChannel, int value) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
		MidiDriver::MIDI_CONTROLLER_PANNING, value);
}

void RSound::muteChannel(int midiChannel) {
	sendVolume(midiChannel, 0);
}

void RSound::sendGmReset(int count) {
	// Counts DOWN from count to 1, using the counter
	// itself as the MIDI channel number each iteration.
	for (int midiChannel = count; midiChannel >= 1; --midiChannel) {
		_fadeCheckPeriod = 0; // reset at the top of every iteration

		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
			MidiDriver::MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
			MidiDriver::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS, 0);
		sendVolume(midiChannel, 100);
		sendPan(midiChannel, 0x40);
	}
}

const byte *RSound::sendSysExData(const byte *pData, uint maxLength) {
	uint length = 0;
	while (length < maxLength && pData[length] != 0xFF)
		++length;

	if (length == maxLength) {
		warning("RSound::sendSysExData: unterminated SysEx message");
		return nullptr;
	}

	_midiDriver->sysExMT32(pData, length);
	return &pData[length];
}

const byte *RSound::sendSysEx(int offset) {
	if (offset < 0 || (uint)offset >= _soundData.size()) {
		warning("RSound::sendSysEx: offset %d is outside the sound data", offset);
		return nullptr;
	}

	return sendSysExData(loadData(offset), _soundData.size() - offset);
}

void RSound::sendSysExSequence() {
	const byte *pData = sendSysEx(_sysExOffset);
	if (!pData)
		return;

	const byte *const dataEnd = _soundData.end();
	for (;;) {
		++pData;
		if (pData == dataEnd) {
			warning("RSound::sendSysExSequence: unterminated SysEx sequence");
			return;
		}
		if (*pData == 0xFF)
			break;

		pData = sendSysExData(pData, dataEnd - pData);
		if (!pData)
			return;
	}
}

void RSound::restorePatchMemory() {
	// Native export 2 uses this during teardown. Four blocks restore the
	// complete MT-32 Patch Memory table; each block contains 32 records.
	byte base = 0;
	for (int outer = 0; outer < 4; ++outer) {
		byte message[3 + 32 * 8];
		uint16 length = 0;
		message[length++] = 5;
		message[length++] = (byte)(outer << 1);
		message[length++] = 0;

		for (int inner = 0; inner < 0x20; ++inner) {
			message[length++] = (byte)(outer >> 1);
			message[length++] = (byte)(inner + base);
			static const byte tail[] = { 0x18, 0x32, 0x0C, 0, 1, 0 };
			for (int t = 0; t < ARRAYSIZE(tail); ++t)
				message[length++] = tail[t];
		}

		_midiDriver->sysExMT32(message, length);

		base = (byte)((base + 0x20) & 0x3F);
	}
}

void RSound::sendReverbSysEx(int mode, int time, int level) {
	// CONFIRMED: the fixed 3-byte Roland address 10 00 01h is the real
	// MT-32 System Area Reverb parameter address, a hardware protocol
	// constant rather than driver-specific sound data - immediately
	// followed by the 3 mutable payload bytes that this function fills
	// in before sending. Hardcoded (not read via loadData()) since
	// there's no reason to expect this address to live at the same
	// offset in every driver's own resource file.
	byte buffer[7] = { 0x10, 0x00, 0x01, (byte)(mode & 3), (byte)(time & 7), (byte)(level & 7), 0xFF };
	sendSysExData(buffer, ARRAYSIZE(buffer));
}

/*-----------------------------------------------------------------------*/

void RSound::Channel_flushHeldNotes(Channel *channel) {
	byte *slots = _heldNotes[channel->_midiChannel];
	for (int i = 0; i < 4; ++i) {
		if (slots[i] != 0xFF) {
			sendNoteOn(channel->_midiChannel, slots[i], 0);
			slots[i] = 0xFF;
		}
	}
}

void RSound::Channel_checkFade(Channel *channel, int midiChannel) {
	if (!channel->_activeCount)
		return;
	if (!channel->_pendingStop)
		return;

	if (channel->_volume == 0) {
		// This is 3 zero bytes - a generic silence/no-op stream, not
		// driver-specific sound data. Hardcoded (not read via loadData())
		// for the same reason as sendReverbSysEx()'s fixed address - no
		// reason to expect the same offset holds the same bytes in every
		// driver's own resource file.
		static byte silenceStream[3] = { 0, 0, 0 };
		channel->_pSrc = silenceStream;
		channel->_pendingStop = 0;
		return;
	}

	channel->_volume -= 1;
	sendVolume(midiChannel, channel->_volume);
}

void RSound::checkFadingChannels() {
	if (!_fadeCheckPeriod)
		return;
	if (--_fadeCheckCounter > 0)
		return;

	_fadeCheckCounter = _fadeCheckPeriod;
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		Channel_checkFade(&_channels[i], i + 1);
}

/*-----------------------------------------------------------------------*/

void RSound::resetChannelRange(int first, int last) {
	// Matches the confirmed struct-field disassembly exactly: two
	// word-sized writes, each zeroing a pair of adjacent byte fields -
	// [bx+_activeCount] (covers _activeCount + _pitchBendFadeStep) and
	// [bx+_volumeFadeStep] (covers _volumeFadeStep + _panFadeStep).
	for (int i = first; i <= last; ++i) {
		_channels[i]._activeCount = 0;
		_channels[i]._pitchBendFadeStep = 0;
		_channels[i]._volumeFadeStep = 0;
		_channels[i]._panFadeStep = 0;
	}
}

void RSound::resetAllChannels() {
	bool wasDisabled = _isDisabled;
	_isDisabled = true;
	resetChannelRange(0, RSOUND_CHANNEL_COUNT - 1);
	for (int i = 0; i <= RSOUND_CHANNEL_COUNT; ++i)
		for (int j = 0; j < 4; ++j)
			_heldNotes[i][j] = 0xFF;
	_isDisabled = wasDisabled;
}

void RSound::resetChannels1to5() {
	_isDisabled = true;
	resetChannelRange(0, 4);
	for (int i = 0; i <= RSOUND_CHANNEL_COUNT; ++i)
		for (int j = 0; j < 4; ++j)
			_heldNotes[i][j] = 0xFF;
	_isDisabled = false;
}

void RSound::resetChannels4to9() {
	// CORRECTED (was wrongly named/ranged resetChannels6to9): channels
	// 4-9, 0-based indices 3-8 - confirmed directly from disassembly.
	_isDisabled = true;
	resetChannelRange(3, 8);
	_isDisabled = false;
}

/*-----------------------------------------------------------------------*/

int RSound::command0() {
	bool wasDisabled = _isDisabled;
	_isDisabled = true;

	resetAllChannels();
	sendGmReset(RSOUND_CHANNEL_COUNT);
	sendSysEx(_sysExOffset);

	_isDisabled = wasDisabled;
	return 0;
}

int RSound::command1() {
	// IMPORTANT: falls through to the SAME tail as command5()
	// directly and ungated - it must NOT call the virtual command5()
	// here, since that would wrongly apply whatever driver-specific
	// isSoundActive() gate command5() has. command1() itself is never
	// gated in any driver confirmed so far.
	command3();
	enableUpperChannels();
	return 0;
}

int RSound::command2() {
	resetChannels1to5();
	sendGmReset(4);
	return 0;
}

int RSound::command3() {
	// Matches rsound_command3: enables channels 1,2,3,4 AND 9 (not 5-8) -
	// a genuine, confirmed asymmetry (channel 9 falls through into the
	// tail-shared Channel_enable call with the others).
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[0].enable(0xFF);
	_channels[1].enable(0xFF);
	_channels[2].enable(0xFF);
	_channels[3].enable(0xFF);
	_channels[8].enable(0xFF);
	return 0;
}

void RSound::resetAndGmResetUpperChannels() {
	// command4()'s shared tail in every driver
	// confirmed so far: reset channels 4-9, then a full sendGmReset(9)
	// (all 9 channels) - both really execute, matching the original
	// exactly despite the apparent redundancy.
	resetChannels4to9();
	sendGmReset(RSOUND_CHANNEL_COUNT);
}

void RSound::enableUpperChannels() {
	// command1()'s and command5()'s shared tail in
	// every driver confirmed so far: enables channels 5,6,7,8.
	_fadeCheckPeriod = 1;
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
}

int RSound::command6() {
	_isDisabled = true;
	for (int i = 1; i <= RSOUND_CHANNEL_COUNT; ++i)
		muteChannel(i);
	return 0;
}

int RSound::command7() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		sendVolumeCC(i + 1, _channels[i]._volume);
	_isDisabled = false;
	return 0;
}

int RSound::command8() {
	int result = 0;
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		result |= _channels[i]._activeCount;
	return result;
}

/*-----------------------------------------------------------------------*/

int RSound::readScriptByte(byte *&pSrc) {
	++pSrc;
	return (int8)*pSrc;
}

uint16 RSound::readScriptWord(byte *&pSrc) {
	byte lo = *++pSrc;
	byte hi = *++pSrc;
	return lo | (hi << 8);
}

void RSound::update() {
	getRandomNumber();
	if (_isDisabled)
		return;

	++_frameCounter;
	++_tickCounter; // incremented alongside _frameCounter
	checkRandomAmbianceTrigger();
	pollAllChannels();
	checkFadingChannels();
}

void RSound::pollAllChannels() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		pollActiveChannel(&_channels[i]);
}

/*-----------------------------------------------------------------------*/
// Per-channel opcode interpreter. Bytes with the high bit clear are
// two-byte [note][duration] events (or, if > 0xBD, an opcode 0xBE-0xFF).
// Matches the sibling ASound driver's pollActiveChannel() structure: a
// goto-based re-entrant dispatch, re-looping after every opcode that
// doesn't consume a duration tick.

void RSound::pollActiveChannel(Channel *ch) {
	int midiChannel = ch->_midiChannel;

	if (!ch->_activeCount)
		return;

	if (ch->_keyOnDelay) {
		if (--ch->_keyOnDelay == 0)
			Channel_flushHeldNotes(ch);
	}

	if (--ch->_activeCount != 0)
		goto post_keyon;

dispatch:
	{
		// NOTE: ch->_pSrc is left untouched by each case below until its
		// own final "ch->_pSrc = ..." / "ch->_pSrc += N" assignment, so
		// any earlier use of ch->_pSrc within the same case (e.g. to
		// compute a resume/branch target) still refers to this opcode's
		// start position - only the local pSrc advances as operand bytes
		// are read via readScriptByte()/readScriptWord().
		byte *pSrc = ch->_pSrc;
		byte b = *pSrc;

		if (!(b & 0x80)) {
			// ---- Simple note event: [note][duration] (distinct from -
			// and simpler than - the explicit chord opcode
			// 0xED below, which is count-prefixed and can hold up to 4
			// simultaneous notes) ----
			int note = (int8)pSrc[0] + ch->_transpose;
			int duration = pSrc[1];
			ch->_activeCount = duration;
			ch->_pSrc += 2;

			if (note != ch->_note)
				Channel_flushHeldNotes(ch);

			if (note != 0 && duration != 0) {
				if (ch->_keyOnDelayOverride)
					ch->_keyOnDelay = ch->_keyOnDelayOverride;
				else
					ch->_keyOnDelay = ch->_activeCount - ch->_noteOffset;

				bool skip = false;
				if (ch->_noteOffset < 0 && _heldNotes[midiChannel][0] == (byte)note)
					skip = true;

				if (!skip) {
					ch->_note = note;
					_heldNotes[midiChannel][0] = (byte)note;
					sendNoteOn(midiChannel, note, ch->_velocity);
				}
			}
			goto post_keyon;
		}

		if (b <= 0xBD)
			goto post_keyon; // matches the disassembly's fallthrough for 0x80-0xBD

		switch (b) {
		case 0xBE: {
			// TODO: Native purpose unresolved. The full overlay corpus only
			// stores this value; no reader was found.
			_clockUnknown = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xBF: {
			// TODO: Native purpose unresolved. Every overlay performs these
			// stores, but no later reader was found. The coarse value is copied
			// only before the first update tick, and both enable words are set.
			_clockCoarseTarget = readScriptWord(pSrc);
			if (_tickCounter == 0)
				_clockCoarse = _clockCoarseTarget;
			_clockEnabled1 = 1;
			_clockEnabled2 = 1;
			ch->_pSrc += 3;
			goto dispatch;
		}
		case 0xC0: {
			// TODO: Native purpose unresolved. This has the same first-tick
			// copy behavior as 0xBF, with no later reader. It does not set the
			// two enable words.
			_clockMedTarget = readScriptByte(pSrc);
			if (_tickCounter == 0)
				_clockMed = _clockMedTarget;
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xC1: {
			// TODO: Native purpose unresolved. Unlike 0xBF/0xC0, this stores
			// the value directly and unconditionally, with no tick gate, but no
			// later reader was found.
			_clockFine = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xC2: {
			int v1 = readScriptByte(pSrc);
			int v2 = readScriptByte(pSrc);
			int v3 = readScriptByte(pSrc);
			sendReverbSysEx(v1, v2, v3);
			ch->_pSrc += 4;
			goto dispatch;
		}
		case 0xC3: {
			int v = readScriptByte(pSrc);
			noOpHandler(v);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xC4: {
			const uint16 targetOffset = readScriptWord(pSrc);
			if (!callFunction(targetOffset))
				error("RSound::pollActiveChannel: unsupported opcode 0xC4 target 0x%04x",
						targetOffset);
			ch->_pSrc += 3;
			goto dispatch;
		}

		// ---- Conditional branches, WITH resume-point bookkeeping ----
		// (field_22/ _branchTarget), matching the call/return pair below.
		case 0xC5: case 0xC6: case 0xC7: case 0xC8:
		case 0xC9: case 0xCA: case 0xCB: case 0xCC:
		// ---- Same 8 conditions, WITHOUT bookkeeping (plain goto-if) ----
		case 0xCD: case 0xCE: case 0xCF: case 0xD0:
		case 0xD1: case 0xD2: case 0xD3: case 0xD4: {
			bool withSave = (b <= 0xCC);
			bool varMode = (b <= 0xC8) || (b >= 0xCD && b <= 0xD0); // 0xC5-C8, 0xCD-D0: var vs var; 0xC9-CC, 0xD1-D4: var vs immediate
			int idx1 = readScriptByte(pSrc);
			int idx2 = readScriptByte(pSrc);
			int lhs = _scriptVariables[idx1 & 0xFF];
			int rhs = varMode ? _scriptVariables[idx2 & 0xFF] : idx2;

			int cond = (b - (withSave ? 0xC5 : 0xCD)) % 4; // 0:'>' 1:'<' 2:'!=' 3:'=='
			bool take = false;
			switch (cond) {
			case 0: take = (lhs > rhs); break;
			case 1: take = (lhs < rhs); break;
			case 2: take = (lhs != rhs); break;
			case 3: take = (lhs == rhs); break;
			}

			if (take) {
				if (withSave)
					ch->_branchTarget = ch->_pSrc + 5;
				ch->_pSrc = loadData(readScriptWord(pSrc));
			} else {
				ch->_pSrc += 5;
			}
			goto dispatch;
		}

		// ---- Bitwise/arithmetic ops on script variables ----
		case 0xD5: { // XOR, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] ^= _scriptVariables[idx2 & 0xFF];
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xD6: { // XOR, immediate
			int idx1 = readScriptByte(pSrc), imm = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] ^= (byte)imm;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xD7: { // OR, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] |= _scriptVariables[idx2 & 0xFF];
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xD8: { // OR, immediate
			int idx1 = readScriptByte(pSrc), imm = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] |= (byte)imm;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xD9: { // AND, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] &= _scriptVariables[idx2 & 0xFF];
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xDA: { // AND, immediate
			int idx1 = readScriptByte(pSrc), imm = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] &= (byte)imm;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xDB: { // MOD, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			byte divisor = _scriptVariables[idx2 & 0xFF];
			if (divisor)
				_scriptVariables[idx1 & 0xFF] = _scriptVariables[idx1 & 0xFF] % divisor;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xDC: {
			// SUSPECTED BUG in the original (MOD, "immediate" form): the
			// disassembly reads a second operand byte but never uses it,
			// instead computing scriptVar[idx1] % scriptVar[idx1] (always
			// 0 when nonzero) - unlike every other immediate-mode
			// arithmetic opcode (0xD6/D8/DA/E0/E2/E4), which all use the
			// operand directly. Preserved exactly rather than "fixed" to
			// use the operand as a real divisor, since that would be
			// guessing at intended behavior.
			int idx1 = readScriptByte(pSrc);
			readScriptByte(pSrc); // operand read but unused, matching the disassembly
			byte self = _scriptVariables[idx1 & 0xFF];
			if (self)
				_scriptVariables[idx1 & 0xFF] = self % self;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xDD: { // DIV, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			byte divisor = _scriptVariables[idx2 & 0xFF];
			if (divisor)
				_scriptVariables[idx1 & 0xFF] = _scriptVariables[idx1 & 0xFF] / divisor;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xDE: {
			// Same suspected bug shape as 0xDC (DIV "immediate" form:
			// self-divide, operand read but unused - always yields 1
			// when nonzero) - preserved exactly.
			int idx1 = readScriptByte(pSrc);
			readScriptByte(pSrc); // operand read but unused, matching the disassembly
			byte self = _scriptVariables[idx1 & 0xFF];
			if (self)
				_scriptVariables[idx1 & 0xFF] = self / self;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xDF: { // MUL, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] = (byte)(_scriptVariables[idx1 & 0xFF] * _scriptVariables[idx2 & 0xFF]);
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE0: { // MUL, immediate
			int idx1 = readScriptByte(pSrc), imm = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] = (byte)(_scriptVariables[idx1 & 0xFF] * imm);
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE1: { // SUB, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] -= _scriptVariables[idx2 & 0xFF];
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE2: { // SUB, immediate
			int idx1 = readScriptByte(pSrc), imm = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] -= (byte)imm;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE3: { // ADD, variable
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] += _scriptVariables[idx2 & 0xFF];
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE4: { // ADD, immediate
			int idx1 = readScriptByte(pSrc), imm = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] += (byte)imm;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE5: { // DEC
			int idx1 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF]--;
			ch->_pSrc += 2; goto dispatch;
		}
		case 0xE6: { // INC
			int idx1 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF]++;
			ch->_pSrc += 2; goto dispatch;
		}
		case 0xE7: {
			// Pokes scriptVar[idx1] into the sound-data stream itself at
			// offset (currentPos + 1 + idx2) - a self-modifying-script op.
			int idx1 = readScriptByte(pSrc);
			int idx2 = readScriptByte(pSrc);
			byte v = _scriptVariables[idx1 & 0xFF];
			(pSrc + 1 + idx2)[0] = v;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE8: { // copy: scriptVar[idx1] = scriptVar[idx2]
			int idx1 = readScriptByte(pSrc), idx2 = readScriptByte(pSrc);
			_scriptVariables[idx1 & 0xFF] = _scriptVariables[idx2 & 0xFF];
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xE9: { // set: scriptVar[idx1] = immediate (raw, unsigned)
			int idx1 = readScriptByte(pSrc);
			byte imm = *++pSrc;
			_scriptVariables[idx1 & 0xFF] = imm;
			ch->_pSrc += 3; goto dispatch;
		}
		case 0xEA: {
			// Selects a byte using a script variable as the table index,
			// then writes it at the self-relative displacement stored after
			// the table. The operation is confirmed, but its higher-level
			// purpose remains unknown.
			int idx1 = readScriptByte(pSrc);
			int len2 = readScriptByte(pSrc);
			byte *table1Base = pSrc + 1;
			byte v1 = table1Base[_scriptVariables[idx1 & 0xFF]];
			byte *table2 = pSrc + 1 + len2 + 1;
			int8 sizeByte = (int8)*(pSrc + 1 + len2);
			table2[sizeByte] = v1;
			ch->_pSrc += len2 + 4;
			goto dispatch;
		}
		case 0xEB: {
			// Picks a uniform random value in [rangeLow, rangeHigh], reads
			// a "tableByte" positioned right after the two range operands,
			// and writes the random value at offset (pSrc+2+tableByte)
			// relative to pSrc's position after reading both operands
			// (an extra increment in the disassembly, beyond the two
			// operand reads, is what puts tableByte one byte further
			// out than the 0xEC case below).
			int rangeLow = readScriptByte(pSrc);
			int rangeHigh = readScriptByte(pSrc);
			int range = rangeHigh - rangeLow + 1;
			int r = range ? (getRandomNumber() % range) : 0;
			byte value = (byte)(rangeLow + r);
			int tableByte = (int8)*(pSrc + 1);
			(pSrc + 2 + tableByte)[0] = value;
			ch->_pSrc += 4;
			goto dispatch;
		}
		case 0xEC: {
			// Same general shape as 0xEA: random-pick from a table1 of
			// length idx1, then poke it into table2 at an offset given
			// by table2's own leading "size" byte.
			int len1 = readScriptByte(pSrc);
			int r = len1 ? (getRandomNumber() % len1) : 0;
			byte *table1 = pSrc + 1;
			byte v1 = table1[r];
			byte *table2 = pSrc + 1 + len1 + 1;
			int8 sizeByte = (int8)*(pSrc + 1 + len1);
			table2[sizeByte] = v1;
			ch->_pSrc += len1 + 3;
			goto dispatch;
		}
		case 0xFE: {
			// End an outer loop. A zero count establishes the next outer
			// and inner loop anchors without repeating. The signed byte is
			// stored in the native 16-bit counter, so 0xFF means 65535.
			if (!ch->_outerLoopCount) {
				ch->_outerLoopCount = (uint16)(int16)(int8)
						readScriptByte(pSrc);
				if (!ch->_outerLoopCount) {
					ch->_pSrc += 2;
					ch->_outerLoopPtr = ch->_pSrc;
					ch->_innerLoopCount = 0;
					ch->_outerLoopCount = 0;
				} else {
					ch->_pSrc = ch->_outerLoopPtr;
				}
			} else if (--ch->_outerLoopCount == 0) {
				ch->_pSrc += 2;
				ch->_outerLoopPtr = ch->_pSrc;
			} else {
				ch->_pSrc = ch->_outerLoopPtr;
			}
			ch->_innerLoopPtr = ch->_pSrc;
			goto post_keyon;
		}
		case 0xFF: {
			// End an inner loop. The restart pointer is the beginning of
			// the current inner-loop region and advances after completion.
			if (!ch->_innerLoopCount) {
				ch->_innerLoopCount = (uint16)(int16)(int8)
						readScriptByte(pSrc);
				if (!ch->_innerLoopCount) {
					ch->_pSrc += 2;
					ch->_innerLoopPtr = ch->_pSrc;
				} else {
					ch->_pSrc = ch->_innerLoopPtr;
				}
			} else if (--ch->_innerLoopCount == 0) {
				ch->_pSrc += 2;
				ch->_innerLoopPtr = ch->_pSrc;
			} else {
				ch->_pSrc = ch->_innerLoopPtr;
			}
			goto post_keyon;
		}

		// ---- Loop / restart-pointer opcodes ----
		case 0xFD: {
			if (ch->_soundData == nullptr) {
				ch->_pSrc = ch->_loopStartPtr;
			} else {
				ch->_loopStartPtr = ch->_soundData;
				ch->_pSrc = ch->_soundData;
				ch->_innerLoopPtr = ch->_soundData;
				ch->_outerLoopPtr = ch->_soundData;
			}
			goto post_keyon; // tail used by this cluster
		}
		case 0xFC: {
			byte *ptr = loadData(readScriptWord(pSrc));
			ch->_loopStartPtr = ptr;
			ch->_pSrc = ptr;
			ch->_innerLoopPtr = ptr;
			ch->_outerLoopPtr = ptr;
			ch->_soundData = ptr;
			goto post_keyon;
		}
		case 0xFB: { // unconditional jump, no bookkeeping
			ch->_pSrc = loadData(readScriptWord(pSrc));
			goto post_keyon;
		}
		case 0xFA: { // "call": jump, saving a resume point
			byte *target = loadData(readScriptWord(pSrc));
			ch->_branchTarget = ch->_pSrc + 3;
			ch->_pSrc = target;
			goto post_keyon;
		}
		case 0xF9: { // "return"
			if (ch->_branchTarget) {
				ch->_pSrc = ch->_branchTarget;
				ch->_branchTarget = nullptr;
			} else {
				ch->_pSrc++;
			}
			goto post_keyon;
		}
		case 0xF8: {
			ch->_program = readScriptByte(pSrc);
			ch->_pSrc += 2;
			sendProgramChange(midiChannel, ch->_program);
			goto post_keyon;
		}
		case 0xF7: {
			ch->_noteOffset = readScriptByte(pSrc);
			ch->_keyOnDelayOverride = 0;
			ch->_pSrc += 2;
			goto post_keyon;
		}
		case 0xF6: {
			ch->_keyOnDelayOverride = readScriptByte(pSrc);
			ch->_noteOffset = 0;
			ch->_pSrc += 2;
			goto post_keyon;
		}
		case 0xF5: {
			ch->_pitchBendFadeReload = readScriptByte(pSrc);
			ch->_pitchBendFadeStep = readScriptByte(pSrc);
			ch->_pitchBendFadeCount = readScriptByte(pSrc);
			ch->_pitchBendFadeCounter = 1;
			ch->_pSrc += 4;
			goto post_keyon;
		}
		case 0xF4: {
			ch->_volume = readScriptByte(pSrc);
			ch->_pSrc += 2;
			sendVolume(midiChannel, ch->_volume);
			goto post_keyon;
		}
		case 0xF3: {
			ch->_velocity = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto post_keyon;
		}
		case 0xF2: {
			ch->_volumeFadeReload = readScriptByte(pSrc);
			ch->_volumeFadeStep = readScriptByte(pSrc);
			ch->_volumeFadeCounter = 1;
			ch->_pSrc += 3;
			goto post_keyon;
		}
		case 0xF1: {
			ch->_pitchBend = readScriptByte(pSrc);
			ch->_pSrc += 2;
			sendPitchBend(midiChannel, ch->_pitchBend);
			goto post_keyon;
		}
		case 0xF0: {
			ch->_pan = readScriptByte(pSrc);
			sendPan(midiChannel, ch->_pan);
			ch->_pSrc += 2;
			goto post_keyon;
		}
		case 0xEF: {
			ch->_panFadeReload = readScriptByte(pSrc);
			ch->_panFadeStep = readScriptByte(pSrc);
			ch->_panFadeCounter = 1;
			ch->_pSrc += 3;
			goto post_keyon;
		}
		case 0xEE: {
			ch->_transpose = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto post_keyon;
		}
		case 0xED: {
			// ---- Chord event: [count][note1..noteN][duration] ----
			// Distinct from (and richer than) the
			// simple single-note "high bit clear" format at the top of
			// dispatch: this one is count-prefixed and can hold up to
			// 4 simultaneous notes (matching _heldNotes' 4 slots).
			int count = readScriptByte(pSrc);
			int firstNote = (int8)*(pSrc + 1) + ch->_transpose;
			if (firstNote != _heldNotes[midiChannel][0])
				Channel_flushHeldNotes(ch);

			int i;
			for (i = 0; i < count; ++i) {
				int note = (int8)*(pSrc + 1 + i) + ch->_transpose;
				bool skip = false;
				if (ch->_noteOffset < 0 && _heldNotes[midiChannel][i] == (byte)note)
					skip = true;
				if (!skip) {
					_heldNotes[midiChannel][i] = (byte)note;
					sendNoteOn(midiChannel, note, ch->_velocity);
				}
			}
			for (; i < 4; ++i)
				_heldNotes[midiChannel][i] = 0xFF;

			ch->_activeCount = *(pSrc + 1 + count);
			if (ch->_keyOnDelayOverride)
				ch->_keyOnDelay = ch->_keyOnDelayOverride;
			else
				ch->_keyOnDelay = ch->_activeCount - ch->_noteOffset;

			ch->_pSrc += count + 3;
			goto post_keyon;
		}
		default:
			// Unknown/unhandled opcode - matches the disassembly falling
			// through to re-dispatch on an unrecognised value.
			goto dispatch;
		}
	}

post_keyon:
	// ---- Volume fade tail ----
	if (ch->_volumeFadeStep) {
		if (--ch->_volumeFadeCounter == 0) {
			ch->_volumeFadeCounter = ch->_volumeFadeReload;
			ch->_volume += ch->_volumeFadeStep;
			if ((byte)ch->_volume > 0x7F) {
				ch->_volumeFadeStep = 0;
				ch->_volume = ((byte)ch->_volume > 0xAF) ? 0 : 0x7F;
			}
			sendVolume(midiChannel, ch->_volume);
		}
	}

	// ---- Pitch bend fade tail ----
	if (ch->_pitchBendFadeStep) {
		if (--ch->_pitchBendFadeCounter == 0) {
			ch->_pitchBendFadeCounter = ch->_pitchBendFadeReload;
			ch->_pitchBend += ch->_pitchBendFadeStep;
			sendPitchBend(midiChannel, ch->_pitchBend);
		}
		if (--ch->_pitchBendFadeCount == 0)
			ch->_pitchBendFadeStep = 0;
	}

	// ---- Pan fade tail ----
	if (ch->_panFadeStep) {
		if (--ch->_panFadeCounter == 0) {
			ch->_panFadeCounter = ch->_panFadeReload;
			ch->_pan += ch->_panFadeStep;
			sendPan(midiChannel, ch->_pan);
		}
	}
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
