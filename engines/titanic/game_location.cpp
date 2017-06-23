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

#include "titanic/game_location.h"
#include "titanic/game_manager.h"
#include "titanic/game_state.h"
#include "titanic/core/game_object.h"
#include "titanic/core/project_item.h"

namespace Titanic {

#define STARTING_ROOM 3
#define STARTING_NODE 1
#define STARTING_VIEW 4

CGameLocation::CGameLocation(CGameState *owner) : _gameState(owner),
	_view(nullptr), _roomNumber(STARTING_ROOM),
	_nodeNumber(STARTING_NODE), _viewNumber(STARTING_VIEW) {
}

void CGameLocation::save(SimpleFile *file) const {
	file->writeNumber(_roomNumber);
	file->writeNumber(_nodeNumber);
	file->writeNumber(_viewNumber);
}

void CGameLocation::load(SimpleFile *file) {
	_view = nullptr;
	_roomNumber = file->readNumber();
	_nodeNumber = file->readNumber();
	_viewNumber = file->readNumber();
}

void CGameLocation::setView(CViewItem *view) {
	if (_view) {
		for (CTreeItem *treeItem = _view; treeItem;
				treeItem = treeItem->scan(_view)) {
			CGameObject *obj = dynamic_cast<CGameObject *>(treeItem);
			if (obj)
				obj->stopMovie();
		}
	}

	_view = view;
	if (_view) {
		_viewNumber = _view->_viewNumber;
		_nodeNumber = _view->findNode()->_nodeNumber;
		_roomNumber = _view->findRoom()->_roomNumber;
	} else {
		_viewNumber = _nodeNumber = _roomNumber = -1;
	}
}

CViewItem *CGameLocation::getView() {
	if (!_view) {
		CGameManager *gm = _gameState->_gameManager;
		_view = gm->_project->findView(_roomNumber, _nodeNumber, _viewNumber);

		if (!_view) {
			// Fallback if view not found
			_view = gm->_project->findView(STARTING_ROOM,
				STARTING_NODE, STARTING_VIEW);

			if (!_view) {
				// Fallback for the fallback
				for (int idx = 0; idx < 99 && !_view; ++idx) {
					_view = gm->_project->findView(idx, 1, 1);
				}
			}
		}
	}

	if (!_view) {
		// Okay seriously, yet another fallback if view not found
		_viewNumber = _nodeNumber = _roomNumber = -1;
		_view = nullptr;
	} else {
		_viewNumber = _view->_viewNumber;
		_nodeNumber = _view->findNode()->_nodeNumber;
		_roomNumber = _view->findRoom()->_roomNumber;
	}

	return _view;
}

CNodeItem *CGameLocation::getNode() {
	CViewItem *view = getView();
	return !view ? nullptr : view->findNode();
}

CRoomItem *CGameLocation::getRoom() {
	CViewItem *view = getView();
	return !view ? nullptr : view->findRoom();
}

} // End of namespace Titanic
