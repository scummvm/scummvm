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

#ifndef AUDIO_MIDIDRV_H
#define AUDIO_MIDIDRV_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/timer.h"
#include "common/array.h"

class MidiChannel;

/**
 * @defgroup audio_mididrv MIDI drivers
 * @ingroup audio
 *
 * @brief API for managing MIDI drivers.
 * @{
 */

/**
 * Music types that music drivers can implement and engines can rely on.
 */
enum MusicType {
	MT_INVALID = -1,	// Invalid output
	MT_AUTO = 0,		// Auto
	MT_NULL,			// Null
	MT_PCSPK,			// PC Speaker
	MT_PCJR,			// PCjr
	MT_CMS,				// CMS
	MT_ADLIB,			// AdLib
	MT_C64,				// C64
	MT_AMIGA,			// Amiga
	MT_APPLEIIGS,		// Apple IIGS
	MT_TOWNS,			// FM-TOWNS
	MT_PC98,			// PC98
	MT_SEGACD,			// SegaCD
	MT_GM,				// General MIDI
	MT_MT32,			// MT-32
	MT_GS				// Roland GS
};

/**
 * A set of flags to be passed to detectDevice() which can be used to
 * specify what kind of music driver is preferred / accepted.
 *
 * The flags (except for MDT_PREFER_MT32 and MDT_PREFER_GM) indicate whether a
 * given driver type is acceptable. E.g. the TOWNS music driver could be
 * returned by detectDevice if and only if MDT_TOWNS is specified.
 *
 * MDT_PREFER_MT32 and MDT_PREFER_GM indicate the MIDI device type to use when
 * no device is selected in the music options, or when the MIDI device selected
 * does not match the requirements of a game engine. With these flags, more
 * priority is given to an MT-32 device, or a GM device respectively.
 *
 * @todo Rename MidiDriverFlags to MusicDriverFlags
 */
enum MidiDriverFlags {
	MDT_NONE        = 0,
	MDT_PCSPK       = 1 << 0,		// PC Speaker: Maps to MT_PCSPK and MT_PCJR
	MDT_CMS         = 1 << 1,		// Creative Music System / Gameblaster: Maps to MT_CMS
	MDT_PCJR        = 1 << 2,		// Tandy/PC Junior driver
	MDT_ADLIB       = 1 << 3,		// AdLib: Maps to MT_ADLIB
	MDT_C64         = 1 << 4,
	MDT_AMIGA       = 1 << 5,
	MDT_APPLEIIGS   = 1 << 6,
	MDT_TOWNS       = 1 << 7,		// FM-TOWNS: Maps to MT_TOWNS
	MDT_PC98        = 1 << 8,		// PC-98: Maps to MT_PC98
	MDT_SEGACD		= 1 << 9,
	MDT_MIDI        = 1 << 10,		// Real MIDI
	MDT_PREFER_MT32 = 1 << 11,		// MT-32 output is preferred
	MDT_PREFER_GM   = 1 << 12,		// GM output is preferred
	MDT_PREFER_FLUID= 1 << 13		// FluidSynth driver is preferred
};

/**
 * TODO: Document this, give it a better name.
 */
class MidiDriver_BASE {
public:
	static const uint8 MIDI_CHANNEL_COUNT = 16;
	static const uint8 MIDI_RHYTHM_CHANNEL = 9;

	static const byte MIDI_COMMAND_NOTE_OFF = 0x80;
	static const byte MIDI_COMMAND_NOTE_ON = 0x90;
	static const byte MIDI_COMMAND_POLYPHONIC_AFTERTOUCH = 0xA0;
	static const byte MIDI_COMMAND_CONTROL_CHANGE = 0xB0;
	static const byte MIDI_COMMAND_PROGRAM_CHANGE = 0xC0;
	static const byte MIDI_COMMAND_CHANNEL_AFTERTOUCH = 0xD0;
	static const byte MIDI_COMMAND_PITCH_BEND = 0xE0;
	static const byte MIDI_COMMAND_SYSTEM = 0xF0;

