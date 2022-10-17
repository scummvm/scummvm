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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/characters_shadow.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_3d_texture.h"

namespace Tetraedge {

/*static*/
Te3DObject2 *CharactersShadow::_camTarget = nullptr;

CharactersShadow::CharactersShadow() {
}

void CharactersShadow::create(InGameScene *scene) {
	_texSize = 720;
	_camTarget = new Te3DObject2();
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->enableTexture();
	_camera = new TeCamera();
	_camera->_projectionMatrixType = 2;
	// TODO: set camera field 0x130 to 1.0?
	_camera->viewport(0, 0, _texSize, _texSize);
	Te3DTexture::unbind();
	glGenTextures(1, &_glTex);
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, _texSize, _texSize, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, nullptr);
	renderer->disableTexture();
}

void CharactersShadow::createTexture(InGameScene *scene) {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->enableTexture();
	//TeLight *light = scene->shadowLight();
	error("TODO: Implement CharactersShadow::createTexture");
}

void CharactersShadow::destroy() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->disableTexture();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &_glTex);
	if (_camera)
		_camera = nullptr;
	if (_camTarget) {
		delete _camTarget;
		_camTarget = nullptr;
	}
}

void CharactersShadow::draw(InGameScene *scene) {
	error("TODO: Implement CharactersShadow::draw");
}

} // end namespace Tetraedge
