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

#ifndef TETRAEDGE_TE_TE_ANIMATION_H
#define TETRAEDGE_TE_TE_ANIMATION_H

#include "common/array.h"
#include "tetraedge/te/te_timer.h"
#include "tetraedge/te/te_signal.h"

namespace Tetraedge {

class TeAnimation {
public:
	TeAnimation();
	virtual ~TeAnimation();

	virtual void cont();
	virtual void pause();
	virtual void stop();
	virtual void reset();
	void play() {
		cont();
	}
	virtual void update(double millis) = 0;

	void seekToStart();
	//void staticDestroy();

	static void pauseAll();
	static void resumeAll();
	static void updateAll();

	static void cleanup();

	TeSignal0Param &onStop() { return _onStopSignal; }
	TeSignal0Param &onFinished() { return _onFinishedSignal; }

	TeTimer _runTimer;
	int _repeatCount;

protected:
	bool _dontRepeat;
	TeSignal0Param _onStopSignal;
	TeSignal0Param _onFinishedSignal;

private:
	void removeThisFromAnimations();

	static Common::Array<TeAnimation *> *animations();
	static Common::Array<TeAnimation *> *_animations;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_ANIMATION_H
