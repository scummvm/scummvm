/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(WIN32) && !defined(__SYMBIAN32__)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "common/endian.h"
#include "common/file.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

#ifdef USE_OPENGL_SHADERS

#include "graphics/surface.h"
#include "graphics/pixelbuffer.h"

#include "engines/grim/actor.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/colormap.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/font.h"
#include "engines/grim/gfx_opengl_shaders.h"
#include "engines/grim/grim.h"
#include "engines/grim/material.h"
#include "engines/grim/model.h"
#include "engines/grim/primitives.h"
#include "engines/grim/set.h"
#include "engines/grim/sprite.h"

namespace Grim {

template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * 8; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

static float textured_quad[] = {
//	X   , Y   , S   , T
	0.0f, 0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
};

static float textured_quad_centered[] = {
//	 X   ,  Y   , Z   , S   , T
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
	-0.5f, +0.5f, 0.0f, 0.0f, 0.0f,
	+0.5f, +0.5f, 0.0f, 1.0f, 0.0f,
	+0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
};

static unsigned short quad_indices[] = { 0, 1, 2, 0, 2, 3};

static float zero_texVerts[] = { 0.0, 0.0 };

struct GrimVertex {
	GrimVertex(const float *verts, const float *texVerts, const float *normals) {
		memcpy(_position, verts, 3 * sizeof(float));
		memcpy(_texcoord, texVerts, 2 * sizeof(float));
		memcpy(_normal, normals, 3 * sizeof(float));
	}
	float _position[3];
	float _texcoord[2];
	float _normal[3];
};

struct TextUserData {
	Graphics::Shader * shader;
	uint32 characters;
	Color  color;
	GLuint texture;
};

struct FontUserData {
	int size;
	GLuint texture;
};

// taken from glm
Math::Matrix4 makeLookMatrix(const Math::Vector3d& pos, const Math::Vector3d& interest, const Math::Vector3d& up) {
	Math::Vector3d f = (interest - pos).getNormalized();
	Math::Vector3d u = up.getNormalized();
	Math::Vector3d s = Math::Vector3d::crossProduct(f, u).getNormalized();
	u = Math::Vector3d::crossProduct(s, f);

	Math::Matrix4 look;
	look(0,0) = s.x();
	look(1,0) = s.y();
	look(2,0) = s.z();
	look(0,1) = u.x();
	look(1,1) = u.y();
	look(2,1) = u.z();
	look(0,2) = -f.x();
	look(1,2) = -f.y();
	look(2,2) = -f.z();
	look(3,0) = -Math::Vector3d::dotProduct(s, pos);
	look(3,1) = -Math::Vector3d::dotProduct(u, pos);
	look(3,2) =  Math::Vector3d::dotProduct(f, pos);

	look.transpose();

	return look;
}

// taken from glm
Math::Matrix4 makeRotationMatrix(const Math::Angle& angle, Math::Vector3d axis) {
	float c = angle.getCosine();
	float s = angle.getSine();
	axis.normalize();
	Math::Vector3d temp = (1.f - c) * axis;
	Math::Matrix4 rotate;
	rotate(0, 0) = c + temp.x() * axis.x();
	rotate(0, 1) = 0 + temp.x() * axis.y() + s * axis.z();
	rotate(0, 2) = 0 + temp.x() * axis.z() - s * axis.y();
	rotate(0, 3) = 0;
	rotate(1, 0) = 0 + temp.y() * axis.x() - s * axis.z();
	rotate(1, 1) = c + temp.y() * axis.y();
	rotate(1, 2) = 0 + temp.y() * axis.z() + s * axis.x();
	rotate(1, 3) = 0;
	rotate(2, 0) = 0 + temp.z() * axis.x() + s * axis.y();
	rotate(2, 1) = 0 + temp.z() * axis.y() - s * axis.x();
	rotate(2, 2) = c + temp.z() * axis.z();
	rotate(2, 3) = 0;
	rotate(3, 0) = 0;
	rotate(3, 1) = 0;
	rotate(3, 2) = 0;
	rotate(3, 3) = 1;

	return rotate;
}

GfxBase *CreateGfxOpenGL() {
	return new GfxOpenGLS();
}

GfxOpenGLS::GfxOpenGLS() {
	_smushTexId = 0;
	_matrixStack.push(Math::Matrix4());
	_fov = -1.0;
	_nclip = -1;
	_fclip = -1;
	_selectedTexture = NULL;
	_emergTexture = 0;
	_maxLights = 8;
	_lights = new Light[_maxLights];
	_lightsEnabled = false;
}

GfxOpenGLS::~GfxOpenGLS() {
	delete[] _lights;
}

void GfxOpenGLS::setupZBuffer() {
}

void GfxOpenGLS::setupBigEBO() {
	// FIXME: Probably way too big...
	unsigned short quad_indices[6 * 1000];

	unsigned short start = 0;
	for (unsigned short *p = quad_indices; p < &quad_indices[6 * 1000]; p += 6) {
		p[0] = p[3] = start++;
		p[1] = start++;
		p[2] = p[4] = start++;
		p[5] = start++;
	}

	_bigQuadEBO = Graphics::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
}

void GfxOpenGLS::setupQuadEBO() {
	_quadEBO = Graphics::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
}

void GfxOpenGLS::setupTexturedQuad() {
	_smushVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(textured_quad), textured_quad, GL_STATIC_DRAW);
	_emergProgram->enableVertexAttribute("position", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	_emergProgram->enableVertexAttribute("texcoord", _smushVBO, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 2 * sizeof(float));
}

void GfxOpenGLS::setupTexturedCenteredQuad() {
	_spriteVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(textured_quad_centered), textured_quad_centered, GL_STATIC_DRAW);
}

