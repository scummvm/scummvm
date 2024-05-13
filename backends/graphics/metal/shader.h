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

#ifndef BACKENDS_GRAPHICS_METAL_SHADER_H
#define BACKENDS_GRAPHICS_METAL_SHADER_H

#include "common/singleton.h"

namespace MTL {
class Device;
class Function;
class Library;
}

namespace Metal {

class ShaderManager : public Common::Singleton<ShaderManager> {
public:
	enum ShaderUsage {
		/** Default shader implementing the Metal fixed-function pipeline. */
		kDefaultFragmentShader = 0,

		/** CLUT8 look up shader. */
		kCLUT8LookUpFragmentShader,

		/** Default vertex shader */
		kDefaultVertexShader,

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
	void notifyCreate(MTL::Device *device);

	/**
	 * Query a built-in shader.
	 * Shader returned must be destroyed by caller.
	 */
	MTL::Function *query(ShaderUsage shader) const;

private:
	friend class Common::Singleton<SingletonBaseType>;
	ShaderManager();
	~ShaderManager();

	MTL::Library *_shaderLibrary;
	MTL::Function *_defaultVertexShader;
	MTL::Function *_defaultFragmentShader;
	MTL::Function *_lookUpFragmentShader;
};

} // End of namespace Metal

/** Shortcut for accessing the font manager. */
#define ShaderMan (Metal::ShaderManager::instance())

#endif
