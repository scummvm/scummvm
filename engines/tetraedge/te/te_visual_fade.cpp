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

#include "common/textconsole.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/te/te_visual_fade.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

TeVisualFade::TeVisualFade() {
}

void TeVisualFade::animateBlackFade() {
	_fadeCaptureSprite.setVisible(false);
	_fadeCurveAnim.stop();
	_fadeCurveAnim._startVal = TeColor(255, 255, 255, 0);
	_fadeCurveAnim._endVal = TeColor(255, 255, 255, 255);
	Common::Array<float> curve;
	curve.push_back(0.0);
	curve.push_back(0.0);
	curve.push_back(1.0);
	curve.push_back(1.0);
	curve.push_back(1.0);
	_fadeCurveAnim.setCurve(curve);
	_fadeCurveAnim._duration = 2000.0;
	_fadeCurveAnim._callbackObj = &_fadeCaptureSprite;
	_fadeCurveAnim._callbackMethod = &TeSpriteLayout::setColor;
	_fadeCurveAnim.play();

	_blackFadeSprite.setVisible(true);
	_blackFadeCurveAnim.stop();
	_blackFadeCurveAnim._startVal = TeColor(255, 255, 255, 255);
	_blackFadeCurveAnim._endVal = TeColor(255, 255, 255, 0);
	_blackFadeCurveAnim.setCurve(curve);
	_blackFadeCurveAnim._duration = 2000.0;
	_blackFadeCurveAnim._callbackObj = &_blackFadeSprite;
	_blackFadeCurveAnim._callbackMethod = &TeSpriteLayout::setColor;
	_blackFadeCurveAnim.play();
}

void TeVisualFade::animateFade() {
	double duration = g_engine->gameIsAmerzone() ? 1000 : 400;
	_fadeCurveAnim.stop();
	_fadeCurveAnim._runTimer.pausable(false);
	_fadeCaptureSprite.setVisible(true);
	_fadeCurveAnim._startVal = TeColor(255, 255, 255, 255);
	_fadeCurveAnim._endVal = TeColor(255, 255, 255, 0);
	Common::Array<float> curve;
	curve.push_back(0.0);
	curve.push_back(1.0);
	_fadeCurveAnim.setCurve(curve);
	_fadeCurveAnim._duration = duration;
	_fadeCurveAnim._callbackObj = &_fadeCaptureSprite;
	_fadeCurveAnim._callbackMethod = &TeSpriteLayout::setColor;
	_fadeCurveAnim.play();
}

void TeVisualFade::animateFadeWithZoom() {
	animateFade();
	_fadeZoomAnim.stop();
	_fadeZoomAnim._runTimer.pausable(false);
	_fadeZoomAnim._startVal = TeVector3f32(1, 1, 1);
	_fadeZoomAnim._endVal = TeVector3f32(3, 3, 3);
	Common::Array<float> curve;
	curve.push_back(0.0);
	curve.push_back(0.33f);
	curve.push_back(1.0);
	curve.push_back(1.0);
	_fadeZoomAnim.setCurve(curve);
	_fadeZoomAnim._duration = 1000;
	_fadeZoomAnim._callbackObj = &_fadeCaptureSprite;
	_fadeZoomAnim._callbackMethod = &TeSpriteLayout::setScale;
	_fadeZoomAnim.play();
}

void TeVisualFade::captureFrame() {
	//debug("visual fade %p capture", this);
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->enableTexture();
	_texturePtr->copyCurrentRender(0, 0, 0, 0);
	_fadeCaptureSprite.load(_texturePtr);
	renderer->disableTexture();
}

void TeVisualFade::init() {
	//debug("visual fade %p init", this);
	_fadeCaptureSprite.setName("fadeCaptureSprite");
	_fadeCaptureSprite.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_fadeCaptureSprite.setSize(TeVector3f32(1.0, 1.0, 0.0));
	_fadeCaptureSprite.setColor(TeColor(255, 255, 255, 0));
	_fadeCaptureSprite.setVisible(false);
	_fadeCaptureSprite.unload();

	_blackFadeSprite.setName("blackFadeSprite");
	_blackFadeSprite.setSizeType(TeLayout::CoordinatesType::RELATIVE_TO_PARENT);
	_blackFadeSprite.setSize(TeVector3f32(2.0, 2.0, 0.0));
	_blackFadeSprite.load("pictures/black64x64.png");
	_blackFadeSprite.setColor(TeColor(255, 255, 255, 0));
	_blackFadeSprite.setVisible(false);

	if (_texturePtr) {
		_texturePtr->destroy();
	} else {
		_texturePtr = Te3DTexture::makeInstance();
	}
	_texturePtr->create();
	// create an image the size of the window, no palette, format 6.
	Common::SharedPtr<TePalette> nullpal;
	_image.destroy();
	const TeVector3f32 winSize = g_engine->getApplication()->getMainWindow().size();
	_image.createImg((int)winSize.x(), (int)winSize.y(), nullpal, TeImage::RGBA8);
	_texturePtr->load(_image);
	g_engine->getRenderer()->enableTexture();
	_texturePtr->load(_image);
}


} // end namespace Tetraedge
