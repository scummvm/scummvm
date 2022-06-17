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

#define GLAD_GL_IMPLEMENTATION

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

	majorVersion = 0;
	minorVersion = 0;

	NPOTSupported = false;
	shadersSupported = false;
	multitextureSupported = false;
	framebufferObjectSupported = false;
	packedPixelsSupported = false;
	textureEdgeClampSupported = false;

	isInitialized = false;

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

#ifdef USE_GLAD
static GLADapiproc loadFunc(void *userptr, const char *name) {
	OpenGLGraphicsManager *openglGraphicsManager = (OpenGLGraphicsManager *)userptr;
	return (GLADapiproc)openglGraphicsManager->getProcAddress(name);
}
#endif

void OpenGLGraphicsManager::initializeGLContext() {
	// Initialize default state.
	g_context.reset();

#ifdef USE_GLAD
	switch (g_context.type) {
	case kContextGL:
		gladLoadGLUserPtr(loadFunc, this);
		break;

	case kContextGLES:
		gladLoadGLES1UserPtr(loadFunc, this);
		break;

	case kContextGLES2:
		gladLoadGLES2UserPtr(loadFunc, this);
		break;

	default:
		break;
	}
#endif

	g_context.isInitialized = true;

	// Obtain maximum texture size.
	GL_CALL(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_context.maxTextureSize));
	debug(5, "OpenGL maximum texture size: %d", g_context.maxTextureSize);

	const char *verString = (const char *)glGetString(GL_VERSION);
	debug(5, "OpenGL version: %s", verString);

	if (g_context.type == kContextGL) {
		// OpenGL version number is either of the form major.minor or major.minor.release,
		// where the numbers all have one or more digits
		if (sscanf(verString, "%d.%d", &g_context.majorVersion, &g_context.minorVersion) != 2) {
			g_context.majorVersion = g_context.minorVersion = 0;
			warning("Could not parse GL version '%s'", verString);
		}
	} else if (g_context.type == kContextGLES) {
		// The form of the string is "OpenGL ES-<profile> <major>.<minor>",
		// where <profile> is either "CM" (Common) or "CL" (Common-Lite),
		// and <major> and <minor> are integers.
		char profile[3];
		if (sscanf(verString, "OpenGL ES-%2s %d.%d", profile,
					&g_context.majorVersion, &g_context.minorVersion) != 3) {
			g_context.majorVersion = g_context.minorVersion = 0;
			warning("Could not parse GL ES version '%s'", verString);
		}
	} else if (g_context.type == kContextGLES2) {
		// The version is of the form
		// OpenGL<space>ES<space><version number><space><vendor-specific information>
		// version number format is not defined
		// There is only OpenGL ES 2.0 anyway
		if (sscanf(verString, "OpenGL ES %d.%d", &g_context.majorVersion, &g_context.minorVersion) != 2) {
			g_context.minorVersion = 0;
			if (sscanf(verString, "OpenGL ES %d ", &g_context.majorVersion) != 1) {
				g_context.majorVersion = 0;
				warning("Could not parse GL ES 2 version '%s'", verString);
			}
		}
	}

	const char *extString = (const char *)glGetString(GL_EXTENSIONS);
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
		} else if (token == "GL_ARB_framebuffer_object") {
			g_context.framebufferObjectSupported = true;
		} else if (token == "GL_EXT_packed_pixels" || token == "GL_APPLE_packed_pixels") {
			g_context.packedPixelsSupported = true;
		} else if (token == "GL_SGIS_texture_edge_clamp") {
			g_context.textureEdgeClampSupported = true;
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

	// OpenGL 1.2 and later always has packed pixels and texture edge clamp support
	if (g_context.type != kContextGL || g_context.isGLVersionOrHigher(1, 2)) {
		g_context.packedPixelsSupported = true;
		g_context.textureEdgeClampSupported = true;
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
#if !USE_FORCED_GLES
	if (g_context.shadersSupported)
		debug(5, "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
	debug(5, "OpenGL vendor: %s", glGetString(GL_VENDOR));
	debug(5, "OpenGL renderer: %s", glGetString(GL_RENDERER));
	debug(5, "OpenGL: NPOT texture support: %d", g_context.NPOTSupported);
	debug(5, "OpenGL: Shader support: %d", g_context.shadersSupported);
	debug(5, "OpenGL: Multitexture support: %d", g_context.multitextureSupported);
	debug(5, "OpenGL: FBO support: %d", g_context.framebufferObjectSupported);
	debug(5, "OpenGL: Packed pixels support: %d", g_context.packedPixelsSupported);
	debug(5, "OpenGL: Texture edge clamping support: %d", g_context.textureEdgeClampSupported);
}

} // End of namespace OpenGL
