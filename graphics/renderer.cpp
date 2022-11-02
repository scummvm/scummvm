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

#include "graphics/renderer.h"

#include "common/translation.h"
#include "common/system.h"

#include "graphics/opengl/context.h"

namespace Graphics {

static const RendererTypeDescription rendererTypes[] = {
#if defined(USE_OPENGL_GAME)
	{ "opengl", _s("OpenGL"), kRendererTypeOpenGL },
#endif
#if defined(USE_OPENGL_SHADERS)
	{ "opengl_shaders", _s("OpenGL with shaders"), kRendererTypeOpenGLShaders },
#endif
#ifdef USE_TINYGL
	{ "software", "Software", kRendererTypeTinyGL },
#endif
	{ 0, 0, kRendererTypeDefault }
};

DECLARE_TRANSLATION_ADDITIONAL_CONTEXT("OpenGL with shaders", "lowres")

Common::Array<RendererTypeDescription> Renderer::listTypes() {
	uint32 available = getAvailableTypes();
	Common::Array<RendererTypeDescription> ret;
	for (const RendererTypeDescription *rt = rendererTypes; rt->code; ++rt) {
		if (available & rt->id) {
			ret.push_back(*rt);
		}
	}
	return ret;
}

RendererType Renderer::parseTypeCode(const Common::String &code) {
	const RendererTypeDescription *rt = rendererTypes;
	while (rt->code) {
		if (rt->code == code) {
			return rt->id;
		}
		rt++;
	}

	return kRendererTypeDefault;
}

Common::String Renderer::getTypeCode(RendererType type) {
	const RendererTypeDescription *rt = rendererTypes;
	while (rt->code) {
		if (rt->id == type) {
			return rt->code;
		}
		rt++;
	}

	return "";
}

uint32 Renderer::getAvailableTypes() {
	uint32 available = 0;

#if defined(USE_TINYGL)
	/* TinyGL doesn't depend on hardware support */
	available |= kRendererTypeTinyGL;
#endif

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	bool backendCapableOpenGL = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);

	if (backendCapableOpenGL) {
		/* Backend either support OpenGL or OpenGL ES(2) */
#if defined(USE_OPENGL_GAME)
		/* OpenGL classic is compiled in, check if hardware supports it */
		if (g_system->getOpenGLType() == OpenGL::kContextGL) {
			available |= kRendererTypeOpenGL;
		}
#endif
#if defined(USE_OPENGL_SHADERS)
		/* OpenGL with shaders is compiled in, check if hardware supports it */
		if (g_system->hasFeature(OSystem::kFeatureShadersForGame)) {
			available |= kRendererTypeOpenGLShaders;
		}
#endif
	}
#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS
	return available;
}

RendererType Renderer::getBestMatchingType(RendererType desired, uint32 available) {
	/* What we want is possible */
	if (available & desired) {
		return desired;
	}

	/* We apply the same logic when nothing is desired and when what we want is not possible */
	if (desired != kRendererTypeDefault) {
		warning("Unable to create a '%s' renderer", getTypeCode(desired).c_str());
	}

	/* Shaders are the best experience */
	if (available & kRendererTypeOpenGLShaders) {
		return kRendererTypeOpenGLShaders;
	}
	/* then OpenGL */
	if (available & kRendererTypeOpenGL) {
		return kRendererTypeOpenGL;
	}
	/* then TinyGL */
	if (available & kRendererTypeTinyGL) {
		return kRendererTypeTinyGL;
	}

	/* Failure is not an option */
	error("Unable to create a renderer");
}

} // End of namespace Graphics
