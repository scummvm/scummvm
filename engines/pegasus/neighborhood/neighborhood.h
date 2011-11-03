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
#include "pegasus/transition.h"
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

const TimeScale kDefaultLoopFadeScale = kThirtyTicksPerSecond;
const TimeValue kDefaultLoopFadeOut = kHalfSecondPerThirtyTicks;
const TimeValue kDefaultLoopFadeIn = kHalfSecondPerThirtyTicks;

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

class GameInteraction;
class Item;
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

class Neighborhood : public IDObject, public NotificationReceiver, public InputHandler, public Idler {
friend class StriderCallBack;
friend void timerFunction(FunctionPtr *, void *);

public:
	Neighborhood(InputHandler *nextHandler, PegasusEngine *vm, const Common::String &resName, tNeighborhoodID id);
	virtual ~Neighborhood();

	virtual void init();
	virtual void start();
	virtual void moveNavTo(const tCoordType, const tCoordType);
	virtual void checkContinuePoint(const tRoomID, const tDirectionConstant) = 0;
	void makeContinuePoint();

	virtual void activateHotspots();
	virtual void clickInHotspot(const Input &, const Hotspot *);

	virtual tCanMoveForwardReason canMoveForward(ExitTable::Entry &entry);
	virtual tCanTurnReason canTurn(tTurnDirection turn, tDirectionConstant &nextDir);
	virtual tCanOpenDoorReason canOpenDoor(DoorTable::Entry &entry);

	virtual void cantMoveThatWay(tCanMoveForwardReason);
	virtual void cantTurnThatWay(tCanTurnReason) {}
	virtual void cantOpenDoor(tCanOpenDoorReason);
	virtual void arriveAt(tRoomID room, tDirectionConstant direction);
	virtual void turnTo(const tDirectionConstant);
	virtual void spotCompleted();
	virtual void doorOpened();
	virtual void closeDoorOffScreen(const tRoomID, const tDirectionConstant) {}

	virtual void moveForward();
	virtual void turn(const tTurnDirection);
	virtual void turnLeft();
	virtual void turnRight();
	virtual void turnUp();
	virtual void turnDown();
	virtual void openDoor();
	virtual void zoomTo(const Hotspot *);

	virtual void updateViewFrame();
	
	void requestExtraSequence(const tExtraID, const tNotificationFlags, const tInputBits interruptionFilter);
	void requestSpotSound(const TimeValue, const TimeValue, const tInputBits interruptionFilter, const tNotificationFlags);
	void playSpotSoundSync(const TimeValue in, const TimeValue out);
	void requestDelay(const TimeValue, const TimeScale, const tInputBits interruptionFilter, const tNotificationFlags);

	Notification *getNeighborhoodNotification() { return &_neighborhoodNotification; }

	virtual void getExtraEntry(const uint32 id, ExtraTable::Entry &extraEntry);
	virtual void startSpotLoop(TimeValue, TimeValue, tNotificationFlags = 0);
	virtual bool actionQueueEmpty() { return _actionQueue.empty(); }
	virtual void showViewFrame(TimeValue);
	virtual void findSpotEntry(const tRoomID room, const tDirectionConstant direction, tSpotFlags flags, SpotTable::Entry &spotEntry);
	virtual void startExtraSequence(const tExtraID, const tNotificationFlags, const tInputBits interruptionFilter);
	bool startExtraSequenceSync(const tExtraID, const tInputBits);
	virtual void loopExtraSequence(const uint32, tNotificationFlags = 0);
	int32 getLastExtra() const { return _lastExtra; }
	virtual void scheduleNavCallBack(tNotificationFlags);
	
	Movie *getNavMovie() { return &_navMovie; }
	bool navMoviePlaying();
	
	void setCurrentAlternate(const tAlternateID alt) { _currentAlternate = alt; }
	tAlternateID getCurrentAlternate() const { return _currentAlternate; }
	
	void setCurrentActivation(const tHotSpotActivationID a) { _currentActivation = a; }
	tHotSpotActivationID getCurrentActivation() { return _currentActivation; }
	
	virtual void playDeathExtra(tExtraID, tDeathReason);
	virtual void die(const tDeathReason);

	virtual void setSoundFXLevel(const uint16);
	virtual void setAmbienceLevel(const uint16);
	
