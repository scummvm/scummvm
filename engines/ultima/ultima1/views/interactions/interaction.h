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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_INTERACTION_H
#define ULTIMA1_VIEWS_INTERACTIONS_INTERACTION_H

#include "ultima/shared/gfx/view.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

/**
 * Base view for any interaction that prompts the user for information.
 * It creates a dummy 0x0 size window on top of the current one that
 * can receive input, and does any output in the commands window
 */
class Interaction : public Shared::Gfx::View {
protected:
	void writeString(const Common::String &msg);
	void writeString(const char *format, ...);
	void resetLine();
	void showCursor();
	void prompt();

public:
	Interaction(const Common::String &name);
	~Interaction() override {
	}

	bool tick() override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
