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

#ifndef WATCH_INSTANCE_H_
#define WATCH_INSTANCE_H_

namespace Wintermute {
class Watch;
class ScValue;
class DebuggableScript;

class WatchInstance {
	Watch* _watch;
	ScValue *_lastValue;
	DebuggableScript* _script;
public:
	WatchInstance (Watch* watch, DebuggableScript* script);
	~WatchInstance();
	void evaluate();
friend class DebuggableScript;
friend class Watch;
};
} // End of namespace Wintermute

#endif /* WATCH_INSTANCE_H_ */
