/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-2013 Presto Studios, Inc.
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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_TUNNELPOD_H
#define PEGASUS_NEIGHBORHOOD_MARS_TUNNELPOD_H

#include "pegasus/chase.h"
#include "pegasus/movie.h"

namespace Pegasus {

class Mars;

class TunnelPod : public ChaseInteraction {
friend class Mars;
friend struct MusicTimerEvent;
public:

	TunnelPod(Neighborhood *);
	virtual ~TunnelPod() {}

	void setSoundFXLevel(const uint16);

protected:

	void openInteraction();
	void initInteraction();
	void closeInteraction();

	void receiveNotification(Notification *, const NotificationFlags);

	void setUpBranch();
	void branchLeft();
	void branchRight();
	void dontBranch();

	void switchTo(Movie &, NotificationCallBack &);

	Movie _tunnelMainMovie;
	Movie _tunnelAltMovie;
	Movie _deathMovie;
	NotificationCallBack _tunnelCallBack;
	NotificationCallBack _tunnelMainCallBack;
	NotificationCallBack _tunnelAltCallBack;
	NotificationCallBack _deathCallBack;

	Movie *_currentMovie;
	NotificationCallBack *_currentCallBack;
	short _tunnelState;
};

} // End of namespace Pegasus

#endif
