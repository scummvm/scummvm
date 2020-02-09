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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_DROP_H
#define ULTIMA_ULTIMA1_U1DIALOGS_DROP_H

#include "ultima/ultima1/u1dialogs/full_screen_dialog.h"
#include "ultima/shared/gfx/text_input.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

using Shared::CShowMsg;
using Shared::CCharacterInputMsg;
using Shared::CTextInputMsg;

/**
 * Implements the drop dialog
 */
class Drop : public FullScreenDialog {
	DECLARE_MESSAGE_MAP;
	bool ShowMsg(CShowMsg &msg);
	bool CharacterInputMsg(CCharacterInputMsg &msg);
	bool TextInputMsg(CTextInputMsg &msg);

	enum Mode { SELECT, DROP_PENCE, DROP_WEAPON, DROP_armour };
private:
	Mode _mode;
private:
	/**
	 * Sets the mode
	 */
	void setMode(Mode mode);

	/**
	 * Nothing selected
	 */
	void nothing();

	/**
	 * None response
	 */
	void none();

	/**
	 * Draw the drop weapon display
	 */
	void drawDropWeapon();

	/**
	 * Draw the drop armor display
	 */
	void drawDropArmor();
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Drop(Ultima1Game *game);

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
