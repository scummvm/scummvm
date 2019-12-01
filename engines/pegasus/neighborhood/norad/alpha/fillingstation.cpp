/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/alpha/fillingstation.h"
#include "pegasus/neighborhood/norad/alpha/noradalpha.h"

namespace Pegasus {

static const NotificationFlags kFSPowerUpFinishedFlag = 1;
static const NotificationFlags kFSSplashFinishedFlag = kFSPowerUpFinishedFlag << 1;
static const NotificationFlags kFSIntakeWarningFinishedFlag = kFSSplashFinishedFlag << 1;
static const NotificationFlags kFSIntakeHiliteFinishedFlag = kFSIntakeWarningFinishedFlag << 1;
static const NotificationFlags kFSDispenseHiliteFinishedFlag = kFSIntakeHiliteFinishedFlag << 1;
static const NotificationFlags kFSArHiliteFinishedFlag = kFSDispenseHiliteFinishedFlag << 1;
static const NotificationFlags kFSCO2HiliteFinishedFlag = kFSArHiliteFinishedFlag << 1;
static const NotificationFlags kFSHeHiliteFinishedFlag = kFSCO2HiliteFinishedFlag << 1;
static const NotificationFlags kFSOHiliteFinishedFlag = kFSHeHiliteFinishedFlag << 1;
static const NotificationFlags kFSNHiliteFinishedFlag = kFSOHiliteFinishedFlag << 1;

static const NotificationFlags kFSNotificationFlags = kFSPowerUpFinishedFlag |
												kFSSplashFinishedFlag |
												kFSIntakeWarningFinishedFlag |
												kFSIntakeHiliteFinishedFlag |
												kFSDispenseHiliteFinishedFlag |
												kFSArHiliteFinishedFlag |
												kFSCO2HiliteFinishedFlag |
												kFSHeHiliteFinishedFlag |
												kFSOHiliteFinishedFlag |
												kFSNHiliteFinishedFlag;

static const int16 kNoState = 0;
static const int16 kMainMenu = 1;
static const int16 kWaitingForAttach = 2;
static const int16 kDispenseMenu = 3;
static const int16 kWaitingForDispense = 4;

// Dummy itemIDs
static const ItemID kCO2Item = 10000;
static const ItemID kHeItem = 10001;

// Interactive points.
enum {
	kFSPowerUpStartStart = 0,
	kFSPowerUpStartStop = 600,
	kFSSplashStart = 600,
	kFSSplashStop = 7800,
	kFSSplashIntakeStart = 7800,
	kFSSplashIntakeStop = 18600,

	kFSMainMenu = 18600,
	kFSIntakeHiliteStart = 19200,
	kFSIntakeHiliteStop = 19800,
	kFSDispenseHiliteStart = 19800,
	kFSDispenseHiliteStop = 20400,

	kFSDispenseMenu = 20400,

	kFSArHiliteStart = 21000,
	kFSArHiliteStop = 21600,
	kFSArAttach = 21600,
	kFSArFilledStart = 22200,
	kFSArFilledStop = 25200,
	kFSArIncompatibleStart = 25200,
	kFSArIncompatibleStop = 30000,

	kFSCO2HiliteStart = 30000,
	kFSCO2HiliteStop = 30600,
	kFSCO2Attach = 30600,
	kFSCO2FilledStart = 31200,
	kFSCO2FilledStop = 34200,
	kFSCO2IncompatibleStart = 34200,
	kFSCO2IncompatibleStop = 39000,

	kFSHeHiliteStart = 39000,
	kFSHeHiliteStop = 39600,
	kFSHeAttach = 39600,
	kFSHeFilledStart = 40200,
	kFSHeFilledStop = 43200,
	kFSHeIncompatibleStart = 43200,
	kFSHeIncompatibleStop = 48000,

	kFSOHiliteStart = 48000,
	kFSOHiliteStop = 48600,
	kFSOAttach = 48600,
	kFSOFilledStart = 49200,
	kFSOFilledStop = 52200,
	kFSOIncompatibleStart = 52200,
	kFSOIncompatibleStop = 57000,

