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
#include "mads/dragonsphere/sound/rsound.h"

namespace MADS {
namespace Dragonsphere {
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
		_soundData = RSound::_silenceStream;
	}
}

void Channel::load(byte *pData) {
	reset(pData);
	_activeCount = 1;
	_owner->resetPitchBend(_midiChannel);
}

/*-----------------------------------------------------------------------*/

byte RSound::_silenceStream[2] = { 0, 0 };

RSound::RSound(Audio::Mixer *mixer, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset,
		bool usesDemoOpcodeSemantics) :
		SoundDriver(mixer, filename, dataOffset, dataSize) {
	_commandParam = 0;
	_frameCounter = 0;
	_isDisabled = false;
	_randomSeed = 1234;
	_stateChangedFlag = 0;
	_pollResult = 0;
	_sysExOffset = sysExOffset;
	_fadeCheckCounter = 0;
	_fadeCheckPeriod = 0;
	_opcodeBeBfValue = 0;
	_opcodeC0Value = 0;
	_opcodeC1Value = 0;
	_usesImmediateArithmeticOperands = false;
	_usesDemoOpcodeSemantics = usesDemoOpcodeSemantics;

	_callbackCounter = 0;
	_callbackPeriod = 0;
	_callbackFnPtr = nullptr;
	_musicIndex = 0;

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
		"a34a4c5ee91d1ad0b007e3f6d791725e",
		"af04277f9b6d7206a683383ee8fa4a13",
		"9b5ff2c3ec2265771e440cbc0f60b3ff",
		"2ae7ec33e79d326653925ff8ddc7550c",
		"504d445acd33487aa1653c3f476b1199",
		"e342cfa5c8e968d132e525f1ed9c975c",
		nullptr,
		nullptr,
		"94a28abdbb536f87f5d470c72734d8e3"
	};

	for (int i = 1; i <= 9; ++i) {
		if (i == 7 || i == 8)
			continue;

		Common::Path filename(Common::String::format("rsound.dr%d", i));
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
		// Export 4 is a return stub in every audited Dragonsphere RSOUND overlay.
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

Channel *RSound::playSoundChannels6to8(int offset) {
	// Channels 6-8 (0-based 5-7), symmetric free/fallback scan. Matches
	// the disassembly's own playSoundChannels6to8 exactly.
	return playSoundData(loadData(offset), 5, 7, 7);
}

Channel *RSound::playSoundChannels7to8(int offset) {
	// Channels 7,8 (0-based 6-7), symmetric free/fallback scan. Matches
	// the disassembly's own playSoundChannels7to8 exactly.
	return playSoundData(loadData(offset), 6, 7, 7);
}

Channel *RSound::playSoundChannels1To5(int offset) {
	// Matches the disassembly's own playSoundAny exactly: channels 1-5
	// (0-based 0-4), fully symmetric free/fallback scan. Renamed from
	// playSoundAny for clarity - NOT Phantom's same-named function's 1-8
	// range.
	return playSoundData(loadData(offset), 0, 4, 4);
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

int RSound::isMusicChannelsActive() {
	// Channels 1-5 AND 9 (this driver's "lower"/music
	// group) - NOT Phantom/ASound's fixed channel range.
	return _channels[0]._activeCount || _channels[1]._activeCount ||
		_channels[2]._activeCount || _channels[3]._activeCount ||
		_channels[4]._activeCount || _channels[8]._activeCount;
}

/*-----------------------------------------------------------------------*/

void RSound::sendNoteOn(int midiChannel, int note, int velocity) {
	// The disassembly derives midiChannel/velocity from
	// the currently-active channel context rather than taking them as
	// explicit call-site parameters, but the transmitted bytes are
	// identical either way - kept parameterized here for API consistency
	// with the rest of the class (and with the Phantom RSound family).
	_midiDriver->send(MidiDriver::MIDI_COMMAND_NOTE_ON | midiChannel,
			note, velocity);
}

void RSound::sendProgramChange(int midiChannel, int program) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PROGRAM_CHANGE | midiChannel,
			program, 0);
}

void RSound::sendVolume(Channel *ch) {
	// CORRECTED naming and NEW gate - see rsound.h class comment. The
	// disassembly's real (unnamed) volume-sender only
	// transmits when the channel is not pending-stop; Channel_checkFade's
	// own fade-out mechanism takes over otherwise.
	if (ch->_pendingStop)
		return;
	const int volume = CLIP(ch->_volume, 0, 127) * _masterVolume / 255;
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE |
			ch->_midiChannel, MidiDriver::MIDI_CONTROLLER_VOLUME, volume);
}

