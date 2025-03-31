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

#ifndef HODJNPODJ_METAGAME_VIEWS_BOARDGAME_OPTIONS_H
#define HODJNPODJ_METAGAME_VIEWS_BOARDGAME_OPTIONS_H

#include "bagel/hodjnpodj/views/dialog.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class BoardgameOptions : public Dialog {
private:
	ColorButton _pPlayButton;
	ColorButton _pCancelButton;

	RadioButton _pHSHButton;	// Hodj Skill High Radio Button
	RadioButton _pHSMButton;	// Hodj Skill Medium Radio Button
	RadioButton _pHSLButton;	// Hodj Skill Low Radio Button

	RadioButton _pPSHButton;	// Podj Skill High Radio Button
	RadioButton _pPSMButton;	// Podj Skill Medium Radio Button
	RadioButton _pPSLButton;	// Podj Skill Low Radio Button

	RadioButton _pGTLButton;	// Game Time Long Radio Button
	RadioButton _pGTMButton;	// Game Time Medium Radio Button
	RadioButton _pGTSButton;	// Game Time Short Radio Button

	RadioButton _pPCButton;	// Podj is Computer High Radio Button
	RadioButton _pPHButton;	// Podj is Human High Radio Button

	int m_nHodjSkillLevel = SKILLLEVEL_LOW;
	int m_nPodjSkillLevel = SKILLLEVEL_LOW;
	int m_nGameTime = SHORT_GAME;
	bool m_bPodjIsComputer = true;

	void updateRadioButtons();

public:
	BoardgameOptions();
	~BoardgameOptions() override {}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
