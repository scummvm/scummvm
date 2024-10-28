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

bool Sound::isPlayingSpeech() const {
	return _mixer->isSoundHandleActive(_speechHandle);
}

bool Sound::isPlayingMusic() {
	return _musicPlayer->isPlaying();
}

void Sound::waitForSpeech() {
	while (isPlayingSpeech()) {
		// TODO poll events / wait a bit here.
	}
}

void Sound::resetSpeech() {
	for (int i = 0; i < (int)_didSpeech.size(); i++) {
		_didSpeech[i] = 0;
	}
}

void Sound::playMusic(MusicId musicId) {
	if (musicId == MusicId::kNone) {
		return;
	}
	int filenameIdx = static_cast<uint8>(musicId) - 1;
	playMusic(g_engine->isCdVersion()
		? musicDosCDFilenameTbl[filenameIdx]
		: musicDosFloppyFilenameTbl[filenameIdx],
		true);
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
	_musicPlayer->load(&file, file.size());
	file.close();

	_musicPlayer->play(loop);
}

void Sound::stopMusic() {
	_musicPlayer->stop();
}

void Sound::killAllSound() {
	stopMusic();
}


void Sound::syncSoundSettings() {
	_musicPlayer->syncSoundSettings();
}

Common::Error Sound::sync(Common::Serializer &s) {
	s.syncArray(_didSpeech.data(), _didSpeech.size(), Common::Serializer::Byte);
	return Common::kNoError;
}

} // End of namespace Darkseed
