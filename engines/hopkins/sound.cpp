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

#include "audio/decoders/adpcm_intern.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/sound.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"
#include "audio/audiostream.h"
#include "audio/mods/protracker.h"

namespace Audio {

class APC_ADPCMStream : public Audio::DVI_ADPCMStream {
public:
	APC_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) : DVI_ADPCMStream(stream, disposeAfterUse, stream->size(), rate, channels, 0) {
		stream->seek(-12, SEEK_CUR);
		_status.ima_ch[0].last = _startValue[0] = stream->readUint32LE();
		_status.ima_ch[1].last = _startValue[1] = stream->readUint32LE();
		stream->seek(4, SEEK_CUR);
	}

	void reset() {
		DVI_ADPCMStream::reset();
		_status.ima_ch[0].last = _startValue[0];
		_status.ima_ch[1].last = _startValue[1];
	}

private:
	int16 _startValue[2];
};

Audio::RewindableAudioStream *makeAPCStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	if (stream->readUint32BE() != MKTAG('C', 'R', 'Y', 'O'))
		return 0;
	if (stream->readUint32BE() != MKTAG('_', 'A', 'P', 'C'))
		return 0;
	stream->readUint32BE(); // version
	stream->readUint32LE(); // out size
	uint32 rate = stream->readUint32LE();
	stream->skip(8); // initial values, will be handled by the class
	bool stereo = stream->readUint32LE() != 0;

	return new APC_ADPCMStream(stream, disposeAfterUse, rate, stereo ? 2 : 1);
}

class TwaAudioStream : public AudioStream {
public:
	TwaAudioStream(Common::String name, Common::SeekableReadStream *stream) {
		_name = name;
		_cueSheet.clear();
		_cueStream = NULL;
		_cue = 0;

		for (;;) {
			char buf[3];
			stream->read(buf, 3);

			if (buf[0] == 'x' || stream->eos())
				break;

			_cueSheet.push_back(atol(buf));
		}

		for (_cue = 0; _cue < _cueSheet.size(); _cue++) {
			if (loadCue(_cue))
				break;
		}
	}

	~TwaAudioStream() {
		delete _cueStream;
		_cueStream = NULL;
	}

	virtual bool isStereo() const {
		return _cueStream ? _cueStream->isStereo() : true;
	}

	virtual int getRate() const {
		return _cueStream ? _cueStream->getRate() : 22050;
	}

	virtual bool endOfData() const {
		return _cueStream == NULL;
	}

	virtual int readBuffer(int16 *buffer, const int numSamples) {
		if (!_cueStream)
			return 0;

		int16 *buf = buffer;
		int samplesLeft = numSamples;

		while (samplesLeft) {
			if (_cueStream) {
				int readSamples = _cueStream->readBuffer(buf, samplesLeft);
				buf += readSamples;
				samplesLeft -= readSamples;
			}

			if (samplesLeft > 0) {
				if (++_cue >= _cueSheet.size()) {
					_cue = 0;
				}
				loadCue(_cue);
			}
		}

		return numSamples;
	}

protected:
	bool loadCue(int nr) {
		delete _cueStream;
		_cueStream = NULL;

		Common::String filename = Common::String::format("%s_%02d", _name.c_str(), _cueSheet[nr]);
		Common::File *file = new Common::File();

		if (file->open(filename + ".APC")) {
			_cueStream = Audio::makeAPCStream(file, DisposeAfterUse::YES);
			return true;
		}

		if (file->open(filename + ".WAV")) {
			_cueStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
			return true;
		}

		warning("TwaAudioStream::loadCue: Missing cue %d (%s)", nr, filename.c_str());
		delete file;
		return false;
	}

private:
	Common::String _name;
	Common::Array<int> _cueSheet;
	Audio::AudioStream *_cueStream;
	uint _cue;
};

Audio::AudioStream *makeTwaStream(Common::String name, Common::SeekableReadStream *stream) {
	return new TwaAudioStream(name, stream);
}

}

