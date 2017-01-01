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

#ifndef TITANIC_STAR_CONTROL_SUB20_H
#define TITANIC_STAR_CONTROL_SUB20_H

#include "titanic/support/simple_file.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/fmatrix.h"

namespace Titanic {

struct CStar20Data {
	double _field0;
	double _field4;
	double _field8;
	double _fieldC;
	double _field10;
	double _field14;
	double _field18;
	double _field1C;
};

class CStarControlSub20 : public CStar20Data {
public:
	int _lockCounter;
	byte *_dataP;
public:
	CStarControlSub20(const CStar20Data *src);
	virtual ~CStarControlSub20();

	virtual void copyFrom(const CStar20Data *src);
	virtual void copyTo(CStar20Data *dest);
	virtual void proc4();
	virtual void proc5();
	virtual void proc6();
	virtual void proc7();
	virtual void proc8() {}
	virtual void proc9(FVector *v, int v2, FMatrix *matrix) {}
	virtual void proc10() {}
	virtual void proc11(CErrorCode &errorCode, FVector &v, const FMatrix &m);

	/**
	 * Set the data
	 */
	virtual void setData(void *data);

	/**
	 * Clear the class
	 */
	virtual void clear();

	/**
	 * Load the class
	 */
	virtual void load(SimpleFile *file, int val = 0);

	/**
	 * Save the class
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Increment tthe lock counter
	 */
	void incLockCount();

	/**
	 * Decrement the lock counter
	 */
	void decLockCount();

	/**
	 * Returns true if the lock counter is non-zero
	 */
	bool isLocked() const { return _lockCounter > 0; }
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB20_H */
