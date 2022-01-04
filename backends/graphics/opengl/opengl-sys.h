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

#ifndef BACKENDS_GRAPHICS_OPENGL_OPENGL_SYS_H
#define BACKENDS_GRAPHICS_OPENGL_OPENGL_SYS_H

#include "common/scummsys.h"
#include "graphics/opengl/system_headers.h"

#include "backends/graphics/opengl/debug.h"

// This is an addition from us to alias ARB shader object extensions to
// OpenGL (ES) 2.0 style functions. It only works when GLhandleARB and GLuint
// are type compatible.
typedef GLuint GLprogram;
typedef GLuint GLshader;

namespace OpenGL {

enum ContextType {
	kContextGL,
	kContextGLES,
	kContextGLES2
};

class Pipeline;
class Framebuffer;

/**
 * Description structure of the OpenGL (ES) context.
 */
struct Context {
	/** The type of the active context. */
	ContextType type;

	/** Whether the context is initialized or not. */
	bool isInitialized;

	/**
	 * Reset context.
	 *
	 * This marks all extensions as unavailable and clears all function
	 * pointers.
	 */
	void reset();

	/** Helper function for checking the GL version supported by the context. */
	inline bool isGLVersionOrHigher(int major, int minor) {
		return ((majorVersion > major) || ((majorVersion == major) && (minorVersion >= minor)));
	}

	/** The GL version supported by the context. */
	int majorVersion, minorVersion;

	/** The maximum texture size supported by the context. */
	GLint maxTextureSize;

	/** Whether GL_ARB_texture_non_power_of_two is available or not. */
	bool NPOTSupported;

	/** Whether shader support is available or not. */
	bool shadersSupported;

	/** Whether multi texture support is available or not. */
	bool multitextureSupported;

	/** Whether FBO support is available or not. */
	bool framebufferObjectSupported;

	/** Whether packed pixels support is available or not. */
	bool packedPixelsSupported;

	/** Whether texture coordinate edge clamping is available or not. */
	bool textureEdgeClampSupported;

	//
	// Wrapper functionality to handle fixed-function pipelines and
	// programmable pipelines in the same fashion.
	//

private:
	/** Currently active rendering pipeline. */
	Pipeline *activePipeline;

public:
	/**
	 * Set new pipeline.
	 *
	 * Client is responsible for any memory management related to pipelines.
	 *
	 * @param pipeline Pipeline to activate.
	 * @return Formerly active pipeline.
	 */
	Pipeline *setPipeline(Pipeline *pipeline);

	/**
	 * Query the currently active rendering pipeline.
	 */
	Pipeline *getActivePipeline() const { return activePipeline; }
};

/**
 * The (active) OpenGL context.
 */
extern Context g_context;

} // End of namespace OpenGL

#define GL_CALL(x)                 GL_WRAP_DEBUG(x, x)
#define GL_CALL_SAFE(func, params) \
	do { \
		if (g_context.isInitialized) { \
			GL_CALL(func params); \
		} \
	} while (0)
#define GL_ASSIGN(var, x)          GL_WRAP_DEBUG(var = x, x)

#endif
