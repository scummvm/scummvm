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

#ifndef GLK_SCOTT_HULK_H
#define GLK_SCOTT_HULK_H

struct GameInfo;

namespace Glk {
namespace Scott {

void hulkShowImageOnExamine(int noun);
void hulkLook();
void drawHulkImage(int p);
int tryLoadingHulk(GameInfo info, int dictStart);

} // End of namespace Scott
} // End of namespace Glk

#endif
