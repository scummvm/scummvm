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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AUDIO_MILES_MIDIDRIVER_H
#define AUDIO_MILES_MIDIDRIVER_H

#include "audio/mididrv.h"
#include "common/error.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/stream.h"

namespace Audio {

#define MILES_MIDI_CHANNEL_COUNT 16
#define MILES_RHYTHM_CHANNEL 9

// Miles Audio supported controllers for control change messages
#define MILES_CONTROLLER_SELECT_PATCH_BANK 114
#define MILES_CONTROLLER_PROTECT_VOICE 112
#define MILES_CONTROLLER_PROTECT_TIMBRE 113
#define MILES_CONTROLLER_LOCK_CHANNEL 110
#define MILES_CONTROLLER_PROTECT_CHANNEL 111
#define MILES_CONTROLLER_BANK_SELECT_MSB 0
#define MILES_CONTROLLER_BANK_SELECT_LSB 32
#define MILES_CONTROLLER_MODULATION 1
#define MILES_CONTROLLER_VOLUME 7
#define MILES_CONTROLLER_EXPRESSION 11
#define MILES_CONTROLLER_PANNING 10
#define MILES_CONTROLLER_SUSTAIN 64
#define MILES_CONTROLLER_PITCH_RANGE 6
#define MILES_CONTROLLER_RESET_ALL 121
#define MILES_CONTROLLER_ALL_NOTES_OFF 123
#define MILES_CONTROLLER_OMNI_ON 124
#define MILES_CONTROLLER_OMNI_OFF 125
#define MILES_CONTROLLER_MONO_ON 126
#define MILES_CONTROLLER_POLY_ON 127
#define MILES_CONTROLLER_PATCH_REVERB 59
#define MILES_CONTROLLER_PATCH_BENDER 60
#define MILES_CONTROLLER_REVERB_MODE 61
#define MILES_CONTROLLER_REVERB_TIME 62
#define MILES_CONTROLLER_REVERB_LEVEL 63
#define MILES_CONTROLLER_RHYTHM_KEY_TIMBRE 58

// 3 SysEx controllers, each range 5
// 32-36 for 1st queue
// 37-41 for 2nd queue
// 42-46 for 3rd queue
#define MILES_CONTROLLER_SYSEX_RANGE_BEGIN 32
#define MILES_CONTROLLER_SYSEX_RANGE_END 46

#define MILES_CONTROLLER_SYSEX_QUEUE_COUNT 3
#define MILES_CONTROLLER_SYSEX_QUEUE_SIZE 32

#define MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS1   0
#define MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS2   1
#define MILES_CONTROLLER_SYSEX_COMMAND_ADDRESS3   2
#define MILES_CONTROLLER_SYSEX_COMMAND_DATA       3
#define MILES_CONTROLLER_SYSEX_COMMAND_FINAL_DATA 4

#define MILES_CONTROLLER_XMIDI_RANGE_BEGIN 110
#define MILES_CONTROLLER_XMIDI_RANGE_END 120

// Miles Audio actually used 0x4000, because they didn't shift the 2 bytes properly
#define MILES_PITCHBENDER_DEFAULT 0x2000

// The maximum number of sources sending MIDI data to this driver.
// This is based on the requirements of the KYRA engine, but can be increased if
// necessary.
#define MILES_MAXIMUM_SOURCES 4
// Maximum number of tracked active notes for the MT-32
// This is the maximum polyphony of the MT-32 plus some overhead (MIDI data may send
// more notes than the MT-32 can handle simultaneously).
#define MILES_MT32_ACTIVE_NOTES 48
// Maximum number of tracked active notes for GM
// This is the maximum polyphony of the SC-88 and AWE64 plus some overhead (MIDI data
// may send more notes than the GM device can handle simultaneously).
#define MILES_GM_ACTIVE_NOTES 96

#define MILES_MT32_PATCHES_COUNT 128
#define MILES_MT32_CUSTOMTIMBRE_COUNT 64

#define MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE 14
#define MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE 58
#define MILES_MT32_PATCHDATA_PARTIALPARAMETERS_COUNT 4
#define MILES_MT32_PATCHDATA_TOTAL_SIZE (MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE + (MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE * MILES_MT32_PATCHDATA_PARTIALPARAMETERS_COUNT))

struct MilesMT32InstrumentEntry {
	byte bankId;
	byte patchId;
	byte commonParameter[MILES_MT32_PATCHDATA_COMMONPARAMETER_SIZE + 1];
	byte partialParameters[MILES_MT32_PATCHDATA_PARTIALPARAMETERS_COUNT][MILES_MT32_PATCHDATA_PARTIALPARAMETER_SIZE + 1];
};

/**
 * Abstract class containing the interface for loading
 * the XMIDI timbres specified in the timbre chunks of
 * an XMIDI file.
 */
class MidiDriver_Miles_Xmidi_Timbres {
public:
	/**
	 * Processes the timbre chunk specified for a track
	 * in an XMIDI file. This will load the necessary
	 * timbres into the MIDI device using SysEx messages.
	 *
	 * This function will likely return before all SysEx
	 * messages have been sent. Use the isReady method to
	 * check if the driver has finished preparing for
	 * playback. Playback should not be started before
	 * this process has finished.
	 */
	virtual void processXMIDITimbreChunk(const byte *timbreListPtr, uint32 timbreListSize) = 0;
	virtual ~MidiDriver_Miles_Xmidi_Timbres() { }
};

class MidiDriver_Miles_Midi : public MidiDriver, public MidiDriver_Miles_Xmidi_Timbres {
public:
	MidiDriver_Miles_Midi(MusicType midiType, MilesMT32InstrumentEntry *instrumentTablePtr, uint16 instrumentTableCount);
	virtual ~MidiDriver_Miles_Midi();

public:
	// MidiDriver
	int open() override;
	// Open the Miles driver using the specified MidiDriver instance.
	int open(MidiDriver *driver, bool nativeMT32);
	void close() override;
	bool isOpen() const override { return _isOpen; }