/*------------------------------------------------------------------------*/

namespace Hopkins {

SoundManager::SoundManager() {
	SPECIAL_SOUND = 0;
	_soundVolume = 0;
	_voiceVolume = 0;
	_musicVolume = 0;
	_soundOffFl = true;
	_musicOffFl = true;
	_voiceOffFl = true;
	_textOffFl = false;
	_soundFl = false;
	VBL_MERDE = false;
	SOUND_NUM = 0;
	old_music = 0;
	MOD_FLAG = false;

	for (int i = 0; i < VOICE_COUNT; ++i)
		Common::fill((byte *)&Voice[i], (byte *)&Voice[i] + sizeof(VoiceItem), 0);
	for (int i = 0; i < SWAV_COUNT; ++i)
		Common::fill((byte *)&Swav[i], (byte *)&Swav[i] + sizeof(SwavItem), 0);
	for (int i = 0; i < SOUND_COUNT; ++i)
		Common::fill((byte *)&SOUND[i], (byte *)&SOUND[i] + sizeof(SoundItem), 0);
	Common::fill((byte *)&Music, (byte *)&Music + sizeof(MusicItem), 0);
}

SoundManager::~SoundManager() {
	stopMusic();
	delMusic();
	_vm->_mixer->stopHandle(_musicHandle);
	MOD_FLAG = false;
}

void SoundManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
	SPECIAL_SOUND = 0;
}

void SoundManager::WSOUND_INIT() {
	warning("TODO: WSOUND_INIT");
}

void SoundManager::VERIF_SOUND() {
	if (!_soundOffFl && _soundFl) {
		if (!VOICE_STAT(1)) {
			stopVoice(1);
			DEL_NWAV(SOUND_NUM);
		}
	}
}

void SoundManager::LOAD_ANM_SOUND() {
	switch (SPECIAL_SOUND) {
	case 2:
		loadSample(5, "mitra1.wav");
		loadSample(1, "tir2.wav");
		loadSample(2, "sound6.wav");
		loadSample(3, "sound5.WAV");
		loadSample(4, "sound4.WAV");
		break;
	case 5:
		LOAD_WAV("CRIE.WAV", 1);
		break;
	case 14:
		LOAD_WAV("SOUND14.WAV", 1);
		break;
	case 16:
		LOAD_WAV("SOUND16.WAV", 1);
		break;
	case 198:
		LOAD_WAV("SOUND3.WAV", 1);
		break;
	case 199:
		LOAD_WAV("SOUND22.WAV", 1);
		break;
	case 200:
		mixVoice(682, 1);
		break;
	case 208:
		LOAD_WAV("SOUND77.WAV", 1);
		break;
	case 210:
		LOAD_WAV("SOUND78.WAV", 1);
		break;
	case 211:
		LOAD_WAV("SOUND78.WAV", 1);
		break;
	case 229:
		LOAD_WAV("SOUND80.WAV", 1);
		LOAD_WAV("SOUND82.WAV", 2);
		break;
	}
}

void SoundManager::playAnim_SOUND(int soundNumber) {
	if (!_vm->_globals._censorshipFl && SPECIAL_SOUND == 2) {
		switch (soundNumber) {
		case 20:
			PLAY_SAMPLE2(5);
			break;
		case 57:
		case 63:
		case 69:
			PLAY_SAMPLE2(1);
			break;
		case 75:
			PLAY_SAMPLE2(2);
			break;
		case 109:
			PLAY_SAMPLE2(3);
			break;
		case 122:
			PLAY_SAMPLE2(4);
			break;
		}
	} else if (SPECIAL_SOUND == 1 && soundNumber == 17)
		playSound("SOUND42.WAV");
	else if (SPECIAL_SOUND == 5 && soundNumber == 19)
		playWav(1);
	else if (SPECIAL_SOUND == 14 && soundNumber == 625)
		playWav(1);
	else if (SPECIAL_SOUND == 16 && soundNumber == 25)
		playWav(1);
	else if (SPECIAL_SOUND == 17) {
		if (soundNumber == 6)
			PLAY_SAMPLE2(1);
		else if (soundNumber == 14)
			PLAY_SAMPLE2(2);
		else if (soundNumber == 67)
			PLAY_SAMPLE2(3);
	} else if (SPECIAL_SOUND == 198 && soundNumber == 15)
		playWav(1);
	else if (SPECIAL_SOUND == 199 && soundNumber == 72)
		playWav(1);
	else if (SPECIAL_SOUND == 208 && soundNumber == 40)
		playWav(1);
	else if (SPECIAL_SOUND == 210 && soundNumber == 2)
		playWav(1);
	else if (SPECIAL_SOUND == 211 && soundNumber == 22)
		playWav(1);
	else if (SPECIAL_SOUND == 229) {
		if (soundNumber == 15)
			playWav(1);
		else if (soundNumber == 91)
			playWav(2);
	}
}

