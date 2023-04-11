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

#ifndef TETRAEDGE_TE_TE_VISUAL_FADE_H
#define TETRAEDGE_TE_TE_VISUAL_FADE_H

#include "common/ptr.h"

#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_curve_anim2.h"
#include "tetraedge/te/te_sprite_layout.h"
#include "tetraedge/te/te_button_layout.h"

namespace Tetraedge {

class TeVisualFade : public TeSpriteLayout {
public:
	TeVisualFade();

	void animateBlackFade();
	void animateFade();
	void animateFadeWithZoom();
	void captureFrame();
	void clear() {};
	void init();

	TeSpriteLayout _fadeCaptureSprite;
	TeSpriteLayout _blackFadeSprite;
	TeButtonLayout _buttonLayout;

	bool fading() const { return _fadeCurveAnim._runTimer.running(); }
	bool blackFading() const { return _blackFadeCurveAnim._runTimer.running(); }

	TeCurveAnim2<TeSpriteLayout, TeColor> blackFadeCurveAnim() { return _blackFadeCurveAnim; }

	TeIntrusivePtr<Te3DTexture> texture() { return _texturePtr; }

private:

	TeIntrusivePtr<Te3DTexture> _texturePtr;
	TeCurveAnim2<TeSpriteLayout, TeColor> _fadeCurveAnim;
	TeCurveAnim2<TeSpriteLayout, TeColor> _blackFadeCurveAnim;
	TeCurveAnim2<TeSpriteLayout, TeVector3f32> _fadeZoomAnim;
	TeImage _image;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_VISUAL_FADE_H
