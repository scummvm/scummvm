/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef AGS_ENGINE_AC_GLOBAL_REGION_H
#define AGS_ENGINE_AC_GLOBAL_REGION_H

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

#endif
