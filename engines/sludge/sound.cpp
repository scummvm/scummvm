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

#include "common/debug.h"
#include "common/file.h"
#include "common/list.h"
#include "common/memstream.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/vorbis.h"
#include "audio/mods/mod_xm_s3m.h"

#include "sludge/allfiles.h"
#include "sludge/fileset.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sound.h"

namespace Sludge {

const int SoundManager::MAX_SAMPLES = 8;
const int SoundManager::MAX_MODS = 3;

SoundManager::SoundManager() {
	_soundCache = nullptr;
	_soundCache = new SoundThing[MAX_SAMPLES];

	_modCache = nullptr;
	_modCache = new SoundThing[MAX_MODS];

	init();
}

SoundManager::~SoundManager() {
	killSoundStuff();

	delete []_soundCache;
	_soundCache = nullptr;

	delete []_modCache;
	_modCache = nullptr;
}

void SoundManager::init() {
	// there's possibility that several sound list played at the same time
	_soundListHandles.clear();

	_soundOK = false;
	_silenceIKillYou = false;
	_isHandlingSoundList = false;

	_defVol = 128;
	_defSoundVol = 255;
	_modLoudness = 0.95f;

	_emptySoundSlot = 0;
}

bool SoundManager::initSoundStuff() {
	for (int a = 0; a < MAX_SAMPLES; ++a) {
		_soundCache[a].fileLoaded = -1;
		_soundCache[a].looping = false;
		_soundCache[a].inSoundList = false;
	}

	for (int a = 0; a < MAX_MODS; ++a) {
		_modCache[a].fileLoaded = -1;
		_modCache[a].looping = false;
		_modCache[a].inSoundList = false;
	}

	return _soundOK = true;
}

void SoundManager::killSoundStuff() {
	if (!_soundOK)
		return;

	for (int i = 0; i < MAX_SAMPLES; ++i)
		freeSound(i);

	for (int i = 0; i < MAX_MODS; ++i)
		stopMOD(i);
}

/*
 * Some setters:
 */
void SoundManager::setMusicVolume(int a, int v) {
	if (!_soundOK)
		return;

	if (g_sludge->_mixer->isSoundHandleActive(_modCache[a].handle)) {
		_modCache[a].vol = v;
		g_sludge->_mixer->setChannelVolume(_modCache[a].handle, _modLoudness * v);
	}
}

void SoundManager::setDefaultMusicVolume(int v) {
	_defVol = v;
}

void SoundManager::setSoundVolume(int a, int v) {
	if (!_soundOK)
		return;
	int ch = findInSoundCache(a);
	if (ch != -1) {
		if (g_sludge->_mixer->isSoundHandleActive(_soundCache[ch].handle)) {
			_soundCache[ch].vol = v;
			g_sludge->_mixer->setChannelVolume(_soundCache[ch].handle, v);
		}
	}
}

void SoundManager::setDefaultSoundVolume(int v) {
	_defSoundVol = v;
}

void SoundManager::setSoundLoop(int a, int s, int e) {
//#pragma unused (a,s,e)
}

int SoundManager::findInSoundCache(int a) {
	int i;
	for (i = 0; i < MAX_SAMPLES; i++) {
		if (_soundCache[i].fileLoaded == a) {
			return i;
		}
	}
	return -1;
}

void SoundManager::stopMOD(int i) {
	if (!_soundOK)
		return;

	if (_modCache[i].fileLoaded >= 0) {
		if (g_sludge->_mixer->isSoundHandleActive(_modCache[i].handle)) {
			g_sludge->_mixer->stopHandle(_modCache[i].handle);
		}
	}
	_modCache[i].fileLoaded = -1;
}

void SoundManager::huntKillSound(int filenum) {
	if (!_soundOK)
		return;

	int gotSlot = findInSoundCache(filenum);
	if (gotSlot == -1)
		return;

	freeSound(gotSlot);
}

void SoundManager::freeSound(int a) {
	if (!_soundOK)
		return;

	_silenceIKillYou = true;
	if (_soundCache[a].fileLoaded >= 0) {
		if (g_sludge->_mixer->isSoundHandleActive(_soundCache[a].handle)) {
			g_sludge->_mixer->stopHandle(_soundCache[a].handle);
			if (_soundCache[a].inSoundList)
				handleSoundLists();
		}
	}

	_soundCache[a].inSoundList = false;
	_soundCache[a].looping = false;
	_soundCache[a].fileLoaded = -1;

	_silenceIKillYou = false;
}

void SoundManager::huntKillFreeSound(int filenum) {
	if (!_soundOK)
		return;
	int gotSlot = findInSoundCache(filenum);
	if (gotSlot == -1)
		return;
	freeSound(gotSlot);
}

/*
 * Loading and playing:
 */
bool SoundManager::playMOD(int f, int a, int fromTrack) {
	if (!_soundOK)
		return true;
	stopMOD(a);

	// load sound
	setResourceForFatal(f);
	uint length = g_sludge->_resMan->openFileFromNum(f);
	if (length == 0) {
		g_sludge->_resMan->finishAccess();
		setResourceForFatal(-1);
		return false;
	}

	// make audio stream
	Common::SeekableReadStream *readStream = g_sludge->_resMan->getData();
	Common::SeekableReadStream *memImage = readStream->readStream(length);

// debug output
#if 0
	Common::DumpFile *dump = new Common::DumpFile();
	Common::String name = Common::String::format("mod_sound_%i", f);
	dump->open(name);
	byte *soundData = new byte[length];
	memImage->read(soundData, length);
	dump->write(soundData, length);
	dump->finalize();
	delete []soundData;
	delete dump;
	memImage->seek(0, SEEK_SET);
#endif

	if (memImage->size() != (int)length || readStream->err()) {
		return fatal("Sound reading failed");
	}
	Audio::AudioStream *stream = Audio::makeModXmS3mStream(memImage, DisposeAfterUse::NO);

	if (stream) {
		// play sound
		_modCache[a].fileLoaded = f;
		_modCache[a].vol = _defVol;
		g_sludge->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_modCache[a].handle, stream, -1, _modCache[a].vol);
	} else {
		_modCache[a].fileLoaded = -1;
	}

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);
	return true;
}

