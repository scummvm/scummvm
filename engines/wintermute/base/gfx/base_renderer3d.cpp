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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/wintermute/base/gfx/base_renderer3d.h"
#include "engines/wintermute/base/base_game.h"

#include "math/glmath.h"

namespace Wintermute {

BaseRenderer3D::BaseRenderer3D(Wintermute::BaseGame *inGame) : BaseRenderer(inGame) {
	_camera = nullptr;

	_state = RSTATE_NONE;
	_fov = (float)M_PI / 4;

	_nearClipPlane = DEFAULT_NEAR_PLANE;
	_farClipPlane = DEFAULT_FAR_PLANE;

	_spriteBatchMode = false;

	_ambientLightColor = 0x00000000;
	_ambientLightOverride = false;
}

BaseRenderer3D::~BaseRenderer3D() {
	_camera = nullptr; // ref only
}

void BaseRenderer3D::initLoop() {
	BaseRenderer::initLoop();
	setup2D();
}

bool BaseRenderer3D::drawSprite(BaseSurfaceOpenGL3D &tex, const Wintermute::Rect32 &rect,
							float zoomX, float zoomY, const Wintermute::Vector2 &pos,
							uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
							bool mirrorX, bool mirrorY) {
	Vector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	return drawSpriteEx(tex, rect, pos, Vector2(0.0f, 0.0f), scale, 0.0f, color, alphaDisable, blendMode, mirrorX, mirrorY);
}

bool BaseRenderer3D::getProjectionParams(float *resWidth, float *resHeight, float *layerWidth, float *layerHeight,
										 float *modWidth, float *modHeight, bool *customViewport) {
	*resWidth = _width;
	*resHeight = _height;

	if (_gameRef->_editorResolutionWidth > 0)
		*resWidth = _gameRef->_editorResolutionWidth;
	if (_gameRef->_editorResolutionHeight > 0)
		*resHeight = _gameRef->_editorResolutionHeight;

	int lWidth, lHeight;
	Rect32 sceneViewport;
	_gameRef->getLayerSize(&lWidth, &lHeight, &sceneViewport, customViewport);
	*layerWidth = (float)lWidth;
	*layerHeight = (float)lHeight;

	*modWidth = 0.0f;
	*modHeight = 0.0f;
	if (*layerWidth > *resWidth)
		*modWidth  = (*layerWidth - *resWidth) / 2.0f;
	if (*layerHeight > *resHeight)
		*modHeight = (*layerHeight - *resHeight) / 2.0f;

	// new in 1.7.2.1
	// if layer height is smaller than resolution, we assume that we don't want to scroll
	// and that the camera overviews the entire resolution
	if (*layerHeight < *resHeight) {
		*modHeight -= (*resHeight - *layerHeight) / 2;
		*layerHeight = *resHeight;
	}

	return true;
}

void BaseRenderer3D::fade(uint16 alpha) {
	fadeToColor(0, 0, 0, (byte)(255 - alpha));
}

bool BaseRenderer3D::setAmbientLightColor(uint32 color) {
	_ambientLightColor = color;
	_ambientLightOverride = true;

	setAmbientLightRenderState();
	return true;
}

bool BaseRenderer3D::setDefaultAmbientLightColor() {
	_ambientLightColor = 0x00000000;
	_ambientLightOverride = false;

	setAmbientLightRenderState();
	return true;
}

Rect32 BaseRenderer3D::getViewPort() {
	return _viewportRect;
}

Graphics::PixelFormat BaseRenderer3D::getPixelFormat() const {
	return g_system->getScreenFormat();
}

Math::Matrix3 BaseRenderer3D::build2dTransformation(const Vector2 &center, float angle) {
	Math::Matrix3 translateCenter;
	translateCenter.setToIdentity();
	translateCenter(0, 2) = -center.x;
	translateCenter(1, 2) = -center.y;

	float sinOfAngle = Math::Angle(angle).getSine();
	float cosOfAngle = Math::Angle(angle).getCosine();

	Math::Matrix3 rotation;
	rotation.setToIdentity();
	rotation(0, 0) = cosOfAngle;
	rotation(0, 1) = sinOfAngle;
	rotation(1, 0) = -sinOfAngle;
	rotation(1, 1) = cosOfAngle;

	Math::Matrix3 translateCenterBack;
	translateCenterBack.setToIdentity();
	translateCenterBack(0, 2) = center.x;
	translateCenterBack(1, 2) = center.y;

	return translateCenterBack * rotation * translateCenter;
}

void BaseRenderer3D::flipVertical(Graphics::Surface *s) {
	for (int y = 0; y < s->h / 2; ++y) {
		// Flip the lines
		byte *line1P = (byte *)s->getBasePtr(0, y);
		byte *line2P = (byte *)s->getBasePtr(0, s->h - y - 1);

		for (int x = 0; x < s->pitch; ++x)
			SWAP(line1P[x], line2P[x]);
	}
}

} // namespace Wintermute
