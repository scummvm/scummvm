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
static Graphics::Surface *loadViaImageDecoder(const Common::Path &fileName, Common::Archive *container, Common::SearchSet &archSet) {
	Common::SeekableReadStream *stream = nullptr;

	if (container) {
		// Look first in our current container and fallback on archive set
		if (container->hasFile(fileName)) {
			stream = container->createReadStreamForMember(fileName);
		}
		if (!stream) {
			stream = archSet.createReadStreamForMemberNext(fileName, container);
		}
		if (!stream) {
			warning("LibRetroPipeline::loadViaImageDecoder: Invalid file path '%s'", fileName.toString().c_str());
			return nullptr;
		}
	} else {
		Common::FSNode fsnode(fileName);
		if (!fsnode.exists() || !fsnode.isReadable() || fsnode.isDirectory()
				|| !(stream = fsnode.createReadStream())) {
			warning("LibRetroPipeline::loadViaImageDecoder: Invalid file path '%s'", fileName.toString(Common::Path::kNativeSeparator).c_str());
			return nullptr;
		}
	}

	DecoderType decoder;
	const bool success = decoder.loadStream(*stream);

	delete stream;

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
	Graphics::Surface *(*load)(const Common::Path &fileName, Common::Archive *container, Common::SearchSet &archSet);
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
static const char *const g_compatVertex =
	"#if defined(GL_ES)\n"
		"#if !defined(HAS_ROUND)\n"
			"#define round(x) (sign(x) * floor(abs(x) + .5))\n"
		"#endif\n"
	"#elif __VERSION__ < 130\n"
		"#if !defined(HAS_ROUND)\n"
			"#define round(x) (sign(x) * floor(abs(x) + .5))\n"
		"#endif\n"
	"#endif\n";

static const char *const g_compatFragment =
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

/* This TextureTarget handles the scaling of coordinates from the window coordinates space
 * to the input coordinates space */
class LibRetroTextureTarget : public TextureTarget {
public:
	bool setScaledSize(uint width, uint height, const Common::Rect &scalingRect) {
		Texture *texture = getTexture();
		if (!texture->setSize(width, height)) {
			return false;
		}

		const uint texWidth  = texture->getWidth();
		const uint texHeight = texture->getHeight();

		// Set viewport dimensions.
		_viewport[0] = 0;
		_viewport[1] = 0;
		_viewport[2] = texWidth;
		_viewport[3] = texHeight;

		const float ratioW = (float)width  / scalingRect.width();
		const float ratioH = (float)height / scalingRect.height();

		// Setup scaling projection matrix.
		// This projection takes window screen coordinates and converts it to input coordinates normalized
		_projectionMatrix(0, 0) = 2.f * ratioW / texWidth;
		_projectionMatrix(0, 1) = 0.f;
		_projectionMatrix(0, 2) = 0.f;
		_projectionMatrix(0, 3) = 0.f;

		_projectionMatrix(1, 0) = 0.f;
		_projectionMatrix(1, 1) = 2.f * ratioH / texHeight;
		_projectionMatrix(1, 2) = 0.f;
		_projectionMatrix(1, 3) = 0.f;

		_projectionMatrix(2, 0) = 0.f;
		_projectionMatrix(2, 1) = 0.f;
		_projectionMatrix(2, 2) = 0.f;
		_projectionMatrix(2, 3) = 0.f;

		_projectionMatrix(3, 0) = -1.f - (2.f * scalingRect.left) * ratioW / texWidth;
		_projectionMatrix(3, 1) = -1.f - (2.f * scalingRect.top) * ratioH / texHeight;
		_projectionMatrix(3, 2) = 0.0f;
		_projectionMatrix(3, 3) = 1.0f;

		// Directly apply changes when we are active.
		if (isActive()) {
			applyViewport();
			applyProjectionMatrix();
		}
		return true;
	}
};

/* This Pipeline is used by the Framebuffer objects in the LibRetro passes.
 * It does nothing as everything is already handled by us. */
class FakePipeline : public Pipeline {
public:
	FakePipeline() {}
	void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) override { }
	void setProjectionMatrix(const Math::Matrix4 &projectionMatrix) override {}
protected:
	void activateInternal() override {}
	void deactivateInternal() override {}
	void drawTextureInternal(const Texture &texture, const GLfloat *coordinates, const GLfloat *texcoords) override { }
};

