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
 * $URL$
 * $Id$
 *
 */

#include "gob/gob.h"
#include "gob/sound/sound.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/inter.h"

#include "gob/sound/pcspeaker.h"
#include "gob/sound/soundblaster.h"
#include "gob/sound/adlib.h"
#include "gob/sound/infogrames.h"
#include "gob/sound/protracker.h"
#include "gob/sound/cdrom.h"

namespace Gob {

Sound::Sound(GobEngine *vm) : _vm(vm) {
	_pcspeaker = new PCSpeaker(*_vm->_mixer);
	_blaster = new SoundBlaster(*_vm->_mixer);

	_adlPlayer = 0;
	_mdyPlayer = 0;
	_infogrames = 0;
	_protracker = 0;
	_cdrom = 0;
	_bgatmos = 0;

	_hasAdLib = (!_vm->_noMusic && _vm->hasAdlib());

	if (!_vm->_noMusic && (_vm->getPlatform() == Common::kPlatformAmiga)) {
		_infogrames = new Infogrames(*_vm->_mixer);
		_protracker = new Protracker(*_vm->_mixer);
	}
	if (_vm->isCD())
		_cdrom = new CDROM;
	if (_vm->getGameType() == kGameTypeWoodruff)
		_bgatmos = new BackgroundAtmosphere(*_vm->_mixer);
	if (_vm->getGameType() == kGameTypeUrban) {
		_bgatmos = new BackgroundAtmosphere(*_vm->_mixer);
		_bgatmos->setShadable(false);
	}
}

Sound::~Sound() {
	delete _pcspeaker;
	delete _blaster;
	delete _adlPlayer;
	delete _mdyPlayer;
	delete _infogrames;
	delete _protracker;
	delete _cdrom;
	delete _bgatmos;

	for (int i = 0; i < kSoundsCount; i++)
		_sounds[i].free();
}

void Sound::convToSigned(byte *buffer, int length) {
	while (length-- > 0)
		*buffer++ ^= 0x80;
}

SoundDesc *Sound::sampleGetBySlot(int slot) {
	if ((slot < 0) || (slot >= kSoundsCount))
		return 0;

	return &_sounds[slot];
}

const SoundDesc *Sound::sampleGetBySlot(int slot) const {
	if ((slot < 0) || (slot >= kSoundsCount))
		return 0;

	return &_sounds[slot];
}

int Sound::sampleGetNextFreeSlot() const {
	for (int i = 0; i < kSoundsCount; i++)
		if (_sounds[i].empty())
			return i;

	return -1;
}

bool Sound::sampleLoad(SoundDesc *sndDesc, SoundType type, const char *fileName, bool tryExist) {
	if (!sndDesc)
		return false;

	debugC(2, kDebugSound, "Loading sample \"%s\"", fileName);

	if (!_vm->_dataIO->existData(fileName)) {
		warning("Can't open sample file \"%s\"", fileName);
		return false;
	}

	byte *data;
	uint32 size;

	data = (byte *) _vm->_dataIO->getData(fileName);
	if (!data)
		return false;

	size = _vm->_dataIO->getDataSize(fileName);
	return sndDesc->load(type, SOUND_FILE, data, size);
}

void Sound::sampleFree(SoundDesc *sndDesc, bool noteAdlib, int index) {
	if (!sndDesc || sndDesc->empty())
		return;

	if (sndDesc->getType() == SOUND_ADL) {

		if (noteAdlib) {
			if (_adlPlayer)
				if ((index == -1) || (_adlPlayer->getIndex() == index))
					_adlPlayer->stopPlay();
			if (_mdyPlayer)
				if ((index == -1) || (_mdyPlayer->getIndex() == index))
					_mdyPlayer->stopPlay();
		}

	} else {

		if (_blaster)
			_blaster->stopSound(0, sndDesc);

	}

	sndDesc->free();
}

void Sound::speakerOn(int16 frequency, int32 length) {
	if (!_pcspeaker)
		return;

	debugC(1, kDebugSound, "PCSpeaker: Playing tone (%d, %d)", frequency, length);

	_pcspeaker->speakerOn(frequency, length);
}

void Sound::speakerOff() {
	if (!_pcspeaker)
		return;

	debugC(1, kDebugSound, "PCSpeaker: Stopping tone");

	_pcspeaker->speakerOff();
}

void Sound::speakerOnUpdate(uint32 millis) {
	if (!_pcspeaker)
		return;

	_pcspeaker->onUpdate(millis);
}

bool Sound::infogramesLoadInstruments(const char *fileName) {
	if (!_infogrames)
		return false;

	debugC(1, kDebugSound, "Infogrames: Loading instruments \"%s\"", fileName);

	return _infogrames->loadInstruments(fileName);
}

bool Sound::infogramesLoadSong(const char *fileName) {
	if (!_infogrames)
		return false;

	debugC(1, kDebugSound, "Infogrames: Loading song \"%s\"", fileName);

	return _infogrames->loadSong(fileName);
}

bool Sound::protrackerPlay(const char *fileName) {
	if (!_protracker)
		return false;

	debugC(1, kDebugSound, "Protracker: Playing song \"%s\"", fileName);

	return _protracker->play(fileName);
}

void Sound::protrackerStop() {
	if (!_protracker)
		return;

	debugC(1, kDebugSound, "Protracker: Stopping playback");

	_protracker->stop();
}

void Sound::infogramesPlay() {
	if (!_infogrames)
		return;

	debugC(1, kDebugSound, "Infogrames: Starting playback");

	_infogrames->play();
}

void Sound::infogramesStop() {
	if (!_infogrames)
		return;

	debugC(1, kDebugSound, "Infogrames: Stopping playback");

	_infogrames->stop();
}

bool Sound::adlibLoadADL(const char *fileName) {
	if (!_hasAdLib)
		return false;

	if (!_adlPlayer)
		_adlPlayer = new ADLPlayer(*_vm->_mixer);

	debugC(1, kDebugSound, "Adlib: Loading ADL data (\"%s\")", fileName);

	return _adlPlayer->load(fileName);
}

bool Sound::adlibLoadADL(byte *data, uint32 size, int index) {
	if (!_hasAdLib)
		return false;

	if (!_adlPlayer)
		_adlPlayer = new ADLPlayer(*_vm->_mixer);

	debugC(1, kDebugSound, "Adlib: Loading ADL data (%d)", index);

	return _adlPlayer->load(data, size, index);
}

void Sound::adlibUnload() {
	if (!_hasAdLib)
		return;

	debugC(1, kDebugSound, "Adlib: Unloading data");

	if (_adlPlayer)
		_adlPlayer->unload();
	if (_mdyPlayer)
		_mdyPlayer->unload();
}

bool Sound::adlibLoadMDY(const char *fileName) {
	if (!_hasAdLib)
		return false;

	if (!_mdyPlayer)
		_mdyPlayer = new MDYPlayer(*_vm->_mixer);

	debugC(1, kDebugSound, "Adlib: Loading MDY data (\"%s\")", fileName);

	return _mdyPlayer->loadMDY(fileName);
}

bool Sound::adlibLoadTBR(const char *fileName) {
	if (!_hasAdLib)
		return false;

	if (!_mdyPlayer)
		_mdyPlayer = new MDYPlayer(*_vm->_mixer);

	debugC(1, kDebugSound, "Adlib: Loading MDY instruments (\"%s\")", fileName);

	return _mdyPlayer->loadTBR(fileName);
}

void Sound::adlibPlayTrack(const char *trackname) {
	if (!_hasAdLib)
		return;

	if (!_adlPlayer)
		_adlPlayer = new ADLPlayer(*_vm->_mixer);

	if (_adlPlayer->isPlaying())
		return;

	debugC(1, kDebugSound, "Adlib: Playing ADL track \"%s\"", trackname);

	_adlPlayer->unload();
	_adlPlayer->load(trackname);
	_adlPlayer->startPlay();
}

void Sound::adlibPlayBgMusic() {
	if (!_hasAdLib)
		return;

	if (!_adlPlayer)
		_adlPlayer = new ADLPlayer(*_vm->_mixer);

	static const char *tracksMac[] = {
//		"musmac1.adl", // TODO: This track isn't played correctly at all yet
		"musmac2.adl",
		"musmac3.adl",
		"musmac4.adl",
		"musmac5.adl",
		"musmac6.adl"
	};

	static const char *tracksWin[] = {
		"musmac1.mid",
		"musmac2.mid",
		"musmac3.mid",
		"musmac4.mid",
		"musmac5.mid"
	};

	if (_vm->getPlatform() == Common::kPlatformWindows) {
		int track = _vm->_util->getRandom(ARRAYSIZE(tracksWin));
		adlibPlayTrack(tracksWin[track]);
	} else {
		int track = _vm->_util->getRandom(ARRAYSIZE(tracksMac));
		adlibPlayTrack(tracksMac[track]);
	}
}

void Sound::adlibPlay() {
	if (!_hasAdLib)
		return;

	debugC(1, kDebugSound, "Adlib: Starting playback");

	if (_adlPlayer)
		_adlPlayer->startPlay();
	if (_mdyPlayer)
		_mdyPlayer->startPlay();
}

void Sound::adlibStop() {
	if (!_hasAdLib)
		return;

	debugC(1, kDebugSound, "Adlib: Stopping playback");

	if (_adlPlayer)
		_adlPlayer->stopPlay();
	if (_mdyPlayer)
		_mdyPlayer->stopPlay();
}

bool Sound::adlibIsPlaying() const {
	if (!_hasAdLib)
		return false;

	if (_adlPlayer && _adlPlayer->isPlaying())
		return true;
	if (_mdyPlayer && _mdyPlayer->isPlaying())
		return true;

	return false;
}

int Sound::adlibGetIndex() const {
	if (!_hasAdLib)
		return -1;

	if (_adlPlayer)
		return _adlPlayer->getIndex();
	if (_mdyPlayer)
		return _mdyPlayer->getIndex();

	return -1;
}

bool Sound::adlibGetRepeating() const {
	if (!_hasAdLib)
		return false;

	if (_adlPlayer)
		return _adlPlayer->getRepeating();
	if (_mdyPlayer)
		return _mdyPlayer->getRepeating();

	return false;
}

void Sound::adlibSetRepeating(int32 repCount) {
	if (!_hasAdLib)
		return;

	if (_adlPlayer)
		_adlPlayer->setRepeating(repCount);
	if (_mdyPlayer)
		_mdyPlayer->setRepeating(repCount);
}

void Sound::blasterPlay(SoundDesc *sndDesc, int16 repCount,
		int16 frequency, int16 fadeLength) {
	if (!_blaster || !sndDesc)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Playing sample (%d, %d, %d)",
			repCount, frequency, fadeLength);

