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

#ifndef WATCHMAKER_LLMATH_H
#define WATCHMAKER_LLMATH_H

#include "watchmaker/t3d.h"

namespace Watchmaker {

void t3dMatCopy(t3dM3X3F *d, t3dM3X3F *s);
void t3dMatIdentity(t3dM3X3F *d);
void t3dMatSet(t3dM3X3F *d,
               t3dF32 _m11, t3dF32 _m12, t3dF32 _m13,
               t3dF32 _m21, t3dF32 _m22, t3dF32 _m23,
               t3dF32 _m31, t3dF32 _m32, t3dF32 _m33);
void t3dMatMul(t3dM3X3F *Dest, t3dM3X3F *a, t3dM3X3F *b);
void t3dMatRotAxis(t3dM3X3F *m, t3dF32 x, t3dF32 y, t3dF32 z, t3dF32 rad);
void t3dMatRot(t3dM3X3F *matrix, t3dF32 x, t3dF32 y, t3dF32 z);
void t3dMatReflect(t3dM3X3F *Matrix, t3dV3F *mirrorpos, t3dNORMAL *n);
void t3dMatCopy(t3dM3X3F *d, t3dM3X3F *s);
void t3dMatView(t3dM3X3F *dest, t3dV3F *eye, t3dV3F *center);
void t3dVectTransform(t3dV3F *d, t3dV3F *s, t3dM3X3F *mat);
void t3dVectTransformInv(t3dV3F *d, t3dV3F *s, t3dM3X3F *mat);
void t3dVectInit(t3dV3F *a, t3dF32 x, t3dF32 y, t3dF32 z);
void t3dVectCross(t3dV3F *d, t3dV3F *v2, t3dV3F *v3);
void t3dVectSub(t3dV3F *d, t3dV3F *a, t3dV3F *b);            // d = a - b
void t3dVectAdd(t3dV3F *d, t3dV3F *a, t3dV3F *b);            // d = a - b
void t3dVectFill(t3dV3F *d, t3dF32 a);
void t3dVectInit(t3dV3F *d, t3dF32 a1, t3dF32 a2, t3dF32 a3);
void t3dVectCopy(t3dV3F *d, t3dV3F *s);
t3dF32 t3dVectMod(t3dV3F *c);
t3dF32 t3dVectDistance(t3dV3F *a, t3dV3F *b);
t3dF32 t3dVectSquaredDistance(t3dV3F *a, t3dV3F *b);
t3dF32 t3dPointSquaredDistance(t3dV3F *c);
void t3dVectNormalize(t3dV3F *c);
t3dF32 t3dVectDot(t3dV3F *a, t3dV3F *b);
void t3dMatMulInv(t3dM3X3F *Dest, t3dM3X3F *a, t3dM3X3F *b);
void t3dVectReflection(t3dV3F *r, t3dV3F *p1, t3dNORMAL *plane, t3dF32 dist);
void t3dMatInv(t3dM3X3F *d, t3dM3X3F *s);
void t3dPlaneNormal(t3dNORMAL *n, t3dV3F *p0, t3dV3F *p1, t3dV3F *p2);
void t3dVector2dTo3d(t3dV3F *end, t3dV2F *start);
int32 t3dFloatToInt(t3dF32 nfloat);
t3dF32 t3dVectPlaneDistance(t3dV3F start, t3dNORMAL n);  //return the distance of start from plane normal n
uint8 t3dVectPlaneIntersection(t3dV3F *inter, t3dV3F start, t3dV3F end, t3dNORMAL n);    //if return value !=0 inter contains the intersection point
uint8 t3dVectTriangleIntersection(t3dV3F *inter, t3dV3F start, t3dV3F end, t3dV3F v1, t3dV3F v2, t3dV3F v3, t3dNORMAL n); //return 1 if the vector from start to end pass trought the triangle v1,v2,v3

} // End of namespace Watchmaker

#endif // WATCHMAKER_LLMATH_H