	kFSNHiliteStart = 57000,
	kFSNHiliteStop = 57600,
	kFSNAttach = 57600,
	kFSNFilledStart = 58200,
	kFSNFilledStop = 61200,
	kFSNIncompatibleStart = 61200,
	kFSNIncompatibleStop = 66000,

	kFSIntakeMenu = 66000,
	kFSIntakeInProgressStart = 66600,
	kFSIntakeInProgressStop = 69600
};

NoradAlphaFillingStation::NoradAlphaFillingStation(Neighborhood *owner) : GameInteraction(kNoradFillingStationInteractionID, owner),
		_rightSideMovie(kN01RightSideID), _rightSideNotification(kNoradFillingStationNotificationID, ((PegasusEngine *)g_engine)) {
	_state = kNoState;
}

void NoradAlphaFillingStation::openInteraction() {
	_rightSideMovie.initFromMovieFile("Images/Norad Alpha/N01W Right Side");
	_rightSideMovie.moveElementTo(kNoradAlpha01RightSideLeft, kNoradAlpha01RightSideTop);
	_rightSideMovie.setDisplayOrder(kN01RightSideOrder);
	_rightSideMovie.startDisplaying();
	_rightSideCallBack.setNotification(&_rightSideNotification);
	_rightSideCallBack.initCallBack(&_rightSideMovie, kCallBackAtExtremes);
	_rightSideCallBack.setCallBackFlag(kFSPowerUpFinishedFlag);
	_rightSideNotification.notifyMe(this, kFSNotificationFlags, kFSNotificationFlags);
	_rightSideCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	_rightSideMovie.show();
	_rightSideMovie.redrawMovieWorld();
	_rightSideMovie.setSegment(kFSPowerUpStartStart, kFSPowerUpStartStop);
}

void NoradAlphaFillingStation::initInteraction() {
	allowInput(false);

	_rightSideMovie.setRate(2);
}

void NoradAlphaFillingStation::closeInteraction() {
	_rightSideMovie.stop();
	_rightSideMovie.stopDisplaying();
	_rightSideMovie.releaseMovie();
	_rightSideCallBack.releaseCallBack();
	((NoradAlpha *)getOwner())->turnOffFillingStation();
}

void NoradAlphaFillingStation::setStaticState(TimeValue time, int16 state) {
	_rightSideMovie.stop();
	_rightSideMovie.setSegment(0, _rightSideMovie.getDuration());
	_rightSideMovie.setTime(time);
	_rightSideMovie.redrawMovieWorld();
	_state = state;
	allowInput(true);
}

void NoradAlphaFillingStation::setSegmentState(TimeValue start, TimeValue stop, NotificationFlags flag, int16 state) {
	_rightSideMovie.stop();
	_rightSideMovie.setSegment(start, stop);
	_rightSideMovie.setTime(start);
	_rightSideCallBack.setCallBackFlag(flag);
	_rightSideCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	_state = state;
	allowInput(false);
	_rightSideMovie.setRate(2);
}

void NoradAlphaFillingStation::powerUpFinished() {
	((NoradAlpha *)getOwner())->turnOnFillingStation();
	setSegmentState(kFSSplashStart, kFSSplashStop, kFSSplashFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::splashFinished() {
	if (GameState.getNoradGassed())
		setSegmentState(kFSSplashIntakeStart, kFSSplashIntakeStop, kFSIntakeWarningFinishedFlag, kNoState);
	else
		intakeWarningFinished();
}

void NoradAlphaFillingStation::intakeWarningFinished() {
	setStaticState(kFSMainMenu, kMainMenu);
}

void NoradAlphaFillingStation::showIntakeInProgress(uint16 numSeconds) {
	if (numSeconds == 0) {
		setSegmentState(kFSIntakeInProgressStart, kFSIntakeInProgressStop, kFSIntakeWarningFinishedFlag, kNoState);
		Item *item = ((NoradAlpha *)getOwner())->getFillingItem();

		if (item->getObjectID() == kGasCanister) {
			GameState.setNoradGassed(true);
			((NoradAlpha *)getOwner())->checkAirMask();
			getOwner()->restoreStriding(kNorad03, kEast, kAltNoradAlphaNormal);
		}
	} else {
		setSegmentState(kFSIntakeInProgressStart, kFSIntakeInProgressStart + _rightSideMovie.getScale() * numSeconds,
				kFSIntakeWarningFinishedFlag, kNoState);
	}
}

void NoradAlphaFillingStation::intakeHighlightFinished() {
	_rightSideMovie.stop();

	if (GameState.getNoradGassed()) {
		showIntakeInProgress(2);
	} else {
		Item *item = ((NoradAlpha *)getOwner())->getFillingItem();
		if (item)
			showIntakeInProgress(0);
		else
			setStaticState(kFSIntakeMenu, kWaitingForAttach);
	}
}

void NoradAlphaFillingStation::dispenseHighlightFinished() {
	setStaticState(kFSDispenseMenu, kDispenseMenu);
}

void NoradAlphaFillingStation::dispenseGas() {
	Item *item = ((NoradAlpha *)getOwner())->getFillingItem();

	if (item) {
		if (item->getObjectID() != _dispenseItemID)
			switch (_dispenseItemID) {
			case kArgonCanister:
				setSegmentState(kFSArIncompatibleStart, kFSArIncompatibleStop,
						kFSIntakeWarningFinishedFlag, kNoState);
				break;
			case kCO2Item:
				setSegmentState(kFSCO2IncompatibleStart, kFSCO2IncompatibleStop,
						kFSIntakeWarningFinishedFlag, kNoState);
				break;
			case kHeItem:
				setSegmentState(kFSHeIncompatibleStart, kFSHeIncompatibleStop,
						kFSIntakeWarningFinishedFlag, kNoState);
				break;
			case kAirMask:
				setSegmentState(kFSOIncompatibleStart, kFSOIncompatibleStop,
						kFSIntakeWarningFinishedFlag, kNoState);
				break;
			case kNitrogenCanister:
				setSegmentState(kFSNIncompatibleStart, kFSNIncompatibleStop,
						kFSIntakeWarningFinishedFlag, kNoState);
				break;
			default:
				break;
			}
		else {
			if (_dispenseItemID == kArgonCanister) {
				setSegmentState(kFSArFilledStart, kFSArFilledStop, kFSIntakeWarningFinishedFlag, kNoState);
				item->setItemState(kArgonFull);
				GameState.setScoringFilledArgonCanister(true);
			} else if (_dispenseItemID == kAirMask) {
				setSegmentState(kFSOFilledStart, kFSOFilledStop, kFSIntakeWarningFinishedFlag, kNoState);
				((AirMask *)item)->refillAirMask();
				GameState.setScoringFilledOxygenCanister(true);
			} else if (_dispenseItemID == kNitrogenCanister) {
				setSegmentState(kFSNFilledStart, kFSNFilledStop, kFSIntakeWarningFinishedFlag, kNoState);
				item->setItemState(kNitrogenFull);
			}
		}
	} else {
		switch (_dispenseItemID) {
		case kArgonCanister:
			setStaticState(kFSArAttach, kWaitingForDispense);
			break;
		case kCO2Item:
			setStaticState(kFSCO2Attach, kWaitingForDispense);
			break;
		case kHeItem:
			setStaticState(kFSHeAttach, kWaitingForDispense);
			break;
		case kAirMask:
			setStaticState(kFSOAttach, kWaitingForDispense);
			break;
		case kNitrogenCanister:
			setStaticState(kFSNAttach, kWaitingForDispense);
			break;
		default:
			break;
		}
	}
}

void NoradAlphaFillingStation::ArHighlightFinished() {
	_dispenseItemID = kArgonCanister;
	dispenseGas();
}

void NoradAlphaFillingStation::CO2HighlightFinished() {
	_dispenseItemID = kCO2Item;
	dispenseGas();
}

void NoradAlphaFillingStation::HeHighlightFinished() {
	_dispenseItemID = kHeItem;
	dispenseGas();
}

void NoradAlphaFillingStation::OHighlightFinished() {
	_dispenseItemID = kAirMask;
	dispenseGas();
}

void NoradAlphaFillingStation::NHighlightFinished() {
	_dispenseItemID = kNitrogenCanister;
	dispenseGas();
}

void NoradAlphaFillingStation::receiveNotification(Notification *, const NotificationFlags flags) {
	switch (flags) {
	case kFSPowerUpFinishedFlag:
		powerUpFinished();
		break;
	case kFSSplashFinishedFlag:
		splashFinished();
		break;
	case kFSIntakeWarningFinishedFlag:
		intakeWarningFinished();
		break;
	case kFSIntakeHiliteFinishedFlag:
		intakeHighlightFinished();
		break;
	case kFSDispenseHiliteFinishedFlag:
		dispenseHighlightFinished();
		break;
	case kFSArHiliteFinishedFlag:
		ArHighlightFinished();
		break;
	case kFSCO2HiliteFinishedFlag:
		CO2HighlightFinished();
		break;
	case kFSHeHiliteFinishedFlag:
		HeHighlightFinished();
		break;
	case kFSOHiliteFinishedFlag:
		OHighlightFinished();
		break;
	case kFSNHiliteFinishedFlag:
		NHighlightFinished();
		break;
	default:
		break;
	}
}

void NoradAlphaFillingStation::handleInput(const Input &input, const Hotspot *cursorSpot) {
	InputHandler::handleInput(input, cursorSpot);
}

void NoradAlphaFillingStation::clickInIntake() {
	setSegmentState(kFSIntakeHiliteStart, kFSIntakeHiliteStop, kFSIntakeHiliteFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::clickInDispense() {
	setSegmentState(kFSDispenseHiliteStart, kFSDispenseHiliteStop, kFSDispenseHiliteFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::clickInAr() {
	setSegmentState(kFSArHiliteStart, kFSArHiliteStop, kFSArHiliteFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::clickInCO2() {
	setSegmentState(kFSCO2HiliteStart, kFSCO2HiliteStop, kFSCO2HiliteFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::clickInHe() {
	setSegmentState(kFSHeHiliteStart, kFSHeHiliteStop, kFSHeHiliteFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::clickInO() {
	setSegmentState(kFSOHiliteStart, kFSOHiliteStop, kFSOHiliteFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::clickInN() {
	setSegmentState(kFSNHiliteStart, kFSNHiliteStop, kFSNHiliteFinishedFlag, kNoState);
}

void NoradAlphaFillingStation::clickInHotspot(const Input &input, const Hotspot *spot) {
	GameInteraction::clickInHotspot(input, spot);

	switch (spot->getObjectID()) {
	case kNorad01IntakeSpotID:
		clickInIntake();
		break;
	case kNorad01DispenseSpotID:
		clickInDispense();
		break;
	case kNorad01ArSpotID:
		clickInAr();
		break;
	case kNorad01CO2SpotID:
		clickInCO2();
		break;
	case kNorad01HeSpotID:
		clickInHe();
		break;
	case kNorad01OSpotID:
		clickInO();
		break;
	case kNorad01NSpotID:
		clickInN();
		break;
	default:
		break;
	}
}

void NoradAlphaFillingStation::activateHotspots() {
	GameInteraction::activateHotspots();

	switch (_state) {
	case kMainMenu:
		g_allHotspots.activateOneHotspot(kNorad01IntakeSpotID);
		g_allHotspots.activateOneHotspot(kNorad01DispenseSpotID);
		break;
	case kDispenseMenu:
		g_allHotspots.activateOneHotspot(kNorad01ArSpotID);
		g_allHotspots.activateOneHotspot(kNorad01CO2SpotID);
		g_allHotspots.activateOneHotspot(kNorad01HeSpotID);
		g_allHotspots.activateOneHotspot(kNorad01OSpotID);
		g_allHotspots.activateOneHotspot(kNorad01NSpotID);
		break;
	default:
		break;
	}
}

void NoradAlphaFillingStation::newFillingItem(Item *item) {
	switch (_state) {
	case kWaitingForAttach:
		if (item)
			showIntakeInProgress(0);
		break;
	case kWaitingForDispense:
		dispenseGas();
		break;
	default:
		break;
	}
}

} // End of namespace Pegasus
