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

#ifndef HODJNPODJ_VIEWS_MAIN_MENU_H
#define HODJNPODJ_VIEWS_MAIN_MENU_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {

#define NO_RULES    0x0001
#define NO_NEWGAME  0x0002
#define NO_OPTIONS  0x0004
#define NO_RETURN   0x0008
#define NO_QUIT     0x0010
#define NO_AUDIO    0x0020

class MainMenu: public View {
private:
	uint _flags = 0;
	const char *_rulesFilename = nullptr;
	const char *_rulesSoundFilename = nullptr;

	GfxSurface _background;
	ColorButton _rulesButton;
	ColorButton _newGameButton;
	ColorButton _optionsButton;
	ColorButton _audioButton;
	ColorButton _continueButton;
	ColorButton _quitButton;

public:
	MainMenu();
	virtual ~MainMenu() {}

	static void show(
		uint nFlags, const char *rulesFileName,
		const char *rulesSoundFilename);

	void draw() override;
	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
