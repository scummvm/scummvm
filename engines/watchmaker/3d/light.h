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

#ifndef WATCHMAKER_LIGHT_H
#define WATCHMAKER_LIGHT_H

#include "watchmaker/t3d.h"

#define LIGHT_MAPVERSION    1
#define VOLLIGHTFILEVERSION 1
#define OUTDOORLIGHTSFILEVERSION    2

#define LIGHT_COORDS        (1<<0)
#define LIGHT_LIGHTMAPS     (1<<1)
#define LIGHT_SHADOWMAPS    (1<<2)

namespace Watchmaker {

void GetBoundaries(t3dBODY *b, float *minx, float *miny, float *minz, float *maxx, float *maxy, float *maxz);
unsigned char LightgVertex(gVertex *v, t3dLIGHT *light);
void t3dLoadOutdoorLights(const char *pname, t3dBODY *b, int32 ora);
void LoadVolumetricMap(WorkDirs &workDirs, const char *pname, t3dBODY *b);
Common::String setDirectoryAndName(const Common::String &path, const Common::String &name);
uint8 LightVertex(t3dVERTEX *vv, t3dV3F *v, t3dLIGHT *light);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LIGHT_H
