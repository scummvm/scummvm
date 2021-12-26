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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_STATE_STATE_H
#define NANCY_STATE_STATE_H

namespace Nancy {
namespace State {

// Base class for all states. Provides a simple API for handling state switching
class State {
public:
	State() {}
	virtual ~State() {};

	virtual void process() = 0;
	virtual void onStateEnter() {}
	virtual void onStateExit() {}
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_STATE_H
