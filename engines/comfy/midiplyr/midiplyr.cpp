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

#include "comfy/midiplyr/midiplyr.h"

#include "audio/mididrv.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"

namespace Comfy {

MidiPlyrDriver::MidiPlyrDriver(DriverVersion driverVersion) : _driverVersion(driverVersion) {
	resetState();
}

MidiPlyrDriver::~MidiPlyrDriver() {
	musicEnd();
}

void MidiPlyrDriver::resetState() {
	_noteCount = 0;
	_midiDeviceId = 0;
	_midiDeviceHandle = 0;
	_deferredCc7QueuedCount = 0;
	_sendDirectGuard = 0;
	_seqCurrentTick = 0;
	_vocBlockNo = 0;
	_vocUnknown = 0;
	_comfyboardStopReading = 1;
	_comfyboardSampleIndex = 0;
	_comfyboardButtons = 0;
	_comfyboardHostButtons = 0;
	_comfyboardSleepUseStopped = 1;
	_comfyboardPort = 0;
	_comfyboardXorMask = 0;
	_comfyboardSavedControl = 0;
	_timeFracHi = 0;
	_timeFracLo = 0;
	_increaseVocCounter = 0;
	_tempoAcc = 0;
	_timeFracAcc = 0;
	_midiOutResult = 0;
	_midiFileFormat = 0;
	_ticksPerQuarter = 0;
	_channelAvailMask = 0xFFFF;
	_drumsChannel = 9;
	_midiNoteCountCap = 0;
	_vocSnapshotHi = 0;
	_vocSnapshotLo = 0;

	for (uint i = 0; i < MIDIPLYR_DEFERRED_CC7_COUNT_MAX; i++)
		_deferredCc7[i] = 0xFFFF;

	for (uint i = 0; i < MIDIPLYR_CHANNEL_COUNT; i++) {
		_deferredCc7Order[i] = 0;
		_channels[i] = ChannelSlot();
	}

	for (uint i = 0; i < MIDIPLYR_NOTE_CAPACITY; i++)
		_notes[i] = Note();

	for (uint i = 0; i < ARRAYSIZE(_comfyboardSamples); i++)
		_comfyboardSamples[i] = 0;

	for (uint i = 0; i < kTrackCount; i++)
		_tracks[i] = Track();
}

uint32 MidiPlyrDriver::midiFindDevice() {
	_midiDeviceHandle = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	return _midiDeviceHandle;
}

bool MidiPlyrDriver::midiOpen() {
	_midiDriver = MidiDriver::createMidi(midiFindDevice());

	if (!_midiDriver)
		return false;

	_midiOutResult = _midiDriver->open();

	if (_midiOutResult != 0) {
		delete _midiDriver;
		_midiDriver = nullptr;
		return false;
	}

	_channelAvailMask = 0xFFFF;
	_midiNoteCountCap = 0;
	_midiDeviceId = 0;
	return true;
}

void MidiPlyrDriver::midiClose() {
	if (!_midiDriver)
		return;

	_midiDriver->stopAllNotes(true);
	_midiDriver->close();
	delete _midiDriver;
	_midiDriver = nullptr;
}

bool MidiPlyrDriver::musicStart(byte flags) {
	Common::StackLock lock(_mutex);

	if (_started)
		return true;

	resetState();
	_midiFileFormat = flags & 1;

	if (!midiOpen())
		return false;

	for (uint channel = 0; channel < MIDIPLYR_CHANNEL_COUNT; channel++) {
		midiSetChannelValue(channel, 100);
		midiSendControlChange(channel, 100);
	}

	midiFreeAllChannels();

	if ((flags & 2) == 0) {
		_timerInstalled = g_system->getTimerManager()->installTimerProc(
			&timerProc, MIDIPLYR_TIMER_PERIOD, this, "Comfy::MidiPlyrDriver");

		if (!_timerInstalled) {
			midiClose();
			return false;
		}
	}

	_started = true;
	return true;
}

void MidiPlyrDriver::uninstallTimer() {
	_mutex.lock();
	bool removeTimer = _timerInstalled;
	_timerInstalled = false;
	_mutex.unlock();

	// removeTimerProc() waits for an active callback, which may itself be
	// waiting for _mutex, so the mutex must be released before this call...
	if (removeTimer)
		g_system->getTimerManager()->removeTimerProc(&timerProc);
}

void MidiPlyrDriver::musicEnd() {
	uninstallTimer();

	Common::StackLock lock(_mutex);

	if (!_started && !_midiDriver)
		return;

	musicStopAllInternal(1);
	midiClose();
	_started = false;
}

void MidiPlyrDriver::timerProc(void *refCon) {
	static_cast<MidiPlyrDriver *>(refCon)->timeCallback();
}

void MidiPlyrDriver::timeCallback() {
	Common::StackLock lock(_mutex);

	if (_started)
		timeCallbackInternal();
}

void MidiPlyrDriver::timeCallbackInternal() {
	comfyboardPollTimer();

	if (_tempoAcc != 0xFFFFFFFF)
		_tempoAcc++;

	if (_increaseVocCounter && _timeFracAcc != 0xFFFFFFFF) {
		_timeFracAcc++;
		_timeFracHi = _timeFracAcc >> 16;
		_timeFracLo = _timeFracAcc;
	}

	_seqCurrentTick += 100;
	if (_sendDirectGuard == 0) {
		_sendDirectGuard = 1;
		sequencerTick();
	}

	_sendDirectGuard = 0;
}

void MidiPlyrDriver::midiSendShortMessage(uint32 message) {
	if (_midiDriver) {
		_midiDriver->send(message);
		_midiOutResult = 0;
	}
}

void MidiPlyrDriver::noteTrackAdd(uint16 channel, uint16 pitch) {
	// The original has no overflow check. Keep the fixed-size representation,
	// but do not allow malformed data to overwrite adjacent driver state...
	if (_noteCount >= MIDIPLYR_NOTE_CAPACITY)
		return;

	_notes[_noteCount].channel = channel;
	_notes[_noteCount].pitch = pitch;
	_noteCount++;
}

void MidiPlyrDriver::noteTrackRemove(uint16 channel, uint16 pitch) {
	for (uint i = 0; i < _noteCount; i++) {
		if (_notes[i].channel == channel && _notes[i].pitch == pitch) {
			_noteCount--;
			_notes[i] = _notes[_noteCount];
			return;
		}
	}
}

void MidiPlyrDriver::midiSendNoteOff(uint16 channel, uint16 pitch, uint16 velocity) {
	if (_driverVersion == kDriverVersion1 && (channel > 15 || pitch > 127))
		channel--;

	midiSendShortMessage((velocity << 16) | ((pitch & 0xFF) << 8) | ((0x80 + channel) & 0xFFFF));
	noteTrackRemove(channel, pitch);
}

void MidiPlyrDriver::midiSendNoteOn(uint16 channel, uint16 pitch, uint16 velocity) {
	if (_driverVersion == kDriverVersion1 && (channel > 15 || pitch > 127))
		channel--;

	if (velocity == 0) {
		midiSendNoteOff(channel, pitch, 0);
		return;
	}

	midiSendShortMessage((velocity << 16) | ((pitch & 0xFF) << 8) | ((0x90 + channel) & 0xFFFF));
	noteTrackAdd(channel, pitch);
}

void MidiPlyrDriver::midiSendProgramChange(uint16 channel, uint16 program) {
	if (_driverVersion == kDriverVersion1 && (channel > 15 || program > 127))
		channel--;

	midiSendShortMessage(((program & 0xFF) << 8) | ((0xC0 + channel) & 0xFFFF));
}

uint MidiPlyrDriver::deferredCc7Count() const {
	return _driverVersion == kDriverVersion1 ? MIDIPLYR_DEFERRED_CC7_COUNT_DRV_V1 : MIDIPLYR_DEFERRED_CC7_COUNT_DRV_V2;
}

void MidiPlyrDriver::midiSendControlChange(uint16 channel, uint16 value) {
	if (_sendDirectGuard == 0) {
		if (channel >= deferredCc7Count())
			return;

		if (_deferredCc7[channel] == 0xFFFF) {
			if (_deferredCc7QueuedCount >= MIDIPLYR_CHANNEL_COUNT)
				return;

			_deferredCc7Order[_deferredCc7QueuedCount++] = channel;
		}

		_deferredCc7[channel] = value;
		return;
	}

	if (channel != MIDIPLYR_CC7_FLUSH_SENTINEL) {
		midiSendShortMessage((value << 16) | (7 << 8) | (0xB0 + channel));
		return;
	}

	while (_deferredCc7QueuedCount != 0) {
		uint16 queuedChannel = _deferredCc7Order[--_deferredCc7QueuedCount];
		if (queuedChannel >= deferredCc7Count() || _deferredCc7[queuedChannel] == 0xFFFF)
			continue;

		midiSendShortMessage((_deferredCc7[queuedChannel] << 16) | (7 << 8) | (0xB0 + queuedChannel));

		if (_driverVersion == kDriverVersion2 && _midiOutResult != 0) {
			_deferredCc7QueuedCount++;
			break;
		}

		_deferredCc7[queuedChannel] = 0xFFFF;
	}
}

void MidiPlyrDriver::midiSetChannelValue(byte channel, byte value) {
	if (channel < MIDIPLYR_CHANNEL_COUNT)
		_channels[channel].volumePercent = value;
}

byte MidiPlyrDriver::midiGetChannelValue(byte channel) const {
	return channel < MIDIPLYR_CHANNEL_COUNT ? _channels[channel].volumePercent : 0;
}

byte MidiPlyrDriver::midiFindFreeChannel() const {
	for (uint channel = 0; channel < MIDIPLYR_CHANNEL_COUNT; channel++) {
		if (channel != _drumsChannel && (_channelAvailMask & (1 << channel)) && !_channels[channel].inUse)
			return channel;
	}

	return MIDIPLYR_FREE_CHANNEL;
}

byte MidiPlyrDriver::midiAllocChannel() {
	byte channel = midiFindFreeChannel();

	if (channel != MIDIPLYR_FREE_CHANNEL)
		_channels[channel].inUse = 1;

	return channel;
}

void MidiPlyrDriver::midiFreeChannel(byte channel) {
	if (channel < MIDIPLYR_CHANNEL_COUNT)
		_channels[channel].inUse = 0;
}

void MidiPlyrDriver::midiFreeAllChannels() {
	for (uint channel = 0; channel < MIDIPLYR_CHANNEL_COUNT; channel++)
		midiFreeChannel(channel);
}

void MidiPlyrDriver::midiAllNotesOff(byte channel) {
	if (channel >= MIDIPLYR_CHANNEL_COUNT)
		channel--;

	for (uint i = _noteCount; i > 0; i--) {
		uint noteIndex = i - 1;

		if (_notes[noteIndex].channel == channel)
			midiSendNoteOff(channel, _notes[noteIndex].pitch, 0);
	}
}

void MidiPlyrDriver::musicSetRateInternal(uint16 rate, byte track) {
	if (track < kTrackCount)
		_tracks[track].rate = rate;
}

void MidiPlyrDriver::musicSetRate(uint16 rate, byte track) {
	Common::StackLock lock(_mutex);
	musicSetRateInternal(rate, track);
}

void MidiPlyrDriver::musicSetPitchInternal(uint16 pitch, byte trackIndex) {
	if (trackIndex >= kTrackCount)
		return;

	Track &track = _tracks[trackIndex];
	for (uint i = 0; i < MIDIPLYR_CHANNEL_COUNT; i++) {
		if (track.channelState[i] != MIDIPLYR_FREE_CHANNEL)
			track.pitch = pitch;
	}
}

void MidiPlyrDriver::musicSetPitch(uint16 pitch, byte track) {
	Common::StackLock lock(_mutex);
	musicSetPitchInternal(pitch, track);
}

void MidiPlyrDriver::musicSetLoop(byte flag, uint16 track) {
	Common::StackLock lock(_mutex);

	if (_driverVersion == kDriverVersion2 && track < kTrackCount)
		_tracks[track].midiLoopFlag = flag;
}

void MidiPlyrDriver::musicPlaySong(const byte *data, uint32 size, uint16 trackIndex) {
	Common::StackLock lock(_mutex);

	if (trackIndex >= kTrackCount || !data || size < 7)
		return;

	Track &track = _tracks[trackIndex];
	uint32 headerSize = _driverVersion == kDriverVersion2 ? 6 : 7;
	for (uint channel = 0; channel < MIDIPLYR_CHANNEL_COUNT; channel++)
		track.channelState[channel] = MIDIPLYR_FREE_CHANNEL;

	track.channelState[_drumsChannel] = _drumsChannel;
	track.streamBase = data + headerSize;
	track.streamSize = size - headerSize;
	track.streamFault = false;
	track.eventCursor = 0;
	track.marker = 0;
	track.midiLoopFlag = 0;
	track.eventDataSize = READ_LE_UINT16(data + 4);

	if (_midiFileFormat == 1)
		_ticksPerQuarter = READ_LE_UINT16(data + 2);
	else
		musicSetRateInternal(READ_LE_UINT16(data + 2), trackIndex);

	musicSetPitchInternal(5000, trackIndex);
	track.startTick = _seqCurrentTick;
	track.state = 1;
	if (_driverVersion == kDriverVersion2) {
		streamReadVarLen(track);

		if (track.streamFault) {
			track.state = 0;
			return;
		}

		_sendDirectGuard = 1;
		midiSendControlChange(MIDIPLYR_CC7_FLUSH_SENTINEL, 0);
		trackAdvance(trackIndex);
		_sendDirectGuard = 0;
	} else {
		trackAdvance(trackIndex);
	}

	track.state = track.streamFault ? 0 : 2;
}

void MidiPlyrDriver::musicStopSongInternal(byte fullStop, uint16 trackIndex) {
	if (trackIndex >= kTrackCount)
		return;

	Track &track = _tracks[trackIndex];
	track.state = 0;
	if (fullStop) {
		track.marker = 0;
		track.eventCursor = 0;

		for (uint i = 0; i < MIDIPLYR_CHANNEL_COUNT; i++) {
			byte channel = track.channelState[i];
			if (channel != MIDIPLYR_FREE_CHANNEL) {
				midiFreeChannel(channel);
				midiSetChannelValue(channel, 100);
				midiAllNotesOff(channel);
			}
		}

		// The original writes one byte past channelState after this loop,
		// changing the low byte of rate instead of clearing the channel array.
		// We replicate this bug...
		track.rate = (track.rate & 0xFF00) | MIDIPLYR_FREE_CHANNEL;
	} else {
		for (uint i = 0; i < MIDIPLYR_CHANNEL_COUNT; i++) {
			if (track.channelState[i] != MIDIPLYR_FREE_CHANNEL)
				midiAllNotesOff(track.channelState[i]);
		}
	}
}

void MidiPlyrDriver::musicStopSong(byte fullStop, uint16 track) {
	Common::StackLock lock(_mutex);
	musicStopSongInternal(fullStop, track);
}

void MidiPlyrDriver::musicStopAllInternal(byte fullStop) {
	for (uint track = 0; track < kTrackCount; track++)
		musicStopSongInternal(fullStop, track);
}

void MidiPlyrDriver::musicStopAll(byte fullStop) {
	Common::StackLock lock(_mutex);
	musicStopAllInternal(fullStop);
}

void MidiPlyrDriver::musicContinueSong(uint16 trackIndex) {
	Common::StackLock lock(_mutex);

	if (trackIndex < kTrackCount && _tracks[trackIndex].eventCursor < _tracks[trackIndex].eventDataSize) {
		_tracks[trackIndex].state = 2;
		_tracks[trackIndex].startTick = _seqCurrentTick;
	}
}

bool MidiPlyrDriver::musicIsSongPlaying(uint16 track) const {
	Common::StackLock lock(_mutex);
	return track < kTrackCount && _tracks[track].state == 2;
}

uint16 MidiPlyrDriver::musicGetClearMarker(uint16 track) {
	Common::StackLock lock(_mutex);

	if (track >= kTrackCount)
		return 0;

	uint16 marker = _tracks[track].marker;
	_tracks[track].marker = 0;
	return marker;
}

void MidiPlyrDriver::musicSetVolume(uint16 volume, uint16 trackIndex) {
	Common::StackLock lock(_mutex);

	if (trackIndex >= kTrackCount)
		return;

	if (_midiFileFormat != 1)
		volume = (int16(volume) * 0x55) / 100;

	volume = (int16(volume) * 0x7F) / 100;
	Track &track = _tracks[trackIndex];

	for (uint i = 0; i < MIDIPLYR_CHANNEL_COUNT; i++) {
		byte channel = track.channelState[i];
		if (channel != MIDIPLYR_FREE_CHANNEL)
			midiSendControlChange(channel, (volume * midiGetChannelValue(channel)) / 100);
	}

	track.volume = volume;
}

byte MidiPlyrDriver::streamReadByte(Track &track) {
	if (!track.streamBase || track.eventCursor >= track.streamSize) {
		track.streamFault = true;
		return 0;
	}

	return track.streamBase[track.eventCursor++];
}

void MidiPlyrDriver::streamSkipBytes(Track &track, byte count) {
	for (uint i = 0; i < count; i++)
		streamReadByte(track);
}

uint32 MidiPlyrDriver::streamReadVarLen(Track &track) {
	uint32 value = 0;
	byte current = 0;

	do {
		current = streamReadByte(track);
		value = (value << 7) | (current & 0x7F);
	} while (!track.streamFault && (current & 0x80));

	return value;
}

void MidiPlyrDriver::sequencerParseEvent(uint16 trackIndex) {
	Track &track = _tracks[trackIndex];
	byte statusByte = streamReadByte(track);
	byte status = statusByte & 0xF0;
	byte logicalChannel = statusByte & 0x0F;
	byte physicalChannel = logicalChannel;

	if (status != 0xF0) {
		if (track.channelState[logicalChannel] == MIDIPLYR_FREE_CHANNEL) {
			physicalChannel = midiAllocChannel();
			track.channelState[logicalChannel] = physicalChannel;

			if (physicalChannel != MIDIPLYR_FREE_CHANNEL)
				midiSendControlChange(physicalChannel, track.volume);
		} else {
			physicalChannel = track.channelState[logicalChannel];
		}
	}

	switch (status) {
	case 0x80: {
		byte pitch = streamReadByte(track);
		midiSendNoteOff(physicalChannel, pitch, streamReadByte(track));
		break;
	}
	case 0x90: {
		byte pitch = streamReadByte(track);
		midiSendNoteOn(physicalChannel, pitch, streamReadByte(track));
		break;
	}
	case 0xA0:
		streamReadByte(track);
		streamReadByte(track);
		break;
	case 0xB0: {
		byte controller = streamReadByte(track);
		byte value = streamReadByte(track);

		if (controller == 7 && _midiFileFormat == 1) {
			byte channelValue = (value * 100) / 127;
			midiSetChannelValue(physicalChannel, channelValue);
			midiSendControlChange(physicalChannel, (channelValue * track.volume) / 100);
		} else if (controller == 10) {
			track.marker = value;
		}

		break;
	}
	case 0xC0:
		midiSendProgramChange(physicalChannel, streamReadByte(track));
		break;
	case 0xD0:
		streamReadByte(track);
		break;
	case 0xE0:
		streamReadByte(track);
		streamReadByte(track);
		break;
	case 0xF0: {
		byte event = streamReadByte(track);
		byte skipCount = 0;
		switch (event) {
		case 0x2F:
			skipCount = streamReadByte(track);

			if (_driverVersion == kDriverVersion2 && track.midiLoopFlag) {
				track.eventCursor = 0;
				return;
			}

			musicStopSongInternal(1, trackIndex);
			break;
		case 0x14:
			skipCount = streamReadByte(track) - 1;
			streamReadByte(track);
			break;
		case 0x15:
			skipCount = streamReadByte(track) - 2;
			streamReadByte(track);
			streamReadByte(track);
			break;
		case 0x51:
			if (_midiFileFormat == 1) {
				streamReadByte(track);
				uint32 tempo = (streamReadByte(track) << 16) |
					(streamReadByte(track) << 8) | streamReadByte(track);

				if (_ticksPerQuarter)
					musicSetRateInternal(tempo / _ticksPerQuarter, trackIndex);
			} else {
				skipCount = streamReadByte(track);
			}

			break;
		default:
			skipCount = streamReadByte(track);
			break;
		}

		streamSkipBytes(track, skipCount);
		break;
	}
	default:
		streamReadByte(track);
		break;
	}
}

void MidiPlyrDriver::trackAdvance(uint16 trackIndex) {
	if (trackIndex >= kTrackCount)
		return;

	Track &track = _tracks[trackIndex];
	while (!track.streamFault && track.state && track.eventCursor < track.eventDataSize &&
	       track.startTick <= _seqCurrentTick) {

		sequencerParseEvent(trackIndex);

		if (track.streamFault) {
			track.state = 0;
			break;
		}

		uint32 delta = streamReadVarLen(track);

		if (track.streamFault) {
			track.state = 0;
			break;
		}

		if (delta == 0)
			continue;

		uint32 scaled = (track.rate * delta) / 100;
		if (track.state == 1)
			track.startTick = _seqCurrentTick + scaled;
		else
			track.startTick += scaled;

		break;
	}
}

void MidiPlyrDriver::sequencerTick() {
	midiSendControlChange(MIDIPLYR_CC7_FLUSH_SENTINEL, 0);
	bool active = false;
	for (uint track = 0; track < kTrackCount; track++) {
		if (_tracks[track].state == 2)
			trackAdvance(track);

		if (_tracks[track].state)
			active = true;
	}

	if (!active)
		_seqCurrentTick = 0;
}

void MidiPlyrDriver::setIncreaseVocCounter(byte flag) {
	Common::StackLock lock(_mutex);
	_increaseVocCounter = flag;
}

void MidiPlyrDriver::setVocCounter(uint32 value) {
	Common::StackLock lock(_mutex);
	_timeFracAcc = value;
	_timeFracHi = value >> 16;
	_timeFracLo = value;
}

uint32 MidiPlyrDriver::getVocCounter() const {
	Common::StackLock lock(_mutex);
	return _timeFracAcc;
}

void MidiPlyrDriver::setTimeCounter(uint32 value) {
	Common::StackLock lock(_mutex);
	_tempoAcc = value;
}

uint32 MidiPlyrDriver::getTimeCounter() const {
	Common::StackLock lock(_mutex);
	return _tempoAcc;
}

void MidiPlyrDriver::vocSrResetCounters() {
	Common::StackLock lock(_mutex);
	_vocBlockNo = 0;
	_vocUnknown = 0;
	_vocSnapshotHi = 0;
	_vocSnapshotLo = 0;
}

void MidiPlyrDriver::vocSrResetBlockNo() {
	Common::StackLock lock(_mutex);
	_vocBlockNo = 0;
}

void MidiPlyrDriver::vocSrUpdateCountersInternal() {
	_vocBlockNo++;
	_vocSnapshotHi = _timeFracHi;
	_vocSnapshotLo = _timeFracLo;
}

void MidiPlyrDriver::vocSrUpdateCounters() {
	Common::StackLock lock(_mutex);
	vocSrUpdateCountersInternal();
}

void MidiPlyrDriver::vocSrGetCounters(uint32 &timeFrac, uint16 &blockNo) const {
	Common::StackLock lock(_mutex);
	timeFrac = (uint32(_vocSnapshotHi) << 16) | _vocSnapshotLo;
	blockNo = _vocBlockNo;
}

uint16 MidiPlyrDriver::getMidiDevId() const {
	Common::StackLock lock(_mutex);
	return _midiDeviceId;
}

uint32 MidiPlyrDriver::comfyboardGetButtons() const {
	Common::StackLock lock(_mutex);
	return _comfyboardButtons;
}

void MidiPlyrDriver::comfyboardSetHostButtons(uint32 buttons) {
	Common::StackLock lock(_mutex);
	_comfyboardHostButtons = buttons & 0x00FFFFFF;
}

void MidiPlyrDriver::comfyboardStartReading(uint32 xorMask, uint16 port) {
	Common::StackLock lock(_mutex);
	_comfyboardPort = port;
	_comfyboardXorMask = xorMask;
	_comfyboardButtons = 0;
	_comfyboardHostButtons = 0;
	_comfyboardSampleIndex = 0;
	_comfyboardStopReading = 0;
}

void MidiPlyrDriver::comfyboardStopReading() {
	Common::StackLock lock(_mutex);
	_comfyboardStopReading = 1;
	_comfyboardSampleIndex = 0;
	_comfyboardButtons = 0;
	_comfyboardHostButtons = 0;
}

void MidiPlyrDriver::comfyboardStartSleepUse() {
	Common::StackLock lock(_mutex);
	_comfyboardSleepUseStopped = 0;
}

void MidiPlyrDriver::comfyboardStopSleepUse() {
	Common::StackLock lock(_mutex);
	_comfyboardSleepUseStopped = 1;
}

void MidiPlyrDriver::comfyboardPollTimer() {
	if (_driverVersion != kDriverVersion2 || _comfyboardStopReading)
		return;

	if (!_comfyboardSleepUseStopped && !_comfyboardSampleIndex) {
		// The original saves the LPT control byte, writes 0x0C to port + 2, and writes 1 to the data port.
		_comfyboardSavedControl = 0;
	}

	if (_comfyboardSampleIndex) {
		uint16 sampleIndex = _comfyboardSampleIndex - 1;
		uint32 rawButtons = _comfyboardHostButtons ^ _comfyboardXorMask;
		byte bits = (rawButtons >> ((7 - sampleIndex) * 3)) & 7;
		_comfyboardSamples[sampleIndex] = ((bits & 4) << 5) | ((bits & 3) << 4);
	}

	if (_comfyboardSampleIndex >= ARRAYSIZE(_comfyboardSamples)) {
		_comfyboardSampleIndex = 0;
		uint32 packedButtons = 0;

		for (uint i = 0; i < ARRAYSIZE(_comfyboardSamples); i++) {
			uint16 sample = _comfyboardSamples[i];
			uint16 bits = ((sample & 0x80) >> 5) | ((sample & 0x30) >> 4);
			packedButtons = (packedButtons << 3) | bits;
		}

		_comfyboardButtons = packedButtons ^ _comfyboardXorMask;
		return;
	}

	// The original writes ((_comfyboardSampleIndex << 1) | 1) to the LPT data port here.
	_comfyboardSampleIndex++;
}

} // End of namespace Comfy
