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

#include "hopkins/sound.h"

#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

#include "audio/decoders/adpcm_intern.h"
#include "audio/decoders/wave.h"
#include "audio/softsynth/pcspk.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "audio/audiostream.h"
#include "audio/mods/module.h"
#include "audio/mods/protracker.h"
#include "audio/decoders/raw.h"

namespace Hopkins {

class APC_ADPCMStream : public Audio::DVI_ADPCMStream {
public:
	APC_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) : DVI_ADPCMStream(stream, disposeAfterUse, stream->size(), rate, channels, 0) {
		stream->seek(-12, SEEK_CUR);
		_status.ima_ch[0].last = _startValue[0] = stream->readUint32LE();
		_status.ima_ch[1].last = _startValue[1] = stream->readUint32LE();
		stream->seek(4, SEEK_CUR);
	}

	void reset() override {
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

class TwaAudioStream : public Audio::AudioStream {
public:
	TwaAudioStream(Common::String name, Common::SeekableReadStream *stream) {
		_name = name;
		_cueSheet.clear();
		_cueStream = NULL;
		_cue = 0;
		_loadedCue = -1;

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

	~TwaAudioStream() override {
		delete _cueStream;
		_cueStream = NULL;
	}

	bool isStereo() const override {
		return _cueStream ? _cueStream->isStereo() : true;
	}

	int getRate() const override {
		return _cueStream ? _cueStream->getRate() : 22050;
	}

	bool endOfData() const override {
		return _cueStream == NULL;
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
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
		if (_loadedCue == _cueSheet[nr]) {
			_cueStream->rewind();
			return true;
		}

		delete _cueStream;
		_cueStream = NULL;
		_loadedCue = _cueSheet[nr];

		Common::String filename = Common::String::format("%s_%02d", _name.c_str(), _cueSheet[nr]);
		Common::File *file = new Common::File();

		if (file->open(filename + ".APC")) {
			_cueStream = makeAPCStream(file, DisposeAfterUse::YES);
			return true;
		}

		if (file->open(filename + ".WAV")) {
			_cueStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
			return true;
		}

		if (file->open(filename + ".RAW")) {
			_cueStream = Audio::makeRawStream(file, 22050, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
			return true;
		}

		warning("TwaAudioStream::loadCue: Missing cue %d (%s)", nr, filename.c_str());
		_loadedCue = -1;
		delete file;
		return false;
	}

private:
	Common::String _name;
	Common::Array<int> _cueSheet;
	Audio::RewindableAudioStream *_cueStream;
	uint _cue;
	int _loadedCue;
};

Audio::AudioStream *makeTwaStream(Common::String name, Common::SeekableReadStream *stream) {
	return new TwaAudioStream(name, stream);
}

SoundManager::SoundManager(HopkinsEngine *vm) {
	_vm = vm;

	_specialSoundNum = 0;
	_soundVolume = 0;
	_voiceVolume = 0;
	_musicVolume = 0;
	_soundOffFl = true;
	_musicOffFl = true;
	_voiceOffFl = true;
	_textOffFl = false;
	_soundFl = false;
	_skipRefreshFl = false;
	_currentSoundIndex = 0;
	_oldSoundNumber = 0;
	_modPlayingFl = false;
}

SoundManager::~SoundManager() {
	stopMusic();
	delMusic();
	_vm->_mixer->stopHandle(_musicHandle);
	_modPlayingFl = false;
}

void SoundManager::checkSoundEnd() {
	if (!_soundOffFl && _soundFl) {
		if (!checkVoiceStatus(1)) {
			stopVoice(1);
			delWav(_currentSoundIndex);
		}
	}
}

void SoundManager::loadAnimSound() {
	switch (_specialSoundNum) {
	case 2:
		loadSample(5, "mitra1.wav");
		loadSample(1, "tir2.wav");
		loadSample(2, "sound6.wav");
		loadSample(3, "sound5.WAV");
		loadSample(4, "sound4.WAV");
		break;
	case 5:
		loadWav("CRIE.WAV", 1);
		break;
	case 14:
		loadWav("SOUND14.WAV", 1);
		break;
	case 16:
		loadWav("SOUND16.WAV", 1);
		break;
	case 198:
		loadWav("SOUND3.WAV", 1);
		break;
	case 199:
		loadWav("SOUND22.WAV", 1);
		break;
	case 200:
		mixVoice(682, 1);
		break;
	case 208:
		loadWav("SOUND77.WAV", 1);
		break;
	case 210:
		loadWav("SOUND78.WAV", 1);
		break;
	case 211:
		loadWav("SOUND78.WAV", 1);
		break;
	case 229:
		loadWav("SOUND80.WAV", 1);
		loadWav("SOUND82.WAV", 2);
		break;
	default:
		break;
	}
}

void SoundManager::playAnimSound(int animFrame) {
	if (!_vm->_globals->_censorshipFl && _specialSoundNum == 2) {
		switch (animFrame) {
		case 20:
			playSample(5);
			break;
		case 57:
		case 63:
		case 69:
			playSample(1);
			break;
		case 75:
			// This removes the sound of the gun played while the guard is being shot, as this part of the scene has been
			// removed in the Polish version of the game
			if (_vm->getLanguage() != Common::PL_POL)
				playSample(2);
			break;
		case 95:
			// This fixes an original bug in the Polish version of the game, which was literally butchered for some reason
			if (_vm->getLanguage() == Common::PL_POL)
				playSample(3);
			break;
		case 109:
			if (_vm->getLanguage() != Common::PL_POL)
				playSample(3);
			break;
		case 108:
			// This fixes an original bug in the Polish version of the game, which was literally butchered for some reason
			if (_vm->getLanguage() == Common::PL_POL)
				playSample(4);
			break;
		case 122:
			if (_vm->getLanguage() != Common::PL_POL)
				playSample(4);
			break;
		default:
			break;
		}
	} else if (_specialSoundNum == 1 && animFrame == 17)
		playSoundFile("SOUND42.WAV");
	else if (_specialSoundNum == 5 && animFrame == 19)
		playWav(1);
	else if (_specialSoundNum == 14 && animFrame == 625)
		playWav(1);
	else if (_specialSoundNum == 16 && animFrame == 25)
		playWav(1);
	else if (_specialSoundNum == 17) {
		if (animFrame == 6)
			playSample(1);
		else if (animFrame == 14)
			playSample(2);
		else if (animFrame == 67)
			playSample(3);
	} else if (_specialSoundNum == 198 && animFrame == 15)
		playWav(1);
	else if (_specialSoundNum == 199 && animFrame == 72)
		playWav(1);
	else if (_specialSoundNum == 208 && animFrame == 40)
		playWav(1);
	else if (_specialSoundNum == 210 && animFrame == 2)
		playWav(1);
	else if (_specialSoundNum == 211 && animFrame == 22)
		playWav(1);
	else if (_specialSoundNum == 229) {
		if (animFrame == 15)
			playWav(1);
		else if (animFrame == 91)
			playWav(2);
	}
}

static const char *const modSounds[] = {
	"appart", "ville", "Rock", "police", "deep",
	"purgat", "riviere", "SUSPENS", "labo", "cadavre",
	"cabane", "purgat2", "foret", "ile", "ile2",
	"hopkins", "peur", "URAVOLGA", "BASE", "cadavre2",
	"usine", "chien", "coeur", "stand", "ocean",
	"base3", "gloop", "cant", "feel", "lost",
	"tobac"
};

void SoundManager::playSound(int soundNumber) {
	if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
		if (soundNumber > 27)
			return;
	}

	if (_oldSoundNumber != soundNumber || !_modPlayingFl) {
		if (_modPlayingFl)
			stopSound();

		playMod(modSounds[soundNumber - 1]);
		_oldSoundNumber = soundNumber;
	}
}

void SoundManager::stopSound() {
	stopVoice(0);
	stopVoice(1);
	stopVoice(2);
	if (_soundFl)
		delWav(_currentSoundIndex);

	for (int i = 1; i <= 48; ++i)
		removeWavSample(i);

	if (_modPlayingFl) {
		stopMusic();
		delMusic();
		_modPlayingFl = false;
	}
}

void SoundManager::playMod(const Common::String &file) {
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
	if (_modPlayingFl) {
		stopMusic();
		delMusic();
		_modPlayingFl = false;
	}

	loadMusic(modFile);
	playMusic();
	_modPlayingFl = true;
}

void SoundManager::loadMusic(const Common::String &file) {
	if (_music._active)
		delMusic();

	Common::File f;
	if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
		Common::String filename = Common::String::format("%s.MOD", file.c_str());

		if (!f.open(filename))
			error("Error opening file %s", filename.c_str());

		Modules::Module *module;
		Audio::AudioStream *modStream = Audio::makeProtrackerStream(&f, 0, 44100, true, &module);

		// WORKAROUND: This song is played at the empty lot where the
		// bank robbers have left the helicopter. The MOD file appears
		// to be slightly broken. Almost half of it is just the same
		// noise repeating. We fix this by only playing the working
		// part of it. The result is pretty close to the Windows music.
		if (file.equalsIgnoreCase("cadavre")) {
			module->songlen = 3;
		}

		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, modStream);

	} else {
		Common::String filename = Common::String::format("%s.TWA", file.c_str());

		if (!f.open(filename))
			error("Error opening file %s", filename.c_str());

		Audio::AudioStream *twaStream = makeTwaStream(file.c_str(), &f);
		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, twaStream);
		f.close();
	}

