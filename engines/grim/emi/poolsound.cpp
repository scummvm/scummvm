/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/str.h"
#include "engines/grim/emi/sound/emisound.h"
#include "engines/grim/emi/poolsound.h"
#include "engines/grim/resource.h"


namespace Grim {

PoolSound::PoolSound() : _filename(""), _loaded(false), _soundId(0) {
}

PoolSound::PoolSound(const Common::String &filename) : _filename(""), _loaded(false), _soundId(0) {
	openFile(filename);
}

// Called when the engine restarts or Lua code calls FreeSound
PoolSound::~PoolSound() {
	if (_loaded) {
		g_emiSound->freeLoadedSound(_soundId);
	}
}

void PoolSound::setVolume(int volume) {
	if (_loaded) {
		g_emiSound->setLoadedSoundVolume(_soundId, volume);
	}
}

void PoolSound::setBalance(int balance) {
	if (_loaded) {
		g_emiSound->setLoadedSoundPan(_soundId, balance);
	}
}

void PoolSound::setPosition(Math::Vector3d &pos) {
	if (_loaded) {
		g_emiSound->setLoadedSoundPosition(_soundId, pos);
	}
}

void PoolSound::play(bool looping) {
	if (_loaded) {
		g_emiSound->playLoadedSound(_soundId, looping);
	}
}

void PoolSound::playFrom(const Math::Vector3d &pos, bool looping) {
	if (_loaded) {
		g_emiSound->playLoadedSoundFrom(_soundId, pos, looping);
	}
}

void PoolSound::stop() {
	if (_loaded) {
		g_emiSound->stopLoadedSound(_soundId);
	}
}

int PoolSound::getVolume() {
	if (_loaded) {
		return g_emiSound->getLoadedSoundVolume(_soundId);
	}
	return 0;
}

bool PoolSound::isPlaying() {
	if (_loaded) {
		return g_emiSound->getLoadedSoundStatus(_soundId);
	}
	return false;
}

void PoolSound::openFile(const Common::String &filename) {
	_filename = filename;
	_loaded = g_emiSound->loadSfx(filename.c_str(), _soundId);
	if (!_loaded) {
		warning("Could not open PoolSound file %s", filename.c_str());
	}
}

void PoolSound::saveState(SaveGame *state) {
	state->writeBool(_loaded);
	state->writeLESint32(_soundId);
}

void PoolSound::restoreState(SaveGame *state) {
	if (state->saveMinorVersion() >= 21) {
		_loaded = state->readBool();
		_soundId = state->readLESint32();
	} else {
		bool hasStream = state->readBool();
		if (hasStream)
			openFile(state->readString());
	}
}
}
