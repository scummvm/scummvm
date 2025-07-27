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
#include "graphics/renderer.h"
#include "graphics/surface.h"
#include "hpl1/opengl.h"

namespace Hpl1 {

#ifdef HPL1_USE_OPENGL
static Graphics::RendererType getBestRendererType() {
	Common::String renderConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::Renderer::parseTypeCode(renderConfig);
	uint32 availableRendererTypes = Graphics::Renderer::getAvailableTypes();
	// only the following renderer types are supported for now
	availableRendererTypes &= Graphics::kRendererTypeOpenGL | Graphics::kRendererTypeOpenGLShaders | Graphics::kRendererTypeTinyGL;
	return Graphics::Renderer::getBestMatchingType(
		desiredRendererType, availableRendererTypes);
}
#endif

bool areShadersAvailable() {
#ifdef HPL1_USE_OPENGL
	return getBestRendererType() == Graphics::kRendererTypeOpenGLShaders;
#else
	return false;
#endif
}

Common::ScopedPtr<Graphics::Surface> createViewportScreenshot() {
#ifdef HPL1_USE_OPENGL
	return createGLViewportScreenshot();
#else
	return nullptr;
#endif
}

bool useOpenGL() {
#ifdef HPL1_USE_OPENGL
	Graphics::RendererType bestRendererType = getBestRendererType();
	return bestRendererType == Graphics::kRendererTypeOpenGL ||
	       bestRendererType == Graphics::kRendererTypeOpenGLShaders;
#else
	return false;
#endif
}

} // namespace Hpl1
