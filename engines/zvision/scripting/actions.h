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

#ifndef ZVISION_ACTIONS_H
#define ZVISION_ACTIONS_H

#include "common/str.h"

#include "audio/mixer.h"


namespace ZVision {

// Forward declaration of ZVision. This file is included before ZVision is declared
class ZVision;

/**
  * The base class that represents any action that a Puzzle can take.
  * This class is purely virtual.
  */
class ResultAction {
public:
	virtual ~ResultAction() {}
	/**
	 * This is called by the script system whenever a Puzzle's criteria are found to be true.
	 * It should execute any necessary actions and return a value indicating whether the script
	 * system should continue to test puzzles. In 99% of cases this will be 'true'.
	 *
	 * @param engine    A pointer to the base engine so the ResultAction can access all the necessary methods
	 * @return          Should the script system continue to test any remaining puzzles (true) or immediately break and go on to the next frame (false)
	 */
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
	ActionAdd(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	uint _value;
};

class ActionAssign : public ResultAction {
public:
	ActionAssign(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	uint _value;
};

class ActionAttenuate : public ResultAction {
public:
	ActionAttenuate(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	int _attenuation;
};

class ActionChangeLocation : public ResultAction {
public:
	ActionChangeLocation(const Common::String &line);
	bool execute(ZVision *engine);

private:
	char _world;
	char _room;
	char _node;
	char _view;
	uint32 _offset;
};

class ActionCrossfade : public ResultAction {
public:
	ActionCrossfade(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _keyOne;
	uint32 _keyTwo;
	uint _oneStartVolume;
	uint _twoStartVolume;
	uint _oneEndVolume;
	uint _twoEndVolume;
	uint _timeInMillis;
};

class ActionDebug : public ResultAction {
public:
	ActionDebug(const Common::String &line);
	bool execute(ZVision *engine);

private:
};

class ActionDelayRender : public ResultAction {
public:
	ActionDelayRender(const Common::String &line);
	bool execute(ZVision *engine);

private:
	// TODO: Check if this should actually be frames or if it should be milliseconds/seconds
	uint32 framesToDelay;
};

class ActionDisableControl : public ResultAction {
public:
	ActionDisableControl(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
};

class ActionDisableVenus : public ResultAction {
public:
	ActionDisableVenus(const Common::String &line);
	bool execute(ZVision *engine);

private:
};

class ActionDisplayMessage : public ResultAction {
public:
	ActionDisplayMessage(const Common::String &line);
	bool execute(ZVision *engine);

private:
};

class ActionDissolve : public ResultAction {
public:
	ActionDissolve();
	bool execute(ZVision *engine);
};

class ActionDistort : public ResultAction {
public:
	ActionDistort(const Common::String &line);
	bool execute(ZVision *engine);

private:
};

class ActionEnableControl : public ResultAction {
public:
	ActionEnableControl(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
};

class ActionMusic : public ResultAction {
public:
	ActionMusic(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	Audio::Mixer::SoundType _soundType;
	Common::String _fileName;
	bool _loop;
	byte _volume;
};

class ActionPlayAnimation : public ResultAction {
public:
	ActionPlayAnimation(const Common::String &line);
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
	uint _mask;
	uint _framerate;
	uint _loopCount;
};

class ActionPlayPreloadAnimation : public ResultAction {
public:
	ActionPlayPreloadAnimation(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _animationKey;
	uint32 _controlKey;
	uint32 _x1;
	uint32 _y1;
	uint32 _x2;
	uint32 _y2;
	uint _startFrame;
	uint _endFrame;
	uint _loopCount;
};

class ActionPreloadAnimation : public ResultAction {
public:
	ActionPreloadAnimation(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	Common::String _fileName;
	uint _mask;
	uint _framerate;
};

class ActionQuit : public ResultAction {
public:
	ActionQuit() {}
	bool execute(ZVision *engine);
};

// TODO: See if this exists in ZGI. It doesn't in ZNem
class ActionUnloadAnimation : public ResultAction {
public:
	ActionUnloadAnimation(const Common::String &line);
	bool execute(ZVision *engine);
};

class ActionRandom : public ResultAction {
public:
	ActionRandom(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	uint _max;
};

class ActionSetPartialScreen : public ResultAction {
public:
	ActionSetPartialScreen(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint _x;
	uint _y;
	Common::String _fileName;
	uint16 _backgroundColor;
};

class ActionSetScreen : public ResultAction {
public:
	ActionSetScreen(const Common::String &line);
	bool execute(ZVision *engine);

private:
	Common::String _fileName;
};

class ActionStreamVideo : public ResultAction {
public:
	ActionStreamVideo(const Common::String &line);
	bool execute(ZVision *engine);

private:
	enum {
		DIFFERENT_DIMENSIONS = 0x1 // 0x1 flags that the destRect dimensions are different from the original video dimensions
	};

	Common::String _fileName;
	uint _x1;
	uint _y1;
	uint _x2;
	uint _y2;
	uint _flags;
	bool _skippable;
};

class ActionTimer : public ResultAction {
public:
	ActionTimer(const Common::String &line);
	bool execute(ZVision *engine);

private:
	uint32 _key;
	uint _time;
};

} // End of namespace ZVision

#endif
