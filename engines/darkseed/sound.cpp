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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "common/config-manager.h"
#include "darkseed/sound.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

static constexpr char musicDosFloppyFilenameTbl[][14] = {
	"lab1",
	"victory1",
	"cemetry1",
	"leech1",
	"ext1",
	"quiet",
	"package",
	"dth",
	"library",
	"radio",
	"outdoor",
	"town"
};

static constexpr char musicDosCDFilenameTbl[][14] = {
	"lab.mid",
	"victory.mid",
	"cemetery.mid",
	"leech.mid",
	"exterior.mid",
	"passtime.mid",
	"mirrorst.mid",
	"dth.mid",
	"lib_moz.mid",
	"carradio.mid",
	"greenwal.mid",
	"walktown.mid"
};

static constexpr char startMusicDosFloppyFilenameTbl[][14] = {
	"credits",
	"alien",
	"implant",
	"launch",
	"night2",
	"night3",
	"book",
	"doll"
};

static constexpr char startMusicDosCDFilenameTbl[][14] = {
	"openingt.mid",
	"alienmou.mid",
	"mindfuck.mid",
	"spaceshi.mid",
	"mindfuck.mid",
	"zombie.mid",
	"booktran.mid",
	"babydoll.mid"
};

static constexpr char sfxCDFilenameTbl[][14] = {
	"",
	"opendoor.sfx",
	"showers2.sfx",
	"razzsysb.sfx",
	"medicine.sfx",
	"pop.sfx",
	"pickupit.sfx",
	"rockener.sfx",
	"pullleve.sfx",
	"starship.sfx",
	"podwrith.sfx",
	"starterc.sfx",
	"sigils.sfx",
	"tombdoor.sfx",
	"digging.sfx",
	"opendoor.sfx",
	"carstart.sfx",
	"makehamm.sfx",
	"picklock.sfx",
	"impaled.sfx",
	"evilbeas.sfx",
	"laser.sfx",
	"knock.sfx",
	"bubblesi.sfx",
	"phone.sfx",
	"softphon.sfx",
	"pulsar.sfx",
	"doorbell.sfx",
	"mirrorsm.sfx",
	"softdoor.sfx",
	"electroc.sfx",
	"medicine.sfx",
	"pourings.sfx",
	"tuneinra.sfx",
	"opendoor.sfx",
	"showers1.sfx",
	"yo.sfx",
	"showers2.sfx",
	"popii.sfx",
	"carhorn.sfx",
	"yo.sfx",
	"secretdo.sfx",
	"opendoor.sfx",
	"tick.sfx",
	"tock.sfx",
	"chime.sfx",
	"softchim.sfx",
	"shakeurn.sfx",
	"beaming.sfx"
};

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer) {
	_musicPlayer = new MusicPlayer(g_engine);
	_didSpeech.resize(978);
	resetSpeech();
}

Sound::~Sound() {
	delete _musicPlayer;
}

int Sound::init() {
	return _musicPlayer->open();
}

void Sound::playTosSpeech(int tosIdx) {
	if (g_engine->isDosFloppy()) {
		playFloppySpeech(tosIdx);
		return;
	}

	if (!g_engine->isCdVersion() || _didSpeech[tosIdx] == 1) {
		return;
	}
	Common::String filename = Common::String::format("%d.voc", tosIdx + 1);
	Common::Path path = Common::Path("speech").join(filename);
	Common::File f;
	if (!f.open(path)) {
		return;
	}
	Common::SeekableReadStream *srcStream = f.readStream((uint32)f.size());
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(srcStream,
															  Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);
	_didSpeech[tosIdx] = 1;
}

void Sound::stopSpeech() {
	_mixer->stopHandle(_speechHandle);
}

bool Sound::isPlayingSpeech() const {
	return _mixer->isSoundHandleActive(_speechHandle);
}

bool Sound::isPlayingSfx() const {
	return _mixer->isSoundHandleActive(_sfxHandle);
}

bool Sound::isPlayingMusic() {
	return _musicPlayer->isPlaying();
}

void Sound::resetSpeech() {
	for (int i = 0; i < (int)_didSpeech.size(); i++) {
		_didSpeech[i] = 0;
	}
}

void Sound::playMusic(MusicId musicId, bool loop) {
	if (musicId == MusicId::kNone) {
		return;
	}
	int filenameIdx = static_cast<uint8>(musicId) - 1;
	playMusic(g_engine->isCdVersion()
		? musicDosCDFilenameTbl[filenameIdx]
		: musicDosFloppyFilenameTbl[filenameIdx],
		loop);
}

