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

#include "common/scummsys.h"

#include "zvision/scripting/effects/timer_effect.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"

#include "common/stream.h"

namespace ZVision {

TimerNode::TimerNode(ZVision *engine, uint32 key, uint timeInSeconds)
	: ScriptingEffect(engine, key, SCRIPTING_EFFECT_TIMER) {
	_timeLeft = 0;

	if (_engine->getGameId() == GID_NEMESIS)
		_timeLeft = timeInSeconds * 1000;
	else if (_engine->getGameId() == GID_GRANDINQUISITOR)
		_timeLeft = timeInSeconds * 100;

	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 1);
}

TimerNode::~TimerNode() {
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
	int32 timeLeft = _timeLeft / (_engine->getGameId() == GID_NEMESIS ? 1000 : 100);
	if (timeLeft > 0)
		_engine->getScriptManager()->setStateValue(_key, timeLeft); // If timer was stopped by stop or kill
}

bool TimerNode::process(uint32 deltaTimeInMillis) {
	_timeLeft -= deltaTimeInMillis;

	if (_timeLeft <= 0)
		return stop();

	return false;
}

bool TimerNode::stop() {
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
	return true;
}

void TimerNode::serialize(Common::WriteStream *stream) {
	stream->writeUint32BE(MKTAG('T', 'I', 'M', 'R'));
	stream->writeUint32LE(8); // size
	stream->writeUint32LE(_key);
	stream->writeUint32LE(_timeLeft);
}

void TimerNode::deserialize(Common::SeekableReadStream *stream) {
	_timeLeft = stream->readUint32LE();
}

} // End of namespace ZVision
