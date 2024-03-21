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

#ifndef TWP_THREAD_H
#define TWP_THREAD_H

#include "common/array.h"
#include "common/str.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

class ThreadBase {
public:
	virtual ~ThreadBase() {}

	void pause() {
		if (_pauseable) {
			_paused = true;
			suspend();
		}
	}

	void unpause() {
		_paused = false;
		resume();
	}

	void setName(const Common::String &name) { _name = name; }
	Common::String getName() const { return _name; }

	int getId() const { return _id; }
	virtual HSQUIRRELVM getThread() = 0;

	virtual bool isGlobal() = 0;
	bool isSuspended();
	bool isDead();

	void suspend();
	void resume();

	virtual bool update(float elapsed) = 0;
	virtual void stop() = 0;

protected:
public:
	float _waitTime = 0.f;
	int _numFrames = 0;
	bool _paused = false;
	bool _pauseable = false;
	uint32 _lastUpdateTime = 0;

protected:
	int _id = 0;
	Common::String _name;
	bool _stopRequest = false;
	bool _stopped = false;
};

class Thread final : public ThreadBase {
public:
	Thread(const Common::String &name, bool global, HSQOBJECT threadObj, HSQOBJECT envObj, HSQOBJECT closureObj, const Common::Array<HSQOBJECT> args);
	virtual ~Thread() override final;

	virtual bool isGlobal() override final { return _global; }
	virtual HSQUIRRELVM getThread() override final { return _threadObj._unVal.pThread; }

	bool call();
	virtual bool update(float elapsed) override final;
	virtual void stop() override final;

public:
	bool _global = false;
	HSQOBJECT _threadObj, _envObj, _closureObj;
	Common::Array<HSQOBJECT> _args;
};

enum CutsceneState {
	csStart,
	csCheckEnd,
	csOverride,
	csCheckOverride,
	csEnd,
	csQuit
};

class Object;
class Cutscene final : public ThreadBase {
public:
	Cutscene(const Common::String &name, int parentThreadId, HSQOBJECT threadObj, HSQOBJECT closure, HSQOBJECT closureOverride, HSQOBJECT envObj);
	~Cutscene() override final;

	void start();
	bool isGlobal() override final { return false; }
	HSQUIRRELVM getThread() override final;
	bool update(float elapsed) override final;
	void stop() override final;

	bool hasOverride() const;
	void cutsceneOverride();
	bool isStopped();

	void setInputState(InputStateFlag state) { _inputState = state; }
	void setShowCursor(bool state) { _showCursor = state; }

private:
	void checkEndCutscene();
	void checkEndCutsceneOverride();
	void doCutsceneOverride();

private:
	int _parentThreadId = 0;
	HSQOBJECT _threadObj, _closure, _closureOverride, _envObj;
	CutsceneState _state;
	bool _showCursor = false;
	InputStateFlag _inputState = (InputStateFlag)0;
	Common::SharedPtr<Object> _actor;
};

} // namespace Twp

#endif
