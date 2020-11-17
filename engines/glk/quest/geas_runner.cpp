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

#include "glk/quest/geas_runner.h"
#include "glk/quest/read_file.h"
#include "glk/quest/geas_state.h"
#include "glk/quest/geas_util.h"
#include "glk/quest/reserved_words.h"
#include "glk/quest/geas_impl.h"
#include "glk/quest/quest.h"
#include "glk/quest/streams.h"
#include "glk/quest/string.h"

namespace Glk {
namespace Quest {

class GeasInterface;

static const char *dir_names[] = {"north", "south", "east", "west", "northeast", "northwest", "southeast", "southwest", "up", "down", "out"};
static const char *short_dir_names[] = {"n", "s", "e", "w", "ne", "nw", "se", "sw", "u", "d", "out"};

const ObjectRecord *get_obj_record(const Common::Array<ObjectRecord> &v, const String &name) {
	for (uint i = 0; i < v.size(); i ++)
		if (ci_equal(v[i].name, name))
			return &v[i];
	return nullptr;
}



GeasRunner *GeasRunner::get_runner(GeasInterface *gi) {
	return new geas_implementation(gi);
}

bool geas_implementation::find_ivar(String name, uint &rv) const {
	for (uint n = 0; n < state.ivars.size(); n ++)
		if (ci_equal(state.ivars[n].name, name)) {
			rv = n;
			return true;
		}
	return false;
}

bool geas_implementation::find_svar(String name, uint &rv) const {
	//name = lcase (name);
	for (uint n = 0; n < state.svars.size(); n ++)
		if (ci_equal(state.svars[n].name, name)) {
			rv = n;
			return true;
		}
	return false;
}

void geas_implementation::set_svar(String varname, String varval) {
	cerr << "set_svar (" << varname << ", " << varval << ")\n";
	int i1 = varname.find('[');
	if (i1 == -1)
		return set_svar(varname, 0, varval);
	if (varname[varname.length() - 1] != ']') {
		gi->debug_print("set_svar: Badly formatted name " + varname);
		return;
	}
	String arrayname = varname.substr(0, i1);
	String indextext = varname.substr(i1 + 1, varname.length() - i1 - 2);
	cerr << "set_svar(" << varname << ") --> set_svar (" << arrayname << ", " << indextext << ")\n";
	for (uint c3 = 0; c3 < indextext.size(); c3 ++)
		if (indextext[c3] < '0' || indextext[c3] > '9') {
			set_svar(arrayname, get_ivar(indextext), varval);
			return;
		}
	set_svar(arrayname, parse_int(indextext), varval);
	return;
}

void geas_implementation::set_svar(String varname, uint index, String varval) {
	uint n, m;
	if (!find_svar(varname, n)) {
		if (find_ivar(varname, m)) {
			gi->debug_print("Defining " + varname + " as String variable when there is already a numeric variable of that name.");
			return;
		}
		SVarRecord svr;
		svr.name = varname;
		n = state.svars.size();
		state.svars.push_back(svr);
	}
	state.svars[n].set(index, varval);
	if (index == 0) {
		for (uint varn = 0; varn < gf.size("variable"); varn ++) {
			const GeasBlock &go(gf.block("variable", varn));
			if (ci_equal(go.name, varname)) {
				String script = "";
				uint c1, c2;
				for (uint j = 0; j < go.data.size(); j ++)
					// SENSITIVE ?
					if (first_token(go.data[j], c1, c2) == "onchange")
						script = trim(go.data[j].substr(c2 + 1));
				if (script != "")
					run_script(script);
			}
		}
	}
}

String geas_implementation::get_svar(String varname) const {
	int i1 = varname.find('[');
	if (i1 == -1)
		return get_svar(varname, 0);
	if (varname[varname.length() - 1] != ']') {
		gi->debug_print("get_svar: badly formatted name " + varname);
		return "";
	}
	String arrayname = varname.substr(0, i1);
	String indextext = varname.substr(i1 + 1, varname.length() - i1 - 2);
	cerr << "get_svar(" << varname << ") --> get_svar (" << arrayname << ", " << indextext << ")\n";
	for (uint c3 = 0; c3 < indextext.size(); c3 ++)
		if (indextext[c3] < '0' || indextext[c3] > '9')
			return get_svar(arrayname, get_ivar(indextext));
	return get_svar(arrayname, parse_int(indextext));
}
String geas_implementation::get_svar(String varname, uint index) const {
	for (uint i = 0; i < state.svars.size(); i ++) {
		if (ci_equal(state.svars[i].name, varname))
			return state.svars[i].get(index);
	}

	gi->debug_print("get_svar (" + varname + ", " + string_int(index) + "): No such variable defined.");
	return "";
}

int geas_implementation::get_ivar(String varname) const {
	int i1 = varname.find('[');
	if (i1 == -1)
		return get_ivar(varname, 0);
	if (varname[varname.length() - 1] != ']') {
		gi->debug_print("get_ivar: Badly formatted name " + varname);
		return -32767;
	}
	String arrayname = varname.substr(0, i1);
	String indextext = varname.substr(i1 + 1, varname.length() - i1 - 2);
	cerr << "get_ivar(" << varname << ") --> get_ivar (" << arrayname << ", " << indextext << ")\n";
	for (uint c3 = 0; c3 < indextext.size(); c3 ++)
		if (indextext[c3] < '0' || indextext[c3] > '9')
			return get_ivar(arrayname, get_ivar(indextext));
	return get_ivar(arrayname, parse_int(indextext));
}
int geas_implementation::get_ivar(String varname, uint index) const {
	for (uint i = 0; i < state.ivars.size(); i ++)
		if (ci_equal(state.ivars[i].name, varname))
			return state.ivars[i].get(index);
	gi->debug_print("get_ivar: Tried to read undefined int '" + varname +
	                "' [" + string_int(index) + "]");
	return -32767;
}
void geas_implementation::set_ivar(String varname, int varval) {
	int i1 = varname.find('[');
	if (i1 == -1)
		return set_ivar(varname, 0, varval);
	if (varname[varname.length() - 1] != ']') {
		gi->debug_print("set_ivar: Badly formatted name " + varname);
		return;
	}
	String arrayname = varname.substr(0, i1);
	String indextext = varname.substr(i1 + 1, varname.length() - i1 - 2);
	cerr << "set_svar(" << varname << ") --> set_svar (" << arrayname << ", " << indextext << ")\n";
	for (uint c3 = 0; c3 < indextext.size(); c3 ++)
		if (indextext[c3] < '0' || indextext[c3] > '9') {
			set_ivar(arrayname, get_ivar(indextext), varval);
			return;
		}
	set_ivar(arrayname, parse_int(indextext), varval);
}

void geas_implementation::set_ivar(String varname, uint index, int varval) {
	uint n, m;
	if (!find_ivar(varname, n)) {
		if (find_svar(varname, m)) {
			gi->debug_print("Defining " + varname + " as numeric variable when there is already a String variable of that name.");
			return;
		}
		IVarRecord ivr;
		ivr.name = varname;
		n = state.ivars.size();
		state.ivars.push_back(ivr);
	}
	state.ivars[n].set(index, varval);
	if (index == 0) {
		for (uint varn = 0; varn < gf.size("variable"); varn ++) {
			const GeasBlock &go(gf.block("variable", varn));
			//if (go.lname == varname)
			if (ci_equal(go.name, varname)) {
				String script = "";
				uint c1, c2;
				for (uint j = 0; j < go.data.size(); j ++)
					// SENSITIVE?
					if (first_token(go.data[j], c1, c2) == "onchange")
						script = trim(go.data[j].substr(c2 + 1));
				if (script != "")
					run_script(script);
			}
		}
	}
}



Common::WriteStream &operator<<(Common::WriteStream &o, const match_binding &mb) {
	o << "MB['" << mb.var_name << "' == '" << mb.var_text << "' @ "
	  << mb.start << " to " << mb.end << "]";
	return o;
}

String match_binding::tostring() {
	ostringstream oss;
	oss << *this;
	return oss.str();
}

Common::WriteStream &operator<<(Common::WriteStream &o, const Set<String> &s) {
	o << "{ ";
	for (Set<String>::const_iterator i = s.begin(); i != s.end(); i ++) {
		if (i != s.begin())
			o << ", ";
		o << (*i);
	}
	o << " }";
	return o;
}

bool geas_implementation::has_obj_action(String obj, String prop) const {
	String tmp;
	return get_obj_action(obj, prop, tmp);
}


bool geas_implementation::get_obj_action(String objname, String actname,
        String &rv) const {
	//String backup_object = this_object;
	//this_object = objname;

	cerr << "get_obj_action (" << objname << ", " << actname << ")\n";
	String tok;
	uint c1, c2;
	for (uint i = state.props.size() - 1; i + 1 > 0; i --)
		if (state.props[i].name == objname) {
			String line = state.props[i].data;
			// SENSITIVE?
			if (first_token(line, c1, c2) != "action")
				continue;
			tok = next_token(line, c1, c2);
			if (!is_param(tok) || ci_equal(param_contents(tok), actname))
				continue;
			rv = trim(line.substr(c2));
			cerr << "  g_o_a: returning true, \"" << rv << "\".";
			return true;
		}
	return gf.get_obj_action(objname, actname, rv);
	//bool bool_rv = gf.get_obj_action (objname, actname, rv);
	//this_object = backup_object;
	//return bool_rv;
}

bool geas_implementation::has_obj_property(String obj, String prop) const {
	String tmp;
	return get_obj_property(obj, prop, tmp);
}

bool geas_implementation::get_obj_property(String obj, String prop,
        String &string_rv) const {
	String is_prop = "properties " + prop;
	String not_prop = "properties not " + prop;
	for (uint i = state.props.size() - 1; i + 1 > 0; i --)
		if (ci_equal(state.props[i].name, obj)) {
			String dat = state.props[i].data;
			//cerr << "In looking for " << obj << ":" << prop << ", got line "
			//     << dat << endl;
			if (ci_equal(dat, not_prop)) {
				//cerr << "   not_prop, returning false\n";
				string_rv = "!";
				return false;
			}
			if (ci_equal(dat, is_prop)) {
				//cerr << "   is_prop, returning true\n";
				string_rv = "";
				return true;
			}
			int index = dat.find('=');
			if (index != -1 && ci_equal(dat.substr(0, index), is_prop)) {
				string_rv = dat.substr(index + 1);
				return true;
			}
		}
	return gf.get_obj_property(obj, prop, string_rv);
}

void geas_implementation::set_obj_property(String obj, String prop) {
	state.props.push_back(PropertyRecord(obj, "properties " + prop));
	if (ci_equal(prop, "hidden") || ci_equal(prop, "not hidden") ||
	        ci_equal(prop, "invisible") || ci_equal(prop, "not invisible")) {
		gi->update_sidebars();
		regen_var_objects();
	}
}

void geas_implementation::set_obj_action(String obj, String act) {
	state.props.push_back(PropertyRecord(obj, "action " + act));
}

void geas_implementation::move(String obj, String dest) {
	for (uint i = 0; i < state.objs.size(); i ++)
		if (ci_equal(state.objs[i].name, obj)) {
			state.objs[i].parent = dest;
			gi->update_sidebars();
			regen_var_objects();
			return;
		}
	gi->debug_print("Tried to move nonexistent object '" + obj +
	                "' to '" + dest + "'.");
}

String geas_implementation::get_obj_parent(String obj) {
	//obj = lcase (obj);
	for (uint i = 0; i < state.objs.size(); i ++)
		if (ci_equal(state.objs[i].name, obj))
			return state.objs[i].parent;
	gi->debug_print("Tried to find parent of nonexistent object " + obj);
	return "";
}

void geas_implementation::goto_room(String room) {
	state.location = room;
	regen_var_room();
	regen_var_dirs();
	regen_var_look();
	regen_var_objects();
	String scr;
	if (get_obj_action(room, "script", scr))
		run_script_as(room, scr);
	//run_script (scr);
	look();
}

void geas_implementation::display_error(String errorname, String obj) {
	cerr << "display_error (" << errorname << ", " << obj << ")\n";
	if (obj != "") {
		String tmp;
		if (!get_obj_property(obj, "gender", tmp))
			tmp = "it";
		set_svar("quest.error.gender", tmp);

		if (!get_obj_property(obj, "article", tmp))
			tmp = "it";
		set_svar("quest.error.article", tmp);

		cerr << "In erroring " << errorname << " / " << obj << ", qeg == "
		     << get_svar("quest.error.gender") << ", qea == "
		     << get_svar("quest.error.article") << endl;
		// TODO quest.error.charactername
	}

	const GeasBlock *game = gf.find_by_name("game", "game");
	assert(game != NULL);
	String tok;
	uint c1, c2;
	for (uint i = 0; i < game->data.size(); i ++) {
		String line = game->data[i];
		tok = first_token(line, c1, c2);
		// SENSITIVE?
		if (tok == "error") {
			tok = next_token(line, c1, c2);
			if (is_param(tok)) {
				String text = param_contents(tok);
				int index = text.find(';');
				String errortype = trim(text.substr(0, index));
				// SENSITIVE?
				if (errortype == errorname) {
					print_eval_p(trim(text.substr(index + 1)));
					return;
				}
			} else
				gi->debug_print("Bad error line: " + line);
		}
	}
	//print_formatted ("Default error " + errorname);

	// ARE THESE SENSITIVE?
	if (errorname == "badcommand")
		print_eval("I don't understand your command. Type HELP for a list of valid commands.");
	else if (errorname == "badgo")
		print_eval("I don't understand your use of 'GO' - you must either GO in some direction, or GO TO a place.");
	else if (errorname == "badgive")
		print_eval("You didn't say who you wanted to give that to.");
	else if (errorname == "badcharacter")
		print_eval("I can't see anybody of that name here.");
	else if (errorname == "noitem")
		print_eval("You don't have that.");
	else if (errorname == "itemunwanted")
		print_eval_p("#quest.error.gender# doesn't want #quest.error.article#.");
	else if (errorname == "badlook")
		print_eval("You didn't say what you wanted to look at.");
	else if (errorname == "badthing")
		print_eval("I can't see that here.");
	else if (errorname == "defaultlook")
		print_eval("Nothing out of the ordinary.");
	else if (errorname == "defaultspeak")
		print_eval_p("#quest.error.gender# says nothing.");
	else if (errorname == "baditem")
		print_eval("I can't see that anywhere.");
	else if (errorname == "defaulttake")
		print_eval("You pick #quest.error.article# up.");
	else if (errorname == "baduse")
		print_eval("You didn't say what you wanted to use that on.");
	else if (errorname == "defaultuse")
		print_eval("You can't use that here.");
	else if (errorname == "defaultout")
		print_eval("There's nowhere you can go out to around here.");
	else if (errorname == "badplace")
		print_eval("You can't go there.");
	else if (errorname == "defaultexamine")
		print_eval("Nothing out of the ordinary.");
	else if (errorname == "badtake")
		print_eval("You can't take #quest.error.article#.");
	else if (errorname == "cantdrop")
		print_eval("You can't drop that here.");
	else if (errorname == "defaultdrop")
		print_eval("You drop #quest.error.article#.");
	else if (errorname == "baddrop")
		print_eval("You are not carrying such a thing.");
	else if (errorname == "badpronoun")
		print_eval("I don't know what '#quest.error.pronoun#' you are referring to.");
	else if (errorname == "badexamine")
		print_eval("You didn't say what you wanted to examine.");
	else
		gi->debug_print("Bad error name " + errorname);
}

String geas_implementation::displayed_name(String obj) const {
	String rv = obj, tmp;

	if (get_obj_property(obj, "alias", tmp))
		rv = tmp;
	else {
		for (uint i = 0; i < gf.blocks.size(); i ++)
			if (ci_equal(gf.blocks[i].name, obj)) {
				rv = gf.blocks[i].name;
				break;
			}
	}
	return rv;
}

/* For each destination, give:
 * - printed name
 * - accepted name, with prefix
 * - accepted name, without prefix
 * - destination, internal format
 * - script (optional)
 */
Common::Array<Common::Array<String> > geas_implementation::get_places(String room) {
	Common::Array<Common::Array<String> > rv;

	const GeasBlock *gb = gf.find_by_name("room", room);
	if (gb == NULL)
		return rv;

	String line, tok;
	uint c1, c2;
	for (uint i = 0; i < gb->data.size(); i ++) {
		line = gb->data[i];
		tok = first_token(line, c1, c2);
		if (tok == "place") {
			tok = next_token(line, c1, c2);
			if (!is_param(tok)) {
				gi->debug_print("Expected parameter after 'place' in " + line);
				continue;
			}
			String dest_param = eval_param(tok);
			if (dest_param == "") {
				gi->debug_print("Parameter empty in " + line);
				continue;
			}

			int j = dest_param.find(';');
			String dest, prefix = "";
			if (j == -1)
				dest = trim(dest_param);
			else {
				dest = trim(dest_param.substr(j + 1));
				prefix = trim(dest_param.substr(0, j));
			}
			String displayed = displayed_name(dest);
			String printed_dest = (prefix != "" ? prefix + " " : "") +
			                      "|b" + displayed + "|xb";

			Common::Array<String> tmp;
			tmp.push_back(printed_dest);
			tmp.push_back(prefix + " " + displayed);
			tmp.push_back(displayed);
			tmp.push_back(dest);
			String rest = trim(line.substr(c2));
			if (rest != "")
				tmp.push_back(rest);
			rv.push_back(tmp);
		}
	}

	for (uint i = 0; i < state.exits.size(); i ++) {
		if (state.exits[i].src != room)
			continue;
		line = state.exits[i].dest;
		tok = first_token(line, c1, c2);
		if (tok == "exit") {
			tok = next_token(line, c1, c2);
			if (!is_param(tok))
				continue;
			tok = next_token(line, c1, c2);
			assert(is_param(tok));
			tok = param_contents(tok);
			Common::Array<String> args = split_param(tok);
			if (args.size() != 2) {
				gi->debug_print("Expected two arguments in " + tok);
				continue;
			}
			assert(args[0] == room);
			Common::Array<String> tmp;
			String displayed = displayed_name(args[1]);
			tmp.push_back("|b" + displayed + "|xb");
			tmp.push_back(displayed);
			tmp.push_back(displayed);
			tmp.push_back(args[1]);
			rv.push_back(tmp);
		} else if (tok == "destroy") {
			tok = next_token(line, c1, c2);
			assert(tok == "exit");
			tok = next_token(line, c1, c2);

			for (v2string::iterator j = rv.begin(); j != rv.end(); j ++)
				if ((*j)[3] == tok) {
					rv.erase(j);
					break;
				}
		}


	}

	cerr << "get_places (" << room << ") -> " << rv << "\n";
	return rv;
}

String geas_implementation::exit_dest(String room, String dir, bool *is_script) const {
	uint c1, c2;
	String tok;
	if (is_script != NULL)
		*is_script = false;
	for (uint i = state.exits.size() - 1; i + 1 > 0; i --)
		if (state.exits[i].src == room) {
			String line = state.exits[i].dest;
			cerr << "Processing exit line '" << state.exits[i].dest << "'\n";
			tok = first_token(line, c1, c2);
			cerr << "   first tok is " << tok << " (vs. exit)\n";
			// SENSITIVE?
			if (tok != "exit")
				continue;
			tok = next_token(line, c1, c2);
			cerr << "   second tok is " << tok << " (vs. " << dir << ")\n";
			if (tok != dir)
				continue;
			tok = next_token(line, c1, c2);
			cerr << "   third tok is " << tok << " (expecting parameter)\n";
			assert(is_param(tok));
			Common::Array<String> p = split_param(param_contents(tok));
			assert(p.size() == 2);
			assert(ci_equal(p[0], room));
			return p[1];
		}
	/*
	if (gf.get_obj_action (room, dir, tok))
	  {
	    if (is_script != NULL)
	*is_script = true;
	    return tok;
	  }
	if (gf.get_obj_property (room, dir, tok))
	  return tok;
	else
	  return "";
	*/

	const GeasBlock *gb = gf.find_by_name("room", room);
	if (gb == NULL) {
		gi->debug_print(String("Trying to find exit <") + dir +
		                "> of nonexistent room <" + room + ">.");
		return "";
	}
	// TODO: what's the priority on this?
	for (uint i = 0; i < gb->data.size(); i ++) {
		String line = gb->data[i];
		tok = first_token(line, c1, c2);
		if (tok == dir) {
			uint line_start = c2;
			tok = next_token(line, c1, c2);
			if (is_param(tok))
				return param_contents(tok);
			if (tok != "") {
				if (is_script != NULL)
					*is_script = true;
				return trim(line.substr(line_start + 1));
			}
			return "";
		}
	}
	return "";
}

void geas_implementation::look() {
	String tmp;
	if (get_obj_action(state.location, "description", tmp))
		run_script_as(state.location, tmp);
	//run_script(tmp);
	else if (get_obj_property(state.location, "description", tmp))
		print_formatted(tmp);
	else if (get_obj_action("game", "description", tmp))
		run_script_as("game", tmp);
	//run_script (tmp);
	else if (get_obj_property("game", "description", tmp))
		print_formatted(tmp);
	else {
		String in_desc;
		if (get_obj_property(state.location, "indescription", tmp))
			in_desc = tmp;
		else
			in_desc = "You are in";
		print_formatted(in_desc + " " + get_svar("quest.formatroom"));

		if ((tmp = get_svar("quest.formatobjects")) != "")
			//print_formatted ("There is " + tmp + " here.");
			print_eval("There is #quest.formatobjects# here.");
		if ((tmp = get_svar("quest.doorways.out")) != "")
			print_formatted("You can go out to " + tmp + ".");
		if ((tmp = get_svar("quest.doorways.dirs")) != "")
			//print_formatted ("You can go " + tmp + ".");
			print_eval("You can go #quest.doorways.dirs#.");
		if ((tmp = get_svar("quest.doorways.places")) != "")
			print_formatted("You can go to " + tmp + ".");
		if ((tmp = get_svar("quest.lookdesc")) != "")
			print_formatted(tmp);
	}
}

void geas_implementation::set_game(const String &fname) {
	cerr << "set_game (...)\n";

	gf = read_geas_file(gi, fname);
	if (gf.blocks.size() == 0) {
		is_running_ = false;
		return;
	}
	//print_formatted ("Ready...|n|cbblack|crred|clblue|cggreen|cyyellow|n|uunderlined: |cbblack|crred|clblue|cggreen|cyyellow|xu|n");
	//cerr << "Read game " << gf << endl;
	uint tok_start, tok_end;
	outputting = true;

	state = GeasState(*gi, gf);

	state.running = true;

	for (uint gline = 0; gline < gf.block("game", 0).data.size(); gline ++) {
		String s = gf.block("game", 0).data[gline];
		String tok = first_token(s, tok_start, tok_end);
		// SENSITIVE?
		if (tok == "asl-version") {
			String ver = next_token(s, tok_start, tok_end);
			if (!is_param(ver)) {
				gi->debug_print("Version " + s + " has invalid version " +
					            ver);
				continue;
			}
			int vernum = parse_int(param_contents(ver));
			if (vernum < 311 || vernum > 353)
				gi->debug_print("Warning: Geas only supports ASL "
					            " versions 3.11 to 3.53");
		}
		// SENSITIVE?
		else if (tok == "background") {
			tok = next_token(s, tok_start, tok_end);
			if (!is_param(tok))
				gi->debug_print(nonparam("background color", s));
			else
				gi->set_background(param_contents(tok));
		}
		// SENSITIVE?
		else if (tok == "default") {
			tok = next_token(s, tok_start, tok_end);
			// SENSITIVE?
			if (tok == "fontname") {
				tok = next_token(s, tok_start, tok_end);
				if (!is_param(tok))
					gi->debug_print(nonparam("font name", s));
				else
					gi->set_default_font(param_contents(tok));
			}
			// SENSITIVE?
			else if (tok == "fontsize") {
				tok = next_token(s, tok_start, tok_end);
				if (!is_param(tok))
					gi->debug_print(nonparam("font size", s));
				else
					gi->set_default_font_size(param_contents(tok));
			}
		}
		// SENSITIVE?
		else if (tok == "foreground") {
			tok = next_token(s, tok_start, tok_end);
			if (!is_param(tok))
				gi->debug_print(nonparam("foreground color", s));
			else
				gi->set_foreground(param_contents(tok));
		}
		// SENSITIVE?
		else if (tok == "gametype") {
			tok = next_token(s, tok_start, tok_end);
			// SENSITIVE?
			if (tok == "singleplayer")
				continue;
			// SENSITIVE?
			if (tok == "multiplayer")
				error("Error: geas is single player only.");
			gi->debug_print("Unexpected game type " + s);
		}
		// SENSITIVE?
		else if (tok == "nodebug") {
		}
		// SENSITIVE?
		else if (tok == "start") {
			tok = next_token(s, tok_start, tok_end);
			if (!is_param(tok))
				gi->debug_print(nonparam("start room", s));
			else {
				state.location = param_contents(tok);
			}
		}
	}

	const GeasBlock &game = gf.block("game", 0);
	cerr << gf << endl;
	//print_formatted ("Done loading " + game.name);
	uint c1, c2;
	String tok;

	/* TODO do I run the startscript or print the opening text first? */
	run_script("displaytext <intro>");

	for (uint i = 0; i < game.data.size(); i ++)
		// SENSITIVE?
		if (first_token(game.data[i], c1, c2) == "startscript") {
			run_script_as("game", game.data[i].substr(c2 + 1));
			//run_script (game.data[i].substr (c2 + 1));
			break;
		}

	regen_var_room();
	regen_var_objects();
	regen_var_dirs();
	regen_var_look();
	look();

	cerr << "s_g: done with set_game (...)\n\n";
}

void geas_implementation::regen_var_objects() {
	String tmp;
	Common::Array <String> objs;
	for (uint i = 0; i < state.objs.size(); i ++) {
		//cerr << "r_v_o: Checking '" << state.objs[i].name << "' (" << state.objs[i].parent << "): " << ((state.objs[i].parent == state.location) ? "YES" : "NO") << endl;
		if (ci_equal(state.objs[i].parent, state.location) &&
		        !get_obj_property(state.objs[i].name, "hidden", tmp) &&
		        !get_obj_property(state.objs[i].name, "invisible", tmp))
			//!state.objs[i].hidden &&
			//!state.objs[i].invisible)
			objs.push_back(state.objs[i].name);
	}
	String qobjs = "", qfobjs = "";
	String objname, prefix, main, suffix, propval, print1, print2;
	for (uint i = 0; i < objs.size(); i ++) {
		objname = objs[i];
		if (!get_obj_property(objname, "alias", main))
			main = objname;
		print1 = main;
		print2 = "|b" + main + "|xb";
		if (get_obj_property(objname, "prefix", prefix)) {
			print1 = prefix + " " + print1;
			print2 = prefix + " " + print2;
		}
		if (get_obj_property(objname, "suffix", suffix)) {
			print1 = print1 + " " + suffix;
			print2 = print2 + " " + suffix;
		}
		qobjs = qobjs + print1;
		qfobjs = qfobjs + print2;
		if (i + 2 < objs.size()) {
			qobjs = qobjs + ", ";
			qfobjs = qfobjs + ", ";
		} else if (i + 2 == objs.size()) {
			qobjs = qobjs + " and ";
			qfobjs = qfobjs + " and ";
		}
	}
	set_svar("quest.objects", qobjs);
	set_svar("quest.formatobjects", qfobjs);
}

void geas_implementation::regen_var_room() {
	set_svar("quest.currentroom", state.location);

	String tmp, formatroom;
	if (!get_obj_property(state.location, "alias", formatroom))
		formatroom = state.location;
	formatroom = "|cr" + formatroom + "|cb";
	if (get_obj_property(state.location, "prefix", tmp))
		formatroom = tmp + " " + formatroom;
	if (get_obj_property(state.location, "suffix", tmp))
		formatroom = formatroom + " " + tmp;
	//set_svar ("quest.formatroom", displayed_name (state.location));
	set_svar("quest.formatroom", formatroom);

	// regen_var_objects();
	/*
	String out_dest = exit_dest (state.location, "out");
	if (out_dest == "")
	  {
	    set_svar ("quest.doorways.out", "");
	    set_svar ("quest.doorways.out.display", "");
	  }
	else
	  {
	    cerr << "Updating quest.doorways.out; out_dest == {" << out_dest << "}";
	    uint i = out_dest.find (';');
	    cerr << ", i == " << i;
	    String prefix = "";
	    if (i != -1)
	{
	  prefix = trim (out_dest.substr (0, i-1));
	  out_dest = trim (out_dest.substr (i + 1));
	  cerr << "; prefix == {" << prefix << "}, out_dest == {" << out_dest << "}";
	}
	    cerr << "  quest.doorways.out == {" << out_dest << "}";
	    set_svar ("quest.doorways.out", out_dest);
	    cerr << endl;

	    String tmp = displayed_name (out_dest);

	    cerr << ", tmp == {" << tmp << "}";

	    if (tmp != "")
	tmp = "|b" + tmp + "|xb";
	    else if (prefix != "")
	tmp = prefix + " |b" + out_dest + "|xb";
	    else
	tmp = "|b" + out_dest + "|xb";

	    cerr << ",    final value {" << tmp << "}" << endl;

	    set_svar ("quest.doorways.out.display", tmp);
	  }
	*/
}


void geas_implementation::regen_var_look() {
	String look_tag;
	if (!get_obj_property(state.location, "look", look_tag))
		look_tag = "";
	set_svar("quest.lookdesc", look_tag);
}


void geas_implementation::regen_var_dirs() {
	Common::Array <String> dirs;
	// the -1 is so that it skips 'out'
	for (uint i = 0; i < ARRAYSIZE(dir_names) - 1; i ++)
		if (exit_dest(state.location, dir_names[i]) != "")
			dirs.push_back(dir_names[i]);
	String exits = "";
	if (dirs.size() == 1)
		exits = "|b" + dirs[0] + "|xb";
	else if (dirs.size() > 1) {
		for (uint i = 0; i < dirs.size(); i ++) {
			exits = exits + "|b" + dirs[i] + "|xb";
			if (i < dirs.size() - 2)
				exits = exits + ", ";
			else if (i == dirs.size() - 2)
				exits = exits + " or ";
		}
	}
	set_svar("quest.doorways.dirs", exits);

	/*
	String tmp;
	if ((tmp = exit_dest (state.location, "out")) != "")
	  set_svar ("quest.doorways.out", displayed_name (tmp));
	else
	  set_svar ("quest.doorways.out", "");
	*/

	String out_dest = exit_dest(state.location, "out");
	if (out_dest == "") {
		set_svar("quest.doorways.out", "");
		set_svar("quest.doorways.out.display", "");
	} else {
		cerr << "Updating quest.doorways.out; out_dest == {" << out_dest << "}";
		int i = out_dest.find(';');
		cerr << ", i == " << i;
		String prefix = "";
		if (i != -1) {
			prefix = trim(out_dest.substr(0, i - 1));
			out_dest = trim(out_dest.substr(i + 1));
			cerr << "; prefix == {" << prefix << "}, out_dest == {" << out_dest << "}";
		}
		cerr << "  quest.doorways.out == {" << out_dest << "}";
		set_svar("quest.doorways.out", out_dest);
		cerr << endl;

		String tmp = displayed_name(out_dest);

		cerr << ", tmp == {" << tmp << "}";

		if (tmp != "")
			tmp = "|b" + tmp + "|xb";
		else if (prefix != "")
			tmp = prefix + " |b" + out_dest + "|xb";
		else
			tmp = "|b" + out_dest + "|xb";

		cerr << ",    final value {" << tmp << "}" << endl;

		set_svar("quest.doorways.out.display", tmp);
	}

	/* TODO handle this */
	//set_svar ("quest.doorways.places", "");
	current_places = get_places(state.location);
	String printed_places = "";
	for (uint i = 0; i < current_places.size(); i ++) {
		if (i == 0)
			printed_places = current_places[i][0];
		else if (i < current_places.size() - 1)
			printed_places = printed_places + ", " + current_places[i][0];
		else if (current_places.size() == 2)
			printed_places = printed_places + " or " + current_places[i][0];
		else
			printed_places = printed_places + ", or " + current_places[i][0];
	}
	set_svar("quest.doorways.places", printed_places);
}



// TODO:  SENSITIVE???
String geas_implementation::substitute_synonyms(String s) const {
	String orig = s;
	cerr << "substitute_synonyms (" << s << ")\n";
	const GeasBlock *gb = gf.find_by_name("synonyms", "");
	if (gb != NULL) {
		/* TODO: exactly in what order does it try synonyms?
		 * Does it have to be flanked by whitespace?
		 */
		for (uint i = 0; i < gb->data.size(); i ++) {
			String line = gb->data[i];
			int index = line.find('=');
			if (index == -1)
				continue;
			Common::Array<String> words = split_param(line.substr(0, index));
			String rhs = trim(line.substr(index + 1));
			if (rhs == "")
				continue;
			for (uint j = 0; j < words.size(); j ++) {
				String lhs = words[j];
				if (lhs == "")
					continue;
				int k = 0;
				while ((k = s.find(lhs, k)) != -1) {
					uint end_index = k + lhs.length();
					if ((k == 0 || s[k - 1] == ' ') &&
					        (end_index == s.length() || s[end_index] == ' ')) {
						s = s.substr(0, k) + rhs + s.substr(k + lhs.length());
						k = k + rhs.length();
					} else
						k ++;
				}
			}
		}
	}
	cerr << "substitute_synonyms (" << orig << ") -> '" << s << "'\n";
	return s;
}

bool geas_implementation::is_running() const {
	return is_running_;
}

String geas_implementation::get_banner() {
	String banner;
	const GeasBlock *gb = gf.find_by_name("game", "game");
	if (gb) {
		String line = gb->data[0];
		uint c1, c2;
		String tok = first_token(line, c1, c2);
		tok = next_token(line, c1, c2);
		tok = next_token(line, c1, c2);
		if (is_param(tok)) {
			banner = eval_param(tok);

			for (uint i = 0; i < gb->data.size(); i ++) {
				line = gb->data[i];
				if (first_token(line, c1, c2) == "game" &&
				        next_token(line, c1, c2) == "version" &&
				        is_param(tok = next_token(line, c1, c2))) {
					banner += ", v";
					banner += eval_param(tok);
				}
			}

			for (uint i = 0; i < gb->data.size(); i ++) {
				line = gb->data[i];
				if (first_token(line, c1, c2) == "game" &&
				        next_token(line, c1, c2) == "author" &&
				        is_param(tok = next_token(line, c1, c2))) {
					banner += " | ";
					banner += eval_param(tok);
				}
			}
		}
	}
	return banner;
}

void geas_implementation::run_command(String s) {
	/* if s == "restore" or "restart" or "quit" or "undo" */

	if (!is_running_)
		return;

	print_newline();
	print_normal("> " + s);
	print_newline();

	if (s == "dump status") {
		//cerr << state << endl;
		ostringstream oss;
		oss << state;
		print_normal(oss.str());
		return;

	} else if (s == "undo") {
		if (undo_buffer.size() < 2) {
			print_formatted("(No more undo information available!)");
			return;
		}
		undo_buffer.pop();
		state = undo_buffer.peek();
		print_formatted("Undone.");
		return;

	} else if (s == "save") {
		if (g_vm->saveGame().getCode() == Common::kNoError)
			print_formatted("Saved.");
		return;

	} else if (s == "restore") {
		if (g_vm->loadGame().getCode() == Common::kNoError)
			run_command("look");
		return;
	}

	if (!state.running)
		return;
	// TODO: does this get the original command, or the lowercased version?
	set_svar("quest.originalcommand", s);
	s = substitute_synonyms(lcase(s));
	set_svar("quest.command", s);

	bool overridden = false;
	dont_process = false;

	const GeasBlock *gb = gf.find_by_name("room", state.location);
	if (gb != NULL) {
		String line, tok;
		uint c1, c2;
		for (uint i = 0; i < gb->data.size(); i ++) {
			line = gb->data[i];
			tok = first_token(line, c1, c2);
			// SENSITIVE?
			if (tok == "beforeturn") {
				uint scr_starts = c2;
				tok = next_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "override") {
					overridden = true;
					scr_starts = c2;
				}
				String scr = line.substr(scr_starts);
				run_script(state.location, scr);
				//run_script (scr);
			}
		}
	} else
		gi->debug_print("Unable to find block " + state.location + ".\n");

