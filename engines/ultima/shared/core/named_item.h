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

#ifndef ULTIMA_SHARED_CORE_NAMED_ITEM_H
#define ULTIMA_SHARED_CORE_NAMED_ITEM_H

#include "ultima/shared/core/tree_item.h"

namespace Ultima {
namespace Shared {

/**
 * Derived tree item class that adds a name that can be used to direct messages to later
 */
class NamedItem: public TreeItem {
	DECLARE_MESSAGE_MAP;
public:
	Common::String _name;
public:
	CLASSDEF;
	NamedItem() {}
	NamedItem(const Common::String &name) : _name(name) {}

	/**
	 * Gets the name of the item, if any
	 */
	const Common::String getName() const override { return _name; }
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
