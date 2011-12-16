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

const AlternateID kAltPrehistoricNormal = 0;
const AlternateID kAltPrehistoricBridgeSet = 1;

//	Room IDs.

const RoomID kPrehistoric01 = 0;
const RoomID kPrehistoric02 = 1;
const RoomID kPrehistoric03 = 2;
const RoomID kPrehistoric04 = 3;
const RoomID kPrehistoric05 = 4;
const RoomID kPrehistoric06 = 5;
const RoomID kPrehistoric07 = 6;
const RoomID kPrehistoric08 = 7;
const RoomID kPrehistoric09 = 8;
const RoomID kPrehistoric10 = 9;
const RoomID kPrehistoric11 = 10;
const RoomID kPrehistoric12 = 11;
const RoomID kPrehistoric13 = 12;
const RoomID kPrehistoric14 = 13;
const RoomID kPrehistoric15 = 14;
const RoomID kPrehistoric16 = 15;
const RoomID kPrehistoric17 = 16;
const RoomID kPrehistoric18 = 17;
const RoomID kPrehistoric19 = 18;
const RoomID kPrehistoric20 = 19;
const RoomID kPrehistoric21 = 20;
const RoomID kPrehistoric22 = 21;
const RoomID kPrehistoric22North = 22;
const RoomID kPrehistoric23 = 23;
const RoomID kPrehistoric24 = 24;
const RoomID kPrehistoric25 = 25;
const RoomID kPrehistoricDeath = 26;

//	Hot Spot Activation IDs.

const HotSpotActivationID kActivationVaultClosed = 1;
const HotSpotActivationID kActivationVaultOpen = 2;

//	Hot Spot IDs.

const HotSpotID kPre18EastSpotID = 5000;
const HotSpotID kPre22NorthSpotID = 5001;
const HotSpotID kPre22NorthOutSpotID = 5002;
const HotSpotID kPre22NorthBreakerSpotID = 5003;
const HotSpotID kPrehistoricKeyDropSpotID = 5004;
const HotSpotID kPrehistoricHistoricalLogSpotID = 5005;

//	Extra sequence IDs.

const ExtraID kPreArrivalFromTSA = 0;
const ExtraID kPre18EastBridgeOut = 1;
const ExtraID kPre18EastBridgeOn = 2;
const ExtraID kPre18EastZoom = 3;
const ExtraID kPre18EastZoomOut = 4;
const ExtraID kPre22ThrowBreaker = 5;
const ExtraID kPre25EastUnlockingVaultWithLog = 6;
const ExtraID kPre25EastVaultOpenWithLog = 7;
const ExtraID kPre25EastViewWithLog = 8;
const ExtraID kPre25EastUnlockingVaultNoLog = 9;
const ExtraID kPre25EastVaultOpenNoLog = 10;
const ExtraID kPre25EastViewNoLog = 11;

class PegasusEngine;

class Prehistoric : public Neighborhood {
public:
	Prehistoric(InputHandler *, PegasusEngine *);
	virtual ~Prehistoric() {}
	
	virtual uint16 getDateResID() const;
	virtual void init();
	
	virtual void arriveAt(const RoomID, const DirectionConstant);
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

	void checkContinuePoint(const RoomID, const DirectionConstant);

	bool canSolve();
	void doSolve();

protected:
	enum {
		kPrehistoricPrivateVaultOpenFlag,
		kPrehistoricPrivateExtendedBridgeFlag,
		kNumPrehistoricPrivateFlags
	};

	void setUpAIRules();
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	virtual void receiveNotification(Notification *, const NotificationFlags);
	void turnTo(const DirectionConstant);
	void zoomToVault();
	TimeValue getViewTime(const RoomID, const DirectionConstant);
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &);

	void loadAmbientLoops();
	
	FlagsArray<byte, kNumPrehistoricPrivateFlags> _privateFlags;

	Common::String getNavMovieName();
	Common::String getSoundSpotsName();
};

} // End of namespace Pegasus

#endif
