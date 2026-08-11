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
		int dataOffset, int dataSize, int sysExOffset) : SoundDriver(mixer, filename, dataOffset, dataSize) {
	_commandParam = 0;
	_frameCounter = 0;
	_tickCounter = 0;
	_isDisabled = false;
	_randomSeed = 1234;
	_lastMidiStatus = 0;
	_sysexChecksum = 0;
	_stateChangedFlag = 0;
	_pollResult = 0;
	_sysExOffset = sysExOffset;
	_fadeCheckCounter = 0;
	_fadeCheckPeriod = 0;

	_clockMedTarget = 0;
	_clockCoarseTarget = 0;
	_clockUnknown = 0;
	_clockCoarse = 112;
	_clockMed = 28;
	_clockFine = 7;
	_clockEnabled1 = 0;
	_clockEnabled2 = 0;

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

	// Matches initDeviceOnce: command0() then sendSysExSequence(). The
	// disassembly's _deviceInitialized guard flag is omitted - this
	// constructor only ever runs once per driver instance, so there's
	// nothing to guard against.
	command0();
	sendSysExSequence();
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

void RSound::setVolume(int volume) {
	// TODO: no confirmed handler for this in the disassembly seen so far.
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
// Low-level MIDI transmission. sendMidiByte() is the single point that
// needs to change once the real MT-32/MIDI output interface is wired up;
// everything else funnels through it.

void RSound::sendMidiByte(byte value) {
	warning("RSound: MIDI byte %02X", value);
}

void RSound::sendStatus(int midiChannel, byte statusNibble) {
	byte status = statusNibble | midiChannel;
	if (_lastMidiStatus != status) {
		_lastMidiStatus = status;
		sendMidiByte(status);
	}
}

void RSound::sendNoteOn(int midiChannel, int note, int velocity) {
	// The disassembly derives midiChannel/velocity from
	// the currently-active channel context rather than taking them as
	// explicit call-site parameters, but the transmitted bytes are
	// identical either way - kept parameterized here for API consistency
	// with the rest of the class (and with the Phantom RSound family).
	sendStatus(midiChannel, 0x90);
	sendMidiByte(note);
	sendMidiByte(velocity);
}

void RSound::sendProgramChange(int midiChannel, int program) {
	sendStatus(midiChannel, 0xC0);
	sendMidiByte(program);
}

void RSound::sendVolume(Channel *ch) {
	// CORRECTED naming and NEW gate - see rsound.h class comment. The
	// disassembly's real (unnamed) volume-sender only
	// transmits when the channel is not pending-stop; Channel_checkFade's
	// own fade-out mechanism takes over otherwise.
	if (ch->_pendingStop)
		return;
	sendStatus(ch->_midiChannel, 0xB0);
	sendMidiByte(7);
	sendMidiByte(ch->_volume);
}

void RSound::sendVolumeCC(int midiChannel, int volume) {
	// Unlike sendVolume(), this sends the status byte UNCONDITIONALLY (no
	// _lastMidiStatus dedup check) - used by command7 when restoring all
	// 9 channels' volumes in a row.
	byte status = 0xB0 | midiChannel;
	_lastMidiStatus = status;
	sendMidiByte(status);
	sendMidiByte(7);
	sendMidiByte(volume);
}

void RSound::sendPitchBend(int midiChannel, int value) {
	sendStatus(midiChannel, 0xE0);
	sendMidiByte(0); // LSB always 0 - only coarse (MSB) control is used
	sendMidiByte(value);
}

void RSound::resetPitchBend(int midiChannel) {
	sendPitchBend(midiChannel, 0x40);
}

void RSound::sendPan(int midiChannel, int value) {
	// CORRECTED naming - see rsound.h class comment: this is the function
	// the disassembly auto-named "sendVolume", which actually sends CC#10.
	sendStatus(midiChannel, 0xB0);
	sendMidiByte(0x0A); // CC#10: Pan
	sendMidiByte(value);
}

void RSound::muteChannel(int midiChannel) {
	byte status = 0xB0 | midiChannel;
	_lastMidiStatus = status;
	sendMidiByte(status);
	sendMidiByte(7);
	sendMidiByte(0);
}

void RSound::sendGmReset(int count) {
	sendGmResetRange(count, 1);
}

void RSound::sendGmResetRange(int high, int low) {
	// Matches sendGmReset: counts DOWN from high to low, using the
	// counter itself as the MIDI channel number each iteration.
	for (int midiChannel = high; midiChannel >= low; --midiChannel) {
		_fadeCheckPeriod = 0;

		byte status = 0xB0 | midiChannel;
		_lastMidiStatus = status;
		sendMidiByte(status);
		sendMidiByte(0x7B); // All Notes Off
		sendMidiByte(0);
		sendMidiByte(0x79); // Reset All Controllers
		sendMidiByte(0);
		sendMidiByte(7);    // Channel Volume
		sendMidiByte(100);
		sendMidiByte(0x0A); // Pan
		sendMidiByte(0x40);
	}
}

const byte *RSound::sendSysExData(const byte *pData) {
	static const byte header[] = { 0xF0, 0x41, 0x10, 0x16, 0x12 };
	for (int i = 0; i < ARRAYSIZE(header); ++i)
		sendMidiByte(header[i]);

	_sysexChecksum = 0;
	int i = 0;
	for (; pData[i] != 0xFF; ++i) {
		sendMidiByte(pData[i]);
		_sysexChecksum += pData[i];
	}

	sendMidiByte((~_sysexChecksum + 1) & 0x7F);
	sendMidiByte(0xF7);

	return &pData[i];
}

const byte *RSound::sendSysEx(int offset) {
	return sendSysExData(loadData(offset));
}

void RSound::sendSysExSequence() {
	const byte *pData = loadData(_sysExOffset);
	for (;;) {
		pData = sendSysExData(pData);
		++pData;
		if (*pData == 0xFF)
			break;
	}
}

void RSound::sendPatchInitSequence() {
	// Matches sendPatchInitSequence exactly: 4 outer iterations, each
	// sending one SysEx message built from the fixed header at
	// loadData(0x61) plus a computed payload.
	byte base = 0;
	for (int outer = 0; outer < 4; ++outer) {
		byte *header = loadData(0x61);
		for (int i = 0; header[i] != 0xFF; ++i)
			sendMidiByte(header[i]);

		_sysexChecksum = 0;
		byte b1 = 5;
		_sysexChecksum += b1; sendMidiByte(b1);
		byte b2 = (byte)(outer << 1);
		_sysexChecksum += b2; sendMidiByte(b2);
		byte b3 = 0;
		_sysexChecksum += b3; sendMidiByte(b3);

		for (int inner = 0; inner < 0x20; ++inner) {
			byte v1 = (byte)(outer >> 1);
			_sysexChecksum += v1; sendMidiByte(v1);
			byte v2 = (byte)(inner + base);
			_sysexChecksum += v2; sendMidiByte(v2);
			static const byte tail[] = { 0x18, 0x32, 0x0C, 0, 1, 0 };
			for (int t = 0; t < ARRAYSIZE(tail); ++t) {
				_sysexChecksum += tail[t];
				sendMidiByte(tail[t]);
			}
		}

		sendMidiByte((~_sysexChecksum + 1) & 0x7F);
		sendMidiByte(0xF7);

		base = (byte)((base + 0x20) & 0x3F);
	}
}

void RSound::sendReverbSysEx(int mode, int time, int level) {
	byte buffer[7] = { 0x10, 0x00, 0x01, (byte)(mode & 3), (byte)(time & 7), (byte)(level & 7), 0xFF };
	sendSysExData(buffer);
}

/*-----------------------------------------------------------------------*/

void RSound::Channel_flushHeldNotes(Channel *channel) {
	byte *slots = _heldNotes[channel->_midiChannel];
	for (int i = 0; i < 4; ++i) {
		if (slots[i] != 0xFF) {
			sendStatus(channel->_midiChannel, 0x90);
			sendMidiByte(slots[i]);
			sendMidiByte(0); // velocity 0 = note off
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

void RSound::callFunction(uint16 offset) {
	error("Unsupported call to sound driver function at offset %.4x", offset);
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
	++_tickCounter;
	pollAllChannels();
	tickCallback();
	checkFadingChannels();
}

void RSound::pollAllChannels() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		pollActiveChannel(&_channels[i]);
}

/*-----------------------------------------------------------------------*/
// Per-channel opcode interpreter. Ported by structural analogy to the
// already-confirmed Phantom RSound::pollActiveChannel() - see the class
// comment in rsound.h for exactly which parts were independently
// re-checked against THIS game's disassembly (the dispatch range and the
// loop/restart/branch opcode cluster) versus carried over unchanged.

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

		if (!(b & 0x80)) {
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

		if (b <= 0xBD)
			goto post_keyon;

		switch (b) {
		case 0xBE: {
			_clockUnknown = readScriptByte(pSrc);
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xBF: {
			_clockCoarseTarget = readScriptWord(pSrc);
			if (_tickCounter == 0)
				_clockCoarse = _clockCoarseTarget;
			_clockEnabled1 = 1;
			_clockEnabled2 = 1;
			ch->_pSrc += 3;
			goto dispatch;
		}
		case 0xC0: {
			_clockMedTarget = readScriptByte(pSrc);
			if (_tickCounter == 0)
				_clockMed = _clockMedTarget;
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xC1: {
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
			readScriptByte(pSrc); // reads one operand, does nothing with it
			ch->_pSrc += 2;
			goto dispatch;
		}
		case 0xC4: {
			uint16 fnOffset = readScriptWord(pSrc);
			callFunction(fnOffset);
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
			readScriptByte(pSrc); // operand read but unused, matching Phantom's confirmed same-shaped bug
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
			int idx1 = readScriptByte(pSrc);
			readScriptByte(pSrc); // operand read but unused, matching Phantom's confirmed same-shaped bug
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
			int r = range ? (getRandomNumber() % range) : 0;
			byte value = (byte)(rangeLow + r);
			int tableByte = (int8)*(pSrc + 1);
			(pSrc + 2 + tableByte)[0] = value;
			ch->_pSrc += 4;
			goto dispatch;
		}
		case 0xEC: {
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
			goto post_keyon;
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
			sendVolume(ch);
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