	if (!overridden) {
		gb = gf.find_by_name("game", "game");
		if (gb != NULL) {
			String line, tok;
			uint c1, c2;
			for (uint i = 0; i < gb->data.size(); i ++) {
				line = gb->data[i];
				tok = first_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "beforeturn") {
					uint scr_starts = c2;
					tok = next_token(line, c1, c2);
					// SENSITIVE?
					if (tok == "override") {
						overridden = true;
						scr_starts = c2;
					}
					String scr = line.substr(scr_starts);
					run_script_as("game", scr);
					//run_script (scr);
				}
			}
		} else
			gi->debug_print("Unable to find block game.\n");
	}

	if (!dont_process) {
		if (try_match(s, false, false)) {
			/* TODO TODO */
			// run after turn events ???
		} else
			display_error("badcommand");
	}

	overridden = false;

	gb = gf.find_by_name("room", state.location);
	if (gb != NULL) {
		String line, tok;
		uint c1, c2;
		for (uint i = 0; i < gb->data.size(); i ++) {
			line = gb->data[i];
			tok = first_token(line, c1, c2);
			// SENSITIVE?
			if (tok == "afterturn") {
				uint scr_starts = c2;
				tok = next_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "override") {
					overridden = true;
					scr_starts = c2;
				}
				String scr = line.substr(scr_starts);
				run_script_as(state.location, scr);
				//run_script (scr);
			}
		}
	}
	if (!overridden) {
		gb = gf.find_by_name("game", "game");
		if (gb != NULL) {
			String line, tok;
			uint c1, c2;
			for (uint i = 0; i < gb->data.size(); i ++) {
				line = gb->data[i];
				tok = first_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "afterturn") {
					uint scr_starts = c2;
					tok = next_token(line, c1, c2);
					// SENSITIVE?
					if (tok == "override") {
						overridden = true;
						scr_starts = c2;
					}
					String scr = line.substr(scr_starts);
					run_script_as("game", scr);
					//run_script (scr);
				}
			}
		}
	}

	if (state.running)
		undo_buffer.push(state);
}

