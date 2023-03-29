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

#include "hpl1/graphics.h"

#include "common/config-manager.h"
#include "graphics/opengl/context.h"
#include "graphics/surface.h"
#include "hpl1/opengl.h"

namespace Hpl1 {

bool areShadersAvailable() {
#ifdef USE_OPENGL
	return useOpenGL() && OpenGLContext.enginesShadersSupported &&
		   (!ConfMan.hasKey("renderer") || ConfMan.get("renderer") == "opengl_shaders");
#endif
	return false;
}

Common::ScopedPtr<Graphics::Surface> createViewportScreenshot() {
#ifdef USE_OPENGL
	return createGLViewportScreenshot();
#endif
	return nullptr;
}

bool useOpenGL() {
#ifdef USE_OPENGL
	return (!ConfMan.hasKey("renderer") || ConfMan.get("renderer").contains("opengl"));
#endif
	return false;
}

} // namespace Hpl1