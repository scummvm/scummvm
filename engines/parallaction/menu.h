/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_MENU_H
#define PARALLACTION_MENU_H

#include "common/rect.h"

#include "parallaction/defs.h"

namespace Parallaction {

class Parallaction;

class Menu {

public:
	Menu(Parallaction *vm);
	virtual 	~Menu();

	void		start();
	void		selectCharacter();

protected:
	void 		splash();
	void		newGame();
	uint16		chooseLanguage();
	uint16		selectGame();

public:


protected:
	Parallaction*	_vm;


};

#endif

} // namespace Parallaction
