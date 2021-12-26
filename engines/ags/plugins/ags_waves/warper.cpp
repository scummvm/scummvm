/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "common/util.h"
#include "ags/plugins/ags_waves/ags_waves.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {


void AGSWaves::ReturnNewHeight(ScriptMethodParams &params) {
	params._result = _newHeight;
}

void AGSWaves::ReturnNewWidth(ScriptMethodParams &params) {
	params._result = _newWidth;
}

void AGSWaves::Warper(ScriptMethodParams &params) {
	PARAMS5(int, swarp, int, sadjust, int, x1, int, y1, int, x2);

	ix = 0.0;
	iy = 0.0;
	ua = 0.0;
	// some precautions against non-positive values for width and height

	float ax = float(x1), ay = float(y1);
	float bx = float(x2), by = float(_y2);
	float cx = float(_x3), cy = float(_y3);
	float dx = float(_x4), dy = float(_y4);

	int w = int(max4(ax, bx, cx, dx)) + 1;
	int h = int(max4(ay, by, cy, dy)) + 1;

	BITMAP *refsrc = _engine->GetSpriteGraphic(swarp);
	int32 refsrc_width = 640;
	int32 refsrc_height = 360;
	int32 refsrc_depth = 32;
	_engine->GetBitmapDimensions(refsrc, &refsrc_width, &refsrc_height, &refsrc_depth);
	unsigned int **refsprite_pixels = (unsigned int **)_engine->GetRawBitmapSurface(refsrc);
	_engine->ReleaseBitmapSurface(refsrc);


	// create temporary sprite holding the warped version
	BITMAP *resizeb = _engine->GetSpriteGraphic(sadjust);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(resizeb, &src_width, &src_height, &src_depth);
	unsigned int **sprite_pixels = (unsigned int **)_engine->GetRawBitmapSurface(resizeb);


	int ow = refsrc_width, oh = refsrc_height;

	int x, y;  // pixel coords
	float fx, fy; // original sprite's in between pixel coords

	int il;

	// calculate intersections of opposing sides
	float orx_x, orx_y, ory_x, ory_y;
	bool xp = false, yp = false; // parallel sides?

	// AC and BD to get intersection of all "vertical lines"

	il = IntersectLines(ax, ay, cx, cy, bx, by, dx, dy);
	if (il == 0) {
		// parallel sides, store directional vector
		orx_x = cx - ax;
		orx_y = cy - ay;
		xp = true;
	} else {
		// store intersection of sides
		orx_x = ix;
		orx_y = iy;
	}
	// AB and CD to get intersection of all "horizontal lines"
	il = IntersectLines(ax, ay, bx, by, cx, cy, dx, dy);
	if (il == 0) {
		// parallel sides, store directional vector
		ory_x = bx - ax;
		ory_y = by - ay;
		yp = true;
	} else {
		// store intersection of sides
		ory_x = ix;
		ory_y = iy;
	}

	int xm = int(min4(ax, bx, cx, dx)); // x loop starts here

	y = int(min4(ay, by, cy, dy));
	while (y < h) {
		x = xm;
		while (x < w) {

			// calculate original pixel

			// x:
			if (xp) il = IntersectLines(ax, ay, bx, by, float(x), float(y), float(x) + orx_x, float(y) + orx_y);
			else il = IntersectLines(ax, ay, bx, by, float(x), float(y), orx_x, orx_y);
			fx = float(ow - 1) * ua;

			float ux = ua;

			// y:
			if (yp) il = IntersectLines(ax, ay, cx, cy, float(x), float(y), float(x) + ory_x, float(y) + ory_y);
			else il = IntersectLines(ax, ay, cx, cy, float(x), float(y), ory_x, ory_y);
			fy = float(oh - 1) * ua;

			// only draw if within original sprite
			if (ux >= 0.0 && ux <= 1.0 && ua >= 0.0 && ua <= 1.0) {
				int refY = (int)CLIP(fy, (float)0.0, float(refsrc_height - 1));
				int refX = (int)CLIP(fx, (float)0.0, float(refsrc_width - 1));

				int setcolor = refsprite_pixels[refY][refX];

				int setY = (int)CLIP((float)y, (float)0.0, (float)(src_height - 1));
				int setX = (int)CLIP((float)x, (float)0.0, (float)(src_width - 1));

				sprite_pixels[setY][setX] = setcolor;
			}

			x++;
		}

		y++;
	}

	_newWidth = w;
	_newHeight = h;
	_engine->ReleaseBitmapSurface(resizeb);
}

void AGSWaves::SetWarper(ScriptMethodParams &params) {
	//PARAMS5(int, y2x, int, x3x, int, y3x, int, x4x, int, y4x);
}


int AGSWaves::IntersectLines(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
	// check a
	if (x1 == x2 && y1 == y2)
		return -1;
	// check b
	if (x3 == x4 && y3 == y4)
		return -1;
	float den = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
	float num12 = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);
	float num34 = (x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3);

	if (den == 0.0) {  // no intersection
		if (num12 == 0.0 && num34 == 0.0)
			return 2;
		return 0;
	}

	ua = num12 / den;
	ix = x1 + ua * (x2 - x1);
	iy = y1 + ua * (y2 - y1);

	return 1;
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
