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

#ifndef COMMON_TOUCHMAPPER_H
#define COMMON_TOUCHMAPPER_H

#include "common/scummsys.h"

#include "common/events.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/stack.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"

namespace Common {

class Touchmapper : public Common::DefaultEventMapper {
public:

	Touchmapper(EventManager *eventMan);
	~Touchmapper();

	// EventMapper interface
	virtual List<Event> mapEvent(const Event &ev, EventSource *source);
};

} // End of namespace Common

#endif // #ifndef COMMON_TOUCHMAPPER_H
