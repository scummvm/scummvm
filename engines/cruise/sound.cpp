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
 */

#include "common/endian.h"

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "cruise/sound.h"
#include "cruise/volume.h"

namespace Cruise {

MusicPlayer::MusicPlayer(): _looping(false), _isPlaying(false), _songPlayed(false) {
	_songPointer = NULL;
	_masterVolume = 0;
}

MusicPlayer::~MusicPlayer() {
	stop();
	if (_songPointer != NULL)
		free(_songPointer);
}

void MusicPlayer::setVolume(int volume) {
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;
}

void MusicPlayer::stop() {
	_isPlaying = false;
	_songPlayed = songLoaded();
}

void MusicPlayer::pause() {
	setVolume(-1);
	_isPlaying = false;
}

void MusicPlayer::resume() {
	setVolume(_masterVolume);
	_isPlaying = true;
	_songPlayed = false;
}

void MusicPlayer::doSync(Common::Serializer &s) {
	// synchronise current music name, if any, state, and position
	s.syncBytes((byte *)_musicName, 33);
	uint16 v = (uint16)songLoaded();
	s.syncAsSint16LE(v);
	s.syncAsSint16LE(_songPlayed);
	s.syncAsSint16LE(_looping);
}

void MusicPlayer::loadSong(const char *name) {
	char tempName[20], baseName[20];
	uint8 *sampleData;

	if (songLoaded())
		removeSong();

	// Load the correct file
	int fileIdx = findFileInDisks(name);
	if (fileIdx < 0) return;

	int unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;
	_songPointer = (byte *)malloc(unpackedSize);
	assert(_songPointer);

	if (volumePtrToFileDescriptor[fileIdx].size + 2 != unpackedSize) {
		uint8 *packedBuffer = (uint8 *)mallocAndZero(volumePtrToFileDescriptor[fileIdx].size + 2);

		loadPackedFileToMem(fileIdx, packedBuffer);

		uint32 realUnpackedSize = READ_BE_UINT32(packedBuffer + volumePtrToFileDescriptor[fileIdx].size - 4);

		delphineUnpack(_songPointer, packedBuffer, volumePtrToFileDescriptor[fileIdx].size);
		_songSize = realUnpackedSize;

		free(packedBuffer);
	} else {
		loadPackedFileToMem(fileIdx, _songPointer);
		_songSize = unpackedSize;
	}

	strcpy(_musicName, name);

	// Get the details of the song
	// TODO: Figure this out for sure for use in actually playing song
	//int size = *(_songPointer + 470);
	//int speed = 244 - *(_songPointer + 471);
	//int musicSpeed = (speed * 100) / 1060;


	// Get the file without the extension
	strcpy(baseName, name);
	char *p = strchr(baseName, '.');
	if (p)
		*p = '\0';

	// Get the instruments states file
	strcpy(tempName, baseName);
	strcat(tempName, ".IST");

	fileIdx = findFileInDisks(tempName);
	if (fileIdx >= 0) {
		// TODO: Figure out instrument state usage
		uint8 instrumentState[15];
		loadPackedFileToMem(fileIdx, instrumentState);
	}

	for (int instrumentCtr = 0; instrumentCtr < 15; ++instrumentCtr) {
		if (_vm->mt32()) {
			// Handle loading Roland instrument data
			strcpy(tempName, baseName);
			strcat(tempName, ".H32");

			sampleData = loadInstrument(tempName, instrumentCtr);
			if (sampleData) {
				int v = *sampleData;
				if ((v >= 128) && (v < 192))
					patchMidi(0x80000L + (instrumentCtr * 512), sampleData + 1, 254);

				// TODO: Currently I'm freeing the instrument data immediately. The original
				// holds onto the sample data, so it may actually still be needed
				free(sampleData);
			}
		} else if (_vm->adlib()) {
			// Handle loading Adlib instrument data
			strcpy(tempName, baseName);
			strcat(tempName, ".ADL");

			fileIdx = findFileInDisks(tempName);
			if (fileIdx >= 0) {
				sampleData = (byte *)malloc(volumePtrToFileDescriptor[fileIdx].extSize + 2);
				assert(sampleData);
				loadPackedFileToMem(fileIdx, sampleData);

				// TODO: Make use of sample data

				free(sampleData);
			}
		}
	}

	_songPlayed = false;
	_isPlaying = false;
}

void MusicPlayer::startSong() {
	if (songLoaded()) {
		// Start playing song here
	}
}

void MusicPlayer::removeSong() {
	if (isPlaying())
		stop();

	if (_songPointer) {
		free(_songPointer);
		_songPointer = NULL;
	}

	_songPlayed = false;

	strcpy(_musicName, "");
}

void MusicPlayer::fadeSong() {
	// TODO: Implement fading properly
	stop();
}

void MusicPlayer::patchMidi(uint32 adr, const byte *data, int size) {
	// TODO: Handle patching midi
}

byte *MusicPlayer::loadInstrument(const char *name, int i) {
	// Find the resource
	int fileIdx = findFileInDisks(name);
	if (fileIdx < 0) {
		warning("Instrument '%s' not found", name);
		return NULL;
	}

	int size = volumePtrToFileDescriptor[fileIdx].extSize;

	// Get the data
	byte *tmp = (byte *)malloc(size);
	assert(tmp);
	loadPackedFileToMem(fileIdx, tmp);

	// Create a copy of the resource that's 22 bytes smaller
	byte *result = (byte *)malloc(size - 22);
	assert(result);
	Common::copy(tmp, tmp + size - 22, result);

	free(tmp);
	return result;
}

} // End of namespace Cruise
