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

#include "glk/quest/geas_state.h"
#include "glk/quest/geas_runner.h"
#include "glk/quest/geas_util.h"
#include "glk/quest/read_file.h"
#include "glk/quest/streams.h"

namespace Glk {
namespace Quest {

Common::WriteStream &operator<< (Common::WriteStream &o, const StringMap &m) {
	for (StringMap::iterator i = m.begin(); i != m.end(); ++i)
		o << (*i)._key << " -> " << (*i)._value << "\n";

	return o;
}

class GeasOutputStream {
	Common::WriteStream *_ws;
public:
	GeasOutputStream &put(const String &s) {
		_ws->writeString(s);
		_ws->writeByte(0);
		return *this;
	}
	GeasOutputStream &put(char ch) {
		_ws->writeByte(ch);
		return *this;
	}
	GeasOutputStream &put(int i) {
		Common::String s = Common::String::format("%d", i);
		_ws->writeString(s);
		_ws->writeByte(0);
		return *this;
	}
	GeasOutputStream &put(uint i) {
		Common::String s = Common::String::format("%u", i);
		_ws->writeString(s);
		_ws->writeByte(0);
		return *this;
	}
	GeasOutputStream &put(unsigned long i) {
		Common::String s = Common::String::format("%lu", i);
		_ws->writeString(s);
		_ws->writeByte(0);
		return *this;
	}

