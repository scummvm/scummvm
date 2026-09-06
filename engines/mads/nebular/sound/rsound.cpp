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
	_pitchBend = 0x40;
	_panning = 0x40;
	_soundDataStart = soundData;
	_pSrc = soundData;
	_innerLoopStart = soundData;
	_outerLoopStart = soundData;
	_soundData = soundData;
}

void Channel::setFadeOut(bool fadeOut) {
	if (_deltaCounter > 0) {
		_fadeOutActive = fadeOut;

		// WORKAROUND: matches the same apparent original-code quirk
		// documented in AdlibChannel::setFadeOut() - the original set
		// _soundData to the flag value here, which we replace with
		// a simple null pointer.
		_soundData = nullptr;
	}
}

/*-----------------------------------------------------------------------*/

RSound::RSound(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset, RSoundFadeCheckMode fadeOutCheckMode) : 
		SoundDriver(mixer, filename, dataOffset, dataSize) {
	_commandParam = 0;
	_ticksSinceLastCommand = 0;
	_ticksProcessingDisabled = false;
	_mute = false;
	_randomSeed = 0x005C;
	_runningStatus = 0;
	_pollResult = 0;
	_resultFlag = 0;
	_sysExOffset = sysExOffset;
	_midiDriver = midiDriver;
	_fadeOutCheckMode = fadeOutCheckMode;
	_fadeOutSpeed = (_fadeOutCheckMode == kRSoundFadeCheckAlternating ? 2 : 0);
	_fadeOutCounter = _fadeOutSpeed;

	_dynamicStartChannel = 5;
	_dynamicIncludeChannel9 = false;
	// The full game RSOUND drivers command 2 and 3 include channel 9 when
	// initializing channel data and fading channels to stop, but command 2
	// does not include channel 9 when initializing MIDI channels. Instead,
	// command 4 includes channel 9 when initializing MIDI channels, but
	// command 4 and 5 do not include channel 9 when initializing channel
	// data and fading channels to stop. This is probably a bug, which is
	// not replicated in this reimplementation.
	_staticIncludeChannel9 = true;

	for (int i = 0; i < RSOUND_CHANNEL_COUNT; ++i) {
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

	if (_paused)
		return 0;

	processTick();

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

Channel *RSound::getChannel(byte channel) {
	return &_channels[channel - 1];
}

Channel *RSound::playSoundStatic(byte *soundData, byte channel) {
	assert(channel >= 1 && channel <= RSOUND_CHANNEL_COUNT);

	Channel *chan = getChannel(channel);
	Channel_playData(chan, soundData);

	return chan;
}

Channel *RSound::playSoundStatic(int offset, byte channel) {
	return playSoundStatic(loadData(offset), channel);
}

Channel *RSound::playSoundDynamic(int offset) {
	return allocateAndPlay(loadData(offset), _dynamicStartChannel);
}

Channel *RSound::playSoundAnyChannel(int offset) {
	return allocateAndPlay(loadData(offset), 1);
}

Channel *RSound::allocateAndPlay(byte *pData, int startingChannel) {
	int endChannel = RSOUND_CHANNEL_COUNT - 1;

	Channel *foundChannel = nullptr;

	// Scan for a free channel
	for (int i = startingChannel; i <= endChannel; ++i) {
		if (getChannel(i)->_deltaCounter == 0) {
			foundChannel = getChannel(i);
			break;
		}
	}

	if (foundChannel == nullptr) {
		// None found; fall back to a channel that is fading out to stop
		for (int i = endChannel; i >= startingChannel; --i) {
			if (getChannel(i)->_fadeOutActive) {
				foundChannel = getChannel(i);
				break;
			}
		}
	}

	if (foundChannel != nullptr)
		Channel_playData(foundChannel, pData);

	return foundChannel;
}

bool RSound::isSoundPlaying(int offset) {
	return isSoundPlaying(loadData(offset));
}

bool RSound::isSoundPlaying(byte *pData) {
	// Deliberately excludes channel 9, matching the disassembly -
	// same as allocateAndPlay()'s scan never reaching channel 9 either.
	for (int i = 1; i < RSOUND_CHANNEL_COUNT; ++i) {
		if (isSoundPlaying(i, pData))
			return true;
	}

	return false;
}

bool RSound::isSoundPlaying(byte channel, byte *pData) {
	return getChannel(channel)->_deltaCounter > 0 && getChannel(channel)->_soundData == pData;
}

uint16 RSound::generateRandomNumber() {
	uint16 newValue = 0x9249 + _randomSeed;
	_randomSeed = ((newValue >> 3) | (newValue << 13)) & 0xFFFF;
	return _randomSeed;
}

int RSound::getTicksSinceLastCommand() {
	return _ticksSinceLastCommand;
}

/*-----------------------------------------------------------------------*/

void RSound::sendNoteOn(byte midiChannel, byte note, byte velocity) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_NOTE_ON | midiChannel, note, velocity);
}

