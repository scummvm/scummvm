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

#ifndef NUVIE_CORE_CONVERSE_INTERPRET_H
#define NUVIE_CORE_CONVERSE_INTERPRET_H

#include "ultima/nuvie/core/converse.h"

namespace Ultima {
namespace Nuvie {

using Std::string;
using Std::stack;
using Std::vector;

/* Control and value opcodes for op() & evop() (U6) */
#define U6OP_GT         0x81
#define U6OP_GE         0x82
#define U6OP_LT         0x83
#define U6OP_LE         0x84
#define U6OP_NE         0x85
#define U6OP_EQ         0x86
#define U6OP_ADD        0x90
#define U6OP_SUB        0x91
#define U6OP_MUL        0x92
#define U6OP_DIV        0x93
#define U6OP_LOR        0x94
#define U6OP_LAND       0x95
#define U6OP_CANCARRY   0x9a
#define U6OP_WEIGHT     0x9b
#define U6OP_HORSED     0x9d
#define U6OP_HASOBJ     0x9f
#define U6OP_RAND       0xa0
#define U6OP_EVAL       0xa7
#define U6OP_FLAG       0xab
#define U6OP_VAR        0xb2
#define U6OP_SVAR       0xb3
#define U6OP_DATA       0xb4
#define U6OP_OBJCOUNT   0xbb
#define U6OP_INPARTY    0xc6
#define U6OP_OBJINPARTY 0xc7
#define U6OP_JOIN       0xca
#define U6OP_LEAVE      0xcc
#define U6OP_NPCNEARBY  0xd7
#define U6OP_WOUNDED    0xda
#define U6OP_POISONED   0xdc
#define U6OP_NPC        0xdd
#define U6OP_EXP        0xe0
#define U6OP_LVL        0xe1
#define U6OP_STR        0xe2
#define U6OP_INT        0xe3
#define U6OP_DEX        0xe4

#define U6OP_HORSE     0x9c
#define U6OP_SLEEP     0x9e
#define U6OP_IF        0xa1
#define U6OP_ENDIF     0xa2
#define U6OP_ELSE      0xa3
#define U6OP_SETF      0xa4
#define U6OP_CLEARF    0xa5
#define U6OP_DECL      0xa6
#define U6OP_ASSIGN    0xa8
#define U6OP_JUMP      0xb0
#define U6OP_DPRINT    0xb5
#define U6OP_BYE       0xb6
#define U6OP_INDEXOF   0xb7
#define U6OP_NEW       0xb9
#define U6OP_DELETE    0xba
#define U6OP_INVENTORY 0xbe
#define U6OP_PORTRAIT  0xbf
#define U6OP_ADDKARMA  0xc4
#define U6OP_SUBKARMA  0xc5
#define U6OP_GIVE      0xc9
#define U6OP_WAIT      0xcb
#define U6OP_WORKTYPE  0xcd
#define U6OP_RESURRECT 0xd6
#define U6OP_SETNAME   0xd8
#define U6OP_HEAL      0xd9
#define U6OP_CURE      0xdb
#define U6OP_ENDANSWER 0xee
#define U6OP_KEYWORDS  0xef
#define U6OP_SLOOK     0xf1
#define U6OP_SCONVERSE 0xf2
#define U6OP_SPREFIX   0xf3
#define U6OP_ANSWER    0xf6
#define U6OP_ASK       0xf7
#define U6OP_ASKC      0xf8
#define U6OP_INPUTSTR  0xf9
#define U6OP_INPUT     0xfb
#define U6OP_INPUTNUM  0xfc
#define U6OP_SIDENT    0xff

#define U6OP_ENDDATA   0xb8

#define MDOP_MISC_ACTION  0xd1

/* Script is executed as it is stepped through byte-by-byte, and can have
 * text, data, and control codes. Flow is controlled by run-level stack.
 */
class ConverseInterpret {
protected:
	Converse *converse; // to get data from container

	bool is_waiting; // return control to Converse, paused waiting for something
	bool stopped; // conversation will end, after control returns to Converse
	uint8 answer_mode; // should a response has been triggered by input?
#define ANSWER_NO   0 /* keywords don't match */
#define ANSWER_YES  1 /* keywords match */
#define ANSWER_DONE 2 /* already answered */

	// input values (from the script)
	struct in_val_s {
		converse_value v; // data
		uint8 d; // data-size or 0x00
	};
	// ONE data item from a converse script db
	struct converse_db_s {
		uint8 type; // 0=s 1=i
		char *s;
		uint8 i;
	};
	// frame around blocks of code that may or may not execute
	struct convi_frame_s {
		uint32 start;
		converse_value start_c; // enter on c
		bool run; // run(true) or skip(false) instructions
		converse_value break_c; // will toggle run setting
	};

	vector<struct in_val_s> in; // control values (input/instruction)
	uint32 in_start;
	string text; // input text from script
	vector<Std::string> rstrings; // string value(s) returned by op
	string ystring; // modified by SETNAME, accessed with "$Y"
	uint8 decl_v; // declared/initialized variable number
	uint8 decl_t; // declared variable type: 0x00=none,0xb2=int,0xb3=string
	stack<struct convi_frame_s *> *b_frame;

	bool db_lvar;
	converse_value db_loc;
	converse_value db_offset;


	const char *get_rstr(uint32 sn) {
		return ((sn < rstrings.size()) ? rstrings[sn].c_str() : "");
	}
	const char *get_ystr()          {
		return (ystring.c_str());
	}
	void set_ystr(const char *s);
	void set_rstr(uint32 sn, const char *s);
	converse_value add_rstr(const char *s);

