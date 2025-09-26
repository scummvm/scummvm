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
#include "common/util.h"
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

// Maps CD SFX IDs to floppy SFX IDs
static constexpr int sfxCdFloppyMapping[60] = {
	//  0
	 -1,	// Unused
	 10,	// House front door
	 11,	// Shower
	 -1,	// Delbert's dog (not present in floppy version)
	 13,	// Stairs
	 14,	// Press button
	 15,	// Pick up item
	 16,	// Unused (energizing hammer head?)
	 17,	// Lever
	 18,	// Spaceship
	// 10
	 19,	// Leech
	 20,	// Car engine start
	 21,	// Mausoleum sigils (only used by floppy version)
	 22,	// Mausoleum door (only used by CD version)
	 23,	// Shovel
	 24,	// Car door
	 25,	// Car engine start failure
	 26,	// Assembling hammer
	 27,	// Lock picking
	 28,	// Guardian
	// 20
	 29,	// Alien dog
	 30,	// Alien cop gun
	 12,	// Cup
	 32,	// Cocoon
	 90,	// Phone (loud)
	 91,	// Phone (quiet)
	118,	// Fixing mirror
	 93,	// Doorbell (loud)
	 94,	// Destroying mirror
	 95,	// Doorbell (quiet)
	// 30
	 96,	// Electricity
	 97,	// Car trunk / trunk (attic)
	 98,	// Drinking / pouring
	 99,	// Unused (tuning radio?)
	100,	// Bathroom cabinet
	101,	// Bathroom faucet
	 -1,	// Unused
	 -1,	// Unused (running water?)
	 -1,	// Unused (press button alternate?)
	111,	// Car horn
	// 40
	 -1,	// Unused
	116,	// Secret door
	115,	// Kitchen doors
	107,	// Clock tick (only used by floppy version)
	108,	// Clock tock (only used by floppy version)
	104,	// Clock chime (loud)
	106,	// Clock chime (quiet)
	113,	// Urn
	114,	// Teleporter
	 -1,	// Unused
	// 50	Floppy-only sound effects
	 35,	// Recruitment center
	 51,	// Car engine running (original code uses 50 here, which sounds wrong)
	 92,	// Footstep
	105,	// Mosquito (upstairs hallway)
	109,	// Book stamp
	110,	// Kitchen faucet / fountain
	112,	// Phone dial tone
	 -1,
	 -1,
	 -1
};

// Maps DOS CD speech IDs to DOS floppy SFX IDs
static Common::Pair<int, int> speechCdFloppyMapping[] = {
				  // 59: ralph1 (unused & invalid)
	{ 904,  60 }, // m1-1   Librarian (phone): "Hello?" "Hello Mike. This is Sue at the library. ..."
	{ 927,  61 }, // cl1    Store clerk: "Serve yourself, Mr. Dawson."
	{ 905,  62 }, // cl2    Store clerk: "That's the last bottle of Scotch. ..."
	{ 907,  63 }, // d4a-1  Delbert: "Hi you must be Mike. ..."
	{ 908,  64 }, // d6c-2  Delbert: "You're a writer, huh? ..."
	{ 909,  65 }, // d5a-1  Delbert: "Good to see you, Dawson. ..."
	{ 910,  66 }, // d6a-2  Delbert: "Boy, that's smooth. ..."
	{ 906,  67 }, // cl3    Store clerk: "I'm sorry, Mr. Dawson, ..."
				  // CD ID 925 includes both this line and the next. These are 2 separate IDs in the floppy version. CD ID 926 is unused.
	{ 925,  68 }, // gl0a   Librarian: "I'm not really sure why I'm here, ..."
	{ 926,  69 }, // gl1b   Librarian: "I know it sounds strange, ..."
	{ 924,  70 }, // gl2a   Librarian: "This card really should be kept with the book. ..."
				  // 71: s7a-1 (invalid)
	{ 912,  72 }, // s8a-2  Cop: "You're under arrest, Dawson. ..."
	{ 913,  73 }, // k9a-3  Keeper: "Greetings Michael. ..."
	{ 914,  74 }, // k9c-3  Keeper: "If born, this creature will destroy you..."
	{ 915,  75 }, // k9e-3  Keeper: "Also, the Police in your world..."
	{ 928,  76 }, // gl3a   Librarian: "Hi Mike. Here's the book that was put on hold for you."
				  // 77: k9e-3 (duplicate of 75)
				  // 78: k9f-3 (invalid)
	{ 916,  79 }, // g10a-1 Sargo: "Greetings, human. ..."
	{ 917,  80 }, // g10b-1 Sargo: "I am prepared to give you the gift..."
	{ 918,  81 }, // m11a-1 Mike: "I'm just beginning to understand."
	{ 919,  82 }, // o12a-1 Keeper (radio): "Steal from those who protect you..."
	{ 920,  83 }, // o13a-1 Keeper (radio): "What you do in the light..."
				  // 84: o13b-1 (invalid)
	{ 921,  85 }, // o14a-1 Keeper (radio): "Turn yourself in and leave behind the key..."
	{ 922,  86 }, // k15a-1 Keeper (phone): "Remember, anything seen in the mirror..."
	{ 923,  87 }, // s16a-1 Alien cop: "So that's where my gun went! ..."
				  // 88: l17a-1 (invalid)
				  // 89: l18a-1 (invalid)
	{  -1,  -1 }
};

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer), _lastPlayedDigitalSfx(0) {
	memset(_sfxFloppyDigFilenameTbl, 0, 120 * 14);

	bool floppyMusicSetting = ConfMan.hasKey("use_floppy_music") ? ConfMan.getBool("use_floppy_music") : false;
	_useFloppyMusic = g_engine->isDosFloppy() || floppyMusicSetting;

	// SFX mode 0: CD SFX only
	// SFX mode 1: CD SFX + additional floppy SFX
	// SFX mode 2: floppy SFX only
	// CD SFX are only available when using the CD version
	// Floppy SFX are only available when using floppy music
	int sfxMode = ConfMan.hasKey("sfx_mode") ? ConfMan.getInt("sfx_mode") : -1;
	_useCdSfx = g_engine->isCdVersion() && (!_useFloppyMusic || sfxMode != 2);
	_useFloppySfx = _useFloppyMusic && (g_engine->isDosFloppy() || sfxMode == 1 || sfxMode == 2);

	_musicPlayer = new MusicPlayer(g_engine, _useFloppyMusic, _useFloppySfx);

	_didSpeech.resize(978);
	resetSpeech();
}

