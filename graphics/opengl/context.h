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

#ifndef GRAPHICS_OPENGL_CONTEXT_H
#define GRAPHICS_OPENGL_CONTEXT_H

#include "common/singleton.h"

namespace OpenGL {

enum ContextOGLType {
	kOGLContextGL,
	kOGLContextGLES2
};

/**
 * Description structure of the OpenGL (ES) context.
 *
 * This class is based on LordHoto's OpenGL backend for ScummVM
 */
class ContextGL : public Common::Singleton<ContextGL> {
public:
	ContextGL();

	/**
	 * Initialize the context description from currently active context.
	 *
	 * The extensions and features are marked as available according
	 * to the current context capabilities.
	 */
	void initialize(ContextOGLType contextType);

	/**
	 * Reset context.
	 *
	 * This marks all extensions as unavailable.
	 */
	void reset();

	/** The type of the active context. */
	ContextOGLType type;

	/** The maximum texture size supported by the context. */
	int maxTextureSize;

	/** Whether GL_ARB_texture_non_power_of_two is available or not. */
	bool NPOTSupported;

	/** Whether shader support is available or not. */
	bool shadersSupported;

	/** Whether FBO support is available or not. */
	bool framebufferObjectSupported;

	/** Whether multisample FBO support is available or not */
	bool framebufferObjectMultisampleSupported;

	/**
	 * Contains the maximum number of supported multisample samples
	 * if multisample FBOs are supported. Contains -1 otherwise.
	 */
	int multisampleMaxSamples;

	/** Whether packing the depth and stencil buffers is possible or not. */
	bool packedDepthStencilSupported;

	/** Whether specifying a pitch when uploading to textures is available or not */
	bool unpackSubImageSupported;
	
	/** Whether depth component 24 is supported or not */
	bool OESDepth24;

	int getGLSLVersion() const;
};

} // End of namespace OpenGL

/** Shortcut for accessing the active OpenGL context. */
#define OpenGLContext OpenGL::ContextGL::instance()

#endif