	void forceStridingStop(const tRoomID, const tDirectionConstant, const tAlternateID);
	void restoreStriding(const tRoomID, const tDirectionConstant, const tAlternateID);
	
	HotspotInfoTable::Entry *findHotspotEntry(const tHotSpotID);
	
	Push *getTurnPush() { return &_turnPush; }
	Picture *getTurnPushPicture() { return &_pushIn; }
	
	void hideNav();
	void showNav();
	
	virtual void loadAmbientLoops() {}
	
	virtual void flushGameState() {}

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

	virtual void loadLoopSound1(const Common::String &, const uint16 volume = 0x100,
			const TimeValue fadeOut = kDefaultLoopFadeOut, const TimeValue fadeIn = kDefaultLoopFadeIn,
			const TimeScale fadeScale = kDefaultLoopFadeScale);
	virtual void loadLoopSound2(const Common::String &, const uint16 volume = 0x100,
			const TimeValue fadeOut = kDefaultLoopFadeOut, const TimeValue fadeIn = kDefaultLoopFadeIn,
			const TimeScale fadeScale = kDefaultLoopFadeScale);
	bool loop1Loaded(const Common::String &soundName) { return _loop1SoundString == soundName; }
	bool loop2Loaded(const Common::String &soundName) { return _loop2SoundString == soundName; }
	void startLoop1Fader(const FaderMoveSpec &);
	void startLoop2Fader(const FaderMoveSpec &);

	virtual void takeItemFromRoom(Item *);
	virtual void dropItemIntoRoom(Item *, Hotspot *);
	virtual Hotspot *getItemScreenSpot(Item *, DisplayElement *) { return 0; }

	virtual GameInteraction *makeInteraction(const tInteractionID);
	virtual void requestDeleteCurrentInteraction() { _doneWithInteraction = true; }

	virtual uint16 getDateResID() const = 0;

	virtual void showExtraView(uint32);
	virtual void startExtraLongSequence(const uint32, const uint32, tNotificationFlags, const tInputBits interruptionFilter);
	
	void openCroppedMovie(const Common::String &, tCoordType, tCoordType);
	void loopCroppedMovie(const Common::String &, tCoordType, tCoordType);
	void closeCroppedMovie();
	void playCroppedMovieOnce(const Common::String &, tCoordType, tCoordType, const tInputBits interruptionFilter = kFilterNoInput);

	void playMovieSegment(Movie *, TimeValue = 0, TimeValue = 0xffffffff);

	virtual void recallToTSASuccess();
	virtual void recallToTSAFailure();
	
	virtual void pickedUpItem(Item *) {}

	virtual void handleInput(const Input &, const Hotspot *);
protected:
	PegasusEngine *_vm;
	Common::String _resName;
	
	virtual Common::String getSoundSpotsName() = 0;
	virtual Common::String getNavMovieName() = 0;

	// Notification function.
	virtual void receiveNotification(Notification *, const tNotificationFlags);

	// Map info functions.
	virtual void getExitEntry(const tRoomID room, const tDirectionConstant direction, ExitTable::Entry &entry);
	virtual TimeValue getViewTime(const tRoomID room, const tDirectionConstant direction);
	virtual void getDoorEntry(const tRoomID room, const tDirectionConstant direction, DoorTable::Entry &doorEntry);
	virtual tDirectionConstant getTurnEntry(const tRoomID room, const tDirectionConstant direction, const tTurnDirection turn);
	virtual void getZoomEntry(const tHotSpotID id, ZoomTable::Entry &zoomEntry);
	virtual void getHotspotEntry(const tHotSpotID id, HotspotInfoTable::Entry &hotspotEntry);

	// Nav movie sequences.
	virtual void startExitMovie(const ExitTable::Entry &);
	virtual void keepStriding(ExitTable::Entry &);
	virtual void stopStriding();
	virtual void checkStriding();
	virtual bool stillMoveForward();
	virtual void scheduleStridingCallBack(const TimeValue, tNotificationFlags flags);
	virtual void startZoomMovie(const ZoomTable::Entry &);
	virtual void startDoorOpenMovie(const TimeValue, const TimeValue);
	virtual void startTurnPush(const tTurnDirection, const TimeValue, const tDirectionConstant);
	virtual void playExtraMovie(const ExtraTable::Entry &, const tNotificationFlags, const tInputBits interruptionFilter);
	