Common::WriteStream &operator<< (Common::WriteStream &o, const match_rv &rv) {
	//o << "match_rv {" << (rv.success ? "TRUE" : "FALSE") << ": " << rv.bindings << "}";
	o << "match_rv {" << (rv.success ? "TRUE" : "FALSE") << ": [";
	//o << rv.bindings.size();
	o << rv.bindings;
	//for (uint i = 0; i < rv.bindings.size(); i ++)
	//  o << rv.bindings[i] << ", ";
	o << "]}";
	return o;
}

match_rv geas_implementation::match_command(String input, String action) const {
	//cerr << "match_command (\"" << input << "\", \"" << action << "\")" << endl;
	match_rv rv = match_command(input, 0, action, 0, match_rv());
	cerr << "match_command (\"" << input << "\", \"" << action << "\") -> " << rv << endl;
	return rv;
	//return match_command (input, 0, action, 0, match_rv ());
}

match_rv geas_implementation::match_command(String input, uint ichar, String action, uint achar, match_rv rv) const {
	//cerr << "match_command (\"" << input << "\", " << ichar << ", \"" << action << "\", " << achar << ", " << rv << ")" << endl;
	for (;;) {
		if (achar == action.length()) {
			//cerr << "End of action, returning " << (ichar == input.length()) << "\n";
			return match_rv(ichar == input.length(), rv);
		}
		if (action[achar] == '#') {

			achar ++;
			String varname;
			while (achar != action.length() && action[achar] != '#') {
				varname += action[achar];
				achar ++;
			}
			if (achar == action.length())
				error("Unpaired hashes in command String %s", action.c_str());
			//rv.bindings.push_back (varname);
			int index = rv.bindings.size();
			rv.bindings.push_back(match_binding(varname, ichar));
			achar ++;
			varname = "";
			//rv.bindings.push_back (varname);
			rv.bindings[index].set(varname, ichar);
			while (ichar < input.length()) {
				match_rv tmp = match_command(input, ichar, action, achar, rv);
				if (tmp.success)
					return tmp;
				varname += input[ichar];
				ichar ++;
				//rv.bindings[index] = varname;
				rv.bindings[index].set(varname, ichar);
			}
			return match_rv(achar == action.length(), rv);
		}
		// SENSITIVE?
		if (ichar == input.length() || !c_equal_i(input[ichar], action[achar]))
			return match_rv();
		//cerr << "Matched " << input[ichar] << " to " << action[achar] << endl;
		++ achar;
		++ ichar;
	}
}

