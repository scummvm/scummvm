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

#ifndef PINK_ACTION_PLAY_H
#define PINK_ACTION_PLAY_H

#include "pink/objects/actions/action_still.h"

namespace Pink {

class ActionPlay : public ActionStill {
public:
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	void end() override;

	void update() override;

	void pause(bool paused) override;

protected:
	void onStart() override;

	int32 _stopFrame;
};

} // End of namespace Pink

#endif
