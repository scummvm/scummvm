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
	: _vm(scumm), _mixer(mixer), _musicId(), _tfmxPlay(0), _tfmxSfx(0), _musicHandle(), _sfxHandle() {
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

		play = new Audio::Tfmx(_mixer->getOutputRate(), true);
		fileMdat.seek(0);
		fileSample.seek(0);
		if (play->load(fileMdat, fileSample)) {
			_tfmxSfx = play;
		} else
			delete play;
	}
	return _tfmxPlay != 0;
}

Player_V4A::~Player_V4A() {
	_mixer->stopHandle(_musicHandle);
	_mixer->stopHandle(_sfxHandle);
	delete _tfmxPlay;
	delete _tfmxSfx;
}

void Player_V4A::setMusicVolume(int vol) {
	debug("player_v4a: setMusicVolume %i", vol);
}


void Player_V4A::stopAllSounds() {
	if (_musicId)
		stopSound(_musicId);
}

void Player_V4A::stopSound(int nr) {
	if (nr == _musicId) {
		_mixer->stopHandle(_musicHandle);
		_musicId = 0;
	} else
		warning("player_v4a: stop Sound %d", nr);
}

void Player_V4A::startSound(int nr) {
	assert(_vm);
	const byte *ptr = _vm->getResourceAddress(rtSound, nr);
	assert(ptr);

	static const int8 monkeyCommands[52] = {
		 -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,
		 -9, -10, -11, -12, -13, -14,  18,  17,
		-17, -18, -19, -20, -21, -22, -23, -24,
		-25, -26, -27, -28, -29, -30, -31, -32,
		-33,  16, -35,   0,   1,   2,   3,   7,
		  8,  10,  11,   4,   5,  14,  15,  12,
		  6,  13,   9,  19
	};

	const int val = ptr[9];
	if (val < 0 || val >= ARRAYSIZE(monkeyCommands)) {
		debug(3, "Tfmx: illegal Songnumber %i", val);
		return;
	}

	if (!_tfmxSfx || !_tfmxPlay)
		return;

	int index = monkeyCommands[val];
	if (index < 0) {
		// SoundFX
		index = -index - 1;
		debug(5, "player_v4a: play custom %i", index);

		if (_tfmxSfx->getSongIndex() < 0)
			_tfmxSfx->doSong(0x18);
		_tfmxSfx->doSfx(index);

		if (!_mixer->isSoundHandleActive(_sfxHandle))
			_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, _tfmxSfx, -1, Audio::Mixer::kMaxChannelVolume, 0, true, false);
	} else {
		// Song
		debug(5, "player_v4a: play song %i", index);
		assert(_tfmxPlay);

		_tfmxPlay->doSong(index);
		_musicId = nr;


		if (!_mixer->isSoundHandleActive(_musicHandle))
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _tfmxPlay, -1, Audio::Mixer::kMaxChannelVolume, 0, false, false);
	}
}


int Player_V4A::getMusicTimer() const {
	if (_musicId) {
		// TODO: The titlesong is running with ~70 ticks per second and the scale seems to be based on that. 
		// Other songs dont and I have no clue if this scalevalue is anything close to correct for them.
		// The Amiga-Game doesnt counts the ticks of the song, but has an own timer and I hope thespeed is constant through the game
		const int magicScale = 359; // ~ 1000 * 25 * (10173 / 709379.0)
		return (_mixer->getSoundElapsedTime(_musicHandle)) / magicScale;
	} else
		return 0;
}

int Player_V4A::getSoundStatus(int nr) const {
	if (nr == _musicId && _mixer->isSoundHandleActive(_musicHandle))
		return _tfmxPlay->getSignal(0);
	return 0;
}

} // End of namespace Scumm