	_blaster->playSample(*sndDesc, repCount, frequency, fadeLength);
}

void Sound::blasterStop(int16 fadeLength, SoundDesc *sndDesc) {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Stopping playback");

	_blaster->stopSound(fadeLength, sndDesc);
}

void Sound::blasterPlayComposition(int16 *composition, int16 freqVal,
		SoundDesc *sndDescs, int8 sndCount) {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Playing composition (%d, %d)",
			freqVal, sndCount);

	blasterWaitEndPlay();
	_blaster->stopComposition();

	if (!sndDescs)
		sndDescs = _sounds;

	_blaster->playComposition(composition, freqVal, sndDescs, sndCount);
}

void Sound::blasterStopComposition() {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Stopping composition");

	_blaster->stopComposition();
}

char Sound::blasterPlayingSound() const {
	if (!_blaster)
		return 0;

	return _blaster->getPlayingSound();
}

void Sound::blasterSetRepeating(int32 repCount) {
	if (!_blaster)
		return;

	_blaster->setRepeating(repCount);
}

void Sound::blasterWaitEndPlay(bool interruptible, bool stopComp) {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Waiting for playback to end");

	if (stopComp)
		_blaster->endComposition();

	while (_blaster->isPlaying() && !_vm->shouldQuit()) {
		if (interruptible && (_vm->_util->checkKey() == 0x11B)) {
			WRITE_VAR(57, (uint32) -1);
			return;
		}
		_vm->_util->longDelay(200);
	}

	_blaster->stopSound(0);
}

