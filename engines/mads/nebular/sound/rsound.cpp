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
#include "common/util.h"
#include "mads/nebular/sound/rsound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

void Channel::loadData(byte *soundData) {
	_pitchSlideStepSize = 0;
	_panningSweepStepSize = 0;
	_innerLoopCounter = 0;
	_outerLoopCounter = 0;
	_noteDurationOffset = 0;
	_fadeOutActive = 0;
	_volumeFadeSpeed = 0xFF;
	_pitchBend = 64;
	_panning = 64;
	_soundDataStart = soundData;
	_pSrc = soundData;
	_innerLoopStart = soundData;
	_outerLoopStart = soundData;
	_soundData = soundData;
}

void Channel::setFadeOut(bool fadeOut) {
	if (_deltaCounter) {
		_fadeOutActive = fadeOut;

		// WORKAROUND: matches the same apparent original-code quirk
		// documented in AdlibChannel::setFadeOut() - the original set
		// _soundData to the flag value here, which we replace with
		// a simple null pointer.
		_soundData = nullptr;
	}
}

void Channel::playData(byte *soundData) {
	loadData(soundData);
	_deltaCounter = 1;
	_owner->sendPitchBend(_midiChannel, 0x40);
}

/*-----------------------------------------------------------------------*/

RSound::RSound(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset, RSoundFadeCheckMode fadeCheckMode) : 
		SoundDriver(mixer, filename, dataOffset, dataSize) {
	_commandParam = 0;
	_frameCounter = 0;
	_isDisabled = false;
	_masterVolume = 255;
	_randomSeed = 1234;
	_lastMidiStatus = 0;
	_noteTriggeredThisPoll = false;
	_pollResult = 0;
	_resultFlag = 0;
	_sysExOffset = sysExOffset;
	_midiDriver = midiDriver;
	_fadeCheckMode = fadeCheckMode;
	_fadeCheckAlternate = false;
	_fadeCheckCounter = 0;
	_fadeCheckPeriod = 0;

	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i) {
		_channels[i]._owner = this;
		_channels[i]._midiChannel = i + 1;
	}

	// rsound_init calls resetAllChannels directly, then later
	// (via initDeviceOnce, on successful hardware detection) calls
	// rsound_command0, which resets the channels again and sends the
	// MIDI channel reset messages to the device. Since we don't do real hardware
	// detection here, just go straight to command0() - matches ASound's
	// constructor calling command0() directly.
	// Matches initDeviceOnce: command0() then sendSysExSequence(). The
	// disassembly's _deviceInitialized guard flag is omitted - this
	// constructor only ever runs once per driver instance, so there's
	// nothing to guard against.
	command0();
	sendSysExSequence();
}

RSound::~RSound() {
}

