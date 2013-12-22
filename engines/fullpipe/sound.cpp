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
 */

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/sound.h"
#include "fullpipe/ngiarchive.h"

namespace Fullpipe {

SoundList::SoundList() {
	_soundItems = 0;
	_soundItemsCount = 0;
	_libHandle = 0;
}

bool SoundList::load(MfcArchive &file, char *fname) {
	debug(5, "SoundList::load()");

	_soundItemsCount = file.readUint32LE();
	_soundItems = (Sound **)calloc(_soundItemsCount, sizeof(Sound *));

	if (fname) {
	  _libHandle = (NGIArchive *)makeNGIArchive(fname);
	} else {
		_libHandle = 0;
	}

	for (int i = 0; i < _soundItemsCount; i++) {
		Sound *snd = new Sound();

		_soundItems[i] = snd;
		snd->load(file, _libHandle);
	}

	return true;
	
}

bool SoundList::loadFile(const char *fname, char *libname) {
	Common::File file;

	if (!file.open(fname))
		return false;

	MfcArchive archive(&file);

	return load(archive, libname);
}

Sound::Sound() {
	_id = 0;
	_directSoundBuffer = 0;
	_soundData = 0;
	_objectId = 0;
	memset(_directSoundBuffers, 0, sizeof(_directSoundBuffers));
	_description = 0;
}


bool Sound::load(MfcArchive &file, NGIArchive *archive) {
	debug(5, "Sound::load()");

	MemoryObject::load(file);

	_id = file.readUint32LE();
	_description = file.readPascalString();

	assert(g_fp->_gameProjectVersion >= 6);

	_objectId = file.readUint16LE();

	if (archive && archive->hasFile(_memfilename)) {
		Common::SeekableReadStream *s = archive->createReadStreamForMember(_memfilename);

		_soundData = (byte *)calloc(s->size(), 1);

		s->read(_soundData, s->size());

		delete s;
	}

	return true;
}

void Sound::updateVolume() {
	debug(3, "STUB Sound::updateVolume()");
}

void Sound::setPanAndVolumeByStaticAni() {
	debug(3, "STUB Sound::setPanAndVolumeByStaticAni()");
}

void FullpipeEngine::setSceneMusicParameters(GameVar *var) {
	warning("STUB: FullpipeEngine::setSceneMusicParameters()");
}

void FullpipeEngine::startSceneTrack() {
	debug(3, "STUB: FullpipeEngine::startSceneTrack()");
}

void FullpipeEngine::stopAllSounds() {
	warning("STUB: FullpipeEngine::stopAllSounds()");
}

void FullpipeEngine::toggleMute() {
	warning("STUB: FullpipeEngine::toggleMute()");
}

void FullpipeEngine::playSound(int id, int flag) {
	warning("STUB: FullpipeEngine::playSound(%d, %d)", id, flag);
}

void FullpipeEngine::playTrack(GameVar *sceneVar, const char *name, bool delayed) {
	warning("STUB: FullpipeEngine::playTrack(var, %s, %d)", name, delayed);
}

void global_messageHandler_handleSound(ExCommand *cmd) {
	debug(0, "STUB: global_messageHandler_handleSound()");
}

void FullpipeEngine::stopSoundStream2() {
	warning("STUB: FullpipeEngine::stopSoundStream2()");
}

void FullpipeEngine::stopAllSoundStreams() {
	warning("STUB: FullpipeEngine::stopAllSoundStreams()");
}

void FullpipeEngine::stopAllSoundInstances(int id) {
	warning("STUB: FullpipeEngine::stopAllSoundInstances(%d)", id);
}

} // End of namespace Fullpipe
