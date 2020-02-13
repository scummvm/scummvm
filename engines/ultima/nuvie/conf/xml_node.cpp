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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/misc.h"
#include "ultima/nuvie/conf/xml_node.h"
#include "ultima/nuvie/nuvie.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

using Std::string;
using Std::vector;

static  string  encode_entity(const string &s);
static  string  close_tag(const string &s);

XMLNode::~XMLNode() {
	for (Std::vector<XMLNode *>::iterator i = nodelist.begin();
	        i != nodelist.end(); ++i) {
		delete *i;
	}
}

const string &XMLNode::reference(const string &h, bool &exists) {
	if (h.find('/') == string::npos) {
		// Must refer to me.
		if (id == h) {
			exists = true;
			return content;
		}
	} else {
		// Otherwise we want to split the string at the first /
		// then locate the branch to walk, and pass the rest
		// down.

		string k;
		k = h.substr(h.find('/') + 1);
		string k2 = k.substr(0, k.find('/'));
		for (Std::vector<XMLNode *>::iterator it = nodelist.begin();
		        it != nodelist.end(); ++it) {
			if ((*it)->id == k2)
				return (*it)->reference(k, exists);
		}
	}

	exists = false;
	return g_engine->c_empty_string;
}


const XMLNode *XMLNode::subtree(const string &h) const {
	if (h.find('/') == string::npos) {
		// Must refer to me.
		if (string_i_compare(id, h))
			return this;
	} else {
		// Otherwise we want to split the string at the first /
		// then locate the branch to walk, and pass the rest
		// down.

		string k;
		k = h.substr(h.find('/') + 1);
		string k2 = k.substr(0, k.find('/'));
		for (Std::vector<XMLNode *>::const_iterator it = nodelist.begin();
		        it != nodelist.end(); ++it) {
			if (string_i_compare((*it)->id, k2)) {
				return (*it)->subtree(k);
			}
		}
	}

	return 0;
}


string XMLNode::dump(int depth) {
	string s(depth, ' ');

	s += "<";
	s += id;
	s += ">";
	if (id[id.length() - 1] != '/') {
		if (nodelist.empty() == false)
			s += "\n";
		for (Std::vector<XMLNode *>::const_iterator it = nodelist.begin();
		        it != nodelist.end(); ++it) {
			s += (**it).dump(depth + 1);
		}

		if (content.length()) {
			//s += string(depth,' ');
			s += encode_entity(content);
		}
		if (id[0] == '?') {
			return s;
		}
		//if(content.length())
		//  s += "\n";

		if (!no_close) {
			if (!content.length())
				s += string(depth, ' ');

			s += "</";
			s += close_tag(id);
			s += ">\n";
		}
	}

	return s;
}

void XMLNode::xmlassign(const string &key, const string &value) {
	if (key.find('/') == string::npos) {
		// Must refer to me.
		if (id == key)
			content = value;
		else
			DEBUG(0, LEVEL_CRITICAL, "Walking the XML tree failed to create a final node.");
		return;
	}
	string k;
	k = key.substr(key.find('/') + 1);
	string k2 = k.substr(0, k.find('/'));
	for (Std::vector<XMLNode *>::iterator it = nodelist.begin();
	        it != nodelist.end(); ++it) {
		if ((*it)->id == k2) {
			(**it).xmlassign(k, value);
			return;
		}
	}

	// No match, so create a new node and do recursion
	XMLNode *t = new XMLNode(k2);
	nodelist.push_back(t);
	(*t).xmlassign(k, value);
}


void XMLNode::listkeys(const string &key, vector<string> &vs,
                       bool longformat) const {
	string s(key);
	s += "/";

	for (Std::vector<XMLNode *>::const_iterator it = nodelist.begin();
	        it != nodelist.end(); ++it) {
		if (!longformat)
			vs.push_back((*it)->id);
		else
			vs.push_back(s + (*it)->id);
	}
}

static string encode_entity(const string &s) {
	string  ret;

	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		switch (*it) {
		case '<':
			ret += "&lt;";
			break;
		case '>':
			ret += "&gt;";
			break;
		case '"':
			ret += "&quot;";
			break;
		case '\'':
			ret += "&apos;";
			break;
		case '&':
			ret += "&amp;";
			break;
		default:
			ret += *it;
		}
	}
	return ret;
}

