/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-2013 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_SUBCHASE_H
#define PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_SUBCHASE_H

#include "pegasus/chase.h"
#include "pegasus/movie.h"

namespace Pegasus {

class NoradAlpha;
class SubChase;

enum HintTimerCode {
	kStartedHint,
	kEndedHint
};

struct HintTimerEvent {
	SubChase *subChase;
	HintTimerCode theEvent;

	void fire();
};

enum BlinkTimerCode {
	kEnteredBlinkState,
	kExitedBlinkState
};

struct BlinkTimerEvent {
	SubChase *subChase;
	BlinkTimerCode theEvent;

	void fire();
};

class SubChase : public ChaseInteraction {
friend class NoradAlpha;
friend struct HintTimerEvent;
friend struct BlinkTimerEvent;
public:

	SubChase(Neighborhood *);
	virtual ~SubChase() {}

	void setSoundFXLevel(const uint16);

	void handleInput(const Input &, const Hotspot *);

protected:

	void openInteraction();
	void initInteraction();
	void closeInteraction();

	void receiveNotification(Notification *, const NotificationFlags);

	void startBranching();
	void setUpBranch();
	void branchLeft();
	void branchRight();
	void dontBranch();

	void startHintTimer(TimeValue, TimeScale, HintTimerCode);
	void hintTimerExpired(HintTimerEvent &);

	void startBlinkTimer(TimeValue, TimeScale, BlinkTimerCode);
	void blinkTimerExpired(BlinkTimerEvent &);

	Movie _subMovie;
	NotificationCallBack _subCallBack;
	Picture _hintPict;
	Picture _blinkPict;
	FuseFunction _hintFuse;
	FuseFunction _blinkFuse;

	HintTimerEvent _hintEvent;
	BlinkTimerEvent _blinkEvent;

	short _subState;

	bool _canSteerSub;
};

} // End of namespace Pegasus

#endif
