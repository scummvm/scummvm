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

#ifndef TITANIC_STAR_CONTROL_SUB23_H
#define TITANIC_STAR_CONTROL_SUB23_H

#include "titanic/star_control/error_code.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/fvector.h"
#include "titanic/star_control/star_control_sub25.h"

namespace Titanic {

class CStarControlSub23 {
protected:
	int _field4;
	int _field8;
	FVector _row1, _row2;
	double _field24;
	FVector _row3;
	int _field34;
	double _field38;
	int _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _field4C;
	Common::Array<double> _powers;
	int _field54;
	int _field58;
	double _field5C;
	double _field60;
	double _field64;
	CStarControlSub25 _sub25;
public:
	CStarControlSub23();
	virtual ~CStarControlSub23() {}

	virtual void proc2(FVector &v1, FVector &v2, FMatrix &m1, FMatrix &m2);
	virtual void proc3(const FMatrix &m1, const FMatrix &m2);
	virtual void proc4(FVector &v1, FVector &v2, FMatrix &m);
	virtual int proc5(CErrorCode &errorCode, FVector &v, const FMatrix &m) { return 2; }
	virtual void proc6(int val1, int val2, float val);

	int get8() const { return _field8; }
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB23_H */
