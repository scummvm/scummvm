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

#include "graphics/tinygl/tinygl.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/characters_shadow_tinygl.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_3d_texture_tinygl.h"

namespace Tetraedge {

void CharactersShadowTinyGL::createInternal() {
	Te3DTextureTinyGL::unbind();
	tglGenTextures(1, &_glTex);
	tglBindTexture(TGL_TEXTURE_2D, _glTex);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_CLAMP);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_CLAMP);
	// TODO: not supported in TGL
	//tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_LUMINANCE_ALPHA, _texSize, _texSize, 0, TGL_LUMINANCE_ALPHA, TGL_UNSIGNED_BYTE, nullptr);
}

void CharactersShadowTinyGL::createTextureInternal(InGameScene *scene) {
	TeRenderer *renderer = g_engine->getRenderer();
	tglClearColor(0.0, 0.0, 0.0, 0.0);
	renderer->clearBuffer(TeRenderer::ColorAndDepth);

	for (Character *character : scene->_characters) {
		character->_model->draw();
	}
	scene->_character->_model->draw();
	Te3DTextureTinyGL::unbind();
	tglBindTexture(TGL_TEXTURE_2D, _glTex);
	// TODO: Find TGL equivalent for this..
	// tglCopyTexSubImage2D(TGL_TEXTURE_2D, 0, 0, 0, 0, 0, _texSize, _texSize);
	renderer->clearBuffer(TeRenderer::ColorAndDepth);
}

void CharactersShadowTinyGL::deleteTexture() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->disableTexture();
	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDeleteTextures(1, &_glTex);
}

void CharactersShadowTinyGL::draw(InGameScene *scene) {
	TeRenderer *renderer = g_engine->getRenderer();
	tglDepthMask(false);
	renderer->disableZBuffer();
	renderer->enableTexture();
	tglBindTexture(TGL_TEXTURE_2D, _glTex);
	Te3DTextureTinyGL::unbind();
	tglBindTexture(TGL_TEXTURE_2D, _glTex);
	tglEnable(TGL_BLEND);
	renderer->setCurrentColor(scene->shadowColor());

	TeMatrix4x4 matrix;
	matrix.translate(TeVector3f32(0.5f, 0.5f, 0.5f));
	matrix.scale(TeVector3f32(0.5f, 0.5f, 0.5f));
	matrix = matrix * _camera->projectionMatrix();

	TeMatrix4x4 cammatrix = _camera->worldTransformationMatrix();
	cammatrix.inverse();

	matrix = matrix * cammatrix;

	/* TODO: Find TGL equivalents for the following block. */
	/*
	tglTexGeni(TGL_S, TGL_TEXTURE_GEN_MODE, TGL_EYE_LINEAR);

	float f[4];
	for (uint i = 0; i < 4; i++)
		f[i] = matrix(0, i);

	tglTexGenfv(TGL_S, TGL_EYE_PLANE, f);
	tglTexGeni(TGL_T, TGL_TEXTURE_GEN_MODE, TGL_EYE_LINEAR);

	for (uint i = 0; i < 4; i++)
		f[i] = matrix(1, i);

	tglTexGenfv(TGL_T, TGL_EYE_PLANE, f);
	tglTexGeni(TGL_R, TGL_TEXTURE_GEN_MODE, TGL_EYE_LINEAR);

	for (uint i = 0; i < 4; i++)
		f[i] = matrix(2, i);

	tglTexGenfv(TGL_R, TGL_EYE_PLANE, f);
	tglTexGeni(TGL_Q, TGL_TEXTURE_GEN_MODE, TGL_EYE_LINEAR);

	for (uint i = 0; i < 4; i++)
		f[i] = matrix(3, i);

	tglTexGenfv(TGL_Q, TGL_EYE_PLANE, f);
	*/

	Te3DTextureTinyGL::unbind();
	tglBindTexture(TGL_TEXTURE_2D, _glTex);
	tglEnable(TGL_BLEND);
	renderer->setCurrentColor(scene->shadowColor());

	Common::Array<TeIntrusivePtr<TeModel>> &models =
			(g_engine->gameType() == TetraedgeEngine::kSyberia ?
					scene->zoneModels() : scene->shadowReceivingObjects());
	for (TeIntrusivePtr<TeModel> model : models) {
		if (model->meshes().size() > 0 && model->meshes()[0]->materials().empty()) {
			model->meshes()[0]->defaultMaterial(TeIntrusivePtr<Te3DTexture>());
			model->meshes()[0]->materials()[0]._isShadowTexture = true;
			model->meshes()[0]->materials()[0]._diffuseColor = scene->shadowColor();
		}
		model->draw();
	}

	renderer->disableTexture();
	tglDepthMask(true);
	renderer->enableZBuffer();
}

} // end namespace Tetraedge
