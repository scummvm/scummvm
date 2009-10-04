/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 */

#include "common/util.h"

namespace Graphics {

void drawLine(int x0, int y0, int x1, int y1, int color, void (*plotProc)(int, int, int, void *), void *data) {
	// Bresenham's line algorithm, as described by Wikipedia
	const bool steep = ABS(y1 - y0) > ABS(x1 - x0);

	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}

	const int delta_x = ABS(x1 - x0);
	const int delta_y = ABS(y1 - y0);
	const int delta_err = delta_y;
	int x = x0;
	int y = y0;
	int err = 0;

	const int x_step = (x0 < x1) ? 1 : -1;
	const int y_step = (y0 < y1) ? 1 : -1;

	if (steep)
		(*plotProc)(y, x, color, data);
	else
		(*plotProc)(x, y, color, data);

	while (x != x1) {
		x += x_step;
		err += delta_err;
		if (2 * err > delta_x) {
			y += y_step;
			err -= delta_x;
		}
		if (steep)
			(*plotProc)(y, x, color, data);
		else
			(*plotProc)(x, y, color, data);
	}
}


// FIXME: This is a limited version of thick line drawing
// it draws striped lines at some angles. Better algorithm could
// be found here:
//
//   http://homepages.enterprise.net/murphy/thickline/index.html
//
// Feel free to replace it with better implementation
void drawThickLine(int x0, int y0, int x1, int y1, int thickness, int color, void (*plotProc)(int, int, int, void *), void *data) {
	const bool steep = ABS(y1 - y0) > ABS(x1 - x0);

	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}

	float dx = x1 - x0;
	float dy = y1 - y0;
	float d = (float)sqrt(dx * dx + dy * dy);

	if (!d)
		return;

	int thickX = (int)((float)thickness * dy / d / 2);
	int thickY = (int)((float)thickness * dx / d / 2);

	const int delta_x = ABS(x1 - x0);
	const int delta_y = ABS(y1 - y0);
	const int delta_err = delta_y;
	int x = x0;
	int y = y0;
	int err = 0;

	const int x_step = (x0 < x1) ? 1 : -1;
	const int y_step = (y0 < y1) ? 1 : -1;

	if (steep)
		drawLine(y - thickY, x + thickX, y + thickY, x - thickX, color, plotProc, data);
	else
		drawLine(x - thickX, y + thickY, x + thickX, y - thickY, color, plotProc, data);

	while (x != x1) {
		x += x_step;
		err += delta_err;
		if (2 * err > delta_x) {
			y += y_step;
			err -= delta_x;
		}
		if (steep)
			drawLine(y - thickY, x + thickX, y + thickY, x - thickX, color, plotProc, data);
		else
			drawLine(x - thickX, y + thickY, x + thickX, y - thickY, color, plotProc, data);
	}
}

}	// End of namespace Graphics
