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

#include "sci/sound/drivers/mididriver.h"

#include "audio/casio.h"

#include "sci/resource/resource.h"

namespace Sci {

class MidiDriver_Casio : public ::MidiDriver_Casio {
protected:
	// The instrument number used for the slap bass instrument on MT-540.
	static const uint8 SLAP_BASS_INSTRUMENT_MT540;
	// The instrument number used for the slap bass instrument on CT-460 and
	// CSM-1.
	static const uint8 SLAP_BASS_INSTRUMENT_CT460;
	static const uint8 PATCH_RESOURCE_SIZE;

public:
	MidiDriver_Casio(MusicType midiType) : ::MidiDriver_Casio(midiType),
			_highSplitInstOutputChannel(-1), _rhythmChannelMapped(false), _playSwitch(true) {
		Common::fill(_instrumentRemapping, _instrumentRemapping + ARRAYSIZE(_instrumentRemapping), 0);
		setInstrumentRemapping(_instrumentRemapping);
		_rhythmNoteRemapping = new byte[128];

		Common::fill(_instrumentFixedNotes, _instrumentFixedNotes + ARRAYSIZE(_instrumentFixedNotes), 0);
		Common::fill(_channelMap, _channelMap + ARRAYSIZE(_channelMap), 0);
		Common::fill(_channelFixedNotes, _channelFixedNotes + ARRAYSIZE(_channelFixedNotes), 0);

		_sendUntrackedNoteOff = false;
	}
	~MidiDriver_Casio() {
		delete[] _rhythmNoteRemapping;
	}

	bool loadResource(const SciSpan<const byte> &data, MusicType midiType = MT_AUTO);
	void initTrack(SciSpan<const byte> &header);

	void playSwitch(bool play);

protected:
	void noteOn(byte outputChannel, byte note, byte velocity, int8 source) override;
	void programChange(byte outputChannel, byte patchId, int8 source, bool applyRemapping = true) override;
	void programChange(byte outputChannel, byte patchId, int8 source, bool applyRemapping, bool applyBassSwap);

	int8 mapSourceChannel(uint8 source, uint8 dataChannel) override;
	byte mapInstrument(byte program, bool applyRemapping) override;
	int8 mapNote(byte outputChannel, byte note) override;
	bool isRhythmChannel(uint8 outputChannel) override;
	byte calculateVelocity(int8 source, byte velocity) override;

	byte _instrumentRemapping[128];
	// If > 0, a fixed note value should be played for the corresponding
	// instrument instead of the MIDI event note value.
	byte _instrumentFixedNotes[0x60];
	// Tracks the output channel which is currently being used by the "high"
	// split bass instrument (if any). Will be either -1 or 2.
	int8 _highSplitInstOutputChannel;

	int8 _channelMap[16];
	// True if the rhythm channel has been mapped to output channel 3.
	bool _rhythmChannelMapped;
	// The fixed note that needs to be played on each output channel instead of
	// the MIDI event note value (or 0 if there is no fixed note).
	byte _channelFixedNotes[4];
	bool _playSwitch;
};

class MidiPlayer_Casio : public MidiPlayer {
public:
	static const uint8 RESOURCE_HEADER_FLAG;

protected:
	static const byte PATCH_RESOURCE_MT540;
	static const byte PATCH_RESOURCE_CT460;

public:
	MidiPlayer_Casio(SciVersion soundVersion, MusicType midiType);
	~MidiPlayer_Casio() override;

	int open(ResourceManager *resMan) override;
	void close() override;

	byte getPlayId() const override;
	int getPolyphony() const override;
	bool hasRhythmChannel() const override;
	void setVolume(byte volume) override;
	void playSwitch(bool play) override;
	void initTrack(SciSpan<const byte> &header) override;
	int getLastChannel() const override;

