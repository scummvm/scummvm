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

#ifndef CHEWY_EVENTS_H
#define CHEWY_EVENTS_H

#include "chewy/events_base.h"

namespace Chewy {

class EventsManager : public EventsBase {
private:
	void init_timer_handler();
	static void timer_handler();

	void handleMouseEvent(const Common::Event &event);
	void handleKbdEvent(const Common::Event &event);
protected:
	void handleEvent(const Common::Event &event) override;
public:
	Common::Point _mousePos;
	bool _flag1 = false;
	bool _flag2 = false;

	EventsManager(Graphics::Screen *screen);
	virtual ~EventsManager();

	void delay(size_t time);
};

extern EventsManager *g_events;

} // namespace Chewy

#endif
