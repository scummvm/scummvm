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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_DELTA_GLOBEGAME_H
#define PEGASUS_NEIGHBORHOOD_NORAD_DELTA_GLOBEGAME_H

#include "pegasus/interaction.h"
#include "pegasus/movie.h"
#include "pegasus/notification.h"

namespace Pegasus {

enum GlobeTrackDirection {
	kTrackLeft,
	kTrackRight,
	kTrackUp,
	kTrackDown
};

// This class assumes that the globe movie is built at 15 frames per second with a
// time scale of 600, yielding 40 time unit per frame.

class GlobeTracker : public Tracker {
public:
	GlobeTracker(Movie *, Picture *, Picture *, Picture *, Picture *);
	~GlobeTracker() override {}

	void setTrackParameters(const Hotspot *, GlobeTrackDirection);
	void continueTracking(const Input &) override;
	void startTracking(const Input &) override;
	void stopTracking(const Input &) override;
	void activateHotspots() override;
	bool stopTrackingInput(const Input &) override;

protected:
	void trackGlobeMovie();
	void stopGlobeMovie();

	Movie *_globeMovie;
	Picture *_leftHighlight;
	Picture *_rightHighlight;
	Picture *_upHighlight;
	Picture *_downHighlight;
	const Hotspot *_trackSpot;
	int _trackTime;
	GlobeTrackDirection _trackDirection;
};

class GlobeCountdown : public IdlerAnimation {
public:
	GlobeCountdown(const DisplayElementID);
	~GlobeCountdown() override {}

	void setCountdownTime(const int);
	void startCountdown();
	void stopCountdown();

	void setDisplayOrder(const DisplayOrder);
	void show() override;
	void hide() override;
	void moveElementTo(const CoordType, const CoordType) override;

	void draw(const Common::Rect &) override;

protected:
	Surface _digits;
	int16 _digitOffset;
};

static const int16 kNumAllSilos = 40;
static const int16 kNumTargetSilos = 10;
static const int16 kNumLongSlices = 72;

class GlobeGame : public GameInteraction, public NotificationReceiver {
public:
	GlobeGame(Neighborhood *);
	~GlobeGame() override {}

	void setSoundFXLevel(const uint16) override;

	void handleInput(const Input &, const Hotspot *) override;
	void clickInHotspot(const Input &, const Hotspot *) override;
	void activateHotspots() override;

	bool canSolve() override;
	void doSolve() override;

	struct Point3D {
		float x, y, z;
	};

	struct Line3D {
		Point3D	pt1, pt2;
	};

protected:
	// Latitude (-90 - 90) and longitude (-180 - 180)
	static const int16 _siloCoords[kNumAllSilos][2];

	static const int16 _targetSilo[kNumTargetSilos];
	static const int16 _timeLimit[kNumTargetSilos];
	static const TimeValue _siloName[kNumTargetSilos][2];

	void openInteraction() override;
	void initInteraction() override;
	void closeInteraction() override;

	void receiveNotification(Notification *, const NotificationFlags) override;

	void spinGlobe(const Input &, const Hotspot *, GlobeTrackDirection);
	void clickGlobe(const Input &);

	int16 findClickedSilo(const Input &);

	void globeMovieFrameToOrigin(int16, int16 &, int16 &);
	void globePointToLatLong(const Point3D &, int16, int16, int16 &, int16 &);
	void screenPointTo3DPoint(int16, int16, Point3D &);
	bool lineHitsGlobe(const Line3D &, Point3D &);

	Movie _monitorMovie;
	Movie _globeMovie;
	Movie _upperNamesMovie;
	Movie _lowerNamesMovie;
	Notification _globeNotification;
	NotificationCallBack _monitorCallBack;
	GlobeTracker _globeTracker;
	Picture _globeCircleLeft;
	Picture _globeCircleRight;
	Picture _globeCircleUp;
	Picture _globeCircleDown;
	Picture _motionHighlightLeft;
	Picture _motionHighlightRight;
	Picture _motionHighlightUp;
	Picture _motionHighlightDown;
	Picture _targetHighlightUpperLeft;
	Picture _targetHighlightUpperRight;
	Picture _targetHighlightLowerLeft;
	Picture _targetHighlightLowerRight;
	GlobeCountdown _countdown;
	NotificationCallBack _countdownCallBack;
	int16 _gameState;
	int16 _currentSiloIndex;
	Notification *_neighborhoodNotification;
	bool _playedInstructions;
};

} // End of namespace Pegasus

#endif
