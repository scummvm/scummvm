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
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_3DUTILS_H
#define WINTERMUTE_3DUTILS_H

#ifdef ENABLE_WME3D
#include "engines/wintermute/base/gfx/xmath.h"
#endif

namespace Wintermute {

#ifdef ENABLE_WME3D

#define DX_PI    ((float)3.141592654)
#define degToRad(_val) (_val * DX_PI * (1.0f / 180.0f))
#define radToDeg(_val) (_val * (180.0f / DX_PI))

class C3DUtils {
public:
	static bool intersectTriangle(const DXVector3 &orig, const DXVector3 &dir,
	                              DXVector3 &v0, DXVector3 &v1, DXVector3 &v2,
	                              float *t, float *u, float *v);
	static bool pickGetIntersect(DXVector3 linestart, DXVector3 lineend,
	                             DXVector3 v0, DXVector3 v1, DXVector3 v2,
	                             DXVector3 *intersection, float *distance);
	static DXMatrix *matrixSetTranslation(DXMatrix *mat, DXVector3 *vec);
	static DXMatrix *matrixSetRotation(DXMatrix *mat, DXVector3 *vec);
	static void decomposeMatrixSimple(const DXMatrix *mat, DXVector3 *transVec,
	                                  DXVector3 *scaleVec, DXQuaternion *rotQ);
};

#endif

} // End of namespace Wintermute

#endif
