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

void Serializer::sync(bool &b) {
	byte v = b ? 1 : 0;
	syncAsByte(v);
	if (isLoading())
		b = v != 0;
}

void Serializer::sync(String &s) {
	Common::String str = s;
	Common::Serializer::syncString(str);
	if (isLoading())
		s = String(str.c_str());
}
	
void Serializer::sync(PropertyRecord &pr) {
	sync(pr.name);
	sync(pr.data);
}

void Serializer::sync(ObjectRecord &pr) {
	sync(pr.name);
	sync(pr.hidden);
	sync(pr.invisible);
	sync(pr.parent);
}

void Serializer::sync(ExitRecord &er) {
	sync(er.src);
	sync(er.dest);
}

void Serializer::sync(TimerRecord &tr) {
	sync(tr.name);
	sync(tr.is_running);
	syncAsUint32LE(tr.interval);
	syncAsUint32LE(tr.timeleft);
}

void Serializer::sync(SVarRecord &svr) {
	svr.sync(*this);
}

void Serializer::sync(IVarRecord &ivr) {
	ivr.sync(*this);
}

void Serializer::sync(GeasState &gs) {
	sync(gs.location);
	sync(gs.props);
	sync(gs.objs);
	sync(gs.exits);
	sync(gs.timers);
	sync(gs.svars);
	sync(gs.ivars);
}

/*----------------------------------------------------------------------*/

void SVarRecord::sync(Serializer &s) {
	s.sync(name);
	
	uint count = data.size();
	s.syncAsUint32LE(count);
	if (s.isLoading())
		data.resize(count);

	for (uint i = 0; i < size(); ++i)
		s.sync(data[i]);
}

/*----------------------------------------------------------------------*/

void IVarRecord::sync(Serializer &s) {
	s.sync(name);

	uint count = data.size();
	s.syncAsUint32LE(count);
	if (s.isLoading())
		data.resize(count);

	for (uint i = 0; i < size(); ++i)
		s.syncAsSint32LE(data[i]);
}

/*----------------------------------------------------------------------*/

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

void GeasState::load(Common::SeekableReadStream *rs) {
	Serializer s(rs, nullptr);
	s.sync(*this);
}

void GeasState::save(Common::WriteStream *ws) {
	Serializer s(nullptr, ws);
	s.sync(*this);
}

/*----------------------------------------------------------------------*/

Common::WriteStream &operator<<(Common::WriteStream &o, const PropertyRecord &pr) {
	o << pr.name << ", data == " << pr.data;
	return o;
}

Common::WriteStream &operator<<(Common::WriteStream &o, const ObjectRecord &objr) {
	o << objr.name << ", parent == " << objr.parent;
	if (objr.hidden)
		o << ", hidden";
	if (objr.invisible)
		o << ", invisible";
	return o;
}

Common::WriteStream &operator<<(Common::WriteStream &o, const ExitRecord er) {
	return o << er.src << ": " << er.dest;
}

Common::WriteStream &operator<<(Common::WriteStream &o, const TimerRecord &tr) {
	return o << tr.name << ": " << (tr.is_running ? "" : "not ") << "running ("
		<< tr.timeleft << " // " << tr.interval << ")";
}

Common::WriteStream &operator<<(Common::WriteStream &o, const SVarRecord &sr) {
	o << sr.name << ": ";
	if (sr.size() == 0)
		o << "(empty)";
	else if (sr.size() <= 1)
		o << "<" << sr.get(0) << ">";
	else
		for (uint i = 0; i < sr.size(); i++) {
			o << i << ": <" << sr.get(i) << ">";
			if (i + 1 < sr.size())
				o << ", ";
		}
	return o;
}

Common::WriteStream &operator<<(Common::WriteStream &o, const IVarRecord &ir) {
	o << ir.name << ": ";
	if (ir.size() == 0)
		o << "(empty)";
	else if (ir.size() <= 1)
		o << ir.get(0);
	else
		for (uint i = 0; i < ir.size(); i++) {
			o << i << ": " << ir.get(i);
			if (i + 1 < ir.size())
				o << ", ";
		}
	return o;
}

Common::WriteStream &operator<<(Common::WriteStream &o, const GeasState &gs) {
	o << "location == " << gs.location << "\nprops: \n";

	for (uint i = 0; i < gs.props.size(); i++)
		o << "    " << i << ": " << gs.props[i] << "\n";

	o << "objs:\n";
	for (uint i = 0; i < gs.objs.size(); i++)
		o << "    " << i << ": " << gs.objs[i] << "\n";

	o << "exits:\n";
	for (uint i = 0; i < gs.exits.size(); i++)
		o << "    " << i << ": " << gs.exits[i] << "\n";

	o << "timers:\n";
	for (uint i = 0; i < gs.timers.size(); i++)
		o << "    " << i << ": " << gs.timers[i] << "\n";

	o << "String variables:\n";
	for (uint i = 0; i < gs.svars.size(); i++)
		o << "    " << i << ": " << gs.svars[i] << "\n";

	o << "integer variables:\n";
	for (uint i = 0; i < gs.svars.size(); i++)
		o << "    " << i << ": " << gs.svars[i] << "\n";

	return o;
}

} // End of namespace Quest
} // End of namespace Glk
