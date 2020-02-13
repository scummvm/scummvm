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

#ifndef ULTIMA_ULTIMA1_U1GFX_STATUS_H
#define ULTIMA_ULTIMA1_U1GFX_STATUS_H

#include "ultima/shared/gfx/visual_item.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

using Shared::CFrameMsg;

/**
 * Displays the total hits, food, experience, and coins you have
 */
class Status : public Shared::Gfx::VisualItem {
	DECLARE_MESSAGE_MAP;
	bool FrameMsg(CFrameMsg &msg);
private:
	uint _hitPoints, _food, _experience, _coins;
private:
public:
	CLASSDEF;
	Status(TreeItem *parent);
	~Status() override {}

	/**
	 * Draw the contents
	 */
	void draw() override;
};

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
