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

#ifndef ILLUSIONS_SCRIPTMAN_H
#define ILLUSIONS_SCRIPTMAN_H

#include "illusions/illusions_bbdou.h"
#include "illusions/scriptresource.h"
#include "illusions/thread.h"
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

class IllusionsEngine_BBDOU;

class ScriptStack {
public:
	ScriptStack();
	void clear();
	void push(int16 value);
	int16 pop();
	int16 peek();
	int16 *topPtr();
protected:
	int _stackPos;
	int16 _stack[256];
};

class ScriptMan {
public:
	ScriptMan(IllusionsEngine_BBDOU *vm);
	~ScriptMan();
public:

	IllusionsEngine_BBDOU *_vm;
	
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCRIPTMAN_H
