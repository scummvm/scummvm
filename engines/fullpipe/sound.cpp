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

#include "common/config-manager.h"
#include "common/memstream.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

namespace Fullpipe {

bool SoundList::load(MfcArchive &file, const Common::String &fname) {
	debugC(5, kDebugLoading, "SoundList::load()");

	uint32 count = file.readUint32LE();
	_soundItems.resize(count);

	if (!fname.empty()) {
		_libHandle.reset(makeNGIArchive(fname));
	} else {
		_libHandle.reset();
	}

	for (uint i = 0; i < count; i++) {
		_soundItems[i].load(file, _libHandle.get());
	}

	return true;
}

bool SoundList::loadFile(const Common::String &fname, const Common::String &libname) {
	Common::File file;

	if (!file.open(fname))
		return false;

	MfcArchive archive(&file);

	return load(archive, libname);
}

Sound *SoundList::getSoundItemById(int id) {
	for (uint i = 0; i < _soundItems.size(); ++i) {
		if (_soundItems[i].getId() == id)
			return &_soundItems[i];
	}
	return nullptr;
}

Sound::Sound() :
	_id(0),
	_directSoundBuffer(0),
	_directSoundBuffers(),
	_soundData(nullptr),
	_handle(new Audio::SoundHandle()),
	_volume(100),
	_objectId(0) {}

Sound::~Sound() {
	freeSound();
	delete _handle;
}

bool Sound::load(MfcArchive &file, NGIArchive *archive) {
	debugC(5, kDebugLoading, "Sound::load()");

	MemoryObject::load(file);

	_id = file.readUint32LE();
	/*_description = */file.readPascalString();

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
	// not needed in our implementation
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
	g_fp->_mixer->setChannelVolume(*_handle, MIN((vol + 10000) / 39, 255)); // -10000..0
	g_fp->_mixer->setChannelBalance(*_handle, CLIP(pan / 78, -127, 127)); // -10000..10000
}

void Sound::play(int flag) {
	Audio::SoundHandle *handle = getHandle();

	if (g_fp->_mixer->isSoundHandleActive(*handle)) { // We need to restart the music
		g_fp->_mixer->stopHandle(*handle);
	}

	byte *soundData = loadData();
	Common::MemoryReadStream *dataStream = new Common::MemoryReadStream(soundData, getDataSize());
	Audio::RewindableAudioStream *wav = Audio::makeWAVStream(dataStream, DisposeAfterUse::YES);
	Audio::AudioStream *audioStream = new Audio::LoopingAudioStream(wav, (flag == 1) ? 0 : 1);

	g_fp->_mixer->playStream(Audio::Mixer::kSFXSoundType, handle, audioStream);
}

void Sound::freeSound() {
	stop();

	free(_soundData);
}

int Sound::getVolume() {
	return g_fp->_mixer->getChannelVolume(*_handle) * 39;  // 0..10000
}

void Sound::stop() {
	g_fp->_mixer->stopHandle(*_handle);
}

void FullpipeEngine::setSceneMusicParameters(GameVar *gvar) {
	stopSoundStream2();

	if (_mixer->isSoundHandleActive(_soundStream3))
		_mixer->stopHandle(_soundStream4);

	if (_musicLocal)
		stopAllSoundStreams();

	GameVar *var = gvar->getSubVarByName("MUSIC");

	for (int i = 0; i < 10; i++)
		_sceneTracks[i].clear();

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
				_sceneTracks[_numSceneTracks] = sub->_varName;

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

		_trackName = seq->_value.stringValue;
	}

	if (_musicLocal)
		stopAllSoundStreams();

	if (!_sceneTrackIsPlaying || _musicLocal)
		_trackStartDelay = var->getSubVarAsInt("STARTDELAY");
}

void FullpipeEngine::updateTrackDelay() {
	_sceneTrackIsPlaying = false;
	_trackStartDelay = _musicMinDelay + (_musicMaxDelay - _musicMinDelay) * (_updateTicks % 10) / 9;
}

