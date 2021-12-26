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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_CANYONCHASE_H
#define PEGASUS_NEIGHBORHOOD_MARS_CANYONCHASE_H

#include "pegasus/chase.h"
#include "pegasus/fader.h"
#include "pegasus/movie.h"
#include "pegasus/sound.h"

namespace Pegasus {

class CanyonChase;
class Mars;

enum MusicTimerCode {
	kCanyonRanIntoWall,
	kCanyonExited,
	kCanyonFaded
};

struct MusicTimerEvent {
	CanyonChase *canyonChase;
	MusicTimerCode theEvent;

	void fire();
};

class CanyonChase : public ChaseInteraction {
friend class Mars;
friend struct MusicTimerEvent;
public:

	CanyonChase(Neighborhood *);
	virtual ~CanyonChase() {}

	void setSoundFXLevel(const uint16);
	void setAmbienceLevel(const uint16);

protected:

	void startCanyonMusicLoop();
	void stopCanyonMusicLoop(const long);

	void openInteraction();
	void initInteraction();
	void closeInteraction();

	void receiveNotification(Notification *, const NotificationFlags);

	void setUpBranch();
	void branchLeft();
	void branchRight();
	void dontBranch();

	void showControlsHint();
	void hideControlsHint();

	void switchTo(Movie &, NotificationCallBack &);
	void startMusicTimer(TimeValue, TimeScale, MusicTimerCode);
	void musicTimerExpired(MusicTimerEvent &);
	void doGenoChase();

	Movie _canyonMovie1;
	Movie _canyonMovie2;
	Movie _deathMovie;
	Movie _genoMovie;
	NotificationCallBack _canyon1CallBack;
	NotificationCallBack _canyon2CallBack;
	NotificationCallBack _deathCallBack;
	NotificationCallBack _genoCallBack;
	Sound _musicLoop;
	SoundFader _musicFader;
	FuseFunction _musicFuse;

	MusicTimerEvent _musicEvent;

	Movie *_currentMovie;
	NotificationCallBack *_currentCallBack;
	short _canyonState;
};

} // End of namespace Pegasus

#endif
