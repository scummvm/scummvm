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

#ifndef AUDIO_CASIO_H
#define AUDIO_CASIO_H

#include "audio/mididrv.h"
#include "audio/mididrv_ms.h"

/**
 * MIDI driver implementation for the Casio MT-540, CT-640 and CSM-1 devices.
 *
 * This driver provides source volume and user volume scaling, as well as
 * fades (due to device limitations these are applied to note velocity instead
 * of the volume controller). It also provides instrument mapping between the
 * MT-540 and CT-640/CSM-1 instrument map and can map rhythm notes from the 
 * input MIDI data by specifying a remapping.
 *
 * TODO This driver does not provide a full multisource functionality
 * implementation because this was not needed for the game for which it was
 * added (Elvira). It assumes only 1 source sends MIDI data at the same time
 * and this source has access to all MIDI channels.
 *
 * Some details of these Casio devices:
 * - They seem to support only note on, note off and program change MIDI
 *   events. Because they do not support the volume controller, volume is
 *   applied to note velocity (and I'm not sure if they support even that...).
 *   All Notes Off is performed by keeping track of active notes and sending
 *   note off events for all active notes.
 * - They only use MIDI channels 0-3. The MT-32 and GM devices have channel 9
 *   as the fixed rhythm channel. The Casio devices can switch any used channel
 *   to a rhythm channel by setting a specific instrument.
 * - They have only 30 instruments, 3 of which are rhythm or SFX banks.
 * - All devices seem to have the same capabilities, but the instrument
 *   numbering is different between the MT-540 on the one hand and the CT-640
 *   and CSM-1 on the other.
 */
class MidiDriver_Casio : public MidiDriver_Multisource {
protected:
	// Tracks a note currently playing on the device.
	struct ActiveNote {
		// The source that played the note (0x7F if no note is tracked).
		int8 source;
		// The output MIDI channel on which the note is playing
		// (0xFF if no note is tracked).
		uint8 channel;
		// The MIDI note number of the playing note
		// (0xFF if no note is tracked).
		uint8 note;
		// True if this note is sustained (turned off but held due to the
		// sustain controller).
		bool sustained;

		ActiveNote();

		// Sets the struct to values indicating no note is currently tracked.
		void clear();
	};

public:
	// The maximum polyphony for each output channel.
	static const int CASIO_CHANNEL_POLYPHONY[4];

	// Array for remapping instrument numbers from CT-460/CSM-1 to MT-540.
	static const uint8 INSTRUMENT_REMAPPING_CT460_TO_MT540[30];
	// Array for remapping instrument numbers from MT-540 to CT-460/CSM-1.
	static const uint8 INSTRUMENT_REMAPPING_MT540_TO_CT460[30];

	// The instrument number used for rhythm sounds on the MT-540.
	static const uint8 RHYTHM_INSTRUMENT_MT540;
	// The instrument number used for rhythm sounds on the CT-460 and CSM-1.
	static const uint8 RHYTHM_INSTRUMENT_CT460;

	// The instrument number used for the bass instruments on the MT-540.
	static const uint8 BASS_INSTRUMENT_MT540;
	// The instrument number used for the bass instruments on the CT-460 and
	// CSM-1.
	static const uint8 BASS_INSTRUMENT_CT460;

	/**
	 * Constructs a new Casio MidiDriver instance.
	 * 
	 * @param midiType The type of MIDI data that will be sent to the driver
	 * (MT-540 or CT-460/CSM-1).
	 */
	MidiDriver_Casio(MusicType midiType);
	~MidiDriver_Casio();

	int open() override;
	/**
	 * Opens the driver wrapping the specified MidiDriver instance.
	 * 
	 * @param driver The driver that will receive MIDI events from this driver.
	 * @param deviceType The type of MIDI device that will receive MIDI events
	 * from this driver (MT-540 or CT-460/CSM-1).
	 * @return 0 if the driver was opened successfully; >0 if an error occurred.
	 */
	virtual int open(MidiDriver *driver, MusicType deviceType);
	void close() override;
	bool isOpen() const override;

	using MidiDriver_BASE::send;
	void send(int8 source, uint32 b) override;
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;

