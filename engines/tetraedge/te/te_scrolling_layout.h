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

#ifndef TETRAEDGE_TE_TE_SCROLLING_LAYOUT_H
#define TETRAEDGE_TE_TE_SCROLLING_LAYOUT_H

#include "tetraedge/te/te_animation.h"
#include "tetraedge/te/te_curve_anim2.h"
#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_timer.h"
#include "tetraedge/te/te_signal.h"

namespace Tetraedge {

class TeScrollingLayout : public TeLayout {
public:
	TeScrollingLayout();
	virtual ~TeScrollingLayout();

	void setInertiaAnimationDuration(int duration) {
		_inertiaAnimationDuration = duration;
	}
	void setInertiaAnimationCurve(const Common::Array<float> &curve) {
		_inertiaAnimationCurve = curve;
	}
	void setAutoScrollDelay(uint val) {
		_autoScrollDelay = val;
	}
	void setAutoScrollLoop(int loop) {
		_autoScrollLoop = loop;
	}
	void setAutoScrollAnimation1Speed(float speed) {
		_autoScrollAnimation1Speed = speed;
	}
	void setAutoScrollAnimation2Speed(float speed) {
		_autoScrollAnimation2Speed = speed;
	}
	void setAutoScrollAnimation1Enabled(bool enabled) {
		_autoScrollAnimation1Enabled = enabled;
	}
	void setAutoScrollAnimation2Enabled(bool enabled) {
		_autoScrollAnimation2Enabled = enabled;
	}
	void setAutoScrollAnimation1Delay(int delay) {
		_autoScrollAnimation1Delay = delay;
	}
	void setAutoScrollAnimation2Delay(int delay) {
		_autoScrollAnimation2Delay = delay;
	}
	void setAutoScrollAnimation1Curve(const Common::Array<float> &curve) {
		_autoScrollAnimation1Curve = curve;
	}
	void setAutoScrollAnimation2Curve(const Common::Array<float> &curve) {
		_autoScrollAnimation2Curve = curve;
	}
	const TeVector3f32 &direction() const {
		return _direction;
	}
	void setDirection(const TeVector3f32 &dir) {
		_direction = dir;
	}
	void setMouseControl(bool val) {
		_mouseControl = val;
	}
	void setEnclose(bool val) {
		_enclose = val;
	}
	void setContentLayout(TeLayout *layout);
	void setSpeed(const TeVector3f32 &speed);

	bool onAutoScrollDelayTimer();
	bool onAutoScrollAnimation1DelayTimer();
	bool onAutoScrollAnimation2DelayTimer();
	bool onAutoScrollAnimation1Finished();
	bool onAutoScrollAnimation2Finished();
	bool onMouseMove(const Common::Point &pt);
	bool onSlideButtonDown();
	bool onMouseLeftUp(const Common::Point &pt);

	void playAutoScrollAnimation1();
	void playAutoScrollAnimation2();

	void resetScrollPosition();
	void playAutoScroll();
	TeVector3f32 scrollPosition();
	void setScrollPosition(const TeVector3f32 &newpos);

private:
	int _inertiaAnimationDuration;
	Common::Array<float> _inertiaAnimationCurve;
	TeCurveAnim2<TeScrollingLayout, TeVector3f32> _inertiaAnimation;

	int _autoScrollLoop;
	int _currentScrollLoopNo;

	uint _autoScrollDelay;
	TeTimer _autoScrollDelayTimer;

	float _autoScrollAnimation1Speed;
	float _autoScrollAnimation2Speed;
	bool _autoScrollAnimation1Enabled;
	bool _autoScrollAnimation2Enabled;
	int _autoScrollAnimation1Delay;
	int _autoScrollAnimation2Delay;
	TeTimer _autoScrollAnimation1Timer;
	TeTimer _autoScrollAnimation2Timer;
	Common::Array<float> _autoScrollAnimation1Curve;
	Common::Array<float> _autoScrollAnimation2Curve;
	TeCurveAnim2<TeScrollingLayout, TeVector3f32> _autoScrollAnimation1;
	TeCurveAnim2<TeScrollingLayout, TeVector3f32> _autoScrollAnimation2;

	TeVector3f32 _direction;
	TeVector3f32 _speed;
	TeTimer _scrollTimer;
	bool _mouseControl;
	bool _enclose;
	TeLayout *_contentLayout;
	TeVector3f32 _contentLayoutUserPos;

	TeVector2s32 _slideDownMousePos;
	float _mouseMoveThreshold;

	TeVector3f32 _lastMouseDownPos;
	bool _insideMouseThreshold;

	TeSignal0Param _posUpdatedSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SCROLLING_LAYOUT_H
