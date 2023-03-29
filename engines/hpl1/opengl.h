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

#ifndef HPL1_OPENGL_H
#define HPL1_OPENGL_H

#include "common/ptr.h"
#include "common/scummsys.h"
#include "graphics/opengl/context.h"
#include "graphics/opengl/system_headers.h"

#ifdef USE_OPENGL

namespace Graphics {

struct Surface;

}

namespace Hpl1 {

void checkOGLErrors(const char *function, const char *file, int line);

Common::ScopedPtr<Graphics::Surface> createGLViewportScreenshot();

} // namespace Hpl1

#define GL_CHECK(x)                                           \
	{                                                         \
		x;                                                    \
		::Hpl1::checkOGLErrors(__func__, __FILE__, __LINE__); \
	}
#define GL_CHECK_FN() \
	{ ::Hpl1::checkOGLErrors(__func__, __FILE__, __LINE__); }

#endif // USE_OPENGL
#endif // HPL1_OPENGL_H
