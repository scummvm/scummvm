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

#include "common/str.h"

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
//	The sub chase is not really a neighborhood, but we define a constant that is used
//	to allow an easy transition out of Norad Alpha.
const tNeighborhoodID kNoradSubChaseID = 1000;

class Neighborhood {
public:
	Neighborhood(PegasusEngine *vm, const Common::String &resName, tNeighborhoodID id);
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

protected:
	PegasusEngine *_vm;
	Common::String _resName;
	tNeighborhoodID _neighborhoodID;

	DoorTable _doorTable;
	ExitTable _exitTable;
	ExtraTable _extraTable;
	HotspotInfoTable _hotspotInfoTable;
	SpotTable _spotTable;
	TurnTable _turnTable;
	ViewTable _viewTable;
	ZoomTable _zoomTable;

	tAlternateID _currentAlternate;
};

} // End of namespace Pegasus

#endif