void Sound::cdLoadLIC(const char *fname) {
	if (!_cdrom)
		return;

	debugC(1, kDebugSound, "CDROM: Loading LIC \"%s\"", fname);

	if (!_vm->_dataIO->existData(fname))
		return;

	_vm->_dataIO->getUnpackedData(fname);

	DataStream *stream = _vm->_dataIO->getDataStream(fname);

	_cdrom->readLIC(*stream);

	delete stream;
}

void Sound::cdUnloadLIC() {
	if (!_cdrom)
		return;

	debugC(1, kDebugSound, "CDROM: Unloading LIC");

	_cdrom->freeLICBuffer();
}

void Sound::cdPlayBgMusic() {
	if (!_cdrom)
		return;

	static const char *tracks[][2] = {
		{"avt00.tot",  "mine"},
		{"avt001.tot", "nuit"},
		{"avt002.tot", "campagne"},
		{"avt003.tot", "extsor1"},
		{"avt004.tot", "interieure"},
		{"avt005.tot", "zombie"},
		{"avt006.tot", "zombie"},
		{"avt007.tot", "campagne"},
		{"avt008.tot", "campagne"},
		{"avt009.tot", "extsor1"},
		{"avt010.tot", "extsor1"},
		{"avt011.tot", "interieure"},
		{"avt012.tot", "zombie"},
		{"avt014.tot", "nuit"},
		{"avt015.tot", "interieure"},
		{"avt016.tot", "statue"},
		{"avt017.tot", "zombie"},
		{"avt018.tot", "statue"},
		{"avt019.tot", "mine"},
		{"avt020.tot", "statue"},
		{"avt021.tot", "mine"},
		{"avt022.tot", "zombie"}
	};

	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (!scumm_stricmp(_vm->_game->_curTotFile, tracks[i][0])) {
			debugC(1, kDebugSound, "CDROM: Playing background music \"%s\" (\"%s\")",
					tracks[i][1], _vm->_game->_curTotFile);

			_cdrom->startTrack(tracks[i][1]);
			break;
		}
}

