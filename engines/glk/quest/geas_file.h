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

#ifndef GLK_QUEST_GEAS_FILE
#define GLK_QUEST_GEAS_FILE

#include "glk/quest/string.h"
#include "common/algorithm.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stream.h"

namespace Glk {
namespace Quest {

class GeasInterface;

class reserved_words;

/**
 * Ordered array of items
 */
template<class T>
class Set : public Common::Array<T> {
public:
	/**
	 * Insert a new item
	 */
	void insert(T val) {
		this->push_back(val);
		Common::sort(this->begin(), this->end());
	}
};

struct GeasBlock {
	////// lname == lowercase name
	////// nname == normal name
	////// parent == initial parent object (lowercased)
	// name == name
	// parent == initial parent object
	String blocktype, name, parent;
	Common::Array<String> data;
	//GeasBlock (const Common::Array<String> &, String, uint, bool);
	GeasBlock() {}
};

struct GeasFile {
	GeasInterface *gi;
	void debug_print(String s) const;

	//vector<GeasBlock> rooms, objects, textblocks, functions, procedures, types;
	//GeasBlock synonyms, game;
	Common::Array <GeasBlock> blocks;

	//Common::Array<GeasBlock> rooms, objects, textblocks, functions, procedures,
	//  types, synonyms, game, variables, timers, choices;
	StringMap obj_types;
	StringArrayIntMap type_indecies;

	void register_block(String blockname, String blocktype);

	const GeasBlock &block(String type, uint index) const;
	uint size(String type) const;

	void read_into(const Common::Array<String> &, String, uint, bool, const reserved_words &, const reserved_words &);



	GeasFile() : gi(nullptr) {}
	explicit GeasFile(const Common::Array<String> &in_data,
	                  GeasInterface *gi);

	bool obj_has_property(String objname, String propname) const;
	bool get_obj_property(String objname, String propname,
	                      String &rv) const;

	void get_type_property(String typenamex, String propname,
	                       bool &, String &) const;
	bool obj_of_type(String object, String type) const;
	bool type_of_type(String subtype, String supertype) const;

	Set<String> get_obj_keys(String obj) const;
	void get_obj_keys(String, Set<String> &) const;
	void get_type_keys(String, Set<String> &) const;

	bool obj_has_action(String objname, String propname) const;
	bool get_obj_action(String objname, String propname,
	                    String &rv) const;
	void get_type_action(String typenamex, String propname,
	                     bool &, String &) const;
	String static_eval(String) const;
	String static_ivar_lookup(String varname) const;
	String static_svar_lookup(String varname) const;

	const GeasBlock *find_by_name(String type, String name) const;
};

Common::WriteStream &operator<<(Common::WriteStream &, const GeasBlock &);
Common::WriteStream &operator<<(Common::WriteStream &, const GeasFile &);

} // End of namespace Quest
} // End of namespace Glk

#endif
