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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"
#include "graphics/renderer.h"

#include "colony/renderer.h"
#include "colony/renderer_opengl_shaders.h"

namespace Colony {

// Forward declaration for the fixed-function OpenGL renderer factory.
Renderer *createOpenGLRenderer(OSystem *system, int width, int height);

// Pick the renderer type. Honors --renderer=<code> on the command line /
// ConfMan key, restricted to what was compiled in.
//
// Phase 1 policy: default users get fixed-function OpenGL. The shader path
// is opt-in (--renderer=opengl_shaders) until later phases bring its
// primitive coverage up to parity. Note that the generic preference order
// in graphics/renderer.cpp:122 picks shaders for the Default case, so we
// must override it here.
static Graphics::RendererType pickRendererType() {
	const Common::String configured = ConfMan.get("renderer");
	const Graphics::RendererType desired = Graphics::Renderer::parseTypeCode(configured);

	if (desired == Graphics::kRendererTypeDefault) {
#ifdef USE_OPENGL_GAME
		return Graphics::kRendererTypeOpenGL;
#elif defined(USE_OPENGL_SHADERS)
		return Graphics::kRendererTypeOpenGLShaders;
#else
		return Graphics::kRendererTypeDefault;
#endif
	}

	const uint32 supported =
#ifdef USE_OPENGL_GAME
		Graphics::kRendererTypeOpenGL |
#endif
#ifdef USE_OPENGL_SHADERS
		Graphics::kRendererTypeOpenGLShaders |
#endif
		0;
	const Graphics::RendererType matching =
		Graphics::Renderer::getBestMatchingAvailableType(desired, supported);

	if (matching != desired)
		warning("Colony: requested renderer '%s' is unavailable, falling back",
			configured.c_str());
	return matching;
}

Renderer *createRenderer(OSystem *system, int width, int height) {
	const Graphics::RendererType type = pickRendererType();
	initGraphics3d(width, height);

#if defined(USE_OPENGL_SHADERS)
	if (type == Graphics::kRendererTypeOpenGLShaders) {
		Renderer *r = createOpenGLShaderRenderer(system, width, height);
		if (r)
			return r;
		warning("Colony: shader renderer factory returned null, falling back to fixed-function");
	}
#endif

#if defined(USE_OPENGL_GAME)
	if (type == Graphics::kRendererTypeOpenGL || type == Graphics::kRendererTypeDefault)
		return createOpenGLRenderer(system, width, height);
#endif

	// Last resort: try fixed-function unconditionally so the engine still
	// runs in builds where neither flag was caught above.
	Renderer *r = createOpenGLRenderer(system, width, height);
	if (!r)
		error("Colony: no renderer available (built without OpenGL support?)");
	return r;
}

} // End of namespace Colony
