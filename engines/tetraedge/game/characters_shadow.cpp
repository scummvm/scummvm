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
#include "tetraedge/game/character.h"
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
	_camera->_somePerspectiveVal = 1.0;
	_camera->setName("_shadowCam");
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
	TeLight *light = scene->shadowLight();
	if (light) {
		TeQuaternion q1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), light->positionRadial().getX() - M_PI_2);
		TeQuaternion q2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(1, 0, 0), light->positionRadial().getY());
		_camera->setRotation(q2 * q1);
		_camera->setPosition(light->position3d());
	}
	_camera->_fov = scene->shadowFov() * M_PI / 180.0;
	_camera->_orthNearVal = scene->shadowNearPlane();
	_camera->_orthFarVal = scene->shadowFarPlane();
	_camera->apply();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (Character *character : scene->_characters) {
		character->_model->draw();
	}
	scene->_character->_model->draw();
	Te3DTexture::unbind();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _texSize, _texSize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	TeCamera::restore();
	TeCamera::restore();
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
	TeRenderer *renderer = g_engine->getRenderer();
	glDepthMask(false);
	renderer->disableZBuffer();
	renderer->enableTexture();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	Te3DTexture::unbind();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glEnable(GL_BLEND);
	renderer->setCurrentColor(scene->shadowColor());

	TeMatrix4x4 matrix;
	matrix.translate(TeVector3f32(0.5f, 0.5f, 0.5f));
	matrix.scale(TeVector3f32(0.5f, 0.5f, 0.5f));
	matrix = matrix * _camera->projectionMatrix();

	TeMatrix4x4 cammatrix = _camera->worldTransformationMatrix();
	cammatrix.inverse();

	matrix = matrix * cammatrix;

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	float f[4];
	for (unsigned int i = 0; i < 4; i++)
		f[i] = matrix(i, 0);

	glTexGenfv(GL_S, GL_EYE_PLANE, f);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	for (unsigned int i = 0; i < 4; i++)
		f[i] = matrix(i, 1);

	glTexGenfv(GL_T, GL_EYE_PLANE, f);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	for (unsigned int i = 0; i < 4; i++)
		f[i] = matrix(i, 2);

	glTexGenfv(GL_R, GL_EYE_PLANE, f);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	for (unsigned int i = 0; i < 4; i++)
		f[i] = matrix(i, 3);

	glTexGenfv(GL_Q, GL_EYE_PLANE, f);

	Te3DTexture::unbind();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glEnable(GL_BLEND);
	renderer->setCurrentColor(scene->shadowColor());

	for (TeIntrusivePtr<TeModel> model : scene->zoneModels()) {
		if (model->_meshes.size() > 0 && model->_meshes[0].materials().empty()) {
			model->_meshes[0].defaultMaterial(TeIntrusivePtr<Te3DTexture>());
			model->_meshes[0].materials()[0]._enableSomethingDefault0 = true;
			model->_meshes[0].materials()[0]._diffuseColor = scene->shadowColor();
		}
		model->draw();
	}

	renderer->disableTexture();
	glDepthMask(true);
	renderer->enableZBuffer();
}

} // end namespace Tetraedge