	void write_out(const String &gameName, const String &saveName) {
#ifdef TODO
		ofstream ofs;
		ofs.open(savename.c_str());
		if (!ofs.is_open())
			error("Unable to open \"%s\"", savename.c_str());
		ofs << "QUEST300" << char(0) << gamename << char(0);
		String tmp = o.str();
		for (uint i = 0; i < tmp.size(); i ++)
			ofs << char (255 - tmp[i]);
		cerr << "Done writing save game\n";
#else
		error("TODO");
#endif
	}
};

template <class T> void write_to(GeasOutputStream &gos, const Common::Array<T> &v) {
	gos.put(v.size());
	for (uint i = 0; i < v.size(); i ++)
		write_to(gos, v[i]);
}

void write_to(GeasOutputStream &gos, const PropertyRecord &pr) {
	gos.put(pr.name).put(pr.data);
}

void write_to(GeasOutputStream &gos, const ObjectRecord &pr) {
	gos.put(pr.name).put(char(pr.hidden ? 0 : 1))
	.put(char(pr.invisible ? 0 : 1)).put(pr.parent);
}

void write_to(GeasOutputStream &gos, const ExitRecord &er) {
	gos.put(er.src).put(er.dest);
}

void write_to(GeasOutputStream &gos, const TimerRecord &tr) {
	gos.put(tr.name).put(tr.is_running ? 0 : 1).put(tr.interval)
	.put(tr.timeleft);
}


void write_to(GeasOutputStream &gos, const SVarRecord &svr) {
	gos.put(svr.name);
	gos.put(svr.max());
	for (uint i = 0; i < svr.size(); i ++)
		gos.put(svr.get(i));
}

void write_to(GeasOutputStream &gos, const IVarRecord &ivr) {
	gos.put(ivr.name);
	gos.put(ivr.max());
	for (uint i = 0; i < ivr.size(); i ++)
		gos.put(ivr.get(i));
}

void write_to(GeasOutputStream &gos, const GeasState &gs) {
	gos.put(gs.location);
	write_to(gos, gs.props);
	write_to(gos, gs.objs);
	write_to(gos, gs.exits);
	write_to(gos, gs.timers);
	write_to(gos, gs.svars);
	write_to(gos, gs.ivars);
}

void save_game_to(String gamename, String savename, const GeasState &gs) {
	GeasOutputStream gos;
	write_to(gos, gs);
	gos.write_out(gamename, savename);
}

GeasState::GeasState(GeasInterface &gi, const GeasFile &gf) {
	running = false;

	cerr << "GeasState::GeasState()" << endl;
	for (uint i = 0; i < gf.size("game"); i ++) {
		//const GeasBlock &go = gf.game[i];
		//register_block ("game", "game");
		ObjectRecord data;
		data.name = "game";
		data.parent = "";
		data.hidden = false;
		data.invisible = true;
		objs.push_back(data);
	}

	cerr << "GeasState::GeasState() done setting game" << endl;
	for (uint i = 0; i < gf.size("room"); i ++) {
		const GeasBlock &go = gf.block("room", i);
		ObjectRecord data;
		//data.name = go.lname;
		data.name = go.name;
		data.parent = "";
		data.hidden = data.invisible = true;
		//register_block (data.name, "room");
		objs.push_back(data);
	}

	cerr << "GeasState::GeasState() done setting rooms" << endl;
	for (uint i = 0; i < gf.size("object"); i++) {
		const GeasBlock &go = gf.block("object", i);
		ObjectRecord data;
		//data.name = go.lname;
		data.name = go.name;
		if (go.parent == "")
			data.parent = "";
		else
			//data.parent = lcase (param_contents (go.parent));
			data.parent = param_contents(go.parent);
		//register_block (data.name, "object");
		data.hidden = data.invisible = false;
		objs.push_back(data);
	}

	cerr << "GeasState::GeasState() done setting objects" << endl;
	for (uint i = 0; i < gf.size("timer"); i ++) {
		const GeasBlock &go = gf.block("timer", i);
		//cerr << "GS::GS: Handling timer " << go << "\n";
		TimerRecord tr;
		String interval = "", status = "";
		for (uint j = 0; j < go.data.size(); j ++) {
			String line = go.data[j];
			uint c1, c2;
			String tok = first_token(line, c1, c2);
			if (tok == "interval") {
				tok = next_token(line, c1, c2);
				if (!is_param(tok))
					gi.debug_print(nonparam("interval", line));
				else
					interval = param_contents(tok);
			} else if (tok == "enabled" || tok == "disabled") {
				if (status != "")
					gi.debug_print("Repeated status for timer");
				else
					status = tok;
			} else if (tok == "action") {
			} else {
				gi.debug_print("Bad timer line " + line);
			}
		}
		//tr.name = go.lname;
		tr.name = go.name;
		tr.is_running = (status == "enabled");
		tr.interval = tr.timeleft = parse_int(interval);
		//register_block (tr.name, "timer");
		timers.push_back(tr);
	}

	cerr << "GeasState::GeasState() done with timers" << endl;
	for (uint i = 0; i < gf.size("variable"); i ++) {
		const GeasBlock &go(gf.block("variable", i));
		cerr << "GS::GS: Handling variable #" << i << ": " << go << endl;
		String vartype;
		String value;
		for (uint j = 0; j < go.data.size(); j ++) {
			String line = go.data[j];
			cerr << "   Line #" << j << " of var: \"" << line << "\"" << endl;
			uint c1, c2;
			String tok = first_token(line, c1, c2);
			if (tok == "type") {
				tok = next_token(line, c1, c2);
				if (tok == "")
					gi.debug_print(String("Missing variable type in ")
					               + string_geas_block(go));
				else if (vartype != "")
					gi.debug_print(String("Redefining var. type in ")
					               + string_geas_block(go));
				else if (tok == "numeric" || tok == "String")
					vartype = tok;
				else
					gi.debug_print(String("Bad var. type ") + line);
			} else if (tok == "value") {
				tok = next_token(line, c1, c2);
				if (!is_param(tok))
					gi.debug_print(String("Expected parameter in " + line));
				else
					value = param_contents(tok);
			} else if (tok == "display" || tok == "onchange") {
			} else {
				gi.debug_print(String("Bad var. line: ") + line);
			}
		}
		if (vartype == "" || vartype == "numeric") {
			IVarRecord ivr;
			//ivr.name = go.lname;
			ivr.name = go.name;
			ivr.set(0, parse_int(value));
			ivars.push_back(ivr);
			//register_block (ivr.name, "numeric");
		} else {
			SVarRecord svr;
			//svr.name = go.lname;
			svr.name = go.name;
			svr.set(0, value);
			svars.push_back(svr);
			//register_block (svr.name, "String");
		}
	}
	//cerr << obj_types << endl;
	cerr << "GeasState::GeasState() done with variables" << endl;
}

Common::WriteStream &operator<< (Common::WriteStream &o, const PropertyRecord &pr) {
	o << pr.name << ", data == " << pr.data;
	return o;
}

Common::WriteStream &operator<< (Common::WriteStream &o, const ObjectRecord &objr) {
	o << objr.name << ", parent == " << objr.parent;
	if (objr.hidden)
		o << ", hidden";
	if (objr.invisible)
		o << ", invisible";
	return o;
}

Common::WriteStream &operator<< (Common::WriteStream &o, const ExitRecord er) {
	return o << er.src << ": " << er.dest;
}

Common::WriteStream &operator<< (Common::WriteStream &o, const TimerRecord &tr) {
	return o << tr.name << ": " << (tr.is_running ? "" : "not ") << "running ("
	       << tr.timeleft << " // " << tr.interval << ")";
}

Common::WriteStream &operator<< (Common::WriteStream &o, const SVarRecord &sr) {
	o << sr.name << ": ";
	if (sr.size() == 0)
		o << "(empty)";
	else if (sr.size() <= 1)
		o << "<" << sr.get(0) << ">";
	else
		for (uint i = 0; i < sr.size(); i ++) {
			o << i << ": <" << sr.get(i) << ">";
			if (i + 1 < sr.size())
				o << ", ";
		}
	return o;
}

Common::WriteStream &operator<< (Common::WriteStream &o, const IVarRecord &ir) {
	o << ir.name << ": ";
	if (ir.size() == 0)
		o << "(empty)";
	else if (ir.size() <= 1)
		o << ir.get(0);
	else
		for (uint i = 0; i < ir.size(); i ++) {
			o << i << ": " << ir.get(i);
			if (i + 1 < ir.size())
				o << ", ";
		}
	return o;
}

Common::WriteStream &operator<< (Common::WriteStream &o, const GeasState &gs) {
	o << "location == " << gs.location << "\nprops: \n";

	for (uint i = 0; i < gs.props.size(); i ++)
		o << "    " << i << ": " << gs.props[i] << "\n";

	o << "objs:\n";
	for (uint i = 0; i < gs.objs.size(); i ++)
		o << "    " << i << ": " << gs.objs[i] << "\n";

	o << "exits:\n";
	for (uint i = 0; i < gs.exits.size(); i ++)
		o << "    " << i << ": " << gs.exits[i] << "\n";

	o << "timers:\n";
	for (uint i = 0; i < gs.timers.size(); i ++)
		o << "    " << i << ": " << gs.timers[i] << "\n";

	o << "String variables:\n";
	for (uint i = 0; i < gs.svars.size(); i ++)
		o << "    " << i << ": " << gs.svars[i] << "\n";

	o << "integer variables:\n";
	for (uint i = 0; i < gs.svars.size(); i ++)
		o << "    " << i << ": " << gs.svars[i] << "\n";

	return o;
}

} // End of namespace Quest
} // End of namespace Glk
