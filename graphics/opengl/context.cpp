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

#include "graphics/opengl/context.h"

#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/tokenizer.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

namespace Common {
DECLARE_SINGLETON(OpenGL::ContextGL);
}

namespace OpenGL {

ContextGL::ContextGL() {
	reset();
}

void ContextGL::reset() {
	maxTextureSize = 0;

	NPOTSupported = false;
	shadersSupported = false;
	framebufferObjectSupported = false;
	packedDepthStencilSupported = false;
	unpackSubImageSupported = false;
	framebufferObjectMultisampleSupported = false;
	OESDepth24 = false;
	multisampleMaxSamples = -1;
}

void ContextGL::initialize(ContextOGLType contextType) {
	// Initialize default state.
	reset();

	type = contextType;

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
		} else if (token == "GL_EXT_framebuffer_object") {
			framebufferObjectSupported = true;
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
		}
		
	}

	int glslVersion = getGLSLVersion();
	debug(5, "OpenGL GLSL version: %d", glslVersion);

	if (type == kOGLContextGLES2) {
		// GLES2 always has (limited) NPOT support.
		NPOTSupported = true;

		// GLES2 always has shader support.
		shadersSupported = true;

		// GLES2 always has FBO support.
		framebufferObjectSupported = true;

		// ScummVM does not support multisample FBOs with GLES2 for now
		framebufferObjectMultisampleSupported = false;
		multisampleMaxSamples = -1;
	} else {
		shadersSupported = ARBShaderObjects && ARBShadingLanguage100 && ARBVertexShader && ARBFragmentShader && glslVersion >= 120;

		// Desktop GL always has unpack sub-image support
		unpackSubImageSupported = true;

		framebufferObjectMultisampleSupported = EXTFramebufferMultisample && EXTFramebufferBlit;

		if (framebufferObjectMultisampleSupported) {
			glGetIntegerv(GL_MAX_SAMPLES, (GLint *)&multisampleMaxSamples);
		}
	}

	// Log context type.
	switch (type) {
		case kOGLContextGL:
			debug(5, "OpenGL: GL context initialized");
			break;

		case kOGLContextGLES2:
			debug(5, "OpenGL: GLES2 context initialized");
			break;
	}

	// Log features supported by GL context.
	debug(5, "OpenGL: NPOT texture support: %d", NPOTSupported);
	debug(5, "OpenGL: Shader support: %d", shadersSupported);
	debug(5, "OpenGL: FBO support: %d", framebufferObjectSupported);
	debug(5, "OpenGL: Packed depth stencil support: %d", packedDepthStencilSupported);
	debug(5, "OpenGL: Unpack subimage support: %d", unpackSubImageSupported);
}

int ContextGL::getGLSLVersion() const {
	const char *glslVersionString = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (!glslVersionString) {
		warning("Could not get GLSL version");
		return 0;
	}

	const char *glslVersionFormat;
	if (type == kOGLContextGL) {
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
}

} // End of namespace OpenGL

#endif
