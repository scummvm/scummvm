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

#include "glk/quest/geas_file.h"
#include "glk/quest/reserved_words.h"
#include "glk/quest/read_file.h"
#include "glk/quest/geas_util.h"
#include "glk/quest/geas_impl.h"
#include "glk/quest/streams.h"
#include "glk/quest/string.h"

namespace Glk {
namespace Quest {

void report_error(const String &s);

// FIXME: This requires global constructor
reserved_words obj_tag_property("look", "examine", "speak", "take", "alias", "prefix", "suffix", "detail", "displaytype", "gender", "article", "hidden", "invisible", (char *) NULL);

// FIXME: This requires global constructor
reserved_words room_tag_property("look", "alias", "prefix", "indescription", "description", "north", "south", "east", "west", "northwest", "northeast", "southeast", "southwest", "up", "down", "out", (char *) NULL);

void GeasFile::debug_print(String s) const {
	if (gi == NULL)
		cerr << s << endl;
	else
		gi->debug_print(s);
}

const GeasBlock *GeasFile::find_by_name(String type, String name) const {
	//name = lcase (name);
	for (uint i = 0; i < size(type); i ++) {
		//cerr << "find_by_name (" << type << ", " << name << "), vs. '"
		//     << block(type, i).name << "'\n";
		//if (block(type, i).lname == name)
		if (ci_equal(block(type, i).name, name))
			return &block(type, i);
	}
	return NULL;
}

const GeasBlock &GeasFile::block(String type, uint index) const {
	StringArrayIntMap::const_iterator iter;
	iter = type_indecies.find(type);
	if (!(iter != type_indecies.end() && index < (*iter)._value.size()))
		cerr << "Unable to find type " << type << "\n";

	assert(iter != type_indecies.end() && index < (*iter)._value.size());
	//assert (index >= 0 && index < size(type));
	return blocks[(*iter)._value[index]];
}

uint GeasFile::size(String type) const {
	//cerr << "GeasFile::size (" << type << ")" << endl;

	// SENSITIVE?
	//std::map<String, Common::Array<int>, CI_LESS>::const_iterator iter;
	StringArrayIntMap::const_iterator iter;
	//cerr << type_indecies << endl;
	iter = type_indecies.find(type);
	if (iter == type_indecies.end()) {
		//cerr << "  returning 0" << endl;
		return 0;
	}
	//cerr << "  returning " << (*iter)._value.size() << endl;
	return (*iter)._value.size();
}


bool GeasFile::obj_has_property(String objname, String propname) const {
	String tmp;
	return get_obj_property(objname, propname, tmp);
}

/**
 * Currently only works for actual objects, not rooms or the game
 */
//Set<String, CI_LESS> GeasFile::get_obj_keys (String obj) const
Set<String> GeasFile::get_obj_keys(String obj) const {
	//Set<String, CI_LESS> rv;
	Set<String> rv;
	get_obj_keys(obj, rv);
	return rv;
}

void GeasFile::get_obj_keys(String obj, Set<String> &rv) const {
	cerr << "get_obj_keys (gf, <" << obj << ">)\n";
	//Set<String> rv;

	uint c1, c2;
	String tok, line;
	reserved_words *rw = NULL;

	const GeasBlock *gb = find_by_name("object", obj);
	rw = &obj_tag_property;

	if (gb == NULL) {
		cerr << "No such object found, aborting\n";
		//return rv;
		return;
	}

	for (uint i = 0; i < gb->data.size(); i ++) {
		line = gb->data[i];
		cerr << "  handling line <" << line << ">\n";
		tok = first_token(line, c1, c2);
		// SENSITIVE?
		if (tok == "properties") {
			tok = next_token(line, c1, c2);
			if (is_param(tok)) {
				vstring params = split_param(param_contents(tok));
				for (uint j = 0; j < params.size(); j ++) {
					cerr << "   handling parameter <" << params[j] << ">\n";
					int k = params[j].find('=');
					// SENSITIVE?
					if (starts_with(params[j], "not ")) {
						rv.insert(trim(params[j].substr(4)));
						cerr << "     adding <" << trim(params[j].substr(4))
						     << ">\n";
					} else if (k == -1) {
						rv.insert(params[j]);
						cerr << "     adding <" << params[j] << ">\n";
					} else {
						rv.insert(trim(params[j].substr(0, k)));
						cerr << "     adding <" << trim(params[j].substr(0, k))
						     << ">\n";
					}
				}
			}
		}
		// SENSITIVE?
		else if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok))
				get_type_keys(param_contents(tok), rv);
		}
		//else if (has (tag_property, tok) && tag_property[tok])
		else if (rw != NULL && rw->has(tok)) {
			String tok1 = next_token(line, c1, c2);
			if (is_param(tok1))
				rv.insert(tok);
		}
	}

	cerr << "Returning (" << rv << ")\n";
}

