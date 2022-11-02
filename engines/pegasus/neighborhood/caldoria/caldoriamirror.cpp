/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/items/biochips/arthurchip.h"
#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/caldoria/caldoriamirror.h"

namespace Pegasus {

CaldoriaMirror::CaldoriaMirror(Neighborhood *owner) : GameInteraction(kCaldoriaMirrorInteractionID, owner) {
	_neighborhoodNotification = nullptr;
}

void CaldoriaMirror::openInteraction() {
	_neighborhoodNotification = _owner->getNeighborhoodNotification();
	_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag, kExtraCompletedFlag);
}

void CaldoriaMirror::initInteraction() {
	_owner->setCurrentActivation(kActivateMirrorReady);
	_owner->startExtraSequence(kCaBathroomGreeting, kExtraCompletedFlag, kFilterNoInput);
	// The original made the player click to start each of the following sequences,
	// which was unnecessary, so it is automated here.
	_owner->startExtraSequenceSync(kCaBathroomGreeting, kFilterNoInput);
	_owner->startExtraSequenceSync(kCaBathroomBodyFat, kFilterNoInput);
	_owner->startExtraSequence(kCaBathroomStylistIntro, kExtraCompletedFlag, kFilterNoInput);
}

void CaldoriaMirror::closeInteraction() {
	_neighborhoodNotification->cancelNotification(this);
}

void CaldoriaMirror::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (_owner->getLastExtra() == (uint32)kCaBathroomAgencyStandard || !input.anyDirectionInput())
		GameInteraction::handleInput(input, cursorSpot);
}

void CaldoriaMirror::activateHotspots() {
	GameInteraction::activateHotspots();

	switch (_owner->getLastExtra()) {
	case kCaBathroomGreeting:
	case kCaBathroomBodyFat:
	case kCaBathroomRetrothrash:
	case kCaBathroomGeoWave:
		g_allHotspots.activateOneHotspot(kCaBathroomMirrorSpotID);
		g_allHotspots.deactivateOneHotspot(kCaHairStyle1SpotID);
		g_allHotspots.deactivateOneHotspot(kCaHairStyle2SpotID);
		g_allHotspots.deactivateOneHotspot(kCaHairStyle3SpotID);
		break;
	case kCaBathroomStylistIntro:
	case kCaBathroomRetrothrashReturn:
	case kCaBathroomGeoWaveReturn:
		g_allHotspots.activateOneHotspot(kCaHairStyle1SpotID);
		g_allHotspots.activateOneHotspot(kCaHairStyle2SpotID);
		g_allHotspots.activateOneHotspot(kCaHairStyle3SpotID);
		g_allHotspots.deactivateOneHotspot(kCaBathroomMirrorSpotID);
		break;
	default:
		break;
	}
}

void CaldoriaMirror::clickInHotspot(const Input &input, const Hotspot *spot) {
	switch (spot->getObjectID()) {
	// The original made the player click through several interstitial screens before
	// reaching the hairstyle menu, which was unnecessary, so it's skipped here.
	case kCaHairStyle1SpotID:
		_owner->startExtraSequenceSync(kCaBathroomRetrothrash, kFilterNoInput);
		_owner->startExtraSequence(kCaBathroomRetrothrashReturn, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaHairStyle2SpotID:
		_owner->startExtraSequence(kCaBathroomAgencyStandard, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaHairStyle3SpotID:
		_owner->startExtraSequenceSync(kCaBathroomGeoWave, kFilterNoInput);
		_owner->startExtraSequence(kCaBathroomGeoWaveReturn, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		GameInteraction::clickInHotspot(input, spot);
		break;
	}
}

void CaldoriaMirror::receiveNotification(Notification *, const NotificationFlags) {
	switch (_owner->getLastExtra()) {
	case kCaBathroomRetrothrash:
	case kCaBathroomGeoWave:
		_owner->setCurrentActivation(kActivateMirrorReady);
		break;
	case kCaBathroomStylistIntro:
	case kCaBathroomRetrothrashReturn:
	case kCaBathroomGeoWaveReturn:
		_owner->setCurrentActivation(kActivateStylistReady);
		break;
	case kCaBathroomAgencyStandard:
		_owner->setCurrentActivation(kActivateHotSpotAlways);
		_owner->requestDeleteCurrentInteraction();
		GameState.setScoringFixedHair(true);
		GameState.setCaldoriaDoneHygiene(true);
		if (g_arthurChip)
			g_arthurChip->playArthurMovieForEvent("Images/AI/Globals/XGLOBA43", kArthurCaldoriaChoseAgencyHairStyle);
		break;
	default:
		break;
	}
}

} // End of namespace Pegasus
