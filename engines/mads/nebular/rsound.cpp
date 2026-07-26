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
#include "common/memstream.h"
#include "mads/nebular/rsound.h"

namespace MADS {
namespace RexNebular {

void Channel::reset(byte *startPtr) {
	_pitchBendFadeStep = 0;
	_panFadeStep = 0;
	_innerLoopCount = 0;
	_outerLoopCount = 0;
	_noteOffset = 0;
	_pendingStop = 0;
	_volumeFadeReload = 0xFF;
	_pitchBend = 64;
	_pan = 64;
	_loopStartPtr = startPtr;
	_pSrc = startPtr;
	_innerLoopPtr = startPtr;
	_outerLoopPtr = startPtr;
	_soundData = startPtr;
}

void Channel::enable(int flag) {
	if (_activeCount) {
		_pendingStop = flag;

		// WORKAROUND: matches the same apparent original-code quirk
		// documented in AdlibChannel::enable() - the original set
		// _soundData to the flag value here, which we replace with
		// a simple null pointer.
		_soundData = nullptr;
	}
}

void Channel::load(byte *pData) {
	reset(pData);
	_activeCount = 1;
	_owner->sendPitchBend(_midiChannel, 0x40);
}

/*-----------------------------------------------------------------------*/

RSound::RSound(Audio::Mixer *mixer, const Common::Path &filename,
		int dataOffset, int dataSize) : SoundDriver(mixer, filename, dataOffset, dataSize) {
	_commandParam = 0;
	_frameCounter = 0;
	_isDisabled = false;
	_masterVolume = 255;
	_randomSeed = 1234;
	_lastMidiStatus = 0;
	_noteTriggeredThisPoll = false;
	_pollResult = 0;
	_resultFlag = 0;

	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i) {
		_channels[i]._owner = this;
		_channels[i]._midiChannel = i + 1;
	}

	// rsound_init calls sub_10132 (resetAllChannels) directly, then later
	// (via initDeviceOnce, on successful hardware detection) calls
	// rsound_command0, which resets the channels again and sends the
	// GM-reset messages to the device. Since we don't do real hardware
	// detection here, just go straight to command0() - matches ASound's
	// constructor calling command0() directly.
	command0();

