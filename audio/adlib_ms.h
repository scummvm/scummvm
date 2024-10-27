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

#ifndef AUDIO_ADLIB_MS_H
#define AUDIO_ADLIB_MS_H

#include "audio/mididrv_ms.h"
#include "audio/fmopl.h"

/**
 * Rhythm instrument types used by the OPL2 and OPL3 rhythm mode.
 */
enum OplInstrumentRhythmType {
	RHYTHM_TYPE_UNDEFINED,
	RHYTHM_TYPE_HI_HAT,
	RHYTHM_TYPE_CYMBAL,
	RHYTHM_TYPE_TOM_TOM,
	RHYTHM_TYPE_SNARE_DRUM,
	RHYTHM_TYPE_BASS_DRUM
};

/**
 * Data for one operator of an OPL instrument definition.
 */
struct OplInstrumentOperatorDefinition {
	/**
	 * 2x register: frequency multiplier, key scaling rate, envelope gain type,
	 * vibrato and modulation.
	 */
	uint8 freqMultMisc;
	/**
	 * 4x register: level and key scaling level.
	 */
	uint8 level;
	/**
	 * 6x register: decay and attack.
	 */
	uint8 decayAttack;
	/**
	 * 8x register: release and sustain.
	 */
	uint8 releaseSustain;
	/**
	 * Ex register: waveform select.
	 */
	uint8 waveformSelect;

	/**
	 * Check if this operator definition contains any data.
	 *
	 * @return True if this operator is empty; false otherwise.
	 */
	bool isEmpty();
};

/**
 * Instrument definition for an OPL2 or OPL3 chip. Contains the data for all
 * registers belonging to an OPL channel, except the Ax and Bx registers (these
 * determine the frequency and are derived from the note played).
 */
struct OplInstrumentDefinition {
	/**
	 * Indicates if this instrument uses 2 or 4 operators.
	 */
	bool fourOperator;

	/**
	 * Operator data. 2 operator instruments use operators 0 and 1 only.
	 */
	OplInstrumentOperatorDefinition operator0;
	OplInstrumentOperatorDefinition operator1;
	OplInstrumentOperatorDefinition operator2;
	OplInstrumentOperatorDefinition operator3;

	/**
	 * Cx register: connection and feedback.
	 * Note: panning is determined by a MIDI controller and not part of the
	 * instrument definition.
	 */
	uint8 connectionFeedback0;
	/**
	 * Second Cx register (used by 4 operator instruments).
	 */
	uint8 connectionFeedback1;

	/**
	 * Notes played on a MIDI rhythm channel indicate which rhythm instrument
	 * should be played, not which note should be played. This field indicates
	 * the pitch (MIDI note) which should be used to play this rhythm
	 * instrument. Not used for melodic instruments.
	 */
	uint8 rhythmNote;
	/**
	 * The type of OPL rhythm instrument that this definition should be used
	 * with. Type undefined indicates that this definition should not be used
	 * with rhythm mode.
	 */
	OplInstrumentRhythmType rhythmType;

	/**
	 * Check if this instrument definition contains any data.
	 *
	 * @return True if this instrument is empty; false otherwise.
	 */
	bool isEmpty();
	/**
	 * Returns the number of operators used by this instrument definition.
	 *
	 * @return The number of operators (2 or 4).
	 */
	uint8 getNumberOfOperators();
	/**
	 * Returns the definition data for the operator with the specified number.
	 * Specify 0 or 1 for 2 operator instruments or 0-3 for 4 operator
	 * instruments.
	 * 
	 * @param operatorNum The operator for which the data should be returned.
	 * @return Pointer to the definition data for the specified operator.
	 */
	OplInstrumentOperatorDefinition &getOperatorDefinition(uint8 operatorNum);
};

#include "common/pack-start.h" // START STRUCT PACKING

/**
 * Data for one operator of an OPL instrument definition in the AdLib BNK
 * format.
 */
struct AdLibBnkInstrumentOperatorDefinition {
	/**
	 * Individual fields for each setting in the 2x-8x registers.
	 * Note that waveform select is not part of the operator data in this
	 * format; it is included in the instrument data as a separate field.
	 */
	uint8 keyScalingLevel;
	uint8 frequencyMultiplier;
	uint8 feedback; // ignored for operator 1
	uint8 attack;
	uint8 sustain;
	uint8 envelopeGainType; // 0x00: not sustained, >= 0x01: sustained
	uint8 decay;
	uint8 release;
	uint8 level;
	uint8 amplitudeModulation; // 0x00: off, >= 0x01: on
	uint8 vibrato; // 0x00: off, >= 0x01: on
	uint8 keyScalingRate; // 0x00: low, >= 0x01: high
	uint8 connection; // 0x00: additive, >= 0x01: FM; ignored for operator 1

	/**
	 * Copies the data in this AdLib BNK operator definition to the specified
	 * OplInstrumentOperatorDefinition struct.
	 * 
	 * @param operatorDef The operator definition to which the data should be
	 * copied.
	 * @param waveformSelect The value of the waveform select parameter for
	 * this operator.
	 */
	void toOplInstrumentOperatorDefinition(OplInstrumentOperatorDefinition &operatorDef, uint8 waveformSelect);
} PACKED_STRUCT;

/**
 * Instrument definition for an OPL2 chip in the format used by the AdLib BNK
 * instrument bank file format.
 */
struct AdLibBnkInstrumentDefinition {
	/**
	 * The type of instrument (0x00: melodic, 0x01: rhythm).
	 */
	uint8 instrumentType;
	/**
	 * TODO Unclear what this represents; might be the same as rhythmNote.
	 */
	uint8 rhythmVoiceNumber;

	/**
	 * Operator data.
	 */
	AdLibBnkInstrumentOperatorDefinition operator0;
	AdLibBnkInstrumentOperatorDefinition operator1;

