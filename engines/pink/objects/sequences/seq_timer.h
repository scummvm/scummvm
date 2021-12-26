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

#ifndef PINK_SEQ_TIMER_H
#define PINK_SEQ_TIMER_H

#include "pink/objects/object.h"

namespace Pink {

class Sequencer;

class SeqTimer : public Object {
public:
	SeqTimer();

	void deserialize(Archive &archive) override;
	void toConsole() const override;

	void update();

private:
	Common::String _actor;
	Sequencer *_sequencer;
	int _period;
	int _range;
	int _updatesToMessage;
};

} // End of namespace Pink

#endif
