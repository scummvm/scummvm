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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_MATHUTIL_H
#define WINTERMUTE_MATHUTIL_H

#ifdef ENABLE_WME3D
#include "math/vector3d.h"
#include "engines/wintermute/base/gfx/xmath.h"
#endif

namespace Wintermute {

class MathUtil {
public:
	static float round(float val);
	static float roundUp(float val);
};

#ifdef ENABLE_WME3D

#define DX_PI    ((float)3.141592654)
#define degToRad(_val) (_val * DX_PI * (1.0f / 180.0f))
#define radToDeg(_val) (_val * (180.0f / DX_PI))

bool intersectTriangle(const Math::Vector3d &origin, const Math::Vector3d &direction,
							const Math::Vector3d &v0, const Math::Vector3d &v1, const Math::Vector3d &v2,
							float &t, float &u, float &v);
bool pickGetIntersect(const Math::Vector3d &lineStart, const Math::Vector3d &lineEnd,
								   const Math::Vector3d &v0, const Math::Vector3d &v1, const Math::Vector3d &v2,
								   Math::Vector3d &intersection, float &distance);
DXMatrix *matrixSetTranslation(DXMatrix *mat, DXVector3 *vec);
DXMatrix *matrixSetRotation(DXMatrix *mat, DXVector3 *vec);
void decomposeMatrixSimple(const DXMatrix *mat, DXVector3 *transVec, DXVector3 *scaleVec, DXQuaternion *rotQ);

#endif

} // End of namespace Wintermute

#endif
