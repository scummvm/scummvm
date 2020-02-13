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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_ARMOURY_H
#define ULTIMA_ULTIMA1_U1DIALOGS_ARMOURY_H

#include "ultima/ultima1/u1dialogs/buy_sell_dialog.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

/**
 * Implements the buy/sell dialog for the armory
 */
class Armoury : public BuySellDialog {
	DECLARE_MESSAGE_MAP;
	bool CharacterInputMsg(CCharacterInputMsg &msg) override;
private:
//	uint _armouryNum;
	uint _startIndex, _endIndex;
private:
	/**
	 * Draws the Buy dialog content
	 */
	void drawBuy();

	/**
	 * Draws the Sell dialog content
	 */
	void drawSell();
protected:
	/**
	 * Set the mode
	 */
	void setMode(BuySell mode) override;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Armoury(Ultima1Game *game, int armouryNum);

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
