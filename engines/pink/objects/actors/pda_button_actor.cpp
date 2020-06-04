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

#include "pink/constants.h"
#include "pink/cursor_mgr.h"
#include "pink/pink.h"
#include "pink/objects/pages/page.h"
#include "pink/objects/actors/pda_button_actor.h"
#include "pink/objects/actions/action_cel.h"

namespace Pink {

void PDAButtonActor::deserialize(Archive &archive) {
	Actor::deserialize(archive);
	_x = archive.readDWORD();
	_y = archive.readDWORD();
	_hideOnStop = (bool)archive.readDWORD();
	_opaque = (bool)archive.readDWORD();

	int type = archive.readDWORD();
	assert(type != 0 && type != Command::kIncrementFrame && type != Command::kDecrementFrame);
	if (_page->getGame()->isPeril()) {
		_command.type = (Command::CommandType) type;
	} else {
		switch (type) {
		case 1:
			_command.type = Command::kGoToPage;
			break;
		case 2:
			_command.type = Command::kClose;
			break;
		default:
			_command.type = Command::kNull;
			break;
		}
	}
	_command.arg = archive.readString();
}

void PDAButtonActor::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "PDAButtonActor: _name = %s, _x = %u _y = %u _hideOnStop = %u, _opaque = %u, _commandType = %u, _arg = %s",
		  _name.c_str(), _x, _y, _hideOnStop, _opaque, (int)_command.type, _command.arg.c_str());
}

void PDAButtonActor::onLeftClickMessage() {
	if (isActive()) {
		_page->getGame()->getPdaMgr().execute(_command);
	}
}

void PDAButtonActor::onMouseOver(Common::Point point, CursorMgr *mgr) {
	if (_command.type == Command::kNull || !isActive())
		mgr->setCursor(kPDADefaultCursor, point, Common::String());
	else
		mgr->setCursor(kPDAClickableFirstFrameCursor, point, Common::String());
}

bool PDAButtonActor::isActive() const {
	return _action && _action->getName() != "Inactive";
}

void PDAButtonActor::init(bool paused) {
	if (_x != -1 && _y != -1) {
		for (uint i = 0; i < _actions.size(); ++i) {
			ActionCEL *action = dynamic_cast<ActionCEL*>(_actions[i]);
			assert(action);
			action->loadDecoder();
			Common::Point center;
			center.x = _x + action->getDecoder()->getWidth() / 2;
			center.y = _y + action->getDecoder()->getHeight() / 2;
			action->setCenter(center);
		}
	}
	Actor::init(paused);
}

} // End of namespace Pink