bool match_object_alts(String text, const Common::Array<String> &alts, bool is_internal) {
	for (uint i = 0; i < alts.size(); i ++) {
		cerr << "m_o_a: Checking '" << text << "' v. alt '" << alts[i] << "'.\n";
		if (starts_with(text, alts[i])) {
			uint len = alts[i].length();
			if (text.length() == len)
				return true;
			if (text.length() > len  &&  text[len] == ' '  &&
			        match_object_alts(text.substr(len + 1), alts, is_internal))
				return true;
		}
	}
	return false;
}


bool geas_implementation::match_object(String text, String name, bool is_internal) const {
	cerr << "* * * match_object (" << text << ", " << name << ", "
	     << (is_internal ? "true" : "false") << ")\n";

	String alias, alt_list, prefix, suffix;

	if (is_internal && ci_equal(text, name)) return true;

	if (get_obj_property(name, "prefix", prefix) &&
	        starts_with(text, prefix + " ") &&
	        match_object(text.substr(prefix.length() + 1), name, false))
		return true;

	if (get_obj_property(name, "suffix", suffix) &&
	        ends_with(text, " " + suffix) &&
	        match_object(text.substr(0, text.length() - suffix.length() - 1), name, false))
		return true;

	if (!get_obj_property(name, "alias", alias))
		alias = name;
	if (ci_equal(text, alias))
		return true;

	const GeasBlock *gb = gf.find_by_name("object", name);
	if (gb != NULL) {
		String tok, line;
		uint c1, c2;
		for (uint ln = 0; ln < gb->data.size(); ln ++) {
			line = gb->data[ln];
			tok = first_token(line, c1, c2);
			// SENSITIVE?
			if (tok == "alt") {
				tok = next_token(line, c1, c2);
				if (!is_param(tok))
					gi->debug_print("Expected param after alt in " + line);
				else {
					Common::Array<String> alts = split_param(param_contents(tok));
					cerr << "  m_o: alt == " << alts << "\n";
					return match_object_alts(text, alts, is_internal);
				}
			}
		}
	}

	return false;
}


bool geas_implementation::dereference_vars(Common::Array<match_binding> &bindings, bool is_internal) const {
	/* TODO */
	Common::Array<String> where;
	where.push_back("inventory");
	where.push_back(state.location);
	return dereference_vars(bindings, where, is_internal);
}

bool geas_implementation::dereference_vars(Common::Array<match_binding> &bindings, const Common::Array<String> &where, bool is_internal) const {
	bool rv = true;
	for (uint i = 0; i < bindings.size(); i ++)
		if (bindings[i].var_name[0] == '@') {
			String obj_name = get_obj_name(bindings[i].var_text, where, is_internal);
			if (obj_name == "!") {
				print_formatted("You don't see any " + bindings[i].var_text + ".");
				rv = false;
			} else {
				bindings[i].var_text = obj_name;
				bindings[i].var_name = bindings[i].var_name.substr(1);
			}
		}
	return rv;
}

String geas_implementation::get_obj_name(String name, const Common::Array<String> &where, bool is_internal) const {
	Common::Array<String> objs, printed_objs;
	for (uint objnum = 0; objnum < state.objs.size(); objnum ++) {
		bool is_used = false;
		for (uint j = 0; j < where.size(); j ++) {
			cerr << "Object #" << objnum << ": " << state.objs[objnum].name
			     << "@" << state.objs[objnum].parent << " vs. "
			     << where[j] << endl;
			// SENSITIVE?
			if (where[j] == "game" || state.objs[objnum].parent == where[j])
				is_used = true;
		}
		if (is_used && !has_obj_property(state.objs[objnum].name, "hidden") &&
		        match_object(name, state.objs[objnum].name, is_internal)) {
			String printed_name, tmp, oname = state.objs[objnum].name;
			objs.push_back(oname);
			if (!get_obj_property(oname, "alias", printed_name))
				printed_name = oname;
			if (get_obj_property(oname, "detail", tmp))
				printed_name = tmp;
			printed_objs.push_back(printed_name);
		}
	}
	cerr << "objs == " << objs << ", printed_objs == " << printed_objs << "\n";
	if (objs.size() > 1) {
		//bindings[i].var_name = bindings[i].var_name.substr(1);
		uint num = 0;
		//if (objs.size() > 1)
		num = gi->make_choice("Which " + name + " do you mean?", printed_objs);

		//bindings[i].var_text = objs[num];
		return objs[num];
	}
	if (objs.size() == 1)
		return objs[0];
	return "!";
}


void geas_implementation::set_vars(const Common::Array<match_binding> &v) {
	for (uint i = 0; i < v.size(); i ++)
		set_svar(v[i].var_name, v[i].var_text);
}


bool geas_implementation::run_commands(String cmd, const GeasBlock *room, bool is_internal) {
	uint c1, c2;
	String line, tok;
	match_rv match;

	if (room != NULL) {
		for (uint i = 0; i < room->data.size(); i++) {
			line = room->data[i];
			tok = first_token(line, c1, c2);
			// SENSITIVE?
			if (tok == "command") {
				tok = next_token(line, c1, c2);
				if (is_param(tok)) {
					Common::Array<String> tmp = split_param(param_contents(tok));

					for (uint j = 0; j < tmp.size(); j++)
						if ((match = match_command(cmd, tmp[j]))) {
							if (!dereference_vars(match.bindings, is_internal))
								return false;
							set_vars(match.bindings);
							run_script_as(state.location, line.substr(c2 + 1));
							//run_script (line.substr (c2+1));
							return true;
						}
					/*
					  if (match = match_command (cmd, param_contents(tok)))
					  {
					  if (!dereference_vars (match.bindings))
					  return false;
					  set_vars (match.bindings);
					  run_script (line.substr (c2+1));
					  return true;
					  }
					*/
				} else {
					gi->debug_print("Bad command line: " + line);
				}
			}
		}
	} else
		gi->debug_print("room is null\n");

	return false;
}

bool geas_implementation::try_match(String cmd, bool is_internal, bool is_normal) {
	//print_formatted ("geas_impl registers " + cmd);

	String line, tok;
	match_rv match;

	if (!is_normal) {
		if (run_commands(cmd, gf.find_by_name("room", state.location)) ||
		        run_commands(cmd, gf.find_by_name("game", "game")))
			return true;
	}

	if ((match = match_command(cmd, "look at #@object#")) ||
	        (match = match_command(cmd, "look #@object#"))) {
		if (!dereference_vars(match.bindings, is_internal))
			return true;

		String object = match.bindings[0].var_text;

		if (get_obj_action(object, "look", tok))
			run_script_as(object, tok);
		//run_script (tok);
		else if (get_obj_property(object, "look", tok))
			print_formatted(tok);
		else
			display_error("defaultlook", object);

		return true;
	}

	if ((match = match_command(cmd, "examine #@object#")) ||
	        (match = match_command(cmd, "x #@object#"))) {
		if (!dereference_vars(match.bindings, is_internal))
			return true;

		String object = match.bindings[0].var_text;
		if (get_obj_action(object, "examine", tok))
			run_script_as(object, tok);
		//run_script (tok);
		else if (get_obj_property(object, "examine", tok))
			print_formatted(tok);
		else if (get_obj_action(object, "look", tok))
			run_script_as(object, tok);
		//run_script (tok);
		else if (get_obj_property(object, "look", tok))
			print_formatted(tok);
		else
			display_error("defaultexamine", object);
		return true;
	}

	if ((match = match_command(cmd, "look"))) {
		look();
		return true;
	}

	if ((match = match_command(cmd, "give #@first# to #@second#"))) {
		if (!dereference_vars(match.bindings, is_internal))
			return true;
		String script, first = match.bindings[0].var_text, second = match.bindings[1].var_text;
		if (! ci_equal(get_obj_parent(first), "inventory"))
			display_error("noitem", first);
		else if (get_obj_action(second, "give " + first, script))
			run_script(second, script);
		//run_script (script);
		else if (get_obj_action(first, "give to " + second, script))
			run_script_as(first, script);
		//run_script (script);
		else if (get_obj_action(second, "give anything", script)) {
			set_svar("quest.give.object.name", first);
			run_script_as(second, script);
			//run_script (script);
		} else if (get_obj_action(first, "give to anything", script)) {
			set_svar("quest.give.object.name", second);
			run_script_as(first, script);
			//run_script (script);
		} else {
			String tmp;
			if (!get_obj_property(second, "gender", tmp))
				tmp = "it";
			set_svar("quest.error.gender", tmp);
			if (!get_obj_property(first, "article", tmp))
				tmp = "it";
			set_svar("quest.error.article", tmp);
			display_error("itemunwanted");
		}
		return true;
	}

	if ((match = match_command(cmd, "use #@first# on #@second#")) ||
	        (match = match_command(cmd, "use #@first# with #@second#"))) {
		if (!dereference_vars(match.bindings, is_internal))
			return true;
		String script, first = match.bindings[0].var_text, second = match.bindings[1].var_text;
		if (! ci_equal(get_obj_parent(first), "inventory"))
			display_error("noitem", first);
		else if (get_obj_action(second, "use " + first, script)) {
			//set_svar ("quest.use.object", first);
			run_script_as(second, script);
			//run_script (script);
		} else if (get_obj_action(first, "use on " + second, script)) {
			//set_svar ("quest.use.object", second);
			run_script_as(first, script);
			//run_script (script);
		} else if (get_obj_action(second, "use anything", script)) {
			set_svar("quest.use.object", first);
			run_script(second, script);
			//run_script (script);
		} else if (get_obj_action(first, "use on anything", script)) {
			set_svar("quest.use.object", second);
			run_script_as(first, script);
			//run_script (script);
		} else
			display_error("defaultuse");

		return true;
	}

	if ((match = match_command(cmd, "use #@first#"))) {
		if (!dereference_vars(match.bindings, is_internal))
			return true;
		String tmp, obj = match.bindings[0].var_text;
		if (!ci_equal(get_obj_parent(obj), "inventory"))
			display_error("noitem", obj);
		else if (get_obj_action(obj, "use", tmp))
			run_script_as(obj, tmp);
		//run_script (tmp);
		else if (get_obj_property(obj, "use", tmp))
			print_formatted(tmp);
		else
			display_error("defaultuse", obj);
		return true;
	}


	if ((match = match_command(cmd, "take #@object#")) ||
	        (match = match_command(cmd, "get #@object#"))) {
		if (!dereference_vars(match.bindings, is_internal))
			return true;

		String object = match.bindings[0].var_text;
		if (get_obj_action(object, "take", tok)) {
			cerr << "Running script '" << tok << "' for take " << object << endl;
			run_script_as(object, tok);
			//run_script (tok);
		} else if (get_obj_property(object, "take", tok)) {
			cerr << "Found property '" << tok << "' for take " << object << endl;
			if (tok != "")
				print_formatted(tok);
			else
				display_error("defaulttake", object);
			String tmp;
			move(object, "inventory");
			if (get_obj_action(object, "gain", tmp))
				run_script(object, tmp);
			//run_script (tmp);
			else if (get_obj_property(object, "gain", tmp))
				print_formatted(tmp);
		} else {
			cerr << "No match found for take " << object << endl;
			// TODO set variable with object name
			display_error("badtake", object);
		}
		return true;
	}


	if ((match = match_command(cmd, "drop #@object#"))) {
		if (!dereference_vars(match.bindings, is_internal))
			return true;
		String scr, obj = match.bindings[0].var_text;
		if (get_obj_action(obj, "drop", scr)) {
			run_script_as(obj, scr);
			//run_script (scr);
			return true;
		}

		const GeasBlock *gb = gf.find_by_name("object", obj);
		if (gb != NULL) {
			uint c1, c2, script_begins;
			for (uint i = 0; i < gb->data.size(); i ++) {
				line = gb->data[i];
				tok = first_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "drop") {
					script_begins = c2;
					tok = next_token(line, c1, c2);
					// SENSITIVE?
					if (tok == "everywhere") {
						tok = next_token(line, c1, c2);
						move(obj, state.location);
						if (is_param(tok))
							print_eval(tok);
						else
							gi->debug_print("Expected param after drop everywhere in " + line);
						return true;
					}
					// SENSITIVE?
					if (tok == "nowhere") {
						if (is_param(tok))
							print_eval(tok);
						else
							gi->debug_print("Expected param after drop nowhere in " + line);
						return true;
					}
					run_script_as(obj, line.substr(script_begins));
					//run_script (line.substr (script_begins));
					return true;
				}
			}
		}
		move(obj, state.location);
		display_error("defaultdrop", obj);
		return true;
	}

	if ((match = match_command(cmd, "speak to #@object#")) ||
	        (match = match_command(cmd, "speak #@object#")) ||
	        (match = match_command(cmd, "talk to #@object#")) ||
	        (match = match_command(cmd, "talk #@object#"))) {
		//print_formatted ("Talk to <" + String (match.bindings[0]) + ">");
		if (!dereference_vars(match.bindings, is_internal))
			return true;
		String obj = match.bindings[0].var_text;
		String script;
		if (get_obj_action(obj, "speak", script))
			run_script_as(obj, script);
		//run_script (script);
		else
			display_error("defaultspeak", obj);
		//print_formatted ("Talk to <" + String (match.bindings[0]) + ">");
		return true;
	}

	if (cmd == "exit" || cmd == "out" || cmd == "go out") {
		const GeasBlock *gb = gf.find_by_name("room", state.location);
		if (gb == NULL) {
			gi->debug_print("Bad room");
			return true;
		}

		line = "";
		int c1 = -1, c2 = -1;
		uint uc1, uc2;
		// TODO: Use the first matching line or the last?
		for (uint i = 0; i < gb->data.size(); i ++) {
			if (first_token(gb->data[i], uc1, uc2) == "out")
				line = gb->data[i];
			c1 = uc1;
			c2 = uc2;
		}

		//gi->debug_print ("COMMAND " + cmd + ": line == " + line);

		if (line == "")
			display_error("defaultout");
		else {
			c1 = line.find('<');
			if (c1 != -1)
				c2 = line.find('>', c1);

			if (c1 == -1 || c2 == -1)
				gi->debug_print("Bad out line: " + line);
			else {
				String tmp = trim(line.substr(c2 + 1));
				//gi->debug_print ("tmp1 == {" + tmp + "}");
				if (tmp != "")
					run_script_as(state.location, tmp);
				//run_script (tmp);
				else {
					tmp = line.substr(c1, c2 - c1 + 1);
					//gi->debug_print ("tmp2 == {" + tmp + "}");
					assert(is_param(tmp));
					tmp = param_contents(tmp);
					c1 = tmp.find(';');
					if (c1 == -1)
						goto_room(trim(tmp));
					else
						goto_room(trim(tmp.substr(c1 + 1)));
				}
			}
		}
		return true;
	}

	for (uint i = 0; i < ARRAYSIZE(dir_names); i ++)
		if (cmd == dir_names[i] || cmd == (String("go ") + dir_names[i]) ||
		        cmd == short_dir_names[i] || cmd == (String("go ") + short_dir_names[i])) {
			bool is_script = false;
			//print_formatted ("Trying to go " + dir_names[i]);
			if ((tok = exit_dest(state.location, dir_names[i], &is_script)) == "") {
				// TODO Which display_error do I use?
				print_formatted("You can't go that way.");
				return true;
			}
			if (is_script)
				run_script_as(state.location, tok);
			//run_script (tok);
			else {
				int index = tok.find(';');
				if (index == -1)
					goto_room(trim(tok));
				else
					goto_room(trim(tok.substr(index + 1)));
			}
			return true;
		}

	if ((match = match_command(cmd, "go to #@room#")) ||
	        (match = match_command(cmd, "go #@room#"))) {
		assert(match.bindings.size() == 1);
		String destination = match.bindings[0].var_text;
		for (uint i = 0; i < current_places.size(); i ++) {
			if (ci_equal(destination, current_places[i][1]) ||
			        ci_equal(destination, current_places[i][2])) {
				if (current_places[i].size() == 5)
					run_script_as(state.location, current_places[i][4]);
				//run_script (current_places[i][4]);
				else
					goto_room(current_places[i][3]);
				return true;
			}
		}
		display_error("badplace", destination);
		return true;
	}

	if (ci_equal(cmd, "inventory") || ci_equal(cmd, "i")) {
		Common::Array<Common::Array<String> > inv = get_inventory();
		if (inv.size() == 0)
			print_formatted("You are carrying nothing.");
		else
			print_formatted("You are carrying:");
		for (uint i = 0; i < inv.size(); i ++) {
			print_normal(inv[i][0]);
			print_newline();
		}
		return true;
	}

	if (ci_equal(cmd, "help")) {
		print_formatted("|b|cl|s14Quest Quick Help|xb|cb|s00|n|n|cl|bMoving|xb|cb Type |bGO NORTH|xb, |bSOUTH|xb, |bE|xb, etc. |xnTo go into a place, type |bGO TO ...|xb . To leave a place, type |bOUT, EXIT|xb or |bLEAVE|xb, or press the '|crOUT|cb' button.|n|cl|bObjects and Characters|xb|cb Use |bTAKE ...|xb, |bGIVE ... TO ...|xb, |bTALK|xb/|bSPEAK TO ...|xb, |bUSE ... ON|xb/|bWITH ...|xb, |bLOOK AT ...|xb, etc.|n|cl|bExit Quest|xb|cb Type |bQUIT|xb to leave Quest.|n|cl|bMisc|xb|cb Type |bABOUT|xb to get information on the current game.");
		return true;
	}

	if (ci_equal(cmd, "about")) {
		const GeasBlock *gb = gf.find_by_name("game", "game");
		if (gb == NULL)
			return true;
		cerr << *gb << endl;

		uint c1, c2;
		//print_formatted ("Game name: ");
		line = gb->data[0];
		tok = first_token(line, c1, c2);  // game
		tok = next_token(line, c1, c2);  // name
		tok = next_token(line, c1, c2);  // <whatever>
		if (is_param(tok))
			print_formatted("Game name: " + eval_param(tok));


		for (uint i = 0; i < gb->data.size(); i ++) {
			line = gb->data[i];
			// SENSITIVE?
			if (first_token(line, c1, c2) == "game" &&
			        next_token(line, c1, c2) == "version" &&
			        is_param(tok = next_token(line, c1, c2)))
				print_formatted("Version " + eval_param(tok));
		}

		for (uint i = 0; i < gb->data.size(); i ++) {
			line = gb->data[i];
			// SENSITIVE?
			if (first_token(line, c1, c2) == "game" &&
			        next_token(line, c1, c2) == "author" &&
			        is_param(tok = next_token(line, c1, c2)))
				print_formatted("Author: " + eval_param(tok));
		}

		for (uint i = 0; i < gb->data.size(); i ++) {
			line = gb->data[i];
			// SENSITIVE?
			if (first_token(line, c1, c2) == "game" &&
			        next_token(line, c1, c2) == "copyright" &&
			        is_param(tok = next_token(line, c1, c2)))
				print_formatted("Copyright: " + eval_param(tok));
		}

		for (uint i = 0; i < gb->data.size(); i ++) {
			line = gb->data[i];
			// SENSITIVE?
			if (first_token(line, c1, c2) == "game" &&
			        next_token(line, c1, c2) == "info" &&
			        is_param(tok = next_token(line, c1, c2)))
				print_formatted(eval_param(tok));
		}

		return true;
	}

	if (ci_equal(cmd, "quit")) {
		is_running_ = false;
		return true;
	}

	return false;
}

