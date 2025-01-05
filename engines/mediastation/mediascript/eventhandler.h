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

#ifndef MEDIASTATION_MEDIASCRIPT_EVENTHANDLER_H
#define MEDIASTATION_MEDIASCRIPT_EVENTHANDLER_H

#include "mediastation/datafile.h"
#include "mediastation/datum.h"
#include "mediastation/mediascript/codechunk.h"

namespace MediaStation {

enum EventType {
    // TIMER EVENTS.
    kTimerEvent = 5,

    // HOTSPOT EVENTS.
    kMouseDownEvent = 6,
    kMouseUpEvent = 7,
    kMouseMovedEvent = 8,
    kMouseEnteredEvent = 9,
    kMouseExitedEvent = 10,
    kKeyDownEvent = 13, // PARAMS: 1 - ASCII code.

    // SOUND EVENTS.
    kSoundEndEvent = 14,
    kSoundAbortEvent = 19,
    kSoundFailureEvent = 20,
    kSoundStoppedEvent = 29,
    kSoundBeginEvent = 30,

    // MOVIE EVENTS.
    kMovieEndEvent = 15,
    kMovieAbortEvent = 21,
    kMovieFailureEvent = 22,
    kMovieStoppedEvent = 31,
    kMovieBeginEvent = 32,

    //SPRITE EVENTS.
    // Just "MovieEnd" in source.
    kSpriteMovieEndEvent = 23,

    // SCREEN EVENTS.
    kEntryEvent = 17,
    kExitEvent = 27,

    // CONTEXT EVENTS.
    kLoadCompleteEvent = 44, // PARAMS: 1 - Context ID

    // TEXT EVENTS.
    kInputEvent = 37,
    kErrorEvent = 38,

    // CAMERA EVENTS.
    kPanAbortEvent = 43,
    kPanEndEvent = 42,

    // PATH EVENTS.
    kStepEvent = 28,
    kPathStoppedEvent = 33,
    kPathEndEvent = 16
};

enum EventHandlerArgumentType {
    kNullEventHandlerArgument = 0,
    kAsciiCodeEventHandlerArgument = 1, // TODO: Why is this datum type a float?
    kTimeEventHandlerArgument = 3,
    kUnk1EventHandlerArgument = 4, // Appars to happen with MovieStart?
    kContextEventHandlerArgument = 5
};

class EventHandler {
public:
	EventHandler(Chunk &chunk);
	~EventHandler();

	Operand execute(uint assetId);
	EventType _type;
	EventHandlerArgumentType _argumentType;
	Datum _argumentValue;

private:
	CodeChunk *_code = nullptr;
};

} // End of namespace MediaStation

#endif
