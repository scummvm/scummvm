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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * sound functionality
 */

#include "tinsel/sound.h"

#include "tinsel/dw.h"
#include "tinsel/config.h"
#include "tinsel/music.h"
#include "tinsel/strres.h"
#include "tinsel/tinsel.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/system.h"

#include "sound/mixer.h"
#include "sound/audiocd.h"

namespace Tinsel {

//--------------------------- General data ----------------------------------

SoundManager::SoundManager(TinselEngine *vm) :
	//_vm(vm),	// TODO: Enable this once global _vm var is gone
	_sampleIndex(0), _sampleIndexLen(0) {
}

SoundManager::~SoundManager() {
	free(_sampleIndex);
}

/**
 * Plays the specified sample through the sound driver.
 * @param id			Identifier of sample to be played
 * @param type			type of sound (voice or sfx)
 * @param handle		sound handle
 */
bool SoundManager::playSample(int id, Audio::Mixer::SoundType type, Audio::SoundHandle *handle) {
	// Floppy version has no sample file
	if (_vm->getFeatures() & GF_FLOPPY)
		return false;

	// no sample driver?
	if (!_vm->_mixer->isReady())
		return false;

	// stop any currently playing sample
	_vm->_mixer->stopHandle(_handle);

	// make sure id is in range
	assert(id > 0 && id < _sampleIndexLen);

	// get file offset for this sample
	uint32 dwSampleIndex = _sampleIndex[id];
	
	// move to correct position in the sample file
	_sampleStream.seek(dwSampleIndex);
	if (_sampleStream.ioFailed() || _sampleStream.pos() != dwSampleIndex)
		error("File %s is corrupt", SAMPLE_FILE);

	// read the length of the sample
	uint32 sampleLen = _sampleStream.readUint32LE();
	if (_sampleStream.ioFailed())
		error("File %s is corrupt", SAMPLE_FILE);

	// allocate a buffer
	void *sampleBuf = malloc(sampleLen);
	assert(sampleBuf);

	// read all of the sample
	if (_sampleStream.read(sampleBuf, sampleLen) != sampleLen)
		error("File %s is corrupt", SAMPLE_FILE);

	// FIXME: Should set this in a different place ;)
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volSound);
	//_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volVoice);


	// play it
	_vm->_mixer->playRaw(type, &_handle, sampleBuf, sampleLen, 22050,
						 Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED);

	if (handle)
		*handle = _handle;

	return true;
}

/**
 * Returns TRUE if there is a sample for the specified sample identifier.
 * @param id			Identifier of sample to be checked
 */
bool SoundManager::sampleExists(int id) {
	if (_vm->_mixer->isReady()) 	{
		// make sure id is in range
		if (id > 0 && id < _sampleIndexLen) {
			// check for a sample index
			if (_sampleIndex[id])
				return true;
		}
	}

	// no sample driver or no sample
	return false;
}

/**
 * Returns true if a sample is currently playing.
 */
bool SoundManager::sampleIsPlaying(void) {
	return _vm->_mixer->isSoundHandleActive(_handle);
}

/**
 * Stops any currently playing sample.
 */
void SoundManager::stopAllSamples(void) {
	// stop currently playing sample
	_vm->_mixer->stopHandle(_handle);
}

/**
 * Opens and inits all sound sample files.
 */
void SoundManager::openSampleFiles(void) {
	// Floppy and demo versions have no sample files
	if (_vm->getFeatures() & GF_FLOPPY || _vm->getFeatures() & GF_DEMO)
		return;

	Common::File f;

	if (_sampleIndex)
		// already allocated
		return;

	// open sample index file in binary mode
	if (f.open(SAMPLE_INDEX)) 	{
		// get length of index file
		f.seek(0, SEEK_END);		// move to end of file
		_sampleIndexLen = f.pos();	// get file pointer
		f.seek(0, SEEK_SET);		// back to beginning

		if (_sampleIndex == NULL) {
			// allocate a buffer for the indices
			_sampleIndex = (uint32 *)malloc(_sampleIndexLen);

			// make sure memory allocated
			if (_sampleIndex == NULL) {
				// disable samples if cannot alloc buffer for indices
				// TODO: Disabled sound if we can't load the sample index?
				return;
			}
		}

		// load data
		if (f.read(_sampleIndex, _sampleIndexLen) != (uint32)_sampleIndexLen)
			// file must be corrupt if we get to here
			error("File %s is corrupt", SAMPLE_FILE);

#ifdef SCUMM_BIG_ENDIAN
		// Convert all ids from LE to native format
		for (uint i = 0; i < _sampleIndexLen / sizeof(uint32); ++i) {
			_sampleIndex[i] = READ_LE_UINT32(_sampleIndex + i);
		}
#endif

		// close the file
		f.close();

		// convert file size to size in DWORDs
		_sampleIndexLen /= sizeof(uint32);
	} else
		error("Cannot find file %s", SAMPLE_INDEX);

	// open sample file in binary mode
	if (!_sampleStream.open(SAMPLE_FILE))
		error("Cannot find file %s", SAMPLE_FILE);

/*
	// gen length of the largest sample
	sampleBuffer.size = _sampleStream.readUint32LE();
	if (_sampleStream.ioFailed())
		error("File %s is corrupt", SAMPLE_FILE);
*/
}

} // end of namespace Tinsel
