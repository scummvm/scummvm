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

#ifndef GOT_GAME_STATUS_H
#define GOT_GAME_STATUS_H

namespace Got {

extern void addJewels(int num);
extern void addScore(int num);
extern void addMagic(int num);
extern void addHealth(int num);
extern void addKeys(int num);
extern void fillHealth();
extern void fillMagic();
extern void fillScore(int num, const char *endMessage = nullptr);

} // namespace Got

#endif
