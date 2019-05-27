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

#ifndef DRAGONS_TALK_H
#define DRAGONS_TALK_H

#include <common/str.h>

namespace Dragons {

class BigfileArchive;
class Actor;
class DragonsEngine;

class Talk {
private:
	DragonsEngine *_vm;
	BigfileArchive *_bigfileArchive;
public:
	Talk(DragonsEngine *vm, BigfileArchive *bigfileArchive);
	void init();
	char *loadText(uint32 textIndex);
	void printWideText(byte *text);

	void FUN_8003239c(char *dialog, int16 x, int16 y, int32 param_4, uint16 param_5, Actor *actor, uint16 startSequenceId, uint16 endSequenceId, uint32 textId);
};

} // End of namespace Dragons

#endif //DRAGONS_TALK_H
