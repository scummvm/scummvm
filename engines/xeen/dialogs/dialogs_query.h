/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef XEEN_DIALOGS_QUERY_H
#define XEEN_DIALOGS_QUERY_H

#include "xeen/dialogs/dialogs.h"

namespace Xeen {

class Confirm : public ButtonContainer {
private:
	Confirm(XeenEngine *vm) : ButtonContainer(vm) {}

	bool execute(const Common::String &msg, int mode);
public:
	static bool show(XeenEngine *vm, const Common::String &msg, int mode = 0);
};

class YesNo : public ButtonContainer {
private:
	YesNo(XeenEngine *vm) : ButtonContainer(vm) {}

	bool execute(bool type, bool townFlag);
public:
	static bool show(XeenEngine *vm, bool type, bool townFlag = false);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_QUERY_H */
