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

#ifndef ULTIMA_ULTIMA1_GFX_VIEW_CHAR_GEN_H
#define ULTIMA_ULTIMA1_GFX_VIEW_CHAR_GEN_H

#include "ultima/shared/gfx/visual_container.h"
#include "ultima/shared/core/character.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

using Shared::CKeypressMsg;

/**
 * This class implements the character generation view
 */
class ViewCharacterGeneration : public Shared::Gfx::VisualContainer {
	DECLARE_MESSAGE_MAP;
	bool KeypressMsg(CKeypressMsg &msg);
private:
	enum Flag {
		FLAG_FRAME = 1, FLAG_ATTRIBUTES = 2, FLAG_ATTR_POINTERS = 4, FLAG_HELP = 8, FLAG_RACE = 16, FLAG_SEX = 32,
		FLAG_INITIAL = FLAG_FRAME | FLAG_ATTRIBUTES | FLAG_ATTR_POINTERS | FLAG_HELP
	};
	int _flags;
	Shared::Character _character;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	ViewCharacterGeneration(Shared::TreeItem *parent = nullptr);
	
	/**
	 * Destructor
	 */
	virtual ~ViewCharacterGeneration() {}

	/**
	 * Draw the game screen
	 */
	virtual void draw();
};

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
