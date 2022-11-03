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

#ifndef GRIM_EMI_SPRITE_COMPONENT_H
#define GRIM_EMI_SPRITE_COMPONENT_H

#include "engines/grim/costume/component.h"

namespace Grim {

class Sprite;

class EMISpriteComponent : public Component {
public:
	EMISpriteComponent(Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag);
	~EMISpriteComponent();
	void init() override;
	int update(uint time) override;
	void reset() override;
	void draw() override;

public:
	Sprite *_sprite;
};

} // end of namespace Grim

#endif