static const char *modSounds[] = {
	"appart", "ville", "Rock", "police", "deep",
	"purgat", "riviere", "SUSPENS", "labo", "cadavre",
	"cabane", "purgat2", "foret", "ile", "ile2",
	"hopkins", "peur", "URAVOLGA", "BASE", "cadavre2",
	"usine", "chien", "coeur", "stand", "ocean",
	"base3", "gloop", "cant", "feel", "lost",
	"tobac"
};

void SoundManager::WSOUND(int soundNumber) {
	if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
		if (soundNumber > 27)
			return;
	}

	if (old_music != soundNumber || !MOD_FLAG) {
		if (MOD_FLAG)
			WSOUND_OFF();

		PLAY_MOD(modSounds[soundNumber - 1]);
		old_music = soundNumber;
	}
}

void SoundManager::WSOUND_OFF() {
	stopVoice(0);
	stopVoice(1);
	stopVoice(2);
	if (_vm->_soundManager._soundFl)
		DEL_NWAV(SOUND_NUM);

	for (int i = 1; i <= 48; ++i)
		DEL_SAMPLE_SDL(i);

	if (MOD_FLAG) {
		stopMusic();
		delMusic();
		MOD_FLAG = false;
	}
}

void SoundManager::PLAY_MOD(const Common::String &file) {
	if (_musicOffFl)
		return;
	Common::String modFile = file;

	// HACK
	if (modFile == "URAVOLGA" && (_vm->getPlatform() == Common::kPlatformWindows || _vm->getPlatform() == Common::kPlatformLinux))
		modFile = "peur";

	// The Windows/Linux version chops off the music file names to 5 characters
	if (modFile.size() > 5 && (_vm->getPlatform() == Common::kPlatformWindows || _vm->getPlatform() == Common::kPlatformLinux)) {
		if (!modFile.hasSuffix("2")) {
			while (modFile.size() > 5)
				modFile.deleteLastChar();
		} else {
			while (modFile.size() > 4)
				modFile.deleteLastChar();
			modFile += "2";
		}
	}
	_vm->_fileManager.constructFilename("MUSIC", modFile);
	if (MOD_FLAG) {
		stopMusic();
		delMusic();
		MOD_FLAG = false;
	}

	loadMusic(_vm->_globals._curFilename);
	playMusic();
	MOD_FLAG = true;
}

void SoundManager::loadMusic(const Common::String &file) {
	if (Music._active)
		delMusic();

	Common::File f;
	if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
		Common::String filename = Common::String::format("%s.MOD", file.c_str());

		if (!f.open(filename))
			error("Error opening file %s", filename.c_str());

		Audio::AudioStream *modStream = Audio::makeProtrackerStream(&f);
		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, modStream);

	} else {
		Common::String filename = Common::String::format("%s.TWA", file.c_str());

		if (!f.open(filename))
			error("Error opening file %s", filename.c_str());

		Audio::AudioStream *twaStream = Audio::makeTwaStream(file.c_str(), &f);
		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, twaStream);
		f.close();
	}

	Music._active = true;
}

