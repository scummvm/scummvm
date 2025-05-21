/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "zvision/scripting/scroller.h"

namespace ZVision {

Scroller::Scroller(const Common::Point &activePos, const Common::Point &idlePos, int16 period) :
  _activePos(activePos), 
  _idlePos(idlePos),
  _period(period) {
  deltaPos = _activePos - _idlePos;
	Pos = _idlePos;
	prevPos = Pos;
}

Scroller::~Scroller() {
}

bool Scroller::update(uint32 deltatime) {
  prevPos = Pos;

  Common::Point targetPos;
	float dx=0;
	float dy=0;
	
	if (active)
	  targetPos = _activePos;
  else
  	targetPos = _idlePos;

  if (Pos.x != targetPos.x) {
    dx = (deltaPos.x * deltatime) / _period;
    if(dx == 0)
      dx = 1;
  }
  if (Pos.y != targetPos.y) {
    dy = (deltaPos.y * deltatime) / _period;
    if(dy == 0)
      dy = 1;
  }
  
  if(!active) {
    dx = -dx;
    dy = -dy;
  }
  
  Pos.x += (int)dx;
  if( (dx == 0) || ( (dx > 0) && (Pos.x > targetPos.x) ) || ( (dx < 0) && (Pos.x < targetPos.x) ) )
    Pos.x = targetPos.x;
    
  Pos.y += (int)dy;
  if( (dy == 0) || ( (dy > 0) && (Pos.y > targetPos.y) ) || ( (dy < 0) && (Pos.y < targetPos.y) ) )
    Pos.y = targetPos.y;
	
	scrolled = (Pos == _activePos);

	return (Pos != prevPos);  //True if redraw necessary
}

} // End of namespace ZVision
