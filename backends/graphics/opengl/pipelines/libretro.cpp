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
#include "image/jpeg.h"
#include "image/png.h"
#include "image/tga.h"

namespace OpenGL {

using LibRetro::UniformsMap;

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
										   // Use a cast to resolve ambiguities in JPEGDecoder
										   static_cast<Image::ImageDecoder &>(decoder).getPalette());
}

struct ImageLoader {
	const char *extension;
	Graphics::Surface *(*load)(const Common::FSNode &fileNode);
};

static const ImageLoader s_imageLoaders[] = {
	{ "bmp", loadViaImageDecoder<Image::BitmapDecoder> },
	{ "jpg", loadViaImageDecoder<Image::JPEGDecoder> },
	{ "png", loadViaImageDecoder<Image::PNGDecoder> },
	{ "tga", loadViaImageDecoder<Image::TGADecoder> },
	{ nullptr, nullptr }
};

static const char *const g_libretroShaderAttributes[] = {
	"VertexCoord", nullptr
};

// some libretro shaders use texture without checking version
static const char *g_compatVertex =
	"#if defined(GL_ES)\n"
		"#if !defined(HAS_ROUND)\n"
			"#define round(x) (sign(x) * floor(abs(x) + .5))\n"
		"#endif\n"
	"#elif __VERSION__ < 130\n"
		"#if !defined(HAS_ROUND)\n"
			"#define round(x) (sign(x) * floor(abs(x) + .5))\n"
		"#endif\n"
	"#endif\n";

static const char *g_compatFragment =
	"#if defined(GL_ES)\n"
		"#if !defined(HAS_ROUND)\n"
			"#define round(x) (sign(x) * floor(abs(x) + .5))\n"
		"#endif\n"
		"#if !defined(HAS_TEXTURE)\n"
			"#define texture texture2D\n"
		"#endif\n"
	"#elif __VERSION__ < 130\n"
		"#if !defined(HAS_ROUND)\n"
			"#define round(x) (sign(x) * floor(abs(x) + .5))\n"
		"#endif\n"
		"#if !defined(HAS_TEXTURE)\n"
			"#define texture texture2D\n"
		"#endif\n"
	"#endif\n";

LibRetroPipeline::LibRetroPipeline()
	: ShaderPipeline(ShaderMan.query(ShaderManager::kDefault)),
	  _shaderPreset(nullptr), _applyProjectionChanges(false),
	  _inputWidth(0), _inputHeight(0), _outputWidth(0), _outputHeight(0),
	  _isAnimated(false), _frameCount(0) {
}

LibRetroPipeline::LibRetroPipeline(const Common::FSNode &shaderPreset)
	: ShaderPipeline(ShaderMan.query(ShaderManager::kDefault)),
	  _shaderPreset(nullptr), _applyProjectionChanges(false),
	  _inputWidth(0), _inputHeight(0), _outputWidth(0), _outputHeight(0),
	  _isAnimated(false), _frameCount(0) {
	open(shaderPreset);
}

LibRetroPipeline::~LibRetroPipeline() {
	close();
}

void LibRetroPipeline::drawTexture(const GLTexture &texture, const GLfloat *coordinates, const GLfloat *texcoords) {
	Framebuffer *const targetBuffer = _activeFramebuffer;

	// Set input texture for 1st pass to texture to draw.
	_passes[0].inputTexture = &texture;

	// Get back screen size from texture coordinates.
	// FIXME: We assume a fixed set of triangle strip
	GLfloat outputWidth, outputHeight;
	outputWidth = coordinates[6] - coordinates[0];
	outputHeight = coordinates[7] - coordinates[1];

	bool outputSizeChanged = (_outputWidth != outputWidth || _outputHeight != outputHeight);
	// Save output dimensions.
	_outputWidth  = outputWidth;
	_outputHeight = outputHeight;


	// In case texture dimensions or viewport dimensions changed, we need to
	// update the pipeline's state.
	if (   texture.getLogicalWidth() != _inputWidth
		|| texture.getLogicalHeight() != _inputHeight
		|| outputSizeChanged) {
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

	_frameCount++;
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
		texture.glTexture->setWrapMode(i->wrapMode);
		_textures.push_back(texture);
	}
	return true;
}

