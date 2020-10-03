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

#include "common/scummsys.h"

#if defined(RISCOS) && defined(SDL_BACKEND)

#include "backends/events/riscossdl/riscossdl-events.h"
#include "backends/platform/sdl/riscos/riscos-utils.h"

#include "common/events.h"

#include <swis.h>

RISCOSSdlEventSource::RISCOSSdlEventSource()
    : SdlEventSource() {
	int messages[2];
	messages[0] = 3; // Message_DataLoad
	messages[1] = 0;
	_swix(Wimp_AddMessages, _IN(0), messages);

	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}

bool RISCOSSdlEventSource::handleSysWMEvent(SDL_Event &ev, Common::Event &event) {
	int eventCode = ev.syswm.msg->eventCode;
	int pollBlock[64];
	memcpy(pollBlock, ev.syswm.msg->pollBlock, 64 * sizeof(int));

	if (eventCode == 17 || eventCode == 18) {
		char *filename;
		switch (pollBlock[4]) {
		case 3: // Message_DataLoad
			filename = (char *)(pollBlock) + 44;
			event.type = Common::EVENT_DROP_FILE;
			event.path = RISCOS_Utils::toUnix(Common::String(filename));

			// Acknowledge that the event has been received
			pollBlock[4] = 4; // Message_DataLoadAck
			pollBlock[3] = pollBlock[2];
			_swix(Wimp_SendMessage, _INR(0,2), 19, pollBlock, 0);
			return true;
		}
	}
	return false;
}

#endif