	//_opl->start(new Common::Functor0Mem<void, RSound>(this, &RSound::onTimer), CALLBACKS_PER_SECOND);
}

void RSound::validate() {
	Common::File f;
	static const char *const MD5[] = {
		"205398468de2c8873b7d4d73d5be8ddc",
		"f9b2d944a2fb782b1af5c0ad592306d3",
		"7431f8dad77d6ddfc24e6f3c0c4ac7df",
		"eb1f3f5a4673d3e73d8ac1818c957cf4",
		"f936dd853073fa44f3daac512e91c476",
		"3dc139d3e02437a6d9b732072407c366",
		"af0edab2934947982e9a405476702e03",
		"8cbc25570b50ba41c9b5361cad4fbedc",
		"a31e4783e098f633cbb6689adb41dd4f"
	};

	for (int i = 1; i <= 9; ++i) {
		Common::Path filename(Common::String::format("RSOUND.00%d", i));
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

void RSound::resultCheck() {
	if (_resultFlag != 1) {
		_resultFlag = 1;
		_pollResult = 1;
	}
}

Channel *RSound::playSound(int offset) {
	return playSoundData(loadData(offset));
}

Channel *RSound::playSoundData(byte *pData, int startingChannel) {
	// Scan for a free channel. Deliberately excludes channel 9 (index 8),
	// matching the disassembly - playSound() never reaches it.
	for (int i = startingChannel; i < RSOUND_CHANNEL_COUNT - 1; ++i) {
		if (!_channels[i]._activeCount) {
			_channels[i].load(pData);
			return &_channels[i];
		}
	}

	// None found; fall back to an interruptable (pending-stop) channel
	for (int i = RSOUND_CHANNEL_COUNT - 2; i >= startingChannel; --i) {
		if (_channels[i]._pendingStop == 0xFF) {
			_channels[i].load(pData);
			return &_channels[i];
		}
	}

	return nullptr;
}

bool RSound::isSoundActive(byte *pData) {
	// Deliberately excludes channel 9 (index 8), matching the disassembly -
	// same as playSoundData()'s scan never reaching channel 9 either.
	for (int i = 0; i < RSOUND_CHANNEL_COUNT - 1; ++i) {
		if (_channels[i]._activeCount && _channels[i]._soundData == pData)
			return true;
	}

	return false;
}

int RSound::getRandomNumber() {
	int v = 0x9249 + (int)_randomSeed;
	_randomSeed = ((v >> 3) | (v << 13)) & 0xFFFF;
	return _randomSeed;
}

void RSound::onTimer() {
	Common::StackLock slock(_driverMutex);
	poll();
}

void RSound::setVolume(int volume) {
	_masterVolume = volume;
	if (!volume)
		command0();
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
	sendStatus(midiChannel, 0x90);
	sendMidiByte(note);
	sendMidiByte(velocity);
}

void RSound::sendProgramChange(int midiChannel, int program) {
	sendStatus(midiChannel, 0xC0);
	sendMidiByte(program);
}

void RSound::sendVolume(int midiChannel, int volume) {
	sendStatus(midiChannel, 0xB0);
	sendMidiByte(7); // CC#7: Channel Volume
	sendMidiByte(volume);
}

void RSound::sendPitchBend(int midiChannel, int value) {
	sendStatus(midiChannel, 0xE0);
	sendMidiByte(0);     // LSB always 0 - only coarse (MSB) control is used
	sendMidiByte(value);
}

void RSound::sendPan(int midiChannel, int value) {
	sendStatus(midiChannel, 0xB0);
	sendMidiByte(10); // CC#10: Pan
	sendMidiByte(value);
}

void RSound::muteChannel(int midiChannel) {
	sendStatus(midiChannel, 0xB0);
	sendMidiByte(7);
	sendMidiByte(0);
}

void RSound::restoreChannelVolume(int midiChannel, int volume) {
	sendStatus(midiChannel, 0xB0);
	sendMidiByte(7);
	sendMidiByte(volume);
}

void RSound::sendSysEx(int offset) {
	warning("RSound: SysEx block at offset %04X not yet implemented (device init handshake)", offset);
}

/*-----------------------------------------------------------------------*/

void RSound::Channel_flushHeldNotes(Channel *channel) {
	byte *slots = _heldNotes[channel->_midiChannel];

	for (int i = 0; i < 4; ++i) {
		if (slots[i] == 0xFF)
			break;

		sendStatus(channel->_midiChannel, 0x90);
		sendMidiByte(slots[i]);
		sendMidiByte(0); // velocity 0 = note off
		slots[i] = 0xFF;
	}
}

void RSound::Channel_checkFade(Channel *channel) {
	if (!channel->_activeCount)
		return;
	if (!channel->_pendingStop)
		return;

	if (channel->_volume != 0) {
		--channel->_volume;
		sendVolume(channel->_midiChannel, channel->_volume);
	} else {
		// Fully silent - recycle the channel to the fixed "silence" stream
		// (unk_14566 in the disassembly, at offset 0x3246 relative to
		// seg001's load address - matches sub_1029F exactly)
		channel->_pSrc = loadData(0x3246);
		channel->_pendingStop = 0;
	}
}

void RSound::checkFadingChannels() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		Channel_checkFade(&_channels[i]);
}

void RSound::Channel_pollActive(Channel *channel) {
	if (!channel->_activeCount)
		return;

	int midiChannel = channel->_midiChannel;

	if (channel->_keyOnDelay > 0 && --channel->_keyOnDelay == 0)
		Channel_flushHeldNotes(channel);

	if (--channel->_activeCount <= 0) {
		for (;;) {
			byte *pSrc = channel->_pSrc;

			if (*pSrc < 0x80) {
				// Plain (note, duration) pair
				if (_noteTriggeredThisPoll) {
					// Already fired a note-on this poll tick - defer to next tick
					_noteTriggeredThisPoll = false;
					break;
				}

				byte note = pSrc[0];
				byte duration = pSrc[1];
				channel->_note = note;
				channel->_activeCount = duration;
				channel->_pSrc = pSrc + 2;

				if (!note || !duration) {
					Channel_flushHeldNotes(channel);
				} else {
					channel->_keyOnDelay = channel->_activeCount - channel->_noteOffset;

					bool skipRetrigger = false;
					if ((int8)channel->_noteOffset < 0 && _heldNotes[midiChannel][0] == note)
						skipRetrigger = true;

					if (!skipRetrigger) {
						Channel_flushHeldNotes(channel);
						sendNoteOn(midiChannel, note, channel->_velocity);
					}
					_heldNotes[midiChannel][0] = note;
				}

				break;
			}

			// Opcode dispatch (bytes 0xF1-0xFF)
			int opcode = *pSrc - 0xF1;
			switch (opcode) {
			case 0: // 0xF1: no-op (consume and ignore)
				channel->_pSrc = pSrc + 2;
				break;

			case 1: { // 0xF2: self-modifying randomize (byte swap in the data stream)
				int v1 = *++pSrc;
				++pSrc;
				int v2 = (v1 - 1) & getRandomNumber();
				int v3 = pSrc[v2];
				int v4 = pSrc[v1];
				pSrc[v4 + v1 + 1] = v3;
				channel->_pSrc += v1 + 3;
				break;
			}

			case 2: // 0xF3: setup pan fade
				channel->_panFadeReload = pSrc[1];
				channel->_panFadeStep = pSrc[2];
				channel->_panFadeCounter = 1;
				channel->_pSrc = pSrc + 3;
				break;

			case 3: // 0xF4: set pan directly and send
				channel->_pan = pSrc[1];
				sendPan(midiChannel, channel->_pan);
				channel->_pSrc = pSrc + 2;
				break;

			case 4: { // 0xF5: chord note-on trigger (up to 4 simultaneous notes)
				byte noteCount = pSrc[1];
				byte *notes = pSrc + 2;
				byte *slots = _heldNotes[midiChannel];

				int i;
				for (i = 0; i < noteCount; ++i) {
					byte note = notes[i];
					if (slots[i] != note) {
						sendNoteOn(midiChannel, note, channel->_velocity);
						slots[i] = note;
					}
				}
				for (; i < 4; ++i)
					slots[i] = 0xFF;

				byte duration = notes[noteCount];
				channel->_activeCount = duration;
				if (channel->_noteOffset != 0xFF) {
					channel->_keyOnDelay = (duration < channel->_noteOffset) ?
						duration : duration - channel->_noteOffset;
				} else {
					channel->_keyOnDelay = duration + 1;
				}

				channel->_pSrc = pSrc + noteCount + 3;
				_noteTriggeredThisPoll = true;
				break;
			}

			case 5: // 0xF6: set volume directly and send (priority-gated while pending-stop)
				if (!channel->_pendingStop || channel->_volume >= pSrc[1]) {
					channel->_volume = pSrc[1];
					sendVolume(midiChannel, channel->_volume);
				}
				channel->_pSrc = pSrc + 2;
				break;

			case 6: // 0xF7: set pitch bend directly and send
				channel->_pitchBend = pSrc[1];
				sendPitchBend(midiChannel, channel->_pitchBend);
				channel->_pSrc = pSrc + 2;
				break;

			case 7: // 0xF8: setup volume fade (gated by pending-stop)
				if (!channel->_pendingStop) {
					channel->_volumeFadeReload = pSrc[1];
					channel->_volumeFadeCounter = pSrc[1];
					channel->_volumeFadeStep = pSrc[2];
				}
				channel->_pSrc = pSrc + 3;
				break;

			case 8: // 0xF9: set velocity
				channel->_velocity = pSrc[1];
				channel->_pSrc = pSrc + 2;
				break;

			case 9: // 0xFA: setup pitch-bend ramp
				channel->_pitchBendFadeReload = pSrc[1];
				channel->_pitchBendFadeStep = pSrc[2];
				channel->_pitchBendFadeCount = pSrc[3];
				channel->_pitchBendFadeCounter = 1;
				channel->_pSrc = pSrc + 4;
				break;

			case 10: // 0xFB: set note offset
				channel->_noteOffset = pSrc[1];
				channel->_pSrc = pSrc + 2;
				break;

			case 11: // 0xFC: program change
				channel->_program = pSrc[1];
				sendProgramChange(midiChannel, channel->_program);
				channel->_pSrc = pSrc + 2;
				break;

			case 12: { // 0xFD: full reset / loop restart, preserving pending-stop
				int pendingStop = channel->_pendingStop;
				channel->reset(channel->_loopStartPtr);
				channel->_pendingStop = pendingStop;
				break;
			}

			case 13: // 0xFE: outer loop
				if (!channel->_outerLoopCount) {
					if (*++pSrc == 0) {
						channel->_pSrc += 2;
						channel->_outerLoopPtr = channel->_pSrc;
						channel->_innerLoopPtr = channel->_pSrc;
						channel->_innerLoopCount = 0;
						channel->_outerLoopCount = 0;
					} else {
						channel->_outerLoopCount = *pSrc;
						channel->_pSrc = channel->_outerLoopPtr;
						channel->_innerLoopPtr = channel->_outerLoopPtr;
					}
				} else if (--channel->_outerLoopCount) {
					channel->_outerLoopPtr = channel->_pSrc;
					channel->_innerLoopPtr = channel->_pSrc;
				} else {
					channel->_pSrc += 2;
					channel->_outerLoopPtr = channel->_pSrc;
					channel->_innerLoopPtr = channel->_pSrc;
				}
				break;

			case 14: // 0xFF: inner loop
				if (!channel->_innerLoopCount) {
					if (*++pSrc == 0) {
						channel->_pSrc += 2;
						channel->_innerLoopPtr = channel->_pSrc;
						channel->_innerLoopCount = 0;
					} else {
						channel->_innerLoopCount = *pSrc;
						channel->_pSrc = channel->_innerLoopPtr;
					}
				} else if (--channel->_innerLoopCount) {
					channel->_pSrc = channel->_innerLoopPtr;
				} else {
					channel->_pSrc += 2;
					channel->_innerLoopPtr = channel->_pSrc;
				}
				break;

			default:
				break;
			}
		}
	}

	// Fade tail: pitch bend, volume, pan
	if (channel->_pitchBendFadeStep) {
		if (!--channel->_pitchBendFadeCounter) {
			channel->_pitchBendFadeCounter = channel->_pitchBendFadeReload;
			channel->_pitchBend += channel->_pitchBendFadeStep;
			sendPitchBend(midiChannel, channel->_pitchBend);
		}
		if (!--channel->_pitchBendFadeCount)
			channel->_pitchBendFadeStep = 0;
	}

	if (channel->_volumeFadeStep) {
		if (!--channel->_volumeFadeCounter) {
			channel->_volumeFadeCounter = channel->_volumeFadeReload;
			int8 newVolume = (int8)channel->_volume + (int8)channel->_volumeFadeStep;
			if (newVolume < 0) {
				channel->_volumeFadeStep = 0;
				newVolume = 0;
			} else if (newVolume >= 0x7F) {
				channel->_volumeFadeStep = 0;
				newVolume = 0x7F;
			}
			channel->_volume = newVolume;
			sendVolume(midiChannel, newVolume);
		}
	}

	if (channel->_panFadeStep) {
		if (!--channel->_panFadeCounter) {
			channel->_panFadeCounter = channel->_panFadeReload;
			byte newPan = channel->_pan + channel->_panFadeStep;
			if (newPan > 0x7F) {
				newPan = (newPan ^ 0x7F) & 0x7F;
				channel->_panFadeCounter = 0;
			}
			channel->_pan = newPan;
			sendPan(midiChannel, newPan);
		}
	}
}

void RSound::pollAllChannels() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		Channel_pollActive(&_channels[i]);
}

void RSound::update() {
	getRandomNumber();
	if (_isDisabled)
		return;

	tickCallback();

	++_frameCounter;
	pollAllChannels();
	checkFadingChannels();
}

/*-----------------------------------------------------------------------*/

/**
 * Zeroes _activeCount and the three fade-step fields for channels in
 * [first, last) - matches the shared shape of sub_10132/sub_101A0/sub_101EA.
 * Deliberately does NOT touch the loop pointers, volume, program, pan etc,
 * matching the original.
 */
void RSound::resetChannelRange(int first, int last) {
	bool wasDisabled = _isDisabled;
	_isDisabled = true;

	for (int i = first; i < last; ++i) {
		_channels[i]._activeCount = 0;
		_channels[i]._pitchBendFadeStep = 0;
		_channels[i]._volumeFadeStep = 0;
		_channels[i]._panFadeStep = 0;
	}

	_isDisabled = wasDisabled;
}

void RSound::resetHeldNotes() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT + 1; ++i)
		for (int j = 0; j < 4; ++j)
			_heldNotes[i][j] = 0xFF;
}