	void let(uint8 v, uint8 t) {
		decl_v = v;
		decl_t = t;
	}
	void let()                 {
		decl_v = decl_t = 0x00;
	}

	void enter(converse_value c);
	void leave();
	void leave_all() {
		while (b_frame && !b_frame->empty()) leave();
	}
	struct convi_frame_s *top_frame() {
		return ((b_frame && !b_frame->empty()) ? b_frame->top() : NULL);
	}
	void do_frame(converse_value c);

	void set_break(converse_value c) {
		if (top_frame()) top_frame()->break_c = c;
	}
	converse_value get_break()       {
		return (top_frame() ? top_frame()->break_c : 0x00);
	}
	void clear_break()               {
		set_break(0x00);
	}
	void set_run(bool r) {
		if (top_frame()) top_frame()->run = r;
	}
	bool get_run()       {
		return (top_frame() ? top_frame()->run : true);
	}

public:
	ConverseInterpret(Converse *owner);
	virtual ~ConverseInterpret();

	bool waiting() {
		return (is_waiting);
	}
	void wait()    {
		is_waiting = true;
	}
	void unwait()  {
		is_waiting = false;
	}
	void stop()    {
		stopped = true;
		wait();
	}
	bool end()     {
		return (stopped);
	}

	void step();

protected:
	/* collecting from script */
	virtual void collect_input();
	virtual struct in_val_s read_value();
	void eval(uint32 vi = 0);

	void add_val(converse_value c, uint8 d = 0);
	void add_text(unsigned char c = 0);

	/* manipulating collected input */
	uint32 val_count()     {
		return (in.size());
	}
	converse_value get_val(uint32 vi);
	uint8 get_val_size(uint32 vi);
	converse_value pop_val();
	uint8 pop_val_size();
	const Std::string &get_text() {
		return text;
	}
	void flush() {
		in.resize(0);
		in_start = 0;
		text.resize(0);
	}

	/* operating on input */
	void exec();
	void do_ctrl();
	void do_text();
	string get_formatted_text(const char *c_str);
	converse_value pop_arg(stack<converse_typed_value> &vs);
	converse_typed_value pop_typed_arg(stack<converse_typed_value> &vs);
	virtual bool evop(stack<converse_typed_value> &i);
	virtual bool op(stack<converse_typed_value> &i);

	virtual bool op_create_new(stack<converse_typed_value> &i);

	converse_value evop_eq(stack<converse_typed_value> &vs);

public:
	virtual uint8 npc_num(uint32 n);//uint8 npc_num(uint32 n){return((n!=0xeb)?n:converse->npc_num);}
	bool check_keywords(Std::string keystr, Std::string instr);
	bool var_input() {
		return (decl_t != 0x00);
	}
	void assign_input(); // set declared variable to Converse input
	struct converse_db_s *get_db(uint32 loc, uint32 i);
	converse_value get_db_integer(uint32 loc, uint32 i);
	void set_db_integer(uint32 loc, uint32 i, converse_value val);
	char *get_db_string(uint32 loc, uint32 i);
	converse_value find_db_string(uint32 loc, const char *dstring);

	/* value tests */
	virtual bool is_print(converse_value check) {
		return (((check == 0x0a) || (check >= 0x20 && check <= 0x7a) || (check == 0x7e) || (check == 0x7b)));    //added '~' 0x7e, '{' 0x7b  for fm towns.
	}
	virtual bool is_ctrl(converse_value code) {
		return (((code >= 0xa1 || code == 0x9c || code == 0x9e) && !is_valop(code) && !is_datasize(code)));
	}
	virtual bool is_datasize(converse_value check) {
		return ((check == 0xd3 || check == 0xd2 || check == 0xd4));
	}
	virtual bool is_valop(converse_value check) {
		return (((check == 0x81) || (check == 0x82) || (check == 0x83)
		         || (check == 0x84) || (check == 0x85) || (check == 0x86)
		         || (check == 0x90) || (check == 0x91) || (check == 0x92)
		         || (check == 0x93) || (check == 0x94) || (check == 0x95)
		         || (check == 0x9a) || (check == 0x9b) || (check == 0x9d)
		         || (check == 0x9f) || (check == 0xa0) || (check == 0xa7)
		         || (check == 0xab) || (check == 0xb2) || (check == 0xb3)
		         || (check == 0xb4) || (check == 0xb7) || (check == 0xbb) || (check == 0xc6)
		         || (check == 0xc7) || (check == 0xca) || (check == 0xcc)
		         || (check == 0xd7) || (check == 0xda) || (check == 0xdc)
		         || (check == 0xdd) || (check == 0xe0) || (check == 0xe1)
		         || (check == 0xe2) || (check == 0xe3) || (check == 0xe4)));
	}
	const char *evop_str(converse_value op);
	const char *op_str(converse_value op);
};


class U6ConverseInterpret : public ConverseInterpret {
public:
	U6ConverseInterpret(Converse *owner) : ConverseInterpret(owner) { }
//    ~U6ConverseInterpret();
};

class WOUConverseInterpret : public ConverseInterpret {
public:
	WOUConverseInterpret(Converse *owner) : ConverseInterpret(owner) { }

protected:
	bool op_create_new(stack<converse_typed_value> &i) override;
};

class SETalkInterpret : public ConverseInterpret {
public:
	SETalkInterpret(Converse *owner) : ConverseInterpret(owner) { }
};


class MDTalkInterpret : public WOUConverseInterpret {
public:
	MDTalkInterpret(Converse *owner) : WOUConverseInterpret(owner) { }
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