	_music._active = true;
}

void SoundManager::playMusic() {
}

void SoundManager::stopMusic() {
	_vm->_mixer->stopHandle(_musicHandle);
}

void SoundManager::delMusic() {
	_music._active = false;
}

void SoundManager::checkSounds() {
	checkVoiceActivity();
}

/**
 * Checks voices to see if they're finished
 */
void SoundManager::checkVoiceActivity() {
	// Check the status of each voice.
	bool hasActiveVoice = false;
	for (int i = 0; i < VOICE_COUNT; ++i)
		hasActiveVoice |= checkVoiceStatus(i);

	if (!hasActiveVoice && _soundFl) {
		_soundFl = false;
		_currentSoundIndex = 0;
	}
}

bool SoundManager::mixVoice(int voiceId, int voiceMode, bool dispTxtFl) {
	int fileNumber;
	bool breakFlag;
	Common::String prefix;
	Common::String filename;
	Common::File f;
	size_t catPos, catLen;

	fileNumber = voiceId;
	if (_voiceOffFl)
		return false;

	if ((voiceMode == 1 || voiceMode == 2)
	 && (   voiceId == 4   || voiceId == 16  || voiceId == 121
	     || voiceId == 142 || voiceId == 182 || voiceId == 191
		 || voiceId == 212 || voiceId == 225 || voiceId == 239
		 || voiceId == 245 || voiceId == 297 || voiceId == 308
		 || voiceId == 333 || voiceId == 348 || voiceId == 352
		 || voiceId == 358 || voiceId == 364 || voiceId == 371
		 || voiceId == 394 || voiceId == 414 || voiceId == 429
		 || voiceId == 442 || voiceId == 446 || voiceId == 461
		 || voiceId == 468 || voiceId == 476 || voiceId == 484
		 || voiceId == 491 || voiceId == 497 || voiceId == 501
		 || voiceId == 511 || voiceId == 520 || voiceId == 536
		 || voiceId == 554 || voiceId == 566 || voiceId == 573
		 || voiceId == 632 || voiceId == 645))
		fileNumber = 684;

	if (voiceMode == 1 || voiceMode == 2)
		prefix = "DF";
	else if (voiceMode == 3)
		prefix = "IF";
	else if (voiceMode == 4)
		prefix = "TF";
	else if (voiceMode == 5)
		prefix = "OF";

	// BeOS and OS/2 versions are using a slightly different speech order during intro
	// This map those values to the ones used by the Win95 and Linux versions
	int mappedFileNumber = fileNumber;
	if (voiceMode == 3 && (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)) {
		if (fileNumber == 4)
			mappedFileNumber = 0;
		else if (fileNumber > 4)
			mappedFileNumber = fileNumber - 1;
	}

	filename = Common::String::format("%s%d", prefix.c_str(), mappedFileNumber);

	bool fileFoundFl = false;
	_vm->_fileIO->searchCat(filename + ".WAV", RES_VOI, fileFoundFl);
	if (fileFoundFl) {
		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
			filename = "ENG_VOI.RES";
		} else {
			// Win95 and Linux versions uses another set of names
			switch (_vm->_globals->_language) {
			case LANG_FR:
				filename = "RES_VFR.RES";
				break;
			case LANG_EN:
				filename = "RES_VAN.RES";
				break;
			case LANG_SP:
				filename = "RES_VES.RES";
				break;
			default:
				break;
			}
		}

		catPos = _vm->_fileIO->_catalogPos;
		catLen = _vm->_fileIO->_catalogSize;
	} else {
		_vm->_fileIO->searchCat(filename + ".APC", RES_VOI, fileFoundFl);
		if (fileFoundFl) {
			if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
				filename = "ENG_VOI.RES";
			} else {
				// Win95 and Linux versions uses another set of names
				switch (_vm->_globals->_language) {
				case LANG_FR:
					filename = "RES_VFR.RES";
					break;
				case LANG_EN:
					filename = "RES_VAN.RES";
					break;
				case LANG_SP:
					filename = "RES_VES.RES";
					break;
				default:
					break;
				}
			}

			catPos = _vm->_fileIO->_catalogPos;
			catLen = _vm->_fileIO->_catalogSize;
		} else {
			_vm->_fileIO->searchCat(filename + ".RAW", RES_VOI, fileFoundFl);
			if (fileFoundFl) {
				if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS) {
					filename = "ENG_VOI.RES";
				} else {
					// Win95 and Linux versions uses another set of names
					switch (_vm->_globals->_language) {
					case LANG_FR:
						filename = "RES_VFR.RES";
						break;
					case LANG_EN:
						filename = "RES_VAN.RES";
						break;
					case LANG_SP:
						filename = "RES_VES.RES";
						break;
					default:
						break;
					}
				}

				catPos = _vm->_fileIO->_catalogPos;
				catLen = _vm->_fileIO->_catalogSize;
			} else {
				if (!f.exists(filename + ".WAV")) {
					if (!f.exists(filename + ".APC"))
						return false;
					filename = filename + ".APC";
				} else
					filename = filename + ".WAV";

				catPos = 0;
				catLen = 0;
			}
		}
	}
	int oldMusicVol = _musicVolume;
	if (!loadVoice(filename, catPos, catLen, _sWav[20])) {
		// This case only concerns the English Win95 demo
		// If it's not possible to load the voice, we force the active flag
		// to false in order to make sure the missing buffer won't be played
		// accidentally later
		_sWav[20]._active = false;
	} else {
		_sWav[20]._active = true;

		// Reduce music volume during speech
		if (!_musicOffFl && _musicVolume > 2) {
			_musicVolume -= _musicVolume * 9 / 20;
			setMODMusicVolume(_musicVolume);
		}
	}
	playVoice();

	_vm->_events->_escKeyFl = false;

	// Loop for playing voice
	breakFlag = false;
	do {
		if (_specialSoundNum != 4 && !_skipRefreshFl)
			_vm->_events->refreshScreenAndEvents();
		if (_vm->_events->getMouseButton())
			break;
		_vm->_events->refreshEvents();
		if (_vm->_events->_escKeyFl)
			break;
		// We only check the voice status if the file has been loaded properly
		// This avoids skipping completely the talk animations in the Win95 UK Demo
		if (!checkVoiceStatus(2) && _sWav[20]._active)
			breakFlag = true;
		// This is specific to the Win95 UK Demo again: if nothing is displayed,
		// don't wait for a click event.
		if (!_sWav[20]._active && !dispTxtFl)
			break;
	} while (!_vm->shouldQuit() && !breakFlag);


	stopVoice(2);
	removeWavSample(20);

	// Speech is over, set the music volume back to normal
	_musicVolume = oldMusicVol;
	if (!_musicOffFl && _musicVolume > 2) {
		setMODMusicVolume(_musicVolume);
	}
	_vm->_events->_escKeyFl = false;
	_skipRefreshFl = false;
	return true;
}

