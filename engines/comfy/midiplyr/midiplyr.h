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

#ifndef COMFY_MIDIPLYR_MIDIPLYR_H
#define COMFY_MIDIPLYR_MIDIPLYR_H

#include "common/mutex.h"
#include "common/scummsys.h"

#define MIDIPLYR_CHANNEL_COUNT 16
#define MIDIPLYR_FREE_CHANNEL 0x10
#define MIDIPLYR_NOTE_CAPACITY 64
#define MIDIPLYR_DEFERRED_CC7_COUNT_DRV_V1 102
#define MIDIPLYR_DEFERRED_CC7_COUNT_DRV_V2 64
#define MIDIPLYR_DEFERRED_CC7_COUNT_MAX MIDIPLYR_DEFERRED_CC7_COUNT_DRV_V1
#define MIDIPLYR_CC7_FLUSH_SENTINEL 999
#define MIDIPLYR_TIMER_PERIOD 10000

class MidiDriver;

namespace Comfy {

class MidiPlyrDriver {
public:
	enum DriverVersion {
		kDriverVersion1, // 1994 MIDIPLYR.DLL
		kDriverVersion2  // 1999 MIDIPLYR.DLL
	};

	static const uint kTrackCount = 6;

	explicit MidiPlyrDriver(DriverVersion driverVersion);
	~MidiPlyrDriver();

	bool musicStart(byte flags);
	void musicEnd();
	void musicPlaySong(const byte *data, uint32 size, uint16 track);
	void musicStopSong(byte fullStop, uint16 track);
	void musicStopAll(byte fullStop);
	void musicContinueSong(uint16 track);
	bool musicIsSongPlaying(uint16 track) const;
	uint16 musicGetClearMarker(uint16 track);
	void musicSetVolume(uint16 volume, uint16 track);
	void musicSetPitch(uint16 pitch, byte track);
	void musicSetRate(uint16 rate, byte track);
	void musicSetLoop(byte flag, uint16 track);
	uint16 getMidiDevId() const;

	void timeCallback();
	void setTimeCounter(uint32 value);
	uint32 getTimeCounter() const;
	void setVocCounter(uint32 value);
	uint32 getVocCounter() const;
	void setIncreaseVocCounter(byte flag);

	void vocSrResetCounters();
	void vocSrResetBlockNo();
	void vocSrGetCounters(uint32 &timeFrac, uint16 &blockNo) const;
	void vocSrUpdateCounters();

	uint32 comfyboardGetButtons() const;
	void comfyboardSetHostButtons(uint32 buttons);
	void comfyboardStartReading(uint32 xorMask, uint16 port);
	void comfyboardStopReading();
	void comfyboardStartSleepUse();
	void comfyboardStopSleepUse();

private:
	struct Track {
		uint16 state;
		const byte *streamBase;
		uint16 eventDataSize;
		uint16 eventCursor;
		byte channelState[MIDIPLYR_CHANNEL_COUNT];
		uint16 rate;
		uint32 startTick;
		byte volume;
		uint16 pitch;
		uint16 marker;
		byte midiLoopFlag;
		uint32 streamSize;
		bool streamFault;

		Track() {
			state = 0;
			streamBase = nullptr;
			eventDataSize = 0;
			eventCursor = 0;

			for (uint channel = 0; channel < MIDIPLYR_CHANNEL_COUNT; channel++)
				channelState[channel] = MIDIPLYR_FREE_CHANNEL;

			rate = 0;
			startTick = 0;
			volume = 0;
			pitch = 0;
			marker = 0;
			midiLoopFlag = 0;
			streamSize = 0;
			streamFault = false;
		}
	};

	struct Note {
		uint16 channel;
		uint16 pitch;

		Note() {
			channel = 0;
			pitch = 0;
		}
	};

	struct ChannelSlot {
		byte inUse;
		byte volumePercent;

		ChannelSlot() {
			inUse = 0;
			volumePercent = 100;
		}
	};

	static void timerProc(void *refCon);

	void resetState();
	void uninstallTimer();
	uint32 midiFindDevice();
	bool midiOpen();
	void midiClose();
	void midiSendShortMessage(uint32 message);
	void midiSendNoteOff(uint16 channel, uint16 pitch, uint16 velocity);
	void midiSendNoteOn(uint16 channel, uint16 pitch, uint16 velocity);
	void midiSendProgramChange(uint16 channel, uint16 program);
	void midiSendControlChange(uint16 channel, uint16 value);
	void midiAllNotesOff(byte channel);
	byte midiFindFreeChannel() const;
	byte midiAllocChannel();
	void midiFreeChannel(byte channel);
	void midiFreeAllChannels();
	void midiSetChannelValue(byte channel, byte value);
	byte midiGetChannelValue(byte channel) const;
	void noteTrackAdd(uint16 channel, uint16 pitch);
	void noteTrackRemove(uint16 channel, uint16 pitch);
	uint deferredCc7Count() const;

	void musicStopSongInternal(byte fullStop, uint16 track);
	void musicStopAllInternal(byte fullStop);
	void musicSetRateInternal(uint16 rate, byte track);
	void musicSetPitchInternal(uint16 pitch, byte track);
	void trackAdvance(uint16 track);
	void sequencerTick();
	void sequencerParseEvent(uint16 track);
	byte streamReadByte(Track &track);
	uint32 streamReadVarLen(Track &track);
	void streamSkipBytes(Track &track, byte count);
	void timeCallbackInternal();
	void comfyboardPollTimer();
	void vocSrUpdateCountersInternal();

	DriverVersion _driverVersion;
	MidiDriver *_midiDriver = nullptr;
	mutable Common::Mutex _mutex;
	bool _started = false;
	bool _timerInstalled = false;

	byte _noteCount = 0;
	uint16 _midiDeviceId = 0;
	uint32 _midiDeviceHandle = 0;
	uint16 _deferredCc7QueuedCount = 0;
	uint16 _deferredCc7[MIDIPLYR_DEFERRED_CC7_COUNT_MAX];
	uint16 _deferredCc7Order[MIDIPLYR_CHANNEL_COUNT];
	byte _sendDirectGuard = 0;
	uint32 _seqCurrentTick = 0;
	uint16 _vocBlockNo = 0;
	uint32 _vocUnknown = 0;

	byte _comfyboardStopReading = 1;
	uint16 _comfyboardSampleIndex = 0;
	uint32 _comfyboardButtons = 0;
	uint32 _comfyboardHostButtons = 0;
	byte _comfyboardSleepUseStopped = 1;
	uint16 _comfyboardPort = 0;
	uint32 _comfyboardXorMask = 0;
	byte _comfyboardSamples[8];
	byte _comfyboardSavedControl = 0;

	ChannelSlot _channels[MIDIPLYR_CHANNEL_COUNT];
	uint16 _timeFracHi = 0;
	uint16 _timeFracLo = 0;
	byte _increaseVocCounter = 0;
	uint32 _tempoAcc = 0;
	uint32 _timeFracAcc = 0;
	Note _notes[MIDIPLYR_NOTE_CAPACITY];
	uint16 _midiOutResult = 0;
	Track _tracks[kTrackCount];
	uint16 _midiFileFormat = 0;
	uint32 _ticksPerQuarter = 0;
	uint16 _channelAvailMask = 0xFFFF;
	uint16 _drumsChannel = 9;
	uint16 _midiNoteCountCap = 0;
	uint16 _vocSnapshotHi = 0;
	uint16 _vocSnapshotLo = 0;
};

} // End of namespace Comfy

#endif // COMFY_MIDIPLYR_MIDIPLYR_H