void RSound::validate(bool isDemo) {
	Common::File f;
	static const char *const MD5[] = {
		"6b2f2f24b54ba0177938dde17baa6231",
		"598dffd6aaa9c2dec820f981b2caec14",
		"878332bbca47992c18e0eebd33669e9c",
		"8de899aa94c3a9352dd437fca3a5dbbf",
		"d6e1bddda9bd71d4a13825fe4092eefa",
		"aa16aa7b6f27a631b0cc64c4f7f26468",
		"3504e523c888541725f6aeb3d07631bc",
		"40a2a8bd0d49f1acbb0569f1b22ec9b2",
		"2ae093b2ce06f739f200ca3e9ff2af85"
	};
	static const char *const MD5_DEMO[] = {
		"ad14e2a1c900287737b9f43f1d8c3fb2",
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		"e2fafe292239be4afa2bf789bf4f496d"
	};
	const char *const *expectedMD5 = isDemo ? MD5_DEMO : MD5;

	for (int i = 1; i <= 9; ++i) {
		if (!expectedMD5[i - 1])
			continue;

		Common::Path filename(Common::String::format("RSOUND.00%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != expectedMD5[i - 1])
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
	Common::StackLock slock(_driverMutex);

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
	// playSound() (startingChannel=5) deliberately excludes channel 9
	// (index 8), matching rsound.009's disassembly. playSoundAny()
	// (startingChannel=0) reaches all 9 channels.
	int endChannel = (startingChannel == 0) ? RSOUND_CHANNEL_COUNT : RSOUND_CHANNEL_COUNT - 1;

	// Scan for a free channel
	for (int i = startingChannel; i < endChannel; ++i) {
		if (!_channels[i]._deltaCounter) {
			_channels[i].playData(pData);
			return &_channels[i];
		}
	}

	// None found; fall back to an interruptable (pending-stop) channel
	for (int i = endChannel - 1; i >= startingChannel; --i) {
		if (_channels[i]._fadeOutActive) {
			_channels[i].playData(pData);
			return &_channels[i];
		}
	}

	return nullptr;
}

bool RSound::isSoundActive(byte *pData) {
	// Deliberately excludes channel 9 (index 8), matching the disassembly -
	// same as playSoundData()'s scan never reaching channel 9 either.
	for (int i = 0; i < RSOUND_CHANNEL_COUNT - 1; ++i) {
		if (_channels[i]._deltaCounter && _channels[i]._soundData == pData)
			return true;
	}

	return false;
}

int RSound::getRandomNumber() {
	int v = 0x9249 + (int)_randomSeed;
	_randomSeed = ((v >> 3) | (v << 13)) & 0xFFFF;
	return _randomSeed;
}

void RSound::setVolume(int volume) {
	_masterVolume = CLIP(volume, 0, 255);
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		sendVolume(i + 1, _isDisabled ? 0 : _channels[i]._volume);
}

/*-----------------------------------------------------------------------*/

void RSound::sendNoteOn(int midiChannel, int note, int velocity) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_NOTE_ON | midiChannel, note, velocity);
}

void RSound::sendProgramChange(int midiChannel, int program) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PROGRAM_CHANGE | midiChannel, program, 0);
}

void RSound::sendVolume(int midiChannel, int volume) {
	const int scaledVolume = CLIP(volume, 0, 127) * _masterVolume / 255;
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
			MidiDriver::MIDI_CONTROLLER_VOLUME, scaledVolume);
}

void RSound::sendPitchBend(int midiChannel, int value) {
	// LSB always 0 - only coarse (MSB) control is used
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PITCH_BEND | midiChannel, 0, value);
}

void RSound::sendPan(int midiChannel, int value) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel, MidiDriver::MIDI_CONTROLLER_PANNING, value);
}

void RSound::muteChannel(int midiChannel) {
	sendVolume(midiChannel, 0);
}

void RSound::restoreChannelVolume(int midiChannel, int volume) {
	sendVolume(midiChannel, volume);
}

byte *RSound::sendSysExData(byte *pData, uint maxLength) {
	uint length = 0;
	while (length < maxLength && pData[length] != 0xFF)
		++length;

	if (length == maxLength) {
		warning("RSound::sendSysExData: unterminated SysEx message");
		return nullptr;
	}

	// FIXME This call adds the necessary delay for the MT-32 to process the
	// SysEx message, which will make the engine unresponsive.
	// This can be fixed using the SysEx queue (specify true as 3rd param).
	// Driver status can then be checked from the main event loop using
	// _midiDriver->isReady().
	_midiDriver->sysExMT32(pData, length);

	return &pData[length];
}

byte *RSound::sendSysEx(int offset) {
	if (offset < 0) {
		// Defensive guard for future mappings. Every validated retail and
		// demo constructor currently supplies a nonnegative table offset.
		warning("RSound::sendSysEx: command0_array offset not yet known for this driver");
		return nullptr;
	}
	if ((uint)offset >= _soundData.size()) {
		warning("RSound::sendSysEx: offset %d is outside the sound data", offset);
		return nullptr;
	}

	return sendSysExData(loadData(offset), _soundData.size() - offset);
}

