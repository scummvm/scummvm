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

#ifndef GLK_QUEST_GEAS_IMPL
#define GLK_QUEST_GEAS_IMPL

#include "glk/quest/geas_runner.h"
#include "glk/quest/geas_state.h"
#include "glk/quest/limit_stack.h"

namespace Glk {
namespace Quest {

struct match_binding {
	String var_name;
	String var_text;
	uint start, end;
	//operator String();
	String tostring();
	match_binding(String vn, uint i) : var_name(vn), start(i) {}
	void set(String vt, uint i) {
		var_text = vt;
		end = i;
	}
};

Common::WriteStream &operator<< (Common::WriteStream &, const match_binding &);


struct match_rv {
	bool success;
	Common::Array<match_binding> bindings;
	//match_rv (bool b, const Common::Array<String> &v) : success(b), bindings(v) {}
	match_rv() : success(false) {}
	match_rv(bool b, const match_rv &rv) : success(b), bindings(rv.bindings) {}
	operator bool () {
		return success;
	}
};

Common::WriteStream &operator<< (Common::WriteStream &o, const match_rv &rv);
/*
  inline ostream &operator<< (ostream &o, const match_rv &rv)
{
  //o << "match_rv {" << (rv.success ? "TRUE" : "FALSE") << ": " << rv.bindings << "}";
  o << "match_rv {" << (rv.success ? "TRUE" : "FALSE") << ": [";
  //o << rv.bindings.size();
  //o << rv.bindings;
  for (uint i = 0; i < rv.bindings.size(); i ++)
    o << rv.bindings[i] << ", ";
  o << "]}";
  return o;
}
*/

class geas_implementation : public GeasRunner {
	//GeasInterface *gi;
	GeasFile gf;
	//bool running;
	bool dont_process, outputting;
	LimitStack <GeasState> undo_buffer;
	Common::Array <String> function_args;
	String this_object;
	v2string current_places;
	bool is_running_;
	Logger logger;

public:
	geas_implementation(GeasInterface *in_gi)
		: GeasRunner(in_gi), undo_buffer(20), is_running_(true) {}
	void set_game(const String &fname) override;

	bool is_running() const override;
	GeasState *getState() override { return &state; }
	String get_banner() override;
	void run_command(String) override;
	bool try_match(String s, bool, bool);
	match_rv match_command(String input, String action) const;
	match_rv match_command(String input, uint ichar,
	                       String action, uint achar, match_rv rv) const;
	bool dereference_vars(Common::Array<match_binding> &bindings, bool is_internal) const;
	bool dereference_vars(Common::Array<match_binding> &, const Common::Array<String> &, bool is_internal) const;
	bool match_object(String text, String name, bool is_internal = false) const;
	void set_vars(const Common::Array<match_binding> &v);
	bool run_commands(String, const GeasBlock *, bool is_internal = false);

	void display_error(String errorname, String object = "");

	String substitute_synonyms(String) const;

	void set_svar(String, String);
	void set_svar(String, uint, String);
	void set_ivar(String, int);
	void set_ivar(String, uint, int);

	String get_svar(String) const;
	String get_svar(String, uint) const;
	int get_ivar(String) const;
	int get_ivar(String, uint) const;

	bool find_ivar(String, uint &) const;
	bool find_svar(String, uint &) const;

	void regen_var_look();
	void regen_var_dirs();
	void regen_var_objects();
	void regen_var_room();

	void look();

	String displayed_name(String object) const;
	//String get_obj_name (const Common::Array<String> &args) const;
	String get_obj_name(String name, const Common::Array<String> &where, bool is_internal) const;

	bool has_obj_property(String objname, String propname) const;
	bool get_obj_property(String objname, String propname,
	                      String &rv) const;
	bool has_obj_action(String obj, String prop) const;
	bool get_obj_action(String objname, String actname,
	                    String &rv) const;
	String exit_dest(String room, String dir, bool *is_act = NULL) const;
	Common::Array<Common::Array<String> > get_places(String room);

	void set_obj_property(String obj, String prop);
	void set_obj_action(String obj, String act);
	void move(String obj, String dest);
	void goto_room(String room);
	String get_obj_parent(String obj);

	void print_eval(String);
	void print_eval_p(String);
	String eval_string(String s);
	String eval_param(String s) {
		assert(is_param(s));
		return eval_string(param_contents(s));
	}


	void run_script_as(String, String);
	void run_script(String);
	void run_script(String, String &);
	void run_procedure(String);
	void run_procedure(String, Common::Array<String> args);
	String run_function(String);
	String run_function(String, Common::Array<String> args);
	String bad_arg_count(String);

	bool eval_conds(String);
	bool eval_cond(String);
	GeasState state;

	void tick_timers() override;
	v2string get_inventory() override;
	v2string get_room_contents() override;
	v2string get_room_contents(String);
	vstring get_status_vars() override;
	Common::Array<bool> get_valid_exits() override;


	inline void print_formatted(String s) const {
		if (outputting) gi->print_formatted(s);
	}
	inline void print_normal(String s) const {
		if (outputting) gi->print_normal(s);
	}
	inline void print_newline() const {
		if (outputting) gi->print_newline();
	}

	/*
	inline void print_formatted (String s) const {
	  if (outputting)
	    gi->print_formatted(s);
	  else
	    gi->print_formatted ("{{" + s + "}}");
	}
	inline void print_normal (String s) const
	{
	  if (outputting)
	    gi->print_normal (s);
	  else
	    gi->print_normal("{{" + s + "}}");
	}
	inline void print_newline() const {
	  if (outputting)
	    gi->print_newline();
	  else
	    gi->print_normal ("{{|n}}");
	}
	*/
};

} // End of namespace Quest
} // End of namespace Glk

#endif
