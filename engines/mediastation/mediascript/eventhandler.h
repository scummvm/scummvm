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

#include "common/hashmap.h"
#include "common/func.h"

#include "mediastation/datafile.h"
#include "mediastation/datum.h"

#include "mediastation/mediascript/codechunk.h"

namespace MediaStation {

class EventHandler {
public:
	enum class Type {
		// TIMER EVENTS.
		Time = 5,

		// HOTSPOT EVENTS.
		MouseDown = 6,
		MouseUp = 7,
		MouseMoved = 8,
		MouseEntered = 9,
		MouseExited = 10,
		KeyDown = 13, // PARAMS: 1 - ASCII code.

		// SOUND EVENTS.
		SoundEnd = 14,
		SoundAbort = 19,
		SoundFailure = 20,
		SoundStopped = 29,
		SoundBegin = 30,

		// MOVIE EVENTS.
		MovieEnd = 15,
		MovieAbort = 21,
		MovieFailure = 22,
		MovieStopped = 31,
		MovieBegin = 32,

		//SPRITE EVENTS.
		// Just "MovieEnd" in source.
		SpriteMovieEnd = 23,

		// SCREEN EVENTS.
		Entry = 17,
		Exit = 27,

		// CONTEXT EVENTS.
		LoadComplete = 44, // PARAMS: 1 - Context ID

		// TEXT EVENTS.
		Input = 37,
		Error = 38,

		// CAMERA EVENTS.
		PanAbort = 43,
		PanEnd = 42,

		// PATH EVENTS.
		Step = 28,
		PathStopped = 33,
		PathEnd = 16
	};

	enum class ArgumentType {
		Null = 0,
		AsciiCode = 1, // TODO: Why is this datum type a float?
		Time = 3,
		Unk1 = 4, // Appars to happen with MovieStart?
		Context = 5
	};

	EventHandler(Chunk &chunk);
	~EventHandler();

	Operand execute(uint assetId);
	EventHandler::Type _type;
	EventHandler::ArgumentType _argumentType;
	Datum _argumentValue;

private:
	CodeChunk *_code = nullptr;
};

} // End of namespace MediaStation

#endif
