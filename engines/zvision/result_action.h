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

#ifndef ZVISION_RESULT_ACTION_H
#define ZVISION_RESULT_ACTION_H

#include "common/scummsys.h"

#include "zvision/zvision.h"

namespace ZVision {

class ResultAction {
public:
	virtual ~ResultAction() {}
	virtual bool execute(ZVision *zVision) = 0;
};


// The different types of actions
// ADD,
// ANIM_PLAY,
// ANIM_PRELOAD,
// ANIM_UNLOAD,
// ATTENUATE,
// ASSIGN,
// CHANGE_LOCATION,
// CROSSFADE,
// DEBUG,
// DELAY_RENDER,
// DISABLE_CONTROL,
// DISABLE_VENUS,
// DISPLAY_MESSAGE,
// DISSOLVE,
// DISTORT,
// ENABLE_CONTROL,
// FLUSH_MOUSE_EVENTS,
// INVENTORY,
// KILL,
// MENU_BAR_ENABLE,
// MUSIC,
// PAN_TRACK,
// PLAY_PRELOAD,
// PREFERENCES,
// QUIT,
// RANDOM,
// REGION,
// RESTORE_GAME,
// ROTATE_TO,
// SAVE_GAME,
// SET_PARTIAL_SCREEN,
// SET_SCREEN,
// SET_VENUS,
// STOP,
// STREAM_VIDEO,
// SYNC_SOUND,
// TIMER,
// TTY_TEXT,
// UNIVERSE_MUSIC,

class ActionAdd : public ResultAction {
public:
	ActionAdd(Common::String line);
	bool execute(ZVision *zVision);

private:
	uint32 _key;
	byte _value;
};

class ActionPlayAnimation : public ResultAction {
public:
	ActionPlayAnimation(Common::String line);
	bool execute(ZVision *zVision);
};

class ActionPreloadAnimation : public ResultAction {
public:
	ActionPreloadAnimation(Common::String line);
	bool execute(ZVision *zVision);
};

class ActionAttenuate : public ResultAction {
public:
	ActionAttenuate(Common::String line);
	bool execute(ZVision *zVision);
};

class ActionAssign : public ResultAction {
public:
	ActionAssign(Common::String line);
	bool execute(ZVision *zVision);

private:
	uint32 _key;
	byte _value;
};

class ActionRandom : public ResultAction {
public:
	ActionRandom(Common::String line);
	bool execute(ZVision *zVision);

private:
	uint32 _key;
	uint32 _max;
};

} // End of namespace ZVision

#endif
