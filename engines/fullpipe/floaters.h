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

#ifndef FULLPIPE_FLOATERS_H
#define FULLPIPE_FLOATERS_H

namespace Fullpipe {

class StaticANIObject;
class Scene;

struct FloaterArray1 {
	int val1;
	int val2;
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
};

class Floaters {
public:
	//HRGN hRgn;
	Common::Array<FloaterArray1 *> _array1;
	Common::Array<FloaterArray2 *> _array2;

	void init(GameVar *var);
	void genFlies(Scene *sc, int x, int y, int a5, int a6);
	void update();
	void stopAll();
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_FLOATERS_H */