Sound::~Sound() {
	delete _musicPlayer;
}

int Sound::init() {
	int returnCode = _musicPlayer->open();
	if (returnCode != 0)
		return returnCode;

	if (_useFloppyMusic || _useFloppySfx) {
		Common::File file;
		Common::Path path = g_engine->isCdVersion() ? Common::Path("sound").join("tos1.sit") : Common::Path("tos1.sit");
		if (file.open(path)) {
			_musicPlayer->loadTosInstrumentBankData(&file, (int32)file.size());
		} else {
			warning("Failed to load TOS floppy instrument bank data %s", path.toString().c_str());
		}
		file.close();
	}

	if (_useFloppySfx) {
		Common::File file;
		Common::Path path = g_engine->isCdVersion() ? Common::Path("sound").join("tos1.sbr") : Common::Path("tos1.sbr");
		if (file.open(path)) {
			_musicPlayer->load(&file, (int32)file.size(), true);
		} else {
			warning("Failed to load TOS floppy sound effects data %s", path.toString().c_str());
		}
		file.close();
	}

	if (g_engine->isDosFloppy()) {
		Common::File file;
		Common::Path path = Common::Path("tos1.dig");
		if (file.open(path)) {
			loadTosDigData(&file, (int32)file.size());
		} else {
			warning("Failed to load TOS floppy speech data %s", path.toString().c_str());
		}
		file.close();
	}

	return 0;
}

void Sound::loadTosDigData(Common::SeekableReadStream* in, int32 size) {
	int32 bytesRead = 0;
	int entriesRead = 0;
	while (bytesRead < size && entriesRead < 120) {
		byte type = in->readByte();
		if (type == 3) {
			// VOC filename entry
			uint32 entryBytesRead = in->read(_sfxFloppyDigFilenameTbl[entriesRead], 14);
			if (entryBytesRead != 14) {
				warning("Failed to read all bytes from DIG filename entry %i", entriesRead);
				return;
			}
			bytesRead += 15;
		}
		else if (type == 0) {
			// Unknown what this entry type contains. It is ignored by the original code.
			uint16 entrySize = in->readUint16LE();
			if (!in->skip(entrySize - 3)) {
				warning("Failed to read all bytes from DIG type 0 entry %i", entriesRead);
				return;
			}
			bytesRead += entrySize;
		}
		else {
			// Unknown entry type.
			warning("Unknown DIG entry type %X in entry %i", type, entriesRead);
			return;
		}
		entriesRead++;
	}
	if (entriesRead < 100) {
		// DIG files typically contain at least 100 entries
		warning("DIG file only contained %i entries", entriesRead);
	}
}

