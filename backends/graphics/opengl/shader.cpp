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

#include "backends/graphics/opengl/shader.h"
#include "graphics/opengl/debug.h"

#if !USE_FORCED_GLES

namespace Common {
DECLARE_SINGLETON(OpenGL::ShaderManager);
}

namespace OpenGL {

namespace {

#pragma mark - Builtin Shader Sources -

const char *const g_defaultShaderAttributes[] = {
	"position", "texCoordIn", "blendColorIn", nullptr
};

const char *const g_defaultVertexShader =
	"attribute vec4 position;\n"
	"attribute vec2 texCoordIn;\n"
	"attribute vec4 blendColorIn;\n"
	"\n"
	"uniform mat4 projection;\n"
	"\n"
	"varying vec2 texCoord;\n"
	"varying vec4 blendColor;\n"
	"\n"
	"void main(void) {\n"
	"\ttexCoord    = texCoordIn;\n"
	"\tblendColor  = blendColorIn;\n"
	"\tgl_Position = projection * position;\n"
	"}\n";

const char *const g_defaultFragmentShader =
	"varying vec2 texCoord;\n"
	"varying vec4 blendColor;\n"
	"\n"
	"uniform sampler2D shaderTexture;\n"
	"\n"
	"void main(void) {\n"
	"\tgl_FragColor = blendColor * texture2D(shaderTexture, texCoord);\n"
	"}\n";

const char *const g_lookUpFragmentShader =
	"varying vec2 texCoord;\n"
	"varying vec4 blendColor;\n"
	"\n"
	"uniform sampler2D shaderTexture;\n"
	"uniform sampler2D palette;\n"
	"\n"
	"const float adjustFactor = 255.0 / 256.0 + 1.0 / (2.0 * 256.0);"
	"\n"
	"void main(void) {\n"
	"\tvec4 index = texture2D(shaderTexture, texCoord);\n"
	"\tgl_FragColor = blendColor * texture2D(palette, vec2(index.a * adjustFactor, 0.0));\n"
	"}\n";

} // End of anonymous namespace

ShaderManager::ShaderManager() {
	for (int i = 0; i < ARRAYSIZE(_builtIn); ++i) {
		_builtIn[i] = nullptr;
	}
}

ShaderManager::~ShaderManager() {
	for (int i = 0; i < ARRAYSIZE(_builtIn); ++i) {
		delete _builtIn[i];
	}
}

void ShaderManager::notifyDestroy() {
	for (int i = 0; i < ARRAYSIZE(_builtIn); ++i) {
		delete _builtIn[i];
		_builtIn[i] = nullptr;
	}
}

void ShaderManager::notifyCreate() {
	// Ensure everything is destroyed
	notifyDestroy();

	_builtIn[kDefault] = Shader::fromStrings("default", g_defaultVertexShader, g_defaultFragmentShader, g_defaultShaderAttributes, 110);
	_builtIn[kCLUT8LookUp] = Shader::fromStrings("clut8lookup", g_defaultVertexShader, g_lookUpFragmentShader, g_defaultShaderAttributes, 110);
	_builtIn[kCLUT8LookUp]->setUniform("palette", 1);

	for (uint i = 0; i < kMaxUsages; ++i) {
		_builtIn[i]->setUniform("shaderTexture", 0);
	}
}

Shader *ShaderManager::query(ShaderUsage shader) const {
	if (shader == kMaxUsages) {
		warning("OpenGL: ShaderManager::query used with kMaxUsages");
		return nullptr;
	}

	assert(_builtIn[shader]);
	return _builtIn[shader]->clone();
}

} // End of namespace OpenGL

#endif // !USE_FORCED_GLES