void SoundManager::playMusic() {
}

void SoundManager::stopMusic() {
	_vm->_mixer->stopHandle(_musicHandle);
}

void SoundManager::delMusic() {
	Music._active = false;
}

void SoundManager::checkSounds() {
	checkVoices();
}

void SoundManager::checkVoices() {
	// Check the status of each voice.
	bool hasActiveVoice = false;
	for (int i = 0; i < VOICE_COUNT; ++i) {
		VOICE_STAT(i);
		hasActiveVoice |= Voice[i]._status != 0;
	}

	if (!hasActiveVoice && _soundFl) {
		_soundFl = false;
		SOUND_NUM = 0;
	}
}

bool SoundManager::mixVoice(int voiceId, int voiceMode) {
	int fileNumber;
	int oldMusicVol;
	bool breakFlag;
	Common::String prefix;
	Common::String filename;
	Common::File f;
	size_t catPos, catLen;

	fileNumber = voiceId;
	if (_voiceOffFl)
		return false;

	if ((unsigned int)(voiceMode - 1) <= 1
	        && (voiceId == 4
	            || voiceId == 16
	            || voiceId == 121
	            || voiceId == 142
	            || voiceId == 182
	            || voiceId == 191
	            || voiceId == 212
	            || voiceId == 225
	            || voiceId == 239
	            || voiceId == 245
	            || voiceId == 297
	            || voiceId == 308
	            || voiceId == 333
	            || voiceId == 348
	            || voiceId == 352
	            || voiceId == 358
	            || voiceId == 364
	            || voiceId == 371
	            || voiceId == 394
	            || voiceId == 414
	            || voiceId == 429
	            || voiceId == 442
	            || voiceId == 446
	            || voiceId == 461
	            || voiceId == 468
	            || voiceId == 476
	            || voiceId == 484
	            || voiceId == 491
	            || voiceId == 497
	            || voiceId == 501
	            || voiceId == 511
	            || voiceId == 520
	            || voiceId == 536
	            || voiceId == 554
	            || voiceId == 566
	            || voiceId == 573
	            || voiceId == 632
	            || voiceId == 645))
		fileNumber = 684;

	if ((unsigned int)(voiceMode - 1) <= 1) {
		prefix = "DF";
	}
	if (voiceMode == 3) {
		prefix = "IF";
	}
	if (voiceMode == 4) {
		prefix = "TF";
	}
	if (voiceMode == 5) {
		prefix = "OF";
	}

	filename = Common::String::format("%s%d", prefix.c_str(), fileNumber);

	if (!_vm->_fileManager.searchCat(filename + ".WAV", 9)) {
		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			_vm->_fileManager.constructFilename("VOICE", "ENG_VOI.RES");
		// Win95 and Linux versions uses another set of names
		else if (_vm->_globals._language == LANG_FR)
			_vm->_fileManager.constructFilename("VOICE", "RES_VFR.RES");
		else if (_vm->_globals._language == LANG_EN)
			_vm->_fileManager.constructFilename("VOICE", "RES_VAN.RES");
		else if (_vm->_globals._language == LANG_SP)
			_vm->_fileManager.constructFilename("VOICE", "RES_VES.RES");

		catPos = _vm->_globals._catalogPos;
		catLen = _vm->_globals._catalogSize;
	} else if (!_vm->_fileManager.searchCat(filename + ".APC", 9)) {
		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			_vm->_fileManager.constructFilename("VOICE", "ENG_VOI.RES");
		// Win95 and Linux versions uses another set of names
		else if (_vm->_globals._language == LANG_FR)
			_vm->_fileManager.constructFilename("VOICE", "RES_VFR.RES");
		else if (_vm->_globals._language == LANG_EN)
			_vm->_fileManager.constructFilename("VOICE", "RES_VAN.RES");
		else if (_vm->_globals._language == LANG_SP)
			_vm->_fileManager.constructFilename("VOICE", "RES_VES.RES");

		catPos = _vm->_globals._catalogPos;
		catLen = _vm->_globals._catalogSize;
	} else {
		_vm->_fileManager.constructFilename("VOICE", filename + ".WAV");
		if (!f.exists(_vm->_globals._curFilename)) {
			_vm->_fileManager.constructFilename("VOICE", filename + ".APC");
			if (!f.exists(_vm->_globals._curFilename))
				return false;
		}

		catPos = 0;
		catLen = 0;
	}

	SDL_LVOICE(catPos, catLen);
	oldMusicVol = _musicVolume;
	if (!_musicOffFl && _musicVolume > 2)
		_musicVolume = (signed int)((long double)_musicVolume - (long double)_musicVolume / 100.0 * 45.0);

	PLAY_VOICE_SDL();

	// Loop for playing voice
	breakFlag = 0;
	do {
		if (SPECIAL_SOUND != 4 && !VBL_MERDE)
			_vm->_eventsManager.VBL();
		if (_vm->_eventsManager.getMouseButton())
			break;
		_vm->_eventsManager.refreshEvents();
		if (_vm->_eventsManager._escKeyFl)
			break;
		if (!VOICE_STAT(2))
			breakFlag = true;
	} while (!_vm->shouldQuit() && !breakFlag);


	stopVoice(2);
	DEL_SAMPLE_SDL(20);
	_musicVolume = oldMusicVol;
	_vm->_eventsManager._escKeyFl = false;
	VBL_MERDE = 0;
	return true;
}

