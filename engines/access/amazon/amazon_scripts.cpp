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

#include "common/scummsys.h"
#include "access/access.h"
#include "access/amazon/amazon_scripts.h"

namespace Access {

namespace Amazon {

AmazonScripts::AmazonScripts(AccessEngine *vm) : Scripts(vm) {
}

void AmazonScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
	case 1:
		warning("TODO ESTABLISH");
		break;
	case 2:
		warning("TODO LOADBACKGROUND");
		break;
	case 3:
		warning("TODO DOCAST");
		break;
	case 4:
		warning("TODO SETINACTIVE");
		break;
	case 6:
		warning("TODO MWHILE");
		break;
	case 9:
		warning("TODO GUARD");
		break;
	case 10:
		warning("TODO NEWMUSIC");
		break;
	case 11:
		warning("TODO PLOTINACTIVE");
		break;
	case 13:
		warning("TODO RIVER");
		break;
	case 14:
		warning("TODO ANT");
		break;
	case 15:
		warning("TODO BOATWALLS");
		break;
	default:
		warning("Unexpected Special code %d - Skipped", commandIndex);
	}
}


} // End of namespace Amazon

} // End of namespace Access