void geas_implementation::run_script_as(String obj, String scr) {
	String backup_object, garbage;
	backup_object = this_object;
	this_object = obj;
	run_script(scr, garbage);
	this_object = backup_object;
}

void geas_implementation::run_script(String s) {
	String garbage;
	run_script(s, garbage);
}

void geas_implementation::run_script(String s, String &rv) {
	//print_formatted ("     Running script " + s + ".");
	cerr << "Script line '" << s << "'\n";
	String tok;
	uint c1, c2;

	tok = first_token(s, c1, c2);

	if (tok == "") return;

	if (tok[0] == '{') {
		uint brace1 = c1 + 1, brace2;
		for (brace2 = s.length() - 1; brace2 >= brace1 && s[brace2] != '}'; brace2 --)
			;
		if (brace2 >= brace1)
			run_script(s.substr(brace1, brace2 - brace1));
		else
			gi->debug_print("Unterminated brace block in " + s);
		return;
	}

	// SENSITIVE?
	if (tok == "action") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after action in " + s);
			return;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		if (index == -1) {
			gi->debug_print("Error: no semicolon in " + s);
			return;
		}
		set_obj_action(trim(tok.substr(0, index)),
		               "<" + trim(tok.substr(index + 1)) + "> " + s.substr(c2 + 1));
		return;
	}
	// SENSITIVE?
	else if (tok == "animate") {
	}
	// SENSITIVE?
	else if (tok == "background") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			gi->set_background(eval_param(tok));
		else
			gi->debug_print("Expected parameter after foreground in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "choose") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after choose in " + s);
			return;
		}
		tok = eval_param(tok);
		const GeasBlock *gb = gf.find_by_name("selection", tok);
		if (gb == NULL) {
			gi->debug_print("No selection called " + tok + " found");
			return;
		}
		String question, line;
		Common::Array<String> choices, actions;
		for (uint ln = 0; ln < gb->data.size(); ln ++) {
			line = gb->data[ln];
			tok = first_token(line, c1, c2);
			// SENSITIVE?
			if (tok == "info") {
				tok = next_token(line, c1, c2);
				if (is_param(tok))
					question = eval_param(tok);
				else
					gi->debug_print("Expected parameter after info in " + line);
			}
			// SENSITIVE?
			else if (tok == "choice") {
				tok = next_token(line, c1, c2);
				if (is_param(tok)) {
					choices.push_back(eval_param(tok));
					actions.push_back(line.substr(c2));
				} else
					gi->debug_print("Expected parameter after choice in " + line);
			} else
				gi->debug_print("Bad line " + line + " in selection");
		}
		if (choices.size() == 0)
			//gi->debug_print ("No choices in selection " + gb->lname);
			gi->debug_print("No choices in selection " + gb->name);
		else
			run_script(actions[gi->make_choice(question, choices)]);
		return;
	}
	// SENSITIVE?
	else if (tok == "clear") {
		gi->clear_screen();
		return;
	}
	// SENSITIVE?
	else if (tok == "clone") {
		/* TODO */
	}
	// SENSITIVE?
	else if (tok == "create") {
		tok = next_token(s, c1, c2);
		// SENSITIVE?
		if (tok == "exit") { // create exit
			String dir = "";

			tok = next_token(s, c1, c2);
			if (!is_param(tok)) {
				dir = tok;
				tok = next_token(s, c1, c2);
			}

			if (!is_param(tok)) {
				gi->debug_print("Expected param after create exit in " + s);
				return;
			}
			tok = eval_param(tok);
			Common::Array<String> args = split_param(tok);
			if (args.size() != 2) {
				gi->debug_print("Expected 2 elements in param in " + s);
				return;
			}
			if (dir != "")
				state.exits.push_back(ExitRecord(args[0],
				                                 "exit " + dir + " <" + tok + ">"));
			else
				state.exits.push_back(ExitRecord(args[0], "exit <" + tok + ">"));
			//gi->debug_print ("Not yet able to create place type exits");
			regen_var_dirs();
			return;
		}
		// SENSITIVE?
		else if (tok == "object") { // create object
			/* TODO */
		}
		// SENSITIVE?
		else if (tok == "room") { // create room
			/* TODO */
		} else
			gi->debug_print("Bad create line " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "debug") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			gi->debug_print(eval_param(tok));
		else
			gi->debug_print("Expected param after debug in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "destroy") {
		tok = next_token(s, c1, c2);
		if (tok != "exit") {
			gi->debug_print("expected 'exit' after 'destroy' in " + s);
			return;
		}
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected param after 'destroy exit' in " + s);
			return;
		}
		tok = eval_param(tok);
		Common::Array<String> args = split_param(tok);
		if (args.size() != 2) {
			gi->debug_print("Expected two arguments in " + s);
			return;
		}
		//state.exits.push_back (ExitRecord (args[0], "destroy exit <" + tok + ">"));
		state.exits.push_back(ExitRecord(args[0], "destroy exit " + args[1]));
		regen_var_dirs();
		return;
	}
	// SENSITIVE?
	else if (tok == "disconnect") {
		/* QNSO */
	}
	// SENSITIVE?
	else if (tok == "displaytext") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after displaytext in " + s);
			return;
		}
		const GeasBlock *gb = gf.find_by_name("text", param_contents(tok));
		if (gb != NULL) {
			for (uint i = 0; i < gb->data.size(); i ++) {
				print_formatted(gb->data[i]);
				print_newline();
			}
		} else
			gi->debug_print("No such text block " + tok);
		return;
	}
	// SENSITIVE?
	else if (tok == "do") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after do in " + s);
			return;
		}
		String fname = eval_param(tok);
		int index = fname.find('(');
		if (index != -1) {
			int index2 = fname.find(')');
			run_procedure(trim(fname.substr(0, index)),
			              split_f_args(fname.substr(index + 1, index2 - index - 1)));
		} else
			run_procedure(fname);

		//run_procedure (fname);
		return;
	}
	// SENSITIVE?
	else if (tok == "doaction") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after doaction in " + s);
			return;
		}
		String line = eval_param(tok);
		int index = line.find(';');
		String obj = trim(line.substr(0, index));
		String act = trim(line.substr(index + 1));
		String old_object = this_object;
		this_object = obj;
		if (get_obj_action(obj, act, tok))
			run_script_as(obj, tok);
		//run_script (tok);
		else
			gi->debug_print("No action defined for " + obj + " // " + act);
		this_object = old_object;
		return;
	}
	// SENSITIVE?
	else if (tok == "dontprocess") {
		dont_process = true;
		return;
	}
	// SENSITIVE?
	else if (tok == "enter") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after enter in " + s);
			return;
		}
		tok = eval_param(tok);
		set_svar(tok, gi->get_string());
		return;
	}
	// SENSITIVE?
	else if (tok == "exec") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after exec in " + s);
			return;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		if (index != -1) {
			String tmp = trim(tok.substr(index + 1));
			// SENSITIVE?
			if (tmp == "normal") {
				//run_command (trim (tok.substr (0, index)), true, true);
				try_match(trim(tok.substr(0, index)), true, true);
			} else {
				gi->debug_print("Bad " + tmp + " in exec in " + s);
				//run_command (trim (tok.substr (0, index)), true, false);
				try_match(trim(tok.substr(0, index)), true, false);
			}
		} else {
			//run_command (trim (tok.substr (0, index)), true, false);
			try_match(trim(tok.substr(0, index)), true, false);
		}
		return;
	}
	// SENSITIVE?
	else if (tok == "flag") {
		tok = next_token(s, c1, c2);
		bool is_on;
		// SENSITIVE?
		if (tok == "on")
			is_on = true;
		// SENSITIVE?
		else if (tok == "off")
			is_on = false;
		else {
			gi->debug_print("Expected 'on' or 'off' after flag in " + s);
			return;
		}
		String onoff = tok;

		tok = next_token(s, c1, c2);
		if (is_param(tok))
			set_obj_property("game", (is_on ? "" : "not ") + eval_param(tok));
		else
			gi->debug_print("Expected param after flag " + onoff + " in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "font") {
		/* TODO */
	}
	// SENSITIVE?
	else if (tok == "for") {
		tok = next_token(s, c1, c2);
		// SENSITIVE?
		if (tok == "each") {
			// SENSITIVE?
			if (next_token(s, c1, c2) == "object" &&
			        next_token(s, c1, c2) == "in") {
				tok = next_token(s, c1, c2);
				// SENSITIVE?
				if (tok == "game") {
					/* TODO: This will run over the game, rooms, and objects */
					/* It should just do the objects. */
					String script = s.substr(c2);
					// Start at 1 to skip game
					for (uint i = 1; i < state.objs.size(); i ++) {
						cerr << "  quest.thing -> " + state.objs[i].name + "\n";
						set_svar("quest.thing", state.objs[i].name);
						run_script(script);
					}
					return;
				} else if (is_param(tok)) {
					tok = trim(eval_param(tok));
					String script = s.substr(c2);
					for (uint i = 0; i < state.objs.size(); i ++)
						if (state.objs[i].parent == tok) {
							set_svar("quest.thing", state.objs[i].name);
							run_script(script);
						}
					return;
				}
			}
		} else if (is_param(tok)) {
			Common::Array<String> args = split_param(eval_param(tok));
			String varname = args[0];
			String script = s.substr(c2);
			int startindex = parse_int(args[1]);
			int endindex = parse_int(args[2]);
			int step = 1;
			if (args.size() > 3)
				step = parse_int(args[3]);
			for (set_ivar(varname, startindex); get_ivar(varname) < endindex;
			        set_ivar(varname, get_ivar(varname) + step))
				run_script(script);
			return;
		}

	}
	// SENSITIVE?
	else if (tok == "foreground") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			gi->set_foreground(eval_param(tok));
		else
			gi->debug_print("Expected parameter after foreground in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "give") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after give in " + s);
			return;
		}
		tok = eval_param(tok);
		move(tok, "inventory");
		String tmp;
		if (get_obj_action(tok, "gain", tmp))
			run_script_as(tok, tmp);
		//run_script (tmp);
		else if (get_obj_property(tok, "gain", tmp))
			print_formatted(tmp);
		return;
	}
	// SENSITIVE?
	else if (tok == "goto") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			goto_room(trim(eval_param(tok)));
		else
			gi->debug_print("Expected parameter after goto in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "helpclear") {
	}
	// SENSITIVE?
	else if (tok == "helpclose") {
	}
	// SENSITIVE?
	else if (tok == "helpdisplaytext") {
	}
	// SENSITIVE?
	else if (tok == "helpmsg") {
	}
	// SENSITIVE?
	else if (tok == "hide") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			set_obj_property(eval_param(tok), "hidden");
		else
			gi->debug_print("Expected param after conceal in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "show") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			set_obj_property(eval_param(tok), "not hidden");
		else
			gi->debug_print("Expected param after conceal in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "if") {
		/* TODO TODO */
		uint begin_cond = c2 + 1, end_cond, begin_then, end_then;

		do {
			tok = next_token(s, c1, c2);
			// SENSITIVE?
		} while (tok != "then" && tok != "");

		if (tok == "") {
			gi->debug_print("Expected then in if: " + s);
			return;
		}
		end_cond = c1;
		String cond_str = s.substr(begin_cond, end_cond - begin_cond);

		begin_then = c2 + 1;
		int brace_count = 0;
		do {
			tok = next_token(s, c1, c2);
			for (uint i = 0; i < tok.length(); i ++)
				if (tok[i] == '{')
					brace_count ++;
				else if (tok[i] == '}')
					brace_count --;
			// SENSITIVE?
		} while (tok != "" && !(brace_count == 0 && tok == "else"));
		end_then = c1;


		if (eval_conds(cond_str))
			run_script(s.substr(begin_then, end_then - begin_then), rv);
		else if (c2 < s.length())
			run_script(s.substr(c2), rv);
		return;
	}
	// SENSITIVE?
	else if (tok == "inc" || tok == "dec") {
		// SENSITIVE?
		bool is_dec = (tok == "dec");
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after inc in " + s);
			return;
		}
		tok = eval_param(tok);
		int diff;
		int index = tok.find(';');
		String varname;
		if (index == -1) {
			varname = trim(tok);
			diff = 1;
		} else {
			varname = trim(tok.substr(0, index));
			diff = eval_int(tok.substr(index + 1));
		}
		if (is_dec)
			set_ivar(varname, get_ivar(varname) - diff);
		else
			set_ivar(varname, get_ivar(varname) + diff);
		return;
	}
	// SENSITIVE?
	else if (tok == "lose") {
		/* TODO TODO */
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after lose in " + s);
			return;
		}
		tok = eval_param(tok);

		/* TODO: is the object always moved to location, or only
		 * when it had been in the inventory ?
		 */
		bool was_lost = (ci_equal(get_obj_parent(tok), "inventory"));
		if (was_lost) {
			move(tok, state.location);
			String tmp;
			if (get_obj_action(tok, "lose", tmp))
				run_script_as(tok, tmp);
			//run_script (tmp);
			else if (get_obj_property(tok, "lose", tmp))
				print_formatted(tmp);
		}
		return;
	}
	// SENSITIVE?
	else if (tok == "mailto") {
	}
	// SENSITIVE?
	else if (tok == "modvolume") {
	}
	// SENSITIVE?
	else if (tok == "move") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after move in " + s);
			return;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		if (index == -1) {
			gi->debug_print("No semi in " + tok + " in " + s);
			return;
		}
		move(trim(tok.substr(0, index)), trim(tok.substr(index + 1)));
		return;
	}
	// SENSITIVE?
	else if (tok == "msg") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			print_eval(param_contents(tok));
		else
			gi->debug_print("Expected parameter after msg in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "msgto") {
		/* QNSO */
	}
	// SENSITIVE?
	else if (tok == "outputoff") {
		//print_formatted ("<<");
		outputting = false;
		return;
	}
	// SENSITIVE?
	else if (tok == "outputon") {
		outputting = true;
		//print_formatted (">>");
		return;
	}
	// SENSITIVE?
	else if (tok == "panes") {
		/* TODO */
	}
	// SENSITIVE?
	else if (tok == "pause") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after pause in " + s);
			return;
		}
		int i = eval_int(param_contents(tok));
		gi->pause(i);
		return;
	}
	// SENSITIVE?
	else if (tok == "picture") {
	}
	// SENSITIVE?
	else if (tok == "playerlose") {
		run_script("displaytext <lose>");
		state.running = false;
		return;
	}
	// SENSITIVE?
	else if (tok == "playerwin") {
		run_script("displaytext <win>");
		state.running = false;
		return;
	}
	// SENSITIVE?
	else if (tok == "playmidi") {
	}
	// SENSITIVE?
	else if (tok == "playmod") {
	}
	// SENSITIVE?
	else if (tok == "playwav") {
	}
	// SENSITIVE?
	else if (tok == "property") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter in '" + s + "'");
			return;
		}
		Common::Array<String> args = split_param(eval_param(tok));
		for (uint i = 1; i < args.size(); i ++) {
			String val = args[i];
			/*
			if (val[0] == '[' && val[val.length() - 1] == ']')
			  val = val.substr (1, val.length() - 2);
			//state.props.push_back (PropertyRecord (args[0], val));
			*/
			val = trim_braces(val);
			set_obj_property(args[0], val);
		}
		return;
	}
	// SENSITIVE?
	else if (tok == "repeat") {
		/* TODO TODO: assumes script is a "do ..." */
		tok = next_token(s, c1, c2);
		// SENSITIVE?
		if (tok != "while" && tok != "until") {
			gi->debug_print("Expected while or until after repeat in " + s);
			return;
		}
		bool is_while = (tok == "while");
		int start_cond = c2, end_cond = -1;
		while ((tok = next_token(s, c1, c2)) != "") {
			// SENSITIVE?
			if (tok == "do") {
				end_cond = c1;
				break; // TODO: Do I break here?
			}
		}
		if (end_cond == -1) {
			gi->debug_print("No script found after condition in " + s);
			return;
		}
		String cond = trim(s.substr(start_cond, end_cond - start_cond));
		String script = trim(s.substr(end_cond));
		cerr << "Interpreting '" << s << "' as ("
		     << (is_while ? "WHILE" : "UNTIL") << ") ("
		     << cond << ") {" << script << "}\n";
		while (eval_conds(cond) == is_while)
			run_script(script);
		return;
	}
	// SENSITIVE?
	else if (tok == "return") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			rv = eval_param(tok);
		else
			gi->debug_print("Expected parameter after return in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "reveal") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			set_obj_property(eval_param(tok), "not invisible");
		else
			gi->debug_print("Expected param after reveal in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "conceal") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			set_obj_property(eval_param(tok), "invisible");
		else
			gi->debug_print("Expected param after conceal in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "say") {
		tok = next_token(s, c1, c2);
		if (is_param(tok)) {
			tok = eval_param(tok);
			print_formatted("\"" + tok + "\"");
		} else
			gi->debug_print("Expected param after say in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "set") {
		String vartype = "";
		tok = next_token(s, c1, c2);
		// SENSITIVE?
		if (tok == "interval") {
			tok = next_token(s, c1, c2);
			if (!is_param(tok)) {
				gi->debug_print("Expected param after set interval in " + s);
				return;
			}
			tok = eval_param(tok);
			int index = tok.find(';');
			if (index == -1) {
				gi->debug_print("No semicolon in param in " + s);
				return;
			}
			//String timer_name = lcase (trim (tok.substr (0, index)));
			String timer_name = trim(tok.substr(0, index));
			uint time_val = parse_int(trim(tok.substr(index + 1)));

			for (uint i = 0; i < state.timers.size(); i ++)
				if (state.timers[i].name == timer_name) {
					state.timers[i].interval = time_val;
					return;
				}
			gi->debug_print("no interval named " + timer_name + " found!");
			return;
		}
		// SENSITIVE?
		if (tok == "String" || tok == "numeric") {
			vartype = tok;
			tok = next_token(s, c1, c2);
		}
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter in " + s);
			return;
		}
		if (tok.find(';') == -1) {
			gi->debug_print("Only one expression in set in " + s);
			return;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		//String varname = lcase (trim (tok.substr (0, index)));
		String varname = trim(tok.substr(0, index));
		if (vartype == "") {
			for (uint varn = 0; varn < state.ivars.size(); varn ++)
				if (state.ivars[varn].name == varname) {
					vartype = "numeric";
					break;
				}
			if (vartype == "")
				for (uint varn = 0; varn < state.svars.size(); varn ++)
					if (state.svars[varn].name == varname) {
						vartype = "String";
						break;
					}
		}
		if (vartype == "") {
			gi->debug_print("Undefined variable " + varname + " in " + s);
			return;
		}
		if (vartype == "String")
			set_svar(varname, trim_braces(trim(tok.substr(index + 1))));
		else
			set_ivar(varname, eval_int(tok.substr(index + 1)));
		return;
	}
	// SENSITIVE?
	else if (tok == "setstring") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter in " + s);
			return;
		}
		if (tok.find(';') == -1) {
			gi->debug_print("Only one expression in set in " + s);
			return;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		String varname = trim(tok.substr(0, index));
		set_svar(varname, trim_braces(trim(tok.substr(index + 1))));
		return;
	}
	// SENSITIVE?
	else if (tok == "setvar") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter in " + s);
			return;
		}
		if (tok.find(';') == -1) {
			gi->debug_print("Only one expression in set in " + s);
			return;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		String varname = trim(tok.substr(0, index));
		set_ivar(varname, eval_int(tok.substr(index + 1)));
		return;
	}
	// SENSITIVE?
	else if (tok == "shell") {

	}
	// SENSITIVE?
	else if (tok == "shellexe") {
	}
	// SENSITIVE?
	else if (tok == "speak") {
		tok = next_token(s, c1, c2);
		if (is_param(tok))
			gi->speak(eval_param(tok));
		else
			gi->debug_print("Expected param after speak in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "stop") {
		state.running = false;
		return;
	}
	// SENSITIVE?
	else if (tok == "timeron" || tok == "timeroff") {
		// SENSITIVE?
		bool running = (tok == "timeron");
		//tok = lcase (next_token (s, c1, c2));
		tok = next_token(s, c1, c2);
		if (is_param(tok)) {
			tok = eval_param(tok);
			for (uint i = 0; i < state.timers.size(); i ++)
				if (state.timers[i].name == tok) {
					if (running)
						state.timers[i].timeleft = state.timers[i].interval;
					state.timers[i].is_running = running;
					return;
				}
			gi->debug_print("No timer " + tok + " found");
			return;
		}
		gi->debug_print(String("Expected parameter after timer") +
		                (running ? "on" : "off") + " in " + s);
		return;
	}
	// SENSITIVE?
	else if (tok == "type") {
		/* TODO */
	}
	// SENSITIVE?
	else if (tok == "wait") {
		tok = next_token(s, c1, c2);
		if (tok != "") {
			if (!is_param(tok)) {
				gi->debug_print("Expected parameter after wait in " + s);
				return;
			}
			tok = eval_param(tok);
		}
		gi->wait_keypress(tok);
		return;
	}
	// SENSITIVE?
	else if (tok == "with") {
		// QNSO
	}
	gi->debug_print("Unrecognized script " + s);
}

bool geas_implementation::eval_conds(String s) {
	cerr << "if (" + s + ")" << endl;

	uint c1, c2;
	String tok = first_token(s, c1, c2);

	if (tok == "") return true;

	bool rv = eval_cond(s);

	while (tok != "" && tok != "and")
		tok = next_token(s, c1, c2);

	if (tok == "and")
		rv = rv && eval_conds(s.substr(c2));
	else {
		tok = first_token(s, c1, c2);
		while (tok != "" && tok != "or")
			tok = next_token(s, c1, c2);
		if (tok == "or")
			rv = rv || eval_conds(s.substr(c2));
	}

	cerr << "if (" << s << ") --> " << (rv ? "true" : "false") << endl;
	return rv;
}

bool geas_implementation::eval_cond(String s) {
	uint c1, c2;
	String tok = first_token(s, c1, c2);
	// SENSITIVE?
	if (tok == "not")
		return !eval_cond(s.substr(c2));
	// SENSITIVE?
	else if (tok == "action") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after property in " + s);
			return false;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		if (index == -1) {
			gi->debug_print("Only one argument to property in " + s);
			return false;
		}
		String obj = trim(tok.substr(0, index));
		String act = trim(tok.substr(index + 1));
		return has_obj_action(obj, act);
	}
	// SENSITIVE?
	else if (tok == "ask") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after ask in " + s);
			return false;
		}
		tok = eval_param(tok);
		return gi->choose_yes_no(tok);
	}
	// SENSITIVE?
	else if (tok == "exists") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after exists in " + s);
			return false;
		}
		Common::Array<String> args = split_param(eval_param(tok));
		bool do_report = false;
		for (uint i = 1; i < args.size(); i ++)
			// SENSITIVE?
			if (args[i] == "report")
				do_report = true;
			else
				gi->debug_print("Got modifier " + args[i] + " after exists");
		//args[0] = lcase (args[0]);
		for (uint i = 0; i < state.objs.size(); i ++)
			if (ci_equal(state.objs[i].name, args[0]))
				return state.objs[i].parent != "";
		if (do_report)
			gi->debug_print("exists " + args[0] + " failed due to nonexistence");
		return false;
	}
	// SENSITIVE?
	else if (tok == "flag") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after flag in " + s);
			return false;
		}
		tok = trim(eval_param(tok));
		return has_obj_property("game", tok);
	}
	// SENSITIVE?
	else if (tok == "got") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after got in " + s);
			return false;
		}
		//tok = lcase (trim (eval_param (tok)));
		tok = trim(eval_param(tok));
		for (uint i = 0; i < state.objs.size(); i ++)
			if (ci_equal(state.objs[i].name, tok))
				return ci_equal(state.objs[i].parent, "inventory");
		gi->debug_print("No object " + tok + " found while evaling " + s);
		return false;
	}
	// SENSITIVE?
	else if (tok == "here") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after here in " + s);
			return false;
		}
		//tok = lcase (trim (eval_param (tok)));
		tok = trim(eval_param(tok));
		for (uint i = 0; i < state.objs.size(); i ++)
			if (ci_equal(state.objs[i].name, tok)) {
				//return (ci_equal (state.objs[i].parent, state.location) &&
				//    !has_obj_property (tok, "invisible"));
				return (ci_equal(state.objs[i].parent, state.location));
			}
		/* TODO: is it invisible or hidden? */
		gi->debug_print("No object " + tok + " found while evaling " + s);
		return false;
	}
	// SENSITIVE?
	else if (tok == "is") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after is in " + s);
			return false;
		}
		tok = eval_param(tok);
		int index;
		// SENSITIVE?
		if ((index = tok.find("!=;")) != -1) {
			uint index1 = index;
			do {
				-- index1;
			} while (index1 > 0 && tok[index1] != ';');

			cerr << "Comparing <" << trim_braces(trim(tok.substr(0, index1)))
			     << "> != <" << trim_braces(trim(tok.substr(index + 3)))
			     << ">\n";
			return ci_notequal(trim_braces(trim(tok.substr(0, index - 1))),
			                   trim_braces(trim(tok.substr(index + 3))));
		}
		if ((index = tok.find("lt=;")) != -1) {
			cerr << "Comparing <" << trim_braces(trim(tok.substr(0, index)))
			     << "> < <" << trim_braces(trim(tok.substr(index + 4)))
			     << ">\n";
			return eval_int(tok.substr(0, index - 1))
			       <= eval_int(tok.substr(index + 4));
		}
		if ((index = tok.find("gt=;")) != -1)
			return eval_int(tok.substr(0, index))
			       >= eval_int(tok.substr(index + 4));
		if ((index = tok.find("lt;")) != -1)
			return eval_int(tok.substr(0, index))
			       < eval_int(tok.substr(index + 3));
		if ((index = tok.find("gt;")) != -1)
			return eval_int(tok.substr(0, index))
			       > eval_int(tok.substr(index + 3));
		if ((index = tok.find(";")) != -1) {
			cerr << "Comparing <" << trim_braces(trim(tok.substr(0, index)))
			     << "> == <" << trim_braces(trim(tok.substr(index + 1)))
			     << ">\n";
			return ci_equal(trim_braces(trim(tok.substr(0, index))),
			                trim_braces(trim(tok.substr(index + 1))));
		}
		gi->debug_print("Bad is condition " + tok + " in " + s);
		return false;
	}
	// SENSITIVE?
	else if (tok == "property") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after property in " + s);
			return false;
		}
		tok = eval_param(tok);
		int index = tok.find(';');
		if (index == -1) {
			gi->debug_print("Only one argument to property in " + s);
			return false;
		}
		String obj = trim(tok.substr(0, index));
		String prop = trim(tok.substr(index + 1));
		return has_obj_property(obj, prop);
	}
	// SENSITIVE?
	else if (tok == "real") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("expected parameter after real in " + s);
			return false;
		}
		Common::Array<String> args = split_param(eval_param(tok));
		bool do_report = false;
		for (uint i = 1; i < args.size(); i ++)
			// SENSITIVE?
			if (args[i] == "report")
				do_report = true;
			else
				gi->debug_print("Got modifier " + args[i] + " after exists");
		//args[0] = lcase (args[0]);
		for (uint i = 0; i < state.objs.size(); i ++)
			if (ci_equal(state.objs[i].name, args[0]))
				return true;
		if (do_report)
			gi->debug_print("real " + args[0] + " failed due to nonexistence");
		return false;
	}
	// SENSITIVE?
	else if (tok == "type") {
		tok = next_token(s, c1, c2);
		if (!is_param(tok)) {
			gi->debug_print("Expected parameter after type in " + s);
			return false;
		}
		Common::Array<String> args = split_param(eval_param(tok));
		if (args.size() != 2) {
			gi->debug_print("Expected two parameters to type in " + s);
			return false;
		}
		return gf.obj_of_type(args[0], args[1]);
	}

	gi->debug_print("Bad condition " + s);
	return false;
}

