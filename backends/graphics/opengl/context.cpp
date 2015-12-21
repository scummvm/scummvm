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
#include "backends/graphics/opengl/shader.h"

#include "common/tokenizer.h"

namespace OpenGL {

void Context::reset() {
	NPOTSupported = false;
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	shadersSupported = false;
#endif

#define GL_FUNC_DEF(ret, name, param) name = nullptr;
#include "backends/graphics/opengl/opengl-func.h"
#undef GL_FUNC_DEF
}

void Context::initializePipeline() {
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (g_context.type != kContextGLES2) {
#endif
#if !USE_FORCED_GLES2
		GL_CALL(glDisable(GL_LIGHTING));
		GL_CALL(glDisable(GL_FOG));
		GL_CALL(glShadeModel(GL_FLAT));
		GL_CALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif

#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (g_context.type == kContextGLES2) {
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES
		GL_CALL(glEnableVertexAttribArray(kPositionAttribLocation));
		GL_CALL(glEnableVertexAttribArray(kTexCoordAttribLocation));

		GL_CALL(glActiveTexture(GL_TEXTURE0));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
#if !USE_FORCED_GLES
		if (g_context.shadersSupported) {
			GL_CALL(glEnableVertexAttribArrayARB(kPositionAttribLocation));
			GL_CALL(glEnableVertexAttribArrayARB(kTexCoordAttribLocation));
		} else {
#endif
			// Enable rendering with vertex and coord arrays.
			GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
			GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
#if !USE_FORCED_GLES
		}
#endif

		GL_CALL(glEnable(GL_TEXTURE_2D));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif
}

void Context::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (g_context.type == kContextGLES2) {
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES
		GL_CALL(glVertexAttrib4f(kColorAttribLocation, r, g, b, a));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
#if !USE_FORCED_GLES
		if (g_context.shadersSupported) {
			GL_CALL(glVertexAttrib4fARB(kColorAttribLocation, r, g, b, a));
		} else {
#endif
			GL_CALL(glColor4f(r, g, b, a));
#if !USE_FORCED_GLES
		}
#endif
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif
}

void Context::setDrawCoordinates(const GLfloat *vertices, const GLfloat *texCoords) {
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (g_context.type == kContextGLES2) {
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES
		GL_CALL(glVertexAttribPointer(kTexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, texCoords));
		GL_CALL(glVertexAttribPointer(kPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
#if !USE_FORCED_GLES
		if (g_context.shadersSupported) {
			GL_CALL(glVertexAttribPointerARB(kTexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, texCoords));
			GL_CALL(glVertexAttribPointerARB(kPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices));
		} else {
#endif
			GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
			GL_CALL(glVertexPointer(2, GL_FLOAT, 0, vertices));
#if !USE_FORCED_GLES
		}
#endif
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif
}

Context g_context;

void OpenGLGraphicsManager::setContextType(ContextType type) {
#if USE_FORCED_GL
	type = kContextGL;
#elif USE_FORCED_GLES
	type = kContextGLES;
#elif USE_FORCED_GLES2
	type = kContextGLES2;
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

#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	bool ARBShaderObjects = false;
	bool ARBShadingLanguage100 = false;
	bool ARBVertexShader = false;
	bool ARBFragmentShader = false;
#endif

	Common::StringTokenizer tokenizer(extString, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		if (token == "GL_ARB_texture_non_power_of_two") {
			g_context.NPOTSupported = true;
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
		} else if (token == "GL_ARB_shader_objects") {
			ARBShaderObjects = true;
		} else if (token == "GL_ARB_shading_language_100") {
			ARBShadingLanguage100 = true;
		} else if (token == "GL_ARB_vertex_shader") {
			ARBVertexShader = true;
		} else if (token == "GL_ARB_fragment_shader") {
			ARBFragmentShader = true;
#endif
		}
	}

#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	g_context.shadersSupported = ARBShaderObjects & ARBShadingLanguage100 & ARBVertexShader & ARBFragmentShader;
#endif
}

} // End of namespace OpenGL