void SoundManager::DEL_SAMPLE(int soundIndex) {
	if (VOICE_STAT(1) == 1)
		stopVoice(1);
	if (VOICE_STAT(2) == 2)
		stopVoice(2);
	if (VOICE_STAT(3) == 3)
		stopVoice(3);
	DEL_SAMPLE_SDL(soundIndex);
	SOUND[soundIndex]._active = false;
}

void SoundManager::playSound(const Common::String &file) {
	if (!_soundOffFl) {
		if (_soundFl)
			DEL_NWAV(SOUND_NUM);
		LOAD_NWAV(file, 1);
		PLAY_NWAV(1);
	}
}

void SoundManager::PLAY_SOUND2(const Common::String &file) {
	if (!_soundOffFl) {
		LOAD_NWAV(file, 1);
		PLAY_NWAV(1);
	}
}

void SoundManager::MODSetSampleVolume() {
	// No implementation needed
}

void SoundManager::MODSetVoiceVolume() {
	// No implementation needed
}

void SoundManager::MODSetMusicVolume(int volume) {
	// No implementation needed
}

void SoundManager::loadSample(int wavIndex, const Common::String &file) {
	_vm->_fileManager.constructFilename("SOUND", file);
	LOAD_SAMPLE2_SDL(wavIndex, _vm->_globals._curFilename, 0);
	SOUND[wavIndex]._active = true;
}

void SoundManager::playSample(int wavIndex, int voiceMode) {
	if (!_soundOffFl && SOUND[wavIndex]._active) {
		if (_soundFl)
			DEL_NWAV(SOUND_NUM);
		if (voiceMode == 5) {
			if (VOICE_STAT(1) == 1)
				stopVoice(1);
			PLAY_SAMPLE_SDL(1, wavIndex);
		}
		if (voiceMode == 6) {
			if (VOICE_STAT(2) == 1)
				stopVoice(1);
			PLAY_SAMPLE_SDL(2, wavIndex);
		}
		if (voiceMode == 7) {
			if (VOICE_STAT(3) == 1)
				stopVoice(1);
			PLAY_SAMPLE_SDL(3, wavIndex);
		}
		if (voiceMode == 8) {
			if (VOICE_STAT(1) == 1)
				stopVoice(1);
			PLAY_SAMPLE_SDL(1, wavIndex);
		}
	}
}

