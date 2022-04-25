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

#include "graphics/opengl/context.h"

#include "common/debug.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/tokenizer.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL)

#ifdef USE_GLAD
static GLADapiproc loadFunc(const char *name) {
	return (GLADapiproc)g_system->getOpenGLProcAddress(name);
}
#endif

namespace Common {
DECLARE_SINGLETON(OpenGL::Context);
}

namespace OpenGL {

Context::Context() {
	reset();
}

void Context::reset() {
	type = kContextNone;
	maxTextureSize = 0;

	majorVersion = 0;
	minorVersion = 0;
	glslVersion = 0;

	NPOTSupported = false;
	shadersSupported = false;
	enginesShadersSupported = false;
	multitextureSupported = false;
	framebufferObjectSupported = false;
	framebufferObjectMultisampleSupported = false;
	multisampleMaxSamples = -1;
	packedPixelsSupported = false;
	packedDepthStencilSupported = false;
	unpackSubImageSupported = false;
	OESDepth24 = false;
	textureEdgeClampSupported = false;
}

void Context::initialize(ContextType contextType) {
	// Initialize default state.
	reset();
	if (contextType == kContextNone) {
		return;
	}

	type = contextType;

#ifdef USE_GLAD
	switch (type) {
	case kContextGL:
		gladLoadGL(loadFunc);
		break;

	case kContextGLES:
		gladLoadGLES1(loadFunc);
		break;

	case kContextGLES2:
		gladLoadGLES2(loadFunc);
		break;

	default:
		break;
	}
#endif

	const char *verString = (const char *)glGetString(GL_VERSION);
	debug(5, "OpenGL version: %s", verString);

	if (type == kContextGL) {
		// OpenGL version number is either of the form major.minor or major.minor.release,
		// where the numbers all have one or more digits
		if (sscanf(verString, "%d.%d", &majorVersion, &minorVersion) != 2) {
			majorVersion = minorVersion = 0;
			warning("Could not parse GL version '%s'", verString);
		}
	} else if (type == kContextGLES) {
		// The form of the string is "OpenGL ES-<profile> <major>.<minor>",
		// where <profile> is either "CM" (Common) or "CL" (Common-Lite),
		// and <major> and <minor> are integers.
		char profile[3];
		if (sscanf(verString, "OpenGL ES-%2s %d.%d", profile,
					&majorVersion, &minorVersion) != 3) {
			majorVersion = minorVersion = 0;
			warning("Could not parse GL ES version '%s'", verString);
		}
	} else if (type == kContextGLES2) {
		// The version is of the form
		// OpenGL<space>ES<space><version number><space><vendor-specific information>
		// version number format is not defined
		// There is only OpenGL ES 2.0 anyway
		if (sscanf(verString, "OpenGL ES %d.%d", &majorVersion, &minorVersion) != 2) {
			minorVersion = 0;
			if (sscanf(verString, "OpenGL ES %d ", &majorVersion) != 1) {
				majorVersion = 0;
				warning("Could not parse GL ES 2 version '%s'", verString);
			}
		}
	}

	glslVersion = getGLSLVersion();

	// Obtain maximum texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&maxTextureSize);
	debug(5, "OpenGL maximum texture size: %d", maxTextureSize);

	const char *extString = (const char *)glGetString(GL_EXTENSIONS);

	bool ARBShaderObjects = false;
	bool ARBShadingLanguage100 = false;
	bool ARBVertexShader = false;
	bool ARBFragmentShader = false;
	bool EXTFramebufferMultisample = false;
	bool EXTFramebufferBlit = false;

	Common::StringTokenizer tokenizer(extString, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		if (token == "GL_ARB_texture_non_power_of_two" || token == "GL_OES_texture_npot") {
			NPOTSupported = true;
		} else if (token == "GL_ARB_shader_objects") {
			ARBShaderObjects = true;
		} else if (token == "GL_ARB_shading_language_100") {
			ARBShadingLanguage100 = true;
		} else if (token == "GL_ARB_vertex_shader") {
			ARBVertexShader = true;
		} else if (token == "GL_ARB_fragment_shader") {
			ARBFragmentShader = true;
		} else if (token == "GL_ARB_multitexture") {
			multitextureSupported = true;
		} else if (token == "GL_ARB_framebuffer_object") {
			framebufferObjectSupported = true;
		} else if (token == "GL_EXT_packed_pixels" || token == "GL_APPLE_packed_pixels") {
			packedPixelsSupported = true;
		} else if (token == "GL_EXT_packed_depth_stencil" || token == "GL_OES_packed_depth_stencil") {
			packedDepthStencilSupported = true;
		} else if (token == "GL_EXT_unpack_subimage") {
			unpackSubImageSupported = true;
		} else if (token == "GL_EXT_framebuffer_multisample") {
			EXTFramebufferMultisample = true;
		} else if (token == "GL_EXT_framebuffer_blit") {
			EXTFramebufferBlit = true;
		} else if (token == "GL_OES_depth24") {
			OESDepth24 = true;
		} else if (token == "GL_SGIS_texture_edge_clamp") {
			textureEdgeClampSupported = true;
		}
	}

	if (type == kContextGLES2) {
		// GLES2 always has (limited) NPOT support.
		NPOTSupported = true;

		// GLES2 always has shader support.
		shadersSupported = true;
		// GLES2 should always have GLSL ES 1.00 support but let's make sure
		enginesShadersSupported = (glslVersion >= 100);

		// GLES2 always has multi texture support.
		multitextureSupported = true;

		// GLES2 always has FBO support.
		framebufferObjectSupported = true;

		// ScummVM does not support multisample FBOs with GLES2 for now
		framebufferObjectMultisampleSupported = false;
		multisampleMaxSamples = -1;

		packedPixelsSupported = true;
		textureEdgeClampSupported = true;
		debug(5, "OpenGL: GLES2 context initialized");
	} else if (type == kContextGLES) {
		// GLES doesn't support shaders natively
		// We don't do any aliasing in our code and expect standard OpenGL functions but GLAD does it
		// So if we use GLAD we can check for ARB extensions and expect a GLSL of 1.00
#ifdef USE_GLAD
		shadersSupported = ARBShaderObjects && ARBShadingLanguage100 && ARBVertexShader && ARBFragmentShader;
		glslVersion = 100;
#endif
		// We don't expect GLES to support shaders recent enough for engines

		// ScummVM does not support multisample FBOs with GLES for now
		framebufferObjectMultisampleSupported = false;
		multisampleMaxSamples = -1;

		packedPixelsSupported = true;
		textureEdgeClampSupported = true;
		debug(5, "OpenGL: GLES context initialized");
	} else if (type == kContextGL) {
		shadersSupported = glslVersion >= 100;

		// We don't do any aliasing in our code and expect standard OpenGL functions but GLAD does it
		// So if we use GLAD we can check for ARB extensions and expect a GLSL of 1.00
#ifdef USE_GLAD
		if (!shadersSupported) {
			shadersSupported = ARBShaderObjects && ARBShadingLanguage100 && ARBVertexShader && ARBFragmentShader;
			if (shadersSupported) {
				glslVersion = 100;
			}
		}
#endif
		// In GL mode engines need GLSL 1.20
		enginesShadersSupported = glslVersion >= 120;

		// Desktop GL always has unpack sub-image support
		unpackSubImageSupported = true;

		framebufferObjectMultisampleSupported = EXTFramebufferMultisample && EXTFramebufferBlit;

		if (framebufferObjectMultisampleSupported) {
			glGetIntegerv(GL_MAX_SAMPLES, (GLint *)&multisampleMaxSamples);
		}

		// OpenGL 1.2 and later always has packed pixels and texture edge clamp support
		if (isGLVersionOrHigher(1, 2)) {
			packedPixelsSupported = true;
			textureEdgeClampSupported = true;
		}
		debug(5, "OpenGL: GL context initialized");
	} else {
		warning("OpenGL: Unknown context initialized");
	}

	// Log features supported by GL context.
	debug(5, "OpenGL vendor: %s", glGetString(GL_VENDOR));
	debug(5, "OpenGL renderer: %s", glGetString(GL_RENDERER));
	debug(5, "OpenGL: version %d.%d", majorVersion, minorVersion);
	debug(5, "OpenGL: GLSL version: %d", glslVersion);
	debug(5, "OpenGL: Max texture size: %d", maxTextureSize);
	debug(5, "OpenGL: NPOT texture support: %d", NPOTSupported);
	debug(5, "OpenGL: Shader support: %d", shadersSupported);
	debug(5, "OpenGL: Multitexture support: %d", multitextureSupported);
	debug(5, "OpenGL: FBO support: %d", framebufferObjectSupported);
	debug(5, "OpenGL: Multisample FBO support: %d", framebufferObjectMultisampleSupported);
	debug(5, "OpenGL: Multisample max number: %d", multisampleMaxSamples);
	debug(5, "OpenGL: Packed pixels support: %d", packedPixelsSupported);
	debug(5, "OpenGL: Packed depth stencil support: %d", packedDepthStencilSupported);
	debug(5, "OpenGL: Unpack subimage support: %d", unpackSubImageSupported);
	debug(5, "OpenGL: OpenGL ES depth 24 support: %d", OESDepth24);
	debug(5, "OpenGL: Texture edge clamping support: %d", textureEdgeClampSupported);
}

int Context::getGLSLVersion() const {
#if USE_FORCED_GLES
	return 0;
#else
	// No shader support in GLES
	if (type == kContextGLES) {
		return 0;
	}

	// No shader support in OpenGL 1.x
	if (type == kContextGL && !isGLVersionOrHigher(2, 0)) {
		return 0;
	}

	const char *glslVersionString = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (!glslVersionString) {
		warning("Could not get GLSL version");
		return 0;
	}

	const char *glslVersionFormat;
	if (type == kContextGL) {
		glslVersionFormat = "%d.%d";
	} else {
		glslVersionFormat = "OpenGL ES GLSL ES %d.%d";
	}

	int glslMajorVersion, glslMinorVersion;
	if (sscanf(glslVersionString, glslVersionFormat, &glslMajorVersion, &glslMinorVersion) != 2) {
		warning("Could not parse GLSL version '%s'", glslVersionString);
		return 0;
	}

	return glslMajorVersion * 100 + glslMinorVersion;
#endif
}

} // End of namespace OpenGL

#endif
