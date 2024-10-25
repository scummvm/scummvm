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

#ifndef ZVISION_ACTIONS_H
#define ZVISION_ACTIONS_H

#include "common/path.h"
#include "common/str.h"
#include "common/rect.h"

namespace ZVision {

// Forward declaration of ZVision. This file is included before ZVision is declared
class ZVision;
class ScriptManager;
class ValueSlot;

/**
  * The base class that represents any action that a Puzzle can take.
  * This class is purely virtual.
  */
class ResultAction {
public:
	ResultAction(ZVision *engine, int32 slotkey);
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
	ScriptManager *_scriptManager;
	int32 _slotKey;
};

class ActionAdd : public ResultAction {
public:
	ActionAdd(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionAdd();
	bool execute() override;

private:
	uint32 _key;
	ValueSlot *_value;
};

class ActionAssign : public ResultAction {
public:
	ActionAssign(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionAssign() override;
	bool execute() override;

private:
	uint32 _key;
	ValueSlot *_value;
};

class ActionAttenuate : public ResultAction {
public:
	ActionAttenuate(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint32 _key;
	int32 _attenuation;
};

class ActionChangeLocation : public ResultAction {
public:
	ActionChangeLocation(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

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
	bool execute() override;

private:
	uint32 _keyOne;
	uint32 _keyTwo;
	int32 _oneStartVolume;
	int32 _twoStartVolume;
	int32 _oneEndVolume;
	int32 _twoEndVolume;
	int32 _timeInMillis;
};

class ActionCursor : public ResultAction {
public:
	ActionCursor(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint8 _action;
};

class ActionDelayRender : public ResultAction {
public:
	ActionDelayRender(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint32 _framesToDelay;
};

class ActionDisableControl : public ResultAction {
public:
	ActionDisableControl(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint32 _key;
};

class ActionDisplayMessage : public ResultAction {
public:
	ActionDisplayMessage(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	int16 _control;
	int16 _msgid;
};

class ActionDissolve : public ResultAction {
public:
	ActionDissolve(ZVision *engine);
	bool execute() override;
};

class ActionDistort : public ResultAction {
public:
	ActionDistort(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionDistort() override;
	bool execute() override;

private:
	int16 _distSlot;
	int16 _speed;
	float _startAngle;
	float _endAngle;
	float _startLineScale;
	float _endLineScale;
};

class ActionEnableControl : public ResultAction {
public:
	ActionEnableControl(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint32 _key;
};

class ActionFlushMouseEvents : public ResultAction {
public:
	ActionFlushMouseEvents(ZVision *engine, int32 slotkey);
	bool execute() override;
};

class ActionInventory : public ResultAction {
public:
	ActionInventory(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;
private:
	int8 _type;
	int32 _key;
};

class ActionKill : public ResultAction {
public:
	ActionKill(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint32 _key;
	uint32 _type;
};

class ActionMenuBarEnable : public ResultAction {
public:
	ActionMenuBarEnable(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;
private:
	uint16 _menus;
};

class ActionMusic : public ResultAction {
public:
	ActionMusic(ZVision *engine, int32 slotkey, const Common::String &line, bool global);
	~ActionMusic() override;
	bool execute() override;

private:
	Common::Path _fileName;
	bool _loop;
	ValueSlot *_volume;
	bool _universe;
	bool _midi;
	int8 _note;
	int8 _prog;
};

class ActionPanTrack : public ResultAction {
public:
	ActionPanTrack(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionPanTrack() override;
	bool execute() override;

private:
	int32 _pos;
	uint32 _musicSlot;
};

class ActionPlayAnimation : public ResultAction {
public:
	ActionPlayAnimation(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionPlayAnimation() override;
	bool execute() override;

private:
	Common::Path _fileName;
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
	bool execute() override;

private:
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
	~ActionPreloadAnimation() override;
	bool execute() override;

private:
	Common::Path _fileName;
	int32 _mask;
	int32 _framerate;
};

class ActionPreferences : public ResultAction {
public:
	ActionPreferences(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	bool _save;
};

class ActionQuit : public ResultAction {
public:
	ActionQuit(ZVision *engine, int32 slotkey) : ResultAction(engine, slotkey) {}
	bool execute() override;
};

class ActionRegion : public ResultAction {
public:
	ActionRegion(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionRegion() override;
	bool execute() override;

private:
	Common::String _art;
	Common::String _custom;
	Common::Rect _rect;
	uint16 _delay;
	uint16 _type;
	uint16 _unk1;
	uint16 _unk2;
};

// Only used by ZGI (locations cd6e, cd6k, dg2f, dg4e, dv1j)
class ActionUnloadAnimation : public ResultAction {
public:
	ActionUnloadAnimation(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;
private:
	uint32 _key;
};

class ActionRandom : public ResultAction {
public:
	ActionRandom(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionRandom() override;
	bool execute() override;

private:
	ValueSlot *_max;
};

class ActionRestoreGame : public ResultAction {
public:
	ActionRestoreGame(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	Common::Path _fileName;
};

class ActionRotateTo : public ResultAction {
public:
	ActionRotateTo(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	int32 _toPos;
	int32 _time;
};

class ActionSetPartialScreen : public ResultAction {
public:
	ActionSetPartialScreen(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint _x;
	uint _y;
	Common::Path _fileName;
	int32 _backgroundColor;
};

class ActionSetScreen : public ResultAction {
public:
	ActionSetScreen(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	Common::Path _fileName;
};

class ActionStop : public ResultAction {
public:
	ActionStop(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	uint32 _key;
};

class ActionStreamVideo : public ResultAction {
public:
	ActionStreamVideo(ZVision *engine, int32 slotkey, const Common::String &line);
	bool execute() override;

private:
	enum {
		DIFFERENT_DIMENSIONS = 0x1 // 0x1 flags that the destRect dimensions are different from the original video dimensions
	};

	Common::Path _fileName;
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
	bool execute() override;

private:
	int _syncto;
	Common::Path _fileName;
};

class ActionTimer : public ResultAction {
public:
	ActionTimer(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionTimer() override;
	bool execute() override;
private:
	ValueSlot *_time;
};

class ActionTtyText : public ResultAction {
public:
	ActionTtyText(ZVision *engine, int32 slotkey, const Common::String &line);
	~ActionTtyText() override;
	bool execute() override;

private:
	Common::Path _filename;
	uint32 _delay;
	Common::Rect _r;
};
} // End of namespace ZVision

#endif
