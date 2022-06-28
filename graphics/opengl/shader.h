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

#ifndef GRAPHICS_OPENGL_SHADER_H
#define GRAPHICS_OPENGL_SHADER_H

#include "common/file.h"
#include "common/array.h"
#include "common/ptr.h"

#include "math/matrix3.h"
#include "math/matrix4.h"
#include "math/vector2d.h"
#include "math/vector3d.h"
#include "math/vector4d.h"

#include "graphics/opengl/system_headers.h"

namespace OpenGL {

struct VertexAttrib {
	VertexAttrib(uint32 idx, const char *name) :
		_enabled(false), _idx(idx), _name(name), _vbo(0), _size(0),
		_type(GL_FLOAT), _normalized(false), _stride(0), _pointer(nullptr) {}
	bool _enabled;
	uint32 _idx;
	Common::String _name;
	GLuint _vbo;
	GLint _size;
	GLenum _type;
	bool _normalized;
	GLsizei _stride;
	const void *_pointer;
	float _const[4];
};

class Shader {
	typedef Common::HashMap<Common::String, GLint> UniformsMap;

public:
	~Shader();
	Shader* clone() {
		return new Shader(*this);
	}

	void use(bool forceReload = false);

	bool setUniform(const Common::String &uniform, const Math::Matrix4 &m) {
		GLint pos = getUniformLocation(uniform);
		if (pos != -1) {
			use();
			glUniformMatrix4fv(pos, 1, GL_FALSE, m.getData());
			return true;
		} else {
			return false;
		}
	}

	bool setUniform(const Common::String &uniform, const Math::Matrix3 &m) {
		GLint pos = getUniformLocation(uniform);
		if (pos != -1) {
			use();
			glUniformMatrix3fv(pos, 1, GL_FALSE, m.getData());
			return true;
		} else {
			return false;
		}
	}

	bool setUniform(const Common::String &uniform, const Math::Vector4d &v) {
		GLint pos = getUniformLocation(uniform);
		if (pos != -1) {
			use();
			glUniform4fv(pos, 1, v.getData());
			return true;
		} else {
			return false;
		}
	}

	bool setUniform(const Common::String &uniform, const Math::Vector3d &v) {
		GLint pos = getUniformLocation(uniform);
		if (pos != -1) {
			use();
			glUniform3fv(pos, 1, v.getData());
			return true;
		} else {
			return false;
		}
	}

	bool setUniform(const Common::String &uniform, const Math::Vector2d &v) {
		GLint pos = getUniformLocation(uniform);
		if (pos != -1) {
			use();
			glUniform2fv(pos, 1, v.getData());
			return true;
		} else {
			return false;
		}
	}

	bool setUniform(const Common::String &uniform, unsigned int x) {
		GLint pos = getUniformLocation(uniform);
		if (pos != -1) {
			use();
			glUniform1i(pos, x);
			return true;
		} else {
			return false;
		}
	}

	// Different name to avoid overload ambiguity
	bool setUniform1f(const Common::String &uniform, float f) {
		GLint pos = getUniformLocation(uniform);
		if (pos != -1) {
			use();
			glUniform1f(pos, f);
			return true;
		} else {
			return false;
		}
	}

	GLint getUniformLocation(const Common::String &uniform) const {
		UniformsMap::iterator kv = _uniforms->find(uniform);
		if (kv == _uniforms->end()) {
			GLint ret = glGetUniformLocation(*_shaderNo, uniform.c_str());
			_uniforms->setVal(uniform, ret);
			return ret;
		} else {
			return kv->_value;
		}
	}

	void enableVertexAttribute(const char *attrib, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
	void enableVertexAttribute(const char *attrib, GLuint vbo, GLint size, GLenum type, GLboolean normalized, GLsizei stride, uint32 offset);
	void disableVertexAttribute(const char *attrib, int size, const float *data);
	template <int r>
	void disableVertexAttribute(const char *attrib, const Math::Matrix<r,1> &m) {
		disableVertexAttribute(attrib, r, m.getData());
	}
	VertexAttrib & getAttributeAt(uint32 idx);
	VertexAttrib & getAttribute(const char *attrib);

	static GLuint createBuffer(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage = GL_STATIC_DRAW);
	static void freeBuffer(GLuint vbo);

	/**
	 * Creates a shader object from strings
	 *
	 * For shader files (used by games), we used to require GLSL 1.20, this is the default for compatGLSLVersion.
	 * The GLSL version is converted to GLSL ES version if needed.
	 *
	 * @param name The name of the shader for errors messages
	 * @param vertex The vertex shader code
	 * @param fragment The fragment shader code
	 * @param attributes The vertex attributes names for indexing
	 * @param compatGLSLVersion The GLSL version required: 0 for no preprocessing, 100 for GLSL 1.00 and so on
	 *
	 * @return the shader object created
	 */
	static Shader *fromFiles(const char *vertex, const char *fragment, const char *const *attributes, int compatGLSLVersion = 120);
	static Shader *fromFiles(const char *shared, const char *const *attributes, int compatGLSLVersion = 120) {
		return fromFiles(shared, shared, attributes, compatGLSLVersion);
	}

	/**
	 * Creates a shader object from strings
	 *
	 * Shader strings are usually included in backends and don't need preprocessing, this is the default for compatGLSLVersion.
	 * The GLSL version is converted to GLSL ES version if needed.
	 *
	 * @param name The name of the shader for errors messages
	 * @param vertex The vertex shader code
	 * @param fragment The fragment shader code
	 * @param attributes The vertex attributes names for indexing
	 * @param compatGLSLVersion The GLSL version required: 0 for no preprocessing, 100 for GLSL 1.00 and so on
	 *
	 * @return the shader object created
	 */
	static Shader *fromStrings(const Common::String &name, const char *vertex, const char *fragment, const char *const *attributes, int compatGLSLVersion = 0);

	void unbind();

private:
	Shader(const Common::String &name, GLuint vertexShader, GLuint fragmentShader, const char *const *attributes);

	// Since this class is cloned using the implicit copy constructor,
	// a reference counting pointer is used to ensure deletion of the OpenGL
	// program upon destruction of the last clone.
	Common::SharedPtr<GLuint> _shaderNo;

	Common::String _name;

	Common::Array<VertexAttrib> _attributes;
	Common::SharedPtr<UniformsMap> _uniforms;

	static Shader *_previousShader;
	static uint32 previousNumAttributes;
};

} // End of namespace OpenGL

#endif
