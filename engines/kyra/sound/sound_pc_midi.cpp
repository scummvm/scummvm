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

#include "kyra/sound/sound_intern.h"

#include "kyra/resource/resource.h"

#include "common/system.h"
#include "common/config-manager.h"
#include "common/translation.h"

#include "gui/message.h"

namespace Kyra {

SoundMidiPC::SoundMidiPC(KyraEngine_v1 *vm, Audio::Mixer *mixer, MidiDriver *driver, kType type) : Sound(vm, mixer) {
	_driver = driver;
	_output = 0;

	_musicFile = _sfxFile = 0;
	_currentResourceSet = 0;
	memset(&_resInfo, 0, sizeof(_resInfo));

	_music = MidiParser::createParser_XMIDI(MidiParser::defaultXMidiCallback, NULL, 0);
	assert(_music);
	_music->property(MidiParser::mpDisableAllNotesOffMidiEvents, true);
	_music->property(MidiParser::mpDisableAutoStartPlayback, true);
	for (int i = 0; i < 3; ++i) {
		_sfx[i] = MidiParser::createParser_XMIDI(MidiParser::defaultXMidiCallback, NULL, i + 1);
		assert(_sfx[i]);
		_sfx[i]->property(MidiParser::mpDisableAllNotesOffMidiEvents, true);
		_sfx[i]->property(MidiParser::mpDisableAutoStartPlayback, true);
	}

	_musicVolume = _sfxVolume = 0;
	_fadeMusicOut = false;

	_type = type;
	assert(_type == kMidiMT32 || _type == kMidiGM || _type == kPCSpkr);

	// Only General MIDI isn't a Roland MT-32 MIDI implemenation,
	// even the PC Speaker driver is a Roland MT-32 based MIDI implementation.
	// Thus we set "_nativeMT32" for all types except Gerneral MIDI to true.
	_nativeMT32 = (_type != kMidiGM);

	// KYRA1 does not include any General MIDI tracks, thus we have
	// to overwrite the internal type with MT32 to get the correct
	// file extension.
	if (_vm->game() == GI_KYRA1 && _type == kMidiGM)
		_type = kMidiMT32;

	// Display a warning about possibly wrong sound when the user only has
	// a General MIDI device, but the game is setup to use Roland MT32 MIDI.
	// (This will only happen in The Legend of Kyrandia 1 though, all other
	// supported games include special General MIDI tracks).
	if (_type == kMidiMT32 && !_nativeMT32) {
		::GUI::MessageDialog dialog(_("You appear to be using a General MIDI device,\n"
									"but your game only supports Roland MT32 MIDI.\n"
									"We try to map the Roland MT32 instruments to\n"
									"General MIDI ones. It is still possible that\n"
									"some tracks sound incorrect."));
		dialog.runModal();
	}
}

SoundMidiPC::~SoundMidiPC() {
	Common::StackLock lock(_mutex);
	_output->setTimerCallback(0, 0);

	delete _music;
	for (int i = 0; i < 3; ++i)
		delete _sfx[i];
	_output->stopAllNotes();

	delete _output; // This automatically frees _driver (!)

	if (_musicFile != _sfxFile)
		delete[] _sfxFile;

	delete[] _musicFile;

	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundMidiPC::init() {
	_output = Audio::MidiDriver_Miles_MIDI_create(_type == kMidiGM ? MT_GM : MT_MT32, "");
	assert(_output);
	int returnCode = _output->open(_driver, _nativeMT32);
	if (returnCode > 0) {
		return false;
	}

	updateVolumeSettings();

	_music->setMidiDriver(_output);
	_music->setTempo(_output->getBaseTempo());
	_music->setTimerRate(_output->getBaseTempo());

	for (int i = 0; i < 3; ++i) {
		_sfx[i]->setMidiDriver(_output);
		_sfx[i]->setTempo(_output->getBaseTempo());
		_sfx[i]->setTimerRate(_output->getBaseTempo());
	}

	_output->setTimerCallback(this, SoundMidiPC::onTimer);

	// Load MT-32 and GM initialization files
	const char* midiFile = 0;
	const char* pakFile = 0;
	if (_nativeMT32 && _type == kMidiMT32) {
		if (_vm->game() == GI_KYRA1) {
			midiFile = "INTRO";
		} else if (_vm->game() == GI_KYRA2) {
			midiFile = "HOF_SYX";
			pakFile = "AUDIO.PAK";
		} else if (_vm->game() == GI_LOL) {
			midiFile = "LOREINTR";

			if (_vm->gameFlags().isDemo) {
				if (_vm->resource()->exists("INTROVOC.PAK")) {
					// Intro demo
					pakFile = "INTROVOC.PAK";
				} else {
					// Kyra2 SEQ player based demo
					pakFile = "GENERAL.PAK";
					midiFile = "LOLSYSEX";
				}
			} else {
				if (_vm->gameFlags().isTalkie)
					pakFile = "ENG/STARTUP.PAK";
				else
					pakFile = "INTROVOC.PAK";
			}
		}
	} else if (_type == kMidiGM && _vm->game() == GI_LOL) {
		if (_vm->gameFlags().isDemo && _vm->resource()->exists("INTROVOC.PAK")) {
			// Intro demo
			midiFile = "LOREINTR";
			pakFile = "INTROVOC.PAK";
		} else {
			midiFile = "LOLSYSEX";
			pakFile = "GENERAL.PAK";
		}
	}

	if (!midiFile)
		return true;

	if (pakFile)
		_vm->resource()->loadPakFile(pakFile);

	loadSoundFile(midiFile);
	playTrack(0);

	Common::Event event;
	while (isPlaying() && !_vm->shouldQuit()) {
		_vm->_system->updateScreen();
		_vm->_eventMan->pollEvent(event);
		_vm->_system->delayMillis(10);
	}

	if (pakFile)
		_vm->resource()->unloadPakFile(pakFile);

	return true;
}

void SoundMidiPC::updateVolumeSettings() {
	Common::StackLock lock(_mutex);

	if (!_output)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	const int newMusVol = (mute ? 0 : ConfMan.getInt("music_volume"));
	_sfxVolume = (mute ? 0 : ConfMan.getInt("sfx_volume"));

	_output->setSourceVolume(0, newMusVol);
	_musicVolume = newMusVol;

	for (int i = 1; i < 4; ++i)
		_output->setSourceVolume(i, _sfxVolume);
}

void SoundMidiPC::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : 0;
	}
}

