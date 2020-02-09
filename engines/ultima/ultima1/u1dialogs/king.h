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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_KING_H
#define ULTIMA_ULTIMA1_U1DIALOGS_KING_H

#include "ultima/ultima1/u1dialogs/dialog.h"
#include "ultima/shared/gfx/character_input.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

using Shared::CShowMsg;
using Shared::CCharacterInputMsg;
using Shared::CTextInputMsg;

/**
  * Dialog for talking to kings
  */
class King : public Dialog {
	DECLARE_MESSAGE_MAP;
	bool ShowMsg(CShowMsg &msg);
	bool CharacterInputMsg(CCharacterInputMsg &msg);
	bool TextInputMsg(CTextInputMsg &msg);
	enum KingMode { SELECT, PENCE, SERVICE };
private:
	KingMode _mode;
	uint _kingIndex;
private:
	/**
	 * Set the mode
	 */
	void setMode(KingMode mode);

	/**
	 * Neither option (buy, service) selected
	 */
	void neither();

	/**
	 * No pence entered
	 */
	void none();

	/**
	 * Not that much
	 */
	void notThatMuch();

	/**
	 * Already on a quest
	 */
	void alreadyOnQuest();

	/**
	 * Give hit points
	 */
	void giveHitPoints(uint amount);
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	King(Ultima1Game *game, uint kingIndex);

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
