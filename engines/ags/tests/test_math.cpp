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

#include "ags/shared/core/platform.h"
#include "ags/shared/debugging/assert.h"
#include "ags/engine/util/scaling.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void Test_Scaling(int src, int dst) {
	int x;
	AxisScaling sc;
	sc.Init(src, dst);
	x = sc.ScalePt(0);
	assert(x == 0);
	x = sc.ScalePt(src);
	assert(x == dst);
	x = sc.UnScalePt(dst);
	assert(x == src);
}

void Test_Math() {
	Test_Scaling(100, 100);

	Test_Scaling(100, 1000);
	Test_Scaling(320, 1280);
	Test_Scaling(200, 400);

	Test_Scaling(1000, 100);
	Test_Scaling(1280, 320);
	Test_Scaling(400, 200);

	Test_Scaling(300, 900);
	Test_Scaling(100, 700);
	Test_Scaling(200, 2200);

	Test_Scaling(900, 300);
	Test_Scaling(700, 100);
	Test_Scaling(2200, 200);

	for (int i = 250; i < 2000; i += 25) {
		Test_Scaling(200, i);
		Test_Scaling(i, 200);
	}
}

} // namespace AGS3
