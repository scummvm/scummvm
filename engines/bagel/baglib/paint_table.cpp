
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

#include "bagel/baglib/paint_table.h"

namespace Bagel {

stripEnds STRIP_POINTS[153][120];

void PaintTable::initialize(Common::SeekableReadStream &src) {
 	for (int stripNum = 0; stripNum < 153; ++stripNum) {
		// Read comment line
		Common::String line = src.readLine();
		assert(line.hasPrefix("/*"));

		// Iterate through the following 12 lines to read the 120 points
		int pointIndex = 0;
		for (int lineNum = 0; lineNum < 12; ++lineNum) {
			line = src.readLine();

			for (int pointNum = 0; pointNum < 10; ++pointNum, ++pointIndex) {
				// Get the point
				int y1 = 0, y2 = 0;
				const int result = sscanf(line.c_str(), "{ %d,%d}", &y1, &y2);
				assert(result == 2);

				STRIP_POINTS[stripNum][pointIndex]._top = y1;
				STRIP_POINTS[stripNum][pointIndex]._bottom = y2;

				// Move to next point on line
				line = Common::String(strchr(line.c_str() + 1, '{'));
			}
		}
	}
}

} // namespace Bagel
