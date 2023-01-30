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

#ifndef GRAPHICS_OPENGL_CONTEXT_H
#define GRAPHICS_OPENGL_CONTEXT_H

#include "common/singleton.h"

namespace OpenGL {

enum ContextType {
	kContextNone,
	kContextGL,
	kContextGLES,
	kContextGLES2
};

/**
 * Description structure of the OpenGL (ES) context.
 */
class Context : public Common::Singleton<Context> {
public:
	Context();

	/**
	 * Initialize the context description from currently active context.
	 *
	 * The extensions and features are marked as available according
	 * to the current context capabilities.
	 */
	void initialize(ContextType contextType);

	/**
	 * Reset context.
	 *
	 * This marks all extensions as unavailable.
	 */
	void reset();

	/** The type of the active context. */
	ContextType type;

	/** Helper function for checking the GL version supported by the context. */
	inline bool isGLVersionOrHigher(int major, int minor) const {
		return ((majorVersion > major) || ((majorVersion == major) && (minorVersion >= minor)));
	}

	/** The GL version supported by the context. */
	int majorVersion, minorVersion;

	/** The GLSL version supported by the context */
	int glslVersion;

	/** The maximum texture size supported by the context. */
	int maxTextureSize;

	/** Whether GL_ARB_texture_non_power_of_two is available or not. */
	bool NPOTSupported;

	/** Whether shader support is available or not. */
	bool shadersSupported;

	/** Whether shader support is good enough for engines or not. */
	bool enginesShadersSupported;

	/** Whether multi texture support is available or not. */
	bool multitextureSupported;

	/** Whether FBO support is available or not. */
	bool framebufferObjectSupported;

	/** Whether multisample FBO support is available or not */
	bool framebufferObjectMultisampleSupported;

	/**
	 * Contains the maximum number of supported multisample samples
	 * if multisample FBOs are supported. Contains -1 otherwise.
	 */
	int multisampleMaxSamples;

	/** Whether packed pixels support is available or not. */
	bool packedPixelsSupported;

	/** Whether packing the depth and stencil buffers is possible or not. */
	bool packedDepthStencilSupported;

	/** Whether specifying a pitch when uploading to textures is available or not */
	bool unpackSubImageSupported;

	/** Whether depth component 24 is supported or not */
	bool OESDepth24;

	/** Whether texture coordinate edge clamping is available or not. */
	bool textureEdgeClampSupported;

	/** Whether texture coordinate border clamping is available or not. */
	bool textureBorderClampSupported;

	/** Whether texture coordinate mirrored repeat is available or not. */
	bool textureMirrorRepeatSupported;

	/** Whether texture max level is available or not. */
	bool textureMaxLevelSupported;

private:
	/**
	 * Returns the native GLSL version supported by the driver.
	 * This does NOT take shaders ARB extensions into account.
	 */
	int getGLSLVersion() const;
};

} // End of namespace OpenGL

/** Shortcut for accessing the active OpenGL context. */
#define OpenGLContext OpenGL::Context::instance()

#endif