void geas_implementation::run_procedure(String pname, Common::Array<String> args) {
	cerr << "run_procedure " << pname << " (" << args << ")\n";
	Common::Array<String> backup = function_args;
	function_args = args;
	run_procedure(pname);
	function_args = backup;
}

void geas_implementation::run_procedure(String pname) {
	for (uint i = 0; i < gf.size("procedure"); i ++)
		if (ci_equal(gf.block("procedure", i).name, pname)) {
			const GeasBlock &proc = gf.block("procedure", i);
			//cerr << "Running procedure " << proc << endl;
			for (uint j = 0; j < proc.data.size(); j ++) {
				//cerr << "  Running line #" << j << ": " << proc.data[j] << endl;
				run_script(proc.data[j]);
			}
			return;
		}
	gi->debug_print("No procedure " + pname + " found.");
}

String geas_implementation::run_function(String pname, Common::Array<String> args) {
	cerr << "run_function (w/ args) " << pname << " (" << args << ")\n";
	/* Parameter is handled specially because it can't change the stack */
	// SENSITIVE?
	if (pname == "parameter") {
		if (args.size() != 1) {
			gi->debug_print("parameter called with " + string_int(args.size())
			                + " args");
			return "";
		}
		uint num = parse_int(args[0]);
		if (0 < num && num <= function_args.size()) {
			cerr << "   --> " << function_args[num - 1] << "\n";
			return function_args[num - 1];
		}
		cerr << "   --> too many arguments\n";
		return "";
	}
	Common::Array<String> backup = function_args;
	function_args = args;
	for (uint i = 0; i < args.size(); i ++)
		set_svar("quest.function.parameter." + string_int(i + 1), args[i]);
	String rv = run_function(pname);
	function_args = backup;
	return rv;
}

