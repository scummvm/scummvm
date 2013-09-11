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

#include "fullpipe/fullpipe.h"

#include "fullpipe/constants.h"

namespace Fullpipe {

int FullpipeEngine::lift_getButtonIdP(int objid) {
	switch (objid) {
	case ST_LBN_0N:
		return ST_LBN_0P;
		break;
	case ST_LBN_1N:
		return ST_LBN_1P;
		break;
	case ST_LBN_2N:
		return ST_LBN_2P;
		break;
	case ST_LBN_3N:
		return ST_LBN_3P;
		break;
	case ST_LBN_4N:
		return ST_LBN_4P;
		break;
	case ST_LBN_5N:
		return ST_LBN_5P;
		break;
	case ST_LBN_6N:
		return ST_LBN_6P;
		break;
	case ST_LBN_7N:
		return ST_LBN_7P;
		break;
	case ST_LBN_8N:
		return ST_LBN_8P;
		break;
	case ST_LBN_9N:
		return ST_LBN_9P;
		break;
	default:
		return 0;
		break;
	}
}

} // End of namespace Fullpipe
