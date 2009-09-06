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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_ENGINE_INTMAP_H
#define SCI_ENGINE_INTMAP_H

#include "common/scummsys.h"
#include "common/serializer.h"

namespace Sci {


// Assumes that the ints are relatively evenly distributed
enum {
	DCS_INT_HASH_MAX = 256
};

const uint32 INTMAPPER_MAGIC_KEY = 0xDEADBEEF;

/**
 * Defines a map from arbitrary integers to "small" integers, useable as index
 * into small arrays. This class is somewhat like a hashmap, but not quite:
 * Unlike a hashmap, it generates the values associated to each key. It does
 * not try to be very clever about it, either, e.g. using a linked list of
 * values to keep track of what is mapped where.
 * Another important feature is that it reclaims unused values when they
 * are removed.
 *
 * All in all, this implementation is not very elegant, and wastes memory.
 * But it does the job. Any rewrite of this class would have to provide a
 * way to load the old savegames made using the current implementation.
 *
 * One approach to implement a replacement: Combine a Common::HashMap<int,int>
 * with a bitfield which track which low-value integers are in use.
 * That way, lookup just invokes the hashmap, and insertion (which requires
 * finding an unmapped low-value integer) can still be implemented efficiently.
 */
struct IntMapper : public Common::Serializable {

	struct Node {
		int key;
		int idx;
		Node *next;
	};

	int base_value;  // Starts at zero, counts upwards
	Node *nodes[DCS_INT_HASH_MAX];
	Node *holes; /* List of freed entries to minimize
				     ** memory operations and modifications
				     ** to base_value  */

	void free_node_recursive(Node *node);
protected:
	void insert(int key, int idx);	// For loading only

public:
	IntMapper();
	~IntMapper();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	void clear();

	/**
	 * Checks whether a key is in the map, adds it if neccessary.
	 * @param key		The key to check for/add
	 * @param add		Whether to add the key if it's not in there
	 * @param was_added	Set to non-zero if and only if the key is new, ignored if NULL.
	 * @return The new (or old) index, or -1 if add was zero and
	 *                   the key couldn't be found
	 */
	int checkKey(int key, bool add, bool *wasAdded = 0);

	/**
	 * Looks up a key in the map
	 * @parmam key The key to look for
	 * @return The value or -1 if not found 
	 */
	int lookupKey(int key) const;


	/**
	 * Removes a key from the map.
	 * @param key		The key to remove
	 * @return	The index of the key, or -1 if it wasn't present
	 */
	int removeKey(int key);

};

} // End of namespace Sci

#endif // SCI_ENGINE_INTMAP_H