void GfxOpenGLS::setupShaders() {
	bool isEMI = g_grim->getGameType() == GType_MONKEY4;

	static const char* commonAttributes[] = {"position", "texcoord", NULL};
	_emergProgram = Graphics::Shader::fromFiles("emerg", commonAttributes);

	setupBigEBO();
	setupQuadEBO();
	setupTexturedQuad();
	setupTexturedCenteredQuad();
}

byte *GfxOpenGLS::setupScreen(int screenW, int screenH, bool fullscreen) {
	_pixelFormat = g_system->setupScreen(screenW, screenH, fullscreen, true).getFormat();
#ifndef USE_GLES2
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		error("Error: %s\n", glewGetErrorString(err));
	}
	assert(GLEW_OK == err);
#endif

	_screenWidth = screenW;
	_screenHeight = screenH;
	_scaleW = _screenWidth / (float)_gameWidth;
	_scaleH = _screenHeight / (float)_gameHeight;

	_isFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
#ifdef USE_GLES2
	g_system->setFeatureState(OSystem::kFeatureVirtControls, true);
#endif

	g_system->showMouse(false);

	char GLDriver[1024];
	sprintf(GLDriver, "ResidualVM: %s/%s with shaders", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	g_system->setWindowCaption(GLDriver);

	setupZBuffer();
	setupShaders();

	// Load emergency built-in font
	loadEmergFont();

	_screenSize = _screenWidth * _screenHeight * 4;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return NULL;
}

// matrix calculation based on the glm library.
void GfxOpenGLS::setupCamera(float fov, float nclip, float fclip, float roll) {
	if (_fov == fov && _nclip == nclip && _fclip == fclip)
		return;

	_fov = fov; _nclip = nclip; _fclip = fclip;

	float right = nclip * tan(fov / 2 * (LOCAL_PI / 180));
	float left = -right;
	float top = right * 0.75;
	float bottom = -right * 0.75;

	Math::Matrix4 proj;
	proj(0,0) = (2.0f * nclip) / (right - left);
	proj(1,1) = (2.0f * nclip) / (top - bottom);
	proj(2,0) = (right + left) / (right - left);
	proj(2,1) = (top + bottom) / (top - bottom);
	proj(2,2) = -(fclip + nclip) / (fclip - nclip);
	proj(2,3) = -1.0f;
	proj(3,2) = -(2.0f * fclip * nclip) / (fclip - nclip);
	proj(3,3) = 0.0f;

	_projMatrix = proj;
}

void GfxOpenGLS::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) {
}


void GfxOpenGLS::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GfxOpenGLS::flipBuffer() {
	g_system->updateScreen();
}


void GfxOpenGLS::getBoundingBoxPos(const Mesh *mesh, int *x1, int *y1, int *x2, int *y2) {

}

void GfxOpenGLS::getBoundingBoxPos(const EMIModel *model, int *x1, int *y1, int *x2, int *y2) {

}

void GfxOpenGLS::startActorDraw(const Actor *actor) {
}


void GfxOpenGLS::finishActorDraw() {
}

void GfxOpenGLS::setShadow(Shadow *shadow) {

}

void GfxOpenGLS::drawShadowPlanes() {

}

void GfxOpenGLS::setShadowMode() {

}

void GfxOpenGLS::clearShadowMode() {

}

bool GfxOpenGLS::isShadowModeActive() {
	return false;
}

void GfxOpenGLS::setShadowColor(byte r, byte g, byte b) {

}

void GfxOpenGLS::getShadowColor(byte *r, byte *g, byte *b) {

}


void GfxOpenGLS::set3DMode() {

}


void GfxOpenGLS::translateViewpointStart() {
	_matrixStack.push(_matrixStack.top());
}

void GfxOpenGLS::translateViewpoint(const Math::Vector3d &vec) {
	Math::Matrix4 temp;
	temp.setPosition(vec);
	temp.transpose();
	_matrixStack.top() = temp * _matrixStack.top();
}

