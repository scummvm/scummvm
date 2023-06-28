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

#include "m4/graphics/gr_series.h"
#include "m4/core/errors.h"
#include "m4/wscript/ws_load.h"

namespace M4 {

int32 series_load(const char *seriesName, int32 assetIndex, RGB8 *myPal) {
	int32 myAssetIndex = AddWSAssetCELS(seriesName, assetIndex, myPal);

	if ((myAssetIndex < 0) || (myAssetIndex >= 256))
		error_show(FL, 'SPNF', seriesName);

	return myAssetIndex;
}

void series_unload(int32 assetIndex) {
	ClearWSAssets(_WS_ASSET_CELS, assetIndex, assetIndex);
}

bool series_draw_sprite(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y) {
	error("TODO: series_draw_sprite");
	return false;
}

} // namespace M4
