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

#include "agos/sfxparser_accolade.h"

#include "common/stream.h"
#include "common/textconsole.h"

namespace AGOS {

SfxParser_Accolade::SfxSlot::SfxSlot() {
	clear();
}

void SfxParser_Accolade::SfxSlot::reset() {
	noteFractionDelta = 0;
	vibratoTime = 0;
	vibratoCounter = 0;
	vibratoDelta = 0;
	waitCounter = 0;
	loopStart = 0;
	loopCounter = 0;
}

void SfxParser_Accolade::SfxSlot::clear() {
	allocated = false;
	active = false;
	source = -1;
	scriptPos = 0;
	playTime = 0;
	lastEventTime = 0;
	lastPlayedNote = -1;
	currentNoteFraction = 0;
	programChanged = false;
	reset();
}

bool SfxParser_Accolade::SfxSlot::atEndOfScript() {
	return scriptPos >= sfxData->scriptSize;
}

int16 SfxParser_Accolade::SfxSlot::readScript(bool opCode) {
	if (atEndOfScript())
		error("SfxParser_Accolade::SfxData::readScript - attempt to read past the end of the script");

	int16 data = sfxData->scriptData[scriptPos];
	scriptPos++;

	if (opCode && (data <= 0 || data > 0xC)) {
		// Any opcode outside the range 1-B will cause the script to stop.
		data = 0xC;
	}

	return data;
}

const byte SfxParser_Accolade::INSTRUMENT_SIZE_MT32;
const uint16 SfxParser_Accolade::SCRIPT_TIMER_FREQUENCY;
const uint16 SfxParser_Accolade::SCRIPT_TIMER_RATE;

SfxParser_Accolade::SfxParser_Accolade() : _driver(nullptr), _timerRate(0), _sfxData(),
	_numSfx(0), _sourceAllocations { -1, -1, -1, -1 }, _paused(false) { }

SfxParser_Accolade::~SfxParser_Accolade() {
	stopAll();

	if (_sfxData) {
		delete[] _sfxData;
		_sfxData = nullptr;
	}
}

void SfxParser_Accolade::load(Common::SeekableReadStream *in, int32 size) {
	// First word is the total data size.
	uint16 dataSize = in->readUint16LE();
	if (dataSize > size)
		error("SfxParser_Accolade::load - Sound effect bank lists size %d but has file size %d", dataSize, size);

	// Next word is the number of SFX definitions.
	_numSfx = in->readUint16LE();
	_sfxData = new SfxData[_numSfx];

	// Next is a list of start offsets for each SFX definition. Combined with
	// the total size the size of each SFX definition can be determined.
	int64 indexStartPos = in->pos();
	for (int i = 0; i < _numSfx; i++) {
		in->seek(indexStartPos + (i * 2));
		uint16 sfxDataOffset = in->readUint16LE();
		uint16 sfxDataEndOffset = i < _numSfx - 1 ? in->readUint16LE() : dataSize - 4;
		in->seek(indexStartPos + sfxDataOffset);
		uint16 sfxDataSize = sfxDataEndOffset - sfxDataOffset;

		// Read instrument definition.
		readInstrument(&_sfxData[i], in);

		// Instrument data size is fixed; the reset of the SFX definition is
		// the script.
		int scriptSize = sfxDataSize - INSTRUMENT_SIZE_MT32 - INSTRUMENT_SIZE_ADLIB;
		if (scriptSize < 2)
			error("SfxParser_Accolade::load - Unexpected script size %d", scriptSize);
		if (scriptSize % 2 != 0)
			warning("SfxParser_Accolade::load - Script has odd number of bytes %d", scriptSize);
		scriptSize >>= 1;
		// Script size is stored in words.
		_sfxData[i].scriptSize = scriptSize;

		// Read each word into the script data.
		for (int j = 0; j < scriptSize; j++) {
			_sfxData[i].scriptData[j] = in->readSint16LE();
		}
	}
}

void SfxParser_Accolade::setTimerRate(uint32 rate) {
	_timerRate = rate;
}

void SfxParser_Accolade::play(uint8 sfxNumber) {
	Common::StackLock lock(_mutex);

	if (sfxNumber >= _numSfx) {
		warning("SfxParser_Accolade::play - Sound effect %d requested but bank has only %d sound effects", sfxNumber, _numSfx);
		return;
	}

	// Find an unallocated slot.
	SfxSlot *sfxSlot = nullptr;
	int sfxSlotNum = -1;
	for (int i = 0; i < ARRAYSIZE(_sfxSlots); i++) {
		if (!_sfxSlots[i].allocated) {
			_sfxSlots[i].allocated = true;
			sfxSlot = &_sfxSlots[i];
			sfxSlotNum = i;
			break;
		}
	}

	// Note that the original interpreter would only output MIDI data from 2
	// slots simultaneously, but potentially *all* SFX could be active at the
	// same time (but the same sound effect could not play more than once at
	// the same time).
	// This implementation only allows 4 SFX active at the same time, which
	// seems to be more than enough for Elvira 2 and Waxworks.
	if (!sfxSlot)
		return;

	// Allocate a source.
	for (int i = 0; i < getNumberOfSfxSources(); i++) {
		if (_sourceAllocations[i] == -1) {
			_sourceAllocations[i] = sfxSlotNum;
			sfxSlot->source = i + 1;
			break;
		}
	}

	// Set the SFX data and load the instrument into the allocated channel.
	sfxSlot->sfxData = &_sfxData[sfxNumber];
	sfxSlot->programChanged = loadInstrument(sfxSlot);

	// Activate the slot to start script execution.
	sfxSlot->active = true;
}

void SfxParser_Accolade::stopAll() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < ARRAYSIZE(_sfxSlots); i++) {
		if (_sfxSlots[i].active)
			stop(&_sfxSlots[i]);
	}
}

