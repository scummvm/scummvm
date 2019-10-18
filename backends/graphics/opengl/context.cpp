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
#include "backends/graphics/opengl/pipelines/pipeline.h"
#include "backends/graphics/opengl/framebuffer.h"

#include "common/tokenizer.h"
#include "common/debug.h"

namespace OpenGL {

void Context::reset() {
	maxTextureSize = 0;

	NPOTSupported = false;
	shadersSupported = false;
	multitextureSupported = false;
	framebufferObjectSupported = false;

#define GL_FUNC_DEF(ret, name, param) name = nullptr;
#include "backends/graphics/opengl/opengl-func.h"
#undef GL_FUNC_DEF

	activePipeline = nullptr;
}

Pipeline *Context::setPipeline(Pipeline *pipeline) {
	Pipeline *oldPipeline = activePipeline;
	if (oldPipeline) {
		oldPipeline->deactivate();
	}

	activePipeline = pipeline;
	if (activePipeline) {
		activePipeline->activate();
	}

	return oldPipeline;
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

#define LOAD_FUNC(name, loadName) { \
	void *fn = getProcAddress(#loadName); \
	memcpy(&g_context.name, &fn, sizeof(fn)); \
}

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
	GL_CALL(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_context.maxTextureSize));
	debug(5, "OpenGL maximum texture size: %d", g_context.maxTextureSize);

	const char *extString = (const char *)g_context.glGetString(GL_EXTENSIONS);
	debug(5, "OpenGL extensions: %s", extString);

	bool ARBShaderObjects = false;
	bool ARBShadingLanguage100 = false;
	bool ARBVertexShader = false;
	bool ARBFragmentShader = false;

	Common::StringTokenizer tokenizer(extString, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		if (token == "GL_ARB_texture_non_power_of_two" || token == "GL_OES_texture_npot") {
			g_context.NPOTSupported = true;
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
		} else if (token == "GL_EXT_framebuffer_object") {
			g_context.framebufferObjectSupported = true;
		}
	}

	if (g_context.type == kContextGLES2) {
		// GLES2 always has (limited) NPOT support.
		g_context.NPOTSupported = true;

		// GLES2 always has shader support.
		g_context.shadersSupported = true;

		// GLES2 always has multi texture support.
		g_context.multitextureSupported = true;

		// GLES2 always has FBO support.
		g_context.framebufferObjectSupported = true;
	} else {
		g_context.shadersSupported = ARBShaderObjects & ARBShadingLanguage100 & ARBVertexShader & ARBFragmentShader;
	}

	// Log context type.
	switch (g_context.type) {
	case kContextGL:
		debug(5, "OpenGL: GL context initialized");
		break;

	case kContextGLES:
		debug(5, "OpenGL: GLES context initialized");
		break;

	case kContextGLES2:
		debug(5, "OpenGL: GLES2 context initialized");
		break;

	default:
		warning("OpenGL: Unknown context initialized");
		break;
	}

	// Log features supported by GL context.
	debug(5, "OpenGL: NPOT texture support: %d", g_context.NPOTSupported);
	debug(5, "OpenGL: Shader support: %d", g_context.shadersSupported);
	debug(5, "OpenGL: Multitexture support: %d", g_context.multitextureSupported);
	debug(5, "OpenGL: FBO support: %d", g_context.framebufferObjectSupported);
}

} // End of namespace OpenGL