static string decode_entity(const string &s, size_t &pos) {
	size_t old_pos = pos;
	string::size_type entity_name_len = s.findFirstOf("; \t\r\n", pos) - pos - 1;

	/* Call me paranoid... but I don't think having an end-of-line or similar
	    inside a &...; expression is 'good', valid though it may be. */
	assert(s[pos + entity_name_len + 1] == ';');

	string entity_name = s.substr(pos + 1, entity_name_len);

	pos += entity_name_len + 2;

	// Std::cout << "DECODE: " << entity_name << endl;

	if (entity_name == "amp")  return string("&");
	else if (entity_name == "apos") return string("'");
	else if (entity_name == "quot") return string("\"");
	else if (entity_name == "lt")   return string("<");
	else if (entity_name == "gt")   return string(">");

	return s.substr(old_pos, entity_name_len + 2);
}

static string close_tag(const string &s) {
	if (s.find(" ") == string::npos)
		return s;

	return s.substr(0, s.find(" "));
}


static void trim(string &s) {
	// Clean off leading whitespace
	while (s.length() && s[0] <= 32) {
		s = s.substr(1);
	}
	// Clean off trailing whitespace
	while (s.length() && s[s.length() - 1] <= 32) {
		s.erase(s.length() - 1);
	}
}

void XMLNode::xmlparse(const string &s, size_t &pos) {
	bool intag = true;

	id = "";
	while (pos < s.length()) {
		switch (s[pos]) {
		case '<': {
			// New tag?
			if (s[pos + 1] == '/') {
				// No. Close tag.
				while (s[pos] != '>')
					pos++;
				++pos;
				trim(content);
				return;
			}
			XMLNode *t = new XMLNode;
			++pos;
			t->xmlparse(s, pos);
			nodelist.push_back(t);
			break;
		}
		case '>':
			// End of tag
			if (s[pos - 1] == '/') {
				if (s[pos - 2] == '<') {
					++pos;
					return; // An empty tag
				} else {
					++pos;
					no_close = true;
					return;
				}
			} else if ((id[0] == '!') && (id[1] == '-') && (id[2] == '-')) {
				++pos;
				no_close = true;
				return;
			}
			++pos;
			intag = false;
			if (s[pos] < 32)
				++pos;
			break;
		case '&':
			content += decode_entity(s, pos);
			break;
		default:
			if (intag)
				id += s[pos++];
			else
				content += s[pos++];
		}
	}
	trim(content);
}

/* Returns a list of key->value pairs that are found under the provided
   'basekey'.
    Ignores comments (<!-- ... --> and doesn't return them.
    Returns true if search is 'finished'
*/
bool XMLNode::searchpairs(KeyTypeList &ktl, const string &basekey,
                          const string currkey, const unsigned int pos) {
	/* If our 'current key' is longer then the key we're serching for
	    we've obviously gone too deep in this branch, and we won't find
	    it here. */
	if ((currkey.size() <= basekey.size()) && (id[0] != '!')) {
		/* If we've found it, return every key->value pair under this key,
		    then return true, since we've found the key we were looking for.*/
		if (basekey == currkey + id) {
			for (Std::vector<XMLNode *>::iterator i = nodelist.begin();
			        i != nodelist.end(); ++i)
				if ((*i)->id[0] != '!')
					(*i)->selectpairs(ktl, "");
			return true;
		}
		/* Else, keep searching for the key under it's subnodes */
		else
			for (Std::vector<XMLNode *>::iterator i = nodelist.begin();
			        i != nodelist.end(); ++i)
				if ((*i)->searchpairs(ktl, basekey, currkey + id + '/', pos))
					return true;
	}
	return false;
}

/* Just adds every key->value pair under the this node to the ktl */
void XMLNode::selectpairs(KeyTypeList &ktl, const Std::string currkey) {
	ktl.push_back(KeyType(currkey + id, content));

	for (Std::vector<XMLNode *>::iterator i = nodelist.begin();
	        i != nodelist.end(); ++i) {
		(*i)->selectpairs(ktl, currkey + id + '/');
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