void SfxParser_Accolade::pauseAll(bool paused) {
	Common::StackLock lock(_mutex);

	if (_paused == paused)
		return;

	_paused = paused;

	if (_paused) {
		// Stop the current note for all active SFX.
		for (int i = 0; i < ARRAYSIZE(_sfxSlots); i++) {
			if (_sfxSlots[i].active)
				noteOff(&_sfxSlots[i]);
		}
	}
}

void SfxParser_Accolade::stop(SfxSlot *sfxSlot) {
	noteOff(sfxSlot);

	// Deallocate the source.
	if (sfxSlot->source >= 0) {
		_driver->deinitSource(sfxSlot->source);
		_sourceAllocations[sfxSlot->source - 1] = -1;
	}

	// The original interpreter would try to re-assign the source to an active
	// sound effect without a source. This is not implemented here because
	// Elvira 2 and Waxworks use SFX very sparingly, so it seems very unlikely
	// that more than 2 SFX would be active at the same time.

	sfxSlot->clear();
}

void SfxParser_Accolade::processOpCode(SfxSlot *sfxSlot, byte opCode) {
	switch (opCode) {
	case 0x1:
		// Set note and note fraction delta.
		sfxSlot->noteFractionDelta = sfxSlot->readScript(false);
		break;
	case 0x2:
		// Clear note and note fraction delta.
		sfxSlot->noteFractionDelta = 0;
		break;
	case 0x3:
		// Set vibrato.
		int16 vibratoTime;
		vibratoTime = sfxSlot->readScript(false);
		assert(vibratoTime >= 0);
		sfxSlot->vibratoTime = vibratoTime;
		// The counter starts at half the vibrato time, which causes the note
		// frequency to move above and below the note fraction (like a sine
		// wave).
		sfxSlot->vibratoCounter = (vibratoTime >> 1) | 1;
		sfxSlot->vibratoDelta = sfxSlot->readScript(false);
		break;
	case 0x4:
		// Clear vibrato.
		sfxSlot->vibratoTime = 0;
		sfxSlot->vibratoDelta = 0;
		break;
	case 0x5:
		// Wait.
		sfxSlot->waitCounter = sfxSlot->readScript(false);
		assert(sfxSlot->waitCounter >= 0);
		break;
	case 0x6:
		// Play note.
		noteOff(sfxSlot);
		int8 note;
		note = sfxSlot->readScript(false) & 0xFF;
		assert(note >= 0);
		sfxSlot->currentNoteFraction = note << 8;
		noteOn(sfxSlot);
		break;
	case 0x7:
		// Loop start.

		// Just register the loop start position.
		sfxSlot->loopStart = sfxSlot->scriptPos;
		break;
	case 0x8:
		// Loop next.
		int16 loopParam;
		loopParam = sfxSlot->readScript(false);
		assert(loopParam >= 0);
		if (sfxSlot->loopCounter == 0) {
			// Loop counter has not been set yet, so do this now.
			if (loopParam == 0)
				// Loop infinitely.
				loopParam = -1;
			sfxSlot->loopCounter = loopParam;
			// Go back to loop start.
			sfxSlot->scriptPos = sfxSlot->loopStart;
		} else {
			// Decrease loop counter, unless the loop is infinite.
			if (sfxSlot->loopCounter != -1)
				sfxSlot->loopCounter--;
			if (sfxSlot->loopCounter != 0)
				// Go back to loop start.
				sfxSlot->scriptPos = sfxSlot->loopStart;
			// Else continue the script.
		}
		break;
	case 0x9:
		// Stop the current note.
		noteOff(sfxSlot);
		break;
	case 0xA:
		// Reset sound effect data.
		sfxSlot->reset();
		// The original interpreter does this; not sure why...
		sfxSlot->vibratoCounter = 1;
		break;
	case 0xB:
		// Noop. Consumes 1 script parameter.
		sfxSlot->scriptPos++;
		break;
	case 0xC:
	default:
		// Stop the sound effect.
		stop(sfxSlot);
		break;
	}
}