void RSound::sendProgramChange(byte midiChannel, byte program) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PROGRAM_CHANGE | midiChannel, program, 0);
}

void RSound::sendVolume(byte midiChannel, byte volume) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel,
					  MidiDriver::MIDI_CONTROLLER_VOLUME, volume);
}

void RSound::sendPitchBend(byte midiChannel, byte value) {
	// LSB always 0 - only coarse (MSB) control is used
	_midiDriver->send(MidiDriver::MIDI_COMMAND_PITCH_BEND | midiChannel, 0, value);
}

void RSound::sendPanning(byte midiChannel, byte value) {
	_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | midiChannel, MidiDriver::MIDI_CONTROLLER_PANNING, value);
}

void RSound::muteChannel(byte midiChannel) {
	sendVolume(midiChannel, 0);
}

void RSound::unmuteChannel(byte midiChannel, byte volume) {
	sendVolume(midiChannel, volume);
}

void RSound::setChannelVolume(byte channel, byte volume) {
	getChannel(channel)->_volume = volume;
	sendVolume(channel, volume);
}

byte *RSound::sendSysExData(byte *pData, uint maxLength) {
	uint length = 0;
	while (length < maxLength && pData[length] != 0xFF)
		++length;

	if (length == maxLength) {
		warning("RSound::sendSysExData: unterminated SysEx message");
		return nullptr;
	}

	_midiDriver->sysExMT32(pData, length, true);

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

void RSound::stopAllNotes() {
	_midiDriver->stopAllNotes();
}

/*-----------------------------------------------------------------------*/

void RSound::Channel_playData(Channel *channel, byte *soundData) {
	bool ticksProcessingDisabled = _ticksProcessingDisabled;
	_ticksProcessingDisabled = true;

	channel->loadData(soundData);
	channel->_deltaCounter = 1;
	sendPitchBend(channel->_midiChannel, 0x40);

	_ticksProcessingDisabled = ticksProcessingDisabled;
}

void RSound::Channel_turnOffActiveNotes(Channel *channel) {
	byte *channelActiveNotes = _activeNotes[channel->_midiChannel];

	for (int i = 0; i < RSOUND_ACTIVE_NOTES_COUNT; ++i) {
		if (channelActiveNotes[i] == 0xFF)
			break;

		sendNoteOn(channel->_midiChannel, channelActiveNotes[i], 0); // velocity 0 = note off
		channelActiveNotes[i] = 0xFF;
	}
}

void RSound::Channel_processFadeOut(Channel *channel) {
	if (channel->_deltaCounter == 0 || !channel->_fadeOutActive)
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

void RSound::processChannelFadeOuts() {
	if (_fadeOutSpeed == 0 || --_fadeOutCounter > 0)
		return;

	_fadeOutCounter = _fadeOutSpeed;

	for (int i = 1; i <= RSOUND_CHANNEL_COUNT; ++i)
		Channel_processFadeOut(getChannel(i));
}

void RSound::setFadeOutSpeed(int fadeOutSpeed) {
	if (_fadeOutCheckMode == kRSoundFadeCheckProgrammable)
		_fadeOutSpeed = fadeOutSpeed;
}

void RSound::Channel_processTick(Channel *channel) {
	if (channel->_deltaCounter == 0) {
		channel->_pitchSlideStepSize = 0;
		channel->_volumeFadeStepSize = 0;
		channel->_panningSweepStepSize = 0;
		// Original code continues with the effects processing code after the
		// event processing loop, but because all effects have been disabled
		// by setting step size to 0, this does nothing.
		return;
	}

	byte midiChannel = channel->_midiChannel;

	if (channel->_noteDurationCounter > 0 && --channel->_noteDurationCounter == 0)
		Channel_turnOffActiveNotes(channel);

	if (--channel->_deltaCounter <= 0) {
		bool chordEventProcessed = false;
		while (!chordEventProcessed) {
			byte *pSrc = channel->_pSrc;

			if (pSrc[0] < 0x80) {
				// Plain (note, delta) pair
				byte note = pSrc[0];
				byte delta = pSrc[1];
				channel->_note = note;
				channel->_deltaCounter = delta;
				channel->_pSrc += 2;

				// Opcode 0x00: all notes off
				// Delta 0: stop channel playback
				if (note == 0 || delta == 0) {
					Channel_turnOffActiveNotes(channel);
				} else {
					// In the original code this calculation can result in underflow
					// of the duration, which is a byte value. This actually happens
					// during playback of several tracks.
					// Assignment to the noteDurationCounter byte field
					// results in the same behavior as the original code,
					// f.e. duration -1 becomes counter value 0xFF.
					int duration = delta - channel->_noteDurationOffset;
					channel->_noteDurationCounter = duration;

					// If the note duration offset is 0 or positive, the notes played by
					// the previous note or chord event had a duration that was equal to
					// or shorter than the event delta and are already turned off, so the
					// new note should always be played. Otherwise, if this note is
					// already the first active note on the channel, the note should
					// remain active and will not be retriggered.
					// Note that a chord event could also have already played the new
					// note as the second to fourth active note; this is not checked.
					if (channel->_noteDurationOffset >= 0 || _activeNotes[midiChannel][0] != note) {
						Channel_turnOffActiveNotes(channel);
						sendNoteOn(midiChannel, note, channel->_velocity);
					}
					_activeNotes[midiChannel][0] = note;
				}

				// An event delta has been set, so break out of the event processing loop.
				break;
			}

			// Opcode dispatch (bytes 0xF1-0xFF)
			byte opcode = pSrc[0];
			switch (opcode) {
			case 0xF1: // No-op (consume and ignore)
				channel->_pSrc += 2;
				break;

			case 0xF2: { // Randomize
				// Event data byte 1: number of values.
				byte numValues = pSrc[1];
				// Event data byte 2+: values.
				// Generate a random index to pick one of the values.
				byte randomIndex = (numValues - 1) & generateRandomNumber();
				byte randomValue = pSrc[2 + randomIndex];
				// Last event data byte: data offset where the random value should be written.
				byte dataOffset = pSrc[2 + numValues];
				// Advance sound data pointer to after this event.
				channel->_pSrc += numValues + 3;
				// Write the random value at the data offset after this event.
				channel->_pSrc[dataOffset] = randomValue;
				break;
			}

			case 0xF3: // Start panning sweep
				channel->_panningSweepSpeed = pSrc[1];
				channel->_panningSweepStepSize = (int8) pSrc[2];
				channel->_panningSweepCounter = 1;
				channel->_pSrc += 3;
				break;

			case 0xF4: // Panning
				channel->_panning = pSrc[1];
				sendPanning(midiChannel, channel->_panning);
				channel->_pSrc += 2;
				break;

			case 0xF5: { // Chord
				// Event data byte 1: number of notes in the chord
				byte noteCount = pSrc[1];
				byte noteBytesCount = pSrc[1];
				if (noteCount > RSOUND_ACTIVE_NOTES_COUNT) {
					// The original code does not check the note count.
					// It will play more than 4 notes and write them
					// into the active note data of the next channel
					// or out of bounds. Some tracks contain chords
					// with more than 4 notes.
					warning("Encountered chord event with note count %i; limiting to %i notes", noteCount, RSOUND_ACTIVE_NOTES_COUNT);
					noteCount = RSOUND_ACTIVE_NOTES_COUNT;
				}
				// Event data byte 2+: note values
				byte *notes = pSrc + 2;
				byte *activeNotes = _activeNotes[midiChannel];

				int i;
				for (i = 0; i < noteCount; ++i) {
					byte note = notes[i];
					// Do not play the note again if it is already registered
					// at this index in the active notes data. The note could
					// also be registered at another index, but this is not
					// checked.
					if (activeNotes[i] != note) {
						sendNoteOn(midiChannel, note, channel->_velocity);
						activeNotes[i] = note;
					}
				}
				// Clear the remaining active note entries for this channel.
				for (; i < RSOUND_ACTIVE_NOTES_COUNT; ++i)
					activeNotes[i] = 0xFF;

				// Last event data byte: delta
				byte delta = pSrc[noteBytesCount + 2];
				channel->_deltaCounter = delta;
				// Subtract note duration offset from the delta to get the
				// chord duration.
				// Note that the duration is determined here in a different
				// way than for a note event. The offset is checked for
				// the value -1 instead of < 0, suggesting that -1 is the
				// minimum value. Also, a check is added to make sure the
				// duration does not underflow.
				byte duration;
				if (channel->_noteDurationOffset == -1) {
					duration = delta + 1;
				} else {
					duration = (delta < channel->_noteDurationOffset) ? delta : delta - channel->_noteDurationOffset;
				}
				channel->_noteDurationCounter = duration;

				channel->_pSrc += noteBytesCount + 3;
				// An event delta has been set, so break out of the event
				// processing loop.
				chordEventProcessed = true;
				break;
			}

			case 0xF6: // Volume
				// If the channel is fading out to stop and the new volume
				// value is higher than the current value, do not set
				// the new value.
				if (!channel->_fadeOutActive || channel->_volume >= pSrc[1]) {
					channel->_volume = pSrc[1];
					sendVolume(midiChannel, channel->_volume);
				}
				channel->_pSrc += 2;
				break;

			case 0xF7: // Pitch bend
				// Only the pitch bend MSB is used.
				channel->_pitchBend = pSrc[1];
				sendPitchBend(midiChannel, channel->_pitchBend);
				channel->_pSrc += 2;
				break;

			case 0xF8: // Start volume fade
				// If the channel is already fading out to stop, do not start
				// a volume fade.
				if (!channel->_fadeOutActive) {
					channel->_volumeFadeSpeed = pSrc[1];
					channel->_volumeFadeCounter = pSrc[1];
					channel->_volumeFadeStepSize = (int8) pSrc[2];
				}
				channel->_pSrc += 3;
				break;

			case 0xF9: // Note velocity
				// Velocity is not part of a note event; it is set using this
				// event as a channel setting.
				channel->_velocity = pSrc[1];
				channel->_pSrc += 2;
				break;

			case 0xFA: // Start pitch slide
				channel->_pitchSlideSpeed = pSrc[1];
				channel->_pitchSlideStepSize = (int8) pSrc[2];
				channel->_pitchSlideDurationCounter = pSrc[3];
				channel->_pitchSlideCounter = 1;
				channel->_pSrc += 4;
				break;

			case 0xFB: // Note duration offset
				channel->_noteDurationOffset = (int8) pSrc[1];
				channel->_pSrc += 2;
				break;

			case 0xFC: // Program change
				channel->_program = pSrc[1];
				sendProgramChange(midiChannel, channel->_program);
				channel->_pSrc += 2;
				break;

			case 0xFD: { // Loop to start
				// Reload data to completely reset channel state, but keep
				// fade-out to stop state.
				bool fadeOutActive = channel->_fadeOutActive;
				channel->loadData(channel->_soundDataStart);
				channel->_fadeOutActive = fadeOutActive;
				break;
			}

			case 0xFE: { // Outer loop
				byte repeats = pSrc[1];
				if (channel->_outerLoopCounter == 0) {
					// Outer loop is inactive. Set loop start point or start looping.
					if (repeats == 0) {
						// Event data byte is 0: set loop start point after this event.
						// Also set inner loop start point to keep it contained within
						// the outer loop.
						channel->_pSrc += 2;
						channel->_outerLoopStart = channel->_pSrc;
						channel->_innerLoopStart = channel->_pSrc;
						channel->_outerLoopCounter = 0;
						channel->_innerLoopCounter = 0;
					} else {
						// Event data byte is > 0: set number of repeats and loop back
						// to the loop start point. Also reset the inner loop start
						// point to the outer loop start point.
						channel->_outerLoopCounter = repeats;
						channel->_pSrc = channel->_outerLoopStart;
						channel->_innerLoopStart = channel->_outerLoopStart;
					}
				} else if (--channel->_outerLoopCounter > 0) {
					// Outer loop is active and counter is >= 1: loop back to the loop
					// start point. Also reset the inner loop start point to the outer
					// loop start point.
					channel->_pSrc = channel->_outerLoopStart;
					channel->_innerLoopStart = channel->_outerLoopStart;
				} else {
					// Outer loop is active and was counted down to 0. Continue playback
					// with the first event after the loop end point.
					// This also sets both loop start points to that event.
					channel->_pSrc += 2;
					channel->_outerLoopStart = channel->_pSrc;
					channel->_innerLoopStart = channel->_pSrc;
				}
				break;
			}

			case 0xFF: { // Inner loop
				byte repeats = pSrc[1];
				if (channel->_innerLoopCounter == 0) {
					// Inner loop is inactive. Set loop start point or start looping.
					if (repeats == 0) {
						// Event data byte is 0: set loop start point after this event.
						channel->_pSrc += 2;
						channel->_innerLoopStart = channel->_pSrc;
						channel->_innerLoopCounter = 0;
					} else {
						// Event data byte is > 0: set number of repeats and loop back
						// to the loop start point.
						channel->_innerLoopCounter = repeats;
						channel->_pSrc = channel->_innerLoopStart;
					}
				} else if (--channel->_innerLoopCounter > 0) {
					// Inner loop is active and counter is >= 1: loop back to the loop
					// start point.
					channel->_pSrc = channel->_innerLoopStart;
				} else {
					// Inner loop is active and was counted down to 0. Continue playback
					// with the first event after the loop end point.
					// This also sets the loop start point to that event.
					channel->_pSrc += 2;
					channel->_innerLoopStart = channel->_pSrc;
				}
				break;
			}
			
			default:
				// When encountering an invalid opcode, the original code would
				// not advance the sound data pointer or break out of the event
				// processing loop. This causes the code to hang on this opcode.
				// Instead, playback for this channel is stopped here.
				warning("Invalid sound opcode %02X encountered; stopping channel", opcode);
				channel->_deltaCounter = 0;
				return;
			}
		}
	}

	// Process effects

	// Volume fade
	if (channel->_volumeFadeStepSize > 0 && --channel->_volumeFadeCounter == 0) {
		channel->_volumeFadeCounter = channel->_volumeFadeSpeed;
		int newVolume = channel->_volume + channel->_volumeFadeStepSize;
		if (newVolume < 0 || newVolume > 0x7F) {
			// Volume has reached minimum or maximum volume. Stop the fade.
			channel->_volumeFadeStepSize = 0;
			newVolume = CLIP(newVolume, 0, 0x7F);
		}
		channel->_volume = newVolume;
		sendVolume(midiChannel, newVolume);
	}

	// Pitch slide
	if (channel->_pitchSlideStepSize != 0) {
		if (--channel->_pitchSlideCounter == 0) {
			channel->_pitchSlideCounter = channel->_pitchSlideSpeed;
			int newPitchBend = channel->_pitchBend + channel->_pitchSlideStepSize;
			if (newPitchBend < 0 || newPitchBend > 0x7F) {
				// There is no bounds checking in the original code. Overflow or
				// underflow will cause the driver to output invalid MIDI events.
				// This actually happens in the original code, in the intro,
				// when Rex lands his ship.
				warning("Pitch bend overflow or underflow; terminating pitch slide");
				channel->_pitchSlideStepSize = 0;
				newPitchBend = CLIP(newPitchBend, 0, 0x7F);
			}
			channel->_pitchBend = newPitchBend;
			sendPitchBend(midiChannel, channel->_pitchBend);
		}
		if (--channel->_pitchSlideDurationCounter == 0)
			channel->_pitchSlideStepSize = 0;
	}

	// Panning sweep
	if (channel->_panningSweepStepSize != 0 && --channel->_panningSweepCounter == 0) {
		channel->_panningSweepCounter = channel->_panningSweepSpeed;
		byte newPanning = channel->_panning + channel->_panningSweepStepSize;
		if (newPanning > 0x7F) {
			// This inverts overflow or underflow back into the valid range
			// of 0x00-0x7F; f.e. -3 becomes 2 and 0x83 becomes 0x7C.
			newPanning = (newPanning ^ 0x7F) & 0x7F;
			// This causes the next sweep step to be executed in 256 ticks.
			// This is accurate to the original code, but maybe the
			// intention was to stop the sweep by setting the step size
			// to 0?
			channel->_panningSweepCounter = 0;
		}
		channel->_panning = newPanning;
		sendPanning(midiChannel, newPanning);
	}
}

void RSound::processTickAllChannels() {
	for (int i = 1; i <= RSOUND_CHANNEL_COUNT; ++i)
		Channel_processTick(getChannel(i));
}

void RSound::processTick() {
	generateRandomNumber();
	if (_ticksProcessingDisabled)
		return;

	tickCallback();

	++_ticksSinceLastCommand;
	processTickAllChannels();
	processChannelFadeOuts();
}

/*-----------------------------------------------------------------------*/

/**
 * Zeroes _deltaCounter and the three fade-step fields for channels in
 * [first, last].
 * Deliberately does NOT touch the loop pointers, volume, program, pan etc,
 * matching the original.
 */
void RSound::resetChannelRange(int firstChannel, int lastChannel, bool includeChannel9) {
	_ticksProcessingDisabled = true;

	for (int i = firstChannel; i <= lastChannel; ++i) {
		Channel *chan = getChannel(i);
		chan->_deltaCounter = 0;
		chan->_pitchSlideStepSize = 0;
		chan->_volumeFadeStepSize = 0;
		chan->_panningSweepStepSize = 0;
	}

	if (lastChannel <= 8 && includeChannel9) {
		Channel *chan = getChannel(9);
		chan->_deltaCounter = 0;
		chan->_pitchSlideStepSize = 0;
		chan->_volumeFadeStepSize = 0;
		chan->_panningSweepStepSize = 0;
	}

	_ticksProcessingDisabled = false;
}

void RSound::clearActiveNotes() {
	for (int i = 0; i < RSOUND_CHANNEL_COUNT + 1; ++i)
		for (int j = 0; j < RSOUND_ACTIVE_NOTES_COUNT; ++j)
			_activeNotes[i][j] = 0xFF;
}

void RSound::clearActiveNotesRange(int firstChannel, int lastChannel) {
	assert(firstChannel >= 1 && lastChannel <= RSOUND_CHANNEL_COUNT &&
			firstChannel <= lastChannel);
	for (int channel = firstChannel; channel <= lastChannel; ++channel)
		for (int slot = 0; slot < RSOUND_ACTIVE_NOTES_COUNT; ++slot)
			_activeNotes[channel][slot] = 0xFF;
}

byte *RSound::loadData(int offset) {
	return &_soundData[offset];
}

/**
 * Resets all 9 channels and the active notes table.
 * Called both from the constructor (mirroring rsound_init) and from
 * command0.
 */
void RSound::resetAllChannels() {
	resetChannelRange(1, RSOUND_CHANNEL_COUNT);
	clearActiveNotes();
}

/**
 * Resets the MIDI channel state (all notes off, reset all
 * controllers, volume=100, pan=center) of MIDI channels [first, last]
 * (both inclusive, 1-based). Shared tail used by command0/command2/command4.
 */
void RSound::sendMidiChannelReset(int first, int last, bool includeChannel9) {
	for (int ch = first; ch <= last; ++ch) {
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_VOLUME, 100);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | ch, MidiDriver::MIDI_CONTROLLER_PANNING, 0x40);
	}
	if (last <= 8 && includeChannel9) {
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | 9, MidiDriver::MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | 9, MidiDriver::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS, 0);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | 9, MidiDriver::MIDI_CONTROLLER_VOLUME, 100);
		_midiDriver->send(MidiDriver::MIDI_COMMAND_CONTROL_CHANGE | 9, MidiDriver::MIDI_CONTROLLER_PANNING, 0x40);
	}
}

