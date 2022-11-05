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

#include "tetraedge/te/te_layout.h"

namespace Tetraedge {

class TeScrollingLayout : public TeLayout {
public:
	TeScrollingLayout();

	void setInertiaAnimationDuration(int duration) {
		_inertiaAnimationDuration = duration;
	}
	void setInertiaAnimationCurve(const Common::Array<float> &curve) {
		_inertiaAnimationCurve = curve;
	}
	void setAutoScrollDelay(unsigned int val) {
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

private:
	int _inertiaAnimationDuration;
	Common::Array<float> _inertiaAnimationCurve;
	uint _autoScrollDelay;
	int _autoScrollLoop;
	float _autoScrollAnimation1Speed;
	float _autoScrollAnimation2Speed;
	bool _autoScrollAnimation1Enabled;
	bool _autoScrollAnimation2Enabled;
	int _autoScrollAnimation1Delay;
	int _autoScrollAnimation2Delay;
	Common::Array<float> _autoScrollAnimation1Curve;
	Common::Array<float> _autoScrollAnimation2Curve;
	TeVector3f32 _direction;
	bool _mouseControl;
	bool _enclose;
	TeLayout *_contentLayout;
	TeVector3f32 _contentLayoutUserPos;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SCROLLING_LAYOUT_H
