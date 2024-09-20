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

#ifndef WATCHMAKER_WALKUTIL_H
#define WATCHMAKER_WALKUTIL_H

#include "watchmaker/types.h"
#include "watchmaker/t3d.h"

namespace Watchmaker {

t3dF32 SinCosAngle(t3dF32 sinus, t3dF32 cosinus);
t3dF32 t3dVectAngle(t3dV3F *n, t3dV3F *o);
int PointInside(int32 oc, int32 pan, const PointXZ &point);
int PointInside(int32 oc, int32 pan, double x, double z);
bool PointInside2DRectangle(double pgon[4][2], double x, double z);
float DistF(PointXZ a, PointXZ b);
float DistF(float x1, float y1, float x2, float y2);
PointResult IntersLineLine(const PointXZ &a, const PointXZ &b, float xc, float yc, float xd, float yd);
PointResult IntersLineLine(const PointXZ &a, const PointXZ &b, const PointXZ &c, const PointXZ &d);
PointResult IntersLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);
int PathCompare(const void *arg1, const void *arg2);
void SortPath(int32 oc);

} // End of namespace Watchmaker

#endif // WATCHMAKER_WALKUTIL_H
