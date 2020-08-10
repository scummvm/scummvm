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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_MATHUTIL_H
#define WINTERMUTE_MATHUTIL_H

#ifdef ENABLE_WME3D
#include "math/vector3d.h"
#endif

namespace Wintermute {

class MathUtil {
public:
	static float round(float val);
	static float roundUp(float val);
};

#ifdef ENABLE_WME3D
bool lineIntersectsTriangle(const Math::Vector3d &origin, const Math::Vector3d &direction,
                            const Math::Vector3d &v0, const Math::Vector3d &v1, const Math::Vector3d &v2,
                            float &t, float &u, float &v);

bool lineSegmentIntersectsTriangle(const Math::Vector3d &lineStart, const Math::Vector3d &lineEnd,
                                   const Math::Vector3d &v0, const Math::Vector3d &v1, const Math::Vector3d &v2,
								   Math::Vector3d &intersection, float &distance);
#endif

} // End of namespace Wintermute

#endif