void RSound::sendVolumeCC(int midiChannel, int volume) {
	// Structured messages do not retain MIDI running status, so the native
	// unconditional status write is equivalent to an ordinary volume event.
	volume = CLIP(volume, 0, 127) * _masterVolume / 255;
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
			MidiDriver::MIDI_CONTROLLER_VOLUME, volume);
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
	// CORRECTED naming - see rsound.h class comment: this is the function
	// the disassembly auto-named "sendVolume", which actually sends CC#10.
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
			MidiDriver::MIDI_CONTROLLER_PANNING, value);
}

void RSound::muteChannel(int midiChannel) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
			MidiDriver::MIDI_CONTROLLER_VOLUME, 0);
}

void RSound::sendGmReset(int count) {
	sendGmResetRange(count, 1);
}

void RSound::sendGmResetRange(int high, int low) {
	// Matches sendGmReset: counts DOWN from high to low, using the
	// counter itself as the MIDI channel number each iteration.
	for (int midiChannel = high; midiChannel >= low; --midiChannel) {
		_fadeCheckPeriod = 0;

		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
				MidiDriver::MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
				MidiDriver::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS, 0);
		sendVolumeCC(midiChannel, 100);
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
	// Native retail and demo helpers mutate this same fixed template.
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
		// null_sound_data: a fixed 2-byte (0,0) silence marker - the
		// same block Channel::enable() redirects _soundData to.
		channel->_pSrc = _silenceStream;
		channel->_pendingStop = 0;
		return;
	}

	channel->_volume -= 1;
	sendVolume(channel);
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
	for (int i = first; i <= last; ++i) {
		_channels[i]._activeCount = 0;
		_channels[i]._pitchBendFadeStep = 0;
		_channels[i]._volumeFadeStep = 0;
		_channels[i]._panFadeStep = 0;
	}
}

void RSound::disableChannelTo(int channelIndex, byte flag) {
	Channel &ch = _channels[channelIndex];
	if (!ch._activeCount)
		return;
	if (ch._activeCount == 1)
		ch._pSrc = _silenceStream;
	ch._pendingStop = flag;
	ch._soundData = _silenceStream;
}

void RSound::resetHeldNotes() {
	// Zeroes (0xFF-fills) the logically-used region of _heldNotes; the
	// real table has extra unused padding rows beyond channel 9 that
	// this doesn't need to replicate - see the field comment.
	for (int i = 0; i <= RSOUND_CHANNEL_COUNT; ++i)
		for (int j = 0; j < 4; ++j)
			_heldNotes[i][j] = 0xFF;
}

void RSound::resetAllChannels() {
	bool wasDisabled = _isDisabled;
	_isDisabled = true;
	resetChannelRange(0, RSOUND_CHANNEL_COUNT - 1);
	resetHeldNotes();
	_isDisabled = wasDisabled;
}

void RSound::resetChannels1to5() {
	// CORRECTED: also resets channel 9 (confirmed directly from
	// resetChannels1to5's disassembly - channels 1,2,3,4,5,9, matching
	// command3's own 6-channel "lower" group exactly), not just 1-5.
	_isDisabled = true;
	resetChannelRange(0, 4);
	_channels[8]._activeCount = 0;
	_channels[8]._pitchBendFadeStep = 0;
	_channels[8]._volumeFadeStep = 0;
	_channels[8]._panFadeStep = 0;
	resetHeldNotes();
	_isDisabled = false;
}

void RSound::resetChannels6to8() {
	// Matches resetChannels6to8: channels 6,7,8 (0-based indices 5-7).
	_isDisabled = true;
	resetChannelRange(5, 7);
	_isDisabled = false;
}

/*-----------------------------------------------------------------------*/

int RSound::command0() {
	// Matches rsound_command0: clears the deferred-callback state, then
	// falls into the shared reset() (resetAllChannels + sendGmReset(9) +
	// sendSysEx(_sysExOffset)).
	_callbackCounter = 0;
	_callbackPeriod = 0;
	_callbackFnPtr = nullptr;

	resetAllChannels();
	sendGmReset(RSOUND_CHANNEL_COUNT);
	sendSysEx(_sysExOffset);
	return 0;
}

