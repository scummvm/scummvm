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
#include "kyra/sound/drivers/audiomaster2.h"

#include "common/memstream.h"

namespace Kyra {

SoundAmiga_EoB::SoundAmiga_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer) : Sound(vm, mixer), _vm(vm), _driver(0), _currentResourceSet(0), _ready(false) {
	_fileBuffer = new uint8[64000];
	_version2 = _vm->game() == GI_EOB2;
	memset(_resInfo, 0, sizeof(_resInfo));
}

SoundAmiga_EoB::~SoundAmiga_EoB() {
	delete _driver;
	delete[] _fileBuffer;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

Sound::kType SoundAmiga_EoB::getMusicType() const {
	return kAmiga;
}

bool SoundAmiga_EoB::init() {
	_driver = new AudioMaster2(_mixer);
	if (!_driver->init())
		return false;

	_ready = true;
	return true;
}

void SoundAmiga_EoB::initAudioResourceInfo(int set, void *info) {
	delete _resInfo[set];
	_resInfo[set] = info ? new SoundResourceInfo_AmigaEoB(*(SoundResourceInfo_AmigaEoB*)info) : 0;
}

void SoundAmiga_EoB::selectAudioResourceSet(int set) {
	_driver->flushAllResources();
	if (!_resInfo[set])
		return;

	for (uint i = 0; i < _resInfo[set]->fileListSize; ++i)
		loadSoundFile(_resInfo[set]->fileList[i]);

	_currentResourceSet = set;
}

bool SoundAmiga_EoB::hasSoundFile(uint file) const {
	return false;
}

void SoundAmiga_EoB::loadSoundFile(uint file) {
	/*
	_sound->loadSoundFile("INTRO1.CPS");
	_sound->loadSoundFile("INTRO2.CPS");
	_sound->loadSoundFile("INTRO4.CPS");
	_sound->loadSoundFile("INTRO5.CPS");
	_sound->loadSoundFile("NEWINTRO1.CPS");
	_sound->loadSoundFile("CHARGEN1.CPS");

	_sound->loadSoundFile("SFX1.CPS");
	_sound->loadSoundFile("SFX2.CPS");
	_sound->loadSoundFile("SFX3.CPS");
	_sound->loadSoundFile("SFX4.CPS");

	_sound->loadSoundFile("HUM1.CPS");

	_sound->loadSoundFile("SPIDERMOV1.CPS");
	_sound->loadSoundFile("MOVE1.CPS");
	_sound->loadSoundFile("MOVE21.CPS");
	_sound->loadSoundFile("MOVE31.CPS");
	_sound->loadSoundFile("BEASTATK1.CPS");
	_sound->loadSoundFile("BLADE1.CPS");
	_sound->loadSoundFile("MANTISMOV1.CPS");
	_sound->loadSoundFile("FLAYERATK1.CPS");
	_sound->loadSoundFile("LEECHMOV1.CPS");
	_sound->loadSoundFile("SLOSHSUCK1.CPS");
	_sound->loadSoundFile("SCREAM1.CPS");
	_sound->loadSoundFile("RUSTATK1.CPS");
	_sound->loadSoundFile("HOUNDATK1.CPS");
	_sound->loadSoundFile("KUOTOAMOV1.CPS");

	_sound->loadSoundFile("FINALE2.CPS");
	_sound->loadSoundFile("FINALE1.CPS");
	_sound->loadSoundFile("FINALE.CPS");

	for (int i = 1; i < 12; ++i) {
		char n[13];
		sprintf(n, "LEVELSAM%d.CPS", i);
		_sound->loadSoundFile(n);
	}*/
}

void SoundAmiga_EoB::loadSoundFile(Common::String file) {
	if (!_ready)
		return;

	Common::SeekableReadStream *in = _vm->resource()->createReadStream(file);
	if (!in)
		return;

	uint16 readSize = in->readUint16LE() - 10;
	uint8 cmp = in->readByte();
	in->seek(1, SEEK_CUR);
	uint32 outSize = in->readUint32LE();
	in->seek(2, SEEK_CUR);

	if (in->read(_fileBuffer, readSize) != readSize)
		error("SoundAmiga_EoB::loadSoundFile(): Failed to load sound file '%s'", file.c_str());
	delete in;

	uint8 *buf = new uint8[outSize];

	if (cmp == 0) {
		assert(readSize == outSize);
		memcpy(buf, _fileBuffer, outSize);
	} else if (cmp == 3) {			
		Screen::decodeFrame3(_fileBuffer, buf, outSize);
	} else if (cmp == 4) {
		Screen::decodeFrame4(_fileBuffer, buf, outSize);
	} else {
		error("SoundAmiga_EoB::loadSoundFile(): Failed to load sound file '%s'", file.c_str());
	}

	Common::MemoryReadStream soundFile(buf, outSize);
	if (!_driver->loadRessourceFile(&soundFile))
		error("SoundAmiga_EoB::loadSoundFile(): Failed to load sound file '%s'", file.c_str());

	delete[] buf;
}

void SoundAmiga_EoB::playTrack(uint8 track) {
	_driver->startSound("newintro1.smus");
	//_driver->startSound("hum1.sam");
	//_driver->startSound("hum");
	//_driver->startSound("playswing");
	//_driver->startSound("door");
	//_driver->startSound("death");
	//_driver->startSound("teleport");
	//_driver->startSound("scream");
	//_driver->startSound("magica");
	//_driver->startSound("magicb");

	//_driver->startSound("chargen1.smus");
	//_driver->startSound("finale.smus");
}

void SoundAmiga_EoB::haltTrack() {

}

void SoundAmiga_EoB::playSoundEffect(uint8 track, uint8 volume) {
	if (!_resInfo[_currentResourceSet]->soundList || track >= 120 || !_sfxEnabled)
		return;

	_driver->startSound(_resInfo[_currentResourceSet]->soundList[track]);

	static const char *const kEoB1SoundsAmiga[120] = {
		0,
		"button",
		"L1M1A",
		"door",
		"door",
		"slam",
		"button",
		"button",
		"transmute",
		"eat",
		"magica",
		"throw",
		"plate",
		"passage",
		"unlock",
		"teleport",
		"undead",
		"pit",
		"itemland",
		0,
		0,
		"playhit",
		"death",
		"text",
		"electric",
		"dart",
		"dart",
		"unlock",
		"bonus",
		"bump",
		0,
		"electric",
		"playswing",
		"hum",
		"panel",
		"explode",
		"L10M2M",
		"L10M2A",
		"L4M1M",
		"beastatk",
		"L9M2M",
		"L8M1A",
		"L8M1M",
		"L7M1A",
		"L7M1M",
		"L5M1A",
		"L5M1M",
		"flindatk",
		"L3M2M",
		"L4M1A",
		"L8M2M",
		"houndatk",
		"scream",
		"L6M1M",
		"L3M1A",
		"L3M1M",
		"sloshsuck",
		"L1M2M",
		"flayeratk",
		0,
		"rustatk",
		"L9M1M",
		"L10M1A",
		"L10M1M",
		"blade",
		"L7M2M",
		"blade",
		"L2M2M",
		"L12M2A",
		"L12M2M",
		0,
		"L11M1M",
		"L11M1A",
		"L2M1A",
		"L2M1M",
		"L1M1M",
		"button",
		0,
		"drop",
		"text",
		"magicb",
		"lock",
		0,
		0,
		0,
		"Missile",
		0,
		"burnhands",
		"electric",
		"fireball",
		0,
		"magica",
		"magica",
		"magica",
		"magicb",
		"magicb",
		"acid",
		"magicb",
		"fireball",
		"acid",
		"magica",
		"magicb",
		"magicb",
		"undead",
		"magica",
		"magica",
		"magica",
		"magicb",
		"cause",
		"magicb",
		"magicb",
		"magica",
		"magicb",
		"magica",
		"magica",
		"magica",
		"magica",
		"cause",
		0,
		"door"
	};
}

void SoundAmiga_EoB::beginFadeOut() {

}

} // End of namespace Kyra
