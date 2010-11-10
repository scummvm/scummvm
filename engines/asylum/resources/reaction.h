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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_REACTION_H
#define ASYLUM_REACTION_H

#include "common/scummsys.h"

namespace Asylum {

class AsylumEngine;

class Reaction {
public:
	Reaction(AsylumEngine *engine);
	virtual ~Reaction();

	void run(int32 reactionIndex);

private:
	AsylumEngine* _vm;

	//////////////////////////////////////////////////////////////////////////
	// Reaction functions
	//////////////////////////////////////////////////////////////////////////
	void chapter1(int32 reactionIndex);
	void chapter2(int32 reactionIndex);
	void chapter3(int32 reactionIndex);
	void chapter4(int32 reactionIndex);
	void chapter5(int32 reactionIndex);
	void chapter6(int32 reactionIndex);
	void chapter7(int32 reactionIndex);
	void chapter8(int32 reactionIndex);
	void chapter9(int32 reactionIndex);
	void chapter10(int32 reactionIndex);
	void chapter11(int32 reactionIndex);
	void chapter12(int32 reactionIndex);

	//////////////////////////////////////////////////////////////////////////
	// Utils
	//////////////////////////////////////////////////////////////////////////
	void play(int32 index);
};

} // End of namespace Asylum

#endif // ASYLUM_REACTION_H