	virtual void activateCurrentView(const tRoomID, const tDirectionConstant, tSpotFlags);
	
	virtual void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *);
	
	virtual void startSpotOnceOnly(TimeValue, TimeValue);
	
	virtual void startMovieSequence(const TimeValue, const TimeValue, tNotificationFlags,
  			bool loopSequence, const tInputBits interruptionFilter, const TimeValue strideStop = 0xffffffff);
	
	virtual void createNeighborhoodSpots();
	
	void resetLastExtra() { _lastExtra = -1; }
	
	virtual void throwAwayInterface();

	// Action queue stuff
	void popActionQueue();
	void serviceActionQueue();
	void requestAction(const tQueueRequestType, const tExtraID, const TimeValue, const TimeValue, const tInputBits, const tNotificationFlags);

	virtual bool prepareExtraSync(const tExtraID);
	virtual bool waitMovieFinish(Movie *, const tInputBits);
	
	virtual tInputBits getInputFilter();

	// Misc.
	virtual int16 getStaticCompassAngle(const tRoomID, const tDirectionConstant dir);
	virtual void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	virtual void getZoomCompassMove(const ZoomTable::Entry &, FaderMoveSpec&);
	virtual void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec&);

	virtual void setUpAIRules();
	virtual void setHotspotFlags(const tHotSpotID, const tHotSpotFlags);
	virtual void setIsItemTaken(const tItemID);

	virtual void upButton(const Input &);
	virtual void leftButton(const Input &);
	virtual void rightButton(const Input &);
	virtual void downButton(const Input &);

	void initOnePicture(Picture *, const Common::String &, tDisplayOrder, tCoordType, tCoordType, bool);
	void initOneMovie(Movie *, const Common::String &, tDisplayOrder, tCoordType, tCoordType, bool);

	void reinstateMonocleInterface();

	virtual void newInteraction(const tInteractionID);
	virtual void useIdleTime();
	virtual void bumpIntoWall();
	virtual void zoomUpOrBump();

	void scheduleEvent(const TimeValue, const TimeScale, const uint32);
	void cancelEvent();
	virtual void timerExpired(const uint32) {}
	bool isEventTimerRunning() { return _eventTimer.isFuseLit(); }
	uint32 getTimerEvent() { return _timerEvent; }

	void pauseTimer();
	void resumeTimer();
	bool timerPaused();

	// Navigation Data
	DoorTable _doorTable;
	ExitTable _exitTable;
	ExtraTable _extraTable;
	HotspotInfoTable _hotspotInfoTable;
	SpotTable _spotTable;
	TurnTable _turnTable;
	ViewTable _viewTable;
	ZoomTable _zoomTable;
	tAlternateID _currentAlternate;
	tHotSpotActivationID _currentActivation;
	
	int32 _lastExtra;
	tDeathReason _extraDeathReason;

	// Graphics
	Movie _navMovie;
	Picture _pushIn;
	Push _turnPush;

	// Callbacks
	Notification _neighborhoodNotification;
	NotificationCallBack _navMovieCallBack;
	StriderCallBack _stridingCallBack;
	NotificationCallBack _turnPushCallBack;
	NotificationCallBack _spotSoundCallBack;
	NotificationCallBack _delayCallBack;

	// Hotspots
	HotspotList _neighborhoodHotspots;

	// Sounds
	SoundTimeBase _spotSounds;

	// Action queue
	NeighborhoodActionQueue _actionQueue;
	TimeBase _delayTimer;

	// Interruptibility...
	tInputBits _interruptionFilter;

	// Nav hiding (for info support...)
	bool _isRunning;

	GameInteraction *_currentInteraction;
	bool _doneWithInteraction;
	Movie _croppedMovie;
	
	Sound _soundLoop1;
	Common::String _loop1SoundString;
	SoundFader _loop1Fader;
	
	Sound _soundLoop2;
	Common::String _loop2SoundString;
	SoundFader _loop2Fader;

	// The event timer...
	FuseFunction _eventTimer;
	uint32 _timerEvent;
};

extern Neighborhood *g_neighborhood;

} // End of namespace Pegasus

#endif