	/**
	 * Waveform select parameter for each operator.
	 */
	uint8 waveformSelect0;
	uint8 waveformSelect1;

	/**
	 * Copies the data in this AdLib BNK instrument definition to the specified
	 * OplInstrumentDefinition struct.
	 * 
	 * @param instrumentDef The instrument definition to which the data should
	 * be copied.
	 */
	void toOplInstrumentDefinition(OplInstrumentDefinition &instrumentDef);
} PACKED_STRUCT;

/**
 * Instrument definition for an OPL2 chip in the format used by the IBK
 * instrument bank file format. This format is also used by the SBI and CMF
 * file formats.
 */
struct AdLibIbkInstrumentDefinition {
	uint8 o0FreqMultMisc;
	uint8 o1FreqMultMisc;
	uint8 o0Level;
	uint8 o1Level;
	uint8 o0DecayAttack;
	uint8 o1DecayAttack;
	uint8 o0ReleaseSustain;
	uint8 o1ReleaseSustain;
	uint8 o0WaveformSelect;
	uint8 o1WaveformSelect;
	uint8 connectionFeedback;
	/**
	 * Rhythm note type. 0: melodic, 6: bass drum, 7: snare drum, 8: tom tom, 9: cymbal, 10: hi hat
	 */
	uint8 rhythmType;
	/**
	 * Number of semitones to transpose a note using this instrument.
	 */
	int8 transpose;
	uint8 rhythmNote;
	uint8 padding1;
	uint8 padding2;

	/**
	 * Copies the data in this AdLib BNK instrument definition to the specified
	 * OplInstrumentDefinition struct.
	 *
	 * @param instrumentDef The instrument definition to which the data should
	 * be copied.
	 */
	void toOplInstrumentDefinition(OplInstrumentDefinition &instrumentDef);
} PACKED_STRUCT;

#include "common/pack-end.h" // END STRUCT PACKING

/**
 * MIDI driver for AdLib / OPL2 and OPL3 emulators and devices with support for
 * multiple simultaneous sources of MIDI data.
 *
 * This driver converts MIDI events to OPL chip register writes. When opened it
 * will initialize an OPL emulator or device using the specified OPL type. It
 * tracks the MIDI state of each source separately to avoid conflicts.
 * The default behavior of the driver plays General MIDI data with the same
 * output as the SoudBlaster 16 Windows 95 driver. It can be subclassed and
 * customized to match the specific behavior of a game.
 *
 * Customization
 *
 * Depending on the platform and the type of music data the game uses, you can
 * customize the driver to match this behavior:
 * - Windows: If the game uses the standard Windows APIs to play General MIDI
 *   data, the default behavior of the driver should give identical output.
 * - DOS, General MIDI: The default behavior of the driver should give you a
 *   decent starting point, but because there is no standard way to handle GM
 *   on OPL chips in DOS, it is probably not accurate. The instruments used by
 *   the game can be set in the _instrumentBank and _rhythmBank fields.
 *   You can subclass the driver to override more behavior, such as the
 *   calculateFrequency, calculatePitchBend, calculateUnscaledVolume and
 *   allocateOplChannel functions.
 * - DOS, other type of MIDI: Additionally, you will need to override the
 *   functions that handle the various MIDI events and controllers when they do
 *   not match the General MIDI standard. You can override determineInstrument
 *   if the game uses some other way than instrument banks to set instruments.
 * - DOS, does not use MIDI: Write new code to access the OPL registers
 *   directly instead of using this driver.
 * 
 * TODO Dual OPL2 and 4 operator instrument support is unfinished.
 */
class MidiDriver_ADLIB_Multisource : public MidiDriver_Multisource {
public:
	/**
	 * The available accuracy modes for frequency and volume calculation.
	 */
	enum AccuracyMode {
		/**
		 * Accurate to the behavior of the Windows 95 SB16 driver.
		 */
		ACCURACY_MODE_SB16_WIN95,
		/**
		 * Accurate to the General MIDI and MIDI specifications.
		 */
		ACCURACY_MODE_GM
	};

	/**
	 * The available modes for OPL channel allocation.
	 */
	enum ChannelAllocationMode {
		/**
		 * Dynamic channel allocation (new OPL channel allocated to each note
		 * played).
		 */
		ALLOCATION_MODE_DYNAMIC,
		/**
		 * Static channel allocation (fixed OPL channel allocated to each MIDI
		 * channel).
		 */
		ALLOCATION_MODE_STATIC
	};

	/**
	 * The available modes for writing the instrument definition to a channel.
	 */
	enum InstrumentWriteMode {
		/**
		 * Will write the instrument definition before each note on event.
		 * Works with both dynamic and static channel allocation modes, but
		 * is less efficient and resets all parameters of the instrument when
		 * a note is played.
		 */
		INSTRUMENT_WRITE_MODE_NOTE_ON,
		/**
		 * Will write the instrument definition after a program change event.
		 * This will only work with a static channel allocation mode. It will
		 * write the instrument only once for many notes and allows parameters
		 * of the instrument to be changed for the following notes.
		 */
		INSTRUMENT_WRITE_MODE_PROGRAM_CHANGE
	};

	/**
	 * The available modes for the OPL note select setting.
	 */
	enum NoteSelectMode {
		NOTE_SELECT_MODE_0,
		NOTE_SELECT_MODE_1
	};

	/**
	 * The available modes for the OPL modulation depth setting.
	 */
	enum ModulationDepth {
		/**
		 * Low modulation depth (1 dB).
		 */
		MODULATION_DEPTH_LOW,
		/**
		 * High modulation depth (4.8 dB).
		 */
		MODULATION_DEPTH_HIGH
	};

	/**
	 * The available modes for the OPL vibrato depth setting.
	 */
	enum VibratoDepth {
		/**
		 * Low vibrato depth (7 %).
		 */
		VIBRATO_DEPTH_LOW,
		/**
		 * High vibrato depth (14 %).
		 */
		VIBRATO_DEPTH_HIGH
	};