void Sound::playMusic(StartMusicId musicId) {
	int filenameIdx = static_cast<uint8>(musicId);
	playMusic(g_engine->isCdVersion()
		? startMusicDosCDFilenameTbl[filenameIdx]
		: startMusicDosFloppyFilenameTbl[filenameIdx]);
}

void Sound::playMusic(Common::String const &filename, bool loop) {
	debug("Loading music: %s", filename.c_str());
	Common::File file;
	Common::Path path;
	if (!g_engine->isCdVersion()) {
		path = Common::Path(filename);
	} else {
		path = Common::Path("sound").join(filename);
	}
	if (!file.open(path)) {
		debug("Failed to load %s", path.toString().c_str());
		return;
	}
	_musicPlayer->load(&file, (int32)file.size());
	file.close();

	_musicPlayer->play(loop);
}

void Sound::stopMusic() {
	_musicPlayer->stop();
}

void Sound::killAllSound() {
	stopMusic();
	stopSfx();
	stopSpeech();
}

void Sound::syncSoundSettings() {
	_musicPlayer->syncSoundSettings();
}

Common::Error Sound::sync(Common::Serializer &s) {
	s.syncArray(_didSpeech.data(), _didSpeech.size(), Common::Serializer::Byte);
	return Common::kNoError;
}

bool Sound::isMuted() const {
	bool soundIsMuted = false;
	if (ConfMan.hasKey("mute")) {
		soundIsMuted = ConfMan.getBool("mute");
	}
	return soundIsMuted;
}

void Sound::playSfx(uint8 sfxId, int unk1, int unk2) {
	if (g_engine->isCdVersion()) {
		playDosCDSfx(sfxId);
	}
}

void Sound::stopSfx() {
	_mixer->stopHandle(_sfxHandle);
}

void Sound::playDosCDSfx(int sfxId) {
	if (sfxId == 0) {
		// TODO midi SFX
		return;
	}
	if (sfxId > 48) {
		error("playDosCDSfx: Invalid sfxId %d", sfxId);
	}
	if (isPlayingSfx()) {
		return;
	}
	Common::Path path = Common::Path("sound").join(sfxCDFilenameTbl[sfxId]);
	Common::File f;
	if (!f.open(path)) {
		debug("Failed to load sfx. %s", path.toString().c_str());
		return;
	}
	Common::SeekableReadStream *srcStream = f.readStream((uint32)f.size());
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(srcStream,
															  Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_sfxHandle, stream);
}

void Sound::playFloppySpeech(int tosIdx) {
	Common::String filename;

	switch (tosIdx) {
	case 927 : filename = "CL1.VOC"; break;
	case 905 : filename = "CL2.VOC"; break;
	case 906 : filename = "CL3.VOC"; break;
	case 907 : filename = "D4A-1.VOC"; break;
	case 909 : filename = "D5A-1.VOC"; break;
	case 910 : filename = "D6A-2.VOC"; break;
	case 908 : filename = "D6C-2.VOC"; break;
	case 916 : filename = "G10A-1.VOC"; break;
	case 917 : filename = "G10B-1.VOC"; break;
	case 925 : filename = "GL0A.VOC"; break;
	case 926 : filename = "GL1B.VOC"; break;
	case 924 : filename = "GL2A.VOC"; break;
	case 928 : filename = "GL3A.VOC"; break; // TODO is this correct?
	case 922 : filename = "K15A-1.VOC"; break;
	case 913 : filename = "K9A-3.VOC"; break;
	case 914 : filename = "K9C-3.VOC"; break;
	case 915 : filename = "K9E-3.VOC"; break;
	case 904 : filename = "M1-1.VOC"; break;
	case 918 : filename = "M11A-1.VOC"; break;
	case 919 : filename = "O12A-1.VOC"; break;
	case 920 : filename = "O13A-1.VOC"; break;
	case 921 : filename = "O14A-1.VOC"; break;
	case 923 : filename = "S16A-1.VOC"; break;
	case 912 : filename = "S8A-2.VOC"; break;
	default : return;
	}

	Common::Path path = Common::Path(filename);
	Common::File f;
	if (!f.open(path)) {
		return;
	}

	Common::SeekableReadStream *srcStream = f.readStream((uint32)f.size());
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(srcStream,
															  Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);
}

} // End of namespace Darkseed