void RSound::sendSysExSequence() {
	byte *pData = sendSysEx(_sysExOffset);
	if (!pData)
		return;

	byte *const dataEnd = _soundData.end();
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

/*-----------------------------------------------------------------------*/

void RSound::Channel_flushHeldNotes(Channel *channel) {
	byte *slots = _heldNotes[channel->_midiChannel];

	for (int i = 0; i < 4; ++i) {
		if (slots[i] == 0xFF)
			break;

		sendNoteOn(channel->_midiChannel, slots[i], 0); // velocity 0 = note off
		slots[i] = 0xFF;
	}
}

void RSound::Channel_checkFade(Channel *channel) {
	if (!channel->_deltaCounter)
		return;
	if (!channel->_fadeOutActive)
		return;

	if (channel->_volume != 0) {
		--channel->_volume;
		sendVolume(channel->_midiChannel, channel->_volume);
	} else {
		// Fully silent - recycle the channel to a fixed 2-byte (0,0)
		// silence stream. pollActiveChannel() processing a (note=0,
		// duration=0) pair sets _deltaCounter to 0, and its own
		// top-of-function guard (checked before any decrement) then
		// short-circuits every later call before _pSrc is ever read
		// again - so only these first 2 bytes are ever consumed.
		static byte silenceStream[2] = { 0, 0 };
		channel->_pSrc = silenceStream;
		channel->_fadeOutActive = false;
	}
}

void RSound::checkFadingChannels() {
	if (_fadeCheckMode == kRSoundFadeCheckAlternating) {
		_fadeCheckAlternate = !_fadeCheckAlternate;
		if (_fadeCheckAlternate)
			return;
	} else {
		if (!_fadeCheckPeriod)
			return;
		if (--_fadeCheckCounter > 0)
			return;

		_fadeCheckCounter = _fadeCheckPeriod;
	}

	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		Channel_checkFade(&_channels[i]);
}

void RSound::Channel_pollActive(Channel *channel) {
	if (!channel->_deltaCounter)
		return;

	int midiChannel = channel->_midiChannel;

	if (channel->_noteDurationCounter > 0 && --channel->_noteDurationCounter == 0)
		Channel_flushHeldNotes(channel);

	if (--channel->_deltaCounter <= 0) {
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
				channel->_deltaCounter = duration;
				channel->_pSrc = pSrc + 2;

				if (!note || !duration) {
					Channel_flushHeldNotes(channel);
				} else {
					channel->_noteDurationCounter = channel->_deltaCounter - channel->_noteDurationOffset;

					bool skipRetrigger = false;
					if ((int8)channel->_noteDurationOffset < 0 && _heldNotes[midiChannel][0] == note)
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
				channel->_panningSweepSpeed = pSrc[1];
				channel->_panningSweepStepSize = pSrc[2];
				channel->_panningSweepCounter = 1;
				channel->_pSrc = pSrc + 3;
				break;

			case 3: // 0xF4: set pan directly and send
				channel->_panning = pSrc[1];
				sendPan(midiChannel, channel->_panning);
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
				channel->_deltaCounter = duration;
				if (channel->_noteDurationOffset != 0xFF) {
					channel->_noteDurationCounter = (duration < channel->_noteDurationOffset) ?
						duration : duration - channel->_noteDurationOffset;
				} else {
					channel->_noteDurationCounter = duration + 1;
				}

				channel->_pSrc = pSrc + noteCount + 3;
				_noteTriggeredThisPoll = true;
				break;
			}

			case 5: // 0xF6: set volume directly and send (priority-gated while pending-stop)
				if (!channel->_fadeOutActive || channel->_volume >= pSrc[1]) {
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
				if (!channel->_fadeOutActive) {
					channel->_volumeFadeSpeed = pSrc[1];
					channel->_volumeFadeCounter = pSrc[1];
					channel->_volumeFadeStepSize = pSrc[2];
				}
				channel->_pSrc = pSrc + 3;
				break;

			case 8: // 0xF9: set velocity
				channel->_velocity = pSrc[1];
				channel->_pSrc = pSrc + 2;
				break;

			case 9: // 0xFA: setup pitch-bend ramp
				channel->_pitchSlideSpeed = pSrc[1];
				channel->_pitchSlideStepSize = pSrc[2];
				channel->_pitchSlideDurationCounter = pSrc[3];
				channel->_pitchSlideCounter = 1;
				channel->_pSrc = pSrc + 4;
				break;

			case 10: // 0xFB: set note offset
				channel->_noteDurationOffset = pSrc[1];
				channel->_pSrc = pSrc + 2;
				break;

			case 11: // 0xFC: program change
				channel->_program = pSrc[1];
				sendProgramChange(midiChannel, channel->_program);
				channel->_pSrc = pSrc + 2;
				break;

			case 12: { // 0xFD: full reset / loop restart, preserving pending-stop
				bool fadeOutActive = channel->_fadeOutActive;
				channel->loadData(channel->_soundDataStart);
				channel->_fadeOutActive = fadeOutActive;
				break;
			}

			case 13: // 0xFE: outer loop
				if (!channel->_outerLoopCounter) {
					if (*++pSrc == 0) {
						channel->_pSrc += 2;
						channel->_outerLoopStart = channel->_pSrc;
						channel->_innerLoopStart = channel->_pSrc;
						channel->_innerLoopCounter = 0;
						channel->_outerLoopCounter = 0;
					} else {
						channel->_outerLoopCounter =
							(uint16)(int16)(int8)*pSrc;
						channel->_pSrc = channel->_outerLoopStart;
						channel->_innerLoopStart = channel->_outerLoopStart;
					}
				} else if (--channel->_outerLoopCounter) {
					channel->_pSrc = channel->_outerLoopStart;
					channel->_innerLoopStart = channel->_outerLoopStart;
				} else {
					channel->_pSrc += 2;
					channel->_outerLoopStart = channel->_pSrc;
					channel->_innerLoopStart = channel->_pSrc;
				}
				break;

			case 14: // 0xFF: inner loop
				if (!channel->_innerLoopCounter) {
					if (*++pSrc == 0) {
						channel->_pSrc += 2;
						channel->_innerLoopStart = channel->_pSrc;
						channel->_innerLoopCounter = 0;
					} else {
						channel->_innerLoopCounter =
							(uint16)(int16)(int8)*pSrc;
						channel->_pSrc = channel->_innerLoopStart;
					}
				} else if (--channel->_innerLoopCounter) {
					channel->_pSrc = channel->_innerLoopStart;
				} else {
					channel->_pSrc += 2;
					channel->_innerLoopStart = channel->_pSrc;
				}
				break;

			default:
				break;
			}
		}
	}

	// Fade tail: pitch bend, volume, pan
	if (channel->_pitchSlideStepSize) {
		if (!--channel->_pitchSlideCounter) {
			channel->_pitchSlideCounter = channel->_pitchSlideSpeed;
			channel->_pitchBend += channel->_pitchSlideStepSize;
			sendPitchBend(midiChannel, channel->_pitchBend);
		}
		if (!--channel->_pitchSlideDurationCounter)
			channel->_pitchSlideStepSize = 0;
	}

	if (channel->_volumeFadeStepSize) {
		if (!--channel->_volumeFadeCounter) {
			channel->_volumeFadeCounter = channel->_volumeFadeSpeed;
			int8 newVolume = (int8)channel->_volume + (int8)channel->_volumeFadeStepSize;
			if (newVolume < 0) {
				channel->_volumeFadeStepSize = 0;
				newVolume = 0;
			} else if (newVolume >= 0x7F) {
				channel->_volumeFadeStepSize = 0;
				newVolume = 0x7F;
			}
			channel->_volume = newVolume;
			sendVolume(midiChannel, newVolume);
		}
	}

	if (channel->_panningSweepStepSize) {
		if (!--channel->_panningSweepCounter) {
			channel->_panningSweepCounter = channel->_panningSweepSpeed;
			byte newPan = channel->_panning + channel->_panningSweepStepSize;
			if (newPan > 0x7F) {
				newPan = (newPan ^ 0x7F) & 0x7F;
				channel->_panningSweepCounter = 0;
			}
			channel->_panning = newPan;
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
 * Zeroes _deltaCounter and the three fade-step fields for channels in
 * [first, last).
 * Deliberately does NOT touch the loop pointers, volume, program, pan etc,
 * matching the original.
 */
void RSound::resetChannelRange(int first, int last) {
	bool wasDisabled = _isDisabled;
	_isDisabled = true;

	for (int i = first; i < last; ++i) {
		_channels[i]._deltaCounter = 0;
		_channels[i]._pitchSlideStepSize = 0;
		_channels[i]._volumeFadeStepSize = 0;
		_channels[i]._panningSweepStepSize = 0;
	}

	_isDisabled = wasDisabled;
}

void RSound::resetHeldNotes() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT + 1; ++i)
		for (int j = 0; j < 4; ++j)
			_heldNotes[i][j] = 0xFF;
}

void RSound::resetHeldNotesRange(int firstChannel, int lastChannel) {
	assert(firstChannel >= 1 && lastChannel <= RSOUND_CHANNEL_COUNT &&
			firstChannel <= lastChannel);
	for (int channel = firstChannel; channel <= lastChannel; ++channel)
		for (int slot = 0; slot < 4; ++slot)
			_heldNotes[channel][slot] = 0xFF;
}

/**
 * Resets all 9 channels and the held-notes table.
 * Called both from the constructor (mirroring rsound_init) and from
 * command0.
 */
void RSound::resetAllChannels() {
	resetChannelRange(0, RSOUND_CHANNEL_COUNT);
	resetHeldNotes();
}

/**
 * Resets the MIDI channel state (all notes off, reset all
 * controllers, volume=100, pan=center) of MIDI channels [first, last]
 * (both inclusive, 1-based). Shared tail used by command0/command2/command4.
 */
void RSound::sendMidiChannelReset(int first, int last) {
	for (int ch = first; ch <= last; ++ch) {
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_VOLUME, 100);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_PANNING, 0x40);
	}
}

