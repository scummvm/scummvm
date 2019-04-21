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

#include "titanic/sound/proximity.h"
#include "titanic/true_talk/tt_talker.h"

namespace Titanic {

CProximity::CProximity() : _channelVolume(100), _balance(0),
		_priorSoundHandle(-1), _frequencyMultiplier(0.0), _frequencyAdjust(1.875),
		_repeated(false), _channelMode(10), _positioningMode(POSMODE_NONE),
		_azimuth(0.0), _range(0.5), _elevation(0),
		_posX(0.0), _posY(0.0), _posZ(0.0),
		_hasVelocity(false), _velocityX(0), _velocityY(0), _velocityZ(0),
		_disposeAfterUse(DisposeAfterUse::NO), _endTalkerFn(nullptr), _talker(nullptr),
		_soundDuration(0), _soundType(Audio::Mixer::kPlainSoundType) {
}

CProximity::CProximity(Audio::Mixer::SoundType soundType, int volume) :
	_soundType(soundType), _channelVolume(volume),
	_balance(0), _priorSoundHandle(-1), _frequencyMultiplier(0.0),
	_frequencyAdjust(1.875), _repeated(false), _channelMode(10),
	_positioningMode(POSMODE_NONE), _azimuth(0.0), _range(0.5), _elevation(0),
	_posX(0.0), _posY(0.0), _posZ(0.0), _hasVelocity(false), _velocityX(0),
	_velocityY(0), _velocityZ(0), _disposeAfterUse(DisposeAfterUse::NO),
	_endTalkerFn(nullptr), _talker(nullptr), _soundDuration(0) {
}

} // End of namespace Titanic
