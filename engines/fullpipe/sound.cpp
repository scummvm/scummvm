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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/scene.h"
#include "fullpipe/sound.h"
#include "fullpipe/ngiarchive.h"
#include "fullpipe/messages.h"
#include "fullpipe/statics.h"

#include "common/memstream.h"
#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

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

Sound *SoundList::getSoundItemById(int id) {
	if (_soundItemsCount == 0) {
		return _soundItems[0]->getId() != id ? 0 : _soundItems[0];
	}

	for (int i = 0; i < _soundItemsCount; i++) {
		if (_soundItems[i]->getId() == id)
			return _soundItems[i];
	}
	return NULL;
}

Sound::Sound() {
	_id = 0;
	_directSoundBuffer = 0;
	_soundData = 0;
	_objectId = 0;
	memset(_directSoundBuffers, 0, sizeof(_directSoundBuffers));
	_description = 0;
	_volume = 100;
}

Sound::~Sound() {
	freeSound();

	free(_description);
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
	if (!_objectId)
		return;

	StaticANIObject *ani = g_fp->_currentScene->getStaticANIObject1ById(_objectId, -1);
	if (!ani)
		return;

	int a, b;

	if (ani->_ox >= g_fp->_sceneRect.left) {
		int par, pan;

		if (ani->_ox <= g_fp->_sceneRect.right) {
			int dx;

			if (ani->_oy <= g_fp->_sceneRect.bottom) {
				if (ani->_oy >= g_fp->_sceneRect.top) {
					setPanAndVolume(g_fp->_sfxVolume, 0);

					return;
				}
				dx = g_fp->_sceneRect.top - ani->_oy;
			} else {
				dx = ani->_oy - g_fp->_sceneRect.bottom;
			}

		    par = 0;

			if (dx > 800) {
				setPanAndVolume(-3500, 0);
				return;
			}

			pan = -3500;
			a = g_fp->_sfxVolume - (-3500);
			b = 800 - dx;
		} else {
			int dx = ani->_ox - g_fp->_sceneRect.right;

			if (dx > 800) {
				setPanAndVolume(-3500, 0);
				return;
			}

			pan = -3500;
			par = dx * (-3500) / -800;
			a = g_fp->_sfxVolume - (-3500);
			b = 800 - dx;
		}

		int32 pp = b * a;

		setPanAndVolume(pan + pp / 800, par);

		return;
	}

	int dx = g_fp->_sceneRect.left - ani->_ox;
	if (dx <= 800) {
		int32 s = (800 - dx) * (g_fp->_sfxVolume - (-3500));
		int32 p = -3500 + s / 800;

		if (p > g_fp->_sfxVolume)
			p = g_fp->_sfxVolume;

		setPanAndVolume(p, dx * (-3500) / 800);
	} else {
		setPanAndVolume(-3500, 0);
	}
}

void Sound::setPanAndVolume(int vol, int pan) {
	g_fp->_mixer->setChannelVolume(_handle, vol / 39); // 0..10000
	g_fp->_mixer->setChannelBalance(_handle, pan / 78); // -10000..10000
}

void Sound::play(int flag) {
	Audio::SoundHandle handle = getHandle();

	if (g_fp->_mixer->isSoundHandleActive(handle))
		return;

	byte *soundData = loadData();
	Common::MemoryReadStream *dataStream = new Common::MemoryReadStream(soundData, getDataSize());
	Audio::RewindableAudioStream *wav = Audio::makeWAVStream(dataStream, DisposeAfterUse::YES);
	Audio::AudioStream *audioStream = new Audio::LoopingAudioStream(wav, (flag == 1) ? 0 : 1);

	g_fp->_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, audioStream);
}

void Sound::freeSound() {
	stop();

	free(_soundData);
}

int Sound::getVolume() {
	return g_fp->_mixer->getChannelVolume(_handle) * 39;  // 0..10000
}

void Sound::stop() {
	g_fp->_mixer->stopHandle(_handle);
}

