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

#include "common/scummsys.h"

#include "zvision/timer_node.h"

#include "zvision/zvision.h"
#include "zvision/script_manager.h"

#include "common/stream.h"


namespace ZVision {

TimerNode::TimerNode(ZVision *engine, uint32 key, uint timeInSeconds) 
	: Control(engine, key), _timeLeft(timeInSeconds * 1000) {
}

bool TimerNode::process(uint32 deltaTimeInMillis) {
	_timeLeft -= deltaTimeInMillis;

	if (_timeLeft <= 0) {
		_engine->getScriptManager()->setStateValue(_key, 0);
		return true;
	}

	return false;
}

void TimerNode::serialize(Common::WriteStream *stream) {
	stream->writeUint32LE(_key);
	stream->writeUint32LE(_timeLeft);
}

void TimerNode::deserialize(Common::SeekableReadStream *stream) {
	_timeLeft = stream->readUint32LE();
}

} // End of namespace ZVision
