/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tesoro/kinds/menubutton.h"
#include "tesoro/world.h"

namespace Tesoro {

enum Action { kPrevious = 0, kNext = 1, kLoad = 2, kSave = 3, kNewGame = 5, kClose = 6, kSubtitles = 7 };

void MenuButton::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host) {
	_host = &host;
	initAnimated(record, roomSortOffset, levelPlayers, 0);
	if (_anim.bankCount() > 0) {
		AnimPlayer &player = _anim.current();
		const bool image = player.bank() && player.bank()->step(player.current()).hasImage();
		_flags = (_flags & ~kVisible) | (image ? kVisible : 0) | kCollidable | kInRoomList | kHitter;
	}
	_action = record.dataByte(1);
	_sortValue = 1;
	_flags |= kVisible;
}

bool MenuButton::acceptsPointer() {
	_host->setHoverText(nullptr);
	if (_action != kSubtitles && _anim.bankCount() > 0)
		_anim.current().setRange(1, 1, false, false, _host->now());
	showHoverText();
	return true;
}

int MenuButton::clearHoverText() {
	if (_action != kSubtitles && _anim.bankCount() > 0)
		_anim.current().setRange(0, 0, false, false, _host->now());
	return 1;
}

int MenuButton::setVerbAndClick(int verb) {
	switch (_action) {
	case kPrevious:
		_host->previousSlot();
		break;
	case kNext:
		_host->nextSlot();
		break;
	case kLoad:
		_host->loadSlot();
		break;
	case kSave:
		_host->saveSlot();
		_host->fadeOut(4, 100);
		break;
	case kNewGame:
		_host->fadeOut(4, 100);
		_host->clearMainList();
		return 3;
	case kClose:
		_host->closeSaveLoadMenu();
		break;
	case kSubtitles: {
		const int on = _host->toggleSubtitles() ? 1 : 0;
		if (_anim.bankCount() > 0)
			_anim.current().setRange(on, on, false, false, _host->now());
		break;
	}
	default:
		break;
	}
	return 1;
}

} // End of namespace Tesoro
