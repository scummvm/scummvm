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
#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/vorbis.h"
#include "audio/mods/protracker.h"

#include "sludge/allfiles.h"
#include "sludge/newfatal.h"
#include "sludge/sound.h"
#include "sludge/moreio.h"
#include "sludge/fileset.h"
#include "sludge/sludge.h"

#define MAX_SAMPLES 8
#define MAX_MODS 3
#define NUM_BUFS 3

namespace Sludge {

bool soundOK = false;
bool SilenceIKillYou = false;
bool isHandlingSoundList = false;
// there's possibility that several sound list played at the same time
typedef Common::List<SoundList*> SoundListHandles;
SoundListHandles soundListHandles;

struct soundThing {
	Audio::SoundHandle handle;
	int fileLoaded, vol;    //Used for sounds only. (sound saving/loading)
	bool looping;      		//Used for sounds only. (sound saving/loading)
	bool inSoundList;
};

soundThing soundCache[MAX_SAMPLES];
#if 0
soundThing modCache[MAX_MODS];
#endif

int defVol = 128;
int defSoundVol = 255;
const float modLoudness = 0.95f;

/*
 * Set up, tear down:
 */

bool initSoundStuff(HWND hwnd) {
	for (int a = 0; a < MAX_SAMPLES; a ++) {
		soundCache[a].fileLoaded = -1;
		soundCache[a].looping = false;
		soundCache[a].inSoundList = false;
	}
#if 0
	for (int a = 0; a < MAX_MODS; a ++) {
		modCache[a].stream = NULL;
		modCache[a].playing = false;
	}
#endif
	return soundOK = true;
}

void killSoundStuff() {
	if (!soundOK)
		return;

	SilenceIKillYou = true;
	for (int i = 0; i < MAX_SAMPLES; i ++) {
		if (g_sludge->_mixer->isSoundHandleActive(soundCache[i].handle)) {
			g_sludge->_mixer->stopHandle(soundCache[i].handle);
		}
	}
#if 0
	for (int i = 0; i < MAX_MODS; i ++) {
		if (modCache[i].playing) {

			if (! alureStopSource(modCache[i].playingOnSource, AL_TRUE)) {
				debugOut("Failed to stop source: %s\n",
						alureGetErrorString());
			}

		}

		if (modCache[i].stream != NULL) {

			if (! alureDestroyStream(modCache[i].stream, 0, NULL)) {
				debugOut("Failed to destroy stream: %s\n",
						alureGetErrorString());
			}

		}
	}
#endif
	SilenceIKillYou = false;
}

/*
 * Some setters:
 */

void setMusicVolume(int a, int v) {
	if (!soundOK)
		return;
#if 0
	if (modCache[a].playing) {
		alSourcef(modCache[a].playingOnSource, AL_GAIN, (float) modLoudness * v / 256);
	}
#endif
}

void setDefaultMusicVolume(int v) {
	defVol = v;
}

void setSoundVolume(int a, int v) {
	if (!soundOK)
		return;
	int ch = findInSoundCache(a);
	if (ch != -1) {
		if (g_sludge->_mixer->isSoundHandleActive(soundCache[ch].handle)) {
			soundCache[ch].vol = v;
			g_sludge->_mixer->setChannelVolume(soundCache[ch].handle, v);
		}
	}
}

void setDefaultSoundVolume(int v) {
	defSoundVol = v;
}

void setSoundLoop(int a, int s, int e) {
//#pragma unused (a,s,e)
}

/*
 * Stopping things:
 */

int findInSoundCache(int a) {
	int i;
	for (i = 0; i < MAX_SAMPLES; i++) {
		if (soundCache[i].fileLoaded == a) {
			return i;
		}
	}
	return -1;
}

void stopMOD(int i) {
	if (!soundOK)
		return;
#if 0
	alGetError();
	if (modCache[i].playing) {
		if (! alureStopSource(modCache[i].playingOnSource, AL_TRUE)) {
			debugOut("Failed to stop source: %s\n", alureGetErrorString());
		}
	}
#endif
}

void huntKillSound(int filenum) {
	if (!soundOK)
		return;

	int gotSlot = findInSoundCache(filenum);
	if (gotSlot == -1) return;

	SilenceIKillYou = true;

	if (g_sludge->_mixer->isSoundHandleActive(soundCache[gotSlot].handle)) {
		g_sludge->_mixer->stopHandle(soundCache[gotSlot].handle);
	}

	SilenceIKillYou = false;
}

void freeSound(int a) {
	if (!soundOK)
		return;

	SilenceIKillYou = true;

	if (g_sludge->_mixer->isSoundHandleActive(soundCache[a].handle)) {
		g_sludge->_mixer->stopHandle(soundCache[a].handle);
		if (soundCache[a].inSoundList)
			handleSoundLists();
	}

	soundCache[a].fileLoaded = -1;

	SilenceIKillYou = false;
}

void huntKillFreeSound(int filenum) {
	if (!soundOK)
		return;
	int gotSlot = findInSoundCache(filenum);
	if (gotSlot == -1)
		return;
	freeSound(gotSlot);
}

/*
 * Loading and playing:
 */
bool playMOD(int f, int a, int fromTrack) {
#if 0
	// load sound
	setResourceForFatal(f);
	uint32 length = openFileFromNum(f);
	if (length == 0) {
		finishAccess();
		setResourceForFatal(-1);
		return false;
	}

	Common::SeekableReadStream *memImage = bigDataFile->readStream(length);
	if (memImage->size() != length || bigDataFile->err())
		debug("Sound reading failed");
	Audio::AudioStream *stream = Audio::makeProtrackerStream(memImage);
	//TODO: replace by xm file decoders
	if (!stream)
		return false;

	// play sound
	Audio::SoundHandle soundHandle;
	g_sludge->_mixer->playStream(Audio::Mixer::kSFXSoundType, &soundHandle,
			stream, -1, Audio::Mixer::kMaxChannelVolume);
#endif
#if 0
	if (!soundOK)
		return true;
	stopMOD(a);

	setResourceForFatal(f);
	uint32 length = openFileFromNum(f);
	if (length == 0) {
		finishAccess();
		setResourceForFatal(-1);
		return false;
	}

	byte *memImage;
	memImage = (byte *) loadEntireFileToMemory(bigDataFile, length);
	if (! memImage) return fatal(ERROR_MUSIC_MEMORY_LOW);

	modCache[a].stream = alureCreateStreamFromMemory(memImage, length, 19200, 0, NULL);

	delete memImage;

	if (modCache[a].stream != NULL) {
		setMusicVolume(a, defVol);

		if (! alureSetStreamOrder(modCache[a].stream, fromTrack)) {
			debugOut("Failed to set stream order: %s\n",
					alureGetErrorString());
		}

		playStream(a, true, true);

	} else {

		debugOut("Failed to create stream from MOD: %s\n",
				alureGetErrorString());

		warning(ERROR_MUSIC_ODDNESS);
		soundCache[a].stream = NULL;
		soundCache[a].playing = false;
		soundCache[a].playingOnSource = 0;
	}
	setResourceForFatal(-1);
#endif
	return true;
}

bool stillPlayingSound(int ch) {
	if (soundOK)
		if (ch != -1)
			if (soundCache[ch].fileLoaded != -1)
				if (g_sludge->_mixer->isSoundHandleActive(soundCache[ch].handle))
					return true;

	return false;
}

bool forceRemoveSound() {
	for (int a = 0; a < MAX_SAMPLES; a++) {
		if (soundCache[a].fileLoaded != -1 && !stillPlayingSound(a)) {
//			soundWarning ("Deleting silent sound", a);
			freeSound(a);
			return 1;
		}
	}

	for (int a = 0; a < MAX_SAMPLES; a++) {
		if (soundCache[a].fileLoaded != -1) {
//			soundWarning ("Deleting playing sound", a);
			freeSound(a);
			return 1;
		}
	}
//	soundWarning ("Cache is empty!", 0);
	return 0;
}

int emptySoundSlot = 0;

int findEmptySoundSlot() {
	for (int t = 0; t < MAX_SAMPLES; t++) {
		emptySoundSlot++;
		emptySoundSlot %= MAX_SAMPLES;
		if (!g_sludge->_mixer->isSoundHandleActive(soundCache[emptySoundSlot].handle) && !soundCache[emptySoundSlot].inSoundList)
			return emptySoundSlot;
	}

	// Argh! They're all playing! Let's trash the oldest that's not looping...

	for (int t = 0; t < MAX_SAMPLES; t++) {
		emptySoundSlot++;
		emptySoundSlot %= MAX_SAMPLES;
		if (!soundCache[emptySoundSlot].looping && !soundCache[emptySoundSlot].inSoundList)
			return emptySoundSlot;
	}

	// Holy crap, they're all looping! What's this twat playing at?

	emptySoundSlot++;
	emptySoundSlot %= MAX_SAMPLES;
	return emptySoundSlot;
}

int cacheSound(int f) {
	return 0; // don't load source in advance
}

int makeSoundAudioStream(int f, Audio::AudioStream *&audiostream, bool loopy) {
	if (!soundOK)
		return -1;

	int a = findInSoundCache(f);
	if (a != -1) { // if this sound has been loaded before
		// still playing
		if (g_sludge->_mixer->isSoundHandleActive(soundCache[a].handle)) {
			g_sludge->_mixer->stopHandle(soundCache[a].handle); // stop it
			if (soundCache[a].inSoundList) {
				handleSoundLists();
			}
		}
	} else {
		if (f == -2)
			return -1;
		a = findEmptySoundSlot();
		freeSound(a);
	}

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
		soundCache[a].fileLoaded = f;
		setResourceForFatal(-1);
	} else {
		audiostream = nullptr;
		warning(ERROR_SOUND_ODDNESS);
		soundCache[a].fileLoaded = -1;
		soundCache[a].looping = false;
		return -1;
	}

