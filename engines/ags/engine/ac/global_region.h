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

#ifndef AGS_ENGINE_AC_GLOBAL_REGION_H
#define AGS_ENGINE_AC_GLOBAL_REGION_H

namespace AGS3 {

// Gets region ID at the given room coordinates;
// if region is disabled or non-existing, returns 0 (no area)
int  GetRegionIDAtRoom(int xxx, int yyy);
void SetAreaLightLevel(int area, int brightness);
void SetRegionTint(int area, int red, int green, int blue, int amount, int luminance = 100);
void DisableRegion(int hsnum);
void EnableRegion(int hsnum);
void DisableGroundLevelAreas(int alsoEffects);
void EnableGroundLevelAreas();
void RunRegionInteraction(int regnum, int mood);

} // namespace AGS3

#endif