int RSound::command0() {
	bool isDisabled = _isDisabled;
	_isDisabled = true;

	resetAllChannels();
	setFadeCheckPeriod(0);
	sendMidiChannelReset(1, RSOUND_CHANNEL_COUNT);

	// Matches the tail of the original rsound_command0.
	// _sysExOffset is this driver's own command0_array offset, supplied
	// via the constructor (0x67 for rsound.001, 0x87 for rsound.002, 0x6F
	// for rsound.009) - each driver's own resource file carries its own
	// copy of this table, so no per-driver command0() override is needed.
	sendSysEx(_sysExOffset);

	_isDisabled = isDisabled;
	return 0;
}

int RSound::command1() {
	setFadeCheckPeriod(1);
	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i)
		_channels[i].setFadeOut(true);
	return 0;
}

int RSound::command2() {
	// Channels 1-5 (also reinitializes the held-notes
	// table) plus the MIDI channel reset for those same channels.
	resetChannelRange(0, 5);
	resetHeldNotes();
	setFadeCheckPeriod(0);
	sendMidiChannelReset(1, 5);
	return 0;
}

int RSound::command3() {
	setFadeCheckPeriod(1);
	for (int i = 0; i < 5; ++i)
		_channels[i].setFadeOut(true);
	return 0;
}

int RSound::command4() {
	// Channels 6-9 (does NOT touch the held-notes
	// table) plus the MIDI channel reset for those same channels.
	resetChannelRange(5, RSOUND_CHANNEL_COUNT);
	setFadeCheckPeriod(0);
	sendMidiChannelReset(6, RSOUND_CHANNEL_COUNT);
	return 0;
}

int RSound::command5() {
	setFadeCheckPeriod(1);
	for (int i = 5; i < RSOUND_CHANNEL_COUNT; ++i)
		_channels[i].setFadeOut(true);
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
		result |= _channels[i]._deltaCounter;

	return result;
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