	static const byte MIDI_CONTROLLER_BANK_SELECT_MSB = 0x00;
	static const byte MIDI_CONTROLLER_MODULATION = 0x01;
	static const byte MIDI_CONTROLLER_DATA_ENTRY_MSB = 0x06;
	static const byte MIDI_CONTROLLER_VOLUME = 0x07;
	static const byte MIDI_CONTROLLER_PANNING = 0x0A;
	static const byte MIDI_CONTROLLER_EXPRESSION = 0x0B;
	static const byte MIDI_CONTROLLER_BANK_SELECT_LSB = 0x20;
	static const byte MIDI_CONTROLLER_DATA_ENTRY_LSB = 0x26;
	static const byte MIDI_CONTROLLER_SUSTAIN = 0x40;
	static const byte MIDI_CONTROLLER_REVERB = 0x5B;
	static const byte MIDI_CONTROLLER_CHORUS = 0x5D;
	static const byte MIDI_CONTROLLER_RPN_LSB = 0x64;
	static const byte MIDI_CONTROLLER_RPN_MSB = 0x65;
	static const byte MIDI_CONTROLLER_RESET_ALL_CONTROLLERS = 0x79;
	static const byte MIDI_CONTROLLER_ALL_NOTES_OFF = 0x7B;
	static const byte MIDI_CONTROLLER_OMNI_ON = 0x7C;
	static const byte MIDI_CONTROLLER_OMNI_OFF = 0x7D;
	static const byte MIDI_CONTROLLER_MONO_ON = 0x7E;
	static const byte MIDI_CONTROLLER_POLY_ON = 0x7F;

	static const byte MIDI_RPN_PITCH_BEND_SENSITIVITY_MSB = 0x00;
	static const byte MIDI_RPN_PITCH_BEND_SENSITIVITY_LSB = 0x00;
	static const byte MIDI_RPN_NULL = 0x7F;

	static const uint16 MIDI_PITCH_BEND_DEFAULT = 0x2000;

	MidiDriver_BASE();

	virtual ~MidiDriver_BASE();

	/**
	 * Output a packed midi command to the midi stream.
	 * The 'lowest' byte (i.e. b & 0xFF) is the status
	 * code, then come (if used) the first and second
	 * opcode.
	 */
	virtual void send(uint32 b) = 0;

	/**
	 * Send a MIDI command from a specific source. If the MIDI driver
	 * does not support multiple sources, the source parameter is
	 * ignored.
	 */
	virtual void send(int8 source, uint32 b) { send(b); }

	/**
	 * Output a midi command to the midi stream. Convenience wrapper
	 * around the usual 'packed' send method.
	 *
	 * Do NOT use this for sysEx transmission; instead, use the sysEx()
	 * method below.
	 */
	void send(byte status, byte firstOp, byte secondOp);
	
	/**
	 * Send a MIDI command from a specific source. If the MIDI driver
	 * does not support multiple sources, the source parameter is
	 * ignored.
	 */
	void send(int8 source, byte status, byte firstOp, byte secondOp);

	/**
	 * Transmit a SysEx to the MIDI device.
	 *
	 * The given msg MUST NOT contain the usual SysEx frame, i.e.
	 * do NOT include the leading 0xF0 and the trailing 0xF7.
	 *
	 * Furthermore, the maximal supported length of a SysEx
	 * is 264 bytes. Passing longer buffers can lead to
	 * undefined behavior (most likely, a crash).
	 */
	virtual void sysEx(const byte *msg, uint16 length) { }

	/**
	 * Transmit a SysEx to the MIDI device and return the necessary
	 * delay until the next SysEx event in milliseconds.
	 *
	 * This can be used to implement an alternate delay method than the
	 * OSystem::delayMillis function used by most sysEx implementations.
	 * Note that not every driver needs a delay, or supports this method.
	 * In this case, 0 is returned and the driver itself will do a delay 
	 * if necessary.
	 *
	 * For information on the SysEx data requirements, see the sysEx method.
	 */
	virtual uint16 sysExNoDelay(const byte *msg, uint16 length) { sysEx(msg, length); return 0; }

	// TODO: Document this.
	virtual void metaEvent(byte type, byte *data, uint16 length) { }

	/**
	 * Send a meta event from a specific source. If the MIDI driver
	 * does not support multiple sources, the source parameter is
	 * ignored.
	 */
	virtual void metaEvent(int8 source, byte type, byte *data, uint16 length) { metaEvent(type, data, length); }

	/**
	 * Stops all currently active notes. Specify stopSustainedNotes if
	 * the MIDI data makes use of the sustain controller to also stop
	 * sustained notes.
	 *
	 * Usually, the MIDI parser tracks active notes and terminates them
	 * when playback is stopped. This method should be used as a backup
	 * to silence the MIDI output in case the MIDI parser makes a
	 * mistake when tracking acive notes. It can also be used when
	 * quitting or pausing a game.
	 *
	 * By default, this method sends an All Notes Off message and, if
	 * stopSustainedNotes is true, a Sustain off message on all MIDI
	 * channels. Driver implementations can override this if they want
	 * to implement this functionality in a different way.
	 */
	virtual void stopAllNotes(bool stopSustainedNotes = false);

	/**
	 * A driver implementation might need time to prepare playback of
	 * a track. Use this function to check if the driver is ready to
	 * receive MIDI events.
	 */
	virtual bool isReady() { return true; }

protected:

