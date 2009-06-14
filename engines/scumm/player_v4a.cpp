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


#include "engines/engine.h"
#include "scumm/player_v4a.h"
#include "scumm/scumm.h"

#include "common/file.h"

namespace Scumm {

Player_V4A::Player_V4A(ScummEngine *scumm, Audio::Mixer *mixer)
	: _vm(scumm), _mixer(mixer), _slots(), _musicId(), _tfmxPlay(0) {
	init();
}

bool Player_V4A::init() {
	if (_vm->_game.id != GID_MONKEY_VGA)
		error("player_v4a - unknown game");
	
	Common::File fileMdat;
	Common::File fileSample;
	bool mdatExists = fileMdat.open("music.dat");
	bool sampleExists = fileSample.open("sample.dat");

	if (mdatExists && sampleExists) {
		Audio::Tfmx *play =  new Audio::Tfmx(_mixer->getOutputRate(), true);
		if (play->load(fileMdat, fileSample)) {
			_tfmxPlay = play;
		} else
			delete play;
	}
	return _tfmxPlay != 0;
}

Player_V4A::~Player_V4A() {
	delete _tfmxPlay;
}

void Player_V4A::setMusicVolume(int vol) {
}

int Player_V4A::getSlot(int id) const {
	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i].id == id)
			return i;
	}

	if (id == 0)
		warning("player_v4a - out of music channels");
	return -1;
}


void Player_V4A::stopAllSounds() {
	if (_musicId)
		stopSound(_musicId);
}

void Player_V4A::stopSound(int nr) {
	if (nr == _musicId) {
		_mixer->stopHandle(_musicHandle);
		_musicId = 0;
	}
}

void Player_V4A::startSound(int nr) {
	assert(_vm);
	byte *ptr = _vm->getResourceAddress(rtSound, nr);
	assert(ptr);

	char buf[22];
	sprintf(buf,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", ptr[0], ptr[1], ptr[2], ptr[3], 
		ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], ptr[9] );
	debug("%s", buf);


	static const int8 monkeyCommands[52] = {
		 -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,
		 -9, -10, -11, -12, -13, -14,  18,  17,
		-17, -18, -19, -20, -21, -22, -23, -24,
		-25, -26, -27, -28, -29, -30, -31, -32,
		-33,  16, -35,   0,   1,   2,   3,   7,
		  8,  10,  11,   4,   5,  14,  15,  12,
		  6,  13,   9,  19
	};

	int val = ptr[9];
	if (val < 0 || val >= ARRAYSIZE(monkeyCommands))
		debug("Tfmx: illegal Songnumber %i", val);
	int index = monkeyCommands[val];
	if (index < 0) {
		// SoundFX
		debug("Tfmx: Soundpattern %i", -index - 1);

	} else {
		// Song
		debug("Tfmx: Song %i", index);
		assert(_tfmxPlay);
		_mixer->stopHandle(_musicHandle);

		_tfmxPlay->doSong(index);
		_musicId = nr;
		_musicLastTicks = _tfmxPlay->getTicks();

		_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _tfmxPlay, -1, Audio::Mixer::kMaxChannelVolume, 0, false, false);
	}
}


int Player_V4A::getMusicTimer() const {
	if (_musicId) {
		return (_tfmxPlay->getTicks() - _musicLastTicks) / 25;
	} else
		return 0;
}

int Player_V4A::getSoundStatus(int nr) const {
	if (nr == _musicId)
		return _mixer->isSoundHandleActive(_musicHandle);
	/*if (getSfxChan(nr) != -1)
		return 1;*/
	return 0;
}

} // End of namespace Scumm
