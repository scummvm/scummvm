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

#include "backends/graphics/opengl/opengl-sys.h"

#if !USE_FORCED_GLES
#include "backends/graphics/opengl/pipelines/libretro.h"
#include "backends/graphics/opengl/pipelines/libretro/parser.h"
#include "backends/graphics/opengl/shader.h"
#include "backends/graphics/opengl/framebuffer.h"

#include "common/textconsole.h"
#include "common/fs.h"
#include "common/stream.h"

#include "graphics/surface.h"

#include "image/bmp.h"
#include "image/png.h"
#include "image/tga.h"

namespace OpenGL {

template<typename DecoderType>
static Graphics::Surface *loadViaImageDecoder(const Common::String &fileName) {
	Common::FSNode fileNode(fileName);
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
	Graphics::Surface *(*load)(const Common::String &fileName);
};

static const ImageLoader s_imageLoaders[] = {
	{ "bmp", loadViaImageDecoder<Image::BitmapDecoder> },
	{ "png", loadViaImageDecoder<Image::PNGDecoder> },
	{ "tga", loadViaImageDecoder<Image::TGADecoder> },
	{ nullptr, nullptr }
};

LibRetroPipeline::LibRetroPipeline(const Common::String &presetFileName)
    : ShaderPipeline(ShaderMan.query(ShaderManager::kDefault)),
      _shaderPreset(LibRetro::parsePreset(presetFileName)), _applyProjectionChanges(false),
      _inputWidth(0), _inputHeight(0), _outputWidth(0), _outputHeight(0) {
	initialize();
}

LibRetroPipeline::~LibRetroPipeline() {
	for (TextureArray::size_type i = 0; i < _textures.size(); ++i) {
		if (_textures[i].textureData) {
			_textures[i].textureData->free();
		}
		delete _textures[i].textureData;
		delete _textures[i].glTexture;
	}

	for (PassArray::size_type i = 0; i < _passes.size(); ++i) {
		delete _passes[i].shader;
		delete _passes[i].target;
	}

	delete _shaderPreset;
}

void LibRetroPipeline::drawTexture(const GLTexture &texture, const GLfloat *coordinates) {
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
	ShaderPipeline::drawTexture(*_passes[_passes.size() - 1].target->getTexture(), coordinates);
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

void LibRetroPipeline::initialize() {
	// TODO: Error handling.

	loadTextures();
	loadPasses();
}

void LibRetroPipeline::loadTextures() {
	for (LibRetro::ShaderPreset::TextureArray::const_iterator
	     i = _shaderPreset->textures.begin(), end = _shaderPreset->textures.end();
	     i != end; ++i) {
		// TODO: proper path look up
		Common::String fileName = Common::normalizePath(_shaderPreset->basePath + '/' + i->fileName, '/');

		Texture texture = loadTexture(fileName);
		texture.id = i->id;

		// TODO: Error handling
		if (!texture.textureData || !texture.glTexture) {
		}

		texture.glTexture->enableLinearFiltering(i->filteringMode == LibRetro::kFilteringModeLinear);
		_textures.push_back(texture);
	}
}

void LibRetroPipeline::loadPasses() {
	for (LibRetro::ShaderPreset::PassArray::const_iterator
	     i = _shaderPreset->passes.begin(), end = _shaderPreset->passes.end();
	     i != end; ++i) {
		// TODO: proper path look up
		Common::String fileName = Common::normalizePath(_shaderPreset->basePath + '/' + i->fileName, '/');
		Common::FSNode fileNode(fileName);

		Common::SeekableReadStream *stream = fileNode.createReadStream();
		if (!stream) {
			warning("LibRetroPipeline::loadPasses: Could not open file '%s'", fileName.c_str());
			// TODO: Error handling
			continue;
		}

		Common::Array<char> shaderFileContents;
		shaderFileContents.resize(stream->size() + 1);
		shaderFileContents[stream->size()] = 0;
		const bool readSuccess = stream->read(shaderFileContents.begin(), stream->size()) == (uint32)stream->size();
		delete stream;
		stream = nullptr;

		if (!readSuccess) {
			warning("LibRetroPipeline::loadPasses: Could not read file '%s'", fileName.c_str());
			// TODO: Error handling
		}

		Shader *shader = new Shader("#define VERTEX\n" + Common::String(shaderFileContents.begin()),
		                            "#define FRAGMENT\n" + Common::String(shaderFileContents.begin()));

		// Set uniforms with fixed value throughout lifetime.
		// We do not support rewinding, thus fix 'forward'.
		shader->setUniform1I("FrameDirection", 1);
		// Input texture is always bound at sampler 0.
		shader->setUniform1I("Texture", 0);

		TextureTarget *target = nullptr;
		// TODO: float and sRGB FBO handling.
		target = new TextureTarget();

		_passes.push_back(Pass(i, shader, target));
		Pass &pass = _passes[_passes.size() - 1];
		const uint passId = _passes.size() - 1;

		pass.vertexCoordLocation = shader->getAttributeLocation("VertexCoord");
		pass.buildTexCoords(passId);
		pass.buildTexSamplers(passId, _textures);
		if (passId > 0) {
			GLTexture *const texture = _passes[passId - 1].target->getTexture();
			texture->enableLinearFiltering(i->filteringMode == LibRetro::kFilteringModeLinear);
			pass.inputTexture = texture;
		}
	}
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

void LibRetroPipeline::setupFBOs() {
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
		pass.shader->setUniform("MVPMatrix", new ShaderUniformMatrix44(pass.target->getProjectionMatrix()));
	}
}

void LibRetroPipeline::setupPassUniforms(const uint id) {
	Pass &pass = _passes[id];
	Shader *const shader = pass.shader;

	// Set output dimensions.
	shader->setUniform2F("OutputSize", _outputWidth, _outputHeight);

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
	shader->setUniform2F(prefix + "InputSize", texture.getLogicalWidth(), texture.getLogicalHeight());
	shader->setUniform2F(prefix + "TextureSize", texture.getWidth(), texture.getHeight());
}

LibRetroPipeline::Texture LibRetroPipeline::loadTexture(const Common::String &fileName) {
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
			Graphics::Surface *textureData = loader->load(fileName);
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

	addTexCoord(Common::String(), TexCoordAttribute::kTypePass, id);
	addTexCoord("Orig", TexCoordAttribute::kTypePass, 0);
	addTexCoord("LUT", TexCoordAttribute::kTypeTexture, 0);

	for (uint pass = 1; id >= 2 && pass <= id - 1; ++pass) {
		addTexCoord(Common::String::format("Pass%u", pass), TexCoordAttribute::kTypePass, pass);
	}

	addTexCoord("Prev", TexCoordAttribute::kTypePrev, 0);
	for (uint prevId = 1; prevId <= 6; ++prevId) {
		addTexCoord(Common::String::format("Prev%u", prevId), TexCoordAttribute::kTypePrev, prevId);
	}
}

void LibRetroPipeline::Pass::addTexCoord(const Common::String &prefix, const TexCoordAttribute::Type type, const uint index) {
	GLint location = shader->getAttributeLocation(prefix + "TexCoord");
	if (location != -1) {
		texCoords.push_back(TexCoordAttribute(location, type, index));
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

	if (shader->setUniform1I(id, *unit)) {
		texSamplers.push_back(TextureSampler((*unit)++, type, index));
	}
}

void LibRetroPipeline::renderPass(const Pass &pass) {
	// Activate shader and framebuffer to be used for rendering.
	pass.shader->activate();
	setFramebuffer(pass.target);

	// Activate attribute arrays and setup matching attributes.
	renderPassSetupCoordinates(pass);

	// Bind textures to samplers.
	renderPassSetupTextures(pass);

	// Actually draw something.
	GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	// Deactivate attribute arrays.
	renderPassCleanupCoordinates(pass);

	// Deactivate shader.
	pass.shader->deactivate();
}

void LibRetroPipeline::renderPassSetupCoordinates(const Pass &pass) {
	GL_CALL(glEnableVertexAttribArray(pass.vertexCoordLocation));
	GL_CALL(glVertexAttribPointer(pass.vertexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, pass.vertexCoord));

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

		GL_CALL(glEnableVertexAttribArray(i->location));
		GL_CALL(glVertexAttribPointer(i->location, 2, GL_FLOAT, GL_FALSE, 0, texCoords));
	}
}

void LibRetroPipeline::renderPassCleanupCoordinates(const Pass &pass) {
	GL_CALL(glDisableVertexAttribArray(pass.vertexCoordLocation));

	for (Pass::TexCoordAttributeArray::const_iterator i = pass.texCoords.begin(), end = pass.texCoords.end();
	     i != end; ++i) {
		GL_CALL(glDisableVertexAttribArray(i->location));
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
