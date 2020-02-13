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

#ifndef NUVIE_CONF_XML_NODE_H
#define NUVIE_CONF_XML_NODE_H

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"

namespace Ultima {
namespace Nuvie {

class   XMLNode {
protected:
	Std::string             id;
	Std::string             content;
	Std::vector<XMLNode *>   nodelist;
	bool                    no_close;

public:
	XMLNode() : no_close(false) {
	}
	XMLNode(const Std::string &i) : id(i), no_close(false) {
	}
	XMLNode(const XMLNode &n)
		: id(n.id), content(n.content), nodelist(n.nodelist), no_close(false) {
	}
	~XMLNode();

	XMLNode &operator=(const XMLNode &n) {
		id = n.id;
		content = n.content;
		nodelist = n.nodelist;
		no_close = n.no_close;
		return *this;
	}

	const Std::string &reference(const Std::string &, bool &);
	const XMLNode *subtree(const Std::string &) const;

	const Std::string &value(void) const {
		return content;
	}

	typedef Std::pair<Std::string, Std::string> KeyType;
	typedef Std::vector<KeyType> KeyTypeList;

	bool searchpairs(KeyTypeList &ktl, const Std::string &basekey,
	                 const Std::string currkey, const unsigned int pos);
	void selectpairs(KeyTypeList &ktl, const Std::string currkey);

	Std::string dump(int depth = 0);

	void xmlassign(const Std::string &key, const Std::string &value);
	void xmlparse(const Std::string &s, size_t &pos);

	void listkeys(const Std::string &, Std::vector<Std::string> &,
	              bool longformat = true) const;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