void Sound::cdPlayMultMusic() {
	if (!_cdrom)
		return;

	static const char *tracks[][6] = {
		{"avt005.tot", "fra1", "all1", "ang1", "esp1", "ita1"},
		{"avt006.tot", "fra2", "all2", "ang2", "esp2", "ita2"},
		{"avt012.tot", "fra3", "all3", "ang3", "esp3", "ita3"},
		{"avt016.tot", "fra4", "all4", "ang4", "esp4", "ita4"},
		{"avt019.tot", "fra5", "all5", "ang5", "esp5", "ita5"},
		{"avt022.tot", "fra6", "all6", "ang6", "esp6", "ita6"}
	};

	// Default to "ang?" for other languages (including EN_USA)
	int language = _vm->_global->_language <= 4 ? _vm->_global->_language : 2;
	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (!scumm_stricmp(_vm->_game->_curTotFile, tracks[i][0])) {
			debugC(1, kDebugSound, "CDROM: Playing mult music \"%s\" (\"%s\")",
					tracks[i][language + 1], _vm->_game->_curTotFile);

			_cdrom->startTrack(tracks[i][language + 1]);
			break;
		}
}

void Sound::cdPlay(const char *trackName) {
	if (!_cdrom)
		return;

	debugC(1, kDebugSound, "CDROM: Playing track \"%s\"", trackName);
	_cdrom->startTrack(trackName);
}

void Sound::cdStop() {
	if (!_cdrom)
		return;

	debugC(1, kDebugSound, "CDROM: Stopping playback");
	_cdrom->stopPlaying();
}

bool Sound::cdIsPlaying() const {
	if (!_cdrom)
		return false;

	return _cdrom->isPlaying();
}

int32 Sound::cdGetTrackPos(const char *keyTrack) const {
	if (!_cdrom)
		return -1;

	return _cdrom->getTrackPos(keyTrack);
}

const char *Sound::cdGetCurrentTrack() const {
	if (!_cdrom)
		return "";

	return _cdrom->getCurTrack();
}

void Sound::cdTest(int trySubst, const char *label) {
	if (!_cdrom)
		return;

	_cdrom->testCD(trySubst, label);
}

void Sound::bgPlay(const char *file, SoundType type) {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Playing \"%s\"", file);

	_bgatmos->stop();
	_bgatmos->queueClear();

	SoundDesc *sndDesc = new SoundDesc;
	if (!sampleLoad(sndDesc, type, file)) {
		delete sndDesc;
		return;
	}

	_bgatmos->queueSample(*sndDesc);
	_bgatmos->play();
}

void Sound::bgPlay(const char *base, const char *ext, SoundType type, int count) {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Playing \"%s\" (%d)", base, count);

	_bgatmos->stop();
	_bgatmos->queueClear();

	int length = strlen(base) + 7;
	char *fileName = new char[length];
	SoundDesc *sndDesc;

	for (int i = 1; i <= count; i++) {
		snprintf(fileName, length, "%s%02d.%s", base, i, ext);

		sndDesc = new SoundDesc;
		if (sampleLoad(sndDesc, type, fileName))
			_bgatmos->queueSample(*sndDesc);
		else
			delete sndDesc;
	}

	_bgatmos->play();
}

void Sound::bgStop() {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Stopping playback");

	_bgatmos->stop();
	_bgatmos->queueClear();
}

void Sound::bgSetPlayMode(BackgroundAtmosphere::PlayMode mode) {
	if (!_bgatmos)
		return;

	_bgatmos->setPlayMode(mode);
}

void Sound::bgShade() {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Shading playback");

	_bgatmos->shade();
}

void Sound::bgUnshade() {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Unshading playback");

	_bgatmos->unshade();
}

} // End of namespace Gob