void SoundManager::PLAY_SAMPLE2(int idx) {
	if (!_soundOffFl && SOUND[idx]._active) {
		if (_soundFl)
			DEL_NWAV(SOUND_NUM);
		if (VOICE_STAT(1) == 1)
			stopVoice(1);
		PLAY_SAMPLE_SDL(1, idx);
	}
}

void SoundManager::LOAD_WAV(const Common::String &file, int wavIndex) {
	LOAD_NWAV(file, wavIndex);
}

void SoundManager::playWav(int wavIndex) {
	PLAY_NWAV(wavIndex);
}

int SoundManager::VOICE_STAT(int voiceIndex) {
	if (Voice[voiceIndex]._status) {
		int wavIndex = Voice[voiceIndex]._wavIndex;
		if (Swav[wavIndex]._audioStream != NULL && Swav[wavIndex]._audioStream->endOfStream())
			stopVoice(voiceIndex);
	}

	return Voice[voiceIndex]._status;
}

void SoundManager::stopVoice(int voiceIndex) {
	if (Voice[voiceIndex]._status) {
		Voice[voiceIndex]._status = 0;
		int wavIndex = Voice[voiceIndex]._wavIndex;
		if (Swav[wavIndex]._active) {
			if (Swav[wavIndex].freeSample)
				DEL_SAMPLE_SDL(wavIndex);
		}
	}
	Voice[voiceIndex].fieldC = 0;
	Voice[voiceIndex]._status = 0;
	Voice[voiceIndex].field14 = 0;
}

void SoundManager::SDL_LVOICE(size_t filePosition, size_t entryLength) {
	if (!SDL_LoadVoice(_vm->_globals._curFilename, filePosition, entryLength, Swav[20]))
		error("Couldn't load the sample %s", _vm->_globals._curFilename.c_str());

	Swav[20]._active = true;
}

void SoundManager::PLAY_VOICE_SDL() {
	if (!Swav[20]._active)
		error("Bad handle");

	if (!Voice[2]._status) {
		int wavIndex = Voice[2]._wavIndex;
		if (Swav[wavIndex]._active && Swav[wavIndex].freeSample)
			DEL_SAMPLE_SDL(wavIndex);
	}

	PLAY_SAMPLE_SDL(2, 20);
}

bool SoundManager::DEL_SAMPLE_SDL(int wavIndex) {
	if (Swav[wavIndex]._active) {
		_vm->_mixer->stopHandle(Swav[wavIndex]._soundHandle);
		delete Swav[wavIndex]._audioStream;
		Swav[wavIndex]._audioStream = NULL;
		Swav[wavIndex]._active = false;

		return true;
	} else {
		return false;
	}
}

bool SoundManager::SDL_LoadVoice(const Common::String &filename, size_t fileOffset, size_t entryLength, SwavItem &item) {
	Common::File f;
	if (!f.open(filename)) {
		// Fallback from WAV to APC...
		if (!f.open(setExtension(filename, ".APC")))
			error("Could not open %s for reading", filename.c_str());
	}

	f.seek(fileOffset);
	item._audioStream = makeSoundStream(f.readStream((entryLength == 0) ? f.size() : entryLength));
	f.close();

	return true;
}

void SoundManager::LOAD_SAMPLE2_SDL(int wavIndex, const Common::String &filename, bool freeSample) {
	if (Swav[wavIndex]._active)
		DEL_SAMPLE_SDL(wavIndex);

	SDL_LoadVoice(filename, 0, 0, Swav[wavIndex]);
	Swav[wavIndex]._active = true;
	Swav[wavIndex].freeSample = freeSample;
}

void SoundManager::LOAD_NWAV(const Common::String &file, int wavIndex) {
	_vm->_fileManager.constructFilename("SOUND", file);
	LOAD_SAMPLE2_SDL(wavIndex, _vm->_globals._curFilename, 1);
}

void SoundManager::PLAY_NWAV(int wavIndex) {
	if (!_soundFl && !_soundOffFl) {
		_soundFl = true;
		SOUND_NUM = wavIndex;
		PLAY_SAMPLE_SDL(1, wavIndex);
	}
}

