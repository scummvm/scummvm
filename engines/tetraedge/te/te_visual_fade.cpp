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
#include "tetraedge/te/te_visual_fade.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

TeVisualFade::TeVisualFade() {
}

void TeVisualFade::animateBlackFade() {
	error("TODO: Implement TeVisualFade::animateBlackFade.");
}

void TeVisualFade::animateFade() {
	//debug("visual fade %p animate", this);
	_animateFadeCurveAnim.stop();
	_animateFadeCurveAnim._runTimer.pausable(false);
	_fadeCaptureSprite.setVisible(true);
	_animateFadeCurveAnim._startVal = TeColor(255, 255, 255, 255);
	_animateFadeCurveAnim._endVal = TeColor(255, 255, 255, 0);
	Common::Array<float> curve;
	curve.push_back(0.0);
	curve.push_back(1.0);
	_animateFadeCurveAnim.setCurve(curve);
	_animateFadeCurveAnim._duration = 400.0;
	_animateFadeCurveAnim._callbackObj = &_fadeCaptureSprite;
	_animateFadeCurveAnim._callbackMethod = &Te3DObject2::setColor;
	_animateFadeCurveAnim.play();
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
		_texturePtr = new Te3DTexture();
	}
	_texturePtr->create();
	// create an image the size of the window, no palette, format 6.
	Common::SharedPtr<TePalette> nullpal;
	_image.destroy();
	_image.create(1024, 768, nullpal, TeImage::RGBA8);
	_texturePtr->load(_image);
	g_engine->getRenderer()->enableTexture();
	_texturePtr->load(_image);
}


} // end namespace Tetraedge
