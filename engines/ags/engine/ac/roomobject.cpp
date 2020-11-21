/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "ac/roomobject.h"
#include "ac/common.h"
#include "ac/common_defines.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/runtime_defines.h"
#include "ac/viewframe.h"
#include "main/update.h"
#include "util/stream.h"

using AGS::Common::Stream;

extern ViewStruct*views;
extern GameState play;
extern GameSetupStruct game;

RoomObject::RoomObject()
{
    x = y = 0;
    transparent = 0;
    tint_r = tint_g = 0;
    tint_b = tint_level = 0;
    tint_light = 0;
    zoom = 0;
    last_width = last_height = 0;
    num = 0;
    baseline = 0;
    view = loop = frame = 0;
    wait = moving = 0;
    cycling = 0;
    overall_speed = 0;
    on = 0;
    flags = 0;
    blocking_width = blocking_height = 0;
}

int RoomObject::get_width() {
    if (last_width == 0)
        return game.SpriteInfos[num].Width;
    return last_width;
}
int RoomObject::get_height() {
    if (last_height == 0)
        return game.SpriteInfos[num].Height;
    return last_height;
}
int RoomObject::get_baseline() {
    if (baseline < 1)
        return y;
    return baseline;
}

void RoomObject::UpdateCyclingView()
{
	if (on != 1) return;
    if (moving>0) {
      do_movelist_move(&moving,&x,&y);
      }
    if (cycling==0) return;
    if (view<0) return;
    if (wait>0) { wait--; return; }

    if (cycling >= ANIM_BACKWARDS) {

      update_cycle_view_backwards();

    }
    else {  // Animate forwards
      
	  update_cycle_view_forwards();

    }  // end if forwards

    ViewFrame*vfptr=&views[view].loops[loop].frames[frame];
    num = vfptr->pic;

    if (cycling == 0)
      return;

    wait=vfptr->speed+overall_speed;
    CheckViewFrame (view, loop, frame);
}


void RoomObject::update_cycle_view_forwards()
{
	frame++;
      if (frame >= views[view].loops[loop].numFrames) {
        // go to next loop thing
        if (views[view].loops[loop].RunNextLoop()) {
          if (loop+1 >= views[view].numLoops)
            quit("!Last loop in a view requested to move to next loop");
          loop++;
          frame=0;
        }
        else if (cycling % ANIM_BACKWARDS == ANIM_ONCE) {
          // leave it on the last frame
          cycling=0;
          frame--;
          }
        else {
          if (play.no_multiloop_repeat == 0) {
            // multi-loop anims, go back to start of it
            while ((loop > 0) && 
              (views[view].loops[loop - 1].RunNextLoop()))
              loop --;
          }
          if (cycling % ANIM_BACKWARDS == ANIM_ONCERESET)
            cycling=0;
          frame=0;
        }
      }
}

void RoomObject::update_cycle_view_backwards()
{
	// animate backwards
      frame--;
      if (frame < 0) {
        if ((loop > 0) && 
           (views[view].loops[loop - 1].RunNextLoop())) 
        {
          // If it's a Go-to-next-loop on the previous one, then go back
          loop --;
          frame = views[view].loops[loop].numFrames - 1;
        }
        else if (cycling % ANIM_BACKWARDS == ANIM_ONCE) {
          // leave it on the first frame
          cycling = 0;
          frame = 0;
        }
        else { // repeating animation
          frame = views[view].loops[loop].numFrames - 1;
        }
      }
}

void RoomObject::ReadFromFile(Stream *in)
{
    in->ReadArrayOfInt32(&x, 3);
    in->ReadArrayOfInt16(&tint_r, 15);
    in->ReadArrayOfInt8((int8_t*)&cycling, 4);
    in->ReadArrayOfInt16(&blocking_width, 2);
}

void RoomObject::WriteToFile(Stream *out) const
{
    out->WriteArrayOfInt32(&x, 3);
    out->WriteArrayOfInt16(&tint_r, 15);
    out->WriteArrayOfInt8((int8_t*)&cycling, 4);
    out->WriteArrayOfInt16(&blocking_width, 2);
}
