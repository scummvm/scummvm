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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_SUBPLATFORM_H
#define PEGASUS_NEIGHBORHOOD_NORAD_SUBPLATFORM_H

#include "pegasus/interaction.h"
#include "pegasus/movie.h"
#include "pegasus/notification.h"
#include "pegasus/timers.h"

namespace Pegasus {

class SubPlatform : public GameInteraction, public NotificationReceiver {
public:
	SubPlatform(Neighborhood *);
	~SubPlatform() override {}

	void setSoundFXLevel(const uint16) override;

protected:
	void openInteraction() override;
	void initInteraction() override;
	void closeInteraction() override;

	void activateHotspots() override;
	void clickInHotspot(const Input &, const Hotspot *) override;

	void receiveNotification(Notification *, const NotificationFlags) override;

	Movie _platformMovie;
	TimeScale _platformScale;
	Notification _platformNotification;
	NotificationCallBack _platformCallBack;
	Notification *_neighborhoodNotification;
	uint16 _stateBits;
};

} // End of namespace Pegasus

#endif
