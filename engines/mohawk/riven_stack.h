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

// NAME Resource ID's
enum RivenNameResource {
	kCardNames = 1,
	kHotspotNames = 2,
	kExternalCommandNames = 3,
	kVariableNames = 4,
	kStackNames = 5
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
	void loadResource(MohawkEngine_Riven *vm, uint16 id);

	Common::StringArray _names;
	Common::Array<uint16> _index;
};

/**
 * A game level
 *
 * The names Card and Stack are legacy from the HyperCard engine used in
 * the original mac version of Myst.
 *
 * Stacks contain behaviors and data that are specific to a game level.
 */
class RivenStack {
public:
	RivenStack(MohawkEngine_Riven *vm, uint16 id);
	virtual ~RivenStack();

	/** Get the id of the stack */
	uint16 getId() const;

	/** Get the name of a resource using its id */
	Common::String getName(RivenNameResource nameResource, uint16 nameId) const;

	/**
	 * Get the id of a resource using its name
	 *
	 * The search is case insensitive.
	 */
	int16 getIdFromName(RivenNameResource nameResource, const Common::String &name) const;

	/** Get the id of a card in the card from its global identifier */
	uint16 getCardStackId(uint32 globalId) const;

	/** Get the global id of the currently active card */
	uint32 getCurrentCardGlobalId() const;
private:
	void loadResourceNames();
	void loadCardIdMap();

	MohawkEngine_Riven *_vm;

	uint16 _id;

	// Stack resource names
	RivenNameList _varNames;
	RivenNameList _externalCommandNames;
	RivenNameList _hotspotNames;
	RivenNameList _cardNames;
	RivenNameList _stackNames;

	Common::Array<uint32> _cardIdMap;
};

} // End of namespace Mohawk

#endif
