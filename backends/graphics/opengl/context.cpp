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

#include "backends/graphics/opengl/opengl-sys.h"
#include "backends/graphics/opengl/opengl-graphics.h"

#include "common/tokenizer.h"

namespace OpenGL {

void Context::reset(bool full) {
	if (full) {
		// GLES supports least features, thus we initialize the context type
		// to this on full reset.
		type = kContextGLES;
	}

	NPOTSupported = false;
}

Context g_context;

void OpenGLGraphicsManager::setContextType(ContextType type) {
#if USE_FORCED_GL
	type = kContextGL;
#elif USE_FORCED_GLES
	type = kContextGLES;
#endif

	g_context.type = type;
}

void OpenGLGraphicsManager::initializeGLContext() {
	// Initialize default state.
	g_context.reset();

	// Load all functions.
	// We use horrible trickery to silence C++ compilers.
	// See backends/plugins/sdl/sdl-provider.cpp for more information.
	assert(sizeof(void (*)()) == sizeof(void *));
	void *fn = nullptr;
#define GL_EXT_FUNC_DEF(ret, name, param) \
	fn = getProcAddress(#name); \
	memcpy(&g_context.name, &fn, sizeof(fn))
#ifdef USE_BUILTIN_OPENGL
#define GL_FUNC_DEF(ret, name, param) g_context.name = &name
#else
#define GL_FUNC_DEF GL_EXT_FUNC_DEF
#endif
#include "backends/graphics/opengl/opengl-func.h"
#undef GL_EXT_FUNC_DEF
#undef GL_FUNC_DEF

	const char *extString = (const char *)g_context.glGetString(GL_EXTENSIONS);

	Common::StringTokenizer tokenizer(extString, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		if (token == "GL_ARB_texture_non_power_of_two") {
			g_context.NPOTSupported = true;
		}
	}
}

} // End of namespace OpenGL
