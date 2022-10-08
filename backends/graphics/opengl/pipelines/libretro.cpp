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

#include "graphics/opengl/system_headers.h"

#if !USE_FORCED_GLES
#include "backends/graphics/opengl/pipelines/libretro.h"
#include "backends/graphics/opengl/pipelines/libretro/parser.h"
#include "backends/graphics/opengl/shader.h"
#include "backends/graphics/opengl/framebuffer.h"
#include "graphics/opengl/debug.h"

#include "common/textconsole.h"
#include "common/tokenizer.h"
#include "common/stream.h"

#include "graphics/surface.h"

#include "image/bmp.h"
#include "image/png.h"
#include "image/tga.h"

namespace OpenGL {

template<typename DecoderType>
static Graphics::Surface *loadViaImageDecoder(const Common::FSNode &fileNode) {
	Common::SeekableReadStream *stream = fileNode.createReadStream();
	if (!stream) {
		return nullptr;
	}

	DecoderType decoder;
	const bool success = decoder.loadStream(*stream);
	delete stream;
	stream = nullptr;

	if (!success) {
		return nullptr;
	}

	return decoder.getSurface()->convertTo(
#ifdef SCUMM_LITTLE_ENDIAN
										   Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24),
#else
										   Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
#endif
										   decoder.getPalette());
}

struct ImageLoader {
	const char *extension;
	Graphics::Surface *(*load)(const Common::FSNode &fileNode);
};

static const ImageLoader s_imageLoaders[] = {
	{ "bmp", loadViaImageDecoder<Image::BitmapDecoder> },
	{ "png", loadViaImageDecoder<Image::PNGDecoder> },
	{ "tga", loadViaImageDecoder<Image::TGADecoder> },
	{ nullptr, nullptr }
};

const char *const g_libretroShaderAttributes[] = {
	"VertexCoord", nullptr
};

LibRetroPipeline::LibRetroPipeline()
	: ShaderPipeline(ShaderMan.query(ShaderManager::kDefault)),
	  _shaderPreset(nullptr), _applyProjectionChanges(false),
	  _inputWidth(0), _inputHeight(0), _outputWidth(0), _outputHeight(0) {
}

LibRetroPipeline::LibRetroPipeline(const Common::FSNode &shaderPreset)
	: ShaderPipeline(ShaderMan.query(ShaderManager::kDefault)),
	  _shaderPreset(nullptr), _applyProjectionChanges(false),
	  _inputWidth(0), _inputHeight(0), _outputWidth(0), _outputHeight(0) {
	open(shaderPreset);
}

LibRetroPipeline::~LibRetroPipeline() {
	close();
}

void LibRetroPipeline::drawTexture(const GLTexture &texture, const GLfloat *coordinates, const GLfloat *texcoords) {
	Framebuffer *const targetBuffer = _activeFramebuffer;

	// Set input texture for 1st pass to texture to draw.
	_passes[0].inputTexture = &texture;

	// XXX:
	setOutputSize(coordinates[6], coordinates[7]);

	// In case texture dimensions or viewport dimensions changed, we need to
	// update the pipeline's state.
	if (   texture.getLogicalWidth() != _inputWidth
		|| texture.getLogicalHeight() != _inputHeight
		|| _outputSizeChanged) {
		_outputSizeChanged = false;
		_inputWidth  = texture.getLogicalWidth();
		_inputHeight = texture.getLogicalHeight();

		setPipelineState();
	}

	// Now we can actually draw the texture with the setup passes.
	for (PassArray::const_iterator i = _passes.begin(), end = _passes.end(); i != end; ++i) {
		renderPass(*i);
	}

	// Finally, we need to render the result to the active framebuffer.
	_applyProjectionChanges = true;
	setFramebuffer(targetBuffer);
	_applyProjectionChanges = false;

	ShaderPipeline::activateInternal();
	ShaderPipeline::drawTexture(*_passes[_passes.size() - 1].target->getTexture(), coordinates, texcoords);
	ShaderPipeline::deactivateInternal();
}

void LibRetroPipeline::setProjectionMatrix(const GLfloat *projectionMatrix) {
	if (_applyProjectionChanges) {
		ShaderPipeline::setProjectionMatrix(projectionMatrix);
	}
}

