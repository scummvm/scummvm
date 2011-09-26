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

#ifndef PEGASUS_NEIGHBORHOOD_H
#define PEGASUS_NEIGHBORHOOD_H

#include "common/queue.h"
#include "common/str.h"

#include "pegasus/fader.h"
#include "pegasus/hotspot.h"
#include "pegasus/input.h"
#include "pegasus/movie.h"
#include "pegasus/notification.h"
#include "pegasus/sound.h"
#include "pegasus/timers.h"
#include "pegasus/util.h"
#include "pegasus/neighborhood/door.h"
#include "pegasus/neighborhood/exit.h"
#include "pegasus/neighborhood/extra.h"
#include "pegasus/neighborhood/hotspotinfo.h"
#include "pegasus/neighborhood/spot.h"
#include "pegasus/neighborhood/turn.h"
#include "pegasus/neighborhood/view.h"
#include "pegasus/neighborhood/zoom.h"

namespace Pegasus {

class PegasusEngine;

// Pegasus Prime neighborhood id's
const tNeighborhoodID kCaldoriaID = 0;
const tNeighborhoodID kFullTSAID = 1;
const tNeighborhoodID kFinalTSAID = 2;
const tNeighborhoodID kTinyTSAID = 3;
const tNeighborhoodID kPrehistoricID = 4;
const tNeighborhoodID kMarsID = 5;
const tNeighborhoodID kWSCID = 6;
const tNeighborhoodID kNoradAlphaID = 7;
const tNeighborhoodID kNoradDeltaID = 8;
// The sub chase is not really a neighborhood, but we define a constant that is used
// to allow an easy transition out of Norad Alpha.
const tNeighborhoodID kNoradSubChaseID = 1000;

enum tQueueRequestType {
	kNavExtraRequest,
	kSpotSoundRequest,
	kDelayRequest
};

// For delay requests, start is interpreted as the total delay and stop is interpreted
// as the scale the delay is in.
// For extra requests, start and stop are not used.
struct tQueueRequest {
	tQueueRequestType requestType;
	tExtraID extra;
	TimeValue start, stop;
	tInputBits interruptionFilter;
	bool playing;
	tNotificationFlags flags;
	Notification *notification;
};

bool operator==(const tQueueRequest &arg1, const tQueueRequest &arg2);
bool operator!=(const tQueueRequest &arg1, const tQueueRequest &arg2);

class Neighborhood;

class StriderCallBack : public TimeBaseCallBack {
public:
	StriderCallBack(Neighborhood *);
	virtual ~StriderCallBack() {}

protected:
	virtual void callBack();

	Neighborhood *_neighborhood;
};

typedef Common::Queue<tQueueRequest> NeighborhoodActionQueue;

class Neighborhood : public IDObject, public NotificationReceiver, public InputHandler {
friend class StriderCallBack;

public:
	Neighborhood(InputHandler *nextHandler, PegasusEngine *vm, const Common::String &resName, tNeighborhoodID id);
	virtual ~Neighborhood();

	virtual void init();
	void start();

	void arriveAt(tRoomID room, tDirectionConstant direction);

	virtual void getExitEntry(const tRoomID room, const tDirectionConstant direction, ExitTable::Entry &entry);
	virtual TimeValue getViewTime(const tRoomID room, const tDirectionConstant direction);
	virtual void getDoorEntry(const tRoomID room, const tDirectionConstant direction, DoorTable::Entry &doorEntry);
	virtual tDirectionConstant getTurnEntry(const tRoomID room, const tDirectionConstant direction, const tTurnDirection turn);
	virtual void findSpotEntry(const tRoomID room, const tDirectionConstant direction, tSpotFlags flags, SpotTable::Entry &spotEntry);
	virtual void getZoomEntry(const tHotSpotID id, ZoomTable::Entry &zoomEntry);
	virtual void getHotspotEntry(const tHotSpotID id, HotspotInfoTable::Entry &hotspotEntry);
	virtual void getExtraEntry(const uint32 id, ExtraTable::Entry &extraEntry);

	tCanMoveForwardReason canMoveForward(ExitTable::Entry &entry);
	tCanTurnReason canTurn(tTurnDirection turn, tDirectionConstant &nextDir);
	tCanOpenDoorReason canOpenDoor(DoorTable::Entry &entry);

	void requestExtraSequence(const tExtraID, const tNotificationFlags, const tInputBits interruptionFilter);
	void requestSpotSound(const TimeValue, const TimeValue, const tInputBits interruptionFilter, const tNotificationFlags);
	void requestDelay(const TimeValue, const TimeScale, const tInputBits interruptionFilter, const tNotificationFlags);

	virtual bool actionQueueEmpty() { return _actionQueue.empty(); }

	virtual Common::String getBriefingMovie();
	virtual Common::String getEnvScanMovie();
	virtual uint getNumHints();
	virtual Common::String getHintMovie(uint);
	virtual bool canSolve();
	virtual void prepareForAIHint(const Common::String &) {}
	virtual void cleanUpAfterAIHint(const Common::String &) {}
	virtual void doSolve();

	virtual bool okayToJump();

	virtual tAirQuality getAirQuality(const tRoomID);
	virtual void checkAirMask() {}
	virtual void checkFlashlight() {}
	virtual void shieldOn() {}
	virtual void shieldOff() {}

	virtual void scheduleNavCallBack(tNotificationFlags);

protected:
	PegasusEngine *_vm;
	Common::String _resName;

	virtual void receiveNotification(Notification *, const tNotificationFlags);

	virtual void createNeighborhoodSpots();
	virtual void loadSoundSpots();

	// Nav movie sequences.
	virtual void checkStriding();
	virtual void keepStriding(ExitTable::Entry &);
	virtual void stopStriding();
	virtual bool stillMoveForward();
	virtual void scheduleStridingCallBack(const TimeValue, tNotificationFlags flags);

	// Action queue stuff
	void popActionQueue();
	void serviceActionQueue();
	void requestAction(const tQueueRequestType, const tExtraID, const TimeValue, const TimeValue, const tInputBits, const tNotificationFlags);

	// Navigation Data
	DoorTable _doorTable;
	ExitTable _exitTable;
	ExtraTable _extraTable;
	HotspotInfoTable _hotspotInfoTable;
	SpotTable _spotTable;
	TurnTable _turnTable;
	ViewTable _viewTable;
	ZoomTable _zoomTable;
	virtual uint16 getStaticCompassAngle(const tRoomID, const tDirectionConstant dir);
	virtual void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);

	// Graphics
	Movie _navMovie;

	// Callbacks
	Notification _neighborhoodNotification;
	NotificationCallBack _navMovieCallBack;
	StriderCallBack _stridingCallBack;

	tAlternateID _currentAlternate;

	HotspotList _neighborhoodHotspots;

	NeighborhoodActionQueue _actionQueue;

	Sound _spotSounds;

	tInputBits _interruptionFilter;
};

extern Neighborhood *g_neighborhood;

} // End of namespace Pegasus

#endif
