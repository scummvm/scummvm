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

#include "titanic/game/brain_slot.h"
#include "titanic/core/project_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBrainSlot, CGameObject)
	ON_MESSAGE(SetFrameMsg)
	ON_MESSAGE(AddHeadPieceMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

int CBrainSlot::_numAdded;
bool CBrainSlot::_woken;

void CBrainSlot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_occupied, indent);
	file->writeQuotedLine(_target, indent);
	file->writeNumberLine(_numAdded, indent);
	file->writeNumberLine(_woken, indent);

	CGameObject::save(file, indent);
}

void CBrainSlot::load(SimpleFile *file) {
	file->readNumber();
	_occupied = file->readNumber();
	_target = file->readString();
	_numAdded = file->readNumber();
	_woken = file->readNumber();

	CGameObject::load(file);
}

bool CBrainSlot::SetFrameMsg(CSetFrameMsg *msg) {
	loadFrame(msg->_frameNumber);
	_occupied = true;
	return true;
}

bool CBrainSlot::AddHeadPieceMsg(CAddHeadPieceMsg *msg) {
	_numAdded++;
	_cursorId = CURSOR_HAND;
	CAddHeadPieceMsg addMsg("NULL");

	if (isEquals("AuditoryCentreSlot")) {
		if (msg->_value == "AuditoryCentre")
			addMsg._value = "AuditoryCentre";
	} else if (isEquals("SpeechCentreSlot")) {
		if (msg->_value == "SpeechCentre")
			addMsg._value = "SpeechCentre";
	} else if (isEquals("OlfactoryCentreSlot")) {
		if (msg->_value == "OlfactoryCentre")
			addMsg._value = "OlfactoryCentre";
	} else if (isEquals("VisionCentreSlot")) {
		if (msg->_value == "VisionCentre")
			addMsg._value = "VisionCentre";
	} else if (isEquals("CentralCoreSlot")) {
		if (msg->_value == "CentralCore")
			addMsg._value = "CentralCore";
	}

	if (addMsg._value != "NULL")
		addMsg.execute("TitaniaControl");

	if (msg->_value == "OlfactoryCentre")
		loadFrame(2);
	else if (msg->_value == "AuditoryCentre")
		loadFrame(1);
	else if (msg->_value == "SpeechCentre")
		loadFrame(3);
	else if (msg->_value == "VisionCentre")
		loadFrame(4);
	else if (msg->_value == "CentralCore") {
		CActMsg actMsg("Insert Central Core");
		actMsg.execute("CentralCoreSlot");
	}

	_target = msg->_value;
	_occupied = true;
	return true;
}

bool CBrainSlot::EnterViewMsg(CEnterViewMsg *msg) {
	if (getName() == "CentralCoreSlot")
		loadFrame(21);
	if (_woken)
		_cursorId = CURSOR_ARROW;

	return true;
}

bool CBrainSlot::ActMsg(CActMsg *msg) {
	if (msg->_action == "Insert Central Core")
		playMovie(0, 21, 0);
	else if (msg->_action == "Woken")
		_woken = true;

	return true;
}

bool CBrainSlot::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!_occupied || _woken || !checkPoint(msg->_mousePos, false, true))
		return false;

	_cursorId = CURSOR_ARROW;
	CVisibleMsg visibleMsg(true);
	visibleMsg.execute(_target);
	CTakeHeadPieceMsg takeMsg(_target);
	takeMsg.execute("TitaniaControl");

	loadFrame(isEquals("CentralCoreSlot") ? 21 : 0);
	_occupied = false;

	CPassOnDragStartMsg passMsg;
	passMsg._mousePos = msg->_mousePos;
	passMsg.execute(_target);

	msg->_dragItem = getRoot()->findByName(_target);
	_numAdded--;

	return true;
}

} // End of namespace Titanic