void LibRetroPipeline::activateInternal() {
}

void LibRetroPipeline::deactivateInternal() {
}

void LibRetroPipeline::setOutputSize(uint outputWidth, uint outputHeight) {
	_outputSizeChanged = (_outputWidth != outputWidth || _outputHeight != outputHeight);

	// Save output dimensions.
	_outputWidth  = outputWidth;
	_outputHeight = outputHeight;
}

bool LibRetroPipeline::open(const Common::FSNode &shaderPreset) {
	_shaderPreset = LibRetro::parsePreset(shaderPreset);
	if (!_shaderPreset)
		return false;

	if (!loadTextures()) {
		close();
		return false;
	}

	if (!loadPasses()) {
		close();
		return false;
	}

	return true;
}

void LibRetroPipeline::close() {
	for (TextureArray::size_type i = 0; i < _textures.size(); ++i) {
		if (_textures[i].textureData) {
			_textures[i].textureData->free();
		}
		delete _textures[i].textureData;
		delete _textures[i].glTexture;
	}
	_textures.clear();

	for (PassArray::size_type i = 0; i < _passes.size(); ++i) {
		delete _passes[i].shader;
		delete _passes[i].target;
	}
	_passes.clear();

	delete _shaderPreset;
	_shaderPreset = nullptr;
}

static Common::FSNode getChildRecursive(const Common::FSNode &basePath, const Common::String &fileName) {
	Common::FSNode finalPath = basePath;
	Common::StringTokenizer tok(fileName, "\\/");
	while (!tok.empty()) {
		finalPath = finalPath.getChild(tok.nextToken());
	}
	return finalPath;
}

bool LibRetroPipeline::loadTextures() {
	for (LibRetro::ShaderPreset::TextureArray::const_iterator
		 i = _shaderPreset->textures.begin(), end = _shaderPreset->textures.end();
		 i != end; ++i) {
		Texture texture = loadTexture(getChildRecursive( _shaderPreset->basePath, i->fileName));
		texture.id = i->id;

		if (!texture.textureData || !texture.glTexture) {
			return false;
		}

		texture.glTexture->enableLinearFiltering(i->filteringMode == LibRetro::kFilteringModeLinear);
		_textures.push_back(texture);
	}
	return true;
}

