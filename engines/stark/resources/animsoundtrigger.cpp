/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/animsoundtrigger.h"

#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/sound.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

AnimSoundTrigger::~AnimSoundTrigger() {
}

AnimSoundTrigger::AnimSoundTrigger(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
        _soundStockType(0),
        _soundTriggerTime(0),
		_anim(nullptr),
		_alreadyPlayed(false),
		_timeRemainingBeforeLoop(34) {
	_type = TYPE;
}

void AnimSoundTrigger::onAllLoaded() {
	Object::onAllLoaded();
	_anim = Object::cast<AnimSkeleton>(_parent);
}

void AnimSoundTrigger::onGameLoop() {
	Object::onGameLoop();

	if (!_anim || !_anim->isInUse()) {
		return;
	}

	if (_alreadyPlayed && _anim->getCurrentTime() < 33) {
		// Animation loop detected, reset
		_alreadyPlayed = false;
	}

	if ((!_alreadyPlayed && _anim->getCurrentTime() >= _soundTriggerTime) || _timeRemainingBeforeLoop < 33) {
		if (_timeRemainingBeforeLoop >= 33) {
			_alreadyPlayed = true;
		}

		if (_subType == kAnimTriggerSound) {
			Location *location = StarkGlobal->getCurrent()->getLocation();
			Sound *sound = location->findStockSound(_soundStockType);
			if (sound && !StarkGlobal->isFastForward()) {
				// TODO: If the location has a 3D layer set the source position of the sound to the item position
				sound->stop();
				sound->play();
			}
		} else {
			warning("Unknown animation trigger subtype '%d'", _subType);
		}
	}

	// Special handling for trigger times right before the animation loop point
	if (!_alreadyPlayed && _soundTriggerTime - _anim->getCurrentTime() < 33) {
		_timeRemainingBeforeLoop = _anim->getRemainingTime();
	} else {
		_timeRemainingBeforeLoop = 34;
	}
}

void AnimSoundTrigger::readData(Formats::XRCReadStream *stream) {
	_soundTriggerTime = stream->readUint32LE();
	_soundStockType = stream->readUint32LE();
}

void AnimSoundTrigger::printData() {
	debug("triggerTime: %d", _soundTriggerTime);
	debug("soundStockType: %d", _soundStockType);
}

} // End of namespace Resources
} // End of namespace Stark
