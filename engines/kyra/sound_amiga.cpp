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
	assert("Dont call me" == 0);
}
} // end of namespace Kyra