bool LibRetroPipeline::loadPasses() {
	for (LibRetro::ShaderPreset::PassArray::const_iterator
		 i = _shaderPreset->passes.begin(), end = _shaderPreset->passes.end();
		 i != end; ++i) {
		Common::FSNode fileNode(getChildRecursive(_shaderPreset->basePath, i->fileName));

		Common::SeekableReadStream *stream = fileNode.createReadStream();
		if (!stream) {
			warning("LibRetroPipeline::loadPasses: Could not open file '%s'", fileNode.isReadable() ? fileNode.getName().c_str() : i->fileName.c_str());
			return false;
		}

		Common::Array<char> shaderFileContents;
		shaderFileContents.resize(stream->size() + 1);
		shaderFileContents[stream->size()] = 0;
		const bool readSuccess = stream->read(shaderFileContents.begin(), stream->size()) == (uint32)stream->size();
		delete stream;
		stream = nullptr;

		if (!readSuccess) {
			warning("LibRetroPipeline::loadPasses: Could not read file '%s'", fileNode.getName().c_str());
			return false;
		}

		char *shaderFileStart = shaderFileContents.begin();
		char version[32] = { '\0' };

		// If the shader contains a version directive, it needs to be parsed and stripped out so that the VERTEX
		// and FRAGMENT defines can be prepended to it.
		const char *existing_version = strstr(shaderFileStart, "#version");
		if (existing_version) {
			const char *shaderFileVersionExtra = "";
			unsigned long shaderFileVersion = strtoul(existing_version + 8, &shaderFileStart, 10);
			if (OpenGLContext.type == kContextGLES2) {
				if (shaderFileVersion < 130) {
					shaderFileVersion = 100;
				} else {
					shaderFileVersionExtra = " es";
					shaderFileVersion = 300;
				}
			}
			snprintf(version, sizeof(version), "#version %lu%s\n",
					shaderFileVersion, shaderFileVersionExtra);
		}

		// TODO: Handle alias defines

		Shader *shader = new Shader;

		const char *const vertexSources[] = {
			version,
			"#define VERTEX\n", // "#define PARAMETER_UNIFORM\n",
			// TODO: alias defines
			shaderFileStart,
		};
		const char *const fragmentSources[] = {
			version,
			"#define FRAGMENT\n", // "#define PARAMETER_UNIFORM\n",
			// TODO: alias defines
			shaderFileStart,
		};


		if (!shader->loadFromStringsArray(fileNode.getName(),
				 ARRAYSIZE(vertexSources), vertexSources,
				 ARRAYSIZE(fragmentSources), fragmentSources,
				 g_libretroShaderAttributes)) {
			return false;
		}

		// Set uniforms with fixed value throughout lifetime.
		// We do not support rewinding, thus fix 'forward'.
		shader->setUniform("FrameDirection", 1);
		// Input texture is always bound at sampler 0.
		shader->setUniform("Texture", 0);

		TextureTarget *target = nullptr;
		// TODO: float and sRGB FBO handling.
		target = new TextureTarget();

		_passes.push_back(Pass(i, shader, target));
		Pass &pass = _passes[_passes.size() - 1];
		const uint passId = _passes.size() - 1;

		pass.buildTexCoords(passId);
		pass.buildTexSamplers(passId, _textures);
		if (passId > 0) {
			GLTexture *const texture = _passes[passId - 1].target->getTexture();
			texture->enableLinearFiltering(i->filteringMode == LibRetro::kFilteringModeLinear);
			pass.inputTexture = texture;
		}
	}

	// Now try to setup FBOs with some dummy size to make sure it could work
	uint bakInputWidth = _inputWidth;
	uint bakInputHeight = _inputHeight;
	uint bakOutputWidth = _outputWidth;
	uint bakOutputHeight = _outputHeight;

	_inputWidth = 320;
	_inputHeight = 200;
	_outputWidth = 640;
	_outputHeight = 480;

	bool ret = setupFBOs();

	_inputWidth = bakInputWidth;
	_inputHeight = bakInputHeight;
	_outputWidth = bakOutputWidth;
	_outputHeight = bakOutputHeight;
	// Force to reset everything at next draw
	_outputSizeChanged = true;

	if (!ret) {
		return false;
	}
	return true;
}

void LibRetroPipeline::setPipelineState() {
	// Setup FBO sizes, we require this to be able to set all uniform values.
	setupFBOs();

	// Setup all pass uniforms. This makes sure all the correct video and
	// output sizes are set.
	for (PassArray::size_type id = 0; id < _passes.size(); ++id) {
		setupPassUniforms(id);
	}
}

bool LibRetroPipeline::setupFBOs() {
	float sourceW = _inputWidth;
	float sourceH = _inputHeight;

	const float viewportW = _outputWidth;
	const float viewportH = _outputHeight;

	for (PassArray::size_type i = 0; i < _passes.size(); ++i) {
		Pass &pass = _passes[i];

		// Apply scaling for current pass.
		pass.shaderPass->applyScale(sourceW, sourceH, viewportW, viewportH, &sourceW, &sourceH);

		// Resize FBO to fit the output of the pass.
		pass.target->setSize((uint)sourceW, (uint)sourceH);
		// Make sure it has been set correctly
		GLint width = 0, height = 0;
		GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,  GL_TEXTURE_WIDTH, &width));
		GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,  GL_TEXTURE_HEIGHT, &height));
		if ((uint)width != (uint)sourceW || (uint)height != (uint)sourceH) {
			return false;
		}

		// Store draw coordinates.
		pass.vertexCoord[0] = 0;
		pass.vertexCoord[1] = 0;

		pass.vertexCoord[2] = (uint)sourceW;
		pass.vertexCoord[3] = 0;

		pass.vertexCoord[4] = 0;
		pass.vertexCoord[5] = (uint)sourceH;

		pass.vertexCoord[6] = (uint)sourceW;
		pass.vertexCoord[7] = (uint)sourceH;

		// Set projection matrix in passes's shader.
		Math::Matrix4 m4;
		m4.setData(pass.target->getProjectionMatrix());
		pass.shader->setUniform("MVPMatrix", m4);
	}
	return true;
}

