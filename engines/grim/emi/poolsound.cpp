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

#include "engines/grim/emi/poolsound.h"
#include "engines/grim/resource.h"

namespace Grim {

PoolSound::PoolSound() : _filename(""), _track(nullptr) {
}

PoolSound::PoolSound(const Common::String &filename) : _filename(""), _track(nullptr) {
	openFile(filename);
}

// Called when the engine restarts or Lua code calls FreeSound
PoolSound::~PoolSound() {
	if (!_track)
		return;
	_track->stop();
	delete _track;
}

void PoolSound::setVolume(int volume) {
	if (!_track) {
		warning("PoolSound::setVolume: no track found");
		return;
	}
	_track->setVolume(volume);
}

void PoolSound::setBalance(int balance) {
	if (!_track) {
		warning("PoolSound::setBalance: no track found");
		return;
	}
	_track->setBalance(balance);
}
void PoolSound::play(bool looping) {
	if (!_track)
		return;
	_track->setLooping(looping);
	_track->play();
}

void PoolSound::stop() {
	if (!_track)
		return;
	_track->stop();
}

void PoolSound::openFile(const Common::String &filename) {
	_filename = filename;
	_track = new AIFFTrack(Audio::Mixer::kSFXSoundType, DisposeAfterUse::NO);
	if (!_track->openSound(filename, filename)) {
		warning("Could not open PoolSound file %s", filename.c_str());
		delete _track;
		_track = nullptr;
		return;
	}
}

void PoolSound::saveState(SaveGame *state) {
	if (_track && _track->isStreamOpen()) {
		state->writeBool(true);
		state->writeString(_filename);
	} else {
		state->writeBool(false);
	}
}

void PoolSound::restoreState(SaveGame *state) {
	bool hasStream = state->readBool();
	if (hasStream)
		openFile(state->readString());
}
}