	void stopAllNotes(bool stopSustainedNotes = false) override;
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;
	uint32 getBaseTempo() override;

protected:
	/**
	 * Maps a data MIDI channel to an output MIDI channel for the specified
	 * source.
	 * TODO This driver has no default implementation for a channel allocation
	 * scheme. It assumes only one source is active at a time and has access to
	 * all output channels. The default implementation for this method just
	 * returns the data channel.
	 * 
	 * @param source The source for which the MIDI channel should be mapped.
	 * @param dataChannel The data channel that should be mapped.
	 * @return The output MIDI channel.
	 */
	virtual int8 mapSourceChannel(uint8 source, uint8 dataChannel);
	/**
	 * Processes a MIDI event.
	 * 
	 * @param source The source sending the MIDI event.
	 * @param b The MIDI event data.
	 * @param outputChannel The MIDI channel on which the event should be sent.
	 */
	virtual void processEvent(int8 source, uint32 b, uint8 outputChannel);
	/**
	 * Processes a MIDI note off event.
	 * 
	 * @param outputChannel The MIDI channel on which the event should be sent.
	 * @param command The MIDI command that triggered the note off event (other
	 * than note off (0x80) this can also be note on (0x90) with velocity 0).
	 * @param note The MIDI note that should be turned off.
	 * @param velocity The note off velocity.
	 * @param source The source sending the MIDI event.
	 */
	virtual void noteOff(byte outputChannel, byte command, byte note, byte velocity, int8 source);
	/**
	 * Processes a MIDI note on event.
	 * 
	 * @param outputChannel The MIDI channel on which the event should be sent.
	 * @param note The MIDI note that should be turned on.
	 * @param velocity The note velocity,
	 * @param source The source sending the MIDI event.
	 */
	virtual void noteOn(byte outputChannel, byte note, byte velocity, int8 source);
	/**
	 * Processes a MIDI program change event.
	 * 
	 * @param outputChannel The MIDI channel on which the event should be sent.
	 * @param patchId The instrument that should be set.
	 * @param source The source sending the MIDI event.
	 * @param applyRemapping True if the instrument remapping
	 * (_instrumentRemapping) should be applied.
	 */
	virtual void programChange(byte outputChannel, byte patchId, int8 source, bool applyRemapping = true);
	/**
	 * Processes a MIDI control change event.
	 *
	 * @param outputChannel The MIDI channel on which the event should be sent.
	 * @param controllerNumber The controller for which the value should be set.
	 * @param controllerValue The controller value that should be set.
	 * @param source The source sending the MIDI event.
	 */
	virtual void controlChange(byte outputChannel, byte controllerNumber, byte controllerValue, int8 source);

	/**
	 * Maps the specified note to a different note according to the rhythm note
	 * mapping. This mapping is only applied if the note is played on a rhythm
	 * channel.
	 * 
	 * @param outputChannel The MIDI channel on which the note is/will be
	 * active.
	 * @param note The note that should be mapped.
	 * @return The mapped note, or the specified note if it was not mapped.
	 */
	virtual int8 mapNote(byte outputChannel, byte note);
	/**
	 * Calculates the velocity for a note on event. This applies source volume
	 * and user volume settings to the specified velocity value.
	 * 
	 * @param source The source that sent the note on event.
	 * @param velocity The velocity specified in the note on event.
	 * @return The calculated velocity.
	 */
	virtual byte calculateVelocity(int8 source, byte velocity);
	/**
	 * Maps the specified instrument to the instrument value that should be
	 * sent to the MIDI device. This applies the current instrument remapping
	 * (if present and if applyRemapping is specified) and maps MT-540
	 * instruments to CT-460/CSM-1 instruments (or the other way around) if
	 * necessary.
	 * 
	 * @param program The instrument that should be mapped.
	 * @param applyRemapping True if the instrument remapping
	 * (_instrumentRemapping) should be applied.
	 * @return The mapped instrument, or the specified instrument if no mapping
	 * was necessary.
	 */
	virtual byte mapInstrument(byte program, bool applyRemapping = true);
	/**
	 * Returns whether the specified MIDI channel is a rhythm channel. On the
	 * Casio devices, the rhythm channel is not fixed but is created by setting
	 * a channel to a specific instrument (see the rhythm instrument constants).
	 * 
	 * @param outputChannel The channel that should be checked.
	 * @return True if the specified channel is a rhythm channel, false
	 * otherwise.
	 */
	virtual bool isRhythmChannel(uint8 outputChannel);
	// This implementation does nothing, because source volume is applied to
	// note velocity and cannot be applied immediately.
	void applySourceVolume(uint8 source) override;
	void stopAllNotes(uint8 source, uint8 channel) override;

	// The wrapped MIDI driver.
	MidiDriver *_driver;
	// The type of Casio device accessed by the wrapped driver: MT-540 or
	// CT-460/CSM-1.
	MusicType _deviceType;
	// The type of MIDI data supplied to the driver: MT-540 or CT-460/CSM-1.
	MusicType _midiType;
	// True if this MIDI driver has been opened.
	bool _isOpen;

	// The current instrument on each MIDI output channel. This is the
	// instrument number as specified in the program change events (before
	// remapping is applied).
	byte _instruments[4];
	// Indicates if each output channel is currently a rhythm channel (i.e. it
	// has the rhythm instrument set).
	bool _rhythmChannel[4];
	// Tracks the notes currently active on the MIDI device.
	ActiveNote _activeNotes[32];
	// Tracks the sustain controller status of the output channels.
	bool _sustain[4];

	// Optional remapping for rhythm notes. Should point to a 128 byte array
	// which maps the rhythm note numbers in the input MIDI data to the rhythm
	// note numbers used by the output device.
	byte *_rhythmNoteRemapping;

	// If true the driver will send note off events for notes which are not in
	// the active note registry. Typically this should be true to prevent
	// hanging notes in case there are more active notes than can be stored in
	// the active note registry. Can be set to false if these note offs are not
	// desirable, f.e. because the driver will be receiving note off events
	// without corresponding note on events.
	bool _sendUntrackedNoteOff;

	// Mutex for operations on active notes.
	Common::Mutex _mutex;

public:
	static void timerCallback(void *data);
};

#endif