String geas_implementation::bad_arg_count(String fname) {
	gi->debug_print("Called " + fname + " with " +
	                string_int(function_args.size()) + " arguments.");
	return "";
}

String geas_implementation::run_function(String pname) {
	cerr << "geas_implementation::run_function (" << pname << ", " << function_args << ")\n";
	//pname = lcase (pname);
	// SENSITIVE?
	if (pname == "getobjectname") {
		if (function_args.size() == 0)
			return bad_arg_count(pname);
		//return get_obj_name (function_args);
		Common::Array<String> where;
		for (uint i = 1; i < function_args.size(); i ++)
			where.push_back(function_args[i]);
		if (where.size() == 0) {
			where.push_back(state.location);
			where.push_back("inventory");
		}
		bool is_internal = false;
		return get_obj_name(function_args[0], where, is_internal);
	}
	// SENSITIVE?
	else if (pname == "loadmethod") {
		/* TODO TODO */
		return "normal";
		// "loaded" on restore from qsg
	}
	// SENSITIVE?
	else if (pname == "locationof") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		return get_obj_parent(function_args[0]);
	}
	// SENSITIVE?
	else if (pname == "objectproperty") {
		if (function_args.size() != 2)
			return bad_arg_count(pname);

		String rv;
		get_obj_property(function_args[0], function_args[1], rv);
		return rv;
	}
	// SENSITIVE?
	else if (pname == "timerstate") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		//String timername = lcase (function_args[0]);
		String timername = function_args[0];
		for (uint i = 0; i < state.timers.size(); i ++)
			if (state.timers[i].name == timername)
				return state.timers[i].is_running ? "1" : "0";
		return "!";
	}
	// SENSITIVE?
	else if (pname == "displayname") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		return displayed_name(function_args[0]);
	}
	// SENSITIVE?
	else if (pname == "capfirst") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		return pcase(function_args[0]);
	}
	// SENSITIVE?
	else if (pname == "instr") {
		/* TODO What if it's not present? */
		if (function_args.size() != 2 && function_args.size() != 3)
			return bad_arg_count(pname);

		int rv;
		if (function_args.size() == 2)
			rv = function_args[0].find(function_args[1]);
		else
			rv = function_args[1].find(function_args[2],
			                           parse_int(function_args[0]));

		if (rv == -1)
			return string_int(rv); // TODO What goes here?
		else
			return string_int(rv);
	}
	// SENSITIVE?
	else if (pname == "lcase") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		String rv = function_args[0];
		for (uint i = 0; i < rv.size(); i ++)
			rv[i] = tolower(rv[i]);
		return rv;
	}
	// SENSITIVE?
	else if (pname == "left") {
		if (function_args.size() != 2)
			return bad_arg_count(pname);

		uint i = parse_int(function_args[1]);
		if (i > function_args[0].length())
			return function_args[0];
		else
			return function_args[0].substr(0, i);
	}
	// SENSITIVE?
	else if (pname == "lengthof") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		return string_int(function_args[0].length());
	}
	// SENSITIVE?
	else if (pname == "mid") {
		if (function_args.size() != 3)
			return bad_arg_count(pname);

		uint start = parse_int(function_args[1]),
		     len = parse_int(function_args[2]);
		if (start > function_args[0].length())
			return "";
		if (start + len > function_args[0].length())
			return function_args[0].substr(start);
		return function_args[0].substr(start, len);
	}
	// SENSITIVE?
	else if (pname == "right") {
		if (function_args.size() != 2)
			return bad_arg_count(pname);

		uint size = parse_int(function_args[1]);
		if (size > function_args[0].length())
			return function_args[0];
		return function_args[0].substr(function_args[0].length() - size);
	} else if (pname == "ubound") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		/* TODO TODO */
		return "";
	}
	// SENSITIVE?
	else if (pname == "ucase") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		String rv = function_args[0];
		for (uint i = 0; i < rv.length(); i ++)
			rv[i] = toupper(rv[i]);
		return rv;
	}
	// SENSITIVE?
	else if (pname == "rand") {
		if (function_args.size() != 2)
			return bad_arg_count(pname);
		uint lower = parse_int(function_args[0]),
		     upper = parse_int(function_args[1]);

		// TODO: change this to use the high order bits of the random # instead
		return string_int(lower + (g_vm->getRandomNumber(0x7fffff) % (upper + 1 - lower)));
	}
	// SENSITIVE?
	else if (pname == "speechenabled") {
		if (function_args.size() != 0)
			return bad_arg_count(pname);

		return "0";
		/* TODO: return 1 if speech is enabled */
	}
	// SENSITIVE?
	else if (pname == "symbol") {
		if (function_args.size() != 1)
			return bad_arg_count(pname);

		// SENSITIVE?
		if (function_args[0] == "gt")
			return ">";
		// SENSITIVE?
		else if (function_args[0] == "lt")
			return "<";
		gi->debug_print("Bad symbol argument: " + function_args[0]);
		return "";
	}
	// SENSITIVE?
	else if (pname == "numberparameters")
		return string_int(function_args.size());
	// SENSITIVE?
	else if (pname == "thisobject")
		return this_object;

	/* disconnectedby, id, name, removeformatting */

	String rv = "";

	for (uint i = 0; i < gf.size("function"); i ++)
		if (ci_equal(gf.block("function", i).name, pname)) {
			const GeasBlock &proc = gf.block("function", i);
			cerr << "Running function " << proc << endl;
			for (uint j = 0; j < proc.data.size(); j ++) {
				cerr << "  Running line #" << j << ": " << proc.data[j] << endl;
				run_script(proc.data[j], rv);
			}
			return rv;
		}
	gi->debug_print("No function " + pname + " found.");
	return "";
}



