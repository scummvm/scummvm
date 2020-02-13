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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_COMBAT_H
#define ULTIMA_ULTIMA1_U1DIALOGS_COMBAT_H

#include "ultima/ultima1/u1dialogs/full_screen_dialog.h"
#include "ultima/shared/maps/map_widget.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

using Shared::CKeypressMsg;

/**
 * Implements player combat attacks
 */
class Combat : public FullScreenDialog {
	DECLARE_MESSAGE_MAP;
	bool KeypressMsg(CKeypressMsg &msg);
private:
	Common::String _weaponName;
	int _direction;
private:
	/**
	 * Nothing selected
	 */
	void nothing();

	/**
	 * Draw the selection prompt
	 */
	void drawSelection();
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Combat(Ultima1Game *game, Shared::Maps::Direction direction, int weaponType, const Common::String weaponName);

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