void SoundManager::DEL_NWAV(int wavIndex) {
	if (DEL_SAMPLE_SDL(wavIndex)) {
		if (VOICE_STAT(1) == 1)
			stopVoice(1);

		SOUND_NUM = 0;
		_soundFl = false;
	}
}

void SoundManager::PLAY_SAMPLE_SDL(int voiceIndex, int wavIndex) {
	if (!Swav[wavIndex]._active)
		warning("Bad handle");

	if (Voice[voiceIndex]._status == 1 && Swav[wavIndex]._active && Swav[wavIndex].freeSample)
		DEL_SAMPLE_SDL(wavIndex);

	Voice[voiceIndex].fieldC = 0;
	Voice[voiceIndex]._status = 1;
	Voice[voiceIndex].field14 = 4;
	Voice[voiceIndex]._wavIndex = wavIndex;

	int volume = (voiceIndex == 2) ? _voiceVolume * 255 / 16 : _soundVolume * 255 / 16;

	// Start the voice playing
	Swav[wavIndex]._audioStream->rewind();
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &Swav[wavIndex]._soundHandle,
		Swav[wavIndex]._audioStream, -1, volume, 0, DisposeAfterUse::NO);
}

void SoundManager::syncSoundSettings() {
	bool muteAll = false;
	if (ConfMan.hasKey("mute"))
		muteAll = ConfMan.getBool("mute");

	// Update the mute settings
	_musicOffFl = muteAll || (ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute"));
	_soundOffFl = muteAll || (ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute"));
	_voiceOffFl = muteAll || (ConfMan.hasKey("speech_mute") && ConfMan.getBool("speech_mute"));

	// Update the volume levels
	_musicVolume = MIN(255, ConfMan.getInt("music_volume")) * 16 / 255;
	_soundVolume = MIN(255, ConfMan.getInt("sfx_volume")) * 16 / 255;
	_voiceVolume = MIN(255, ConfMan.getInt("speech_volume")) * 16 / 255;

	// Update any active sounds
	for (int idx = 0; idx < SWAV_COUNT; ++idx) {
		if (Swav[idx]._active) {
			int volume = (idx == 20) ? (_voiceVolume * 255 / 16) : (_soundVolume * 255 / 16);
			_vm->_mixer->setChannelVolume(Swav[idx]._soundHandle, volume);
		}
	}
	if (_vm->_mixer->isSoundHandleActive(_musicHandle)) {
		_vm->_mixer->setChannelVolume(_musicHandle, _musicVolume * 255 / 16);
	}
}

void SoundManager::updateScummVMSoundSettings() {
	ConfMan.setBool("mute", _musicOffFl && _soundOffFl && _voiceOffFl);
	ConfMan.setBool("music_mute", _musicOffFl);
	ConfMan.setBool("sfx_mute", _soundOffFl);
	ConfMan.setBool("speech_mute", _voiceOffFl);

	ConfMan.setInt("music_volume", _musicVolume * 255 / 16);
	ConfMan.setInt("sfx_volume", _soundVolume * 255 / 16);
	ConfMan.setInt("speech_volume", _voiceVolume * 255 / 16);

	ConfMan.flushToDisk();
}

Audio::RewindableAudioStream *SoundManager::makeSoundStream(Common::SeekableReadStream *stream) {
	if (_vm->getPlatform() == Common::kPlatformWindows)
		return Audio::makeAPCStream(stream, DisposeAfterUse::YES);
	else
		return Audio::makeWAVStream(stream, DisposeAfterUse::YES);
}

// Blatant rip from gob engine. Hi DrMcCoy!
Common::String SoundManager::setExtension(const Common::String &str, const Common::String &ext) {
	if (str.empty())
		return str;

	const char *dot = strrchr(str.c_str(), '.');
	if (dot)
		return Common::String(str.c_str(), dot - str.c_str()) + ext;

	return str + ext;
}
} // End of namespace Hopkins