void SoundManager::removeSample(int soundIndex) {
	if (checkVoiceStatus(1))
		stopVoice(1);
	if (checkVoiceStatus(2))
		stopVoice(2);
	removeWavSample(soundIndex);
	_sound[soundIndex]._active = false;
}

void SoundManager::playSoundFile(const Common::String &file) {
	if (_soundOffFl)
		return;

	// Fallback for the menu option.
	// The BeOS and OS/2 versions don't play sound at this point.
	// sound20 sounds very close to bruit2 from the linux and Win95 versions.
	Common::File f;
	Common::String filename;
	if (file == "bruit2.wav" && !f.exists(file))
		filename = "sound20.wav";
	else
		filename = file;

	if (_soundFl)
		delWav(_currentSoundIndex);
	loadWav(filename, 1);
	playWav(1);
}

void SoundManager::directPlayWav(const Common::String &file) {
	if (_soundOffFl)
		return;

	loadWav(file, 1);
	playWav(1);
}

void SoundManager::setMODSampleVolume() {
	for (int idx = 0; idx < SWAV_COUNT; ++idx) {
		if (idx != 20 && _sWav[idx]._active) {
			int volume = _soundVolume * 255 / 16;
			_vm->_mixer->setChannelVolume(_sWav[idx]._soundHandle, volume);
		}
	}
}