	/**
	 * The number of available channels on each OPL chip.
	 */
	static const uint8 OPL2_NUM_CHANNELS = 9;
	static const uint8 OPL3_NUM_CHANNELS = 18;
	/**
	 * The melodic channel numbers available on an OPL2 chip with rhythm mode
	 * disabled.
	 */
	static const uint8 MELODIC_CHANNELS_OPL2[9];
	/**
	 * The melodic channel numbers available on an OPL2 chip with rhythm mode
	 * enabled.
	 */
	static const uint8 MELODIC_CHANNELS_OPL2_RHYTHM[6];
	/**
	 * The melodic channel numbers available on an OPL3 chip with rhythm mode
	 * disabled.
	 */
	static const uint8 MELODIC_CHANNELS_OPL3[18];
	/**
	 * The melodic channel numbers available on an OPL3 chip with rhythm mode
	 * enabled.
	 */
	static const uint8 MELODIC_CHANNELS_OPL3_RHYTHM[15];
	/**
	 * The number of rhythm instruments available in OPL rhythm mode.
	 */
	static const uint8 OPL_NUM_RHYTHM_INSTRUMENTS = 5;
	/**
	 * The OPL channels used by the rhythm instruments, in order:
	 * hi-hat, cymbal, tom tom, snare drum, bass drum.
	 */
	static const uint8 OPL_RHYTHM_INSTRUMENT_CHANNELS[OPL_NUM_RHYTHM_INSTRUMENTS];

	/**
	 * OPL test and timer registers.
	 */
	static const uint8 OPL_REGISTER_TEST = 0x01;
	static const uint8 OPL_REGISTER_TIMER1 = 0x02;
	static const uint8 OPL_REGISTER_TIMER2 = 0x03;
	static const uint8 OPL_REGISTER_TIMERCONTROL = 0x04;

	/**
	 * OPL global setting registers.
	 */
	static const uint8 OPL_REGISTER_NOTESELECT_CSM = 0x08;
	static const uint8 OPL_REGISTER_RHYTHM = 0xBD;

	/**
	 * OPL operator base registers.
	 */
	static const uint8 OPL_REGISTER_BASE_FREQMULT_MISC = 0x20;
	static const uint8 OPL_REGISTER_BASE_LEVEL = 0x40;
	static const uint8 OPL_REGISTER_BASE_DECAY_ATTACK = 0x60;
	static const uint8 OPL_REGISTER_BASE_RELEASE_SUSTAIN = 0x80;
	static const uint8 OPL_REGISTER_BASE_WAVEFORMSELECT = 0xE0;

	/**
	 * OPL channel base registers.
	 */
	static const uint8 OPL_REGISTER_BASE_FNUMLOW = 0xA0;
	static const uint8 OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON = 0xB0;
	static const uint8 OPL_REGISTER_BASE_CONNECTION_FEEDBACK_PANNING = 0xC0;

	/**
	 * OPL3-specific global setting registers.
	 */
	static const uint16 OPL3_REGISTER_CONNECTIONSELECT = 0x104;
	static const uint16 OPL3_REGISTER_NEW = 0x105;

	/**
	 * Offset to the second register set (for dual OPL2 and OPL3).
	 */
	static const uint16 OPL_REGISTER_SET_2_OFFSET = 0x100;

	/**
	 * Offsets for the rhythm mode instrument registers.
	 */
	static const uint8 OPL_REGISTER_RHYTHM_OFFSETS[];

	/**
	 * Bitmasks for various parameters in the OPL registers.
	 */
	static const uint8 OPL_MASK_LEVEL = 0x3F;
	static const uint8 OPL_MASK_FNUMHIGH_BLOCK = 0x1F;
	static const uint8 OPL_MASK_KEYON = 0x20;
	static const uint8 OPL_MASK_PANNING = 0x30;

	/**
	 * Settings for the panning bits in the OPL Cx registers.
	 */
	static const uint8 OPL_PANNING_CENTER = 0x30;
	static const uint8 OPL_PANNING_LEFT = 0x10;
	static const uint8 OPL_PANNING_RIGHT = 0x20;

	/**
	 * The default melodic instrument definitions.
	 */
	static OplInstrumentDefinition OPL_INSTRUMENT_BANK[];
	/**
	 * The default rhythm instrument definitions.
	 */
	static OplInstrumentDefinition OPL_RHYTHM_BANK[];

protected:
	/**
	 * Default setting for OPL channel volume (level).
	 */
	static const uint8 OPL_LEVEL_DEFAULT = 0x3F;

	/**
	 * The lowest MIDI panning controller value interpreted as left panning.
	 */
	static const uint8 OPL_MIDI_PANNING_LEFT_LIMIT = 0x2F;
	/**
	 * The highest MIDI panning controller value interpreted as right panning.
	 */
	static const uint8 OPL_MIDI_PANNING_RIGHT_LIMIT = 0x51;

	/**
	 * OPL frequency (F-num) value for each octave semitone. The values assume
	 * octave 5.
	 */
	static const uint16 OPL_NOTE_FREQUENCIES[];
	/**
	 * OPL volume lookup array for a MIDI volume value shifted from 7 to 5 bits.
	 */
	static const uint8 OPL_VOLUME_LOOKUP[];

	/**
	 * Contains the current controller settings for a MIDI channel.
	 */
	struct MidiChannelControlData {
		uint8 program;
		uint8 channelPressure;
		uint16 pitchBend; // 14 bit value; 0x2000 is neutral

		uint8 modulation;
		uint8 volume;
		uint8 panning; // 0x40 is center
		uint8 expression;
		bool sustain;
		uint16 rpn; // Two 7 bit values stored in 8 bits each

