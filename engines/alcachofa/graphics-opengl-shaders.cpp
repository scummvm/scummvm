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

#include "alcachofa/alcachofa.h"
#include "alcachofa/graphics-opengl.h"
#include "alcachofa/detection.h"

#include "common/system.h"
#include "engines/util.h"
#include "graphics/opengl/shader.h"

using namespace Common;
using namespace Math;
using namespace Graphics;
using namespace OpenGL;

namespace Alcachofa {

class OpenGLRendererShaders : public OpenGLRenderer {
	struct Vertex {
		Vector2d pos;
		Vector2d uv;
		Color color;
	};

	struct VBO {
		VBO(GLuint bufferId) : _bufferId(bufferId) {}
		~VBO() {
			Shader::freeBuffer(_bufferId);
		}

		GLuint _bufferId;
		uint _capacity = 0;
	};
public:
	OpenGLRendererShaders(Point resolution)
		: OpenGLRenderer(resolution) {
		static constexpr const char *const kAttributes[] = {
			"in_pos",
			"in_uv",
			"in_color",
			nullptr
		};
		if (!_shader.loadFromStrings("alcachofa", kVertexShader, kFragmentShader, kAttributes))
			error("Could not load shader");

		// we use more than one VBO to reduce implicit synchronization
		for (int i = 0; i < 4; i++)
			_vbos.emplace_back(Shader::createBuffer(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW));

		_vertices.resize(8 * 6); // heuristic, we should be lucky if we can batch 8 quads together

		_whiteTexture.reset(new OpenGLTexture(1, 1, false));
		const byte whiteData[] = { 0xff, 0xff, 0xff, 0xff };
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteData));
	}

	void begin() override {
		resetState();
		_currentTexture = nullptr;
		_needsNewBatch = true;
	}

	void end() override {
		if (!_vertices.empty()) // submit last batch
			checkFirstDrawCommand();
		OpenGLRenderer::end();
	}

	void setTexture(ITexture *texture) override {
		if (texture == _currentTexture)
			return;
		_needsNewBatch = true;

		if (texture == nullptr)
			_currentTexture = nullptr;
		else {
			_currentTexture = dynamic_cast<OpenGLTexture *>(texture);
			assert(_currentTexture != nullptr);
		}
	}

	void setBlendMode(BlendMode blendMode) override {
		if (blendMode == _currentBlendMode)
			return;
		_needsNewBatch = true;
		_currentBlendMode = blendMode;
	}

	void setLodBias(float lodBias) override {
		if (abs(_currentLodBias - lodBias) < epsilon)
			return;
		_needsNewBatch = true;
		_currentLodBias = lodBias;
	}

	void quad(
		Vector2d topLeft,
		Vector2d size,
		Color color,
		Angle rotation,
		Vector2d texMin,
		Vector2d texMax) override {
		if (_needsNewBatch) {
			_needsNewBatch = false;
			checkFirstDrawCommand();
		}

		if (_currentTexture != nullptr) {
			// float equality is fine here, if it was calculated it was not a normal graphic
			_currentTexture->setMirrorWrap(texMin != Vector2d() || texMax != Vector2d(1, 1));
		}

		Vector2d positions[4], texCoords[4];
		getQuadPositions(topLeft, size, rotation, positions);
		getQuadTexCoords(texMin, texMax, texCoords);
		_vertices.push_back({ positions[0], texCoords[0], color });
		_vertices.push_back({ positions[1], texCoords[1], color });
		_vertices.push_back({ positions[2], texCoords[2], color });
		_vertices.push_back({ positions[0], texCoords[0], color });
		_vertices.push_back({ positions[2], texCoords[2], color });
		_vertices.push_back({ positions[3], texCoords[3], color });
	}

	void setMatrices(bool flipped) override {
		// adapted from https://en.wikipedia.org/wiki/Orthographic_projection
		const float left = 0.0f;
		const float right = _resolution.x;
		const float bottom = flipped ? _resolution.y : 0.0f;
		const float top = flipped ? 0.0f : _resolution.y;
		const float near = -1.0f;
		const float far = 1.0f;

		_projection.setToIdentity();
		_projection(0, 0) = 2.0f / (right - left);
		_projection(1, 1) = 2.0f / (top - bottom);
		_projection(2, 2) = -2.0f / (far - near);
		_projection(3, 0) = -(right + left) / (right - left);
		_projection(3, 1) = -(top + bottom) / (top - bottom);
		_projection(3, 2) = -(far + near) / (far - near);
	}