void GfxOpenGLS::rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis_) {
	Math::Matrix4 temp = makeRotationMatrix(angle, axis_) * _matrixStack.top();
	_matrixStack.top() = temp;
}

void GfxOpenGLS::translateViewpointFinish() {
	_matrixStack.pop();
}

void GfxOpenGLS::updateEMIModel(const EMIModel* model) {
}

void GfxOpenGLS::drawEMIModelFace(const EMIModel* model, const EMIMeshFace* face) {
}

void GfxOpenGLS::drawMesh(const Mesh *mesh) {
}

void GfxOpenGLS::drawModelFace(const Mesh *mesh, const MeshFace *face) {

}

void GfxOpenGLS::drawSprite(const Sprite *sprite) {
}


void GfxOpenGLS::enableLights() {
	_lightsEnabled = true;
}

void GfxOpenGLS::disableLights() {
	_lightsEnabled = false;
}

void GfxOpenGLS::setupLight(Grim::Light *light, int lightId) {
	_lightsEnabled = true;

	if (lightId >= _maxLights) {
		return;
	}

	// Disable previous lights.
	if (lightId == 0) {
		for (uint32 id = 0; id < _maxLights; ++id)
			_lights[id]._color.w() = 0.0;
	}

	Math::Vector4d &lightColor = _lights[lightId]._color;
	Math::Vector4d &lightPos   = _lights[lightId]._position;
	Math::Vector4d &lightDir   = _lights[lightId]._direction;

	float intensity = light->_intensity / 1.3f;
	lightColor.x() = ((float)light->_color.getRed() / 15.0f) * intensity;
	lightColor.y() = ((float)light->_color.getGreen() / 15.0f) * intensity;
	lightColor.z() = ((float)light->_color.getBlue() / 15.0f) * intensity;
	lightColor.w() = 1.0f;

	if (light->_type == Grim::Light::Omni) {
		lightPos = Math::Vector4d(light->_pos.x(), light->_pos.y(), light->_pos.z(), 1.0f);
		lightDir = Math::Vector4d(0.0f, 0.0f, 0.0f, -1.0f);
	} else if (light->_type == Grim::Light::Direct) {
		lightPos = Math::Vector4d(-light->_dir.x(), -light->_dir.y(), -light->_dir.z(), 0.0f);
		lightDir = Math::Vector4d(0.0f, 0.0f, 0.0f, -1.0f);
	} else if (light->_type == Grim::Light::Spot) {
		lightPos = Math::Vector4d(light->_pos.x(), light->_pos.y(), light->_pos.z(), 1.0f);
		lightDir = Math::Vector4d(light->_dir.x(), light->_dir.y(), light->_dir.z(), Math::Angle(light->_penumbraangle).getCosine());
	}
}

void GfxOpenGLS::turnOffLight(int lightId) {
	if (lightId >= _maxLights) {
		return;
	}

	_lights[lightId]._color = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.0f);
	_lights[lightId]._position = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.0f);
	_lights[lightId]._direction = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.0f);
}


void GfxOpenGLS::createMaterial(Texture *material, const char *data, const CMap *cmap) {
	material->_texture = new GLuint[1];
	glGenTextures(1, (GLuint *)material->_texture);
	char *texdata = new char[material->_width * material->_height * 4];
	char *texdatapos = texdata;

	if (cmap != NULL) { // EMI doesn't have colour-maps
		for (int y = 0; y < material->_height; y++) {
			for (int x = 0; x < material->_width; x++) {
				uint8 col = *(const uint8 *)(data);
				if (col == 0) {
					memset(texdatapos, 0, 4); // transparent
					if (!material->_hasAlpha) {
						texdatapos[3] = '\xff'; // fully opaque
					}
				} else {
					memcpy(texdatapos, cmap->_colors + 3 * (col), 3);
					texdatapos[3] = '\xff'; // fully opaque
				}
				texdatapos += 4;
				data++;
			}
		}
	} else {
		memcpy(texdata, data, material->_width * material->_height * material->_bpp);
	}

	GLuint format = 0;
	GLuint internalFormat = 0;
	if (material->_colorFormat == BM_RGBA) {
		format = GL_RGBA;
		internalFormat = GL_RGBA;
	} else if (material->_colorFormat == BM_BGRA) {
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	} else {	// The only other colorFormat we load right now is BGR
#ifdef USE_GLES2
		format = GL_RGB;
		internalFormat = GL_RGB;
#else
		format = GL_BGR;
		internalFormat = GL_RGB;
#endif
	}

	GLuint *textures = (GLuint *)material->_texture;
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, material->_width, material->_height, 0, format, GL_UNSIGNED_BYTE, texdata);
	delete[] texdata;
}