void SoundManager::setMODVoiceVolume() {
	if (_sWav[20]._active) {
		int volume = _voiceVolume * 255 / 16;
		_vm->_mixer->setChannelVolume(_sWav[20]._soundHandle, volume);
	}
}

void SoundManager::setMODMusicVolume(int volume) {
	if (_vm->_mixer->isSoundHandleActive(_musicHandle))
		_vm->_mixer->setChannelVolume(_musicHandle, volume * 255 / 16);
}

void SoundManager::loadSample(int wavIndex, const Common::String &file) {
	loadWavSample(wavIndex, file, false);
	_sound[wavIndex]._active = true;
}

void SoundManager::playSample(int wavIndex, int voiceMode) {
	if (_soundOffFl || !_sound[wavIndex]._active)
		return;

	if (_soundFl)
		delWav(_currentSoundIndex);

	switch (voiceMode) {
	case 5:
	// Case added to identify the former PLAY_SAMPLE2 calls
	case 9:
		if (checkVoiceStatus(1))
			stopVoice(1);
		playWavSample(1, wavIndex);
		break;
	case 6:
		if (checkVoiceStatus(2))
			stopVoice(1);
		playWavSample(2, wavIndex);
		break;
	default:
		break;
	}
}

bool SoundManager::checkVoiceStatus(int voiceIndex) {
	if (_voice[voiceIndex]._status) {
		int wavIndex = _voice[voiceIndex]._wavIndex;
		if (_sWav[wavIndex]._audioStream && _sWav[wavIndex]._audioStream->endOfStream())
			stopVoice(voiceIndex);
	}

	return _voice[voiceIndex]._status;
}