LibRetroPipeline::LibRetroPipeline()
	: _inputPipeline(ShaderMan.query(ShaderManager::kDefault)),
	  _outputPipeline(ShaderMan.query(ShaderManager::kDefault)),
	  _needsScaling(false), _shaderPreset(nullptr), _linearFiltering(false),
	  _currentTarget(uint(-1)), _inputWidth(0), _inputHeight(0),
	  _isAnimated(false), _frameCount(0) {
}

LibRetroPipeline::~LibRetroPipeline() {
	close();
}

/** Small helper to overcome that texture passed to drawTexture is const
 * This is not that clean but this allows to keep the OpenGLGraphicsManager code simple
 */
static void setLinearFiltering(GLuint glTexture, bool enable) {
	GLuint glFilter;
	if (enable) {
		glFilter = GL_LINEAR;
	} else {
		glFilter = GL_NEAREST;
	}

	GL_CALL(glBindTexture(GL_TEXTURE_2D, glTexture));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter));
}

void LibRetroPipeline::drawTextureInternal(const Texture &texture, const GLfloat *coordinates, const GLfloat *texcoords) {
	if (!_needsScaling) {
		_outputPipeline.drawTexture(texture, coordinates, texcoords);
		return;
	}

	// Disable linear filtering: we apply it after merging all to be scaled surfaces
	setLinearFiltering(texture.getGLTexture(), false);

	/* OpenGLGraphicsManager only knows about _activeFramebuffer and modify framebuffer here
	 * So let's synchronize our _inputTargets with it.
	 * Don't synchronize the scissor test as coordinates are wrong and we should not need it*/
	_inputTargets[_currentTarget].copyRenderStateFrom(*_activeFramebuffer,
			Framebuffer::kCopyMaskClearColor | Framebuffer::kCopyMaskBlendState);

	/* The backend sends us the coordinates in screen coordinates system
	 * So, when we are before libretro scaling, we need to scale back coordinates
	 * to our own coordinates system */
	_inputPipeline.drawTexture(texture, coordinates, texcoords);

	if (_linearFiltering) {
		// Enable back linear filtering as if nothing happened
		setLinearFiltering(texture.getGLTexture(), true);
	}
}

void LibRetroPipeline::beginScaling() {
	if (_shaderPreset != nullptr) {
		_needsScaling = true;
		_inputTargets[_currentTarget].getTexture()->enableLinearFiltering(_linearFiltering);
	}
}

void LibRetroPipeline::finishScaling() {
	if (!_needsScaling) {
		return;
	}

	/* As we have now finished to render everything in the input pipeline
	 * we can do the render through all libretro passes */

	// Now we can actually draw the texture with the setup passes.
	for (PassArray::const_iterator i = _passes.begin(), end = _passes.end(); i != end; ++i) {
		renderPass(*i);
	}

	// Prepare for the next frame
	_frameCount++;

	_currentTarget++;
	if (_currentTarget >= _inputTargets.size()) {
		_currentTarget = 0;
	}
	_passes[0].inputTexture = _inputTargets[_currentTarget].getTexture();

	// Clear the output buffer.
	_activeFramebuffer->activate(this);
	// Disable scissor test for clearing, it will get enabled back when activating the output pipeline
	GL_CALL(glDisable(GL_SCISSOR_TEST));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Finally, we need to render the result to the output pipeline
	_outputPipeline.setFramebuffer(_activeFramebuffer);
	_outputPipeline.activate();

	/* In retroarch, the shader directly draws on the backbuffer while we drew on a texture
	 * so everything is flipped when we draw this texture.
	 * Use custom coordinates to do the flipping. */
	GLfloat coordinates[4*2];
	coordinates[0] = _outputRect.left;
	coordinates[1] = _outputRect.bottom;

	coordinates[2] = _outputRect.right;
	coordinates[3] = _outputRect.bottom;

	coordinates[4] = _outputRect.left;
	coordinates[5] = _outputRect.top;

	coordinates[6] = _outputRect.right;
	coordinates[7] = _outputRect.top;

	_outputPipeline.drawTexture(*_passes[_passes.size() - 1].target->getTexture(), coordinates);

	_needsScaling = false;
}

