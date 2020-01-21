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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_BUY_SELL_DIALOG_H
#define ULTIMA_ULTIMA1_U1DIALOGS_BUY_SELL_DIALOG_H

#include "ultima/ultima1/u1dialogs/dialog.h"
#include "ultima/shared/gfx/character_input.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

enum BuySell { SELECT, BUY, SELL, SOLD, CANT_AFFORD };

using Shared::CCharacterInputMsg;

/**
  * Secondary base class for dialogs that have display for buying and selling
  */
class BuySellDialog : public Dialog {
	DECLARE_MESSAGE_MAP;
	virtual bool CharacterInputMsg(CCharacterInputMsg &msg);
private:
	Shared::Gfx::CharacterInput _charInput;
protected:
	BuySell _mode;
	Common::String _title;
protected:
	/**
	 * Constructor
	 */
	BuySellDialog(Ultima1Game *game, const Common::String &title);

	/**
	 * Nothing selected
	 */
	void nothing();

	/**
	 * Set the mode
	 */
	void setMode(BuySell mode);

	/**
	 *  Switches the dialog to displaying sold
	 */
	void showSold() { setMode(SOLD); }

	/**
	 * Switches the dialog to displaying a can't afford message
	 */
	void cantAfford() { setMode(CANT_AFFORD); }
public:
	CLASSDEF;

	/**
	 * Draws the visual item on the screen
	 */
	virtual void draw();
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
