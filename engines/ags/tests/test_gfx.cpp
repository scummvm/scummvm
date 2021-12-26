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

#include "common/scummsys.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/shared/debugging/assert.h"

namespace AGS3 {

namespace GfxDef = AGS::Shared::GfxDef;

void Test_Gfx() {
	// Test that every transparency which is a multiple of 10 is converted
	// forth and back without loosing precision
	const size_t arr_sz = 11;
	const int trans100[arr_sz] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	int trans255[arr_sz] = { 0 };
	int trans100_back[arr_sz] = { 0 };

	for (int i = 0; i < arr_sz; ++i) {
		trans255[i] = GfxDef::Trans100ToLegacyTrans255(trans100[i]);
		trans100_back[i] = GfxDef::LegacyTrans255ToTrans100(trans255[i]);
		assert(trans100[i] == trans100_back[i]);
	}
}

} // namespace AGS3
