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

#ifndef XEEN_DIALOGS_ERROR_H
#define XEEN_DIALOGS_ERROR_H

#include "xeen/dialogs.h"
#include "xeen/character.h"

namespace Xeen {

enum ErrorWaitType { WT_FREEZE_WAIT = 0, WT_NONFREEZED_WAIT = 1,
	WT_2 = 2, WT_3 = 3 };

class ErrorDialog : public ButtonContainer {
private:
	ErrorDialog(XeenEngine *vm) : ButtonContainer(vm) {}

	void execute(const Common::String &msg, ErrorWaitType waitType);
public:
	static void show(XeenEngine *vm, const Common::String &msg,
		ErrorWaitType waitType = WT_FREEZE_WAIT);
};

class ErrorScroll {
public:
	static void show(XeenEngine *vm, const Common::String &msg,
		ErrorWaitType waitType = WT_FREEZE_WAIT);
};

class CantCast: public ButtonContainer {
private:
	CantCast(XeenEngine *vm) : ButtonContainer(vm) {}

	void execute(int spellId, int componentNum);
public:
	static void show(XeenEngine *vm, int spellId, int componentNum);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_ERROR_H */
