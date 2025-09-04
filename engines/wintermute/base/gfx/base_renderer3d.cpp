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

#include "common/config-manager.h"

namespace Wintermute {

BaseRenderer3D::BaseRenderer3D(Wintermute::BaseGame *inGame) : BaseRenderer(inGame) {
	_camera = nullptr;

	_state = RSTATE_NONE;
	_fov = (float)M_PI / 4;

	_nearClipPlane = DEFAULT_NEAR_PLANE;
	_farClipPlane = DEFAULT_FAR_PLANE;

	_lastTexture = nullptr;

	_ambientLightColor = 0x00000000;
	_ambientLightOverride = false;

	DXMatrixIdentity(&_worldMatrix);
	DXMatrixIdentity(&_viewMatrix);
	DXMatrixIdentity(&_projectionMatrix);
}

BaseRenderer3D::~BaseRenderer3D() {
	_camera = nullptr; // ref only
}

void BaseRenderer3D::initLoop() {
	BaseRenderer::initLoop();
	setup2D();
}

bool BaseRenderer3D::drawSprite(BaseSurface *texture, const Common::Rect32 &rect,
	                        float zoomX, float zoomY, const DXVector2 &pos,
	                        uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
	                        bool mirrorX, bool mirrorY) {
	DXVector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	return drawSpriteEx(texture, rect, pos, DXVector2(0.0f, 0.0f), scale, 0.0f, color, alphaDisable, blendMode, mirrorX, mirrorY);
}

bool BaseRenderer3D::getProjectionParams(float *resWidth, float *resHeight, float *layerWidth, float *layerHeight,
	                                 float *modWidth, float *modHeight, bool *customViewport) {
	*resWidth = _width;
	*resHeight = _height;

	if (_game->_editorResolutionWidth > 0)
		*resWidth = _game->_editorResolutionWidth;
	if (_game->_editorResolutionHeight > 0)
		*resHeight = _game->_editorResolutionHeight;

	int lWidth, lHeight;
	Common::Rect32 sceneViewport;
	_game->getLayerSize(&lWidth, &lHeight, &sceneViewport, customViewport);
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

bool BaseRenderer3D::setup3DCustom(DXMatrix &viewMat, DXMatrix &projMat) {
	setup3D();
	_state = RSTATE_3D;
	if (viewMat)
		setViewTransform(viewMat);
	if (projMat)
		setProjectionTransform(projMat);

	return true;
}

DXViewport BaseRenderer3D::getViewPort() {
	return _viewport;
}

Graphics::PixelFormat BaseRenderer3D::getPixelFormat() const {
	return Graphics::PixelFormat::createFormatRGBA32();
}

bool BaseRenderer3D::flip() {
	_lastTexture = nullptr;
	g_system->updateScreen();
	return true;
}

bool BaseRenderer3D::indicatorFlip(int32 x, int32 y, int32 width, int32 height) {
	flip();
	return true;
}

bool BaseRenderer3D::forcedFlip() {
	flip();
	return true;
}

bool BaseRenderer3D::windowedBlt() {
	flip();
	return true;
}

void BaseRenderer3D::onWindowChange() {
	_windowed = !g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
}

void BaseRenderer3D::setWindowed(bool windowed) {
	ConfMan.setBool("fullscreen", !windowed);
	g_system->beginGFXTransaction();
	g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !windowed);
	g_system->endGFXTransaction();
}

} // namespace Wintermute
