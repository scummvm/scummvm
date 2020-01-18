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

#include "ultima/shared/engine/game_manager.h"
#include "ultima/shared/engine/events.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Shared {

GameManager::GameManager(Shared::Game *game, Audio::Mixer *mixer):
		_game(game), _inputHandler(this), _inputTranslator(&_inputHandler) {
//	_game->setGameManager(this);
}

GameManager::~GameManager() {
	//_game->resetGameManager();
}

void GameManager::update() {
	if (_view) {
		// Signal the next frame
		CFrameMsg frameMsg(g_vm->_events->getTicksCount());
		frameMsg.execute(_view, nullptr, MSGFLAG_SCAN);

		_view->draw();
	}
}

void GameManager::changeView(const Common::String &name) {
	Gfx::VisualItem *newView = dynamic_cast<Gfx::VisualItem *>(_game->findByName(name));
	assert(newView);

	// Hide the current view
	CHideMsg hideMsg(_view, true);
	hideMsg.execute(_view, nullptr, MSGFLAG_SCAN);

	if (hideMsg._fadeOut) {
		// TODO: Fade out
	}

	// Show the new view
	_view = newView;
	CShowMsg showMsg(_view, true);
	showMsg.execute(_view, nullptr, MSGFLAG_SCAN);

	_view->draw();

	if (showMsg._fadeIn) {
		// TODO: Fade in
	}
}

} // End of namespace Shared
} // End of namespace Ultima
