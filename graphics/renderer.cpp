/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/renderer.h"

#include "common/translation.h"

namespace Graphics {

static const RendererTypeDescription rendererTypes[] = {
#if defined(USE_OPENGL_GAME)
	{ "opengl", _s("OpenGL"), kRendererTypeOpenGL },
#endif
#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	{ "opengl_shaders", _s("OpenGL with shaders"), kRendererTypeOpenGLShaders },
#endif
#ifdef USE_TINYGL
	{ "software", "Software", kRendererTypeTinyGL },
#endif
	{ 0, 0, kRendererTypeDefault }
};

const RendererTypeDescription *listRendererTypes() {
	return rendererTypes;
}

RendererType parseRendererTypeCode(const Common::String &code) {
	const RendererTypeDescription *rt = listRendererTypes();
	while (rt->code) {
		if (rt->code == code) {
			return rt->id;
		}
		rt++;
	}

	return kRendererTypeDefault;
}

Common::String getRendererTypeCode(RendererType type) {
	const RendererTypeDescription *rt = listRendererTypes();
	while (rt->code) {
		if (rt->id == type) {
			return rt->code;
		}
		rt++;
	}

	return "";
}

RendererType getBestMatchingAvailableRendererType(RendererType desired) {
	if (desired == kRendererTypeDefault) {
		desired = kRendererTypeOpenGL;
	}

#if !defined(USE_OPENGL_SHADERS) && !defined(USE_GLES2)
	if (desired == kRendererTypeOpenGLShaders) {
		desired = kRendererTypeOpenGL;
	}
#endif

#if (!defined(USE_OPENGL_GAME) && defined(USE_OPENGL_SHADERS)) || defined(USE_GLES2)
	if (desired == kRendererTypeOpenGL) {
		desired = kRendererTypeOpenGLShaders;
	}
#endif

#if !defined(USE_OPENGL_GAME) && !defined(USE_GLES2) && !defined(USE_OPENGL_SHADERS)
	if (desired == kRendererTypeOpenGL || desired == kRendererTypeOpenGLShaders) {
		desired = kRendererTypeTinyGL;
	}
#endif

	return desired;
}

} // End of namespace Graphics