private:
	void checkFirstDrawCommand() {
		OpenGLRenderer::checkFirstDrawCommand();

		// submit batch
		if (!_vertices.empty()) {
			auto &vbo = _vbos[_curVBO];
			_curVBO = (_curVBO + 1) % _vbos.size();

			_shader.enableVertexAttribute("in_pos", vbo._bufferId, 2, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, pos));
			_shader.enableVertexAttribute("in_uv", vbo._bufferId, 2, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, uv));
			_shader.enableVertexAttribute("in_color", vbo._bufferId, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), offsetof(Vertex, color));
			_shader.use(true);

			GL_CALL(glBindTexture(GL_TEXTURE_2D, _batchTexture == nullptr
				? _whiteTexture->handle()
				: _batchTexture->handle()));
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo._bufferId));
			if (vbo._capacity < _vertices.size()) {
				vbo._capacity = _vertices.size();
				glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_STREAM_DRAW);
			} else
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * _vertices.size(), _vertices.data());
			glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
			_vertices.clear();
		}

		// setup next batch
		setBlendFunc(_currentBlendMode);
		_shader.setUniform("projection", _projection);
		_shader.setUniform("blendMode", _currentTexture == nullptr ? 5 : (int)_currentBlendMode);
		_shader.setUniform("posterize", g_engine->config().bits32() ? 0 : 1);
		_shader.setUniform1f("lodBias", _currentLodBias);
		_shader.setUniform("texture", 0);
		_batchTexture = _currentTexture;
	}

	Matrix4 _projection;
	Shader _shader;
	Array<VBO> _vbos;
	Array<Vertex> _vertices;
	uint _curVBO = 0;
	bool _needsNewBatch = false;
	OpenGLTexture *_batchTexture = nullptr;
	ScopedPtr<OpenGLTexture> _whiteTexture;

	static constexpr const char *const kVertexShader = R"(
		uniform mat4 projection;

		attribute vec2 in_pos;
		attribute vec2 in_uv;
		attribute vec4 in_color;

		varying vec2 var_uv;
		varying vec4 var_color;

		void main() {
			gl_Position = projection * vec4(in_pos, 0.0, 1.0);
			var_uv = in_uv;
			var_color = in_color;
		})";

	static constexpr const char *const kFragmentShader = R"(
		#ifdef GL_ES
			precision mediump float;
		#endif

		uniform sampler2D texture;
		uniform int blendMode;
		uniform int posterize;
		uniform float lodBias;

		varying vec2 var_uv;
		varying vec4 var_color;

		void main() {
			vec4 tex_color = texture2D(texture, var_uv, lodBias);
			if (blendMode <= 2) { // AdditiveAlpha, Additive and Multiply
				gl_FragColor.rgb = tex_color.rgb * var_color.a;
				gl_FragColor.a = tex_color.a;
			} else if (blendMode == 3) { // Alpha
				gl_FragColor.rgb = tex_color.rgb;
				gl_FragColor.a = var_color.a;
			} else if (blendMode == 4) { // Tinted
				gl_FragColor.rgb = var_color.rgb * var_color.a * tex_color.rgb;
				gl_FragColor.a = tex_color.a;
			} else { // Disabled texture
				gl_FragColor = var_color;
			}

			if (posterize == 1) {
				// shave off 3 bits for that 16-bit look
				gl_FragColor = floor(gl_FragColor * (256.0 / 8.0)) / (256.0 / 8.0);
			}
		})";
};

IRenderer *IRenderer::createOpenGLRendererShaders(Point resolution) {
	debug("Use OpenGL shaders renderer");
	return new OpenGLRendererShaders(resolution);
}

}