void SoundMidiPC::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundMidiPC::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundMidiPC::loadSoundFile(uint file) {
	if (file < res()->fileListSize)
		loadSoundFile(res()->fileList[file]);
}

void SoundMidiPC::loadSoundFile(Common::String file) {
	Common::StackLock lock(_mutex);
	file = getFileName(file);

	if (_mFileName == file)
		return;

	if (!_vm->resource()->exists(file.c_str()))
		return;

	haltTrack();
	if (_vm->game() == GI_KYRA1) {
		stopAllSoundEffects();
	}

	delete[] _musicFile;
	uint32 fileSize = 0;
	_musicFile = _vm->resource()->fileData(file.c_str(), &fileSize);
	_mFileName = file;

	_music->loadMusic(_musicFile, fileSize);

	// Since KYRA1 uses the same file for SFX and Music
	// we setup sfx to play from music file as well
	if (_vm->game() == GI_KYRA1) {
		for (int i = 0; i < 3; ++i) {
			_sfx[i]->loadMusic(_musicFile, fileSize);
		}
	}
}

void SoundMidiPC::loadSfxFile(Common::String file) {
	Common::StackLock lock(_mutex);

	// Kyrandia 1 doesn't use a special sfx file
	if (_vm->game() == GI_KYRA1)
		return;

	file = getFileName(file);

	if (_sFileName == file)
		return;

	if (!_vm->resource()->exists(file.c_str()))
		return;

	stopAllSoundEffects();

	delete[] _sfxFile;

	uint32 fileSize = 0;
	_sfxFile = _vm->resource()->fileData(file.c_str(), &fileSize);
	_sFileName = file;

	for (int i = 0; i < 3; ++i) {
		_sfx[i]->loadMusic(_sfxFile, fileSize);
		_sfx[i]->stopPlaying();
	}
}

void SoundMidiPC::playTrack(uint8 track) {
	if (!_musicEnabled)
		return;

	haltTrack();

	Common::StackLock lock(_mutex);
	_fadeMusicOut = false;

	_output->setSourceVolume(0, _musicVolume);

	if (_music->setTrack(track))
		_music->startPlaying();
}

void SoundMidiPC::haltTrack() {
	Common::StackLock lock(_mutex);

	_music->stopPlaying();
	_output->deinitSource(0);
}

bool SoundMidiPC::isPlaying() const {
	Common::StackLock lock(_mutex);

	return _music->isPlaying();
}

void SoundMidiPC::playSoundEffect(uint16 track, uint8) {
	if (!_sfxEnabled)
		return;

	Common::StackLock lock(_mutex);
	for (int i = 0; i < 3; ++i) {
		if (!_sfx[i]->isPlaying()) {
			if (_sfx[i]->setTrack(track))
				_sfx[i]->startPlaying();
			return;
		}
	}
}

void SoundMidiPC::stopAllSoundEffects() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < 3; ++i) {
		_sfx[i]->stopPlaying();
		_output->deinitSource(i+1);
	}
}

void SoundMidiPC::beginFadeOut() {
	Common::StackLock lock(_mutex);

	_fadeMusicOut = true;
	_fadeStartTime = _vm->_system->getMillis();
}

void SoundMidiPC::pause(bool paused) {
	Common::StackLock lock(_mutex);

	if (paused) {
		_music->pausePlaying();
		for (int i = 0; i < 3; i++)
			_sfx[i]->pausePlaying();
		if (_output)
			_output->stopAllNotes();
	} else {
		_music->resumePlaying();
		for (int i = 0; i < 3; ++i)
			_sfx[i]->resumePlaying();
		// Possible TODO (IMHO unnecessary): restore notes and/or update _fadeStartTime
	}
}

void SoundMidiPC::onTimer(void *data) {
	SoundMidiPC *midi = (SoundMidiPC *)data;

	Common::StackLock lock(midi->_mutex);

	if (midi->_fadeMusicOut) {
		static const uint32 musicFadeTime = 1 * 1000;

		if (midi->_fadeStartTime + musicFadeTime > midi->_vm->_system->getMillis()) {
			int volume = (byte)((musicFadeTime - (midi->_vm->_system->getMillis() - midi->_fadeStartTime)) * midi->_musicVolume / musicFadeTime);
			midi->_output->setSourceVolume(0, volume);
		} else {
			midi->haltTrack();
			midi->stopAllSoundEffects();

			midi->_fadeMusicOut = false;

			// Restore music volume
			midi->_output->setSourceVolume(0, midi->_musicVolume);
		}
	}

	midi->_music->onTimer();

	for (int i = 0; i < 3; ++i) {
		midi->_sfx[i]->onTimer();
	}
}

Common::String SoundMidiPC::getFileName(const Common::String &str) {
	Common::String file = str;
	if (_type == kMidiMT32)
		file += ".XMI";
	else if (_type == kMidiGM)
		file += ".C55";
	else if (_type == kPCSpkr)
		file += ".PCS";

	if (_vm->resource()->exists(file.c_str()))
		return file;

	return str + ".XMI";
}

} // End of namespace Kyra