void FullpipeEngine::startSceneTrack() {
	if (_sceneTrackIsPlaying) {
		if (!_mixer->isSoundHandleActive(_soundStream1)) { // Simulate end of sound callback
			updateTrackDelay();
		}
	}

	if (!_sceneTrackIsPlaying && _numSceneTracks > 0) {
		if (_trackStartDelay > 0) {
			_trackStartDelay--;
		} else {
			int trackNum = getSceneTrack();

			if (trackNum == -1) {
				_sceneTracksCurrentTrack = "silence";

				_trackStartDelay = 2880;
				_sceneTrackIsPlaying = 0;
			} else {
				_sceneTracksCurrentTrack = _sceneTracks[trackNum];

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

		if (num + 2 >= (int)_trackName.size())
			track = 0;

		_musicGameVar->setSubVarAsInt("TRACKS", track);
	} else {
		res = _numSceneTracks * (_updateTicks % 10) / 10;
	}

	return res;
}

void FullpipeEngine::startSoundStream1(const Common::String &trackName) {
	stopAllSoundStreams();

	playOggSound(trackName, _soundStream1);
}

void FullpipeEngine::playOggSound(const Common::String &trackName, Audio::SoundHandle &stream) {
#ifdef USE_VORBIS
	if (_mixer->isSoundHandleActive(stream))
		return;

	Common::File *track = new Common::File();
	if (!track->open(trackName)) {
		warning("Could not open %s", trackName.c_str());
		delete track;
		return;
	}
	Audio::RewindableAudioStream *ogg = Audio::makeVorbisStream(track, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &stream, ogg);
#endif
}

void FullpipeEngine::stopAllSounds() {
	for (int i = 0; i < _currSoundListCount; i++)
		for (int j = 0; j < _currSoundList1[i]->getCount(); j++) {
			_currSoundList1[i]->getSoundByIndex(j).stop();
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
	if (_mixer->isSoundHandleActive(_soundStream3))
		_mixer->stopHandle(_soundStream4);

	stopSoundStream2();

	if (_musicLocal)
		stopAllSoundStreams();

	GameVar *var = sceneVar->getSubVarByName(name);

	for (int i = 0; i < 10; i++)
		_sceneTracks[i].clear();

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
				_sceneTracks[_numSceneTracks] = sub->_varName;

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

		_trackName = seq->_value.stringValue;
	}

	if (delayed) {
		if (_sceneTrackIsPlaying && _numSceneTracks == 1) {
			if (_sceneTracksCurrentTrack != _sceneTracks[0])
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
		if ((snd = g_fp->_currSoundList1[i]->getSoundItemById(cmd->_messageNum)) != NULL)
			break;

	if (!snd)
		return;

	if (cmd->_z & 1) {
		if (!g_fp->_flgSoundList && (cmd->_z & 4))
			snd->freeSound();

		snd->updateVolume();

		if (snd->_objectId && g_fp->_currentScene->getStaticANIObject1ById(snd->_objectId, -1))
			snd->setPanAndVolumeByStaticAni();
		else
			snd->setPanAndVolume(g_fp->_sfxVolume, 0);

		if (snd->getVolume() > -3500)
			snd->play(cmd->_param);
	} else if (cmd->_z & 2) {
		snd->stop();
	}
}

void FullpipeEngine::stopSoundStream2() {
	_stream2playing = false;

	if (_mixer->isSoundHandleActive(_soundStream3)) {
		_mixer->stopHandle(_soundStream2);
		_mixer->stopHandle(_soundStream3);
	}
}

void FullpipeEngine::stopAllSoundStreams() {
	_mixer->stopHandle(_soundStream1);
	_mixer->stopHandle(_soundStream2);
	_mixer->stopHandle(_soundStream3);
	_mixer->stopHandle(_soundStream4);

	_stream2playing = false;
}

void FullpipeEngine::stopAllSoundInstances(int id) {
	for (int i = 0; i < _currSoundListCount; i++) {
		Sound *sound = _currSoundList1[i]->getSoundItemById(id);

		if (sound)
			sound->stop();
	}
}

void FullpipeEngine::updateSoundVolume() {
	ConfMan.setInt("sfx_volume", MAX((_sfxVolume + 10000) / 39, 255));
	syncSoundSettings();

	for (int i = 0; i < _currSoundListCount; i++)
		for (int j = 0; j < _currSoundList1[i]->getCount(); j++) {
			_currSoundList1[i]->getSoundByIndex(j).setPanAndVolume(_sfxVolume, 0);
		}
}

void FullpipeEngine::setMusicVolume(int vol) {
	_musicVolume = vol;

	ConfMan.setInt("music_volume", _musicVolume);
	syncSoundSettings();
}

} // End of namespace Fullpipe
