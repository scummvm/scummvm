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

#include "ultima/ultima3/views/game.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/interactions/equip.h"
#include "ultima/ultima3/views/interactions/exchange.h"
#include "ultima/ultima3/views/interactions/ztats.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int PANEL_COL = 24;
constexpr int PANEL_SLOT_ROWS = 4;
constexpr int WIND_CHANGE_TICKS = 25;
constexpr int IDLE_FRAMES = 100;
constexpr char GLYPH_LABEL_START = 0x10;
constexpr char GLYPH_LABEL_END = 0x11;
constexpr char GLYPH_PROMPT = 0x10;

static const char *const WIND_NAMES[5] = {
	"Calm  Wind", "North Wind", "East  Wind", "South Wind", "West  Wind"
};

void Game::startGame() {
	_gameOver = false;
	_idleFrames = 0;
	_windCounter = 1;
	_interaction.reset();
	_G(soundEnabled) = true;
	_G(messages).clear();
	_G(effects) = Gfx::ScreenEffects();
	updateWind();
}

void Game::startPrompt() {
	_G(messages).putChar(GLYPH_PROMPT);
	_idleFrames = 0;
	redraw();
}

bool Game::checkPartyWipedOut() {
	if (_G(savegame).hasLivingPartyMember())
		return false;

	_G(messages).print("\n\nAll Players Out!\n");
	_gameOver = true;
	redraw();
	return true;
}

void Game::updateWind() {
	if (--_windCounter != 0)
		return;
	_windCounter = WIND_CHANGE_TICKS;

	// Calm or any of the four directions, but never the one it already is
	// when the choice comes from the four-in-nine chance of changing
	for (;;) {
		int wind = g_events->getRandomNumber(8);
		if (wind >= 5) {
			wind -= 4;
			if (wind == _G(windDirection))
				continue;
		}

		_G(windDirection) = wind;
		break;
	}
}

bool Game::msgFocus(const FocusMessage &msg) {
	_idleFrames = 0;
	delayFrames(1);
	return View::msgFocus(msg);
}

void Game::timeout() {
	_G(shapes).animate();
	_G(effects).tick();
	updateWind();
	processFrame();

	if (!_gameOver && !_interaction && isWaiting() && ++_idleFrames >= IDLE_FRAMES)
		idleTimeout();

	redraw();
	delayFrames(1);
}

void Game::startInteraction(Interactions::Interaction *interaction) {
	_interaction.reset(interaction);

	if (interaction->isFinished()) {
		_interaction.reset();
		endTurn();
		return;
	}

	redraw();
}

void Game::commandFailed(const char *text, byte sound) {
	_G(messages).print(text);
	g_engine->playSoundEffect(sound);
	endTurn();
}

bool Game::handleCommand(const KeypressMessage &msg) {
	switch (commandKey(msg)) {
	case 'Z':
		_G(messages).print("Ztats for # ");
		startInteraction(new Interactions::Ztats());
		return true;

	case 'M':
		_G(messages).print("Modify order!\nPlayer: ");
		startInteraction(new Interactions::Exchange());
		return true;

	case 'R':
		_G(messages).print("Ready for # ");
		startInteraction(new Interactions::Equip(true));
		return true;

	case 'W':
		_G(messages).print("Wear for # ");
		startInteraction(new Interactions::Equip(false));
		return true;

	case 'V':
		_G(soundEnabled) = !_G(soundEnabled);
		_G(messages).print("Volume ");
		_G(messages).print(_G(soundEnabled) ? "On!\n" : "Off!\n");
		endTurn();
		return true;

	default:
		return false;
	}
}

bool Game::msgKeypress(const KeypressMessage &msg) {
	if (isModifierKey(msg.keycode))
		return true;

	if (_gameOver) {
		_G(soundEnabled) = true;
		replaceView("Title", true);
		return true;
	}

	_idleFrames = 0;

	if (_interaction) {
		if (_interaction->keypress(msg)) {
			_interaction.reset();
			endTurn();
		} else {
			redraw();
		}

		return true;
	}

	handleCommand(msg);
	return true;
}

void Game::invertRect(GfxSurface &s, const Common::Rect &r) {
	for (int y = r.top; y < r.bottom; ++y) {
		byte *p = (byte *)s.getBasePtr(r.left, y);

		for (int x = r.left; x < r.right; ++x, ++p)
			*p ^= 3;
	}
}

void Game::drawFrame(GfxSurface &s) {
	// Alternating black and magenta pixels, filling whole 8x8 cells
	auto cell = [&](int col, int row) {
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x)
				s.setPixel(col * 8 + x, row * 8 + y, (x & 1) ? 2 : 0);
	};

	for (int row = 0; row < 24; ++row) {
		cell(0, row);
		cell(23, row);
	}
	for (int col = 0; col < 24; ++col) {
		cell(col, 0);
		cell(col, 23);
	}

	for (int row = 0; row <= 16; ++row)
		cell(FRAME_COLS - 1, row);
	for (int row = 0; row <= 16; row += PANEL_SLOT_ROWS) {
		for (int col = PANEL_COL; col < FRAME_COLS; ++col)
			cell(col, row);
	}

	s.addDirtyRect(Common::Rect(0, 0, 320, 200));
}

void Game::drawPartyStatus(GfxSurface &s) {
	Data::Savegame &save = _G(savegame);

	for (int slot = 0; slot < Data::PARTY_MAX; ++slot) {
		int row = slot * PANEL_SLOT_ROWS;

		s.writeString(Common::Point(30, row),
			Common::String::format("%c%d%c", GLYPH_LABEL_START, slot + 1, GLYPH_LABEL_END));
		if (slot >= save._partySize)
			continue;

		const Data::RosterEntry &e = save.partyMember(slot);
		int level = Data::fromBcd(e._experience >> 8) + 1;
		if (level > 99)
			level = 99;

		s.writeString(Common::Point((63 - (int)strlen(e._name)) / 2, row + 1), e._name);
		s.writeString(Common::Point(38, row + 1), Common::String::format("%c", e._status));
		s.writeString(Common::Point(25, row + 2),
			Common::String::format("%c%c%c M:%02X L:%02X", e._sex, e._race, e._class,
				e._magicPoints, Data::toBcd(level)));
		s.writeString(Common::Point(25, row + 3),
			Common::String::format("H:%04X F:%04X", e._hitPoints, e._food));
	}
}

void Game::drawLabels(GfxSurface &s) {
	drawWind(s);
}

void Game::drawWind(GfxSurface &s) {
	s.writeString(Common::Point(6, 23), Common::String::format("%c%s%c",
		GLYPH_LABEL_START, WIND_NAMES[_G(windDirection)], GLYPH_LABEL_END));
}

void Game::draw() {
	auto s = getSurface();

	s.fillRect(Common::Rect(0, 0, 320, 200), 0);
	drawFrame(s);
	drawPartyStatus(s);
	drawLabels(s);
	drawViewport(s);
	_G(messages).draw(s, !_gameOver);

	Gfx::ScreenEffects &fx = _G(effects);
	if (fx._viewport)
		invertRect(s, Common::Rect(8, 8, 184, 184));
	if (fx._slot >= 0)
		invertRect(s, Common::Rect(PANEL_COL * 8, (fx._slot * PANEL_SLOT_ROWS + 1) * 8, 39 * 8,
			(fx._slot * PANEL_SLOT_ROWS + 4) * 8));
	if (fx._highlight >= 0)
		invertRect(s, Common::Rect(31 * 8, fx._highlight * PANEL_SLOT_ROWS * 8, 32 * 8,
			(fx._highlight * PANEL_SLOT_ROWS + 1) * 8));
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