void Sound::playTosSpeech(int tosIdx) {
	if (g_engine->isDosFloppy()) {
		int floppySfxId = convertCdSpeechToFloppySfxId(tosIdx);
		if (floppySfxId == -1)
			return;

		playDosFloppySfx(floppySfxId, 5);
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
	return _mixer->isSoundHandleActive(_sfxHandle) || _musicPlayer->isPlayingSfx();
}

bool Sound::isPlayingSfx(uint8 sfxId) const {
	if (_useFloppySfx) {
		int floppySfxId = sfxCdFloppyMapping[sfxId];
		if (floppySfxId == -1)
			return false;

		return _musicPlayer->isPlayingSfx(floppySfxId);
	}

	return _lastPlayedDigitalSfx == sfxId && _mixer->isSoundHandleActive(_sfxHandle);
}

bool Sound::isPlayingMusic() {
	return _musicPlayer->isPlayingMusic();
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
	playMusic(_useFloppyMusic ?
			Common::String(musicDosFloppyFilenameTbl[filenameIdx]) + ".sbr" : musicDosCDFilenameTbl[filenameIdx],
		nullptr, 6, loop);
}

void Sound::playMusic(StartMusicId musicId) {
	int filenameIdx = static_cast<uint8>(musicId);
	if (_useFloppyMusic) {
		Common::String const &filenameBase = startMusicDosFloppyFilenameTbl[filenameIdx];
		Common::String const &filenameSbr = filenameBase + ".sbr";
		Common::String const &filenameSit = filenameBase + ".sit";

		playMusic(filenameSbr, &filenameSit, 5);
	}
	else {
		playMusic(startMusicDosCDFilenameTbl[filenameIdx]);
	}
}

void Sound::playMusic(Common::String const &musicFilename, Common::String const *instrBankFilename, uint8 priority, bool loop) {
	Common::File file;
	Common::Path path;
	if (_useFloppyMusic) {
		if (instrBankFilename != nullptr) {
			path = g_engine->isCdVersion() ? Common::Path("sound").join(instrBankFilename->c_str()) : Common::Path(instrBankFilename->c_str());
			debug("Loading instrument bank: %s", path.toString().c_str());
			if (!file.open(path)) {
				debug("Failed to load %s", path.toString().c_str());
				return;
			}
			_musicPlayer->loadInstrumentBank(&file, (int32)file.size());
			file.close();
		}
		else {
			debug("Loading TOS instrument bank");
			_musicPlayer->loadTosInstrumentBank();
		}
	}
	path = g_engine->isCdVersion() ? Common::Path("sound").join(musicFilename) : Common::Path(musicFilename);
	debug("Loading music: %s", path.toString().c_str());
	if (!file.open(path)) {
		debug("Failed to load %s", path.toString().c_str());
		return;
	}
	_musicPlayer->load(&file, (int32)file.size());
	file.close();

	_musicPlayer->playMusic(priority, loop);
}

void Sound::stopMusic() {
	_musicPlayer->stopMusic();
}

void Sound::pauseMusic(bool pause) {
	_musicPlayer->pauseMusic(pause);
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

void Sound::playSfx(uint8 sfxId, uint8 priority, int unk2) {
	// Do not play floppy-only SFX using the CD SFX playing code
	if (_useCdSfx && sfxId <= 48 && sfxId != 43 && sfxId != 44) {
		playDosCDSfx(sfxId);
	}
	else if (_useFloppySfx && sfxId < 60) {
		int floppySfxId = sfxCdFloppyMapping[sfxId];
		if (floppySfxId == -1)
			return;

		playDosFloppySfx(floppySfxId, priority);
	}
}

void Sound::stopSfx() {
	_mixer->stopHandle(_sfxHandle);
	_musicPlayer->stopAllSfx();
}

void Sound::playDosCDSfx(int sfxId) {
	if (sfxId == 0 || sfxId > 48) {
		return;
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
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, stream);
	_lastPlayedDigitalSfx = sfxId;
}

void Sound::playDosFloppySfx(byte floppySfxId, uint8 priority) {
	if (_musicPlayer->isSampleSfx(floppySfxId)) {
		playFloppySpeech(floppySfxId);
	} else if (floppySfxId >= 10 && floppySfxId < 120) {
		_musicPlayer->playSfx(floppySfxId, priority);
	}
}

void Sound::playFloppySpeech(int floppySfxId) {
	Common::String filename = _sfxFloppyDigFilenameTbl[floppySfxId];
	if (filename.size() == 0)
		return;

	Common::Path path = Common::Path(filename + ".voc");
	Common::File f;
	if (!f.open(path)) {
		warning("Failed to load speech file %s", path.toString().c_str());
		return;
	}

	Common::SeekableReadStream *srcStream = f.readStream((uint32)f.size());
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(srcStream,
															  Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);
}

void Sound::startFadeOut() {
	_musicPlayer->startFadeOutMusic();
}

bool Sound::isFading() {
	return _musicPlayer->isFadingMusic();
}

int Sound::convertCdSpeechToFloppySfxId(int cdSfxId) {
	int i = 0;
	while (true) {
		int entryCdSfxId = speechCdFloppyMapping[i].first;
		if (entryCdSfxId == -1)
			return -1;
		if (entryCdSfxId == cdSfxId)
			return speechCdFloppyMapping[i].second;
		i++;
	}
}

bool Sound::isUsingCdSfx() const {
	return _useCdSfx;
}

bool Sound::isUsingFloppySfx() const {
	return _useFloppySfx;
}

} // End of namespace Darkseed
