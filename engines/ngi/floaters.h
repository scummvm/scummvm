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

#ifndef NGI_FLOATERS_H
#define NGI_FLOATERS_H

#include "common/array.h"
#include "common/ptr.h"

namespace NGI {

class StaticANIObject;
class Scene;
class ReactPolygonal;

struct FloaterArray1 {
	int val1;
	int val2;

	FloaterArray1() { val1 = 0; val2 = 0; }
};

struct FloaterArray2 {
	StaticANIObject *ani;
	int val2;
	int val3;
	int val4;
	int val5;
	int val6;
	int val7;
	int val8;
	double val9;
	double val11;
	int val13;
	int countdown;
	int val15;
	int fflags;

	FloaterArray2() : ani(nullptr), val2(0), val3(0), val4(0), val5(0), val6(0), val7(0), val8(0),
		val9(0.0), val11(0.0), val13(0), countdown(0), val15(0), fflags(0) {}
};

class Floaters {
public:
	Common::ScopedPtr<ReactPolygonal> _hRgn;
	Common::Array<FloaterArray1> _array1;
	Common::Array<FloaterArray2> _array2;

	void init(GameVar *var);
	void genFlies(Scene *sc, int x, int y, int priority, int flags);
	void update();
	void stopAll();
};

} // End of namespace NGI

#endif /* NGI_FLOATERS_H */
