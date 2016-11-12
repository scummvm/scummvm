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

#include "engines/stark/resources/animscript.h"

#include "common/debug.h"
#include "common/random.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/container.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/sound.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

AnimScript::~AnimScript() {
}

AnimScript::AnimScript(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_anim(nullptr),
		_msecsToNextUpdate(0),
		_nextItemIndex(-1) {
	_type = TYPE;
}

void AnimScript::onAllLoaded() {
	Object::onAllLoaded();

	_anim = Object::cast<Anim>(_parent);
	_items = listChildren<AnimScriptItem>();

	if (!_items.empty()) {
		// Setup the next item to the first
		_nextItemIndex = 0;
	}
}

void AnimScript::onGameLoop() {
	Object::onGameLoop();

	if (!_anim || !_anim->isInUse() || _nextItemIndex == -1) {
		// The script is disabled, do nothing
		return;
	}

	while (_msecsToNextUpdate <= (int32)StarkGlobal->getMillisecondsPerGameloop()) {
		AnimScriptItem *item = _items[_nextItemIndex];
		_msecsToNextUpdate += item->getDuration();

		switch (item->getOpcode()) {
		case AnimScriptItem::kDisplayFrame:
			_anim->selectFrame(item->getOperand());
			goToNextItem();
			break;
		case AnimScriptItem::kPlayAnimSound: {
			Container *sounds = _parent->findChildWithSubtype<Container>(Container::kSounds);
			Sound *sound = sounds->findChildWithOrder<Sound>(item->getOperand());
			sound->play();

			goToNextItem();
			break;
		}
		case AnimScriptItem::kGoToItem:
			_nextItemIndex = item->getOperand();
			break;
		case AnimScriptItem::kDisplayRandomFrame: {
			uint32 startFrame = item->getOperand() >> 16;
			uint32 endFrame = item->getOperand() & 0xFFFF;

			uint32 frame = StarkRandomSource->getRandomNumberRng(startFrame, endFrame);
			_anim->selectFrame(frame);
			goToNextItem();
			break;
		}
		case AnimScriptItem::kSleepRandomDuration: {
			uint duration = StarkRandomSource->getRandomNumber(item->getOperand());
			_msecsToNextUpdate += duration;
			goToNextItem();
			break;
		}
		case AnimScriptItem::kPlayStockSound: {
			Location *location = StarkGlobal->getCurrent()->getLocation();
			Sound *sound = location->findStockSound(item->getOperand());
			if (sound) {
				sound->play();
			}
			goToNextItem();
			break;
		}
		default:
			error("Unknown anim script type %d", item->getOpcode());
		}
	}

	_msecsToNextUpdate -= StarkGlobal->getMillisecondsPerGameloop();
}

void AnimScript::goToNextItem() {
	_nextItemIndex += 1;
	_nextItemIndex %= _items.size();
}

void AnimScript::goToScriptItem(AnimScriptItem *item) {
	_nextItemIndex = findItemIndex(item);
	_msecsToNextUpdate = 0;

	if (item && item->getOpcode() == AnimScriptItem::kDisplayFrame) {
		_anim->selectFrame(item->getOperand());
	}
}

uint32 AnimScript::getDurationStartingWithItem(AnimScriptItem *startItem) {
	uint32 duration = 0;
	uint32 itemIndex = findItemIndex(startItem);

	while (1) {
		bool goingBackwards = false;
		AnimScriptItem *item = _items[itemIndex];

		switch (item->getOpcode()) {
			case AnimScriptItem::kDisplayFrame:
			case AnimScriptItem::kPlayAnimSound:
			case AnimScriptItem::kDisplayRandomFrame:
				itemIndex += 1;
				itemIndex %= _items.size();
				break;
			case AnimScriptItem::kGoToItem:
				if (item->getOperand() <= itemIndex) {
					goingBackwards = true;
				}
				itemIndex = item->getOperand();
				break;
			default:
				break;
		}

		if (itemIndex == 0 || goingBackwards) {
			break;
		}

		duration += item->getDuration();
	}

	return duration;
}

bool AnimScript::hasReached(AnimScriptItem *item) {
	int32 index = findItemIndex(item);
	return _nextItemIndex >= index;
}

int32 AnimScript::findItemIndex(AnimScriptItem *item) {
	if (!item) {
		return 0;
	}

	for (uint i = 0; i < _items.size(); i++) {
		if (_items[i] == item) {
			return i;
		}
	}

	return 0;
}

void AnimScript::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsSint32LE(_nextItemIndex);

	if (serializer->isLoading()) {
		_msecsToNextUpdate = 0;
	}
}

AnimScriptItem::~AnimScriptItem() {
}

AnimScriptItem::AnimScriptItem(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_opcode(0),
		_duration(0),
		_operand(0) {
	_type = TYPE;
}

void AnimScriptItem::readData(Formats::XRCReadStream *stream) {
	_opcode = stream->readUint32LE();
	_duration = stream->readUint32LE();
	_operand = stream->readUint32LE();
}

void AnimScriptItem::printData() {
	debug("op: %d, duration: %d ms, operand: %d", _opcode, _duration, _operand);
}

} // End of namespace Resources
} // End of namespace Stark
