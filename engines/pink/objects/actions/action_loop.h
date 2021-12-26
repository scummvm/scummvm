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

#ifndef PINK_ACTION_LOOP_H
#define PINK_ACTION_LOOP_H

#include "pink/objects/actions/action_play.h"

namespace Pink {

class ActionLoop : public ActionPlay {
public:
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	void update() override;

	void end() override;

protected:
	void onStart() override;
	virtual bool isTalk() { return false; }

	enum Style {
		kPingPong = 2,
		kRandom = 3,
		kForward = 4
	};
	Style _style;
	bool _intro;
	bool _inLoop;
	bool _forward;
};

} // End of namespace Pink

#endif
