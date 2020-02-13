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

#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/u1gfx/view_game.h"
#include "ultima/ultima1/u1gfx/view_char_gen.h"
#include "ultima/ultima1/u1gfx/view_title.h"
#include "ultima/ultima1/u1gfx/text_cursor.h"
#include "ultima/ultima1/u6gfx/game_view.h"
#include "ultima/ultima1/spells/prayer.h"
#include "ultima/shared/early/font_resources.h"
#include "ultima/shared/gfx/popup.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {

EMPTY_MESSAGE_MAP(Ultima1Game, Shared::Game);

Ultima1Game::Ultima1Game() : Shared::Game(), _quests(this) {
	_res = new GameResources();
	_map = new Maps::Ultima1Map(this);

	_textCursor = new U1Gfx::U1TextCursor(_textColor, _bgColor);
	g_vm->_screen->setCursor(_textCursor);

	if (g_vm->isEnhanced()) {
		_videoMode = VIDEOMODE_VGA;
		loadU6Palette();
		setFont(new Shared::Gfx::Font((const byte *)&_fontResources->_fontU6[0][0]));
		_gameView = new U6Gfx::GameView(this);
		_titleView = nullptr;
		_charGenView = nullptr;
	} else {
		setEGAPalette();
		_gameView = new U1Gfx::ViewGame(this);
		_titleView = new U1Gfx::ViewTitle(this);
		_charGenView = new U1Gfx::ViewCharacterGeneration(this);
	}

	Common::fill(&_gems[0], &_gems[4], 0);
}

Ultima1Game::~Ultima1Game() {
	Shared::Gfx::Popup *popup = dynamic_cast<Shared::Gfx::Popup *>(_currentView);
	if (popup)
		popup->hide();

	delete _map;
	delete _gameView;
	delete _party;
}

void Ultima1Game::synchronize(Common::Serializer &s) {
	Shared::Game::synchronize(s);

	for (int idx = 0; idx < 4; ++idx)
		s.syncAsUint16LE(_gems[idx]);
	_quests.synchronize(s);
}

void Ultima1Game::starting(bool isLoading) {
	Shared::Game::starting(isLoading);

	_res->load();
	_party = new Party(this);
	_gameView->setView(isLoading ? "Game" : "Title");
}

bool Ultima1Game::canSaveGameStateCurrently() {
	return _currentView->getName() == "Game";
}

void Ultima1Game::giveTreasure(int coins, int v2) {
	// TODO
}

} // End of namespace Ultima1
} // End of namespace Ultima