void GeasFile::get_type_keys(String typen, Set<String> &rv) const {
	cerr << "get_type_keys (" << typen << ", " << rv << ")\n";
	const GeasBlock *gb = find_by_name("type", typen);
	if (gb == NULL) {
		cerr << "  g_t_k: Nonexistent type\n";
		return;
	}
	String line, tok;
	uint c1, c2;
	for (uint i = 0; i < gb->data.size(); i ++) {
		line = gb->data[i];
		//cerr << "    g_t_k: Handling line '" << line << "'\n";
		tok = first_token(line, c1, c2);
		// SENSISTIVE?
		if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok)) {
				get_type_keys(param_contents(tok), rv);
				cerr << "      g_t_k: Adding <" << tok << "> to rv: " << rv << "\n";
			}
		}
		// SENSITIVE?
		else if (tok == "action") {
			cerr << "       action, skipping\n";
		} else {
			int ch = line.find('=');
			if (ch != -1) {
				rv.insert(trim(line.substr(0, ch)));
				cerr << "      adding <" << trim(line.substr(0, ch)) << ">\n";
			}
		}
	}
	cerr << "Returning (" << rv << ")\n";
}

bool GeasFile::get_obj_property(String objname, String propname, String &string_rv) const {
	cerr << "g_o_p: Getting prop <" << propname << "> of obj <" << objname << ">\n";
	string_rv = "!";
	bool bool_rv = false;

	//cerr << "obj_types == " << obj_types << endl;
	/*
	cerr << "obj_types == \n";
	for (map<String, String>::const_iterator iter = obj_types.begin();
	     iter != obj_types.end(); iter ++)
	  cerr << "  " << (*iter)._key << " -> " << (*iter)._value << "\n";
	cerr << ".\n";
	*/

	/*
	String objtype;

	if (objname == "game")
	  objtype = "game";
	else if (!has (obj_types, objname))
	  {
	    debug_print ("Checking property of nonexistent object " + objname);
	    return false;
	  }
	else
	  objtype = (*obj_types.find(objname))._value;
	*/

	if (!has(obj_types, objname)) {
		debug_print("Checking nonexistent object <" + objname + "> for property <" + propname + ">");
		return false;
	}
	String objtype = (*obj_types.find(objname))._value;

	const GeasBlock *geasBlock = find_by_name(objtype, objname);

	String not_prop = "not " + propname;
	uint c1, c2;
	assert(geasBlock != NULL);
	//assert (geasBlock->data != NULL);
	for (uint i = 0; i < geasBlock->data.size(); i ++) {
		String line = geasBlock->data[i];
		//cerr << "  g_o_p: Handling line <" << line << ">\n";
		String tok = first_token(line, c1, c2);
		// SENSITIVE?
		if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok))
				get_type_property(param_contents(tok), propname, bool_rv, string_rv);
			else {
				debug_print("Expected parameter for type in " + line);
			}
		}
		// SENSITIVE?
		else if (tok == "properties") {
			tok = next_token(line, c1, c2);
			if (!is_param(tok)) {
				debug_print("Expected param on line " + line);
				continue;
			}
			Common::Array<String> props = split_param(param_contents(tok));
			for (uint j = 0; j < props.size(); j ++) {
				//cerr << "    g_o_p: Comparing against <" << props[j] << ">\n";
				int index;
				if (props[j] == propname) {
					//cerr << "      g_o_p: Present but empty, blanking\n";
					string_rv = "";
					bool_rv = true;
				} else if (props[j] == not_prop) {
					//cerr << "      g_o_p: Negation, removing\n";
					string_rv = "!";
					bool_rv = false;
				} else if ((index = props[j].find('=')) != -1 &&
				           (trim(props[j].substr(0, index)) == propname)) {
					string_rv = props[j].substr(index + 1);
					bool_rv = true;
					//cerr << "      g_o_p: Normal prop, now to <" << string_rv << ">\n";
				}
			}
		}
	}
	cerr << "g_o_p: Ultimately returning " << (bool_rv ? "true" : "false")
	     << ", with String <" << string_rv << ">\n\n";
	return bool_rv;
}

