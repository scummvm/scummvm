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

#ifndef PINK_HANDLER_TIMER_H
#define PINK_HANDLER_TIMER_H

#include <common/str-array.h>
#include "handler.h"

namespace Pink {

class LeadActor;

// This class has difference in games
class HandlerTimer : public Handler {
    virtual void handle(LeadActor *actor) = 0;
};

class HandlerTimerActions : public HandlerTimer {
    virtual void deserialize(Archive &archive);
    virtual void handle(LeadActor *actor);

private:
    Common::StringArray _actions;
};

class HandlerTimerSequences : public HandlerTimer {
    virtual void deserialize(Archive &archive);
    virtual void handle(LeadActor *actor);

private:
    Common::StringArray _sequences;
};

} // End of namespace Pink


#endif