void LibRetroPipeline::setDisplaySizes(uint inputWidth, uint inputHeight, const Common::Rect &outputRect) {
	_inputWidth = inputWidth;
	_inputHeight = inputHeight;
	_outputRect = outputRect;

	setPipelineState();
}

void LibRetroPipeline::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	_inputPipeline.setColor(r, g, b, a);
	_outputPipeline.setColor(r, g, b, a);
}

void LibRetroPipeline::setProjectionMatrix(const Math::Matrix4 &projectionMatrix) {
}

void LibRetroPipeline::activateInternal() {
	// Don't call Pipeline::activateInternal as our framebuffer is passed to _outputPipeline
	if (_needsScaling) {
		_inputPipeline.setFramebuffer(&_inputTargets[_currentTarget]);
		_inputPipeline.activate();
	} else {
		_outputPipeline.setFramebuffer(_activeFramebuffer);
		_outputPipeline.activate();
	}
}

void LibRetroPipeline::deactivateInternal() {
	// Don't call Pipeline::deactivateInternal as our framebuffer is passed to _outputPipeline
}

bool LibRetroPipeline::open(const Common::Path &shaderPreset, Common::SearchSet &archSet) {
	close();

	_shaderPreset = LibRetro::parsePreset(shaderPreset, archSet);
	if (!_shaderPreset)
		return false;

	if (!loadTextures(archSet)) {
		close();
		return false;
	}

	if (!loadPasses(archSet)) {
		close();
		return false;
	}

	setPipelineState();

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

	_isAnimated = false;
	_needsScaling = false;

	_inputTargets.resize(0);
	_currentTarget = uint(-1);
}

