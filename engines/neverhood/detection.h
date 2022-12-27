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

#ifndef NEVERHOOD_DETECTION_H
#define NEVERHOOD_DETECTION_H

namespace Neverhood {

enum NeverhoodGameFeatures {
	GF_BIG_DEMO = (1 << 0)
};

#define GAMEOPTION_ORIGINAL_SAVELOAD      GUIO_GAMEOPTIONS1
#define GAMEOPTION_SKIP_HALL_OF_RECORDS   GUIO_GAMEOPTIONS2
#define GAMEOPTION_SCALE_MAKING_OF_VIDEOS GUIO_GAMEOPTIONS3
#define GAMEOPTION_REPEAT_WILLIE_HINT     GUIO_GAMEOPTIONS4

} // End of namespace Neverhood

#endif // NEVERHOOD_DETECTION_H