void LibRetroPipeline::setupPassUniforms(const uint id) {
	Pass &pass = _passes[id];
	Shader *const shader = pass.shader;

	// Set output dimensions.
	shader->setUniform("OutputSize", Math::Vector2d(_outputWidth, _outputHeight));

	// Set texture dimensions for input, original, and the passes.
	setShaderTexUniforms(Common::String(), shader, *pass.inputTexture);
	setShaderTexUniforms("Orig", shader, *_passes[0].inputTexture);
	for (uint passId = 1; id >= 2 && passId <= id - 1; ++passId) {
		setShaderTexUniforms(Common::String::format("Pass%u", passId), shader, *_passes[passId].inputTexture);
	}

	// TODO: We do not support Prev right now. Instead we always use the orig
	// texture for these.
	setShaderTexUniforms("Prev", shader, *_passes[0].inputTexture);
	for (uint prevId = 1; prevId <= 6; ++prevId) {
		setShaderTexUniforms(Common::String::format("Prev%u", prevId), shader, *_passes[0].inputTexture);
	}
}

void LibRetroPipeline::setShaderTexUniforms(const Common::String &prefix, Shader *shader, const GLTexture &texture) {
	shader->setUniform(prefix + "InputSize", Math::Vector2d(texture.getLogicalWidth(), texture.getLogicalHeight()));
	shader->setUniform(prefix + "TextureSize", Math::Vector2d(texture.getWidth(), texture.getHeight()));
}

LibRetroPipeline::Texture LibRetroPipeline::loadTexture(const Common::FSNode &fileNode) {
	Common::String fileName = fileNode.getName();
	const char *extension = nullptr;
	for (int dotPos = fileName.size() - 1; dotPos >= 0; --dotPos) {
		if (fileName[dotPos] == '.') {
			extension = fileName.c_str() + dotPos + 1;
			break;
		}
	}

	if (!extension) {
		warning("LibRetroPipeline::loadTexture: File name '%s' misses extension", fileName.c_str());
		return Texture();
	}

	for (const ImageLoader *loader = s_imageLoaders; loader->extension; ++loader) {
		if (!scumm_stricmp(loader->extension, extension)) {
			Graphics::Surface *textureData = loader->load(fileNode);
			if (!textureData) {
				warning("LibRetroPipeline::loadTexture: Loader for '%s' could not load file '%s'", loader->extension, fileName.c_str());
				return Texture();
			}

			GLTexture *texture = new GLTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
			texture->setSize(textureData->w, textureData->h);
			texture->updateArea(Common::Rect(textureData->w, textureData->h), *textureData);
			return Texture(textureData, texture);
		}
	}

	warning("LibRetroPipeline::loadTexture: No loader for file '%s' present", fileName.c_str());
	return Texture();
}

void LibRetroPipeline::Pass::buildTexCoords(const uint id) {
	texCoords.clear();

	addTexCoord("TexCoord", TexCoordAttribute::kTypePass, id);
	addTexCoord("OrigTexCoord", TexCoordAttribute::kTypePass, 0);
	addTexCoord("LUTTexCoord", TexCoordAttribute::kTypeTexture, 0);

	for (uint pass = 1; id >= 2 && pass <= id - 1; ++pass) {
		addTexCoord(Common::String::format("Pass%uTexCoord", pass), TexCoordAttribute::kTypePass, pass);
	}

	addTexCoord("PrevTexCoord", TexCoordAttribute::kTypePrev, 0);
	for (uint prevId = 1; prevId <= 6; ++prevId) {
		addTexCoord(Common::String::format("Prev%uTexCoord", prevId), TexCoordAttribute::kTypePrev, prevId);
	}
}

void LibRetroPipeline::Pass::addTexCoord(const Common::String &name, const TexCoordAttribute::Type type, const uint index) {
	if (shader->addAttribute(name.c_str())) {
		texCoords.push_back(TexCoordAttribute(name, type, index));
	}
}

