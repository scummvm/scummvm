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

#include "common/config-manager.h"
#include "common/memstream.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/decoders/raw.h"

// Miles Audio
#include "audio/miles.h"
#include "waynesworld/waynesworld.h"
#include "waynesworld/sound.h"

namespace WaynesWorld {

SoundManager::SoundManager(WaynesWorldEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	_effectsHandle = new Audio::SoundHandle();
	_stream = nullptr;
}

SoundManager::~SoundManager() {
	delete _effectsHandle;
}

byte SoundManager::decompDelta1(int16 *curVal, byte **src, byte **dst, byte count) {
	uint16 loop_cx = count >> 3;
	int val = *curVal;

	for (uint16 i = 0; i < loop_cx; i++) {
		uint8 bits = *(*src)++;
		for (int b = 0; b < 8; b++) {
			// Extract high bit, then shift left
			byte index = (bits & 0x80) ? 1 : 0;
			bits <<= 1;

			val = CLIP(val + _abtLookupTable[index], 0, 0xFF);
			*(*dst)++ = (char)val;
		}
	}
	return (byte)val;
}

// Unpacks 2-bit differences (4 values per byte)
byte SoundManager::decompDelta2(int16 *curVal, byte **src, byte **dst, byte count) {
	uint16 loop_cx = count >> 2;
	int val = *curVal;

	for (uint16 i = 0; i < loop_cx; i++) {
		uint8 bits = *(*src)++;
		for (int b = 0; b < 4; b++) {
			// Extract top 2 bits
			byte index = (bits >> 6) & 0x03;
			bits <<= 2;

			val = CLIP(val + _abtLookupTable[index], 0, 0xFF);
			*(*dst)++ = (char)val;
		}
	}
	return (byte)val;
}

// Unpacks 4-bit differences (2 values per byte)
byte SoundManager::decompDelta4(int16 *curVal, byte **src, byte **dst, byte count) {
	uint16 loop_cx = count >> 1;
	int val = *curVal;

	for (uint16 i = 0; i < loop_cx; i++) {
		uint8 bits = *(*src)++;
		for (int b = 0; b < 2; b++) {
			// Extract top 4 bits
			byte index = (bits >> 4) & 0x0F;
			bits <<= 4;

			val = CLIP(val + _abtLookupTable[index], 0, 0xFF);
			*(*dst)++ = (char)val;
		}
	}
	return (byte)val;
}

byte *SoundManager::abtDecomp(Common::File *fd, int *size, int *freq) {
	fd->seek(0);
	int compSize = fd->size();
	byte *tmpBuffer = new byte[compSize];
	fd->read(tmpBuffer, compSize);

	*size = READ_LE_UINT16(tmpBuffer);
	tmpBuffer += 2;
	*freq = READ_LE_UINT16(tmpBuffer);
	tmpBuffer += 2;
	int cx = *size;

	byte var_6 = *tmpBuffer++; // step size
	tmpBuffer += 3;           // skip header padding

	byte last_out = *tmpBuffer++;
	byte *destBuffer = new byte[*size];
	byte *headPtr = destBuffer;
	*destBuffer++ = last_out;
	cx--;

	while (cx > 0) {
		byte ctrl = *tmpBuffer++;

		if (ctrl & 0x80) { // Mode 1: Direct Shift
			last_out = (ctrl << 1);
			*destBuffer++ = last_out;
			cx--;
		} else if (ctrl & 0x40) { // Mode 2: RLE Fill
			uint16 run = ctrl & 0x3F;
			memset(destBuffer, last_out, run);
			*destBuffer += run;
			cx -= run;
		} else { // Mode 3: Delta Unpacking
			int16 mode_type = (ctrl >> 4);
			char step = (ctrl & 0x0F) + 1;

			// Build Table (Simplified from assembly neg/add logic)
			char entry = -step;
			int table_size = (mode_type == 1) ? 2 : (mode_type == 2) ? 4 : 16;
			if (mode_type == 2)
				entry <<= 1;
			else if (mode_type != 1)
				entry <<= 3; // Assembly loc_10E33 path

			for (int i = 0; i < table_size; i++) {
				_abtLookupTable[i] = entry;
				entry += step;
				if (entry == 0)
					entry = step;
			}

			int16 current_val = last_out;
			if (mode_type == 1)
				last_out = decompDelta1(&current_val, &tmpBuffer, &destBuffer, var_6);
			else if (mode_type == 2)
				last_out = decompDelta2(&current_val, &tmpBuffer, &destBuffer, var_6);
			else
				last_out = decompDelta4(&current_val, &tmpBuffer, &destBuffer, var_6);

			cx -= var_6;
		}
	}
	return headPtr;
}

void SoundManager::playSound(const char *filename, bool flag) {
	while (isSFXPlaying())
		_vm->waitMillis(10);

	_filename = Common::String(filename);
	
	int freq;
	int size;
	byte *buffer = nullptr;
	Common::File fd;
	if (!fd.open(Common::Path(filename))) {
		error("playSound : Enable to open %s", filename);
	}

	if (scumm_stricmp(filename + (_filename.size() - 3), "ABT") == 0) {
		buffer = abtDecomp(&fd, &size, &freq);
#ifdef WW_DUMPAUDIO
		Common::DumpFile dump;
		dump.open(Common::Path{Common::String(filename) + ".dump"});
		dump.write(buffer, size);
		dump.flush();
		dump.close();
#endif
	} else {
		size = fd.size();
		freq = 9000;
		buffer = new byte[size];
		fd.read(buffer, size);
	}

	Common::SeekableReadStream *rawStream = new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);
	Audio::RewindableAudioStream *audioStream = Audio::makeRawStream(rawStream, freq, Audio::FLAG_UNSIGNED);

