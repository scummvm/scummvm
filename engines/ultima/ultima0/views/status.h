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

#ifndef ULTIMA0_VIEWS_STATUS_H
#define ULTIMA0_VIEWS_STATUS_H

#include "ultima/ultima0/views/view.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

class Status : public View {
private:
	Common::String _message;
	Common::String _direction;

public:
	Status(const Common::String &name, UIElement *parent);
	~Status() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgGame(const GameMessage &msg) override;
};

class OverworldStatus : public Status {
public:
	OverworldStatus(UIElement *parent) : Status("OverworldStatus", parent) {
	}
};

class DungeonStatus : public Status {
private:
	Common::StringArray _lines;

public:
	DungeonStatus(UIElement *parent) : Status("DungeonStatus", parent) {
	}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgGame(const GameMessage &msg) override;
};

} // namespace Views
} // namespace Ultima0
} // namespace Ultima

#endif