void GeasFile::get_type_property(String typenamex, String propname, bool &bool_rv, String &string_rv) const {
	//cerr << "  Checking type <" << typenamex << "> for prop <" << propname << ">\n";
	const GeasBlock *geasBlock = find_by_name("type", typenamex);
	if (geasBlock == NULL) {
		debug_print("Object of nonexistent type " + typenamex);
		return;
	}
	for (uint i = 0; i < geasBlock->data.size(); i ++) {
		String line = geasBlock->data[i];
		//cerr << "    Comparing vs. line <" << line << ">\n";
		uint c1, c2;
		int p;
		String tok = first_token(line, c1, c2);

		// SENSITIVE?
		if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok))
				get_type_property(param_contents(tok), propname, bool_rv, string_rv);
		} else if (line == propname) {
			bool_rv = true;
			string_rv = "";
		} else {
			p = line.find('=');
			if (p != -1) {
				tok = trim(line.substr(0, p));
				if (tok == propname) {
					string_rv = trim(line.substr(p + 1));
					bool_rv = true;
				}
			}
		}
		/*
		if (tok == propname)
		{
		  cerr << "      match...";
		  tok = next_token (line, c1, c2);
		  if (tok == "")
		    {
		      bool_rv = true;
		      string_rv = "";
		      //cerr << " present but empty\n";
		    }
		  else if (tok == "=")
		    {
		      bool_rv = true;
		      string_rv = trim (line.substr (c2));
		      //cerr << " now <" << string_rv << ">\n";
		    }
		  else
		    {
		      cerr << "Bad line while checking " << typenamex << " for prop "
		      << propname << ": " << line << endl;
		    }
		}
		     else if (tok == "type")
		{
		  tok = next_token (line, c1, c2);
		  if (is_param (tok))
		    get_type_property (param_contents(tok), propname, bool_rv, string_rv);
		}
		*/
	}
}



bool GeasFile::obj_of_type(String objname, String typenamex) const {
	if (!has(obj_types, objname)) {
		debug_print("Checking nonexistent obj <" + objname + "> for type <" +
		            typenamex + ">");
		return false;
	}
	String objtype = (*obj_types.find(objname))._value;

	const GeasBlock *geasBlock = find_by_name(objtype, objname);

	uint c1, c2;
	assert(geasBlock != NULL);
	for (uint i = 0; i < geasBlock->data.size(); i ++) {
		String line = geasBlock->data[i];
		String tok = first_token(line, c1, c2);
		// SENSITIVE?
		if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok)) {
				if (type_of_type(param_contents(tok), typenamex))
					return true;
			} else {
				debug_print("Eg_o_p: xpected parameter for type in " + line);
			}
		}
	}
	return false;
}


bool GeasFile::type_of_type(String subtype, String supertype) const {
	if (ci_equal(subtype, supertype))
		return true;
	//cerr << "  Checking type <" << subtype << "> for type <" << supertype << ">\n";
	const GeasBlock *geasBlock = find_by_name("type", subtype);
	if (geasBlock == NULL) {
		debug_print("t_o_t: Nonexistent type " + subtype);
		return false;
	}
	for (uint i = 0; i < geasBlock->data.size(); i ++) {
		String line = geasBlock->data[i];
		//cerr << "    Comparing vs. line <" << line << ">\n";
		uint c1, c2;
		String tok = first_token(line, c1, c2);
		// SENSITIVE?
		if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok) && type_of_type(param_contents(tok), supertype))
				return true;
		}
	}
	return false;
}



