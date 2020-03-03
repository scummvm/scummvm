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

#ifndef GROOVIE_T11HGAME_H
#define GROOVIE_T11HGAME_H

#include "common/textconsole.h"
#include "common/random.h"

namespace Groovie {

class GroovieEngine;

class T11hGame
{
public:
	T11hGame();
	~T11hGame();
	/**
	* Sets a pointer to the script variables. This makes it easier if we want
	* to debug write accesses to the script variables
	* @param scriptVariables	The current variables from the script.
	*/
	void setVariables(byte *scriptVariables);

	void opGallery();
	byte opGallerySub(int one, byte *field);

private:
	Common::RandomSource _random;

	void inline setScriptVar(uint16 var, byte value);
	void inline setScriptVar16(uint16 var, uint16 value);
	uint16 inline getScriptVar16(uint16 var);
	byte *_scriptVariables;

};

} // End of Groovie namespace

#endif // GROOVIE_T11HGAME_H
