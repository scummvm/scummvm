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

#ifndef ULTIMA_ULTIMA1_WIDGETS_MERCHANT_TAVERN_H
#define ULTIMA_ULTIMA1_WIDGETS_MERCHANT_TAVERN_H

#include "ultima/ultima1/widgets/merchant.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

/**
 * Implements the tavern keeper
 */
class MerchantTavern : public Merchant {
	DECLARE_MESSAGE_MAP;
public:
	DECLARE_WIDGET(MerchantTavern)
	CLASSDEF;

	/**
	 * Constructor
	 */
	MerchantTavern(Ultima1Game *game, Maps::MapBase *map, int hitPoints) :
		Merchant(game, map, 50, hitPoints) {}

	/**
	 * Constructor
	 */
	MerchantTavern(Ultima1Game *game, Maps::MapBase *map, uint tileNum, int hitPoints) :
		Merchant(game, map, tileNum, hitPoints) {}

	/**
	 * Constructor
	 */
	MerchantTavern(Ultima1Game *game, Maps::MapBase *map) : Merchant(game, map, 50) {}

	/**
	 * Do the talk action
	 */
	void talk() override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