	/**
	 * Enables midi dumping to a 'dump.mid' file and to debug messages on screen
	 * It's set by '--dump-midi' command line parameter
	 */
	bool _midiDumpEnable;

	/** Used for MIDI dumping delta calculation */
	uint32 _prevMillis;

	/** Stores all MIDI events, will be written to disk after an engine quits */
	Common::Array<byte> _midiDumpCache;

	/** Initialize midi dumping mechanism, called only if enabled */
	void midiDumpInit();

	/** Handles MIDI file variable length dumping */
	int midiDumpVarLength(const uint32 &delta);

	/** Handles MIDI file time delta dumping */
	void midiDumpDelta();

	/** Performs dumping of MIDI commands, called only if enabled */
	void midiDumpDo(uint32 b);

	/** Performs dumping of MIDI SysEx commands, called only if enabled */
	void midiDumpSysEx(const byte *msg, uint16 length);

	/** Writes the captured MIDI events to disk, called only if enabled */
	void midiDumpFinish();

};

/**
 * Abstract MIDI Driver Class
 *
 * @todo Rename MidiDriver to MusicDriver
 */
class MidiDriver : public MidiDriver_BASE {
public:
	/**
	 * The device handle.
	 *
	 * The value 0 is reserved for an invalid device for now.
	 * TODO: Maybe we should use -1 (i.e. 0xFFFFFFFF) as
	 * invalid device?
	 */
	typedef uint32 DeviceHandle;

	enum DeviceStringType {
		kDriverName,
		kDriverId,
		kDeviceName,
		kDeviceId
	};

	static Common::String musicType2GUIO(uint32 musicType);

	/** Create music driver matching the given device handle, or NULL if there is no match. */
	static MidiDriver *createMidi(DeviceHandle handle);

	/** Returns device handle based on the present devices and the flags parameter. */
	static DeviceHandle detectDevice(int flags);

	/** Find the music driver matching the given driver name/description. */
	static DeviceHandle getDeviceHandle(const Common::String &identifier);

	/** Check whether the device with the given handle is available. */
	static bool checkDevice(DeviceHandle handle);

	/** Get the music type matching the given device handle, or MT_AUTO if there is no match. */
	static MusicType getMusicType(DeviceHandle handle);

	/** Get the device description string matching the given device handle and the given type. */
	static Common::String getDeviceString(DeviceHandle handle, DeviceStringType type);

	/** Common operations to be done by all drivers on start of send */
	void midiDriverCommonSend(uint32 b);

	/** Common operations to be done by all drivers on start of sysEx */
	void midiDriverCommonSysEx(const byte *msg, uint16 length);

private:
	// If detectDevice() detects MT32 and we have a preferred MT32 device
	// we use this to force getMusicType() to return MT_MT32 so that we don't
	// have to rely on the 'True Roland MT-32' config manager setting (since nobody
	// would possibly think about activating 'True Roland MT-32' when he has set
	// 'Music Driver' to '<default>')
	static bool _forceTypeMT32;

public:
	virtual ~MidiDriver() { }

	static const byte _mt32ToGm[128];
	static const byte _gmToMt32[128];

	/**
	 * Error codes returned by open.
	 * Can be converted to a string with getErrorName().
	 */
	enum {
		MERR_CANNOT_CONNECT = 1,
//		MERR_STREAMING_NOT_AVAILABLE = 2,
		MERR_DEVICE_NOT_AVAILABLE = 3,
		MERR_ALREADY_OPEN = 4
	};

	enum {
//		PROP_TIMEDIV = 1,
		PROP_OLD_ADLIB = 2,
		PROP_CHANNEL_MASK = 3,
		// HACK: Not so nice, but our SCUMM AdLib code is in audio/
		PROP_SCUMM_OPL3 = 4,
		/**
		 * Set this to enable or disable scaling of the MIDI channel
		 * volume with the user volume settings (including setting it
		 * to 0 when Mute All is selected). This is currently
		 * implemented in the MT-32/GM drivers (regular and Miles AIL).
		 *
		 * Default is enabled for the regular driver, and disabled for
		 * the Miles AIL driver.
		 */
		PROP_USER_VOLUME_SCALING = 5,
		/**
		 * Set this property to indicate that the MIDI data used by the
		 * game has reversed stereo panning compared to its intended
		 * device. The MT-32 has reversed stereo panning compared to
		 * the MIDI specification and some game developers chose to
		 * stick to the MIDI specification.
		 *
		 * Do not confuse this with the _midiDeviceReversePanning flag,
		 * which indicates that the output MIDI device has reversed
		 * stereo panning compared to the intended MIDI device targeted
		 * by the MIDI data. This is set by the MT-32/GM driver when
		 * MT-32 data is played on a GM device or the other way around.
		 * Both flags can be set, which results in no change to the
		 * panning.
		 *
		 * Set this property before opening the driver, to make sure
		 * that the default panning is set correctly.
		 */
		 PROP_MIDI_DATA_REVERSE_PANNING = 6
	};