void SoundManager::stopVoice(int voiceIndex) {
	if (_voice[voiceIndex]._status) {
		_voice[voiceIndex]._status = false;
		int wavIndex = _voice[voiceIndex]._wavIndex;
		if (_sWav[wavIndex]._active && _sWav[wavIndex]._freeSampleFl)
			removeWavSample(wavIndex);
	}
	_voice[voiceIndex]._status = false;
}

void SoundManager::playVoice() {
	if (!_sWav[20]._active)
		return;

	if (!_voice[2]._status) {
		int wavIndex = _voice[2]._wavIndex;
		if (_sWav[wavIndex]._active && _sWav[wavIndex]._freeSampleFl)
			removeWavSample(wavIndex);
	}

	playWavSample(2, 20);
}

bool SoundManager::removeWavSample(int wavIndex) {
	if (!_sWav[wavIndex]._active)
		return false;

	_vm->_mixer->stopHandle(_sWav[wavIndex]._soundHandle);
	delete _sWav[wavIndex]._audioStream;
	_sWav[wavIndex]._audioStream = NULL;
	_sWav[wavIndex]._active = false;

	return true;
}

bool SoundManager::loadVoice(const Common::String &filename, size_t fileOffset, size_t entryLength, SwavItem &item) {
	Common::File f;
	if (!f.open(filename)) {
		// Fallback to APC...
		if (!f.open(setExtension(filename, ".APC"))) {
			// The English demo doesn't include the speech file.
			// This avoids it to crash when discussing with other characters
			if (!_vm->getIsDemo())
				error("Could not open %s for reading", filename.c_str());
			return false;
		}
	}

	f.seek(fileOffset);
	item._audioStream = makeSoundStream(f.readStream((entryLength == 0) ? f.size() : entryLength));
	f.close();

	return true;
}