		uint8 pitchBendSensitivity; // Semitones
		uint8 pitchBendSensitivityCents;
		uint16 masterTuningFine; // 14 bit value; 0x2000 is neutral
		uint8 masterTuningCoarse; // Semitones; 0x40 is neutral

		MidiChannelControlData();

		/**
		 * Initializes the controller settings to default values.
		 */
		void init();
	};

	/**
	 * Contains information on the currently active note on an OPL channel.
	 */
	struct ActiveNote {
		/**
		 * True if a note is currently playing (including if it is sustained,
		 * but not if it is in the "release" phase).
		 */
		bool noteActive;
		/**
		 * True if the currently playing note is sustained, i.e. note has been
		 * turned off but is kept active due to the sustain controller.
		 */
		bool noteSustained;

		/**
		 * The MIDI note value as it appeared in the note on event.
		 */
		uint8 note;
		/**
		 * The MIDI velocity value of the note on event.
		 */
		uint8 velocity;
		/**
		 * The MIDI channel that played the current/last note (0xFF if no note
		 * has been played since initialization).
		 */
		uint8 channel;
		/**
		 * The source that played the current/last note (0xFF if no note has
		 * been played since initialization).
		 */
		uint8 source;

		/**
		 * The MIDI note value that is actually played. This is the same as the
		 * note field for melodic instruments, but on the MIDI rhythm channel
		 * the note indicates which rhythm instrument should be played instead
		 * of the pitch. In that case this field is different
		 * (@see determineInstrument).
		 */
		uint8 oplNote;
		/**
		 * The OPL frequency (F-num) and octave (block) (in Ax (low byte) and
		 * Bx (high byte) register format) that was calculated to play the MIDI
		 * note.
		 */
		uint16 oplFrequency;
		/**
		 * The value of the note counter when a note was last turned on or off
		 * on this OPL channel.
		 */
		uint32 noteCounterValue;

		/**
		 * A unique identifier of the instrument that is used to play the note.
		 * In the default implementation this is the MIDI program number for
		 * melodic instruments and the rhythm channel note number + 0x80 for
		 * rhythm instruments (@see determineInstrument).
		 */
		uint8 instrumentId;
		/**
		 * Pointer to the instrument definition used to play the note.
		 */
		OplInstrumentDefinition *instrumentDef;

		/**
		 * True if this OPL channel has been allocated to a MIDI channel.
		 * Note that in the default driver implementation only the static
		 * channel allocation algorithm uses this field.
		 */
		bool channelAllocated;

		ActiveNote();

		/**
		 * Initializes the active note data to default values.
		 */
		void init();
	};

	/**
	 * OPL instrument data for playing a note.
	 */
	struct InstrumentInfo {
		/**
		 * MIDI note value to use for playing this instrument
		 * (@see ActiveNote.oplNote).
		 */
		uint8 oplNote;
		/**
		 * Pointer to the instrument definition.
		 */
		OplInstrumentDefinition *instrumentDef;
		/**
		 * Unique identifer for this instrument (@see ActiveNote.instrumentId).
		 */
		uint8 instrumentId;
	};

public:
	/**
	 * Checks if the specified type of OPL chip is supported by the OPL
	 * emulator or hardware that is used.
	 * 
	 * @param oplType The type of OPL chip that should be detected.
	 * @return True if the specified type of OPL chip is supported by the OPL
	 * emulator/hardware; false otherwise.
	 */
	static bool detectOplType(OPL::Config::OplType oplType);

	/**
	 * Constructs a new AdLib multisource MIDI driver using the specified type
	 * of OPL chip.
	 * 
	 * @param oplType The type of OPL chip that should be used.
	 * @param timerFrequency The number of timer callbacks per second that
	 * should be generated.
	 */
	MidiDriver_ADLIB_Multisource(OPL::Config::OplType oplType, int timerFrequency = OPL::OPL::kDefaultCallbackFrequency);
	~MidiDriver_ADLIB_Multisource();

	/**
	 * Prepares the driver for processing MIDI data and initializes the OPL
	 * emulator or hardware.
	 * 
	 * @return 0 if the driver was opened successfully; a MidiDriver error code
	 * otherwise.
	 */
	int open() override;
	bool isOpen() const override;
	void close() override;
	uint32 property(int prop, uint32 param) override;
	uint32 getBaseTempo() override;
	/**
	 * This driver does not use MidiChannel objects, so this function returns nullptr.
	 * 
	 * @return nullptr
	 */
	MidiChannel *allocateChannel() override;
	/**
	 * This driver does not use MidiChannel objects, so this function returns nullptr.
	 * 
	 * @return nullptr
	 */
	MidiChannel *getPercussionChannel() override;

