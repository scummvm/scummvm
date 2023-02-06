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

#ifndef TETRAEDGE_TE_TE_CURVE_ANIM2_H
#define TETRAEDGE_TE_TE_CURVE_ANIM2_H

#include "tetraedge/te/te_animation.h"
#include "tetraedge/te/te_interpolation.h"

namespace Tetraedge {

template<class T> static T linearInterpolation(T &obj1, T &obj2, double amount) {
	amount = CLIP<double>(amount, 0.0, 1.0);
	return (obj1 * (1.0 - amount)) + (obj2 * amount);
}

template<class T, class S>
class TeCurveAnim2 : public TeAnimation {
public:
	typedef void(T::*TMethod)(const S &);

	TeCurveAnim2() : _callbackObj(nullptr), _callbackMethod(nullptr), _duration(0), _lastUpdateTime(0) {}
	virtual ~TeCurveAnim2() {}

	void setCurve(const Common::Array<float> &curve) {
		// The original writes the curve to a stream to load it back in in
		// the interpolation.. we just skip that with a direct array copy.
		_interp.load(curve);
	}

	void update(double millis) {
		_lastUpdateTime = millis;

		double amount = _interp.interpole(millis, _duration);

		const S interpVal = linearInterpolation<S>(_startVal, _endVal, amount);
		//debug("CurveAnim %.02f/%.02f (%.02f) -> %s", time, _maxTime, amount, interpVal.toString().c_str());
		(_callbackObj->*_callbackMethod)(interpVal);
		if (_lastUpdateTime >= _duration) {
			if (_repeatCount == -1) {
				seekToStart();
			} else {
				stop();
				onFinished().call();
			}
		}
	}

	S _startVal;
	S _endVal;
	T *_callbackObj;
	TMethod _callbackMethod;
	double _duration;

private:
	TeInterpolation _interp;
	double _lastUpdateTime;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_CURVE_ANIM2_H
