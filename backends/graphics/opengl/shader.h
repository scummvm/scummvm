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

#ifndef BACKENDS_GRAPHICS_OPENGL_SHADER_H
#define BACKENDS_GRAPHICS_OPENGL_SHADER_H

#include "graphics/opengl/system_headers.h"

#if !USE_FORCED_GLES

#include "common/singleton.h"

#include "graphics/opengl/shader.h"

namespace OpenGL {

class ShaderManager : public Common::Singleton<ShaderManager> {
public:
	enum ShaderUsage {
		/** Default shader implementing the GL fixed-function pipeline. */
		kDefault = 0,

		/** CLUT8 look up shader. */
		kCLUT8LookUp,

		/** Number of built-in shaders. Should not be used for query. */
		kMaxUsages
	};

	/**
	 * Notify shader manager about context destruction.
	 */
	void notifyDestroy();

	/**
	 * Notify shader manager about context creation.
	 */
	void notifyCreate();

	/**
	 * Query a built-in shader.
	 * Shader returned must be destroyed by caller.
	 */
	Shader *query(ShaderUsage shader) const;

private:
	friend class Common::Singleton<SingletonBaseType>;
	ShaderManager();
	~ShaderManager();

	Shader *_builtIn[kMaxUsages];
};

} // End of namespace OpenGL

/** Shortcut for accessing the font manager. */
#define ShaderMan (OpenGL::ShaderManager::instance())

#endif // !USE_FORCED_GLES

#endif
