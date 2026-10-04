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

#ifndef ULTIMA_SHARED_ENGINE_EVENTS_H
#define ULTIMA_SHARED_ENGINE_EVENTS_H

#include "graphics/views/events.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Shared {

/**
 * Adds handling of Ultima-specific messages to a UIElement-derived class.
 * Engine-defined messages arrive via msgCustom, and are converted here to
 * the dedicated msgXXX virtuals that views override
 */
#define ULTIMA_CUSTOM_MESSAGES(BASE)                                            \
protected:                                                                      \
	virtual bool msgAttackTile(const AttackTileMessage &msg) {                  \
		return false;                                                           \
	}                                                                           \
	bool msgCustom(const Graphics::Views::Message &msg) override {              \
		const AttackTileMessage *attackTile =                                   \
			dynamic_cast<const AttackTileMessage *>(&msg);                      \
		if (attackTile && msgAttackTile(*attackTile))                           \
			return true;                                                        \
		return BASE::msgCustom(msg);                                            \
	}

class UIElement : public Graphics::Views::UIElement {
	ULTIMA_CUSTOM_MESSAGES(Graphics::Views::UIElement)
public:
	UIElement(const Common::String &name, Graphics::Views::UIElement *uiParent) : Graphics::Views::UIElement(name, uiParent) {
	}
	UIElement(const Common::String &name) : Graphics::Views::UIElement(name) {
	}
	~UIElement() override {
	}
};

class Events : public Graphics::Views::Events {
public:
	Events(const Common::String &engineName) : Graphics::Views::Events(engineName) {
	}
	~Events() override {
	}
};

} // namespace Shared
} // namespace Ultima

#endif
