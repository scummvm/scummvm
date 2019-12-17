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

#ifndef ULTIMA6_CONF_XML_NODE_H
#define ULTIMA6_CONF_XML_NODE_H

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"

namespace Ultima {
namespace Ultima6 {

class   XMLNode {
protected:
	std::string             id;
	std::string             content;
	std::vector<XMLNode *>   nodelist;
	bool                    no_close;

public:
	XMLNode() : no_close(false) {
	}
	XMLNode(const std::string &i) : id(i), no_close(false) {
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

	const std::string &reference(const std::string &, bool &);
	const XMLNode *subtree(const std::string &) const;

	const std::string &value(void) const {
		return content;
	}

	typedef std::pair<std::string, std::string> KeyType;
	typedef std::vector<KeyType> KeyTypeList;

	bool searchpairs(KeyTypeList &ktl, const std::string &basekey,
	                 const std::string currkey, const unsigned int pos);
	void selectpairs(KeyTypeList &ktl, const std::string currkey);

	std::string dump(int depth = 0);

	void xmlassign(const std::string &key, const std::string &value);
	void xmlparse(const std::string &s, size_t &pos);

	void listkeys(const std::string &, std::vector<std::string> &,
	              bool longformat = true) const;
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