void SfxParser_Accolade::noteOn(SfxSlot *sfxSlot) {
	byte note = sfxSlot->currentNoteFraction >> 8;
	if (sfxSlot->source >= 0)
		// Send a note on with maximum velocity.
		_driver->send(sfxSlot->source, 0x90 | (note << 8) | (0x7F << 16));
	sfxSlot->lastPlayedNote = note;
}

void SfxParser_Accolade::noteOff(SfxSlot *sfxSlot) {
	if (sfxSlot->lastPlayedNote < 0)
		return;

	if (sfxSlot->source >= 0)
		// Send a note off.
		_driver->send(sfxSlot->source, 0x80 | (sfxSlot->lastPlayedNote << 8));
	sfxSlot->lastPlayedNote = -1;
}

void SfxParser_Accolade::onTimer() {
	Common::StackLock lock(_mutex);

	if (_paused)
		return;

	for (int i = 0; i < ARRAYSIZE(_sfxSlots); i++) {
		if (!_sfxSlots[i].active)
			continue;

		if (!_sfxSlots[i].programChanged) {
			// If the SFX instrument has not yet been set on the allocated
			// channel, wait until the driver is ready.
			if (!_driver->isReady(_sfxSlots[i].source))
				continue;

			// Then change to the SFX instrument.
			changeInstrument(&_sfxSlots[i]);
			_sfxSlots[i].programChanged = true;
		}

		// Determine the end time of the timer callback period that will be
		// processed.
		uint32 endTime = _sfxSlots[i].playTime + _timerRate;
		// Process script ticks while the sound effect remains active.
		while (_sfxSlots[i].active) {
			// Determine the end time of the script tick that will be
			// processed.
			uint32 eventTime = _sfxSlots[i].lastEventTime + SCRIPT_TIMER_RATE;
			if (eventTime > endTime)
				// Script tick end time is after this timer callback period, so
				// leave it to the next callback invocation.
				break;

			// Process this script tick.
			_sfxSlots[i].lastEventTime = eventTime;

			// Process vibrato counter.
			if (_sfxSlots[i].vibratoCounter > 0) {
				// Count down the vibrato counter.
				_sfxSlots[i].vibratoCounter--;
			} else {
				// Vibrato counter has reached zero. The vibrato note fraction
				// delta is negated, so that it will now be subtracted from the
				// note fraction instead of added, or the other way around.
				_sfxSlots[i].vibratoDelta = -_sfxSlots[i].vibratoDelta;
				// Reset the vibrato counter so it counts down to the next
				// delta negation.
				_sfxSlots[i].vibratoCounter = _sfxSlots[i].vibratoTime;
			}

			// Calculate the new note and note fraction by adding the deltas.
			uint16 newNoteFraction = _sfxSlots[i].currentNoteFraction;
			newNoteFraction += _sfxSlots[i].noteFractionDelta;
			newNoteFraction += _sfxSlots[i].vibratoDelta;

			if (newNoteFraction != _sfxSlots[i].currentNoteFraction) {
				// Update the note fraction.
				_sfxSlots[i].currentNoteFraction = newNoteFraction;
				updateNote(&_sfxSlots[i]);
			}

			// Process the script.
			if (_sfxSlots[i].waitCounter > 0) {
				// Count down the wait counter. No script opcode will be
				// processsed.
				_sfxSlots[i].waitCounter--;
			} else if (_sfxSlots[i].atEndOfScript()) {
				// The end of the script has been reached, so stop the sound
				// effect.
				// Note that the original interpreter did not do any bounds
				// checking. Some scripts are not terminated properly and would
				// cause the original interpreter to overread into the
				// instrument data of the next sound effect.
				stop(&_sfxSlots[i]);
			} else {
				// Process the next script opcode.
				byte opCode = _sfxSlots[i].readScript(true) & 0xFF;
				processOpCode(&_sfxSlots[i], opCode);
			}
		}
		
		// If the sound effect is still active, update the play timestamp.
		if (_sfxSlots[i].active)
			_sfxSlots[i].playTime = endTime;
	}
}