void SoundManager::loadWavSample(int wavIndex, const Common::String &filename, bool freeSample) {
	if (_sWav[wavIndex]._active)
		removeWavSample(wavIndex);

	if (loadVoice(filename, 0, 0, _sWav[wavIndex])) {
		_sWav[wavIndex]._active = true;
		_sWav[wavIndex]._freeSampleFl = freeSample;
	} else{
		_sWav[wavIndex]._active = false;
	}
}

void SoundManager::loadWav(const Common::String &file, int wavIndex) {
	loadWavSample(wavIndex, file, true);
}

void SoundManager::playWav(int wavIndex) {
	if (_soundFl || _soundOffFl)
		return;

	_soundFl = true;
	_currentSoundIndex = wavIndex;
	playWavSample(1, wavIndex);
}

void SoundManager::delWav(int wavIndex) {
	if (!removeWavSample(wavIndex))
		return;

	if (checkVoiceStatus(1))
		stopVoice(1);

	_currentSoundIndex = 0;
	_soundFl = false;
}

void SoundManager::playWavSample(int voiceIndex, int wavIndex) {
	if (!_sWav[wavIndex]._active)
		warning("Bad handle");

	if (_voice[voiceIndex]._status && _sWav[wavIndex]._active && _sWav[wavIndex]._freeSampleFl)
		removeWavSample(wavIndex);

	_voice[voiceIndex]._status = true;
	_voice[voiceIndex]._wavIndex = wavIndex;

	int volume = (voiceIndex == 2) ? _voiceVolume * 255 / 16 : _soundVolume * 255 / 16;

	// If the handle is still in use, stop it. Otherwise we'll lose the
	// handle to that sound. This can currently happen (but probably
	// shouldn't) when skipping a movie.
	if (_vm->_mixer->isSoundHandleActive(_sWav[wavIndex]._soundHandle))
		  _vm->_mixer->stopHandle(_sWav[wavIndex]._soundHandle);

	// Start the voice playing
	_sWav[wavIndex]._audioStream->rewind();
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sWav[wavIndex]._soundHandle,
		_sWav[wavIndex]._audioStream, -1, volume, 0, DisposeAfterUse::NO);
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
		if (_sWav[idx]._active) {
			int volume = (idx == 20) ? (_voiceVolume * 255 / 16) : (_soundVolume * 255 / 16);
			_vm->_mixer->setChannelVolume(_sWav[idx]._soundHandle, volume);
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

/**
 * Creates an audio stream based on a passed raw stream
 */
Audio::RewindableAudioStream *SoundManager::makeSoundStream(Common::SeekableReadStream *stream) {
	if (_vm->getPlatform() == Common::kPlatformWindows)
		return makeAPCStream(stream, DisposeAfterUse::YES);
	else if (_vm->getPlatform() == Common::kPlatformLinux)
		return Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	else
		return Audio::makeRawStream(stream, 22050, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
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