	return a;
}

bool startSound(int f, bool loopy) {
	if (soundOK) {
		// Load sound
		Audio::AudioStream *stream = nullptr;
		int a = makeSoundAudioStream(f, stream, loopy);
		if (a == -1) {
			warning("Failed to cache sound!");
			return false;
		}

		// play sound
		soundCache[a].looping = loopy;
		soundCache[a].vol = defSoundVol;
		g_sludge->_mixer->playStream(Audio::Mixer::kSFXSoundType, &soundCache[a].handle, stream, -1, soundCache[a].vol);
	}
	return true;
}

void saveSounds(Common::WriteStream *stream) {
	if (soundOK) {
		for (int i = 0; i < MAX_SAMPLES; i++) {
			if (soundCache[i].looping) {
				stream->writeByte(1);
				stream->writeUint16BE(soundCache[i].fileLoaded);
				stream->writeUint16BE(soundCache[i].vol);
			}
		}
	}
	stream->writeByte(0);
	stream->writeUint16BE(defSoundVol);
	stream->writeUint16BE(defVol);
}

void loadSounds(Common::SeekableReadStream *stream) {
	for (int i = 0; i < MAX_SAMPLES; i++)
		freeSound(i);

	while (stream->readByte()) {
		int fileLoaded = stream->readUint16BE();
		defSoundVol = stream->readUint16BE();
		startSound(fileLoaded, 1);
	}

	defSoundVol = stream->readUint16BE();
	defVol = stream->readUint16BE();
}