	void send(uint32 b) override;

protected:
	MidiDriver_Casio *_casioDriver;
	MusicType _midiType;
};

const uint8 MidiDriver_Casio::SLAP_BASS_INSTRUMENT_MT540 = 0x14;
const uint8 MidiDriver_Casio::SLAP_BASS_INSTRUMENT_CT460 = 0x1E;

const uint8 MidiDriver_Casio::PATCH_RESOURCE_SIZE = 0xE9;

bool MidiDriver_Casio::loadResource(const SciSpan<const byte> &data, MusicType midiType) {
	if (midiType != MT_AUTO) {
		if (!(midiType == MT_MT540 || midiType == MT_CT460)) {
			error("CASIO: Unsupported music data type %i", midiType);
		}
		_midiType = midiType;
	}

	const uint32 size = data.size();
	if (size != PATCH_RESOURCE_SIZE) {
		error("CASIO: Unsupported patch format (%u bytes)", size);
		return false;
	}

	uint32 dataIndex = 0;
	for (int i = 0; i < 0x60; i++) {
		_instrumentRemapping[i] = data.getUint8At(dataIndex++);
		_instrumentFixedNotes[i] = data.getUint8At(dataIndex++);
	}
	for (int i = 0; i < 0x29; i++) {
		_rhythmNoteRemapping[0x23 + i] = data.getUint8At(dataIndex++);
	}

	return true;
}

void MidiDriver_Casio::initTrack(SciSpan<const byte> &header) {
	if (!_isOpen)
		return;

	Common::fill(_channelMap, _channelMap + ARRAYSIZE(_channelMap), -1);
	Common::fill(_rhythmChannel, _rhythmChannel + ARRAYSIZE(_rhythmChannel), false);
	Common::fill(_channelFixedNotes, _channelFixedNotes + ARRAYSIZE(_channelFixedNotes), 0);
	_rhythmChannelMapped = false;

	uint8 readPos = 0;
	uint8 caps = header.getInt8At(readPos++);
	if (caps != 0 && caps != 2)
		// Not a supported sound resource type.
		return;

	uint8 numChannels = 16;
	if (caps == 2)
		// Digital sound data on channel 15; don't use this channel.
		numChannels--;

	byte outputChannel = 0;
	for (int i = 0; i < numChannels; i++) {
		bool rhythmChannel = ((header.getInt8At(readPos++) & 0x80) > 0);
		bool deviceFlag = ((header.getInt8At(readPos++) & MidiPlayer_Casio::RESOURCE_HEADER_FLAG) > 0);
		if (!deviceFlag)
			// Data channel is not used for Casio devices.
			continue;

		if (rhythmChannel) {
			if (!_rhythmChannelMapped) {
				if (outputChannel == 4) {
					// The rhythm channel has already been assigned to a melodic
					// instrument. This means that more than 4 channels have
					// been flagged for Casio, which should not happen, but
					// clear the existing channel mapping just in case.
					for (int j = 0; j < numChannels; j++) {
						if (_channelMap[j] == 3)
							_channelMap[j] = -1;
					}
				}
				_channelMap[i] = 3;
				programChange(3, _midiType == MusicType::MT_MT540 ? RHYTHM_INSTRUMENT_MT540 : RHYTHM_INSTRUMENT_CT460, 0, false);
				_rhythmChannelMapped = true;
			}
		} else if (outputChannel < (_rhythmChannelMapped ? 3 : 4)) {
			_channelMap[i] = outputChannel++;
		}
	}
}

void MidiDriver_Casio::playSwitch(bool play) {
	_playSwitch = play;
	if (!_playSwitch)
		stopAllNotes(0xFF, 0xFF);
}

void MidiDriver_Casio::noteOn(byte outputChannel, byte note, byte velocity, int8 source) {
	if (velocity == 0) {
		// Note on with velocity 0 is a note off.
		noteOff(outputChannel, MIDI_COMMAND_NOTE_ON, note, velocity, source);
		return;
	}

	_mutex.lock();

	// Check if there is an available voice for this note.
	int polyphonyCount = 0;
	for (int i = 0; i < ARRAYSIZE(_activeNotes); i++) {
		// Note that this check ignores sustained notes; original driver does
		// this too.
		if (_activeNotes[i].channel == outputChannel && !_activeNotes[i].sustained) {
			polyphonyCount++;
		}
	}
	if (polyphonyCount >= CASIO_CHANNEL_POLYPHONY[outputChannel]) {
		// Maximum channel polyphony has been reached. Don't play this note.
		_mutex.unlock();
		return;
	}

	::MidiDriver_Casio::noteOn(outputChannel, note, velocity, source);

	_mutex.unlock();
}

void MidiDriver_Casio::programChange(byte outputChannel, byte patchId, int8 source, bool applyRemapping) {
	programChange(outputChannel, patchId, source, applyRemapping, true);
}

void MidiDriver_Casio::programChange(byte outputChannel, byte patchId, int8 source, bool applyRemapping, bool applyBassSwap) {
	if ((_rhythmChannelMapped && outputChannel == 3) || outputChannel >= 4)
		// Ignore program change on the rhythm channel or on unused channels.
		return;

	// Apply instrument mapping.
	byte mappedInstrument = mapInstrument(patchId, applyRemapping);

	// The Casio devices have an instrument (at 0x12 / 0x1C) which combines two
	// different bass instruments with a split note range. SCI assigns a
	// separate number to the slap bass instrument, which is on the "high" note
	// range (0x14 / 0x1E). Check for this number.
	if (mappedInstrument == (_deviceType == MT_MT540 ? SLAP_BASS_INSTRUMENT_MT540 : SLAP_BASS_INSTRUMENT_CT460)) {
		// The "high" split instrument (slap bass) has been selected.
		// Set the channel using this instrument so notes can be remapped to
		// the correct range.
		_highSplitInstOutputChannel = 2; // Output channel is set to 2 below.
		// Set the actual instrument number used by the Casio devices.
		mappedInstrument = (_deviceType == MT_MT540 ? BASS_INSTRUMENT_MT540 : BASS_INSTRUMENT_CT460);
	} else if (_highSplitInstOutputChannel == outputChannel) {
		// The instrument on this channel is changed from the "high" split
		// instrument to a different instrument. Reset the output channel
		// variable.
		_highSplitInstOutputChannel = -1;
	}

	// If the bass instrument is set on any channel, SCI always moves this
	// instrument to output channel 2. This is probably because the Casio
	// devices have a limited fixed polyphony on each channel: 6, 4,
	// 2 and 4 respectively. Moving the bass to channel 2 overcomes this
	// limitation somewhat, because this channel has the lowest polyphony and
	// the bass doesn't tend to play chords.
	// Check if the bass instrument is set to a channel other than 2, and
	// move it to channel 2 if necessary.
	if (applyBassSwap && mappedInstrument == (_deviceType == MT_MT540 ? BASS_INSTRUMENT_MT540 : BASS_INSTRUMENT_CT460) && outputChannel != 2) {
		_mutex.lock();

		int currentDataChannel = -1;
		int currentTargetDataChannel = -1;
		for (int i = 0; i < MIDI_CHANNEL_COUNT; i++) {
			if (_channelMap[i] == outputChannel) {
				currentDataChannel = i;
			} else if (_channelMap[i] == 2) {
				currentTargetDataChannel = i;
			}
		}

		// These data channels should always be mapped, but check it just in case.
		if (currentDataChannel >= 0 && currentTargetDataChannel >= 0) {
			// The original drivers do not stop all notes before swapping channels.
			// This could potentially cause hanging notes, so this is done here to
			// be safe. Instead, the original drivers swap out the registered
			// active notes between the channels. This does not accomplish anything
			// other than putting the driver state out of sync with the device
			// state.
			stopAllNotes(source, outputChannel);
			stopAllNotes(source, 2);

			_channelMap[currentDataChannel] = 2;
			_channelMap[currentTargetDataChannel] = outputChannel;

			programChange(outputChannel, _instruments[2], source, applyRemapping, false);

			outputChannel = 2;
		}

		_mutex.unlock();
	}

	// Register the new instrument.
	_instruments[outputChannel] = patchId;

	_channelFixedNotes[outputChannel] = (patchId < ARRAYSIZE(_instrumentFixedNotes) ? _instrumentFixedNotes[patchId] : 0);

	_rhythmChannel[outputChannel] =
		mappedInstrument == (_deviceType == MT_MT540 ? RHYTHM_INSTRUMENT_MT540 : RHYTHM_INSTRUMENT_CT460);

	_driver->send(MIDI_COMMAND_PROGRAM_CHANGE | outputChannel | (mappedInstrument << 8));
}

int8 MidiDriver_Casio::mapSourceChannel(uint8 source, uint8 dataChannel) {
	// Only source 0 is used by this driver.
	return _channelMap[dataChannel];
}

byte MidiDriver_Casio::mapInstrument(byte program, bool applyRemapping) {
	byte mappedInstrument = ::MidiDriver_Casio::mapInstrument(program, applyRemapping);

	if (applyRemapping) {
		// Correct remapping of the extra SCI slap bass instrument.
		if (_midiType == MT_MT540 && _deviceType == MT_CT460 &&
				mappedInstrument == INSTRUMENT_REMAPPING_MT540_TO_CT460[SLAP_BASS_INSTRUMENT_MT540]) {
			// For the MT-540, SCI uses 0x14 as the slap bass instrument, which
			// actually is the honky-tonk piano. If the instrument has been mapped
			// to the CT-460 honky-tonk piano, correct it to the CT-460 SCI slap
			// bass.
			mappedInstrument = SLAP_BASS_INSTRUMENT_CT460;
		} else if (_midiType == MT_CT460 && _deviceType == MT_MT540 && mappedInstrument == SLAP_BASS_INSTRUMENT_CT460) {
			// For the CT-460, SCI uses 0x1E as the slap bass instrument, which
			// is unused, so it will not be remapped. Manually remap it here to
			// the MT-540 SCI slap bass instrument.
			mappedInstrument = SLAP_BASS_INSTRUMENT_MT540;
		}
	}

	return mappedInstrument;
}

int8 MidiDriver_Casio::mapNote(byte outputChannel, byte note) {
	if (!isRhythmChannel(outputChannel) && outputChannel < 4) {
		if (_highSplitInstOutputChannel == outputChannel) {
			// The slap bass instrument has been set on this output channel.
			// Transpose the note up to the range used by this instrument.
			byte transposedNote = note + 0x18;
			if (transposedNote < 0x3C)
				transposedNote += 0xC;
			return transposedNote;
		}

		// Check if the MIDI event note should be replaced by a fixed note.
		byte fixedNote = _channelFixedNotes[outputChannel];
		return fixedNote > 0 ? fixedNote : note;
	}

	// Apply rhythm note mapping.
	return ::MidiDriver_Casio::mapNote(outputChannel, note);
}

bool MidiDriver_Casio::isRhythmChannel(uint8 outputChannel) {
	// SCI only uses channel 3 as the rhythm channel. If the drum instrument is
	// set on a different channel, it is for a sound effect and rhythm note
	// remapping should not be applied.
	return _rhythmChannelMapped && outputChannel == 3;
}

byte MidiDriver_Casio::calculateVelocity(int8 source, byte velocity) {
	if (!_playSwitch)
		return 0;

	return ::MidiDriver_Casio::calculateVelocity(source, velocity);
}

const uint8 MidiPlayer_Casio::RESOURCE_HEADER_FLAG = 0x08;

const byte MidiPlayer_Casio::PATCH_RESOURCE_MT540 = 4;
const byte MidiPlayer_Casio::PATCH_RESOURCE_CT460 = 7;

MidiPlayer_Casio::MidiPlayer_Casio(SciVersion soundVersion, MusicType midiType) : MidiPlayer(soundVersion) {
	_casioDriver = new MidiDriver_Casio(midiType);
	_driver = _casioDriver;
	_midiType = midiType;
}

MidiPlayer_Casio::~MidiPlayer_Casio() {
	delete _casioDriver;
	_casioDriver = nullptr;
	_driver = nullptr;
}

int MidiPlayer_Casio::open(ResourceManager* resMan) {
	if (_version < SCI_VERSION_0_LATE || _version > SCI_VERSION_01) {
		warning("CASIO: Unsupported SCI version.");
		return -1;
	}

	assert(resMan != nullptr);

	// WORKAROUND The Casio devices have a bass instrument which combines two
	// instruments on different note ranges. To make the second instrument
	// (slap bass) selectable, SCI assigns a new instrument number to this
	// instrument. On the MT-540 Sierra used instrument 0x14 (20), probably
	// because they thought this was the first unused instrument number.
	// However, besides the 20 instruments selectable on the keyboard, the
	// device has 10 more instruments which can only be selected via MIDI.
	// The result of this is that the instrument which actually uses number
	// 0x14 on the MT-540 (honky-tonk piano) cannot be used by the instrument
	// mapping. Sierra worked around this by using the normal piano instead of
	// the honky-tonk piano for the MT-540. This affects at least Hoyle 1.
	// The CT-460 and CSM-1 are not affected by this issue because Sierra used
	// instrument 0x1E (30) as the slap bass instrument. To fix this problem,
	// the CT-460 instrument mapping is also used for the MT-540, with the
	// output instruments remapped to their MT-540 equivalents.

	// Load the CT-460 patch resource.
	int patchResource = PATCH_RESOURCE_CT460;
	_midiType = MusicType::MT_CT460;
	Resource *res = resMan->findResource(ResourceId(kResourceTypePatch, patchResource), false);
	bool ok = false;

	if (res) {
		ok = _casioDriver->loadResource(*res, _midiType);
	}

	if (!ok) {
		// CT-460 patch resource not present. Fall back to the MT-540 resource.
		MusicType altMidiType = MT_MT540;
		int altPatchResource = PATCH_RESOURCE_CT460;
		warning("CASIO: Failed to load patch.00%i - falling back to patch.00%i", patchResource, altPatchResource);
		res = resMan->findResource(ResourceId(kResourceTypePatch, altPatchResource), false);

		if (res) {
			ok = _casioDriver->loadResource(*res, altMidiType);
		}

		if (!ok) {
			warning("CASIO: Failed to load fallback patch.00%i", altPatchResource);
			return -1;
		}

		_midiType = altMidiType;
	}

	return _casioDriver->open();
}

void MidiPlayer_Casio::close() {
	_driver->close();
}

byte MidiPlayer_Casio::getPlayId() const {
	return RESOURCE_HEADER_FLAG;
}

int MidiPlayer_Casio::getPolyphony() const {
	return 16;
}

bool MidiPlayer_Casio::hasRhythmChannel() const {
	// Only use the rhythm channel if it has the Casio flag set.
	return false;
}

void MidiPlayer_Casio::setVolume(byte volume) {
	_casioDriver->setSourceVolume(0, volume);
}

void MidiPlayer_Casio::playSwitch(bool play) {
	_casioDriver->playSwitch(play);
}

void MidiPlayer_Casio::initTrack(SciSpan<const byte> &header) {
	_casioDriver->initTrack(header);
}

int MidiPlayer_Casio::getLastChannel() const {
	// Not relevant for SCI0.
	return 15;
}

void MidiPlayer_Casio::send(uint32 b) {
	_driver->send(0, b);
}

MidiPlayer *MidiPlayer_Casio_create(SciVersion version, MusicType midiType) {
	return new MidiPlayer_Casio(version, midiType);
}

} // End of namespace Sci
