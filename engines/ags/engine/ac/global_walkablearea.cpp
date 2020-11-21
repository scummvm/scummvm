//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "ac/global_walkablearea.h"
#include "ac/common.h"
#include "ac/common_defines.h"
#include "ac/draw.h"
#include "ac/walkablearea.h"
#include "debug/debug_log.h"
#include "game/roomstruct.h"

using namespace AGS::Common;

extern RoomStruct thisroom;


int GetScalingAt (int x, int y) {
    int onarea = get_walkable_area_pixel(x, y);
    if (onarea < 0)
        return 100;

    return get_area_scaling (onarea, x, y);
}

void SetAreaScaling(int area, int min, int max) {
    if ((area < 0) || (area > MAX_WALK_AREAS))
        quit("!SetAreaScaling: invalid walkalbe area");

    if (min > max)
        quit("!SetAreaScaling: min > max");

    if ((min < 5) || (max < 5) || (min > 200) || (max > 200))
        quit("!SetAreaScaling: min and max must be in range 5-200");

    // the values are stored differently
    min -= 100;
    max -= 100;

    if (min == max) {
        thisroom.WalkAreas[area].ScalingFar = min;
        thisroom.WalkAreas[area].ScalingNear = NOT_VECTOR_SCALED;
    }
    else {
        thisroom.WalkAreas[area].ScalingFar = min;
        thisroom.WalkAreas[area].ScalingNear = max;
    }
}

void RemoveWalkableArea(int areanum) {
  if ((areanum<1) | (areanum>15))
    quit("!RemoveWalkableArea: invalid area number specified (1-15).");
  play.walkable_areas_on[areanum]=0;
  redo_walkable_areas();
  debug_script_log("Walkable area %d removed", areanum);
}

void RestoreWalkableArea(int areanum) {
  if ((areanum<1) | (areanum>15))
    quit("!RestoreWalkableArea: invalid area number specified (1-15).");
  play.walkable_areas_on[areanum]=1;
  redo_walkable_areas();
  debug_script_log("Walkable area %d restored", areanum);
}

int GetWalkableAreaAtScreen(int x, int y) {
  VpPoint vpt = play.ScreenToRoomDivDown(x, y);
  if (vpt.second < 0)
    return 0;
  return GetWalkableAreaAtRoom(vpt.first.X, vpt.first.Y);
}

int GetWalkableAreaAtRoom(int x, int y) {
  int area = get_walkable_area_pixel(x, y);
  // IMPORTANT: disabled walkable areas are actually erased completely from the mask;
  // see: RemoveWalkableArea() and RestoreWalkableArea().
  return area >= 0 && area < (MAX_WALK_AREAS + 1) ? area : 0;
}

//=============================================================================