bool SoundManager::stillPlayingSound(int ch) {
	if (_soundOK)
		if (ch != -1)
			if (_soundCache[ch].fileLoaded != -1)
				if (g_sludge->_mixer->isSoundHandleActive(_soundCache[ch].handle))
					return true;
	return false;
}

bool SoundManager::forceRemoveSound() {
	for (int a = 0; a < MAX_SAMPLES; a++) {
		if (_soundCache[a].fileLoaded != -1) {
			freeSound(a);
			return 1;
		}
	}
	return 0;
}

int SoundManager::findEmptySoundSlot() {
	for (int t = 0; t < MAX_SAMPLES; t++) {
		_emptySoundSlot++;
		_emptySoundSlot %= MAX_SAMPLES;
		if (!g_sludge->_mixer->isSoundHandleActive(_soundCache[_emptySoundSlot].handle) && !_soundCache[_emptySoundSlot].inSoundList)
			return _emptySoundSlot;
	}

	// Argh! They're all playing! Let's trash the oldest that's not looping...

	for (int t = 0; t < MAX_SAMPLES; t++) {
		_emptySoundSlot++;
		_emptySoundSlot %= MAX_SAMPLES;
		if (!_soundCache[_emptySoundSlot].looping && !_soundCache[_emptySoundSlot].inSoundList)
			return _emptySoundSlot;
	}

	// Holy crap, they're all looping! What's this twat playing at?

	_emptySoundSlot++;
	_emptySoundSlot %= MAX_SAMPLES;
	return _emptySoundSlot;
}

int SoundManager::cacheSound(int f) {
	return 0; // don't load source in advance
}