v2string geas_implementation::get_inventory() {
	return get_room_contents("inventory");
}

v2string geas_implementation::get_room_contents() {
	return get_room_contents(state.location);
}

v2string geas_implementation::get_room_contents(String room) {
	v2string rv;
	String objname;
	for (uint i = 0; i < state.objs.size(); i ++)
		if (state.objs[i].parent == room) {
			objname = state.objs[i].name;
			if (!has_obj_property(objname, "invisible") &&
			        !has_obj_property(objname, "hidden")) {
				vstring tmp;

				String print_name, temp_str;
				if (!get_obj_property(objname, "alias", print_name))
					print_name = objname;
				/*
				if (get_obj_property (objname, "prefix", temp_str))
				  print_name = temp_str + " " + print_name;
				if (get_obj_property (objname, "suffix", temp_str))
				  print_name = print_name + " " + temp_str;
				*/
				tmp.push_back(print_name);

				String otype;
				if (!get_obj_property(objname, "displaytype", otype))
					otype = "object";
				tmp.push_back(otype);
				rv.push_back(tmp);
			}
		}
	return rv;
}

vstring geas_implementation::get_status_vars() {
	vstring rv;

	String tok, line;
	uint c1, c2;

	for (uint i = 0; i < gf.size("variable"); i ++) {
		const GeasBlock &gb = gf.block("variable", i);

		bool nozero = false;
		String disp;
		bool is_numeric = true;

		cerr << "g_s_v: " << gb << endl;

		for (uint j = 0; j < gb.data.size(); j ++) {
			line = gb.data[j];
			cerr << "  g_s_v:  " << line << endl;
			tok = first_token(line, c1, c2);
			// SENSITIVE?
			if (tok == "display") {
				tok = next_token(line, c1, c2);

				// SENSITIVE?
				if (tok == "nozero") {
					nozero = true;
					tok = next_token(line, c1, c2);
				}
				if (!is_param(tok))
					gi->debug_print("Expected param after display: " + line);
				else
					disp = tok;
			}
			// SENSITIVE?
			else if (tok == "type") {
				tok = next_token(line, c1, c2);
				// SENSITIVE?
				if (tok == "String")
					is_numeric = false;
			}
		}

		cerr << "  g_s_v, block 2, tok == '" << tok << "'" << endl;
		if (!(is_numeric && nozero && get_ivar(gb.name) == 0) && disp != "") {
			disp = param_contents(disp);
			String outval = "";
			for (uint j = 0; j < disp.length(); j ++)
				if (disp[j] == '!') {
					if (is_numeric)
						outval = outval + string_int(get_ivar(gb.name));
					//outval = outval + string_int (get_ivar (gb.lname));
					else
						outval = outval + get_svar(gb.name);
					//outval = outval + get_svar (gb.lname);
				} else if (disp[j] == '*') {
					uint k;
					for (k = j + 1; k < disp.length() && disp[k] != '*'; k ++)
						;
					//if (!is_numeric || get_ivar (gb.lname) != 1)
					if (!is_numeric || get_ivar(gb.name) != 1)
						outval = outval + disp.substr(j + 1, k - j - 1);
					j = k;
				} else
					outval = outval + disp[j];
			rv.push_back(eval_string(outval));
		}
	}
	return rv;
}

Common::Array<bool> geas_implementation::get_valid_exits() {
	Common::Array<bool> rv;
	cerr << "Getting valid exits\n";
	rv.push_back(exit_dest(state.location, "northwest") != "");
	rv.push_back(exit_dest(state.location, "north") != "");
	rv.push_back(exit_dest(state.location, "northeast") != "");
	rv.push_back(exit_dest(state.location, "west") != "");
	rv.push_back(exit_dest(state.location, "out") != "");
	rv.push_back(exit_dest(state.location, "east") != "");
	rv.push_back(exit_dest(state.location, "southwest") != "");
	rv.push_back(exit_dest(state.location, "south") != "");
	rv.push_back(exit_dest(state.location, "southeast") != "");
	rv.push_back(exit_dest(state.location, "up") != "");
	rv.push_back(exit_dest(state.location, "down") != "");
	cerr << "Done getting valid exits\n";

	return rv;
}

void geas_implementation::print_eval_p(String s) {
	print_formatted(pcase(eval_string(s)));
}

void geas_implementation::print_eval(String s) {
	print_formatted(eval_string(s));
}

String geas_implementation::eval_string(String s) {
	String rv;
	uint i, j;
	bool do_print = (s.find('$') != -1);
	if (do_print) cerr << "eval_string (" << s << ")\n";
	for (i = 0; i < s.length(); i ++) {
		//if (do_print) cerr << "e_s: i == " << i << ", s[i] == '" << s[i] << "'\n";
		if (i + 1 < s.length() && s[i] == '#' && s[i + 1] == '@') {
			for (j = i + 1; j < s.length() && s[j] != '#'; j ++)
				;
			if (j == s.length()) {
				gi->debug_print("eval_string: Unmatched hash in " + s);
				break;
			}
			//cerr << "dereferencing " + s.substr (i+2, j-i-2) << endl;
			rv = rv + displayed_name(get_svar(s.substr(i + 2, j - i - 2)));
			i = j;
		} else if (s[i] == '#') {
			for (j = i + 1; j < s.length() && s[j] != '#'; j ++)
				;
			if (j == s.length()) {
				gi->debug_print("eval_string: Unmatched hash in " + s);
				break;
			}
			uint k;
			for (k = i + 1; k < j && s[k] != ':'; k ++)
				;
			if (k == j && j == i + 1)
				rv += "#";
			else if (k == j)
				rv += get_svar(s.substr(i + 1, j - i - 1));
			else {
				String propname = s.substr(k + 1, j - k - 1);
				if (s[i + 1] == '(') {
					if (s[k - 1] != ')') {
						gi->debug_print("e_s: Missing paren in '" +
						                s.substr(i, j - i) + "' of '" + s + "'");
						break;
					}
					String objvar = s.substr(i + 2, k - i - 3);
					String objname = get_svar(objvar);
					/*
					cerr << "e_s: Getting prop [(" << objvar << ")] --> ["
					     << objname
					     << "]:[" << propname << "] for " << s << endl;
					*/
					String tmp;
					if (get_obj_property(objname, propname, tmp))
						rv += tmp;
					else
						gi->debug_print("e_s: Evaluating nonexistent object prop "
						                "{" + objname + "}:{" + propname + "}");
				} else {
					String objname = s.substr(i + 1, k - i - 1);
					/*
					cerr << "e_s: Getting prop [" << objname << "]:["
					     << propname << "] for " << s << endl;
					*/
					String tmp;
					if (get_obj_property(objname, propname, tmp))
						rv += tmp;
					else
						gi->debug_print("e_s: Evaluating nonexistent var " + objname);
				}
			}
			i = j;
		} else if (s[i] == '%') {
			for (j = i + 1; j < s.length() && s[j] != '%'; j ++)
				;
			if (j == s.length()) {
				gi->debug_print("e_s: Unmatched %s in " + s);
				break;
			}
			//cerr << "e_s: Getting ivar [" << s.substr (i+1, j-i-1) << "] for " << s << endl;
			if (j == i + 1)
				rv += "%";
			else
				rv += string_int(get_ivar(s.substr(i + 1, j - i - 1)));
			i = j;
		} else if (s[i] == '$') {
			j = s.find('$', i + 1);
			/*
			for (j = i + 1; j < s.length() && s[j] != '$'; j ++)
			  {
			    cerr << "  In searching for partner, j == " << j
			   << ", s[j] == '" << s[j] << "', (s[j] == '$') == "
			   << ((s[j] == '$') ? "true" : "false") << "\n";
			  }
			*/
			//if (j == rv.size())
			if (j == (uint)-1) {
				gi->debug_print("Unmatched $s in " + s);
				return rv + s.substr(i);
			}
			String tmp1 = s.substr(i + 1, j - i - 1);
			cerr << "e_s: first substr was '" << tmp1 << "'\n";
			String tmp = eval_string(tmp1);
			//String tmp = eval_string (s.substr (i + 1, j - i - 2));
			//cerr << "Taking substring of '" + s + "': '" + tmp + "'\n";
			cerr << "e_s: eval substr " + s + "': '" + tmp + "'\n";

			String func_eval;

			int paren_open, paren_close;
			if ((paren_open = tmp.find('(')) == -1)
				func_eval = run_function(tmp);
			else {
				paren_close = tmp.find(')', paren_open);
				if (paren_close == -1)
					gi->debug_print("No matching right paren in " + tmp);
				else {
					String f_name = tmp.substr(0, paren_open);
					String f_args = tmp.substr(paren_open + 1,
					                           paren_close - paren_open - 1);
					func_eval = run_function(f_name, split_f_args(f_args));
				}
			}
			rv = rv + func_eval;
			i = j;
		} else
			rv += s[i];
	}


	//cerr << "eval_string (" << s << ") -> <" << rv << ">\n";
	return rv;
}

void geas_implementation::tick_timers() {
	if (!state.running)
		return;
	//cerr << "tick_timers()\n";
	for (uint i = 0; i < state.timers.size(); i ++) {
		TimerRecord &tr = state.timers[i];
		//cerr << "  Examining " << tr << "\n";
		if (tr.is_running) {
			//cerr << "    Advancing " << tr.name << ": " << tr.timeleft
			//     << " / " << tr.interval << "\n";
			if (tr.timeleft != 0)
				tr.timeleft --;
			else {
				tr.is_running = false;
				tr.timeleft = tr.interval;
				const GeasBlock *gb = gf.find_by_name("timer", tr.name);
				if (gb != NULL) {
					//cout << "Running it!\n";
					String tok, line;
					uint c1, c2;
					for (uint j = 0; j < gb->data.size(); j ++) {
						line = gb->data[j];
						tok = first_token(line, c1, c2);
						// SENSITIVE?
						if (tok == "action") {
							run_script(line.substr(c2));
							break;
						}
					}
				}
			}
		}
	}
}

/***********************************
 *                                 *
 *                                 *
 *                                 *
 * GeasInterface related functions *
 *                                 *
 *                                 *
 *                                 *
 ***********************************/


GeasResult GeasInterface::print_formatted(String s, bool with_newline) {
	unsigned int i, j;

	cerr << "print_formatted (" << s << ", " << with_newline << ")" << endl;

	for (i = 0; i < s.length(); i ++) {
		//std::cerr << "i == " << i << std::endl;
		if (s[i] == '|') {
			// changed indicated whether cur_style has been changed
			// and update_style should be called at the end.
			// it is true unless cleared (by |n or |w).
			bool changed = true;
			j = i;
			i ++;
			if (i == s.length())
				continue;

			// Are the | codes case-sensitive?
			switch (s[i]) {
			case 'u':
				cur_style.is_underlined = true;
				break;
			case 'i':
				cur_style.is_italic     = true;
				break;
			case 'b':
				cur_style.is_bold       = true;
				break;
			case 'c':
				i ++;

				if (i == s.length()) {
					clear_screen();
					break;
				}

				switch (s[i]) {
				case 'y':
					cur_style.color = "#ffff00";
					break;
				case 'g':
					cur_style.color = "#00ff00";
					break;
				case 'l':
					cur_style.color = "#0000ff";
					break;
				case 'r':
					cur_style.color = "#ff0000";
					break;
				case 'b':
					cur_style.color = "";
					break;

				default:
					clear_screen();
					--i;
				}
				break;

			case 's': {
				i ++;
				if (i == s.length() || !(s[i] >= '0' && s[i] <= '9'))
					continue;
				i ++;
				if (i == s.length() || !(s[i] >= '0' && s[i] <= '9'))
					continue;

				int newsize = parse_int(s.substr(j, i - j));
				if (newsize > 0)
					cur_style.size = newsize;
				else
					cur_style.size = default_size;
			}
			break;

			case 'j':
				i ++;

				if (i == s.length() ||
				        !(s[i] == 'l' || s[i] == 'c' || s[i] == 'r'))
					continue;
				if (s[i] == 'l') cur_style.justify = JUSTIFY_LEFT;
				else if (s[i] == 'r') cur_style.justify = JUSTIFY_RIGHT;
				else if (s[i] == 'c') cur_style.justify = JUSTIFY_CENTER;
				break;

			case 'n':
				print_newline();
				changed = false;
				break;

			case 'w':
				wait_keypress("");
				changed = false;
				break;

			case 'x':
				i ++;

				if (s[i] == 'b')
					cur_style.is_bold = false;
				else if (s[i] == 'u')
					cur_style.is_underlined = false;
				else if (s[i] == 'i')
					cur_style.is_italic = false;
				else if (s[i] == 'n' && i + 1 == s.length())
					changed = with_newline = false;
				break;

			default:
				cerr << "p_f: Fallthrough " << s[i] << endl;
				changed = false;
			}
			if (changed)
				update_style();
		} else {
			for (j = i; i != s.length() && s[i] != '|'; i ++)
				;
			print_normal(s.substr(j, i - j));
			if (i != s.length() && s[i] == '|')
				-- i;
		}
	}
	if (with_newline)
		print_newline();
	return r_success;
}

void GeasInterface::set_default_font_size(String size) {
	default_size = parse_int(size);
}

void GeasInterface::set_default_font(String font) {
	default_font = font;
}

bool GeasInterface::choose_yes_no(String question) {
	Common::Array<String> v;
	v.push_back("Yes");
	v.push_back("No");
	return (make_choice(question, v) == 0);
}

} // End of namespace Quest
} // End of namespace Glk