void GfxOpenGLS::selectMaterial(const Texture *material) {
	GLuint *textures = (GLuint *)material->_texture;
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	if (material->_hasAlpha && g_grim->getGameType() == GType_MONKEY4) {
		glEnable(GL_BLEND);
	}

	_selectedTexture = const_cast<Texture *>(material);
}

void GfxOpenGLS::destroyMaterial(Texture *material) {

}

void GfxOpenGLS::createBitmap(BitmapData *bitmap) {
}


void GfxOpenGLS::drawBitmap(const Bitmap *bitmap, int dx, int dy, uint32 layer) {
}

void GfxOpenGLS::drawDepthBitmap(int x, int y, int w, int h, char *data) {
}

void GfxOpenGLS::destroyBitmap(BitmapData *bitmap) {
}

void GfxOpenGLS::createFont(Font *font) {
}

void GfxOpenGLS::destroyFont(Font *font) {
}

void GfxOpenGLS::createTextObject(TextObject *text) {
}

void GfxOpenGLS::drawTextObject(const TextObject *text) {
}

void GfxOpenGLS::destroyTextObject(TextObject *text) {
}


Bitmap *GfxOpenGLS::getScreenshot(int w, int h) {
}

void GfxOpenGLS::storeDisplay() {

}

void GfxOpenGLS::copyStoredToDisplay() {

}


void GfxOpenGLS::dimScreen() {

}

void GfxOpenGLS::dimRegion(int x, int y, int w, int h, float level) {

}


void GfxOpenGLS::irisAroundRegion(int x1, int y1, int x2, int y2) {

}


void GfxOpenGLS::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
	if (!*text)
		return;
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, _emergTexture);
	_emergProgram->use();
	Math::Vector3d colors(float(fgColor.getRed()) / 255.0f,
	                      float(fgColor.getGreen()) / 255.0f,
	                      float(fgColor.getBlue()) / 255.0f);
	_emergProgram->setUniform("color", colors);
	_emergProgram->setUniform("sizeWH", Math::Vector2d(float(8) / _gameWidth, float(16) / _gameHeight));
	_emergProgram->setUniform("texScale", Math::Vector2d(float(8) / 128, float(16) / 128));

	for (; *text; ++text, x+=10) {
		int blockcol = *text & 0xf;
		int blockrow = *text / 16;
		_emergProgram->setUniform("offsetXY", Math::Vector2d(float(x) / _gameWidth, float(y) / _gameHeight));
		_emergProgram->setUniform("texOffsetXY", Math::Vector2d(float(blockcol * 8) / 128, float(blockrow * 16) / 128));
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
}

void GfxOpenGLS::loadEmergFont() {
	uint8 *atlas = new uint8[128 * 128];
	memset(atlas, 0, 128 * 128);

	for (int c = 32; c < 128; ++c) {
		int blockrow = c / 16;
		int blockcol = c & 0xf;
		for (int row = 0; row < 13; ++row) {
			int base = 128 * (16 * blockrow + row) + 8 * blockcol;
			uint8 val = Font::emerFont[c-32][row];
			atlas[base+0] = (val & 0x80) ? 255 : 0;
			atlas[base+1] = (val & 0x40) ? 255 : 0;
			atlas[base+2] = (val & 0x20) ? 255 : 0;
			atlas[base+3] = (val & 0x10) ? 255 : 0;
			atlas[base+4] = (val & 0x08) ? 255 : 0;
			atlas[base+5] = (val & 0x04) ? 255 : 0;
			atlas[base+6] = (val & 0x02) ? 255 : 0;
			atlas[base+7] = (val & 0x01) ? 255 : 0;
		}
	}

	glGenTextures(1, &_emergTexture);
	glBindTexture(GL_TEXTURE_2D, _emergTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 128, 128, 0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas);

	delete[] atlas;
}


void GfxOpenGLS::drawRectangle(const PrimitiveObject *primitive) {

}

void GfxOpenGLS::drawLine(const PrimitiveObject *primitive) {

}

void GfxOpenGLS::drawPolygon(const PrimitiveObject *primitive) {

}


void GfxOpenGLS::prepareMovieFrame(Graphics::Surface* frame) {
}

void GfxOpenGLS::drawMovieFrame(int offsetX, int offsetY) {
}


void GfxOpenGLS::releaseMovieFrame() {
}


const char *GfxOpenGLS::getVideoDeviceName() {
	return "OpenGLS Renderer";
}

void GfxOpenGLS::renderBitmaps(bool render) {

}

void GfxOpenGLS::renderZBitmaps(bool render) {

}


void GfxOpenGLS::createSpecialtyTextures() {
}

void GfxOpenGLS::createEMIModel(EMIModel *model) {
}

void GfxOpenGLS::createModel(Mesh *mesh) {
}


}

#endif