/**
 * Matches sub_10132: resets all 9 channels and the held-notes table.
 * Called both from the constructor (mirroring rsound_init) and from
 * command0.
 */
void RSound::resetAllChannels() {
	resetChannelRange(0, RSOUND_CHANNEL_COUNT);
	resetHeldNotes();
}

/**
 * Sends the GM-reset Control Change sequence (all notes off, reset all
 * controllers, volume=100, pan=center) to MIDI channels [first, last]
 * (both inclusive, 1-based). Shared tail used by command0/command2/command4.
 */
void RSound::sendGmReset(int first, int last) {
	for (int ch = first; ch <= last; ++ch) {
		sendStatus(ch, 0xB0);
		sendMidiByte(0x7B); // All Notes Off
		sendMidiByte(0);
		sendMidiByte(0x79); // Reset All Controllers
		sendMidiByte(0);
		sendMidiByte(7);    // Channel Volume
		sendMidiByte(100);
		sendMidiByte(10);   // Pan
		sendMidiByte(0x40);
	}
}

int RSound::command0() {
	bool isDisabled = _isDisabled;
	_isDisabled = true;

	resetAllChannels();
	sendGmReset(1, RSOUND_CHANNEL_COUNT);

	// Matches the trailing "lea ax, unk_1138F; jmp sub_1041E" in rsound_command0.
	// TODO: 0x6F is unk_1138F's address (0x1138F) relative to seg001's load
	// address (0x11320) - the exact data-segment offset convention still
	// needs verifying once dataOffset/dataSize below are confirmed.
	sendSysEx(0x6F);

	_isDisabled = isDisabled;
	return 0;
}

int RSound::command1() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int RSound::command2() {
	// Channels 1-5, matching sub_101A0 (also reinitializes the held-notes
	// table) plus the GM-reset CC sequence for those same channels.
	resetChannelRange(0, 5);
	resetHeldNotes();
	sendGmReset(1, 5);
	return 0;
}

int RSound::command3() {
	for (int i = 0; i < 5; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int RSound::command4() {
	// Channels 6-9, matching sub_101EA (does NOT touch the held-notes
	// table) plus the GM-reset CC sequence for those same channels.
	resetChannelRange(5, RSOUND_CHANNEL_COUNT);
	sendGmReset(6, RSOUND_CHANNEL_COUNT);
	return 0;
}

int RSound::command5() {
	for (int i = 5; i < RSOUND_CHANNEL_COUNT; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int RSound::command6() {
	for (int ch = 1; ch <= RSOUND_CHANNEL_COUNT; ++ch)
		muteChannel(ch);
	return 0;
}

int RSound::command7() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		restoreChannelVolume(_channels[i]._midiChannel, _channels[i]._volume);
	return 0;
}

int RSound::command8() {
	int result = 0;
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		result |= _channels[i]._activeCount;

	return result;
}

} // namespace RexNebular
} // namespace MADS
