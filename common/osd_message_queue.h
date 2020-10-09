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

#ifndef COMMON_OSD_MESSAGE_QUEUE_H
#define COMMON_OSD_MESSAGE_QUEUE_H

#include "common/events.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/queue.h"
#include "common/mutex.h"

namespace Common {

/**
 * @defgroup common_osd_message_queue OSD message queue
 * @ingroup common
 *
 * @brief API for managing the queue of On Screen Display (OSD) messages.
 * @{
 */

/**
 * Queue OSD messages from any thread to be displayed by the graphic thread.
 */
class OSDMessageQueue : public Singleton<OSDMessageQueue>, public EventSource {
public:
	OSDMessageQueue();
	~OSDMessageQueue();

	void registerEventSource();

	enum {
		kMinimumDelay = 1000 /** < Minimum delay between two OSD messages (in milliseconds) */
	};

	/**
	 * Add a message to the OSD message queue.
	 */
	void addMessage(const Common::U32String &msg);

	/**
	 * Common::EventSource interface
	 *
	 * The OSDMessageQueue registers itself as an event source even if it does not
	 * actually produce events as a mean to be polled periodically by the GUI or
	 * engine code.
	 *
	 * The periodical polling is used to update the OSD messages.
	 */
	virtual bool pollEvent(Common::Event &event) override;

private:
	Mutex _mutex;
	Queue<U32String> _messages;
	uint32 _lastUpdate;
};

/** @} */

} // End of namespace Common

#endif