void LibRetroPipeline::Pass::buildTexSamplers(const uint id, const TextureArray &textures) {
	texSamplers.clear();
	uint sampler = 1;

	// 1. Step: Assign shader textures to samplers.
	for (TextureArray::size_type i = 0; i < textures.size(); ++i) {
		addTexSampler(textures[i].id, &sampler, TextureSampler::kTypeTexture, i, true);
	}

	// 2. Step: Assign pass inputs to samplers.
	for (uint pass = 1; id >= 2 && pass <= id - 1; ++pass) {
		addTexSampler(Common::String::format("Pass%u", pass), &sampler, TextureSampler::kTypePass, pass);
	}

	// 3. Step: Assign original input to samplers.
	addTexSampler("Orig", &sampler, TextureSampler::kTypePass, 0);

	// 4. Step: Assign previous render inputs.
	addTexSampler("Prev", &sampler, TextureSampler::kTypePrev, 0);
	for (uint prevId = 1; prevId <= 6; ++prevId) {
		addTexSampler(Common::String::format("Prev%u", prevId), &sampler, TextureSampler::kTypePrev, prevId);
	}
}

void LibRetroPipeline::Pass::addTexSampler(const Common::String &prefix, uint *unit, const TextureSampler::Type type, const uint index, const bool prefixIsId) {
	const Common::String id = prefixIsId ? prefix : (prefix + "Texture");

	if (shader->setUniform(id, *unit)) {
		texSamplers.push_back(TextureSampler((*unit)++, type, index));
	}
}

void LibRetroPipeline::renderPass(const Pass &pass) {
	// Activate shader and framebuffer to be used for rendering.
	pass.shader->use();
	setFramebuffer(pass.target);

	// Activate attribute arrays and setup matching attributes.
	renderPassSetupCoordinates(pass);

	// Bind textures to samplers.
	renderPassSetupTextures(pass);

	// Actually draw something.
	GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	// Unbind shader.
	pass.shader->unbind();
}

void LibRetroPipeline::renderPassSetupCoordinates(const Pass &pass) {
	pass.shader->enableVertexAttribute("VertexCoord", 2, GL_FLOAT, GL_FALSE, 0, pass.vertexCoord);

	for (Pass::TexCoordAttributeArray::const_iterator i = pass.texCoords.begin(), end = pass.texCoords.end();
		 i != end; ++i) {
		const GLfloat *texCoords = nullptr;

		switch (i->type) {
		case Pass::TexCoordAttribute::kTypeTexture:
			texCoords = _textures[i->index].glTexture->getTexCoords();
			break;

		case Pass::TexCoordAttribute::kTypePass:
			texCoords = _passes[i->index].inputTexture->getTexCoords();
			break;

		case Pass::TexCoordAttribute::kTypePrev:
			// TODO: Properly support Prev
			texCoords = _passes[0].inputTexture->getTexCoords();
			break;
		}

		if (!texCoords) {
			continue;
		}

		pass.shader->enableVertexAttribute(i->name.c_str(), 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	}
}

void LibRetroPipeline::renderPassSetupTextures(const Pass &pass) {
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	pass.inputTexture->bind();

	// In case the pass requests mipmaps for the input texture we generate
	// we make GL generate them here.
	if (pass.shaderPass->mipmapInput) {
		GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	}

	for (Pass::TextureSamplerArray::const_iterator i = pass.texSamplers.begin(), end = pass.texSamplers.end();
		 i != end; ++i) {
		const GLTexture *texture = nullptr;

		switch (i->type) {
		case Pass::TextureSampler::kTypeTexture:
			texture = _textures[i->index].glTexture;
			break;

		case Pass::TextureSampler::kTypePass:
			texture = _passes[i->index].inputTexture;
			break;

		case Pass::TextureSampler::kTypePrev:
			// TODO: Properly support Prev
			texture = _passes[0].inputTexture;
			break;
		}

		if (!texture) {
			continue;
		}

		GL_CALL(glActiveTexture(GL_TEXTURE0 + i->unit));
		texture->bind();
	}
}

} // End of namespace OpenGL
#endif // !USE_FORCED_GLES
