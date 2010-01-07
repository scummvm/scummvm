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

#include "wage/wage.h"
#include "wage/macresman.h"

#include "common/stream.h"

namespace Wage {

bool WageEngine::loadWorld(MacResManager *resMan) {
	int resSize;
	MacResIDArray resArray;
	byte *res;
	MacResIDArray::const_iterator iter;

	if ((resArray = resMan->getResIDArray("VERS")).size() == 0)
		return false;

	if (resArray.size() > 1)
		warning("Too many VERS resources");

	res = resMan->getResource("VERS", resArray[0], &resSize);

	Common::MemoryReadStream readS(res, resSize);
	readS.skip(10);
	byte b = readS.readByte();
	_weaponMenuDisabled = (b != 0);
	if (b != 0 && b != 1)
		error("Unexpected value for weapons menu");

	readS.skip(3);
	_aboutMessage = readPascalString(readS);

	if (!stricmp(getGameFile(), "Scepters"))
		readS.skip(1); // ????

	_soundLibrary1 = readPascalString(readS);
	_soundLibrary2 = readPascalString(readS);

	debug(0, "%s\n%s", _soundLibrary1.c_str(), _soundLibrary2.c_str());

	return true;
}

} // End of namespace Wage
