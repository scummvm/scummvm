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

#ifndef RIVEN_STACK_H
#define RIVEN_STACK_H

#include "common/str-array.h"

namespace Mohawk {

class MohawkEngine_Riven;
class RivenNameList;

/**
 * A game level
 *
 * The names Card and Stack are legacy from the HyperCard engine used in
 * the original mac version of Myst.
 *
 * Stacks contain behaviors that are specific to a game level.
 */
class RivenStack {
public:
	RivenStack(MohawkEngine_Riven *vm, uint16 id);
	virtual ~RivenStack();

	/** Get the id of the stack */
	uint16 getId() const;
private:
	MohawkEngine_Riven *_vm;

	uint16 _id;
};

/**
 * Name lists provide bidirectional association between an object's name and its id
 */
class RivenNameList {
public:
	RivenNameList();
	RivenNameList(MohawkEngine_Riven *vm, uint16 id);
	~RivenNameList();

	/** Get the name of an object using its id */
	Common::String getName(uint16 nameID) const;

	/**
	 * Get the id of an object using its name
	 *
	 * This query is case insensitive.
	 */
	int16 getNameId(const Common::String &name) const;

private:
	Common::StringArray _names;
	Common::Array<uint16> _index;

	void loadResource(MohawkEngine_Riven *vm, uint16 id);
};

} // End of namespace Mohawk

#endif