static void stripShaderParameters(char *source, UniformsMap &uniforms) {
	char uniformId[64], desc[64];
	float initial, minimum, maximum, step;

	char *s = strstr(source, "#pragma parameter");

	while (s) {
		int ret;
		if ((ret = sscanf(s, "#pragma parameter %63s \"%63[^\"]\" %f %f %f %f",
                  uniformId, desc, &initial, &minimum, &maximum, &step)) >= 5) {
			uniforms[uniformId] = initial;
		}

		// strip parameter to avoid syntax errors in GLSL parser
		while (*s != '\0' && *s != '\n') {
			*s++ = ' ';
		}
		s = strstr(s, "#pragma parameter");
	}
}

bool LibRetroPipeline::loadPasses() {
	// Error out if there are no passes
	if (!_shaderPreset->passes.size()) {
		return false;
	}

	// First of all, build the aliases list
	Common::String aliasesDefines;
	Common::StringArray aliases;

	aliases.reserve(_shaderPreset->passes.size());
	for (LibRetro::ShaderPreset::PassArray::const_iterator
		 i = _shaderPreset->passes.begin(), end = _shaderPreset->passes.end();
		 i != end; ++i) {
		aliases.push_back(i->alias);
		if (!i->alias.empty()) {
			aliasesDefines += Common::String::format("#define %s_ALIAS\n", i->alias.c_str());
		}
	}

	_isAnimated = false;

	// parameters are shared among all passes so we load them first and apply them to all shaders
	UniformsMap uniformParams;
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

		stripShaderParameters(shaderFileStart, uniformParams);

		Common::String shimsDetected;
		if (strstr(shaderFileStart, "#define texture(")) {
			shimsDetected += "#define HAS_TEXTURE\n";
		} else if (strstr(shaderFileStart, "#define texture ")) {
			shimsDetected += "#define HAS_TEXTURE\n";
		}
		if (strstr(shaderFileStart, "#define round(")) {
			shimsDetected += "#define HAS_ROUND\n";
		}

		Shader *shader = new Shader;

		const char *const vertexSources[] = {
			version,
			"#define VERTEX\n#define PARAMETER_UNIFORM\n",
			shimsDetected.c_str(),
			g_compatVertex,
			aliasesDefines.c_str(),
			shaderFileStart,
		};
		const char *const fragmentSources[] = {
			version,
			"#define FRAGMENT\n#define PARAMETER_UNIFORM\n",
			shimsDetected.c_str(),
			g_compatFragment,
			aliasesDefines.c_str(),
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

		pass.hasFrameCount = shader->getUniformLocation("FrameCount") != -1;
		// If pass has FrameCount uniform, preset is animated and must be redrawn on a regular basis
		_isAnimated |= pass.hasFrameCount;

		pass.buildTexCoords(passId, aliases);
		pass.buildTexSamplers(passId, _textures, aliases);
		if (passId > 0) {
			GLTexture *const texture = _passes[passId - 1].target->getTexture();
			texture->enableLinearFiltering(i->filteringMode == LibRetro::kFilteringModeLinear);
			texture->setWrapMode(i->wrapMode);
			pass.inputTexture = texture;
		}
	}

	// Apply preset parameters last to override all others
	for(UniformsMap::iterator it = _shaderPreset->parameters.begin(); it != _shaderPreset->parameters.end(); it++) {
		uniformParams[it->_key] = it->_value;
	}

	// Finally apply parameters to all shaders as uniforms
	for(PassArray::iterator i = _passes.begin(); i != _passes.end(); i++) {
		for(UniformsMap::iterator it = uniformParams.begin(); it != uniformParams.end(); it++) {
			i->shader->setUniform1f(it->_key, it->_value);
		}
	}


	// Now try to setup FBOs with some dummy size to make sure it could work
	_inputWidth = 320;
	_inputHeight = 200;
	_outputWidth = 640;
	_outputHeight = 480;

	bool ret = setupFBOs();

	_inputWidth = 0;
	_inputHeight = 0;
	_outputWidth = 0;
	_outputHeight = 0;

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
		if (!pass.target->setSize((uint)sourceW, (uint)sourceH)) {
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
	shader->setUniform("OutputSize", Math::Vector2d(pass.target->getTexture()->getLogicalWidth(), pass.target->getTexture()->getLogicalHeight()));

	// Set texture dimensions for input, original, and the passes.
	setShaderTexUniforms(Common::String(), shader, *pass.inputTexture);
	setShaderTexUniforms("Orig", shader, *_passes[0].inputTexture);
	if (id >= 1) {
		setShaderTexUniforms(Common::String::format("PassPrev%u", id + 1), shader, *_passes[0].inputTexture);
		for (uint passId = 0; passId < id; ++passId) {
			// Pass1 is the output texture of first pass, ie. the input texture of second pass (indexed 1)
			setShaderTexUniforms(Common::String::format("Pass%u", passId + 1), shader, *_passes[passId + 1].inputTexture);
			// PassPrev1 is the output texture of last pass, ie. the input texture of current pass
			setShaderTexUniforms(Common::String::format("PassPrev%u", id - passId), shader, *_passes[passId + 1].inputTexture);

			// If pass has an alias, define the uniforms using the input texture of the next pass
			if (!_passes[passId].shaderPass->alias.empty()) {
				setShaderTexUniforms(_passes[passId].shaderPass->alias, shader, *_passes[passId + 1].inputTexture);
			}
		}
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

void LibRetroPipeline::Pass::buildTexCoords(const uint id, const Common::StringArray &aliases) {
	texCoords.clear();

	addTexCoord("TexCoord", TexCoordAttribute::kTypePass, id);
	addTexCoord("OrigTexCoord", TexCoordAttribute::kTypePass, 0);
	addTexCoord("LUTTexCoord", TexCoordAttribute::kTypeTexture, 0);

	if (id >= 1) {
		addTexCoord(Common::String::format("PassPrev%uTexCoord", id + 1), TexCoordAttribute::kTypePass, 0);
		for (uint pass = 0; pass < id; ++pass) {
			// Pass1TexCoord is the output texture coords of first pass, ie. the input texture coords of second pass (indexed 1)
			addTexCoord(Common::String::format("Pass%uTexCoord", pass + 1), TexCoordAttribute::kTypePass, pass + 1);
			// PassPrev1TexCoord is the output texture coords of last pass, ie. the input texture coords of current pass
			addTexCoord(Common::String::format("PassPrev%uTexCoord", id - pass), TexCoordAttribute::kTypePass, pass + 1);

			// If pass has an alias, define the uniforms using the input texture coords of the next pass
			if (!aliases[pass].empty()) {
				addTexCoord(Common::String::format("%sTexCoord", aliases[pass].c_str()), TexCoordAttribute::kTypePass, pass + 1);
			}
		}
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

void LibRetroPipeline::Pass::buildTexSamplers(const uint id, const TextureArray &textures, const Common::StringArray &aliases) {
	texSamplers.clear();
	uint sampler = 1;

	// 1. Step: Assign shader textures to samplers.
	for (TextureArray::size_type i = 0; i < textures.size(); ++i) {
		addTexSampler(textures[i].id, &sampler, TextureSampler::kTypeTexture, i, true);
	}

	// 2. Step: Assign pass inputs to samplers.
	if (id >= 1) {
		addTexSampler(Common::String::format("PassPrev%u", id + 1), &sampler, TextureSampler::kTypePass, 0);
		for (uint pass = 0; pass < id; ++pass) {
			// Pass1 is the output texture of first pass, ie. the input texture of second pass (indexed 1)
			addTexSampler(Common::String::format("Pass%u", pass + 1), &sampler, TextureSampler::kTypePass, pass + 1);
			// PassPrev1 is the output texture of last pass, ie. the input texture of current pass
			addTexSampler(Common::String::format("PassPrev%u", id - pass), &sampler, TextureSampler::kTypePass, pass + 1);

			// If pass has an alias, define the uniforms using the input texture of the next pass
			if (!aliases[pass].empty()) {
				addTexSampler(aliases[pass], &sampler, TextureSampler::kTypePass, pass + 1);
			}
		}
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

	/* Search in the samplers if we already have one for the texture */
	for(TextureSamplerArray::iterator it = texSamplers.begin(); it != texSamplers.end(); it++) {
		if (it->type == type && it->index == index) {
			shader->setUniform(id, it->unit);
			return;
		}
	}

	if (shader->setUniform(id, *unit)) {
		texSamplers.push_back(TextureSampler((*unit)++, type, index));
	}
}

void LibRetroPipeline::renderPass(const Pass &pass) {
	// Activate shader and framebuffer to be used for rendering.
	pass.shader->use();
	setFramebuffer(pass.target);

	if (pass.hasFrameCount) {
		uint frameCount = _frameCount;
		if (pass.shaderPass->frameCountMod) {
			frameCount %= pass.shaderPass->frameCountMod;
		}
		pass.shader->setUniform("FrameCount", frameCount);
	}

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
