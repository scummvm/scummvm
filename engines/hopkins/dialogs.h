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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_DIALOGS_H
#define HOPKINS_DIALOGS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"

namespace Hopkins {

class HopkinsEngine;

/**
 * Class for manging game dialogs
 */
class DialogsManager {
private:
	HopkinsEngine *_vm;
public:
	int _inventX, _inventY;
	int _inventWidth, _inventHeight;

	bool _inventFl;
	bool _inventDisplayedFl;
	bool _removeInventFl;
	byte *_inventWin1;
	byte *_inventBuf2;
public:
	DialogsManager();
	~DialogsManager();
	void setParent(HopkinsEngine *vm);
	void showLoadGame();
	void showSaveGame();
	void showSaveLoad(int a1);
	void showOptionsDialog();
	void showInventory();
	int  searchSavegames();
	void testDialogOpening();
	void inventAnim();
};

} // End of namespace Hopkins

#endif /* HOPKINS_MENU_H */