void FullpipeEngine::setSceneMusicParameters(GameVar *gvar) {
	warning("STUB: FullpipeEngine::setSceneMusicParameters()");
#if 0
	stopSoundStream2();

	if (soundStream3)
		FSOUND_Stream_Stop(soundStream4);
#endif

	if (_musicLocal)
		stopAllSoundStreams();

	GameVar *var = gvar->getSubVarByName("MUSIC");

	memset(_sceneTracks, 0, sizeof(_sceneTracks));

	_numSceneTracks = 0;
	_sceneTrackHasSequence = false;

	if (!var)
		return;

	_musicGameVar = var;

	GameVar *tr = var->getSubVarByName("TRACKS");
	if (tr) {
		GameVar *sub = tr->_subVars;

		while (sub) {
			if (_musicAllowed & sub->_value.intValue) {
				Common::strlcpy(_sceneTracks[_numSceneTracks], sub->_varName, 260);

				_numSceneTracks++;
			}

			sub = sub->_nextVarObj;
		}
	}

	_musicMinDelay = var->getSubVarAsInt("MINDELAY");
	_musicMaxDelay = var->getSubVarAsInt("MAXDELAY");
	_musicLocal = var->getSubVarAsInt("LOCAL");

	GameVar *seq = var->getSubVarByName("SEQUENCE");

	if (seq) {
		_sceneTrackHasSequence = true;

		Common::strlcpy(_trackName, seq->_value.stringValue, 2600);
	}

	if (_musicLocal)
		stopAllSoundStreams();

	if (!_sceneTrackIsPlaying || _musicLocal)
		_trackStartDelay = var->getSubVarAsInt("STARTDELAY");
}

void FullpipeEngine::startSceneTrack() {
	if (!_sceneTrackIsPlaying && _numSceneTracks > 0) {
		if (_trackStartDelay > 0) {
			_trackStartDelay--;
		} else {
			int trackNum = getSceneTrack();

			if (trackNum == -1) {
				strcpy(_sceneTracksCurrentTrack, "silence");

				_trackStartDelay = 2880;
				_sceneTrackIsPlaying = 0;
			} else {
				strcpy(_sceneTracksCurrentTrack, _sceneTracks[trackNum]);

				startSoundStream1(_sceneTracksCurrentTrack);

				_sceneTrackIsPlaying = true;
			}
		}
	}
}

int FullpipeEngine::getSceneTrack() {
	int res;

	if (_sceneTrackHasSequence) {
		int num = _musicGameVar->getSubVarAsInt("TRACKS");

		if (_trackName[num + 1] == 's') { // 'silence'
			res = -1;
		} else {
			res = _trackName[num + 1] - '0';

			if (res < 0 || res >= _numSceneTracks)
				res = 0;
		}

		int track = num + 1;

		if (!_trackName[num + 2])
			track = 0;

		_musicGameVar->setSubVarAsInt("TRACKS", track);
	} else {
		res = _numSceneTracks * (_updateTicks % 10) / 10;
	}

	return res;
}

void FullpipeEngine::startSoundStream1(char *trackName) {
	warning("STUB: FullpipeEngine::startSoundStream1(%s)", trackName);

	stopAllSoundStreams();

#ifdef USE_VORBIS
	if (_mixer->isSoundHandleActive(_sceneTrackHandle))
		return;

	Common::File *track = new Common::File();
	if (!track->open(trackName)) {
		warning("Could not open %s", trackName);
		delete track;
		return;
	}
	Audio::RewindableAudioStream *ogg = Audio::makeVorbisStream(track, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_sceneTrackHandle, ogg);
#endif
}

void FullpipeEngine::stopAllSounds() {
	// _mixer->stopAll();

	for (int i = 0; i < _currSoundListCount; i++)
		for (int j = 0; j < _currSoundList1[i]->getCount(); j++) {
			_currSoundList1[i]->getSoundByIndex(j)->stop();
		}
}

void FullpipeEngine::toggleMute() {
	if (_soundEnabled) {
		_sfxVolume = _sfxVolume != -10000 ? -10000 : 0;

		updateSoundVolume();
	}
}

