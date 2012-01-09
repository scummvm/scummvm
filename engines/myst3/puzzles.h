/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef PUZZLES_H_
#define PUZZLES_H_

#include "common/scummsys.h"

namespace Myst3 {

class Myst3Engine;

class Puzzles {
public:
	Puzzles(Myst3Engine *vm);
	virtual ~Puzzles();

	void run(uint16 id, uint16 arg0 = 0, uint16 arg1 = 0, uint16 arg3 = 0);

private:
	Myst3Engine *_vm;

	void journalSaavedro(int16 move);
	uint16 _journalSaavedroGetNode(uint16 chapter);
	uint16 _journalSaavedroPageCount(uint16 chapter);
	bool _journalSaavedroHasChapter(uint16 chapter);
	uint16 _journalSaavedroNextChapter(uint16 chapter, bool forward);

	void journalAtrus(uint16 node, uint16 var);
};

} /* namespace Myst3 */
#endif /* PUZZLES_H_ */
