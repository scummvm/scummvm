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
#include "common/rect.h"

#include "audio/mixer.h"


namespace ZVision {

// Forward declaration of ZVision. This file is included before ZVision is declared
class ZVision;
class ValueSlot;

/**
  * The base class that represents any action that a Puzzle can take.
  * This class is purely virtual.
  */
class ResultAction {
public:
	ResultAction(ZVision *engine, int32 slotkey) : _engine(engine), _slotkey(slotkey) {}
	virtual ~ResultAction() {}
	/**
	 * This is called by the script system whenever a Puzzle's criteria are found to be true.
	 * It should execute any necessary actions and return a value indicating whether the script
	 * system should continue to test puzzles. In 99% of cases this will be 'true'.
	 *
	 * @param engine    A pointer to the base engine so the ResultAction can access all the necessary methods
	 * @return          Should the script system continue to test any remaining puzzles (true) or immediately break and go on to the next frame (false)
	 */
	virtual bool execute() = 0;
protected:
	ZVision *_engine;
	int32 _slotkey;
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
	ActionAdd(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint32 _key;
	int _value;
};

class ActionAssign : public ResultAction {
public:
	ActionAssign(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionAssign();
	bool execute();

private:
	uint32 _key;
	ValueSlot *_value;
};

class ActionAttenuate : public ResultAction {
public:
	ActionAttenuate(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint32 _key;
	int32 _attenuation;
};

class ActionChangeLocation : public ResultAction {
public:
	ActionChangeLocation(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	char _world;
	char _room;
	char _node;
	char _view;
	uint32 _offset;
};

class ActionCrossfade : public ResultAction {
public:
	ActionCrossfade(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint32 _keyOne;
	uint32 _keyTwo;
	int32 _oneStartVolume;
	int32 _twoStartVolume;
	int32 _oneEndVolume;
	int32 _twoEndVolume;
	int32 _timeInMillis;
};

class ActionDebug : public ResultAction {
public:
	ActionDebug(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
};

class ActionDelayRender : public ResultAction {
public:
	ActionDelayRender(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	// TODO: Check if this should actually be frames or if it should be milliseconds/seconds
	uint32 framesToDelay;
};

class ActionDisableControl : public ResultAction {
public:
	ActionDisableControl(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint32 _key;
};

class ActionDisableVenus : public ResultAction {
public:
	ActionDisableVenus(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
};

class ActionDisplayMessage : public ResultAction {
public:
	ActionDisplayMessage(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	int16 _control;
	int16 _msgid;
};

class ActionDissolve : public ResultAction {
public:
	ActionDissolve(ZVision *engine);
	bool execute();
};

class ActionDistort : public ResultAction {
public:
	ActionDistort(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
};

class ActionEnableControl : public ResultAction {
public:
	ActionEnableControl(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint32 _key;
};

class ActionInventory : public ResultAction {
public:
	ActionInventory(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();
private:
	uint8 _type;
	int32 _key;
};

class ActionKill : public ResultAction {
public:
	ActionKill(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint32 _key;
	uint32 _type;
};

class ActionMusic : public ResultAction {
public:
	ActionMusic(ZVision *engine, int32 slotkey, const Common::String &line, bool global);
	~ActionMusic();
	bool execute();

private:
	uint32 _key;
	Audio::Mixer::SoundType _soundType;
	Common::String _fileName;
	bool _loop;
	byte _volume;
	bool _universe;
};

class ActionPanTrack : public ResultAction {
public:
	ActionPanTrack(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionPanTrack();
	bool execute();

private:
	int32 _pos;
	uint32 _mus_slot;
};

class ActionPlayAnimation : public ResultAction {
public:
	ActionPlayAnimation(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionPlayAnimation();
	bool execute();

private:
	uint32 _key;
	Common::String _fileName;
	uint32 _x;
	uint32 _y;
	uint32 _x2;
	uint32 _y2;
	uint32 _start;
	uint32 _end;
	int32 _mask;
	int32 _framerate;
	int32 _loopCount;
};

class ActionPlayPreloadAnimation : public ResultAction {
public:
	ActionPlayPreloadAnimation(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

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
	ActionPreloadAnimation(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionPreloadAnimation();
	bool execute();

private:
	uint32 _key;
	Common::String _fileName;
	int32 _mask;
	int32 _framerate;
};

class ActionQuit : public ResultAction {
public:
	ActionQuit(ZVision *engine, int32 slotkey) : ResultAction(engine, slotkey) {}
	bool execute();
};

// TODO: See if this exists in ZGI. It doesn't in ZNem
class ActionUnloadAnimation : public ResultAction {
public:
	ActionUnloadAnimation(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();
};

class ActionRandom : public ResultAction {
public:
	ActionRandom(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionRandom();
	bool execute();

private:
	uint32 _key;
	ValueSlot *_max;
};

class ActionSetPartialScreen : public ResultAction {
public:
	ActionSetPartialScreen(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint _x;
	uint _y;
	Common::String _fileName;
	int32 _backgroundColor;
};

class ActionSetScreen : public ResultAction {
public:
	ActionSetScreen(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	Common::String _fileName;
};

class ActionStop : public ResultAction {
public:
	ActionStop(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	uint32 _key;
};

class ActionStreamVideo : public ResultAction {
public:
	ActionStreamVideo(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

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

class ActionSyncSound : public ResultAction {
public:
	ActionSyncSound(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute();

private:
	int _syncto;
	Common::String _fileName;
};

class ActionTimer : public ResultAction {
public:
	ActionTimer(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionTimer();
	bool execute();
private:
	uint32 _key;
	ValueSlot *_time;
};

class ActionTtyText : public ResultAction {
public:
	ActionTtyText(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionTtyText();
	bool execute();

private:
	Common::String _filename;
	uint32 _delay;
	Common::Rect _r;
};
} // End of namespace ZVision

#endif