bool GeasFile::get_obj_action(String objname, String propname, String &string_rv) const {
	cerr << "g_o_a: Getting action <" << propname << "> of object <" << objname << ">\n";
	string_rv = "!";
	bool bool_rv = false;

	//cerr << "obj_types == " << obj_types << endl;
	/*
	cerr << "obj_types == \n";
	for (map<String, String>::const_iterator iter = obj_types.begin();
	     iter != obj_types.end(); iter ++)
	  cerr << "  " << (*iter)._key << " -> " << (*iter)._value << "\n";
	cerr << ".\n";
	*/
	if (!has(obj_types, objname)) {
		debug_print("Checking nonexistent object <" + objname + "> for action <" + propname + ">.");
		return false;
	}
	String objtype = (*obj_types.find(objname))._value;

	//reserved_words *rw;

	const GeasBlock *geasBlock = find_by_name(objtype, objname);
	String not_prop = "not " + propname;
	uint c1, c2;
	for (uint i = 0; i < geasBlock->data.size(); i ++) {
		String line = geasBlock->data[i];
		//cerr << "  g_o_a: Handling line <" << line << ">\n";
		String tok = first_token(line, c1, c2);
		// SENSITIVE?
		if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok))
				get_type_action(param_contents(tok), propname, bool_rv, string_rv);
			else {
				gi->debug_print("Expected parameter for type in " + line);
			}
		}
		/*
		else if (rw != NULL && tok == propname && rw->has(propname))
		{
		  tok = next_token (line, c1, c2);
		  if (is_param(tok))
		    {
		      cerr << "   Parameter, skipping\n";
		    }
		  else
		    {
		      //cerr << "   Action, skipping\n";
		      cerr << "   Action, string_rv is now <" << string_rv << ">\n";
		      string_rv = line.substr (c1);
		      bool_rv = true;
		    }
		}
		     */
		// SENSITIVE?
		else if (tok == "action") {
			tok = next_token(line, c1, c2);
			if (is_param(tok) && param_contents(tok) == propname) {
				if (c2 + 1 < line.length())
					string_rv = line.substr(c2 + 1);
				else
					string_rv = "";
				bool_rv = true;
				cerr << "   Action line, string_rv now <" << string_rv << ">\n";
			}
		}
	}

	cerr << "g_o_a: Ultimately returning value " << (bool_rv ? "true" : "false")  << ", with String <" << string_rv << ">\n\n";

	return bool_rv;
}

void GeasFile::get_type_action(String typenamex, String actname, bool &bool_rv, String &string_rv) const {
	//cerr << "  Checking type <" << typenamex << "> for action <" << actname << ">\n";
	const GeasBlock *geasBlock = find_by_name("type", typenamex);
	if (geasBlock == NULL) {
		debug_print("Object of nonexistent type " + typenamex);
		return;
	}
	for (uint i = 0; i < geasBlock->data.size(); i ++) {
		String line = geasBlock->data[i];
		//cerr << "    g_t_a: Comparing vs. line <" << line << ">\n";
		uint c1, c2;
		String tok = first_token(line, c1, c2);
		// SENSITIVE?
		if (tok == "action") {
			//cerr << "      match...\n";
			tok = next_token(line, c1, c2);
			if (is_param(tok) && param_contents(tok) == actname) {
				bool_rv = true;
				string_rv = line.substr(c2);
				//cerr << " present: {" + string_rv + "}\n";
			}
		}
		// SENSITIVE?
		else if (tok == "type") {
			tok = next_token(line, c1, c2);
			if (is_param(tok))
				get_type_action(param_contents(tok), actname, bool_rv, string_rv);
		}
	}
}

void GeasFile::register_block(String blockname, String blocktype) {
	cerr << "registering block " << blockname << " / " << blocktype << endl;
	if (has(obj_types, blockname)) {
		String errdesc = "Trying to register block of named <" + blockname +
		                 "> of type <" + blocktype + "> when there is already one, of type <" +
		                 obj_types[blockname] + ">";
		error("%s", errdesc.c_str());
	}
	obj_types[blockname] = blocktype;
}

