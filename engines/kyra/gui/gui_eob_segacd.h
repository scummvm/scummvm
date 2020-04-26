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

#ifdef ENABLE_EOB

#ifndef KYRA_GUI_EOB_SEGACD_H
#define KYRA_GUI_EOB_SEGACD_H

#include "kyra/gui/gui_eob.h"

#ifdef ENABLE_EOB

namespace Kyra {

class GUI_EoB_SegaCD : public GUI_EoB {
public:
	GUI_EoB_SegaCD(EoBEngine *vm);
	~GUI_EoB_SegaCD() override;

private:
	void drawCampMenu() override;
	bool confirmDialogue(int id) override;
	void displayTextBox(int id) override;
	void drawMenuButton(Button *b, bool clicked, bool highlight, bool noFill) override;
	void drawButtonIntern(int id, int op);
	void updateOptionsStrings() override;
	void restParty_updateRestTime(int hours, bool init) override;

	const uint8 *_campMenu;
	EoBEngine *_vm;

	struct SegaMenuButton {
		uint16 nameTbl;
		uint16 nameTbl2;
		int16 x;
		int16 y;
		uint16 w;
		uint16 h;
	};

	static const SegaMenuButton _menuButtons[22];
};

} // End of namespace Kyra

#endif // ENABLE_EOB

#endif

#endif // ENABLE_EOB || ENABLE_LOL