	/**
	 * Open the midi driver.
	 * @return 0 if successful, otherwise an error code.
	 */
	virtual int open() = 0;

	/**
	 * Check whether the midi driver has already been opened.
	 */
	virtual bool isOpen() const = 0;

	/** Close the midi driver. */
	virtual void close() = 0;

	/** Get or set a property. */
	virtual uint32 property(int prop, uint32 param) { return 0; }

	/** Retrieve a string representation of an error code. */
	static const char *getErrorName(int error_code);

	// HIGH-LEVEL SEMANTIC METHODS
	virtual void setPitchBendRange(byte channel, uint range) {
		send(MIDI_COMMAND_CONTROL_CHANGE | channel, MIDI_CONTROLLER_RPN_MSB, MIDI_RPN_PITCH_BEND_SENSITIVITY_MSB);
		send(MIDI_COMMAND_CONTROL_CHANGE | channel, MIDI_CONTROLLER_RPN_LSB, MIDI_RPN_PITCH_BEND_SENSITIVITY_LSB);
		send(MIDI_COMMAND_CONTROL_CHANGE | channel, MIDI_CONTROLLER_DATA_ENTRY_MSB, range); // Semi-tones
		send(MIDI_COMMAND_CONTROL_CHANGE | channel, MIDI_CONTROLLER_DATA_ENTRY_LSB, 0); // Cents
		send(MIDI_COMMAND_CONTROL_CHANGE | channel, MIDI_CONTROLLER_RPN_MSB, MIDI_RPN_NULL);
		send(MIDI_COMMAND_CONTROL_CHANGE | channel, MIDI_CONTROLLER_RPN_LSB, MIDI_RPN_NULL);
	}

	/**
	 * Send a Roland MT-32 reset sysEx to the midi device.
	 */
	void sendMT32Reset();

	/**
	 * Send a General MIDI reset sysEx to the midi device.
	 */
	void sendGMReset();

	virtual void sysEx_customInstrument(byte channel, uint32 type, const byte *instr) { }

	// Timing functions - MidiDriver now operates timers
	virtual void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) = 0;

	/** The time in microseconds between invocations of the timer callback. */
	virtual uint32 getBaseTempo() = 0;

	// Channel allocation functions
	virtual MidiChannel *allocateChannel() = 0;
	virtual MidiChannel *getPercussionChannel() = 0;

	// Allow an engine to supply its own soundFont data. This stream will be destroyed after use.
	virtual void setEngineSoundFont(Common::SeekableReadStream *soundFontData) { }

	// Does this driver accept soundFont data?
	virtual bool acceptsSoundFontData() { return false; }
};

class MidiChannel {
public:
	virtual ~MidiChannel() {}

	virtual MidiDriver *device() = 0;
	virtual byte getNumber() = 0;
	virtual void release() = 0;

	virtual void send(uint32 b) = 0; // 4-bit channel portion is ignored

	// Regular messages
	virtual void noteOff(byte note) = 0;
	virtual void noteOn(byte note, byte velocity) = 0;
	virtual void programChange(byte program) = 0;
	virtual void pitchBend(int16 bend) = 0; // -0x2000 to +0x1FFF

	// Control Change messages
	virtual void controlChange(byte control, byte value) = 0;
	virtual void modulationWheel(byte value) { controlChange(MidiDriver::MIDI_CONTROLLER_MODULATION, value); }
	virtual void volume(byte value) { controlChange(MidiDriver::MIDI_CONTROLLER_VOLUME, value); }
	virtual void panPosition(byte value) { controlChange(MidiDriver::MIDI_CONTROLLER_PANNING, value); }
	virtual void pitchBendFactor(byte value) = 0;
	virtual void transpose(int8 value) {}
	virtual void detune(byte value) { controlChange(17, value); }
	virtual void priority(byte value) { }
	virtual void sustain(bool value) { controlChange(MidiDriver::MIDI_CONTROLLER_SUSTAIN, value ? 1 : 0); }
	virtual void effectLevel(byte value) { controlChange(MidiDriver::MIDI_CONTROLLER_REVERB, value); }
	virtual void chorusLevel(byte value) { controlChange(MidiDriver::MIDI_CONTROLLER_CHORUS, value); }
	virtual void allNotesOff() { controlChange(MidiDriver::MIDI_CONTROLLER_ALL_NOTES_OFF, 0); }

	// SysEx messages
	virtual void sysEx_customInstrument(uint32 type, const byte *instr) = 0;
};
/** @} */
#endif
