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

#ifndef ILLUSIONS_BBDOU_BBDOU_MENUKEYS_H
#define ILLUSIONS_BBDOU_BBDOU_MENUKEYS_H

#include "illusions/specialcode.h"
#include "illusions/thread.h"
#include "common/array.h"

namespace Illusions {

class IllusionsEngine_BBDOU;

struct MenuKey {
	uint bitMask;
	uint32 threadId;
};

class BBDOUMenuKeys {
public:
	BBDOUMenuKeys(IllusionsEngine_BBDOU *vm);
	~BBDOUMenuKeys();
	void addMenuKey(uint bitMask, uint32 threadId);
	void update();
protected:
	typedef Common::Array<MenuKey> MenuKeys;
	IllusionsEngine_BBDOU *_vm;
	MenuKeys _menuKeys;
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_MENUKEYS_H