bool getSoundCacheStack(StackHandler *sH) {
	Variable newFileHandle;
	newFileHandle.varType = SVT_NULL;

	for (int a = 0; a < MAX_SAMPLES; a++) {
		if (soundCache[a].fileLoaded != -1) {
			setVariable(newFileHandle, SVT_FILE, soundCache[a].fileLoaded);
			if (!addVarToStackQuick(newFileHandle, sH->first))
				return false;
			if (sH->last == NULL)
				sH->last = sH->first;
		}
	}
	return true;
}

bool deleteSoundFromList(SoundList*&s) {
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

void handleSoundLists() {
	if (isHandlingSoundList)
		return;
	isHandlingSoundList = true;
	for (SoundListHandles::iterator it = soundListHandles.begin(); it != soundListHandles.end(); ++it) {
		SoundList*s = (*it);
		int a = s->cacheIndex;
		bool remove = false;
		if (!g_sludge->_mixer->isSoundHandleActive(soundCache[a].handle)) { // reach the end of stream
			s->cacheIndex = false;
			soundCache[a].inSoundList = false;
			if (SilenceIKillYou) {
				while (deleteSoundFromList(s))
					;
				remove = (s == NULL); // s not null if still playing
			} else {
				if (s->next) {
					if (s->next == s) { // loop the same sound
						int v = defSoundVol;
						defSoundVol = soundCache[a].vol;
						startSound(s->sound, true);
						defSoundVol = v;
						while (deleteSoundFromList(s))
							;
						remove = (s == NULL); // s not null if still playing
					} else { // repush the next sound list
						s->next->vol = soundCache[a].vol;
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
			it = soundListHandles.reverse_erase(it);
		}
	}
	isHandlingSoundList = false;
}

// loop a list of sound
void playSoundList(SoundList*s) {
	if (soundOK) {
		// Load sound
		Audio::AudioStream *stream;
		int a = makeSoundAudioStream(s->sound, stream, false);
		if (a == -1) {
			warning("Failed to cache sound!");
			return;
		}

		// Play sound
		soundCache[a].looping = false;
		if (s->vol < 0)
			soundCache[a].vol = defSoundVol;
		else
			soundCache[a].vol = s->vol;
		s-> cacheIndex = a;
		g_sludge->_mixer->playStream(Audio::Mixer::kSFXSoundType, &soundCache[a].handle, stream, -1, soundCache[a].vol);
		soundCache[a].inSoundList = true;

		// push sound list
		soundListHandles.push_back(s);

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

uint getSoundSource(int index) {
	return 0; /*soundCache[index].playingOnSource;*/ //TODO:false value
}

} // End of namespace Sludge
