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

#ifndef TITANIC_STAR_CONTROL_SUB5_H
#define TITANIC_STAR_CONTROL_SUB5_H

#include "titanic/star_control/star_control_sub6.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/surface_area.h"

namespace Titanic {

class CStarControlSub12;

class CStarControlSub5 {
	struct SubEntry {
		int _field0;
		int _field4;
		int _field8;
		int _fieldC;
	};
private:
	bool _flag;
	CStarControlSub6 _sub1, _sub2;
#if 0
	SubEntry _array[5];
	int _field7914;
	int _field78AC;
	int _field78B0;
#endif
public:
	CStarControlSub5();
	virtual ~CStarControlSub5() {}

	virtual bool setup();
	virtual void proc2(CStarControlSub6 *sub6, FVector *vector, double v1, double v2, double v3,
		CSurfaceArea *surfaceArea, CStarControlSub12 *sub12);
	virtual void proc3(CErrorCode *errorCode);

	bool get4() const { return _flag; }
	void set4(bool val) { _flag = val; }

	void fn1();
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB5_H */
