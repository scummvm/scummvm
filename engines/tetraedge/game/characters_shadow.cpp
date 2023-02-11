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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/characters_shadow.h"
#include "tetraedge/game/characters_shadow_opengl.h"
#include "tetraedge/game/characters_shadow_tinygl.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_3d_texture.h"


namespace Tetraedge {

/*static*/
Te3DObject2 *CharactersShadow::_camTarget = nullptr;

CharactersShadow::CharactersShadow() : _glTex(0), _texSize(0) {
}

void CharactersShadow::create(InGameScene *scene) {
	_texSize = 720;
	_camTarget = new Te3DObject2();
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->enableTexture();
	_camera = new TeCamera();
	_camera->setProjMatrixType(2);
	_camera->setAspectRatio(1.0);
	_camera->setName("_shadowCam");
	_camera->viewport(0, 0, _texSize, _texSize);

	createInternal();

	renderer->disableTexture();
}

void CharactersShadow::createTexture(InGameScene *scene) {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->enableTexture();
	TeLight *light = scene->shadowLight();
	if (light) {
		const TeQuaternion q1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), light->positionRadial().getX() - M_PI_2);
		const TeQuaternion q2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(1, 0, 0), light->positionRadial().getY());
		_camera->setRotation(q2 * q1);
		_camera->setPosition(light->position3d());
	}
	_camera->setFov((float)(scene->shadowFov() * M_PI / 180.0));
	_camera->setOrthoPlanes(scene->shadowNearPlane(), scene->shadowFarPlane());
	_camera->apply();

	createTextureInternal(scene);

	TeCamera::restore();
	TeCamera::restore();
}

void CharactersShadow::destroy() {
	deleteTexture();
	if (_camera)
		_camera = nullptr;
	if (_camTarget) {
		delete _camTarget;
		_camTarget = nullptr;
	}
}

/*static*/
CharactersShadow *CharactersShadow::makeInstance() {
	Graphics::RendererType r = g_engine->preferredRendererType();

#if defined(USE_OPENGL_GAME)
	if (r == Graphics::kRendererTypeOpenGL)
		return new CharactersShadowOpenGL();
#endif

#if defined(USE_TINYGL)
	if (r == Graphics::kRendererTypeTinyGL)
		return new CharactersShadowTinyGL();
#endif
	error("Couldn't create CharactersShadow for selected renderer");
}

} // end namespace Tetraedge
