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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ZVISION_PUZZLE_H
#define ZVISION_PUZZLE_H

#include "common/list.h"

#include "zvision/object.h"

namespace ZVision {

/** How criteria should be decided */
enum CriteriaOperator {
	EQUAL_TO,
	NOT_EQUAL_TO,
	GREATER_THAN,
	LESS_THAN
};

/** Criteria for a Puzzle result to be fired */
struct Criteria {
	/** The id of a global state */
	uint32 id;
	/**  
	 * What we're comparing the value of the global state against
	 * This can either be a pure value or it can be the id of another global state
	 */
	uint32 argument;
	/** How to do the comparison */
	CriteriaOperator criteriaOperator;
	/** Is 'argument' the id of a global state or a pure value */
	bool argumentIsAnId;
};

/** What happens when Puzzle criteria are met */
enum ResultAction {
	ADD,
	ANIM_PLAY,
	ANIM_PRELOAD,
	ANIM_UNLOAD,
	ATTENUATE,
	ASSIGN,
	CHANGE_LOCATION,
	CROSSFADE,
	DEBUG,
	DELAY_RENDER,
	DISABLE_CONTROL,
	DISABLE_VENUS,
	DISPLAY_MESSAGE,
	DISSOLVE,
	DISTORT,
	ENABLE_CONTROL,
	FLUSH_MOUSE_EVENTS,
	INVENTORY,
	KILL,
	MENU_BAR_ENABLE,
	MUSIC,
	PAN_TRACK,
	PLAY_PRELOAD,
	PREFERENCES,
	QUIT,
	RANDOM,
	REGION,
	RESTORE_GAME,
	ROTATE_TO,
	SAVE_GAME,
	SET_PARTIAL_SCREEN,
	SET_SCREEN,
	SET_VENUS,
	STOP,
	STREAM_VIDEO,
	SYNC_SOUND,
	TIMER,
	TTY_TEXT,
	UNIVERSE_MUSIC,
};

/** What happens when Puzzle criteria are met */
struct Result {
	ResultAction action;
	Common::List<Object> arguments;
};

enum StateFlags : byte {
	ONCE_PER_INST = 0x01,
	DO_ME_NOW = 0x02,
	DISABLED = 0x04
};

struct Puzzle {
	uint32 id;
	Common::List<Criteria> criteriaList;
	Common::List<Result> resultList;
	byte flags;
};

} // End of namespace ZVision

#endif
