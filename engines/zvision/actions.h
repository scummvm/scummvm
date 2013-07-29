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

#include "common/str.h"

namespace ZVision {

// Forward declaration of ZVision. This file is included before ZVision is declared
class ZVision;

class ResultAction {
public:
	virtual ~ResultAction() {}
	virtual bool execute(ZVision *engine) = 0;
};


// The different types of actions
// DEBUG,
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
// TTY_TEXT,
// UNIVERSE_MUSIC,

class ActionAdd : public ResultAction {
public:
	ActionAdd(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	byte _value;
};

class ActionAssign : public ResultAction {
public:
	ActionAssign(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	byte _value;
};

class ActionAttenuate : public ResultAction {
public:
	ActionAttenuate(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	int16 _attenuation;
};

class ActionChangeLocation : public ResultAction {
public:
	ActionChangeLocation(Common::String *line);
	bool execute(ZVision *engine);

private:
	char _world;
	char _room;
	char _nodeview[2];
	int16 _x;
};

class ActionCrossfade : public ResultAction {
public:
	ActionCrossfade(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _keyOne;
	uint32 _keyTwo;
	byte _oneStartVolume;
	byte _twoStartVolume;
	byte _oneEndVolume;
	byte _twoEndVolume;
	uint16 _timeInMillis;
};

class ActionDelayRender : public ResultAction {
public:
	ActionDelayRender(Common::String *line);
	bool execute(ZVision *engine);

private:
	// TODO: Check if this should actually be frames or if it should be milliseconds/seconds
	byte framesToDelay;
};

class ActionPlayAnimation : public ResultAction {
public:
	ActionPlayAnimation(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	Common::String _fileName;
	uint32 _x;
	uint32 _y;
	uint32 _width;
	uint32 _height;
	uint32 _start;
	uint32 _end;
	uint32 _mask;
	byte _framerate;
	bool _loop;
};

class ActionPreloadAnimation : public ResultAction {
public:
	ActionPreloadAnimation(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	Common::String _fileName;
	uint32 _mask;
	byte _framerate;
};

// TODO: See if this exists in ZGI. It doesn't in ZNem
//class ActionUnloadAnimation : public ResultAction {
//public:
//	ActionUnloadAnimation(Common::String *line);
//	bool execute(ZVision *engine);
//};

class ActionRandom : public ResultAction {
public:
	ActionRandom(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	uint32 _max;
};

class ActionTimer : public ResultAction {
public:
	ActionTimer(Common::String *line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	uint32 _time;
};

} // End of namespace ZVision

#endif