int SoundManager::makeSoundAudioStream(int f, Audio::AudioStream *&audiostream, bool loopy) {
	if (!_soundOK)
		return -1;

	int a = findInSoundCache(f);
	if (a == -1) {
		if (f == -2)
			return -1;
		a = findEmptySoundSlot();
	}
	freeSound(a);

	setResourceForFatal(f);
	uint32 length = g_sludge->_resMan->openFileFromNum(f);
	if (!length)
		return -1;

	Common::SeekableReadStream *readStream = g_sludge->_resMan->getData();
	uint curr_ptr = readStream->pos();
	Audio::RewindableAudioStream *stream = Audio::makeWAVStream(readStream->readStream(length), DisposeAfterUse::NO);

#ifdef USE_VORBIS
	if (!stream) {
		readStream->seek(curr_ptr);
		stream = Audio::makeVorbisStream(readStream->readStream(length), DisposeAfterUse::NO);
	}
#endif
	g_sludge->_resMan->finishAccess();

	if (stream) {
		audiostream = Audio::makeLoopingAudioStream(stream, loopy ? 0 : 1);
		_soundCache[a].fileLoaded = f;
		_soundCache[a].looping = loopy;
		setResourceForFatal(-1);
	} else {
		audiostream = nullptr;
		warning(ERROR_SOUND_ODDNESS);
		_soundCache[a].fileLoaded = -1;
		_soundCache[a].looping = false;
		return -1;
	}

	return a;
}

bool SoundManager::startSound(int f, bool loopy) {
	if (_soundOK) {
		// Load sound
		Audio::AudioStream *stream = nullptr;
		int a = makeSoundAudioStream(f, stream, loopy);
		if (a == -1) {
			warning("Failed to cache sound!");
			return false;
		}

		// play sound
		_soundCache[a].looping = loopy;
		_soundCache[a].vol = _defSoundVol;
		g_sludge->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundCache[a].handle, stream, -1, _soundCache[a].vol);
	}
	return true;
}

void SoundManager::saveSounds(Common::WriteStream *stream) {
	if (_soundOK) {
		for (int i = 0; i < MAX_SAMPLES; i++) {
			if (_soundCache[i].looping) {
				stream->writeByte(1);
				stream->writeUint16BE(_soundCache[i].fileLoaded);
				stream->writeUint16BE(_soundCache[i].vol);
			}
		}
	}
	stream->writeByte(0);
	stream->writeUint16BE(_defSoundVol);
	stream->writeUint16BE(_defVol);
}

void SoundManager::loadSounds(Common::SeekableReadStream *stream) {
	for (int i = 0; i < MAX_SAMPLES; i++)
		freeSound(i);

	while (stream->readByte()) {
		int fileLoaded = stream->readUint16BE();
		_defSoundVol = stream->readUint16BE();
		startSound(fileLoaded, 1);
	}

	_defSoundVol = stream->readUint16BE();
	_defVol = stream->readUint16BE();
}

bool SoundManager::getSoundCacheStack(StackHandler *sH) {
	Variable newFileHandle;
	newFileHandle.varType = SVT_NULL;

	for (int a = 0; a < MAX_SAMPLES; a++) {
		if (_soundCache[a].fileLoaded != -1) {
			newFileHandle.setVariable(SVT_FILE, _soundCache[a].fileLoaded);
			if (!addVarToStackQuick(newFileHandle, sH->first))
				return false;
			if (sH->last == NULL)
				sH->last = sH->first;
		}
	}
	return true;
}

bool SoundManager::deleteSoundFromList(SoundList*&s) {
	// Don't delete a playing sound.
	if (s->cacheIndex)
		return false;

	SoundList*o = NULL;
	if (!s->next) {
		o = s->prev;
		if (o)
			o->next = NULL;
		delete s;
		s = o;
		return (s != NULL);
	}
	if (s != s->next) {
		o = s->next;
		o->prev = s->prev;
		if (o->prev)
			o->prev->next = o;
	}
	delete s;
	s = o;
	return (s != NULL);
}