int RSound::command0() {
	resetAllChannels();
	setFadeOutSpeed(0);
	sendMidiChannelReset(1, RSOUND_CHANNEL_COUNT);

	// Matches the tail of the original rsound_command0.
	// _sysExOffset is this driver's own command0_array offset, supplied
	// via the constructor (0x67 for rsound.001, 0x87 for rsound.002, 0x6F
	// for rsound.009) - each driver's own resource file carries its own
	// copy of this table, so no per-driver command0() override is needed.
	sendSysEx(_sysExOffset);

	return 0;
}

int RSound::command1() {
	setFadeOutSpeed(1);
	for (int i = 1; i <= RSOUND_CHANNEL_COUNT; ++i)
		getChannel(i)->setFadeOut(true);
	return 0;
}

int RSound::command2() {
	// Initialize the static channels (also reinitializes the active notes
	// table) plus the MIDI channel reset for those same channels.
	resetChannelRange(1, _dynamicStartChannel - 1, _staticIncludeChannel9);
	clearActiveNotes();
	setFadeOutSpeed(0);
	sendMidiChannelReset(1, _dynamicStartChannel - 1, _staticIncludeChannel9);
	sendSysEx(_sysExOffset);
	return 0;
}

int RSound::command3() {
	// Start fade-out to stop for the static channels.
	setFadeOutSpeed(1);
	for (int i = 1; i < _dynamicStartChannel; ++i)
		getChannel(i)->setFadeOut(true);
	if (_staticIncludeChannel9)
		getChannel(9)->setFadeOut(true);
	return 0;
}

