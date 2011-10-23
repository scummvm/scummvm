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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_NORADALPHA_H
#define PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_NORADALPHA_H

#include "pegasus/neighborhood/norad/norad.h"

namespace Pegasus {

const TimeValue kAlphaBumpIntoWallIn = 0;
const TimeValue kAlphaBumpIntoWallOut = 303;

const TimeValue kAlphaAccessDeniedIn = 303;
const TimeValue kAlphaAccessDeniedOut = 3045;

const TimeValue kAlphaRegDoorCloseIn = 3045;
const TimeValue kAlphaRegDoorCloseOut = 4476;

const TimeValue kAlphaElevatorDoorCloseIn = 4476;
const TimeValue kAlphaElevatorDoorCloseOut = 5071;

const TimeValue kAlphaCantTransportIn = 5071;
const TimeValue kAlphaCantTransportOut = 9348;

const TimeValue kPressureDoorIntro1In = 9348;
const TimeValue kPressureDoorIntro1Out = 11061;

const TimeValue kPressureDoorIntro2In = 11061;
const TimeValue kPressureDoorIntro2Out = 14098;

const TimeValue kN22ReplyIn = 14098;
const TimeValue kN22ReplyOut = 18442;

const TimeValue kLoadClawIntroIn = 18442;
const TimeValue kLoadClawIntroOut = 20698;

class Item;

class NoradAlpha : public Norad {
public:
	NoradAlpha(InputHandler *, PegasusEngine *);
	virtual ~NoradAlpha() {}

	virtual void init();
	void start();

	virtual bool okayToJump();

	void playClawMonitorIntro();

	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);

	void turnOnFillingStation();
	void turnOffFillingStation();
	Item *getFillingItem() { return _fillingStationItem; }
	bool gasCanisterIntake();

	virtual void takeItemFromRoom(Item *);
	virtual void dropItemIntoRoom(Item *, Hotspot *);

	virtual GameInteraction *makeInteraction(const tInteractionID);

	virtual void getClawInfo(tHotSpotID &outSpotID, tHotSpotID &prepSpotID, tHotSpotID &clawControlSpotID,
			tHotSpotID &pinchClawSpotID, tHotSpotID &moveClawDownSpotID, tHotSpotID &moveClawRightSpotID,
			tHotSpotID &moveClawLeftSpotID, tHotSpotID &moveClawUpSpotID, tHotSpotID &clawCCWSpotID,
			tHotSpotID &clawCWSpotID, uint32 &, const uint32 *&);

	void loadAmbientLoops();

	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);
	void setUpAIRules();

	void setSubPrepFailed(bool value) { _subPrepFailed = value; }
	bool getSubPrepFailed() { return _subPrepFailed; }

	void closeDoorOffScreen(const tRoomID, const tDirectionConstant);
	void findSpotEntry(const tRoomID, const tDirectionConstant, tSpotFlags, SpotTable::Entry &);
	void clickInHotspot(const Input &, const Hotspot *);

	void checkContinuePoint(const tRoomID, const tDirectionConstant);

	bool canSolve();
	void doSolve();

protected:
	static const uint32 _noradAlphaClawExtras[22];

	virtual void arriveAtNorad01();
	virtual void arriveAtNorad01East();
	virtual void arriveAtNorad01West();
	virtual void arriveAtNorad04();
	virtual void arriveAtNorad22();
	
	virtual void arriveAt(const tRoomID, const tDirectionConstant);
	
	virtual void getZoomEntry(const tHotSpotID, ZoomTable::Entry &);
	virtual TimeValue getViewTime(const tRoomID, const tDirectionConstant);
	
	virtual void receiveNotification(Notification *, const tNotificationFlags);

	virtual void activateHotspots();

	Hotspot *getItemScreenSpot(Item *, DisplayElement *);

	void bumpIntoWall();

	Item *_fillingStationItem;

	bool _subPrepFailed;

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();
};

} // End of namespace Pegasus

#endif
