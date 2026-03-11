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

#include "common/file.h"
#include "common/system.h"
#include "scumm/actor.h"
#include "scumm/scumm.h"

namespace Scumm {

void ScummEngine::Playback::reset() {
	_loaded = false;
	_attempted = false;
	_active = false;
	_mi2DemoVarsApplied = false;
	_events.clear();
	_streamOff = 0;
	_streamBytes = 0;
	_nextIndex = 0;
	_lastRoom = -1;
	_hasPrevMbs = false;
	_prevMbs = 0;
	_firstInteractIndex = 0;
	_firstInteractValid = false;
	_curX = 0;
	_curY = 0;
	_pendingLUp = false;
	_pendingRUp = false;
	_sputmCmdActive = false;
	_sputmCmdEnterCount = 0;
	_sputmCmdBuf.clear();
	_name.clear();
}

void ScummEngine::Playback::parseStream(const Common::Array<byte> &stream, Common::Array<FrameEvent> &outEvents) {
	outEvents.clear();

	uint32 off = 0;
	while (off < stream.size()) {
		const byte ctl = stream[off++];
		if ((ctl & 0x0F) != 0)
			break;

		FrameEvent ev;
		ev.mbs = 0;
		ev.key = 0;

		if (ctl & 0x20) {
			if (off + 2 > stream.size())
				break;
			ev.hasPos = true;
			ev.x = (uint16)((uint16)stream[off] << 1);
			ev.y = (uint16)stream[off + 1];
			off += 2;
		}

		if (ctl & 0x10) {
			if (off + 2 > stream.size())
				break;
			ev.key = (uint16)(stream[off] | (stream[off + 1] << 8));
			off += 2;
		}

		if (ctl & 0x80)
			ev.mbs |= MBS_LEFT_CLICK;
		if (ctl & 0x40)
			ev.mbs |= MBS_RIGHT_CLICK;

		outEvents.push_back(ev);
	}
}

bool ScummEngine::applyMi2NiDemoOverride() {
	if (!_scummVars || _numVariables <= 0)
		return false;

	const uint32 numVars = (uint32)_numVariables;

	if (56 < numVars)
		_scummVars[56] = 203;

	const byte ov = VAR_OVERRIDE;
	if (ov != 0xFF)
		scummVar(ov, "VAR_OVERRIDE", "Playback Override", 0) = 1;

	return true;
}


bool ScummEngine::Playback::tryLoadPlayback(ScummEngine *engine, const Common::Path &path) {
	if (_loaded || _attempted)
		return _loaded;

	_attempted = true;

	if (!engine || engine->_targetName.empty())
		return false;

	Common::File f;
	if (!f.open(path)) {
		warning("Playback: couldn't open %s", path.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	_name = path.toString(Common::Path::kNativeSeparator);

	const uint32 fileSize = (uint32)f.size();
	Common::Array<byte> buf;
	buf.resize(fileSize);
	if (fileSize > 0)
		f.read(buf.begin(), fileSize);
	f.close();

	const byte marker0 = 0x01;
	const byte marker1 = 0xFF;

	uint32 streamOff = 0;
	bool foundPlaybackStream = false;

	for (uint32 i = 0; i + 1 < buf.size(); i++) {
		if (buf[i] != marker0 || buf[i + 1] != marker1)
			continue;

		const uint32 candOff = i + 2;

		uint32 off = candOff;
		uint32 count = 0;
		bool ok = true;

		while (off < buf.size()) {
			const byte ctrl = buf[off];

			if ((ctrl & 0x0F) != 0) {
				ok = false;
				break;
			}

			if ((ctrl & ~(0x80 | 0x40 | 0x20 | 0x10)) != 0) {
				ok = false;
				break;
			}

			uint32 recLen = 1;
			if (ctrl & 0x20)
				recLen += 2;
			if (ctrl & 0x10)
				recLen += 2;

			if (off + recLen > buf.size()) {
				ok = false;
				break;
			}

			off += recLen;
			count++;
		}

		if (ok && off == buf.size() && count > 0) {
			streamOff = candOff;
			foundPlaybackStream = true;
		}
	}

	if (!foundPlaybackStream) {
		warning("Playback: demo.rec stream marker 0x01 0xFF not found");
		return false;
	}

	_streamOff = streamOff;
	_streamBytes = buf.size() - _streamOff;

	Common::Array<byte> stream;
	stream.resize(_streamBytes);
	memcpy(stream.begin(), buf.begin() + _streamOff, _streamBytes);

	parseStream(stream, _events);

	_loaded = !_events.empty();
	return _loaded;
}

bool ScummEngine::Playback::startPlayback(ScummEngine *engine) {
	if (!engine || !_loaded || _events.empty())
		return false;

	_active = true;
	_nextIndex = 0;
	_hasPrevMbs = false;
	_prevMbs = 0;
	_pendingLUp = false;
	_pendingRUp = false;
	_sputmCmdActive = false;
	_sputmCmdEnterCount = 0;
	_sputmCmdBuf.clear();
	_curX = engine->_mouse.x;
	_curY = engine->_mouse.y;
	return true;
}

void ScummEngine::Playback::mi2DemoArmPlaybackByRoom(ScummEngine *engine) {
	if (!engine)
		return;

	if (!_loaded)
		tryLoadPlayback(engine);

	if (!_loaded)
		return;

	const int room = (int)engine->_currentRoom;

	if (!_mi2DemoVarsApplied && room == 3) {
		engine->applyMi2NiDemoOverride();

		_mi2DemoVarsApplied = true;
	}

	if (room == 4) {
		if (!_active)
			startPlayback(engine);

		if (!_mi2DemoVarsApplied) {
			engine->applyMi2NiDemoOverride();
			_mi2DemoVarsApplied = true;
		}
	}
}

// The MI2 NI DOS Demo used the SPUTM debugger to jump rooms, we don't have that so we have to fake room jumps.
void ScummEngine::Playback::mi2DemoPlaybackJumpRoom(ScummEngine *engine, int room) {
	if (!engine)
		return;
	if (room <= 0)
		return;

	if (engine->_objectRoomTable) {
		engine->_objectRoomTable[7] = (byte)room;
	}

	if (engine->VAR_EGO != 0xFF) {
		const int egoActorIdx = engine->VAR(engine->VAR_EGO);
		if (egoActorIdx >= 0 && egoActorIdx < engine->_numActors) {
			Actor *ego = engine->_actors[egoActorIdx];
			if (ego)
				ego->_room = room;
		}
	}

	engine->startScene(room, nullptr, 7);

	engine->_cameraIsFrozen = false;
	if (engine->VAR_EGO != 0xFF) {
		const int egoActorIdx = engine->VAR(engine->VAR_EGO);
		if (egoActorIdx >= 0 && egoActorIdx < engine->_numActors) {
			Actor *ego = engine->_actors[egoActorIdx];
			if (ego) {
				engine->setCameraFollows(ego, true);
				engine->actorFollowCamera(egoActorIdx);
				engine->moveCamera();
			}
		}
	}

	engine->_fullRedraw = true;
}

// MI2's 'demo.rec' has some keypresses destined for the SPUTM debugger.
bool ScummEngine::Playback::handleMi2NIDemoSputmDebugKey(ScummEngine *engine, uint16 rawKey) {
	if (rawKey == 0)
		return false;

	if (rawKey == 7) {
		_sputmCmdActive = true;
		_sputmCmdEnterCount = 0;
		_sputmCmdBuf.clear();
		return true;
	}

	if (!_sputmCmdActive)
		return false;

	if (rawKey == 27) {
		_sputmCmdActive = false;
		_sputmCmdEnterCount = 0;
		_sputmCmdBuf.clear();
		return true;
	}

	if (rawKey == 13) {
		if (_sputmCmdEnterCount < 2)
			_sputmCmdEnterCount++;

		if (_sputmCmdEnterCount >= 2) {
			const Common::String cmd = _sputmCmdBuf;
			_sputmCmdActive = false;
			_sputmCmdEnterCount = 0;
			_sputmCmdBuf.clear();

			//Special case, we should jump to to room 47 or 27.
			int targetRoom = -1;
			if (cmd.equalsIgnoreCase("vill"))
				targetRoom = 47;
			else if (cmd.equalsIgnoreCase("whar"))
				targetRoom = 27;

			if (targetRoom >= 0) {
				mi2DemoPlaybackJumpRoom(engine, targetRoom);
			}
		}
		return true;
	}

	if (rawKey >= 32 && rawKey <= 126) {
		if (_sputmCmdBuf.size() < 32)
			_sputmCmdBuf += (char)rawKey;
		return true;
	}

	_sputmCmdActive = false;
	_sputmCmdEnterCount = 0;
	_sputmCmdBuf.clear();
	return true;
}

void ScummEngine::Playback::playbackPump(ScummEngine *engine) {
	if (!engine || !_active)
		return;

	if (_nextIndex >= _events.size()) {
		_active = false;
		return;
	}

	if (_pendingLUp) {
		Common::Event ev;
		ev.type = Common::EVENT_LBUTTONUP;
		ev.mouse.x = _curX;
		ev.mouse.y = _curY;
		engine->parseEvent(ev);
		_pendingLUp = false;
	}
	if (_pendingRUp) {
		Common::Event ev;
		ev.type = Common::EVENT_RBUTTONUP;
		ev.mouse.x = _curX;
		ev.mouse.y = _curY;
		engine->parseEvent(ev);
		_pendingRUp = false;
	}

	if (_nextIndex >= _events.size()) {
		_active = false;
		return;
	}

	const FrameEvent &fev = _events[_nextIndex];

	if (fev.hasPos) {
		_curX = (int16)fev.x;
		_curY = (int16)fev.y;
	}

	{
		Common::Event mv;
		mv.type = Common::EVENT_MOUSEMOVE;
		mv.mouse.x = _curX;
		mv.mouse.y = _curY;
		engine->parseEvent(mv);
	}

	engine->_mouse.x = _curX;
	engine->_mouse.y = _curY;
	g_system->warpMouse(_curX, _curY);

	if (engine->_currentRoom == 4) {
		engine->_virtualMouse.x = _curX;
		engine->_virtualMouse.y = _curY;
	} else {
		engine->_virtualMouse.x = _curX + (engine->_screenStartStrip * 8);
		engine->_virtualMouse.y = _curY + engine->_screenTop;
	}

	const uint16 prev = _hasPrevMbs ? _prevMbs : 0;
	const uint16 cur = fev.mbs;

	const bool prevL = (prev & MBS_LEFT_CLICK) != 0;
	const bool curL = (cur & MBS_LEFT_CLICK) != 0;
	const bool prevR = (prev & MBS_RIGHT_CLICK) != 0;
	const bool curR = (cur & MBS_RIGHT_CLICK) != 0;

	if (!prevL && curL) {
		Common::Event ev;
		ev.type = Common::EVENT_LBUTTONDOWN;
		ev.mouse.x = _curX;
		ev.mouse.y = _curY;
		engine->parseEvent(ev);
		_pendingLUp = true;
	}
	if (!prevR && curR) {
		Common::Event ev;
		ev.type = Common::EVENT_RBUTTONDOWN;
		ev.mouse.x = _curX;
		ev.mouse.y = _curY;
		engine->parseEvent(ev);
		_pendingRUp = true;
	}

	if (fev.key != 0) {
		if (!handleMi2NIDemoSputmDebugKey(engine, fev.key))
			engine->_mouseAndKeyboardStat = fev.key;
	}

	if (fev.key == 0)
		engine->_mouseAndKeyboardStat = (engine->_mouseAndKeyboardStat & ~MBS_MOUSE_MASK) | (cur & MBS_MOUSE_MASK);

	_prevMbs = cur;
	_hasPrevMbs = true;

	++_nextIndex;
}

} // namespace Scumm