int RSound::command1() {
	// Matches rsound_command1: calls command3() then falls straight
	// through into command5() - both unconditional, no gating.
	command3();
	command5();
	return 0;
}

int RSound::command2() {
	resetChannels1to5();
	sendGmReset(4);
	return 0;
}

int RSound::command3() {
	// Matches rsound_command3: enables channels 1,2,3,4,5 AND 9 (SIX
	// channels) - this driver's "lower" group, wider than Phantom's
	// default 1-4,9.
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[0].enable(0xFF);
	_channels[1].enable(0xFF);
	_channels[2].enable(0xFF);
	_channels[3].enable(0xFF);
	_channels[4].enable(0xFF);
	_channels[8].enable(0xFF);
	return 0;
}

int RSound::command4() {
	// Matches rsound_command4: CONCRETE, no isSoundActive() gate at all
	// (unlike Phantom's pure-virtual, always-gated command4/5) - see
	// class comment.
	resetChannels6to8();
	sendGmReset(RSOUND_CHANNEL_COUNT);
	return 0;
}

int RSound::command5() {
	// Matches rsound_command5: enables channels 6,7,8 - this driver's
	// "upper" group, narrower than Phantom's default 5-8.
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[5].enable(0xFF);
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
	return 0;
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

void RSound::tickCallback() {
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter != 0)
		return;

	_callbackCounter = _callbackPeriod;
	if (!_callbackFnPtr)
		return;

	CallbackFunction fn = _callbackFnPtr;
	_callbackFnPtr = nullptr;
	(this->*fn)();
}

void RSound::update() {
	getRandomNumber();
	if (_isDisabled)
		return;

	++_frameCounter;
	pollAllChannels();
	tickCallback();
	checkFadingChannels();
}

void RSound::pollAllChannels() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		pollActiveChannel(&_channels[i]);
}

