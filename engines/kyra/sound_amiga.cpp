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

#include "common/system.h"
#include "common/mutex.h"
#include "kyra/resource.h"
#include "kyra/sound_intern.h"

#include "sound/mixer.h"
#include "sound/mods/maxtrax.h"
#include "sound/audiostream.h"

namespace Kyra {

const char *const SoundAmiga::kFilenameTable[3][2] = {
	{ "introscr.mx", "introinst.mx" },
	{ "kyramusic.mx", 0 },
	{ "finalescr.mx", 0 }
};

SoundAmiga::SoundAmiga(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _driver(0), _fileLoaded(-1) {
}

SoundAmiga::~SoundAmiga() {
	delete _driver;
}

bool SoundAmiga::init() {
	_driver = new Audio::MaxTrax(_mixer->getOutputRate(), true);

	return _driver != 0;
}

void SoundAmiga::loadSoundFile(uint file) {
	assert(file < ARRAYSIZE(kFilenameTable));
	if (_fileLoaded == file)
		return;

	Common::SeekableReadStream *scoreIn = _vm->resource()->createReadStream(kFilenameTable[file][0]);
	if (kFilenameTable[file][1]) {
		Common::SeekableReadStream *sampleIn = _vm->resource()->createReadStream(kFilenameTable[file][1]);
		if (scoreIn && sampleIn) {
			_driver->load(*scoreIn, true, false);
			_driver->load(*sampleIn, false, true);
			_fileLoaded = file;
		}
		delete sampleIn;
	} else {
		if (scoreIn) {
			_driver->load(*scoreIn);
			_fileLoaded = file;
		}
	}
	delete scoreIn;
}
void SoundAmiga::loadSoundFile(Common::String) {
	assert("Dont call me" == 0);
}

void SoundAmiga::playTrack(uint8 track) {
	_driver->doSong(track - 2);
	if (!_mixer->isSoundHandleActive(_soundChannels[0]))
			_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_soundChannels[0], _driver);
}

void SoundAmiga::haltTrack() {

}

void SoundAmiga::beginFadeOut() {

}

void SoundAmiga::playSoundEffect(uint8 track) {
	debug("play sfx %d", track);
	uint16 extVar = 1; // maybe indicates music playing or enabled
	uint16 extVar2 = 1; // no idea
	if (0x61 <= track && track <= 0x63 && extVar) {
		assert(false);//some music-commands
	}
	assert(track < ARRAYSIZE(kEffectsTable));
	const EffectEntry &entry = kEffectsTable[track];
	if (extVar2 && entry.note) {
		_driver->playNote(entry.note, entry.patch, entry.duration, entry.volume, entry.pan != 0);
		if (!_mixer->isSoundHandleActive(_soundChannels[0]))
			_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_soundChannels[0], _driver);
	}
}

const SoundAmiga::EffectEntry SoundAmiga::kEffectsTable[120] = {
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x01, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0156, 0x3C, 0x13, 120,  1 },
	{ 0x272C, 0x3C, 0x14, 120,  1 },
	{ 0x1B91, 0x3C, 0x15, 120,  1 },
	{ 0x1E97, 0x3C, 0x16, 120,  1 },
	{ 0x122B, 0x3C, 0x17, 120,  1 },
	{ 0x1E97, 0x3C, 0x16, 120,  1 },
	{ 0x0224, 0x45, 0x03, 120,  1 },
	{ 0x1E97, 0x3C, 0x16, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x0910, 0x2C, 0x04, 120,  1 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x3AEB, 0x3C, 0x1A, 120,  1 },
	{ 0x138B, 0x25, 0x1B, 120,  1 },
	{ 0x0F52, 0x18, 0x03, 120,  1 },
	{ 0x0622, 0x3E, 0x1C, 120,  1 },
	{ 0x0754, 0x3B, 0x1C, 120,  1 },
	{ 0x206F, 0x16, 0x03, 120,  1 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x09EA, 0x3C, 0x1D, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x272C, 0x3C, 0x14, 120,  1 },
	{ 0x036E, 0x3C, 0x1E, 120,  1 },
	{ 0x122B, 0x3C, 0x17, 120,  1 },
	{ 0x0991, 0x4E, 0x0B, 120,  1 },
	{ 0x02BC, 0x47, 0x1B, 120,  1 },
	{ 0x0211, 0x4C, 0x1B, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0156, 0x3C, 0x13, 120,  1 },
	{ 0x0156, 0x3C, 0x13, 120,  1 },
	{ 0x0E9E, 0x3C, 0x1F, 120,  1 },
	{ 0x010C, 0x3C, 0x20, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x0F7C, 0x3C, 0x21, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x4C47, 0x2A, 0x0B, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0528, 0x3C, 0x1B, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0910, 0x2C, 0x04, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0AEE, 0x3C, 0x22, 120,  1 },
	{ 0x1E97, 0x3C, 0x16, 120,  1 },
	{ 0x1B91, 0x3C, 0x15, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x272C, 0x3C, 0x14, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0AEE, 0x3C, 0x22, 120,  1 },
	{ 0x272C, 0x3C, 0x14, 120,  1 },
	{ 0x1419, 0x32, 0x23, 156,  1 },
	{ 0x171C, 0x3C, 0x19, 120,  1 },
	{ 0x272C, 0x3C, 0x14, 120,  1 },
	{ 0x0622, 0x3E, 0x1C, 120,  1 },
	{ 0x0201, 0x43, 0x13, 120,  1 },
	{ 0x1243, 0x3C, 0x24,  90,  1 },
	{ 0x00EE, 0x3E, 0x20, 120,  1 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x19EA, 0x29, 0x04, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x010C, 0x3C, 0x20, 120,  1 },
	{ 0x30B6, 0x3C, 0x25, 120,  1 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x1E97, 0x3C, 0x16, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x3AEB, 0x3C, 0x1A, 120,  1 },
	{ 0x39F3, 0x1B, 0x04, 120,  1 },
	{ 0x1699, 0x30, 0x23,  80,  1 },
	{ 0x1B91, 0x3C, 0x15, 120,  1 },
	{ 0x19EA, 0x29, 0x06,  80,  1 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x3AEB, 0x3C, 0x1A, 120,  1 },
	{ 0x252C, 0x3C, 0x19, 120,  1 },
	{ 0x0713, 0x3C, 0x26, 120,  1 },
	{ 0x0713, 0x3C, 0x26, 120,  1 },
	{ 0x272C, 0x3C, 0x14, 120,  1 },
	{ 0x1699, 0x30, 0x23,  80,  1 },
	{ 0x1699, 0x30, 0x23,  80,  1 },
	{ 0x0000, 0x00, 0x00,   0,  0 },
	{ 0x0156, 0x3C, 0x13, 120,  1 }
};

} // end of namespace Kyra