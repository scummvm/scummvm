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

#include "common/translation.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/rules.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {

void MainMenu::show(
		uint nFlags, const char *rulesFileName,
		const char *rulesSoundFileName) {
	MainMenu *view = (MainMenu *)g_events->findView("MainMenu");
	view->_flags = nFlags;
	view->_rulesFilename = rulesFileName;
	view->_rulesSoundFilename = rulesSoundFileName;

	view->addView();
}

MainMenu::MainMenu() : View("MainMenu"),
		_rulesButton("MenuRules", _s("&Rules"), this),
		_newGameButton("MenuNewGame", _s("&New Game"), this),
		_optionsButton("MenuOptions", _s("&Options"), this),
		_audioButton("MenuAudio", _s("&Audio"), this),
		_continueButton("MenuContinue", _s("&Continue"), this),
		_quitButton("MenuQuit", _s("&Quit"), this) {
}

void MainMenu::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_background);
	View::draw();
}
bool MainMenu::msgOpen(const OpenMessage &msg) {
	_background.loadBitmap("fuge/art/oscroll.bmp");
	_background.setTransparentColor(255);
	int x = (GAME_WIDTH - _background.w) / 2;
	int y = (GAME_HEIGHT - _background.h) / 2;
	setBounds(Common::Rect(x, y, x + _background.w, y + _background.h));

	// Set up button positions
	ColorButton *BUTTONS[6] = {
		&_rulesButton, &_newGameButton, &_optionsButton,
		&_audioButton, &_continueButton, &_quitButton
	};
	Common::Rect btnRect(0, 0, 80, 23);
	btnRect.translate(_bounds.left + (_bounds.width() - btnRect.width()) / 2,
		_bounds.top + 28);
	for (int i = 0; i < 6; ++i) {
		BUTTONS[i]->setBounds(btnRect);
		btnRect.translate(0, 26);
	}

	// Disable the Rules button if told to do so
	if (_flags & NO_RULES)
		_rulesButton.enableWindow(false);

	// Disable the NewGame button if told to do so
	if (_flags & NO_NEWGAME)
		_newGameButton.enableWindow(false);

	// Disable the Options button if told to do so
	if (_flags & NO_OPTIONS)
		_optionsButton.enableWindow(false);

	// Disable the Audio button if told to do so
	if (_flags & NO_AUDIO)
		_audioButton.enableWindow(false);

	// Disable the Return button if told to do so
	if (_flags & NO_RETURN)
		_continueButton.enableWindow(false);

	// Disable the Return button if told to do so
	if (_flags & NO_QUIT)
		_quitButton.enableWindow(false);

	g_events->showCursor(true);

	return true;
}

bool MainMenu::msgClose(const CloseMessage &msg) {
	_background.clear();
	return true;
}

bool MainMenu::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE &&
			_continueButton.isEnabled()) {
		close();
		return true;
	}

	return false;
}

bool MainMenu::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		Common::String btn = msg._stringValue;

		if (btn == "MenuRules") {
			Rules::show(_rulesFilename, _rulesSoundFilename);
			return true;
		} else if (btn == "MenuOptions") {
			// Pass a message to the minigame to show it's
			// custom options dialog
			_parent->send(GameMessage("OPTIONS"));
			return true;
		} else if (btn == "MenuContinue") {
			// Return to minigame
			close();
			return true;
		} else if (btn == "MenuQuit") {
			close();							// Close menu dialog
			g_events->focusedView()->close();	// Close minigame
			return true;
		}
	}

#if TODO
		_newGameButton("MenuNewGame", _s("&New Game"), this),
		_audioButton("MenuAudio", _s("&Audio"), this),
		_continueButton("MenuContinue", _s("&Continue"), this),

#endif
	return false;
}

} // namespace HodjNPodj
} // namespace Bagel