void SoundManager::handleSoundLists() {
	if (_isHandlingSoundList)
		return;
	_isHandlingSoundList = true;
	for (SoundListHandles::iterator it = _soundListHandles.begin(); it != _soundListHandles.end(); ++it) {
		SoundList*s = (*it);
		int a = s->cacheIndex;
		bool remove = false;
		if (!g_sludge->_mixer->isSoundHandleActive(_soundCache[a].handle)) { // reach the end of stream
			s->cacheIndex = false;
			_soundCache[a].inSoundList = false;
			if (_silenceIKillYou) {
				while (deleteSoundFromList(s))
					;
				remove = (s == NULL); // s not null if still playing
			} else {
				if (s->next) {
					if (s->next == s) { // loop the same sound
						int v = _defSoundVol;
						_defSoundVol = _soundCache[a].vol;
						startSound(s->sound, true);
						_defSoundVol = v;
						while (deleteSoundFromList(s))
							;
						remove = (s == NULL); // s not null if still playing
					} else { // repush the next sound list
						s->next->vol = _soundCache[a].vol;
						playSoundList(s->next);
						remove = true; // remove this one
					}

				} else {
					while (deleteSoundFromList(s))
						;
					remove = (s == NULL); // s not null if still playing
				}
			}
		}
		if (remove) {
			it = _soundListHandles.reverse_erase(it);
		}
	}
	_isHandlingSoundList = false;
}

// loop a list of sound
void SoundManager::playSoundList(SoundList*s) {
	if (_soundOK) {
		// Load sound
		Audio::AudioStream *stream;
		int a = makeSoundAudioStream(s->sound, stream, false);
		if (a == -1) {
			warning("Failed to cache sound!");
			return;
		}

		// Play sound
		_soundCache[a].looping = false;
		if (s->vol < 0)
			_soundCache[a].vol = _defSoundVol;
		else
			_soundCache[a].vol = s->vol;
		s-> cacheIndex = a;
		g_sludge->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundCache[a].handle, stream, -1, _soundCache[a].vol);
		_soundCache[a].inSoundList = true;

		// push sound list
		_soundListHandles.push_back(s);

	}
}

void playMovieStream(int a) {
#if 0
	if (! soundOK) return;
	ALboolean ok;
	ALuint src;

	alGenSources(1, &src);
	if (alGetError() != AL_NO_ERROR) {
		debugOut("Failed to create OpenAL source!\n");
		return;
	}

	alSourcef(src, AL_GAIN, (float) soundCache[a].vol / 256);

	ok = alurePlaySourceStream(src, soundCache[a].stream,
			10, 0, sound_eos_callback, &intpointers[a]);
	if (!ok) {
		debugOut("Failed to play stream: %s\n", alureGetErrorString());
		alDeleteSources(1, &src);
		if (alGetError() != AL_NO_ERROR) {
			debugOut("Failed to delete OpenAL source!\n");
		}

		soundCache[a].playingOnSource = 0;
	} else {
		soundCache[a].playingOnSource = src;
		soundCache[a].playing = true;
	}
#endif
}

#if 0
int initMovieSound(int f, ALenum format, int audioChannels, ALuint samplerate,
		ALuint(*callback)(void *userdata, ALubyte *data, ALuint bytes)) {
	if (! soundOK) return 0;

	int retval;
	int a = findEmptySoundSlot();
	freeSound(a);

	soundCache[a].looping = false;
#if 0
	// audioChannel * sampleRate gives us a buffer of half a second. Not much, but it should be enough.
	soundCache[a].stream = alureCreateStreamFromCallback(
			callback,
			&intpointers[a], format, samplerate,
			audioChannels * samplerate, 0, NULL);
#endif
	if (soundCache[a].stream != NULL) {
		soundCache[a].fileLoaded = f;
		soundCache[a].vol = defSoundVol;
		retval = a;
	} else {
#if 0
		debugOut("Failed to create stream from sound: %s\n",
				alureGetErrorString());
#endif
		warning(ERROR_SOUND_ODDNESS);
		soundCache[a].stream = NULL;
		soundCache[a].playing = false;
		soundCache[a].playingOnSource = 0;
		soundCache[a].fileLoaded = -1;
		retval = -1;
	}
	//fprintf (stderr, "Stream %d created. Sample rate: %d Channels: %d\n", retval, samplerate, audioChannels);

	return retval;
}
#endif

uint SoundManager::getSoundSource(int index) {
	warning("getSoundSource, Unimplemented");
	return 0; /*soundCache[index].playingOnSource;*/
}

} // End of namespace Sludge
