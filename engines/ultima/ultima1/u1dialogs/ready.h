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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_READY_H
#define ULTIMA_ULTIMA1_U1DIALOGS_READY_H

#include "ultima/ultima1/u1dialogs/full_screen_dialog.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

using Shared::CShowMsg;
using Shared::CCharacterInputMsg;

/**
 * Implements the Ready dialog
 */
class Ready : public FullScreenDialog {
	DECLARE_MESSAGE_MAP;
	bool ShowMsg(CShowMsg &msg);
	bool CharacterInputMsg(CCharacterInputMsg &msg);

	enum Mode { SELECT, READY_WEAPON, READY_armour, READY_SPELL };
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
	 * Draw the ready weapon display
	 */
	void drawReadyWeapon();

	/**
	 * Draw the ready armor display
	 */
	void drawReadyArmor();

	/**
	 * Draw the ready spell display
	 */
	void drawReadySpell();
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Ready(Ultima1Game *game);

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
