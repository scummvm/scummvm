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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_TRANSPORTS_H
#define ULTIMA_ULTIMA1_U1DIALOGS_TRANSPORTS_H

#include "ultima/ultima1/u1dialogs/buy_sell_dialog.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

using Shared::CCharacterInputMsg;

/**
 * Implements the dialog for the transport merchant
 */
class Transports : public BuySellDialog {
	DECLARE_MESSAGE_MAP;
	virtual bool CharacterInputMsg(CCharacterInputMsg &msg) override;
private:
	uint _transportsNum;
	uint _water, _woods, _grass;
private:
	/**
	 * Calculates the number of free tiles in the overworld
	 */
	void loadOverworldFreeTiles();

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
	virtual void setMode(BuySell mode) override;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Transports(Ultima1Game *game, int transportNum);

	/**
	 * Draws the visual item on the screen
	 */
	virtual void draw() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
