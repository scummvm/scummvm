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

#include "titanic/core/drop_target.h"
#include "titanic/carry/carry.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CDropTarget, CGameObject)
	ON_MESSAGE(DropObjectMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(DropZoneLostObjectMsg)
END_MESSAGE_MAP()

CDropTarget::CDropTarget() : CGameObject(), _itemFrame(0),
		_itemMatchStartsWith(false), _hideItem(false), _dropEnabled(false), _dropFrame(0),
		_dragFrame(0), _dragCursorId(CURSOR_ARROW), _dropCursorId(CURSOR_HAND),
		_clipFlags(20) {
}

void CDropTarget::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writePoint(_pos1, indent);
	file->writeNumberLine(_itemFrame, indent);
	file->writeQuotedLine(_itemMatchName, indent);
	file->writeNumberLine(_itemMatchStartsWith, indent);
	file->writeQuotedLine(_soundName, indent);
	file->writeNumberLine(_hideItem, indent);
	file->writeQuotedLine(_itemName, indent);
	file->writeNumberLine(_dropEnabled, indent);
	file->writeNumberLine(_dropFrame, indent);
	file->writeNumberLine(_dragFrame, indent);
	file->writeQuotedLine(_clipName, indent);
	file->writeNumberLine(_dragCursorId, indent);
	file->writeNumberLine(_dropCursorId, indent);
	file->writeNumberLine(_clipFlags, indent);

	CGameObject::save(file, indent);
}

void CDropTarget::load(SimpleFile *file) {
	file->readNumber();
	_pos1 = file->readPoint();
	_itemFrame = file->readNumber();
	_itemMatchName = file->readString();
	_itemMatchStartsWith = file->readNumber();
	_soundName = file->readString();
	_hideItem = file->readNumber();
	_itemName = file->readString();
	_dropEnabled = file->readNumber();
	_dropFrame = file->readNumber();
	_dragFrame = file->readNumber();
	_clipName = file->readString();
	_dragCursorId = (CursorId)file->readNumber();
	_dropCursorId = (CursorId)file->readNumber();
	_clipFlags = file->readNumber();

	CGameObject::load(file);
}

bool CDropTarget::DropObjectMsg(CDropObjectMsg *msg) {
	if (!_itemName.empty()) {
		if (msg->_item->getName() != _itemName) {
			if (findByName(_itemName, true))
				return false;
		}
	}

	if (!msg->_item->isEquals(_itemMatchName, _itemMatchStartsWith))
		return false;

	msg->_item->detach();
	msg->_item->addUnder(this);
	msg->_item->setPosition(Point(_bounds.left, _bounds.top));

	msg->_item->loadFrame(_itemFrame);
	if (_hideItem)
		msg->_item->setVisible(false);

	_itemName = msg->_item->getName();
	CDropZoneGotObjectMsg gotMsg(this);
	gotMsg.execute(msg->_item);
	playSound(_soundName);

	if (_clipName.empty()) {
		loadFrame(_dropFrame);
	} else {
		playClip(_clipName, _clipFlags);
	}

	_cursorId = _dropCursorId;
	return true;
}

bool CDropTarget::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	CTreeItem *dragItem = msg->_dragItem;
	if (!checkStartDragging(msg))
		return false;
	msg->_dragItem = dragItem;

	CGameObject *obj = dynamic_cast<CGameObject *>(findByName(_itemName));
	if (_itemName.empty() || _dropEnabled || !obj)
		return false;

	CDropZoneLostObjectMsg lostMsg;
	lostMsg._object = this;
	lostMsg.execute(obj);

	loadFrame(_dragFrame);
	_cursorId = _dragCursorId;

	if (obj->_visible) {
		msg->execute(obj);
	} else {
		msg->_dragItem = obj;
		CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
		passMsg.execute(obj);
		obj->setVisible(true);
	}

	return true;
}

bool CDropTarget::EnterViewMsg(CEnterViewMsg *msg) {
	if (!_itemName.empty()) {
		CGameObject *obj = dynamic_cast<CGameObject *>(findByName(_itemName));
		if (!obj) {
			loadFrame(_dragFrame);
			_cursorId = _dragCursorId;
		} else if (_clipName.empty()) {
			loadFrame(_dropFrame);
			_cursorId = _dropCursorId;
		} else {
			playClip(_clipName, _clipFlags);
			_cursorId = _dropCursorId;
		}
	}

	return true;
}

bool CDropTarget::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	_dropEnabled = !msg->_visible;
	return true;
}

bool CDropTarget::DropZoneLostObjectMsg(CDropZoneLostObjectMsg *msg) {
	if (!_itemName.empty()) {
		CGameObject *obj = dynamic_cast<CGameObject *>(findByName(_itemName));
		if (obj) {
			if (msg->_object) {
				obj->detach();
				obj->addUnder(msg->_object);
			} else if (dynamic_cast<CCarry *>(obj)) {
				obj->petAddToInventory();
			}

			obj->setVisible(true);
			CDropZoneLostObjectMsg lostMsg(this);
			lostMsg.execute(obj);
		}

		loadFrame(_dragFrame);
		_cursorId = _dragCursorId;
	}

	return true;
}

} // End of namespace Titanic