bool LibRetroPipeline::loadTextures(Common::SearchSet &archSet) {
	for (LibRetro::ShaderPreset::TextureArray::const_iterator
		 i = _shaderPreset->textures.begin(), end = _shaderPreset->textures.end();
		 i != end; ++i) {
		LibRetroTexture texture = loadTexture(_shaderPreset->basePath.join(i->fileName).normalize(), _shaderPreset->container, archSet);
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

bool LibRetroPipeline::loadPasses(Common::SearchSet &archSet) {
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
	uint maxPrevCount = 0;

	// parameters are shared among all passes so we load them first and apply them to all shaders
	UniformsMap uniformParams;
	for (LibRetro::ShaderPreset::PassArray::const_iterator
		 i = _shaderPreset->passes.begin(), end = _shaderPreset->passes.end();
		 i != end; ++i) {
		Common::Path fileName(_shaderPreset->basePath.join(i->fileName).normalize());
		Common::SeekableReadStream *stream = nullptr;

		if (_shaderPreset->container) {
			// Look first in our current container and fallback on archive set
			if (_shaderPreset->container->hasFile(fileName)) {
				stream = _shaderPreset->container->createReadStreamForMember(fileName);
			}
			if (!stream) {
				stream = archSet.createReadStreamForMemberNext(fileName, _shaderPreset->container);
			}
			if (!stream) {
				warning("LibRetroPipeline::loadPasses: Invalid file path '%s'", fileName.toString().c_str());
				return false;
			}
		} else {
			Common::FSNode fsnode(fileName);
			if (!fsnode.exists() || !fsnode.isReadable() || fsnode.isDirectory()
					|| !(stream = fsnode.createReadStream())) {
				warning("LibRetroPipeline::loadPasses: Invalid file path '%s'", fileName.toString(Common::Path::kNativeSeparator).c_str());
				return false;
			}
		}

		Common::Array<char> shaderFileContents;
		shaderFileContents.resize(stream->size() + 1);
		shaderFileContents[stream->size()] = 0;
		const bool readSuccess = stream->read(shaderFileContents.begin(), stream->size()) == (uint32)stream->size();

		delete stream;

		if (!readSuccess) {
			warning("LibRetroPipeline::loadPasses: Could not read file '%s'", fileName.toString().c_str());
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

		if (!shader->loadFromStringsArray(fileName.toString(),
				 ARRAYSIZE(vertexSources), vertexSources,
				 ARRAYSIZE(fragmentSources), fragmentSources,
				 g_libretroShaderAttributes)) {
			delete shader;
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
			Texture *const texture = _passes[passId - 1].target->getTexture();
			texture->enableLinearFiltering(i->filteringMode == LibRetro::kFilteringModeLinear);
			texture->setWrapMode(i->wrapMode);
			pass.inputTexture = texture;
		}

		if (pass.prevCount > maxPrevCount) {
			maxPrevCount = pass.prevCount;
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

	// Create enough FBO for previous frames and current image
	// All textures are created and destroyed at this moment
	// FBOs are created on demand and destroyed here
	_isAnimated |= (maxPrevCount > 0);

	_inputTargets.resize(maxPrevCount + 1);

	_currentTarget = 0;
	_passes[0].inputTexture = _inputTargets[_currentTarget].getTexture();

	// Now try to setup FBOs with some dummy size to make sure it could work
	uint bakInputWidth = _inputWidth;
	uint bakInputHeight = _inputHeight;
	Common::Rect bakOutputRect = _outputRect;

	_inputWidth = 320;
	_inputHeight = 200;
	_outputRect = Common::Rect(640, 480);

	bool ret = setupFBOs();

	_inputWidth = bakInputWidth;
	_inputHeight = bakInputHeight;
	_outputRect = bakOutputRect;

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
	// Setup the input targets sizes
	for (Common::Array<LibRetroTextureTarget>::iterator it = _inputTargets.begin(); it != _inputTargets.end(); it++) {
		if (!it->setScaledSize(_inputWidth, _inputHeight, _outputRect)) {
			return false;
		}
	}

	float sourceW = _inputWidth;
	float sourceH = _inputHeight;

	const float viewportW = _outputRect.width();
	const float viewportH = _outputRect.height();

	for (PassArray::size_type i = 0; i < _passes.size(); ++i) {
		Pass &pass = _passes[i];

		// Apply scaling for current pass.
		pass.shaderPass->applyScale(sourceW, sourceH, viewportW, viewportH, &sourceW, &sourceH);

		// Resize FBO to fit the output of the pass.
		if (!pass.target->setSize((uint)sourceW, (uint)sourceH, Common::kRotationNormal)) {
			return false;
		}

		// Store draw coordinates.
		/* RetroArch draws the last pass directly on FB0 and adapts its vertex coordinates for this.
		 * We don't se we should not have to take this into account but some shaders (like metacrt) ignore
		 * the vertex coordinates while drawing and everything gets upside down.
		 * So we act like RetroArch here and flip the texture when rendering on the output pipeline. */
		if (i != _passes.size() - 1) {
			pass.vertexCoord[0] = 0;
			pass.vertexCoord[1] = 0;

			pass.vertexCoord[2] = (uint)sourceW;
			pass.vertexCoord[3] = 0;

			pass.vertexCoord[4] = 0;
			pass.vertexCoord[5] = (uint)sourceH;

			pass.vertexCoord[6] = (uint)sourceW;
			pass.vertexCoord[7] = (uint)sourceH;
		} else {
			pass.vertexCoord[0] = 0;
			pass.vertexCoord[1] = (uint)sourceH;

			pass.vertexCoord[2] = (uint)sourceW;
			pass.vertexCoord[3] = (uint)sourceH;

			pass.vertexCoord[4] = 0;
			pass.vertexCoord[5] = 0;

			pass.vertexCoord[6] = (uint)sourceW;
			pass.vertexCoord[7] = 0;
		}

		// Set projection matrix in passes's shader.
		pass.shader->setUniform("MVPMatrix", pass.target->getProjectionMatrix());
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

	// All frames always have the same sizes: we reset them all at once
	setShaderTexUniforms("Prev", shader, *_passes[0].inputTexture);
	for (uint prevId = 1; prevId <= 6; ++prevId) {
		setShaderTexUniforms(Common::String::format("Prev%u", prevId), shader, *_passes[0].inputTexture);
	}
}

void LibRetroPipeline::setShaderTexUniforms(const Common::String &prefix, Shader *shader, const Texture &texture) {
	shader->setUniform(prefix + "InputSize", Math::Vector2d(texture.getLogicalWidth(), texture.getLogicalHeight()));
	shader->setUniform(prefix + "TextureSize", Math::Vector2d(texture.getWidth(), texture.getHeight()));
}

LibRetroPipeline::LibRetroTexture LibRetroPipeline::loadTexture(const Common::Path &fileName, Common::Archive *container, Common::SearchSet &archSet) {
	Common::String baseName(fileName.baseName());
	const char *extension = nullptr;
	for (int dotPos = baseName.size() - 1; dotPos >= 0; --dotPos) {
		if (baseName[dotPos] == '.') {
			extension = baseName.c_str() + dotPos + 1;
			break;
		}
	}

	if (!extension) {
		warning("LibRetroPipeline::loadTexture: File name '%s' misses extension", fileName.toString().c_str());
		return LibRetroTexture();
	}

	for (const ImageLoader *loader = s_imageLoaders; loader->extension; ++loader) {
		if (!scumm_stricmp(loader->extension, extension)) {
			Graphics::Surface *textureData = loader->load(fileName, container, archSet);
			if (!textureData) {
				warning("LibRetroPipeline::loadTexture: Loader for '%s' could not load file '%s'", loader->extension, fileName.toString().c_str());
				return LibRetroTexture();
			}

			Texture *texture = new Texture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
			texture->setSize(textureData->w, textureData->h);
			texture->updateArea(Common::Rect(textureData->w, textureData->h), *textureData);
			return LibRetroTexture(textureData, texture);
		}
	}

	warning("LibRetroPipeline::loadTexture: No loader for file '%s' present", fileName.toString().c_str());
	return LibRetroTexture();
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
	if (addTexSampler("Prev", &sampler, TextureSampler::kTypePrev, 0)) {
		prevCount = 1;
	}
	for (uint prevId = 1; prevId <= 6; ++prevId) {
		if (addTexSampler(Common::String::format("Prev%u", prevId), &sampler, TextureSampler::kTypePrev, prevId)) {
			prevCount = prevId + 1;
		}
	}
}

bool LibRetroPipeline::Pass::addTexSampler(const Common::String &prefix, uint *unit, const TextureSampler::Type type, const uint index, const bool prefixIsId) {
	const Common::String id = prefixIsId ? prefix : (prefix + "Texture");

	/* Search in the samplers if we already have one for the texture */
	for(TextureSamplerArray::iterator it = texSamplers.begin(); it != texSamplers.end(); it++) {
		if (it->type == type && it->index == index) {
			return shader->setUniform(id, it->unit);
		}
	}

	if (!shader->setUniform(id, *unit)) {
		return false;
	}

	texSamplers.push_back(TextureSampler((*unit)++, type, index));

	return true;
}

void LibRetroPipeline::renderPass(const Pass &pass) {
	// Activate shader and framebuffer to be used for rendering.
	FakePipeline fakePipeline;

	pass.shader->use();
	// We don't need to set the projection matrix here so let's use a fake pipeline
	pass.target->activate(&fakePipeline);

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

	pass.target->deactivate();

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
			// All frames always have the same tex coordinates: we reset them all at once
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
		const Texture *texture = nullptr;

		switch (i->type) {
		case Pass::TextureSampler::kTypeTexture:
			texture = _textures[i->index].glTexture;
			break;

		case Pass::TextureSampler::kTypePass:
			texture = _passes[i->index].inputTexture;
			break;

		case Pass::TextureSampler::kTypePrev: {
			assert(i->index < _inputTargets.size() - 1);
			texture = _inputTargets[(_currentTarget - i->index - 1
					+ _inputTargets.size()) % _inputTargets.size()].getTexture();
			break;
		}
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
