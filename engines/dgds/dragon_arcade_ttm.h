/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DGDS_DRAGON_ARCADE_TTM_H
#define DGDS_DRAGON_ARCADE_TTM_H

#include "common/types.h"

namespace Dgds {

/** A TTM interpreter which is simpler than the main one and
   specialized to the arcade sequences. */
class DragonArcadeTTM {
public:
	DragonArcadeTTM();
	void clearDataPtrs();
	void load(const char *filename);
	void finishTTMParse();
	void runNextPage(uint16 num);
	void freePages(uint16 num);
	void freeShapes();

	uint16 _currentTTMNum;
	bool _doingInit;
};

} // end namespace Dgds

#endif // DGDS_DRAGON_ARCADE_TTM_H
