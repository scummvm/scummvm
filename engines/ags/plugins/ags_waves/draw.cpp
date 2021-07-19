/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_waves/ags_waves.h"
#include "ags/ags.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

void AGSWaves::CastWave(int delayMax, int PixelsWide, int n) {
	tDy[n]++;
	if (tDy[n] > delayMax) {
		tDy[n] = 0;
		if (direction[n] == 0) dY[n]++;
		if (direction[n] == 1) dY[n]--;
		if ((dY[n] > PixelsWide &&direction[n] == 0) || (dY[n] < (-PixelsWide) && direction[n] == 1)) {
			if (direction[n] == 0) {
				dY[n] = PixelsWide; direction[n] = 1;
			} else {
				dY[n] = -PixelsWide; direction[n] = 0;
			}
		}
	}
}

void AGSWaves::DrawEffect(int sprite_a, int sprite_b, int id, int n) {
	int32 x, y;

	BITMAP *src_a = _engine->GetSpriteGraphic(sprite_a);
	BITMAP *src_b = _engine->GetSpriteGraphic(sprite_b);

	uint32 **pixel_a = (uint32 **)_engine->GetRawBitmapSurface(src_a);
	uint32 **pixel_b = (uint32 **)_engine->GetRawBitmapSurface(src_b);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src_a, &src_width, &src_height, &src_depth);



	for (y = 0; y < src_height; y++) {
		if (id == 1)
			CastWave(15, 1, n);
		if (id == 0 || id == 9 || id == 2 || id == 3 || id == 6 || id == 8)
			CastWave(2, 1, n);
		if (id == 4)
			CastWave(15, 4, n);
		if (id == 5 || id == 7 || id == 10) {
			//x=0;
			CastWave(3, 1, n);
		}
		if (id == 11) {
			//x=0;
			CastWave(3, 2, n);
		}
		if (id == 16) CastWave(4, 1, n);
		if (id == 17) CastWave(6, 1, n);


		for (x = 0; x < src_width; x++) {
			uint32 colorfromB = pixel_b[y][x];
			int32 getX = x;
			int32 getY = y;

			if (id == 0) {
				getX = x - ::AGS::g_vm->getRandomNumber(1) - 2;
				getY = y + dY[n];
			}
			if (id == 1 || id == 4) {
				getX = x;
				getY = y + dY[n];
			}
			if (id == 2) {
				getX = x + dY[n];
				getY = y - ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 3) {
				getX = x;
				getY = y - ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 5) {
				getX = x + dY[n];
				getY = y - ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 6 || id == 16) {
				getX = x + dY[n];
				getY = y - (0 /* rand() % 1 */) - 1;
			}
			if (id == 7 || id == 17) {
				getX = x + dY[n];
				getY = y - (0 /* rand() % 1 */) - 1;
			}
			if (id == 8) {
				getX = x + dY[n];
				getY = y + ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 10 || id == 9 || id == 11) {
				getX = x + dY[n];
				getY = y;
			}

			if (getX < 0) getX = 0;
			if (getX > src_width - 1) getX = src_width - 1;
			if (getY > src_height - 1) getY = src_height - 1;
			if (getY < 0) getY = 0;


			pixel_a[getY][getX] = colorfromB;	  //
		}
	}

	_engine->ReleaseBitmapSurface(src_a);
	_engine->ReleaseBitmapSurface(src_b);
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