	if (!_mixer->isSoundHandleActive(*_effectsHandle)) {
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _effectsHandle, audioStream, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}

	if (flag) {
		while (isSFXPlaying())
			_vm->waitMillis(10);
	}
}

bool SoundManager::isSFXPlaying() {
	return _mixer->isSoundHandleActive(*_effectsHandle);
}

void SoundManager::syncVolume() {
	const int sfxVol = CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, sfxVol);
}


void SoundManager::stopSound() {
	_mixer->stopHandle(*_effectsHandle);
}

/******************************************************************************************/

MusicManager::MusicManager(WaynesWorldEngine *vm)/* : _vm(vm) */{
	_music = nullptr;
	_isLooping = false;
	_driver = nullptr;

	const MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	MusicType musicType = MidiDriver::getMusicType(dev);

	switch (musicType) {
	case MT_ADLIB: {
		MidiPlayer::createDriver();
		break;
	}
	case MT_MT32:
		_driver = Audio::MidiDriver_Miles_MT32_create("");
		_nativeMT32 = true;
		break;
	case MT_GM:
		if (ConfMan.getBool("native_mt32")) {
			_driver = Audio::MidiDriver_Miles_MT32_create("");
			_nativeMT32 = true;
		}
		break;

	default:
		break;
	}

	if (_driver) {
		const int retValue = _driver->open();
		if (retValue == 0) {
			if (_nativeMT32)
				_driver->sendMT32Reset();
			else
				_driver->sendGMReset();

			_driver->setTimerCallback(this, &timerCallback);
		}
	}
}

MusicManager::~MusicManager() {
	delete[] _music;
}

void MusicManager::send(uint32 b) {
	// Pass data directly to driver
	_driver->send(b);
}

bool MusicManager::checkMidiDone() {
	return (!_isPlaying);
}

void MusicManager::midiRepeat() {
	if (!_driver)
		return;
	if (!_parser)
		return;

	_isLooping = true;
	_parser->property(MidiParser::mpAutoLoop, _isLooping);
	if (!_isPlaying)
		_parser->setTrack(0);
}

void MusicManager::stopSong() {
	if (!_driver)
		return;

	stop();
}

void MusicManager::playMusic(const char *filename, bool loop) {
	if (!_driver)
		return;

	Common::File fd;
	if (!fd.open(Common::Path(filename)))
		error("playMusic : Unable to open %s", filename);

	const int size = fd.size();
	if (size < 4)
		error("playMusic() wrong music resource size");

	delete[] _music;
	_music = new byte[size];
	fd.read(_music, size);

	stop();

	const uint32 magic = READ_BE_UINT32(_music);
	if (magic == MKTAG('F', 'O', 'R', 'M')) {
		_parser = MidiParser::createParser_XMIDI();

		if (!_parser->loadMusic(_music, size))
			error("midiPlay() wrong music resource");

		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

		// Handle music looping
		_parser->property(MidiParser::mpAutoLoop, loop);
		syncVolume();
		_isPlaying = true;
	} else {
		warning("playMusic() Unexpected signature 0x%08x, expected 'FORM'", magic);
		_isPlaying = false;
	}
}

void MusicManager::setLoop(bool loop) {
	_isLooping = loop;
	if (_parser)
		_parser->property(MidiParser::mpAutoLoop, _isLooping);
}
} // End of namespace WaynesWorld
