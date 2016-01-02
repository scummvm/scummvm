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
#include "common/debug.h"

namespace OpenGL {

#if USE_FORCED_GL
#define HAS_SHADERS_CHECK shadersSupported
#elif USE_FORCED_GLES
#define HAS_SHADERS_CHECK false
#elif USE_FORCED_GLES2
#define HAS_SHADERS_CHECK true
#else
#define HAS_SHADERS_CHECK (type == kContextGLES2 || shadersSupported)
#endif

void Context::reset() {
	_maxTextureSize = 0;

	NPOTSupported = false;
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	shadersSupported = false;
	multitextureSupported = false;
	framebufferObjectSupported = false;
	textureRGSupported = false;
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

	// Enable rendering with vertex and coord arrays.
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (HAS_SHADERS_CHECK) {
#endif
#if !USE_FORCED_GLES
		GL_CALL(glEnableVertexAttribArray(kPositionAttribLocation));
		GL_CALL(glEnableVertexAttribArray(kTexCoordAttribLocation));
#endif
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
		GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
		GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
#endif
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif

#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (g_context.type == kContextGLES2) {
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES
		GL_CALL(glActiveTexture(GL_TEXTURE0));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
		GL_CALL(glEnable(GL_TEXTURE_2D));
#endif
#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif
}

void Context::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (HAS_SHADERS_CHECK) {
#endif
#if !USE_FORCED_GLES
		GL_CALL(glVertexAttrib4f(kColorAttribLocation, r, g, b, a));
#endif
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
		GL_CALL(glColor4f(r, g, b, a));
#endif
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	}
#endif
}

void Context::setDrawCoordinates(const GLfloat *vertices, const GLfloat *texCoords) {
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	if (HAS_SHADERS_CHECK) {
#endif
#if !USE_FORCED_GLES
		GL_CALL(glVertexAttribPointer(kTexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, texCoords));
		GL_CALL(glVertexAttribPointer(kPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices));
#endif
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	} else {
#endif
#if !USE_FORCED_GLES2
		GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, vertices));
#endif
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
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

#define LOAD_FUNC(name, loadName) \
	fn = getProcAddress(#loadName); \
	memcpy(&g_context.name, &fn, sizeof(fn))

#define GL_EXT_FUNC_DEF(ret, name, param) LOAD_FUNC(name, name)

#ifdef USE_BUILTIN_OPENGL
#define GL_FUNC_DEF(ret, name, param) g_context.name = &name
#define GL_FUNC_2_DEF GL_FUNC_DEF
#else
#define GL_FUNC_DEF GL_EXT_FUNC_DEF
#define GL_FUNC_2_DEF(ret, name, extName, param) \
	if (g_context.type == kContextGL) { \
		LOAD_FUNC(name, extName); \
	} else { \
		LOAD_FUNC(name, name); \
	}
#endif
#include "backends/graphics/opengl/opengl-func.h"
#undef GL_FUNC_2_DEF
#undef GL_FUNC_DEF
#undef GL_EXT_FUNC_DEF
#undef LOAD_FUNC

	// Obtain maximum texture size.
	GL_CALL(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_context._maxTextureSize));
	debug(5, "OpenGL maximum texture size: %d", g_context._maxTextureSize);

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
		} else if (token == "GL_ARB_multitexture") {
			g_context.multitextureSupported = true;
		} else if (token == "GL_ARB_texture_rg") {
			g_context.textureRGSupported = true;
		} else if (token == "GL_EXT_framebuffer_object") {
			g_context.framebufferObjectSupported = true;
#endif
		}
	}

#if !USE_FORCED_GLES && !USE_FORCED_GLES2
	g_context.shadersSupported = ARBShaderObjects & ARBShadingLanguage100 & ARBVertexShader & ARBFragmentShader;
#endif
}

} // End of namespace OpenGL
