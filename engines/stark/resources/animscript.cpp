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
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

namespace Stark {

AnimScript::~AnimScript() {
}

AnimScript::AnimScript(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_anim(nullptr),
				_msecsToNextUpdate(0),
				_nextItemIndex(-1) {
	_type = TYPE;
}

void AnimScript::onAllLoaded() {
	Resource::onAllLoaded();

	_anim = Resource::cast<Anim>(_parent);
	_items = listChildren<AnimScriptItem>();

	if (!_items.empty()) {
		// Setup the next item to the first
		_nextItemIndex = 0;
	}
}

void AnimScript::onGameLoop() {
	Resource::onGameLoop();

	if (!_anim || !_anim->isInUse() || _nextItemIndex == -1) {
		// The script is disabled, do nothing
		return;
	}

	Global *global = StarkServices::instance().global;
	Common::RandomSource *randomSource = StarkServices::instance().randomSource;

	while (_msecsToNextUpdate <= (int32)global->getMillisecondsPerGameloop()) {
		AnimScriptItem *item = _items[_nextItemIndex];
		_msecsToNextUpdate += item->getDuration();

		switch (item->getOpcode()) {
		case AnimScriptItem::kDisplayFrame:
			_anim->selectFrame(item->getOperand());
			goToNextItem();
			break;
		case AnimScriptItem::kPlayAnimSound:
			// TODO
			goToNextItem();
			break;
		case AnimScriptItem::kGoToItem:
			_nextItemIndex = item->getOperand();
			break;
		case AnimScriptItem::kDisplayRandomFrame: {
			uint32 startFrame = item->getOperand() >> 16;
			uint32 endFrame = item->getOperand() & 0xFFFF;

			uint32 frame = randomSource->getRandomNumberRng(startFrame, endFrame);
			_anim->selectFrame(frame);
			goToNextItem();
			break;
		}
		case AnimScriptItem::kSleepRandomDuration: {
			uint duration = randomSource->getRandomNumber(item->getOperand());
			_msecsToNextUpdate += duration;
			goToNextItem();
			break;
		}
		case AnimScriptItem::kPlayStockSound:
			// TODO
			goToNextItem();
			break;
		default:
			error("Unknown anim script type %d", item->getOpcode());
		}
	}

	_msecsToNextUpdate -= global->getMillisecondsPerGameloop();
}

void AnimScript::goToNextItem() {
	_nextItemIndex += 1;
	_nextItemIndex %= _items.size();
}

AnimScriptItem::~AnimScriptItem() {
}

AnimScriptItem::AnimScriptItem(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_opcode(0),
				_duration(0),
				_operand(0) {
	_type = TYPE;
}

void AnimScriptItem::readData(XRCReadStream *stream) {
	_opcode = stream->readUint32LE();
	_duration = stream->readUint32LE();
	_operand = stream->readUint32LE();
}

void AnimScriptItem::printData() {
	debug("op: %d, duration: %d ms, operand: %d", _opcode, _duration, _operand);
}

} // End of namespace Stark
