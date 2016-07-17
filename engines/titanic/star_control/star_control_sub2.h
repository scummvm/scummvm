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

#ifndef TITANIC_STAR_CONTROL_SUB2_H
#define TITANIC_STAR_CONTROL_SUB2_H

#include "titanic/star_control/base_star.h"

namespace Titanic {

class CStarControlSub2: public CBaseStar {
public:
	virtual ~CStarControlSub2() {}

	virtual bool proc3(int v1);
	virtual bool proc4(int v1, int v2, int v3, int v4, int v5);
	virtual bool loadStar();
	virtual bool proc7(int v1, int v2);

	/**
	 * Setup the control
	 */
	bool setup();
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB2_H */