String GeasFile::static_svar_lookup(String varname) const {
	cerr << "static_svar_lookup(" << varname << ")" << endl;
	//varname = lcase (varname);
	for (uint i = 0; i < size("variable"); i++) {
		//if (blocks[i].lname == varname)
		if (ci_equal(blocks[i].name, varname)) {
			String rv;
			String tok;
			uint c1, c2;
			bool found_typeline = false;
			for (uint j = 0; j < blocks[i].data.size(); j++) {
				String line = blocks[i].data[j];
				tok = first_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "type") {
					tok = next_token(line, c1, c2);
					// SENSITIVE?
					if (tok == "numeric")
						error("Trying to evaluate int var '%s' as String", varname.c_str());
					// SENSITIVE?
					if (tok != "String")
						error("Bad variable type %s", tok.c_str());
					found_typeline = true;
				}
				// SENSITIVE?
				else if (tok == "value") {
					tok = next_token(line, c1, c2);
					if (!is_param(tok))
						error("Expected param after value in %s", line.c_str());
					rv = param_contents(tok);
				}
			}
			if (!found_typeline)
				error("%s is a numeric variable", varname.c_str());
			cerr << "static_svar_lookup(" << varname << ") -> \"" << rv << "\"" << endl;
			return rv;
		}
	}

	debug_print("Variable <" + varname + "> not found.");
	return "";
}

String GeasFile::static_ivar_lookup(String varname) const {
	//varname = lcase (varname);
	for (uint i = 0; i < size("variable"); i ++)
		//if (blocks[i].lname == varname)
		if (ci_equal(blocks[i].name, varname)) {
			String rv;
			String tok;
			uint c1, c2;
			for (uint j = 0; j < blocks[i].data.size(); j ++) {
				String line = blocks[i].data[j];
				tok = first_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "type") {
					tok = next_token(line, c1, c2);
					// SENSITIVE?
					if (tok == "String")
						error("Trying to evaluate String var '%s' as numeric", varname.c_str());
					// SENSITIVE?
					if (tok != "numeric")
						error("Bad variable type %s", tok.c_str());
				}
				// SENSITIVE?
				else if (tok == "value") {
					tok = next_token(line, c1, c2);
					if (!is_param(tok))
						error("Expected param after value in %s", line.c_str());
					rv = param_contents(tok);
				}
			}
			return rv;
		}
	debug_print("Variable <" + varname + "> not found");
	return "-32768";
}

String GeasFile::static_eval(String input) const {
	//cerr << "static_eval (" << input << ")" << endl;
	String rv = "";
	for (uint i = 0; i < input.length(); i ++) {
		if (input[i] == '#') {
			uint j;
			for (j = i + 1; j < input.length() && input[j] != '#'; j ++)
				;
			if (j == input.length())
				error("Error processing '%s', odd hashes", input.c_str());
			uint k;
			for (k = i + 1; k < j && input[k] != ':'; k ++)
				;
			if (k == ':') {
				String objname;
				if (input[i + 1] == '(' && input[k - 1] == ')')
					objname = static_svar_lookup(input.substr(i + 2, k - i - 4));
				else
					objname = input.substr(i + 1, k - i - 2);
				cerr << "  objname == '" << objname << endl;
				//rv += get_obj_property (objname, input.substr (k+1, j-k-2));
				String tmp;
				bool had_var;

				String objprop = input.substr(k + 1, j - k - 2);
				cerr << "  objprop == " << objprop << endl;
				had_var = get_obj_property(objname, objprop, tmp);
				rv += tmp;
				if (!had_var)
					debug_print("Requesting nonexistent property <" + objprop +
					            "> of object <" + objname + ">");
			} else {
				cerr << "i == " << i << ", j == " << j << ", length is " << input.length() << endl;
				cerr << "Looking up static var " << input.substr(i + 1, j - i - 1) << endl;
				rv += static_svar_lookup(input.substr(i + 1, j - i - 1));
			}
			i = j;
		} else if (input[i] == '%') {
			uint j;
			for (j = i; j < input.length() && input[j] != '%'; j ++)
				;
			if (j == input.length())
				error("Error processing '%s', unmatched %%", input.c_str());
			rv += static_ivar_lookup(input.substr(i + 1, j - i - 2));
			i = j;
		} else
			rv += input[i];
	}
	if (rv != input)
		cerr << "*** CHANGED ***\n";
	//cerr << "static_eval (" << input << ") --> \"" << rv << "\"" << endl;
	return rv;
}

} // End of namespace Quest
} // End of namespace Glk