	using MidiDriver_BASE::send;
	void send(uint32 b) override;
	void send(int8 source, uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;
	uint16 sysExNoDelay(const byte *msg, uint16 length) override;
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;

	/**
	 * De-initialize a source. Call this after playing a track or sound effect using this source.
	 * This will unlock and unprotect channels used by this source and stop any active notes
	 * from this source.
	 * Automatically executed when an End Of Track meta event is received.
	 */
	void deinitSource(uint8 source);
	/**
	 * Set the volume for this source. This will be used to scale the volume values in the MIDI
	 * data from this source. Expected volume values are 0 - 256.
	 * Note that source volume remains set for the source number even after deinitializing the
	 * source. If the same source numbers are consistently used for music and SFX sources, the
	 * source volume will only need to be set once.
	 */
	void setSourceVolume(uint8 source, uint16 volume);

	void stopAllNotes(bool stopSustainedNotes = false) override;

	MidiChannel *allocateChannel() override {
		if (_driver)
			return _driver->allocateChannel();
		return NULL;
	}
	MidiChannel *getPercussionChannel() override {
		if (_driver)
			return _driver->getPercussionChannel();
		return NULL;
	}

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override {
		_timer_param = timer_param;
		_timer_proc = timer_proc;
	}

	void onTimer();

	uint32 getBaseTempo() override {
		if (_driver) {
			return _driver->getBaseTempo();
		}
		return 1000000 / _baseFreq;
	}

protected:
	Common::Mutex _mutex;
	MidiDriver *_driver;
	bool _isOpen;
	// The type of MIDI data supplied to the driver: MT-32 or General MIDI.
	MusicType _midiType;
	// True if the MIDI output is an MT-32 (hardware or 100% emulated),
	// false if the MIDI output is a General MIDI device.
	bool _nativeMT32;
	// True if the General MIDI output supports Roland GS for improved MT-32 mapping.
	bool _enableGS;

	// Bitmask of the MIDI channels in use by the output device
	uint16 _outputChannelMask;

	int _baseFreq;
	uint32 _timerRate;

public:
	void processXMIDITimbreChunk(const byte *timbreListPtr, uint32 timbreListSize) override;
	bool isReady() override { return _sysExQueue.empty(); }

private:
	void initMidiDevice();

	void MT32SysEx(const uint32 targetAddress, const byte *dataPtr, bool useSysExQueue = false);

	uint32 calculateSysExTargetAddress(uint32 baseAddress, uint32 index);

	void writeRhythmSetup(byte note, byte customTimbreId);
	void writePatchTimbre(byte patchId, byte timbreGroup, byte timbreId, bool useSysExQueue = false);
	void writePatchByte(byte patchId, byte index, byte patchValue);
	void writeToSystemArea(byte index, byte value);

	const MilesMT32InstrumentEntry *searchCustomInstrument(byte patchBank, byte patchId);
	int16 searchCustomTimbre(byte patchBank, byte patchId);

	void setupPatch(byte patchBank, byte patchId, bool useSysExQueue = false);
	int16 installCustomTimbre(byte patchBank, byte patchId);

	bool isOutputChannelUsed(uint8 outputChannel) { return _outputChannelMask & (1 << outputChannel); }

private:
	/**
	 * This stores the values of the MIDI controllers for
	 * a MIDI channel. It is used to keep track of controller
	 * values while a channel is locked, so they can be
	 * restored when the channel is unlocked.
	 */
	struct MidiChannelControlData {
		// The source that last sent an event to this channel
		int8 source;
		// True if the source volume has been applied to this channel
		bool sourceVolumeApplied;

		byte program;

		uint16 pitchWheel;

		byte modulation;
		// The volume specified by the MIDI data
		byte volume;
		// The volume scaled using the source volume
		byte scaledVolume;
		byte panPosition;
		byte expression;
		bool sustain;

		// Custom timbre data
		byte   currentPatchBank;

		bool   usingCustomTimbre;
		byte   currentCustomTimbreId;

		MidiChannelControlData() : source(-1),
			sourceVolumeApplied(false),
			program(0),
			pitchWheel(MILES_PITCHBENDER_DEFAULT),
			modulation(0),
			volume(0xFF),
			scaledVolume(0x64),
			panPosition(0x40),
			expression(0x7F),
			sustain(false),
			currentPatchBank(0),
			usingCustomTimbre(false),
			currentCustomTimbreId(0) { }
	};

	struct MidiChannelEntry {
		// True if this channel is locked. A locked channel will
		// only accept MIDI messages from the source that locked it.
		bool   locked;
		// The channel in the MIDI data of the lock source that
		// is assigned to this locked output channel. This is a
		// reverse lookup for MidiSource::channelMap.
		// -1 if the channel is not locked.
		int8   lockDataChannel;
		// True if this channel is protected from locking.
		// The channel can still be locked, but unprotected
		// channels will be prioritized.
		bool   lockProtected;
		// The source that protected this channel from locking.
		// -1 if the channel is not protected.
		int8   protectedSource;

		// The number of notes currently active on the channel.
		uint8  activeNotes;

		// The MIDI controller values currently used by the channel.
		MidiChannelControlData currentData;
		// The MIDI controller values set by the sources which are
		// not currently using the channel because it is locked.
		// These values will be set on the channel when the channel
		// is unlocked.
		MidiChannelControlData unlockData;

		MidiChannelEntry() : locked(false),
			lockDataChannel(-1),
			lockProtected(false),
			protectedSource(-1),
			activeNotes(0) { }
	};

	/**
	 * Send out a control change MIDI message using the specified data.
	 * @param controlData The new MIDI controller value will be set on this MidiChannelControlData
	 * @param sendMessage True if the message should be sent out to the device
	 */
	void controlChange(byte outputChannel, byte controllerNumber, byte controllerValue, int8 source, MidiChannelControlData &controlData, bool sendMessage);
	/**
	 * Removes active notes from the active notes registration on the specified channel.
	 * @param sustainedNotes True if only sustained notes should be removed; otherwise only regular active notes will be removed
	 */
	void removeActiveNotes(uint8 outputChannel, bool sustainedNotes);
	/**
	 * Find and lock an output channel and reserve it for the specified
	 * source. The output channel will be mapped to the specified data
	 * channel.
	 */
	void lockChannel(uint8 source, uint8 dataChannel);
	/**
	 * Find an output channel to lock. This will be based on the number
	 * of active notes on the channels and whether the channel is
	 * protected or not.
	 * @param useProtectedChannels When true, protected channels are considered for locking
	 * @returns The output channel to lock, or -1 if no channel is available
	 */
	int8 findLockChannel(bool useProtectedChannels = false);
	/**
	 * Unlock an output channel. This will stop all notes on the channel,
	 * restore the controller values and make it available to other sources.
	 */
	void unlockChannel(uint8 outputChannel);
	/**
	 * Send a program change MIDI message using the specified data.
	 * @param controlData The new program value will be set on this MidiChannelControlData
	 * @param sendMessage True if the message should be sent out to the device
	 */
	void programChange(byte outputChannel, byte patchId, uint8 source, MidiChannelControlData &controlData, bool sendMessage);

	void stopNotesOnChannel(uint8 outputChannelNumber);

	struct MidiCustomTimbreEntry {
		bool   used;
		bool   protectionEnabled;
		byte   currentPatchBank;
		byte   currentPatchId;

		uint32 lastUsedNoteCounter;

		MidiCustomTimbreEntry() : used(false),
			protectionEnabled(false),
			currentPatchBank(0),
			currentPatchId(0),
			lastUsedNoteCounter(0) {}
	};

	struct MilesMT32SysExQueueEntry {
		uint32 targetAddress;
		byte   dataPos;
		byte   data[MILES_CONTROLLER_SYSEX_QUEUE_SIZE + 1]; // 1 extra byte for terminator

		MilesMT32SysExQueueEntry() : targetAddress(0),
			dataPos(0) {
			memset(data, 0, sizeof(data));
		}
	};

	/**
	 * This stores data about a specific source of MIDI data.
	 */
	struct MidiSource {
		// The source volume as set by ScummVM (music/SFX volume)
		uint16 volume;
		// The mapping of MIDI data channels to output channels
		// for this source.
		int8 channelMap[MILES_MIDI_CHANNEL_COUNT];

		MidiSource() : volume(256) {
			memset(channelMap, 0, sizeof(channelMap));
		}
	};

	// stores information about all MIDI channels
	MidiChannelEntry _midiChannels[MILES_MIDI_CHANNEL_COUNT];

	// stores information about all custom timbres
	MidiCustomTimbreEntry _customTimbres[MILES_MT32_CUSTOMTIMBRE_COUNT];

	byte _patchesBank[MILES_MT32_PATCHES_COUNT];

	// holds all instruments
	MilesMT32InstrumentEntry *_instrumentTablePtr;
	uint16                   _instrumentTableCount;

	uint32 _noteCounter; // used to figure out, which timbres are outdated

	// Queues for Miles SysEx controllers
	MilesMT32SysExQueueEntry _milesSysExQueues[MILES_CONTROLLER_SYSEX_QUEUE_COUNT];

	// MIDI sources sending messages to this driver.
	MidiSource _sources[MILES_MAXIMUM_SOURCES];

	struct ActiveNote {
		int8 source;
		uint8 channel;
		uint8 note;
		bool sustain;

		ActiveNote() : source(0x7F),
			channel(0xFF),
			note(0xFF),
			sustain(false) { }
	};

	// The maximum number of active notes that have to be tracked for this MIDI device.
	uint8 _maximumActiveNotes;
	// Tracks the notes being played by the MIDI device.
	ActiveNote *_activeNotes;

	/**
	 * Stores data which is to be transmitted as a SysEx message
	 * to a MIDI device. Neither data nor length should include
	 * the SysEx start and stop bytes.
	 */
	struct SysExData {
		byte data[270];
		uint16 length;
		SysExData() : length(0) {
			memset(data, 0, sizeof(data));
		}
	};

	// The number of microseconds to wait before sending the
	// next SysEx message.
	uint32 _sysExDelay;

	/**
	 * Queue of SysEx messages that must be sent to the
	 * MIDI device. Used by processXMIDITimbreChunk to
	 * send SysEx messages before starting playback of
	 * a track.
	 *
	 * Sending other MIDI messages to the driver should
	 * be suspended until all SysEx messages in the
	 * queue have been sent to the MIDI device. Use the
	 * isReady function to check if the driver is ready
	 * to receive other messages.
	 */
	Common::Queue<SysExData> _sysExQueue;

	// Mutex for write access to the SysEx queue.
	Common::Mutex _sysExQueueMutex;

	// External timer callback
	void *_timer_param;
	Common::TimerManager::TimerProc _timer_proc;

public:
	// Callback hooked up to the driver wrapped by the
	// Miles MIDI driver object. Executes onTimer and
	// the external callback set by the
	// setTimerCallback function.
	static void timerCallback(void *data) {
		MidiDriver_Miles_Midi *driver = (MidiDriver_Miles_Midi *) data;
		driver->onTimer();
		if (driver->_timer_proc && driver->_timer_param)
			driver->_timer_proc(driver->_timer_param);
	}
};

extern MidiDriver *MidiDriver_Miles_AdLib_create(const Common::String &filenameAdLib, const Common::String &filenameOPL3, Common::SeekableReadStream *streamAdLib = nullptr, Common::SeekableReadStream *streamOPL3 = nullptr);

extern MidiDriver_Miles_Midi *MidiDriver_Miles_MT32_create(const Common::String &instrumentDataFilename);

extern MidiDriver_Miles_Midi *MidiDriver_Miles_MIDI_create(MusicType midiType, const Common::String &instrumentDataFilename);

} // End of namespace Audio

#endif // AUDIO_MILES_MIDIDRIVER_H
