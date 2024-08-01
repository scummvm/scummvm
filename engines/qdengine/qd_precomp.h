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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#ifndef QDENGINE_QD_PRECOMP_H
#define QDENGINE_QD_PRECOMP_H

#define __QD_DEBUG_ENABLE__

#include "qdengine/xmath.h"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "engines/qdengine/qd_fwd.h"

namespace QDEngine {

#define VK_ESCAPE     0x1B
#define VK_RETURN     0x0D
#define VK_SPACE      0x20
#define VK_LEFT       0x25
#define VK_UP         0x26
#define VK_RIGHT      0x27
#define VK_DOWN       0x28
#define VK_BACK       0x08
#define VK_DELETE     0x2E
#define VK_HOME       0x24
#define VK_END        0x23
#define VK_F10        0x79
#define VK_F9         0x78
#define VK_F5         0x74
#define VK_F6         0x75
#define VK_PAUSE      0x13
#define VK_NEXT       0x22
#define VK_PRIOR      0x21

int engineMain();

} // namespace QDEngine

#endif // QDENGINE_QD_PRECOMP_H
