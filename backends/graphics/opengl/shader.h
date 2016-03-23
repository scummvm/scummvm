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

#ifndef BACKENDS_GRAPHICS_OPENGL_SHADER_H
#define BACKENDS_GRAPHICS_OPENGL_SHADER_H

#include "backends/graphics/opengl/opengl-sys.h"

#if !USE_FORCED_GLES

#include "common/singleton.h"
#include "common/hash-str.h"
#include "common/ptr.h"

namespace OpenGL {

/**
 * A generic uniform value interface for a shader program.
 */
class ShaderUniformValue {
public:
	virtual ~ShaderUniformValue() {}

	/**
	 * Setup the the value to the given location.
	 *
	 * @param location Location of the uniform.
	 */
	virtual void set(GLint location) const = 0;
};

/**
 * Integer value for a shader uniform.
 */
class ShaderUniformInteger : public ShaderUniformValue {
public:
	ShaderUniformInteger(GLint value) : _value(value) {}

	virtual void set(GLint location) const override;

private:
	const GLint _value;
};

/**
 * Float value for a shader uniform.
 */
class ShaderUniformFloat : public ShaderUniformValue {
public:
	ShaderUniformFloat(GLfloat value) : _value(value) {}

	virtual void set(GLint location) const override;

private:
	const GLfloat _value;
};

/**
 * 4x4 Matrix value for a shader uniform.
 */
class ShaderUniformMatrix44 : public ShaderUniformValue {
public:
	ShaderUniformMatrix44(const GLfloat *mat44) {
		memcpy(_matrix, mat44, sizeof(_matrix));
	}

	virtual void set(GLint location) const override;

private:
	GLfloat _matrix[4*4];
};

class Shader {
public:
	Shader(const Common::String &vertex, const Common::String &fragment);
	~Shader();

	/**
	 * Destroy the shader program.
	 *
	 * This keeps the vertex and fragment shader sources around and thus
	 * allows for recreating the shader on context recreation. It also keeps
	 * the uniform state around.
	 */
	void destroy();

	/**
	 * Recreate shader program.
	 *
	 * @return true on success, false on failure.
	 */
	bool recreate();

	/**
	 * Make shader active.
	 */
	void activate();

	/**
	 * Make shader inactive.
	 */
	void deactivate();

	/**
	 * Return location for attribute with given name.
	 *
	 * @param name Name of the attribute to look up in the shader.
	 * @return The loctaion of -1 if attribute was not found.
	 */
	GLint getAttributeLocation(const char *name) const;
	GLint getAttributeLocation(const Common::String &name) const {
		return getAttributeLocation(name.c_str());
	}

	/**
	 * Return location for uniform with given name.
	 *
	 * @param name Name of the uniform to look up in the shader.
	 * @return The location or -1 if uniform was not found.
	 */
	GLint getUniformLocation(const char *name) const;
	GLint getUniformLocation(const Common::String &name) const {
		return getUniformLocation(name.c_str());
	}

	/**
	 * Bind value to uniform.
	 *
	 * @param name  The name of the uniform to be set.
	 * @param value The value to be set.
	 * @return 'false' on error (i.e. uniform unknown or otherwise),
	 *         'true' otherwise.
	 */
	bool setUniform(const Common::String &name, ShaderUniformValue *value);

	/**
	 * Bind integer value to uniform.
	 *
	 * @param name  The name of the uniform to be set.
	 * @param value The value to be set.
	 * @return 'false' on error (i.e. uniform unknown or otherwise),
	 *         'true' otherwise.
	 */
	bool setUniform1I(const Common::String &name, GLint value) {
		return setUniform(name, new ShaderUniformInteger(value));
	}
protected:
	/**
	 * Vertex shader sources.
	 */
	const Common::String _vertex;

	/**
	 * Fragment shader sources.
	 */
	const Common::String _fragment;

	/**
	 * Whether the shader is active or not.
	 */
	bool _isActive;

	/**
	 * Shader program handle.
	 */
	GLprogram _program;

	/**
	 * A uniform descriptor.
	 *
	 * This stores the state of a shader uniform. The state is made up of the
	 * uniform location, whether the state was altered since last set, and the
	 * value of the uniform.
	 */
	struct Uniform {
		Uniform() : location(-1), altered(false), value() {}
		Uniform(GLint loc, ShaderUniformValue *val)
		    : location(loc), altered(true), value(val) {}

		/**
		 * Write uniform value into currently active shader.
		 */
		void set() {
			if (altered && value) {
				value->set(location);
				altered = false;
			}
		}

		/**
		 * The location of the uniform or -1 in case it does not exist.
		 */
		GLint location;

		/**
		 * Whether the uniform state was aletered since last 'set'.
		 */
		bool altered;

		/**
		 * The value of the uniform.
		 */
		Common::SharedPtr<ShaderUniformValue> value;
	};

	typedef Common::HashMap<Common::String, Uniform> UniformMap;

	/**
	 * Map from uniform name to associated uniform description.
	 */
	UniformMap _uniforms;

	/**
	 * Compile a vertex or fragment shader.
	 *
	 * @param source     Sources to the shader.
	 * @param shaderType Type of shader to compile (GL_FRAGMENT_SHADER_ARB or
	 *                   GL_VERTEX_SHADER_ARB)
	 * @return The shader object or 0 on failure.
	 */
	static GLshader compileShader(const char *source, GLenum shaderType);
};

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
	 */
	Shader *query(ShaderUsage shader) const;

private:
	friend class Common::Singleton<SingletonBaseType>;
	ShaderManager();
	~ShaderManager();

	bool _initializeShaders;

	Shader *_builtIn[kMaxUsages];
};

} // End of namespace OpenGL

/** Shortcut for accessing the font manager. */
#define ShaderMan (OpenGL::ShaderManager::instance())

#endif // !USE_FORCED_GLES

#endif