void FullpipeEngine::playSound(int id, int flag) {
	Sound *sound = 0;

	for (int i = 0; i < _currSoundListCount; i++) {
		sound = _currSoundList1[i]->getSoundItemById(id);

		if (sound)
			break;
	}

	if (!sound) {
		warning("playSound: Can't find sound with ID %d", id);
		return;
	}

	sound->play(flag);
}

void FullpipeEngine::playTrack(GameVar *sceneVar, const char *name, bool delayed) {
	warning("STUB: FullpipeEngine::playTrack(var, %s, %d)", name, delayed);
#if 0
	stopSoundStream2();

	if (soundStream3)
		FSOUND_Stream_Stop(soundStream4);
#endif

	if (_musicLocal)
		stopAllSoundStreams();

	GameVar *var = sceneVar->getSubVarByName(name);

	memset(_sceneTracks, 0, sizeof(_sceneTracks));

	_numSceneTracks = 0;
	_sceneTrackHasSequence = false;

	if (!var)
		return;

	_musicGameVar = var;

	GameVar *tr = var->getSubVarByName("TRACKS");
	if (tr) {
		GameVar *sub = tr->_subVars;

		while (sub) {
			if (_musicAllowed & sub->_value.intValue) {
				Common::strlcpy(_sceneTracks[_numSceneTracks], sub->_varName, 260);

				_numSceneTracks++;
			}

			sub = sub->_nextVarObj;
		}
	}

	_musicMinDelay = var->getSubVarAsInt("MINDELAY");
	_musicMaxDelay = var->getSubVarAsInt("MAXDELAY");
	_musicLocal = var->getSubVarAsInt("LOCAL");

	GameVar *seq = var->getSubVarByName("SEQUENCE");

	if (seq) {
		_sceneTrackHasSequence = true;

		Common::strlcpy(_trackName, seq->_value.stringValue, 2600);
	}

	if (delayed) {
		if (_sceneTrackIsPlaying && _numSceneTracks == 1) {
			if (strcmp(_sceneTracksCurrentTrack, _sceneTracks[0]))
				stopAllSoundStreams();
		}

		_trackStartDelay = var->getSubVarAsInt("STARTDELAY");
	}
}

void global_messageHandler_handleSound(ExCommand *cmd) {
	if (!g_fp->_soundEnabled)
		return;

	Sound *snd = 0;

	for (int i = 0; i < g_fp->_currSoundListCount; i++)
		snd = g_fp->_currSoundList1[i]->getSoundItemById(cmd->_messageNum);

	if (!snd)
		return;

	if (cmd->_field_14 & 1) {
		if (!g_fp->_flgSoundList && (cmd->_field_14 & 4))
			snd->freeSound();

		snd->updateVolume();

		if (snd->_objectId && g_fp->_currentScene->getStaticANIObject1ById(snd->_objectId, -1))
			snd->setPanAndVolumeByStaticAni();
		else
			snd->setPanAndVolume(g_fp->_sfxVolume, 0);

		if (snd->getVolume() > -3500)
			snd->play(cmd->_keyCode);
	} else if (cmd->_field_14 & 2) {
		snd->stop();
	}
}

void FullpipeEngine::stopSoundStream2() {
	warning("STUB: FullpipeEngine::stopSoundStream2()");
}

void FullpipeEngine::stopAllSoundStreams() {
	// TODO: Differences from stopAllSounds()
	_mixer->stopAll();
}

void FullpipeEngine::stopAllSoundInstances(int id) {
	for (int i = 0; i < _currSoundListCount; i++) {
		Sound *sound = _currSoundList1[i]->getSoundItemById(id);

		if (sound)
			sound->stop();
	}
}

void FullpipeEngine::updateSoundVolume() {
	for (int i = 0; i < _currSoundListCount; i++)
		for (int j = 0; i < _currSoundList1[i]->getCount(); j++) {
			_currSoundList1[i]->getSoundByIndex(j)->setPanAndVolume(_sfxVolume, 0);
		}
}

void FullpipeEngine::setMusicVolume(int vol) {
	_musicVolume = vol;

	g_fp->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
}

} // End of namespace Fullpipe
