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

#ifndef GROOVIE_LOGIC_T11HGAME_H
#define GROOVIE_LOGIC_T11HGAME_H

#include "common/textconsole.h"
#include "common/random.h"

namespace Groovie {

class GroovieEngine;

class T11hGame {
public:
#ifdef ENABLE_GROOVIE2
	T11hGame(byte *scriptVariables);
	~T11hGame();

	void handleOp(uint8 op);

private:
	Common::RandomSource _random;

	void opMouseTrap();
	void opBeehive();
	void opPente();
	void opGallery();

	byte opGallerySub(int one, byte *field);
	void inline setScriptVar(uint16 var, byte value);
	void inline setScriptVar16(uint16 var, uint16 value);
	uint16 inline getScriptVar16(uint16 var);
	byte *_scriptVariables;
	static const byte kGalleryLinks[21][10];
#endif
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_T11HGAME_H
