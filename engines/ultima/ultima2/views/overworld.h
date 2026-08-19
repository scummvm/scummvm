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

#ifndef ULTIMA2_VIEWS_OVERWORLD_H
#define ULTIMA2_VIEWS_OVERWORLD_H

#include "ultima/shared/gfx/view.h"
#include "ultima/ultima2/views/overworld_map.h"
#include "ultima/ultima2/views/commands.h"
#include "ultima/ultima2/views/stats.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

class Overworld : public Shared::Gfx::View {
	enum Mode { kModePreCommand, kModeCommand, kModeAttackDirection };
private:
	OverworldMap _map = OverworldMap(this);
	Commands _commands = Commands(this);
	Stats _stats = Stats(this);
	Mode _mode = kModeCommand;

public:
	Overworld();
	~Overworld() override {}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	void timeout() override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
