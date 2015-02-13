/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/opengl.h"

#include "common/textconsole.h"

namespace Stark {
namespace Gfx {

GfxDriver *GfxDriver::create() {
	GfxDriver *driver = NULL;

#ifdef USE_OPENGL
	// OpenGL
	driver = new OpenGLGfxDriver();
	if (driver)
		return driver;
#endif // USE_OPENGL

	error("Couldn't instance any graphics driver");
}

} // End of namespace Gfx
} // End of namespace Stark
