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

#ifndef DIRECTOR_LINGO_LINGO_H
#define DIRECTOR_LINGO_LINGO_H

#include "common/debug.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "audio/audiostream.h"
#include "common/str.h"
#include "engines/director/director.h"

namespace Director {

enum LEvent {
	kEventPrepareMovie,
	kEventStartMovie,
	kEventStopMovie,

	kEventNew,
	kEventBeginSprite,
	kEventEndSprite,

	kEventNone,
	kEventEnterFrame,
	kEventPrepareFrame,
	kEventIdle,
	kEventStepFrame,
	kEventExitFrame,

	kEventActivateWindow,
	kEventDeactivateWindow,
	kEventMoveWindow,
	kEventResizeWindow,
	kEventOpenWindow,
	kEventCloseWindow,

	kEventKeyUp,
	kEventKeyDown,
	kEventMouseUp,
	kEventMouseDown,
	kEventRightMouseUp,
	kEventRightMouseDown,
	kEventMouseEnter,
	kEventMouseLeave,
	kEventMouseUpOutSide,
	kEventMouseWithin,

	kEventStart
};

class Lingo {
public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void processEvent(LEvent event, int entityId);

	int parse(char *code);

	int func_mci(Common::String *s);

private:
	Common::HashMap<uint32, const char *> _eventHandlerTypes;
	Common::HashMap<Common::String, Audio::AudioStream *> _audioAliases;
	DirectorEngine *_vm;
};

} // End of namespace Director

#endif
