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

#include "common/mutex.h"
#include "common/str.h"
#include "common/stream.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "engines/grim/savegame.h"
#include "engines/grim/emi/sound/track.h"
#include "common/textconsole.h"
#include "engines/grim/grim.h"
#include "engines/grim/set.h"

namespace Grim {

SoundTrack::SoundTrack() {
	_stream = nullptr;
	_handle = nullptr;
	_paused = false;
	_positioned = false;
	_balance = 0;
	_volume = Audio::Mixer::kMaxChannelVolume;
	_disposeAfterPlaying = DisposeAfterUse::YES;
	_sync = 0;
	_fadeMode = FadeNone;
	_fade = 1.0f;
	_attenuation = 1.0f;

	// Initialize to a plain sound for now
	_soundType = Audio::Mixer::kPlainSoundType;
}

SoundTrack::~SoundTrack() {
	if (_stream && (_disposeAfterPlaying == DisposeAfterUse::NO || !_handle))
		delete _stream;
}

Common::String SoundTrack::getSoundName() {
	return _soundName;
}

void SoundTrack::setSoundName(const Common::String &name) {
	_soundName = name;
}

void SoundTrack::setVolume(int volume) {
	_volume = MIN(volume, static_cast<int>(Audio::Mixer::kMaxChannelVolume));
	if (_handle) {
		g_system->getMixer()->setChannelVolume(*_handle, (byte)getEffectiveVolume());
	}
}

void SoundTrack::setPosition(bool positioned, const Math::Vector3d &pos) {
	_positioned = positioned;
	_pos = pos;
	updatePosition();
}

void SoundTrack::updatePosition() {
	if (!_positioned)
		return;

	Set *set = g_grim->getCurrSet();
	Set::Setup *setup = set->getCurrSetup();
	Math::Vector3d cameraPos = setup->_pos;
	Math::Vector3d vector = _pos - cameraPos;
	float distance = vector.getMagnitude();
	if (_volume == 0) {
		_attenuation = 0.0f;
	} else {
		_attenuation = fmaxf(0.0f, 1.0f - distance / (_volume * 100.0f / Audio::Mixer::kMaxChannelVolume));
	}

	Math::Matrix4 worldRot = setup->_rot;
	Math::Vector3d relPos = (_pos - setup->_pos);
	Math::Vector3d p(relPos);
	worldRot.inverseRotate(&p);
	float angle = atan2(p.x(), p.z());
	float pan = sin(angle);
	_balance = (int)(pan * 127.0f);

	if (_handle) {
		g_system->getMixer()->setChannelBalance(*_handle, _balance);
		g_system->getMixer()->setChannelVolume(*_handle, (byte)getEffectiveVolume());
	}
}

void SoundTrack::setBalance(int balance) {
	if (_positioned)
		return;
	_balance = balance;
	if (_handle) {
		g_system->getMixer()->setChannelBalance(*_handle, _balance);
	}
}

bool SoundTrack::play() {
	if (_stream) {
		if (isPlaying()) {
			warning("sound: %s already playing, don't start again!", _soundName.c_str());
			return true;
		}
		// If _disposeAfterPlaying is NO, the destructor will take care of the stream.
		g_system->getMixer()->playStream(_soundType, _handle, _stream, -1, (byte)getEffectiveVolume(), _balance, _disposeAfterPlaying);
		return true;
	}
	return false;
}

void SoundTrack::pause() {
	_paused = !_paused;
	if (_stream) {
		g_system->getMixer()->pauseHandle(*_handle, _paused);
	}
}

void SoundTrack::stop() {
	if (_handle)
		g_system->getMixer()->stopHandle(*_handle);
}

void SoundTrack::setFade(float fade) {
	_fade = fade;
	if (_handle) {
		g_system->getMixer()->setChannelVolume(*_handle, (byte)getEffectiveVolume());
	}
}

int SoundTrack::getEffectiveVolume() {
	return _volume * _attenuation * _fade;
}

} // end of namespace Grim