	using MidiDriver_Multisource::send;
	void send(int8 source, uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;
	void metaEvent(int8 source, byte type, byte *data, uint16 length) override;
	void stopAllNotes(bool stopSustainedNotes = false) override;

	void stopAllNotes(uint8 source, uint8 channel) override;
	void deinitSource(uint8 source) override;

protected:
	void applySourceVolume(uint8 source) override;

	/**
	 * Initializes the OPL registers to their default values.
	 */
	virtual void initOpl();

	/**
	 * Processes a MIDI note off event.
	 * 
	 * @param channel The MIDI channel on which the note is active.
	 * @param note The MIDI note that should be turned off.
	 * @param velocity The release velocity (not implemented).
	 * @param source The source sending the note off event.
	 */
	virtual void noteOff(uint8 channel, uint8 note, uint8 velocity, uint8 source);
	/**
	 * Processes a MIDI note on event.
	 * 
	 * @param channel The MIDI channel on which the note is played.
	 * @param note The MIDI note that should be turned on.
	 * @param velocity The MIDI velocity of the played note.
	 * @param source The source sending the note on event.
	 */
	virtual void noteOn(uint8 channel, uint8 note, uint8 velocity, uint8 source);
	/**
	 * Processes a MIDI polyphonic aftertouch event.
	 * Note: this event has no default implementation because it is not
	 * implemented in the Win95 SB16 driver.
	 * 
	 * @param channel The MIDI channel on which the event is sent.
	 * @param note The MIDI note on which aftertouch should be applied.
	 * @param pressure The aftertouch amount that should be applied.
	 * @param source The source sending the aftertouch event.
	 */
	virtual void polyAftertouch(uint8 channel, uint8 note, uint8 pressure, uint8 source);
	/**
	 * Processes a MIDI control change event. The individual controllers are
	 * handled by separate functions (@see modulation etc.).
	 * 
	 * @param channel The MIDI channel on which the event is sent.
	 * @param controller The MIDI controller whose value should be changed.
	 * @param value The value that should be applied to the controller.
	 * @param source The source sending the conrol change event.
	 */
	virtual void controlChange(uint8 channel, uint8 controller, uint8 value, uint8 source);
	/**
	 * Processes a MIDI program (instrument) change event.
	 * 
	 * @param channel The MIDI channel on which the instrument should be set.
	 * @param program The instrument that should be set on the channel.
	 * @param source The source sending the program change event.
	 */
	virtual void programChange(uint8 channel, uint8 program, uint8 source);
	/**
	 * Processes a MIDI channel aftertouch event.
	 * Note: this event has no default implementation because it is not
	 * implemented in the Win95 SB16 driver.
	 * 
	 * @param channel The MIDI channel on which aftertouch should be applied.
	 * @param pressure The aftertouch amount that should be applied.
	 * @param source The source sending the aftertouch event.
	 */
	virtual void channelAftertouch(uint8 channel, uint8 pressure, uint8 source);
	/**
	 * Processes a MIDI pitch bend event.
	 * Note that MIDI pitch bend is a 14 bit value sent as 2 7 bit values, with
	 * the LSB sent first.
	 * 
	 * @param channel The MIDI channel on which pitch bend should be applied.
	 * @param pitchBendLsb The LSB of the pitch bend value.
	 * @param pitchBendMsb The MSB of the pitch bend value.
	 * @param source The source sending the pitch bend event.
	 */
	virtual void pitchBend(uint8 channel, uint8 pitchBendLsb, uint8 pitchBendMsb, uint8 source);

	/**
	 * Processes a MIDI modulation control change event.
	 * Note: this event has no default implementation because it is not
	 * implemented in the Win95 SB16 driver.
	 * 
	 * @param channel The MIDI channel on which modulation should be applied.
	 * @param modulation The modulation amount that should be applied.
	 * @param source The source sending the control change event.
	 */
	virtual void modulation(uint8 channel, uint8 modulation, uint8 source);
	/**
	 * Processes a MIDI data entry control change event. This sets the MSB
	 * and/or LSB of the currently selected registered parameter number.
	 * Note that a MIDI data entry event contains either the MSB or LSB;
	 * specify 0xFF for the other data byte to leave it unchanged.
	 * RPNs pitch bend sensitivity, master tuning fine and coarse are supported
	 * in accuracy mode GM only.
	 * 
	 * @param channel The MIDI channel on which the RPN data byte should be set.
	 * @param dataMsb The MSB of the RPN data value; 0xFF to not set the MSB.
	 * @param dataLsb The LSB of the RPN data value; 0xFF to not set the LSB.
	 * @param source The source sending the control change event.
	 */
	virtual void dataEntry(uint8 channel, uint8 dataMsb, uint8 dataLsb, uint8 source);
	/**
	 * Process a MIDI volume control change event.
	 * 
	 * @param channel The MIDI channel on which volume should be set.
	 * @param volume The volume level that should be set.
	 * @param source The source sending the control change event.
	 */
	virtual void volume(uint8 channel, uint8 volume, uint8 source);
	/**
	 * Process a MIDI panning control change event.
	 * Note that panning is not supported on an OPL2 chip because it has mono
	 * output.
	 * 
	 * @param channel The MIDI channel on which panning should be set.
	 * @param panning The panning value that should be set.
	 * @param source The source sending the control change event.
	 */
	virtual void panning(uint8 channel, uint8 panning, uint8 source);
	/**
	 * Process a MIDI expression control change event.
	 * 
	 * @param channel The MIDI channel on which expression should be set.
	 * @param expression The expression value that should be set.
	 * @param source The source sending the control change event.
	 */
	virtual void expression(uint8 channel, uint8 expression, uint8 source);
	/**
	 * Process a MIDI sustain control change event.
	 * 
	 * @param channel The MIDI channel on which sustain should be set.
	 * @param sustain The sustain value that should be set.
	 * @param source The source sending the control change event.
	 */
	virtual void sustain(uint8 channel, uint8 sustain, uint8 source);
	/**
	 * Process a MIDI registered parameter number control change event. This
	 * sets the currently active RPN; subsequent data entry control change
	 * events will set the value for the selected RPN.
	 * Note that a MIDI PRN event contains either the MSB or LSB; specify 0xFF
	 * for the other rpn byte to leave it unchanged.
	 * RPNs pitch bend sensitivity, master tuning fine and coarse are supported
	 * in accuracy mode GM only.
	 * 
	 * @param channel The MIDI channel on which the active PRN should be set.
	 * @param rpnMsb The MSB of the RPN number; 0xFF to not set the MSB.
	 * @param rpnLsb The LSB of the RPN number; 0xFF to not set the LSB.
	 * @param source The source sending the control change event.
	 */
	virtual void registeredParameterNumber(uint8 channel, uint8 rpnMsb, uint8 rpnLsb, uint8 source);
	/**
	 * Process a MIDI all sound off channel mode event.
	 * Note that this should immediately stop all sound, but it is not possible
	 * to abort the "release" phase of a note on an OPL chip. So this will
	 * function like an all notes off event, except it will also stop sustained
	 * notes.
	 * 
	 * @param channel The MIDI channel on which the all sound off channel mode
	 * event is sent.
	 * @param source The source sending the control change event.
	 */
	virtual void allSoundOff(uint8 channel, uint8 source);
	/**
	 * Process a MIDI reset all controllers channel mode event. This will reset
	 * the following controllers to their default values:
	 * - modulation
	 * - expression
	 * - sustain
	 * - active RPN
	 * - pitch bend
	 * - channel aftertouch
	 * It should also reset polyphonic aftertouch, but this is not implemented.
	 * 
	 * @param channel The MIDI channel on which the reset all controllers
	 * channel mode event is sent.
	 * @param source The source sending the control change event.
	 */
	virtual void resetAllControllers(uint8 channel, uint8 source);
	/**
	 * Process a MIDI all notes off channel mode event. This will turn off all
	 * non-sustained notes or sustain all notes if the sustain controller is on.
	 * 
	 * @param channel The MIDI channel on which the all notes off channel mode
	 * event is sent.
	 * @param source The source sending the control change event.
	 */
	virtual void allNotesOff(uint8 channel, uint8 source);

	/**
	 * Applies the controller default settings to the controller data for the
	 * specified source.
	 * This will set all supported default values specified on _controllerDefaults
	 * except sustain, which is set by deinitSource.
	 * 
	 * @param source The source triggering the default settings, or 0xFF to
	 * apply controller defaults for all sources.
	 */
	virtual void applyControllerDefaults(uint8 source);

	/**
	 * Recalculates and writes the frequencies of the active notes on the
	 * specified MIDI channel and source.
	 * 
	 * @param channel The MIDI channel on which the note frequencies should be
	 * recalculated.
	 * @param source The source for which the note frequencies should be
	 * recalculated.
	 */
	virtual void recalculateFrequencies(uint8 channel, uint8 source);
	/**
	 * Recalculates and writes the volumes of the active notes on the specified
	 * MIDI channel and source. 0xFF can be specified to recalculate volumes of
	 * notes on all MIDI channels and/or sources.
	 * 
	 * @param channel The MIDI channel on which the note volumes should be
	 * recalculated; 0xFF to recalculate volumes for all channels.
	 * @param source The source for which the note volumes should be
	 * recalculated; 0xFF to recalculate volumes for all sources.
	 */
	virtual void recalculateVolumes(uint8 channel, uint8 source);

	/**
	 * Determines the instrument data necessary to play the specified note on
	 * the specified MIDI channel and source. This will determine the
	 * instrument definition to use, the note that should be played and an
	 * instrument ID for use by the dynamic channel allocation algorithm.
	 * 
	 * @param channel The MIDI channel on which the note is played.
	 * @param source The source playing the note.
	 * @param note The MIDI note which is played.
	 * @return The instrument data for playing the note, or an empty struct if
	 * the note cannot be played.
	 */
	virtual InstrumentInfo determineInstrument(uint8 channel, uint8 source, uint8 note);
	/**
	 * Allocates an OPL channel to play a note on the specified MIDI channel
	 * and source with the specified instrument ID. Allocation behavior depends
	 * on the active channel allocation mode:
	 * - Dynamic: allocates an unused channel, a channel playing a note using
	 *   the same instrument or the channel playing the oldest note. This will
	 *   always allocate a channel to play the note. This is the same behavior
	 *   as the Win95 SB16 driver.
	 * - Static: allocates an unused OPL channel and assigns it to the MIDI
	 *   channel playing the note. All subsequent notes on this MIDI channel
	 *   will be played using this OPL channel. If there are no free channels, 
	 *   it will fail to allocate a channel. The MIDI data must play one note
	 *   at a time on each channel and not use more MIDI channels than there
	 *   are OPL channels for this algorithm to work properly.
	 * 
	 * @param channel The MIDI channel on which the note is played.
	 * @param source The source playing the note.
	 * @param instrumentId The ID of the instrument playing the note. Not used
	 * by the static channel allocation mode.
	 * @return The number of the allocated OPL channel; 0xFF if allocation
	 * failed (not possible using the dynamic channel allocation mode).
	 */
	virtual uint8 allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId);
	/**
	 * Determines which melodic channels are available based on the OPL chip
	 * type and rhythm mode setting and sets _melodicChannels and
	 * _numMelodicChannels to the determined values.
	 * This is called after constructing the driver with the OPL chip type and
	 * after calls to setRhythmMode.
	 */
	void determineMelodicChannels();
	/**
	 * Calculates the OPL frequency (F-num) and octave (block) to play the
	 * specified note on the specified MIDI channel and source, taking into
	 * account the MIDI controllers pitch bend and (on accuracy mode GM) pitch
	 * bend sensitivity and master tuning. The result is returned in the format
	 * of the Ax (low byte) and Bx (high byte) OPL registers.
	 * Note that the MIDI note range exceeds the frequency range of an OPL
	 * chip, so the highest MIDI notes will be shifted down one or two octaves.
	 * The SB16 Win95 accuracy mode calculates the same frequencies as the
	 * Windows 95 SB16 driver. The GM accuracy mode is more accurate, but the
	 * calculations are more CPU intensive. This mode also supports pitch bend
	 * sensitivity (which is fixed at 2 semitones in SB16 Win95 mode) and
	 * master tuning.
	 * 
	 * @param channel The MIDI channel on which the note is played.
	 * @param source The source playing the note.
	 * @param note The MIDI note which is played.
	 * @return The F-num and block to play the note on the OPL chip.
	 */
	virtual uint16 calculateFrequency(uint8 channel, uint8 source, uint8 note);
	/**
	 * Calculates the pitch bend value to apply to the specified OPL frequency
	 * (F-num) on the specified MIDI channel and source. If the accuracy mode
	 * is GM, pitch bend sensitivity and master tuning settings are also
	 * applied. The result is an adjustment which can be added to the OPL
	 * frequency to get the pitch bent note.
	 * 
	 * @param channel The MIDI channel for which pitch bend should be
	 * calculated.
	 * @param source The source for which pitch bend should be calculated.
	 * @param oplFrequency The OPL frequency for which pitch bend should be
	 * calculated.
	 * @return The calculated pitch bend (OPL frequency adjustment).
	 */
	virtual int32 calculatePitchBend(uint8 channel, uint8 source, uint16 oplFrequency);
	/**
	 * Calculates the volume for the specified operator of a note on the
	 * specified MIDI channel and source, using the specified MIDI velocity and
	 * instrument definition.
	 * This function will check if the operator will need to have volume
	 * applied to it or if the operator volume from the instrument definition
	 * should be used without adjustment (this depends on the connection type).
	 * If volume should be applied, unscaled volume is calculated
	 * (@see calculateUnscaledVolume) and volume is scaled to source and user
	 * volume. The volume is returned as an OPL 2x register volume (level)
	 * value, i.e. 0 = maximum volume, 3F = minimum volume.
	 * 
	 * @param channel The MIDI channel on which the note is played.
	 * @param source The source playing the note.
	 * @param velocity The MIDI velocity of the note for which volume should be
	 * calculated.
	 * @param instrumentDef The instrument definition used to play the note.
	 * @param operatorNum The number of the operator for which volume should be
	 * calculated; 0-1 for 2 operator instruments, 0-3 for 4 operator
	 * instruments.
	 * @return The calculated operator volume (level).
	 */
	virtual uint8 calculateVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum);
	/**
	 * Calculates the unscaled volume for the specified operator of a note on
	 * the specified MIDI channel and source, using the specified MIDI velocity
	 * and instrument definition.
	 * The SB16 Win95 accuracy mode calculates the same values as the Windows
	 * 95 SB16 driver. The GM accuracy mode is more accurate to the volume
	 * curve in the General MIDI specification and supports the expression
	 * controller, but the calculation is more CPU intensive.
	 * The volume is returned as an OPL 2x register volume (level) value,
	 * i.e. 0 = maximum volume, 3F = minimum volume.
	 * 
	 * @param channel The MIDI channel on which the note is played.
	 * @param source The source playing the note.
	 * @param velocity The MIDI velocity of the note for which volume should be
	 * calculated.
	 * @param instrumentDef The instrument definition used to play the note.
	 * @param operatorNum The number of the operator for which volume should be
	 * calculated; 0-1 for 2 operator instruments, 0-3 for 4 operator
	 * instruments.
	 * @return The calculated unscaled operator volume (level).
	 */
	virtual uint8 calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum);
	/**
	 * Determines the panning that should be applied to notes played on the
	 * specified MIDI channel and source.
	 * This will convert the MIDI panning controller value to simple left,
	 * right and center panning and return the result as a Cx register panning
	 * value (in bits 4 and 5) for an OPL3 chip.
	 * 
	 * @param channel The MIDI channel for which panning should be calculated.
	 * @param source The source for which panning should be calculated.
	 * @return The calculated panning.
	 */
	virtual uint8 calculatePanning(uint8 channel, uint8 source);

	/**
	 * Activates or deactivates the rhythm mode setting of the OPL chip. This
	 * setting uses 3 OPL channels to make 5 preset rhythm instruments
	 * available. Rhythm mode is turned off by default.
	 * Activating rhythm mode will deallocate and end active notes on channels
	 * 6 to 8. Deactivating rhythm mode will end active rhythm notes.
	 * If the specified setting is the same as the current setting, this method
	 * does nothing.
	 * 
	 * @param rhythmMode True if rhythm mode should be turned on; false if it
	 * should be turned off.
	 */
	virtual void setRhythmMode(bool rhythmMode);

	/**
	 * Determines the offset from a base register for the specified operator of
	 * the specified OPL channel or rhythm instrument.
	 * Add the offset to the base register to get the correct register for this
	 * operator and channel or rhythm instrument.
	 * 
	 * @param oplChannel The OPL channel for which to determine the offset.
	 * Ignored if a rhythm type is specified.
	 * @param operatorNum The operator for which to determine the offset;
	 * 0-1 for 2 operator instruments, 0-3 for 4 operator instruments. Ignored
	 * for rhythm instruments other than bass drum.
	 * @param rhythmType The rhythm instrument for which to determine the
	 * offset. Specify type undefined to determine the offset for a melodic
	 * instrument on the specified channel.
	 * @param fourOperator True if the instrument used is a 4 operator
	 * instrument; false if it is a 2 operator instrument. Ignored if a rhythm
	 * instrument type is specified.
	 * @return The offset to the base register for this operator.
	 */
	uint16 determineOperatorRegisterOffset(uint8 oplChannel, uint8 operatorNum, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED, bool fourOperator = false);
	/**
	 * Determines the offset from a base register for the specified OPL channel.
	 * Add the offset to the base register to get the correct register for this
	 * channel.
	 * 
	 * @param oplChannel The OPL channel for which to determine the offset.
	 * @param fourOperator True if the instrument used is a 4 operator
	 * instrument; false if it is a 2 operator instrument.
	 * @return The offset to the base register for this channel.
	 */
	uint16 determineChannelRegisterOffset(uint8 oplChannel, bool fourOperator = false);
	/**
	 * Writes the specified instrument definition to the specified OPL channel.
	 * It will calculate volume and panning if necessary.
	 * 
	 * @param oplChannel The OPL channel on which to write the instrument.
	 * @param instrument The data of the instrument to write.
	 */
	void writeInstrument(uint8 oplChannel, InstrumentInfo instrument);
	/**
	 * Sets the key on bit to false for the specified OPL channel or rhythm
	 * instrument and updates _activeNotes or _activeRhythmNotes with the new
	 * status.
	 * Specify forceWrite to force the OPL register to be written, even if the
	 * key on bit is already false according to the shadow registers.
	 * 
	 * @param oplChannel The OPL channel on which the key on bit should be set
	 * to false. Ignored if a rhythm type is specified.
	 * @param rhythmType The rhythm instrument for which the key on bit should
	 * be set to false.
	 * @param forceWrite True if the OPL register write should be forced; false
	 * otherwise.
	 */
	void writeKeyOff(uint8 oplChannel, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED, bool forceWrite = false);
	/**
	 * Determines the value for the rhythm register (0xBD) and writes the new
	 * value to the OPL chip. This register controls rhythm mode, the rhythm
	 * instruments and the vibrato and modulation depth settings.
	 */
	void writeRhythm(bool forceWrite = false);
	/**
	 * Calculates the volume for the specified OPL channel or rhythm instrument
	 * and operator (@see calculateVolume) and writes the new value to the OPL
	 * registers.
	 * 
	 * @param oplChannel The OPL channel for which volume should be calculated
	 * and written. Ignored if a rhythm type is specified.
	 * @param operatorNum The operator for which volume should be calculated
	 * and written.
	 * @param rhythmType The rhythm instrument for which volume should be
	 * calculated and written. Use type undefined to calculate volume for a
	 * melodic instrument.
	 */
	virtual void writeVolume(uint8 oplChannel, uint8 operatorNum, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED);
	/**
	 * Calculates the panning for the specified OPL channel or rhythm type
	 * (@see calculatePanning) and writes the new value to the OPL registers.
	 * 
	 * @param oplChannel The OPL channel for which panning should be calculated
	 * and written. Ignored if a rhythm type is specified.
	 * @param rhythmType The rhythm instrument for which panning should be
	 * calculated and written. Use type undefined to calculate panning for a
	 * melodic instrument.
	 */
	virtual void writePanning(uint8 oplChannel, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED);
	/**
	 * Calculates the frequency for the active note on the specified OPL
	 * channel or of the specified rhythm type (@see calculateFrequency) and
	 * writes the new value to the OPL registers.
	 * 
	 * @param oplChannel The OPL channel for which the frequency should be
	 * calculated and written. Ignored if a rhythm type is specified.
	 * @param rhythmType The rhythm instrument for which the frequency should
	 * be calculated and written. Use type undefined to calculate the frequency
	 * for a melodic instrument.
	 */
	virtual void writeFrequency(uint8 oplChannel, OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED);

	/**
	 * Writes the specified value to the specified OPL register.
	 * If the specified value is the same as the current value according to the
	 * shadow registers, the value is not written unless forceWrite is
	 * specified.
	 * 
	 * @param reg The OPL register where the value should be written
	 * (>= 0x100 for the second register set).
	 * @param value The value to write in the register.
	 * @param forceWrite True if the register write should be forced; false
	 * otherwise.
	 */
	void writeRegister(uint16 reg, uint8 value, bool forceWrite = false);

	// The type of OPL chip to use.
	OPL::Config::OplType _oplType;
	// The OPL emulator / hardware interface.
	OPL::OPL *_opl;

	// True if the driver has been successfully opened.
	bool _isOpen;
	// The number of timer callbacks per second.
	int _timerFrequency;
	// Controls the behavior for calculating note frequency and volume.
	AccuracyMode _accuracyMode;
	// Controls the OPL channel allocation behavior.
	ChannelAllocationMode _allocationMode;
	// Controls when the instrument definitions are written.
	InstrumentWriteMode _instrumentWriteMode;
	// Controls response to rhythm note off events when rhythm mode is active.
	bool _rhythmModeIgnoreNoteOffs;
	// Controls whether MIDI channel 10 is treated as the rhythm channel or as
	// a melodic channel.
	bool _channel10Melodic;

	// The default MIDI channel volume (set when opening the driver).
	uint8 _defaultChannelVolume;

	// OPL global settings. Set these, then call oplInit or open to apply the
	// new values.
	NoteSelectMode _noteSelect;
	ModulationDepth _modulationDepth;
	VibratoDepth _vibratoDepth;
	// Current OPL rhythm mode setting. Use setRhythmMode to set and activate.
	bool _rhythmMode;

	// Pointer to the melodic instrument definitions.
	OplInstrumentDefinition *_instrumentBank;
	// Pointer to the rhythm instrument definitions.
	OplInstrumentDefinition *_rhythmBank;
	// The MIDI note value of the first rhythm instrument in the bank.
	uint8 _rhythmBankFirstNote;
	// The MIDI note value of the last rhythm instrument in the bank.
	uint8 _rhythmBankLastNote;

	// The current MIDI controller values for each MIDI channel and source.
	MidiChannelControlData _controlData[MAXIMUM_SOURCES][MIDI_CHANNEL_COUNT];
	// The active note data for each OPL channel.
	ActiveNote _activeNotes[OPL3_NUM_CHANNELS];
	// The active note data for the OPL rhythm instruments.
	ActiveNote _activeRhythmNotes[5];
	// The OPL channel allocated to each MIDI channel and source; 0xFF if a
	// MIDI channel has no OPL channel allocated. Note that this is only used by
	// the static channel allocation mode.
	uint8 _channelAllocations[MAXIMUM_SOURCES][MIDI_CHANNEL_COUNT];
	// Array containing the numbers of the available melodic channels.
	const uint8 *_melodicChannels;
	// The number of available melodic channels (length of _melodicChannels).
	uint8 _numMelodicChannels;
	// The amount of notes played since the driver was opened / reset.
	uint32 _noteCounter;

	// Factor to convert a frequency in Hertz to the format used by the OPL
	// registers (F - num).
	float _oplFrequencyConversionFactor;
	// The values last written to each OPL register.
	uint8 _shadowRegisters[0x200];

	Common::Mutex _allocationMutex; // For operations on channel allocations
	Common::Mutex _activeNotesMutex; // For operations on active notes
};

#endif