/*-----------------------------------------------------------------------*/
// Per-channel opcode interpreter. Every opcode was rechecked against the
// retail and demo overlays. The explicit demo switches below preserve the
// differences found in DR1/DR9 instead of relying on Phantom by analogy.

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
		byte *pSrc = ch->_pSrc;
		byte b = *pSrc;

		// The native signed comparison dispatches every value through 0xBD
		// to the two-byte note path. Values 0x80-0xBD are therefore signed
		// note values, not unknown control opcodes.
		if (b <= 0xBD) {
			// ---- Simple note event: [note][duration] ----
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

		switch (b) {
		case 0xBE: {
			_opcodeBeBfValue = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xBF: {
			_opcodeBeBfValue = readScriptByte(pSrc);
			readScriptByte(pSrc); // Native helper skips this byte and returns.
			ch->_pSrc += 3;
			goto dispatch;
		}
		case 0xC0: {
			_opcodeC0Value = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xC1: {
			_opcodeC1Value = readScriptByte(pSrc);
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
			readScriptByte(pSrc); // reads one operand, does nothing with it
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xC4: {
			const uint16 targetOffset = readScriptWord(pSrc);
			if (!callFunction(targetOffset, *ch))
				error("Unknown Dragonsphere RSOUND callback 0x%04x", targetOffset);
			ch->_pSrc += 3;
			goto dispatch;
		}

		// ---- Conditional branches, WITH resume-point bookkeeping ----
		case 0xC5: case 0xC6: case 0xC7: case 0xC8:
		case 0xC9: case 0xCA: case 0xCB: case 0xCC:
		// ---- Same 8 conditions, WITHOUT bookkeeping (plain goto-if) ----
		case 0xCD: case 0xCE: case 0xCF: case 0xD0:
		case 0xD1: case 0xD2: case 0xD3: case 0xD4: {
			bool withSave = (b <= 0xCC);
			bool varMode = (b <= 0xC8) || (b >= 0xCD && b <= 0xD0);
			int idx1 = readScriptByte(pSrc);
			int idx2 = readScriptByte(pSrc);
			int lhs = _scriptVariables[idx1 & 0xFF];
			int rhs = varMode ? _scriptVariables[idx2 & 0xFF] : idx2;

			int cond = (b - (withSave ? 0xC5 : 0xCD)) % 4;
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
			int idx1 = readScriptByte(pSrc);
			int operand = readScriptByte(pSrc);
			byte value = _scriptVariables[idx1 & 0xFF];
			uint16 divisor = _usesImmediateArithmeticOperands ?
					(uint16)(int16)operand : value;
			if (divisor)
				_scriptVariables[idx1 & 0xFF] = value % divisor;
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
			int idx1 = readScriptByte(pSrc);
			int operand = readScriptByte(pSrc);
			byte value = _scriptVariables[idx1 & 0xFF];
			uint16 divisor = _usesImmediateArithmeticOperands ?
					(uint16)(int16)operand : value;
			if (divisor)
				_scriptVariables[idx1 & 0xFF] = value / divisor;
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
			int rangeLow = readScriptByte(pSrc);
			int rangeHigh = readScriptByte(pSrc);
			int range = rangeHigh - rangeLow + 1;
			uint16 random = getRandomNumber();
			if (!_usesDemoOpcodeSemantics)
				random &= 0x7fff;
			int r = range ? (random % range) : 0;
			byte value = (byte)(rangeLow + r);
			int tableByte = (int8)*(pSrc + 1);
			(pSrc + 2 + tableByte)[0] = value;
			ch->_pSrc += 4;
			goto dispatch;
		}
		case 0xEC: {
			int len1 = readScriptByte(pSrc);
			uint16 random = getRandomNumber();
			if (!_usesDemoOpcodeSemantics)
				random &= 0x7fff;
			int r = len1 ? (random % len1) : 0;
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
			goto dispatch;
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
			goto dispatch;
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
			goto dispatch;
		}
		case 0xFC: {
			byte *ptr = loadData(readScriptWord(pSrc));
			ch->_loopStartPtr = ptr;
			ch->_pSrc = ptr;
			ch->_innerLoopPtr = ptr;
			ch->_outerLoopPtr = ptr;
			ch->_soundData = ptr;
			goto dispatch;
		}
		case 0xFB: { // unconditional jump, no bookkeeping
			ch->_pSrc = loadData(readScriptWord(pSrc));
			goto dispatch;
		}
		case 0xFA: { // "call": jump, saving a resume point
			byte *target = loadData(readScriptWord(pSrc));
			ch->_branchTarget = ch->_pSrc + 3;
			ch->_pSrc = target;
			goto dispatch;
		}
		case 0xF9: { // "return"
			if (ch->_branchTarget) {
				ch->_pSrc = ch->_branchTarget;
				ch->_branchTarget = nullptr;
			} else {
				ch->_pSrc++;
			}
			goto dispatch;
		}
		case 0xF8: {
			ch->_program = readScriptByte(pSrc);
			ch->_pSrc += 2;
			sendProgramChange(midiChannel, ch->_program);
			goto dispatch;
		}
		case 0xF7: {
			ch->_noteOffset = readScriptByte(pSrc);
			ch->_keyOnDelayOverride = 0;
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xF6: {
			ch->_keyOnDelayOverride = readScriptByte(pSrc);
			ch->_noteOffset = 0;
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xF5: {
			ch->_pitchBendFadeReload = readScriptByte(pSrc);
			ch->_pitchBendFadeStep = readScriptByte(pSrc);
			ch->_pitchBendFadeCount = readScriptByte(pSrc);
			ch->_pitchBendFadeCounter = 1;
			ch->_pSrc += 4;
			goto dispatch;
		}
		case 0xF4: {
			ch->_velocity = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xF3: {
			ch->_volumeFadeReload = readScriptByte(pSrc);
			ch->_volumeFadeStep = readScriptByte(pSrc);
			ch->_volumeFadeCounter = 1;
			ch->_pSrc += 3;
			goto dispatch;
		}
		case 0xF2: {
			ch->_pitchBend = readScriptByte(pSrc);
			ch->_pSrc += 2;
			sendPitchBend(midiChannel, ch->_pitchBend);
			goto dispatch;
		}
		case 0xF1: {
			if (!ch->_pendingStop)
				ch->_volume = readScriptByte(pSrc);
			ch->_pSrc += 2;
			sendVolume(ch);
			goto dispatch;
		}
		case 0xF0: {
			ch->_pan = readScriptByte(pSrc);
			sendPan(midiChannel, ch->_pan);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xEF: {
			ch->_panFadeReload = readScriptByte(pSrc);
			ch->_panFadeStep = readScriptByte(pSrc);
			ch->_panFadeCounter = 1;
			ch->_pSrc += 3;
			goto dispatch;
		}
		case 0xEE: {
			ch->_transpose = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xED: {
			// ---- Chord event: [count][note1..noteN][duration] ----
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
			sendVolume(ch);
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
} // namespace Dragonsphere
} // namespace MADS
