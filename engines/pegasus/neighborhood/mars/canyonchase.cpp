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
#include "pegasus/neighborhood/mars/canyonchase.h"
#include "pegasus/neighborhood/mars/mars.h"

namespace Pegasus {

// Segment start and end points.

//static const TimeValue kPrepStart = 0;
static const TimeValue kPrepEnd = 3000;
static const TimeValue kLaunchStart = kPrepEnd;
static const TimeValue kLaunchEnd = 6640;
static const TimeValue kBranch1Start = kLaunchEnd;
static const TimeValue kBranch1End = 22240;
static const TimeValue kBranch2Start = kBranch1End;
static const TimeValue kBranch2End = 28440;
static const TimeValue kBranch3Start = kBranch2End;
static const TimeValue kBranch3End = 38640;
static const TimeValue kBranch4Start = kBranch3End;
static const TimeValue kBranch4End = 43880;
static const TimeValue kBranch5Start = kBranch4End;
static const TimeValue kBranch5End = 58680;
static const TimeValue kExitStart = kBranch5End;
static const TimeValue kExitEnd = 66480;
static const TimeValue kExitLoopPoint = 66200;
static const TimeValue kExitGenoPoint = 62560;

// Death start and end points.

static const TimeValue kDeath1Start = 0;
static const TimeValue kDeath1End = 2400;
static const TimeValue kDeath2Start = kDeath1End;
static const TimeValue kDeath2End = 4720;
static const TimeValue kDeath3Start = kDeath2End;
static const TimeValue kDeath3End = 7120;
static const TimeValue kDeath4Start = kDeath3End;
static const TimeValue kDeath4End = 9280;
static const TimeValue kDeath5Start = kDeath4End;
static const TimeValue kDeath5End = 12000;

// Chase state.

enum {
	kCanyonLaunch,
	kCanyonBranch1Left,
	kCanyonBranch1Right,
	kCanyonBranch2Left,
	kCanyonBranch2Right,
	kCanyonBranch3Left,
	kCanyonBranch4Left,
	kCanyonBranch4Right,
	kCanyonBranch5Left,
	kCanyonBranch5Right,
	kCanyonExit,
	kCanyonLoop
};

void MusicTimerEvent::fire() {
	canyonChase->musicTimerExpired(*this);
}

CanyonChase::CanyonChase(Neighborhood *handler) : ChaseInteraction(kMarsCanyonChaseInteractionID, handler,
							kMarsCanyonChaseNotificationID, (PegasusEngine *)g_engine), _canyonMovie1(kNoDisplayElement),
							_canyonMovie2(kNoDisplayElement), _deathMovie(kNoDisplayElement), _genoMovie(kNoDisplayElement) {
	_currentMovie = nullptr;
	_currentCallBack = nullptr;
}

void CanyonChase::setSoundFXLevel(const uint16 fxLevel) {
	_canyonMovie1.setVolume(fxLevel);
	_canyonMovie2.setVolume(fxLevel);
	_deathMovie.setVolume(fxLevel);
}

void CanyonChase::setAmbienceLevel(const uint16 level) {
	_genoMovie.setVolume(level);
	_musicFader.setMasterVolume(level);
}

void CanyonChase::startCanyonMusicLoop(void) {
	FaderMoveSpec spec;

	_musicLoop.loopSound();
	spec.makeTwoKnotFaderSpec(10, 0, 0, 1, 255);
	_musicFader.startFader(spec);
}

void CanyonChase::stopCanyonMusicLoop(const long ticks) {
	FaderMoveSpec spec;

	spec.makeTwoKnotFaderSpec(10, 0, 255, ticks, 0);
	_musicFader.startFader(spec);
}

void CanyonChase::openInteraction() {
	_canyonMovie1.initFromMovieFile("Images/Mars/Canyon_hq1.mov");
	_canyonMovie1.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_canyonMovie1.moveElementTo(kShuttleWindowLeft, kShuttleWindowTop);
	_canyonMovie1.setDisplayOrder(kShuttleMonitorOrder);

	_canyon1CallBack.setNotification(&_chaseNotification);
	_canyon1CallBack.initCallBack(&_canyonMovie1, kCallBackAtExtremes);
	_canyon1CallBack.setCallBackFlag(kChaseEnteredBranchZone);
	_canyon1CallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_canyonMovie2.initFromMovieFile("Images/Mars/Canyon_hq2.mov");
	_canyonMovie2.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_canyonMovie2.moveElementTo(kShuttleWindowLeft, kShuttleWindowTop);
	_canyonMovie2.setDisplayOrder(kShuttleMonitorOrder);

	_canyon2CallBack.setNotification(&_chaseNotification);
	_canyon2CallBack.initCallBack(&_canyonMovie2, kCallBackAtExtremes);
	_canyon2CallBack.setCallBackFlag(kChaseEnteredBranchZone);
	_canyon2CallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_deathMovie.initFromMovieFile("Images/Mars/Canyon_hqD.mov");
	_deathMovie.setVolume(((PegasusEngine *)g_engine)->getSoundFXLevel());
	_deathMovie.moveElementTo(kShuttleWindowLeft, kShuttleWindowTop);
	_deathMovie.setDisplayOrder(kShuttleMonitorOrder);

	_deathCallBack.setNotification(&_chaseNotification);
	_deathCallBack.initCallBack(&_deathMovie, kCallBackAtExtremes);
	_deathCallBack.setCallBackFlag(kChaseFinished);
	_deathCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_musicLoop.attachFader(&_musicFader);
	_musicLoop.initFromAIFFFile("Sounds/Mars/Canyon Loop.44K.16.AIFF");
	_musicFader.setMasterVolume(((PegasusEngine *)g_engine)->getAmbienceLevel());

	ChaseInteraction::openInteraction();

	_steerPict.setDisplayOrder(kShuttleMonitorOrder + 1);
	_steerPict.moveElementTo(kShuttleSteerLeft, kShuttleSteerTop);
}

void CanyonChase::initInteraction() {
	_steerPict.startDisplaying();

	// Launch branch is identical in both movies
	_canyonState = kCanyonLaunch;
	_canyonMovie1.setSegment(kLaunchStart, kLaunchEnd - kDecisionTime);
	_canyonMovie1.setTime(kLaunchStart);
	switchTo(_canyonMovie1, _canyon1CallBack);
	startCanyonMusicLoop();
	ChaseInteraction::initInteraction();
}

void CanyonChase::closeInteraction() {
	_canyonMovie1.stop();
	_canyonMovie1.stopDisplaying();
	_canyonMovie1.releaseMovie();
	_canyon1CallBack.releaseCallBack();

	_canyonMovie2.stop();
	_canyonMovie2.stopDisplaying();
	_canyonMovie2.releaseMovie();
	_canyon2CallBack.releaseCallBack();

	_deathMovie.stop();
	_deathMovie.stopDisplaying();
	_deathMovie.releaseMovie();
	_deathCallBack.releaseCallBack();

	_genoMovie.stop();
	_genoMovie.stopDisplaying();
	_genoMovie.releaseMovie();
	_genoCallBack.releaseCallBack();

	ChaseInteraction::closeInteraction();
}

void CanyonChase::receiveNotification(Notification *notification, const NotificationFlags flags) {
	Input input;

	if (notification == &_chaseNotification && flags == kChaseFinished) {
		if (_canyonState == kCanyonLoop) {
			// Swallow the notification if we loop back to the beginning
			InputDevice.getInput(input, kFilterAllInput);
			if (JMPPPInput::isEasterEggModifierInput(input)) {
				stopCanyonMusicLoop(15);
				doGenoChase();
			} else {
				_canyonMovie2.setSegment(kExitGenoPoint, kExitLoopPoint - kDecisionTime);
				_canyonMovie2.setTime(kExitGenoPoint);
				switchTo(_canyonMovie2, _canyon2CallBack);
				_canyon2CallBack.setCallBackFlag(kChaseEnteredBranchZone);
				_canyon2CallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
				_canyonState = kCanyonLaunch;
			}
			return;
		} else if (_canyonState != kCanyonExit) {
			// We died
			((Mars *)_owner)->die(kDeathRanIntoCanyonWall);
		}
	}
	ChaseInteraction::receiveNotification(notification, flags);
}

void CanyonChase::setUpBranch() {
	TimeValue branchStart, branchEnd;

	branchStart = 0;
	branchEnd = 0;
	switch (_canyonState) {
	case kCanyonLaunch:
	case kCanyonExit:
		branchStart = kLaunchEnd - kDecisionTime;
		branchEnd = kLaunchEnd;
		break;
	case kCanyonBranch1Left:
	case kCanyonBranch1Right:
		branchStart = kBranch1End - kDecisionTime;
		branchEnd = kBranch1End;
		break;
	case kCanyonBranch2Left:
	case kCanyonBranch2Right:
		branchStart = kBranch2End - kDecisionTime;
		branchEnd = kBranch2End;
		break;
	case kCanyonBranch3Left:
		branchStart = kBranch3End - kDecisionTime;
		branchEnd = kBranch3End;
		break;
	case kCanyonBranch4Left:
	case kCanyonBranch4Right:
		branchStart = kBranch4End - kDecisionTime;
		branchEnd = kBranch4End;
		break;
	case kCanyonBranch5Left:
	case kCanyonBranch5Right:
		branchStart = kBranch5End - kDecisionTime;
		branchEnd = kBranch5End;
		break;
	default:
		break;
	}

	_currentMovie->setSegment(branchStart, branchEnd);
	// Need to call SetTime here in case we loop
	_currentMovie->setTime(branchStart);

	_currentCallBack->setCallBackFlag(kChaseExitedBranchZone);
	_currentCallBack->scheduleCallBack(kTriggerAtStop, 0, 0);
}

void CanyonChase::branchLeft() {
	TimeValue branchStart, branchEnd;
	Movie *movie;
	NotificationCallBack *callBack;

	branchStart = 0;
	branchEnd = 0;
	switch (_canyonState) {
	case kCanyonLaunch:
		branchStart = kBranch1Start;
		branchEnd = kBranch1End - kDecisionTime;
		_canyonState = kCanyonBranch1Left;
		break;
	case kCanyonBranch1Left:
	case kCanyonBranch1Right:
		branchStart = kBranch2Start;
		branchEnd = kBranch2End - kDecisionTime;
		_canyonState = kCanyonBranch2Left;
		break;
	case kCanyonBranch2Left:
	case kCanyonBranch2Right:
		branchStart = kBranch3Start;
		branchEnd = kBranch3End - kDecisionTime;
		_canyonState = kCanyonBranch3Left;
		break;
	case kCanyonBranch3Left:
		branchStart = kBranch4Start;
		branchEnd = kBranch4End - kDecisionTime;
		_canyonState = kCanyonBranch4Left;
		break;
	case kCanyonBranch4Left:
	case kCanyonBranch4Right:
		branchStart = kBranch5Start;
		branchEnd = kBranch5End - kDecisionTime;
		_canyonState = kCanyonBranch5Left;
		break;
	case kCanyonBranch5Left:
	case kCanyonBranch5Right:
		dontBranch();
		return;
	default:
		break;
	}

	// Left branches are in hq2 (except exit)
	// Segment 5 branches are switched
	if (_canyonState == kCanyonBranch5Left || _canyonState == kCanyonBranch5Right) {
		movie = &_canyonMovie1;
		callBack = &_canyon1CallBack;
	} else {
		movie = &_canyonMovie2;
		callBack = &_canyon2CallBack;
	}

	movie->setSegment(branchStart, branchEnd);
	movie->setTime(branchStart);

	switchTo(*movie, *callBack);

	callBack->setCallBackFlag(kChaseEnteredBranchZone);
	callBack->scheduleCallBack(kTriggerAtStop, 0, 0);
}

void CanyonChase::branchRight() {
	TimeValue branchStart, branchEnd;
	NotificationFlags flag;
	Movie *movie;
	NotificationCallBack *callBack;

	branchStart = 0;
	branchEnd = 0;
	flag = 0;
	switch (_canyonState) {
	case kCanyonLaunch:
		branchStart = kBranch1Start;
		branchEnd = kBranch1End - kDecisionTime;
		_canyonState = kCanyonBranch1Right;
		flag = kChaseEnteredBranchZone;
		break;
	case kCanyonBranch1Left:
	case kCanyonBranch1Right:
		branchStart = kBranch2Start;
		branchEnd = kBranch2End - kDecisionTime;
		_canyonState = kCanyonBranch2Right;
		flag = kChaseEnteredBranchZone;
		break;
	case kCanyonBranch2Left:
	case kCanyonBranch2Right:
		dontBranch();
		return;
	case kCanyonBranch3Left:
		branchStart = kBranch4Start;
		branchEnd = kBranch4End - kDecisionTime;
		_canyonState = kCanyonBranch4Right;
		flag = kChaseEnteredBranchZone;
		break;
	case kCanyonBranch4Left:
	case kCanyonBranch4Right:
		branchStart = kBranch5Start;
		branchEnd = kBranch5End - kDecisionTime;
		_canyonState = kCanyonBranch5Right;
		flag = kChaseEnteredBranchZone;
		break;
	case kCanyonBranch5Left:
	case kCanyonBranch5Right:
		// Exit loop branch is in hq2
		branchStart = kExitStart;
		branchEnd = kExitEnd;
		_canyonState = kCanyonExit;
		flag = kChaseFinished;
		startMusicTimer(kCanyonChaseStart + kCanyonChaseExitedTime - kExitStart, kMovieTicksPerSecond,
						kCanyonExited);
		break;
	default:
		break;
	}

	// Right branches are in hq1 (except exit)
	// Segment 5 branches are switched
	if (_canyonState == kCanyonBranch5Left || _canyonState == kCanyonBranch5Right) {
		movie = &_canyonMovie2;
		callBack = &_canyon2CallBack;
	} else {
		movie = &_canyonMovie1;
		callBack = &_canyon1CallBack;
	}

	movie->setSegment(branchStart, branchEnd);
	movie->setTime(branchStart);

	switchTo(*movie, *callBack);

	callBack->setCallBackFlag(flag);
	callBack->scheduleCallBack(kTriggerAtStop, 0, 0);
}

void CanyonChase::dontBranch() {
	TimeValue branchStart, branchEnd;

	branchStart = 0;
	branchEnd = 0;
	switch (_canyonState) {
	case kCanyonLaunch:
		branchStart = kDeath1Start;
		branchEnd = kDeath1End;
		break;
	case kCanyonBranch1Left:
	case kCanyonBranch1Right:
		branchStart = kDeath2Start;
		branchEnd = kDeath2End;
		break;
	case kCanyonBranch2Left:
	case kCanyonBranch2Right:
		branchStart = kDeath3Start;
		branchEnd = kDeath3End;
		break;
	case kCanyonBranch3Left:
		branchStart = kDeath4Start;
		branchEnd = kDeath4End;
		break;
	case kCanyonBranch4Left:
	case kCanyonBranch4Right:
		branchStart = kDeath5Start;
		branchEnd = kDeath5End;
		break;
	case kCanyonBranch5Left:
	case kCanyonBranch5Right:
		_canyonMovie2.setSegment(kExitStart, kExitGenoPoint);
		_canyonMovie2.setTime(kExitStart);
		switchTo(_canyonMovie2, _canyon2CallBack);
		_canyon2CallBack.setCallBackFlag(kChaseFinished);
		_canyon2CallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
		_canyonState = kCanyonLoop;
		return;
	default:
		break;
	}

	_deathMovie.setSegment(branchStart, branchEnd);
	_deathMovie.setTime(branchStart);

	switchTo(_deathMovie, _deathCallBack);

	startMusicTimer(10, 10, kCanyonRanIntoWall);
}

void CanyonChase::showControlsHint() {
	((Mars *)_owner)->_lowerLeftShuttleMovie.setTime(kShuttleLowerLeftKeypadHintTime);
	((Mars *)_owner)->_lowerLeftShuttleMovie.redrawMovieWorld();
	ChaseInteraction::showControlsHint();
}

void CanyonChase::hideControlsHint() {
	((Mars *)_owner)->_lowerLeftShuttleMovie.setTime(kShuttleLowerLeftCollisionTime);
	((Mars *)_owner)->_lowerLeftShuttleMovie.redrawMovieWorld();
	ChaseInteraction::hideControlsHint();
}

void CanyonChase::switchTo(Movie &movie, NotificationCallBack &callBack) {
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

void CanyonChase::startMusicTimer(TimeValue time, TimeScale scale, MusicTimerCode code) {
	_musicFuse.primeFuse(time, scale);
	_musicEvent.canyonChase = this;
	_musicEvent.theEvent = code;
	_musicFuse.setFunctor(new Common::Functor0Mem<void, MusicTimerEvent>(&_musicEvent, &MusicTimerEvent::fire));
	_musicFuse.lightFuse();
}

void CanyonChase::musicTimerExpired(MusicTimerEvent &event) {
	FaderMoveSpec spec;

	switch (event.theEvent) {
	case kCanyonRanIntoWall:
		stopCanyonMusicLoop(5);
		break;
	case kCanyonExited:
		spec.makeTwoKnotFaderSpec(20, 0, 255, 5, 160);
		_musicFader.startFader(spec);
		startMusicTimer(kCanyonChaseFadedTime, kMovieTicksPerSecond, kCanyonFaded);
		break;
	case kCanyonFaded:
		spec.makeTwoKnotFaderSpec(10, 0, 160, 30, 0);
		_musicFader.startFader(spec);
		((Mars *)_owner)->startMarsTimer(kLaunchTubeDVDReachedTime, kMovieTicksPerSecond,
											kMarsLaunchTubeReached);
		break;
	default:
		break;
	}
}

void CanyonChase::doGenoChase() {
	_genoMovie.initFromMovieFile("Images/Mars/Canyon_hqG.mov");
	_genoMovie.setVolume(((PegasusEngine *)g_engine)->getAmbienceLevel());
	_genoMovie.moveElementTo(kShuttleWindowLeft, kShuttleWindowTop);
	_genoMovie.setDisplayOrder(kShuttleMonitorOrder);
	_genoMovie.startDisplaying();
	_genoMovie.show();
	_genoMovie.start();

	_genoCallBack.setNotification(&_chaseNotification);
	_genoCallBack.initCallBack(&_genoMovie, kCallBackAtExtremes);
	_genoCallBack.setCallBackFlag(kChaseFinished);
	_genoCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	_canyonState = kCanyonExit;

	((Mars *)_owner)->startMarsTimer(_genoMovie.getDuration() - 5 * kMovieTicksPerSecond,
										kMovieTicksPerSecond, kMarsLaunchTubeReached);
}

} // End of namespace Pegasus