void SfxParser_Accolade::timerCallback(void *data) {
	((SfxParser_Accolade *)data)->onTimer();
}

void SfxParser_Accolade_AdLib::setMidiDriver(MidiDriver_Multisource *driver) {
	_driver = driver;
	_adLibDriver = dynamic_cast<MidiDriver_Accolade_AdLib *>(driver);
	assert(_adLibDriver);
}

byte SfxParser_Accolade_AdLib::getNumberOfSfxSources() {
	// The number of available sources depends on the OPL chip emulation used.
	return _adLibDriver->getNumberOfSfxSources();
}

void SfxParser_Accolade_AdLib::readInstrument(SfxData *sfxData, Common::SeekableReadStream *in) {
	in->skip(INSTRUMENT_SIZE_MT32);
	in->read(sfxData->instrumentDefinition, INSTRUMENT_SIZE_ADLIB);
}

bool SfxParser_Accolade_AdLib::loadInstrument(SfxSlot *sfxSlot) {
	if (sfxSlot->source < 0)
		return true;

	_adLibDriver->loadSfxInstrument(sfxSlot->source, sfxSlot->sfxData->instrumentDefinition);
	// No separate instrument change is necessary for AdLib, so true is
	// returned to indicate the instrument is already changed.
	return true;
}

void SfxParser_Accolade_AdLib::noteOn(SfxSlot *sfxSlot) {
	if (sfxSlot->source >= 0)
		// Set the current note fraction first.
		_adLibDriver->setSfxNoteFraction(sfxSlot->source, sfxSlot->currentNoteFraction);
	// Then start the note.
	SfxParser_Accolade::noteOn(sfxSlot);
}

void SfxParser_Accolade_AdLib::updateNote(SfxSlot *sfxSlot) {
	if (sfxSlot->source < 0)
		return;

	// Set the current note fraction first.
	_adLibDriver->setSfxNoteFraction(sfxSlot->source, sfxSlot->currentNoteFraction);
	// Then update the note.
	_adLibDriver->updateSfxNote(sfxSlot->source);
}

void SfxParser_Accolade_MT32::setMidiDriver(MidiDriver_Multisource *driver) {
	_driver = driver;
	_mt32Driver = dynamic_cast<MidiDriver_Accolade_MT32 *>(driver);
	assert(_mt32Driver);
}

byte SfxParser_Accolade_MT32::getNumberOfSfxSources() {
	// MT-32 always uses 2 SFX sources.
	return 2;
}

void SfxParser_Accolade_MT32::readInstrument(SfxData *sfxSlot, Common::SeekableReadStream *in) {
	in->read(sfxSlot->instrumentDefinition, INSTRUMENT_SIZE_MT32);
	in->skip(INSTRUMENT_SIZE_ADLIB);
}

bool SfxParser_Accolade_MT32::loadInstrument(SfxSlot *sfxSlot) {
	if (sfxSlot->source < 0)
		return true;

	_mt32Driver->loadSfxInstrument(sfxSlot->source, sfxSlot->sfxData->instrumentDefinition);
	// MT-32 requires a program change after the SysExes to load the instrument
	// have been processed. Return false to indicate this.
	return false;
}

void SfxParser_Accolade_MT32::changeInstrument(SfxSlot *sfxData) {
	if (sfxData->source < 0)
		return;

	_mt32Driver->changeSfxInstrument(sfxData->source);
}

} // End of namespace AGOS
