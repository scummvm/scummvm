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

#include "pegasus/pegasus.h"
#include "pegasus/items/biochips/arthurchip.h"
#include "pegasus/neighborhood/mars/tunnelpod.h"
#include "pegasus/neighborhood/mars/mars.h"

namespace Pegasus {

// Segment start and end points.

static const TimeValue kLaunchStart = 315754;
static const TimeValue kLaunchEnd = 319392;

static const TimeValue kBranch1MainStart = 0;
static const TimeValue kBranch1MainEnd = 3600;
static const TimeValue kBranch2MainStart = kBranch1MainEnd;
static const TimeValue kBranch2MainEnd = 13200;
static const TimeValue kBranch3MainStart = kBranch2MainEnd;
static const TimeValue kBranch3MainEnd = 20400;
static const TimeValue kFinishMainStart = kBranch3MainEnd;
static const TimeValue kFinishMainEnd = 30640;

static const TimeValue kBranch2AltStart = 0;
static const TimeValue kBranch2AltEnd = 13200;
static const TimeValue kBranch3AltStart = kBranch2AltEnd;
static const TimeValue kBranch3AltEnd = 22800;
static const TimeValue kFinishAltStart = kBranch3AltEnd;
static const TimeValue kFinishAltEnd = 33640;

// Tunnel state.

enum {
	kTunnelLaunch,
	kTunnelBranch1Left,
	kTunnelBranch2Left,
	kTunnelBranch2Right,
	kTunnelBranch3Left,
	kTunnelBranch3Right,
	kTunnelFinish
};


TunnelPod::TunnelPod(Neighborhood *handler) : ChaseInteraction(kMarsTunnelPodInteractionID, handler,
						kMarsTunnelPodNotificationID, (PegasusEngine *)g_engine), _tunnelMainMovie(kNoDisplayElement),
						_tunnelAltMovie(kNoDisplayElement), _deathMovie(kNoDisplayElement) {
	_currentMovie = nullptr;
	_currentCallBack = nullptr;
}

void TunnelPod::setSoundFXLevel(const uint16 fxLevel) {
	_tunnelMainMovie.setVolume(fxLevel);
	_tunnelAltMovie.setVolume(fxLevel);
	_deathMovie.setVolume(fxLevel);
}

void TunnelPod::openInteraction() {
	((Mars *)_owner)->_navMovie.stop();

	_tunnelCallBack.setNotification(&_chaseNotification);
	_tunnelCallBack.initCallBack(&((Mars *)_owner)->_navMovie, kCallBackAtExtremes);
	_tunnelCallBack.setCallBackFlag(kChaseEnteredBranchZone);
	_tunnelCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_tunnelMainMovie.initFromMovieFile("Images/Mars/Pod 2345M.mov");
	_tunnelMainMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_tunnelMainMovie.moveElementTo(kNavAreaLeft, kNavAreaTop);
	_tunnelMainMovie.setDisplayOrder(kNavMovieOrder);

	_tunnelMainCallBack.setNotification(&_chaseNotification);
	_tunnelMainCallBack.initCallBack(&_tunnelMainMovie, kCallBackAtExtremes);
	_tunnelMainCallBack.setCallBackFlag(kChaseEnteredBranchZone);
	_tunnelMainCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_tunnelAltMovie.initFromMovieFile("Images/Mars/Pod 345A.mov");
	_tunnelAltMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_tunnelAltMovie.moveElementTo(kNavAreaLeft, kNavAreaTop);
	_tunnelAltMovie.setDisplayOrder(kNavMovieOrder);

	_tunnelAltCallBack.setNotification(&_chaseNotification);
	_tunnelAltCallBack.initCallBack(&_tunnelAltMovie, kCallBackAtExtremes);
	_tunnelAltCallBack.setCallBackFlag(kChaseEnteredBranchZone);
	_tunnelAltCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_deathMovie.initFromMovieFile("Images/Mars/Pod 2D.mov");
	_deathMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_deathMovie.moveElementTo(kNavAreaLeft, kNavAreaTop);
	_deathMovie.setDisplayOrder(kNavMovieOrder);

	_deathCallBack.setNotification(&_chaseNotification);
	_deathCallBack.initCallBack(&_deathMovie, kCallBackAtExtremes);
	_deathCallBack.setCallBackFlag(kChaseFinished);
	_deathCallBack.scheduleCallBack(kTriggerAtStop, 0 ,0);

	ChaseInteraction::openInteraction();

	_steerPict.setDisplayOrder(kNavMovieOrder + 1);
	_steerPict.moveElementTo(kPodSteerLeft, kPodSteerTop);
}

void TunnelPod::initInteraction() {
	_steerPict.startDisplaying();

	_tunnelState = kTunnelLaunch;
	((Mars *)_owner)->_navMovie.setSegment(kLaunchStart, kLaunchEnd - kDecisionTime);
	((Mars *)_owner)->_navMovie.setTime(kLaunchStart);
	((Mars *)_owner)->_navMovie.start();
	_currentMovie = &((Mars *)_owner)->_navMovie;
	_currentCallBack = &_tunnelCallBack;
	ChaseInteraction::initInteraction();
	if (g_arthurChip)
		g_arthurChip->playArthurMovieForEvent("Images/AI/Globals/XGLOBB27", kArthurMarsTurnedOnTransport);
}

void TunnelPod::closeInteraction() {
	((Mars *)_owner)->_navMovie.stop();
	if (_tunnelState == kTunnelFinish) {
	// Only bring nack the nav movie if we successfully finished the chase
		((Mars *)_owner)->_navMovie.startDisplaying();
		((Mars *)_owner)->_navMovie.show();
	}
	_tunnelCallBack.releaseCallBack();

	_tunnelAltMovie.stop();
	_tunnelAltMovie.stopDisplaying();
	_tunnelAltMovie.releaseMovie();
	_tunnelAltCallBack.releaseCallBack();

	_deathMovie.stop();
	_deathMovie.stopDisplaying();
	_deathMovie.releaseMovie();
	_deathCallBack.releaseCallBack();

	ChaseInteraction::closeInteraction();
}

void TunnelPod::receiveNotification(Notification *notification, const NotificationFlags flags) {
	if (notification == &_chaseNotification && flags == kChaseFinished) {
		if (_tunnelState != kTunnelFinish) {
			// We died
			((Mars *)_owner)->die(kDeathCollidedWithPod);
		} else {
			((Mars *)_owner)->startUpFromFinishedTunnelPod();
		}
	}
	ChaseInteraction::receiveNotification(notification, flags);
}

void TunnelPod::setUpBranch() {
	TimeValue branchStart, branchEnd;

	branchStart = 0;
	branchEnd = 0;
	switch (_tunnelState) {
	case kTunnelLaunch:
		branchStart = kLaunchEnd - kDecisionTime;
		branchEnd = kLaunchEnd;
		break;
	case kTunnelBranch1Left:
		branchStart = kBranch1MainEnd - kDecisionTime;
		branchEnd = kBranch1MainEnd;
		break;
	case kTunnelBranch2Left:
		branchStart = kBranch2AltEnd - kDecisionTime;
		branchEnd = kBranch2AltEnd;
		break;
	case kTunnelBranch2Right:
		branchStart = kBranch2MainEnd - kDecisionTime;
		branchEnd = kBranch2MainEnd;
		break;
	case kTunnelBranch3Left:
		branchStart = kBranch3MainEnd - kDecisionTime;
		branchEnd = kBranch3MainEnd;
		break;
	case kTunnelBranch3Right:
		branchStart = kBranch3AltEnd - kDecisionTime;
		branchEnd = kBranch3AltEnd;
		break;
	default:
		break;
	}

	_currentMovie->setSegment(branchStart, branchEnd);

	_currentCallBack->setCallBackFlag(kChaseExitedBranchZone);
	_currentCallBack->scheduleCallBack(kTriggerAtStop, 0, 0);
}

void TunnelPod::branchLeft() {
	TimeValue branchStart, branchEnd;
	NotificationFlags flag;
	Movie *movie;
	NotificationCallBack *callBack;

	branchStart = 0;
	branchEnd = 0;
	flag = 0;
	movie = nullptr;
	callBack = nullptr;
	switch (_tunnelState) {
	case kTunnelLaunch:
		branchStart = kBranch1MainStart;
		branchEnd = kBranch1MainEnd - kDecisionTime;
		_tunnelState = kTunnelBranch1Left;
		flag = kChaseEnteredBranchZone;
		movie = &_tunnelMainMovie;
		callBack = &_tunnelMainCallBack;
		break;
	case kTunnelBranch1Left:
		branchStart = kBranch2AltStart;
		branchEnd = kBranch2AltEnd - kDecisionTime;
		_tunnelState = kTunnelBranch2Left;
		flag = kChaseEnteredBranchZone;
		movie = &_tunnelAltMovie;
		callBack = &_tunnelAltCallBack;
		break;
	case kTunnelBranch2Left:
	case kTunnelBranch2Right:
		branchStart = kBranch3MainStart;
		branchEnd = kBranch3MainEnd - kDecisionTime;
		_tunnelState = kTunnelBranch3Left;
		flag = kChaseEnteredBranchZone;
		movie = &_tunnelMainMovie;
		callBack = &_tunnelMainCallBack;
		break;
	case kTunnelBranch3Left:
	case kTunnelBranch3Right:
		branchStart = kFinishAltStart;
		branchEnd = kFinishAltEnd;
		_tunnelState = kTunnelFinish;
		flag = kChaseFinished;
		movie = &_tunnelAltMovie;
		callBack = &_tunnelAltCallBack;
		break;
	default:
		break;
	}

	movie->setSegment(branchStart, branchEnd);
	movie->setTime(branchStart);

	switchTo(*movie, *callBack);

	callBack->setCallBackFlag(flag);
	callBack->scheduleCallBack(kTriggerAtStop, 0, 0);
}

void TunnelPod::branchRight() {
	TimeValue branchStart, branchEnd;
	NotificationFlags flag;
	Movie *movie;
	NotificationCallBack *callBack;

	branchStart = 0;
	branchEnd = 0;
	flag = 0;
	movie = nullptr;
	callBack = nullptr;
	switch (_tunnelState) {
	case kTunnelLaunch:
		switchTo(_deathMovie, _deathCallBack);
		return;
	case kTunnelBranch1Left:
		branchStart = kBranch2MainStart;
		branchEnd = kBranch2MainEnd - kDecisionTime;
		_tunnelState = kTunnelBranch2Right;
		flag = kChaseEnteredBranchZone;
		movie = &_tunnelMainMovie;
		callBack = &_tunnelMainCallBack;
		break;
	case kTunnelBranch2Left:
	case kTunnelBranch2Right:
		branchStart = kBranch3AltStart;
		branchEnd = kBranch3AltEnd - kDecisionTime;
		_tunnelState = kTunnelBranch3Right;
		flag = kChaseEnteredBranchZone;
		movie = &_tunnelAltMovie;
		callBack = &_tunnelAltCallBack;
		break;
	case kTunnelBranch3Left:
	case kTunnelBranch3Right:
		branchStart = kFinishMainStart;
		branchEnd = kFinishMainEnd;
		_tunnelState = kTunnelFinish;
		flag = kChaseFinished;
		movie = &_tunnelMainMovie;
		callBack = &_tunnelMainCallBack;
		break;
	default:
		break;
	}

	movie->setSegment(branchStart, branchEnd);
	movie->setTime(branchStart);

	switchTo(*movie, *callBack);

	callBack->setCallBackFlag(flag);
	callBack->scheduleCallBack(kTriggerAtStop, 0, 0);
}

void TunnelPod::dontBranch() {
	switch (_tunnelState) {
	case kTunnelLaunch:
	case kTunnelBranch1Left:
		if (_currentMovie == &_tunnelAltMovie)
			branchLeft();
		else
			branchRight();
		break;
	case kTunnelBranch2Left:
	case kTunnelBranch2Right:
		if (_currentMovie == &_tunnelAltMovie)
			branchRight();
		else
			branchLeft();
		break;
	case kTunnelBranch3Left:
	case kTunnelBranch3Right:
		if (_currentMovie == &_tunnelAltMovie)
			branchLeft();
		else
			branchRight();
		break;
	default:
		break;
	}
}

void TunnelPod::switchTo(Movie &movie, NotificationCallBack &callBack) {
	if (_currentMovie != &movie) {
		if (_currentMovie != nullptr) {
			_currentMovie->stop();
			_currentMovie->hide();
			_currentMovie->stopDisplaying();
		}

		_currentMovie = &movie;

		_currentMovie->startDisplaying();
		_currentMovie->show();
		_currentMovie->start();
	}

	if (_currentCallBack != &callBack) {
		_currentCallBack = &callBack;
	}
}

} // End of namespace Pegasus
