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

#include "ultima/shared/gfx/popup.h"
#include "ultima/shared/early/game_base.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

BEGIN_MESSAGE_MAP(Popup, VisualItem)
	ON_MESSAGE(ShowMsg)
END_MESSAGE_MAP()

Popup::Popup(GameBase *game) : VisualItem(nullptr), _game(game), _respondTo(nullptr), _parentView(nullptr) {
	game->addChild(this);
}

Popup::~Popup() {
	detach();
}

void Popup::show(TreeItem *respondTo) {
	// Save a copy of the view the popup is being shown on, and activate it
	_parentView = _game->getView();
	_respondTo = respondTo;
	if (!_respondTo)
		_respondTo = _parentView;

	_game->setPopup(this);
	setDirty();
}


void Popup::hide() {
	CPopupHiddenMsg hiddenMsg(this);
	hiddenMsg.execute(_respondTo, nullptr, 0);

	// Reset back to the parent view
	_game->setView(_parentView);
	_parentView->setDirty();
}

bool Popup::ShowMsg(CShowMsg &msg) {
	CPopupShownMsg shownMsg(this);
	shownMsg.execute(_respondTo, nullptr, 0);
	return true;
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
