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

#ifndef PEGASUS_NEIGHBORHOOD_PREHISTORIC_H
#define PEGASUS_NEIGHBORHOOD_PREHISTORIC_H

#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

const TimeScale kPrehistoricMovieScale = 600;
const TimeScale kPrehistoricFramesPerSecond = 15;
const TimeScale kPrehistoricFrameDuration = 40;

//	Alternate IDs.

const tAlternateID kAltPrehistoricNormal = 0;
const tAlternateID kAltPrehistoricBridgeSet = 1;

//	Room IDs.

const tRoomID kPrehistoric01 = 0;
const tRoomID kPrehistoric02 = 1;
const tRoomID kPrehistoric03 = 2;
const tRoomID kPrehistoric04 = 3;
const tRoomID kPrehistoric05 = 4;
const tRoomID kPrehistoric06 = 5;
const tRoomID kPrehistoric07 = 6;
const tRoomID kPrehistoric08 = 7;
const tRoomID kPrehistoric09 = 8;
const tRoomID kPrehistoric10 = 9;
const tRoomID kPrehistoric11 = 10;
const tRoomID kPrehistoric12 = 11;
const tRoomID kPrehistoric13 = 12;
const tRoomID kPrehistoric14 = 13;
const tRoomID kPrehistoric15 = 14;
const tRoomID kPrehistoric16 = 15;
const tRoomID kPrehistoric17 = 16;
const tRoomID kPrehistoric18 = 17;
const tRoomID kPrehistoric19 = 18;
const tRoomID kPrehistoric20 = 19;
const tRoomID kPrehistoric21 = 20;
const tRoomID kPrehistoric22 = 21;
const tRoomID kPrehistoric22North = 22;
const tRoomID kPrehistoric23 = 23;
const tRoomID kPrehistoric24 = 24;
const tRoomID kPrehistoric25 = 25;
const tRoomID kPrehistoricDeath = 26;

//	Hot Spot Activation IDs.

const tHotSpotActivationID kActivationVaultClosed = 1;
const tHotSpotActivationID kActivationVaultOpen = 2;

//	Hot Spot IDs.

const tHotSpotID kPre18EastSpotID = 5000;
const tHotSpotID kPre22NorthSpotID = 5001;
const tHotSpotID kPre22NorthOutSpotID = 5002;
const tHotSpotID kPre22NorthBreakerSpotID = 5003;
const tHotSpotID kPrehistoricKeyDropSpotID = 5004;
const tHotSpotID kPrehistoricHistoricalLogSpotID = 5005;

//	Extra sequence IDs.

const tExtraID kPreArrivalFromTSA = 0;
const tExtraID kPre18EastBridgeOut = 1;
const tExtraID kPre18EastBridgeOn = 2;
const tExtraID kPre18EastZoom = 3;
const tExtraID kPre18EastZoomOut = 4;
const tExtraID kPre22ThrowBreaker = 5;
const tExtraID kPre25EastUnlockingVaultWithLog = 6;
const tExtraID kPre25EastVaultOpenWithLog = 7;
const tExtraID kPre25EastViewWithLog = 8;
const tExtraID kPre25EastUnlockingVaultNoLog = 9;
const tExtraID kPre25EastVaultOpenNoLog = 10;
const tExtraID kPre25EastViewNoLog = 11;

class PegasusEngine;

class Prehistoric : public Neighborhood {
public:
	Prehistoric(InputHandler *, PegasusEngine *);
	virtual ~Prehistoric() {}
	
	virtual uint16 getDateResID() const;
	virtual void init();
	
	virtual void arriveAt(const tRoomID, const tDirectionConstant);
	virtual void activateHotspots();
	virtual void clickInHotspot(const Input &, const Hotspot *);
	Common::String getBriefingMovie();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);

	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	void dropItemIntoRoom(Item *, Hotspot *);
	void pickedUpItem(Item *);

	void start();

	void bumpIntoWall();

	void checkContinuePoint(const tRoomID, const tDirectionConstant);

	bool canSolve();
	void doSolve();

protected:
	enum {
		kPrehistoricPrivateVaultOpenFlag,
		kPrehistoricPrivateExtendedBridgeFlag,
		kNumPrehistoricPrivateFlags
	};

	void setUpAIRules();
	int16 getStaticCompassAngle(const tRoomID, const tDirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	virtual void receiveNotification(Notification *, const tNotificationFlags);
	void turnTo(const tDirectionConstant);
	void zoomToVault();
	TimeValue getViewTime(const tRoomID, const tDirectionConstant);
	void findSpotEntry(const tRoomID, const tDirectionConstant, tSpotFlags, SpotTable::Entry &);

	void loadAmbientLoops();
	
	FlagsArray<byte, kNumPrehistoricPrivateFlags> _privateFlags;

	Common::String getNavMovieName();
};

} // End of namespace Pegasus

#endif
