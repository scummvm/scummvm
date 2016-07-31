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

namespace Titanic {

class CStarControlSub5 {
	struct SubEntry {
		int _field0;
		int _field4;
		int _field8;
		int _fieldC;
	};
private:
	int _field4;
	SubEntry _array[5];
	CStarControlSub6 _sub1, _sub2;
	int _field7914;
	int _field78AC;
	int _field78B0;
public:
	CStarControlSub5();
	virtual ~CStarControlSub5() {}

	virtual bool setup();
	virtual void proc2();
	virtual void proc3(CErrorCode *errorCode);

	int get4() const { return _field4; }
	void set4(int val) { _field4 = val; }
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB5_H */