int RSound::command4() {
	// Initialize the dynamic Channels (does NOT touch the active notes
	// table) plus the MIDI channel reset for those same channels.
	resetChannelRange(_dynamicStartChannel, 8, _dynamicIncludeChannel9);
	setFadeOutSpeed(0);
	sendMidiChannelReset(_dynamicStartChannel, 8, _dynamicIncludeChannel9);
	return 0;
}

int RSound::command5() {
	// Start fade-out to stop for the dynamic channels.
	setFadeOutSpeed(1);
	for (int i = _dynamicStartChannel; i <= (_dynamicIncludeChannel9 ? 9 : 8); ++i)
		getChannel(i)->setFadeOut(true);
	return 0;
}

int RSound::command6() {
	_ticksProcessingDisabled = true;
	_mute = true;
	for (int ch = 1; ch <= RSOUND_CHANNEL_COUNT; ++ch)
		muteChannel(ch);
	return 0;
}

int RSound::command7() {
	for (int i = 1; i <= RSOUND_CHANNEL_COUNT; ++i)
		unmuteChannel(i, getChannel(i)->_volume);
	_mute = false;
	_ticksProcessingDisabled = false;
	return 0;
}

int RSound::command8() {
	int result = 0;
	for (int i = 1; i <= RSOUND_CHANNEL_COUNT; ++i)
		result |= getChannel(i)->_deltaCounter;

	return result;
}

int RSound::nullCommand() {
	return 0;
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
