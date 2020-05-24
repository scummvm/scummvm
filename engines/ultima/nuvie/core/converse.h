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

#ifndef NUVIE_CORE_CONVERSE_H
#define NUVIE_CORE_CONVERSE_H

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/views/view.h"

namespace Ultima {
namespace Nuvie {

class Actor;
class ActorManager;
class Configuration;
class MsgScroll;
class GameClock;
class ObjManager;
class Player;
class ViewManager;
class U6Lib_n;
class U6Lzw;

class ConverseInterpret;
class ConverseSpeech;
class ConvScript;

using Std::string;


#define CONVERSE_GUMP_DEFAULT   0
#define CONVERSE_GUMP_U7_STYLE  1
#define CONVERSE_GUMP_WOU_STYLE 2

uint8 get_converse_gump_type_from_config(Configuration *config);

typedef uint32 converse_value; // any single value read from a script
typedef unsigned char *convscript_buffer;

typedef struct {
	uint8 type;
	converse_value val;
} converse_typed_value;

#define U6TALK_VAR_SEX       0x10 // sex of avatar: male=0 female=1
#define U6TALK_VAR_KARMA     0x14 // avatar's karma
#define U6TALK_VAR_GARGF     0x15 // 1=player knows Gargish
#define U6TALK_VAR_NPC_NAME  0x17
#define U6TALK_VAR_PARTYLIVE 0x17 // number of people (living) following avatar
#define U6TALK_VAR_PARTYALL  0x18 // number of people (total) following avatar
#define U6TALK_VAR_HP        0x19 // avatar's health
#define U6TALK_VAR_PLAYER_NAME 0x19
#define U6TALK_VAR_QUESTF    0x1A // 0="Thou art not upon a sacred quest!"
#define WOUTALK_VAR_ADD_TO_INVENTORY_FAILED 0x1D
#define U6TALK_VAR_WORKTYPE  0x20 // current activity of npc, from schedule
#define U6TALK_VAR_YSTRING   0x22 // value of $Y variable.
#define U6TALK_VAR_INPUT     0x23 // previous input from player ($Z)
#define U6TALK_VAR__LAST_ 0x25    // (all above 36 appear uninitialized)

/* Conversation engine, apart from the interpreter. Loads converse files,
 * and reads script into buffer. Also manages input/output and has npc-related
 * support functions. This class handles all game types.
 */
class Converse {
	friend class ConverseInterpret;
	friend class SETalkInterpret;
	friend class MDTalkInterpret;
	friend class WOUConverseInterpret;
	friend class U6ConverseInterpret;

	// game system objects from nuvie
	Configuration *config;
	GameClock *clock;
	ActorManager *actors;
	ObjManager *objects;
	Player *player;
	ViewManager *views;
	MsgScroll *scroll; // i/o

	nuvie_game_t gametype; // what game is being played?
	U6Lib_n *src;
	uint8 src_num; // identify source file: 0=unset/unused
	const char *src_name();

	ConverseInterpret *conv_i; // interpreter
	ConvScript *script;
	View *last_view;
	Actor *npc;
	uint8 npc_num;
	uint8 script_num; //this could differ from npc_num when talking to guards or wisps etc.
	Std::string _name, _desc;

	bool active; // running npc script? (either paused or unpaused)
	bool need_input; // waiting for text input
	bool party_all_the_time; // force NPCs to join player's party?

	string in_str; // last input from player
	string out_str; // text that is to be printed
	char *allowed_input; // characters requested for single-character input

	char aname[16]; // return from npc_name()
	struct converse_variables_s {
		converse_value cv;
		char *sv;
	} *variables; /* initialized for [U6TALK_VAR__LAST_+1] items */

	ConverseSpeech *speech;
	bool using_fmtowns;

	void reset();

public:
	Converse();
	~Converse();
	void init(Configuration *cfg, nuvie_game_t t, MsgScroll *s, ActorManager *a,
	          GameClock *c, Player *p, ViewManager *v, ObjManager *o);

	uint32 get_script_num(uint8 a);
	void load_conv(const Std::string &convfilename);
	uint32 load_conv(uint8 a);
	void unload_conv() {
		delete src;
		src = NULL;
	}
	ConvScript *load_script(uint32 n);
	ConverseInterpret *new_interpreter();

	bool start(Actor *a) {
		return (start(a->get_actor_num()));
	}
	bool start(uint8 n);
	void continue_script();
	void stop();

	bool running()    {
		return (active);
	}
	bool is_waiting_for_scroll() {
		return scroll->get_page_break();
	}
	void unwait();
	void poll_input(const char *allowed = NULL, bool nonblock = true);
	bool override_input();
	void collect_input();

	bool input();
	void print(const char *s = NULL);
	const Std::string &get_input()  {
		return in_str;
	}
	const Std::string &get_output() {
		return out_str;
	}
	void set_input(Std::string s) {
		in_str = s;
	}
	void set_output(Std::string s) {
		out_str = s;
	}

	void set_party_all_the_time(bool val) {
		party_all_the_time = val;
	}
	const char *npc_name(uint8 num);
	void show_portrait(uint8 n);
	converse_value get_var(uint8 varnum) {
		return (varnum <= U6TALK_VAR__LAST_ ? variables[varnum].cv : 0x00);
	}
	const char *get_svar(uint8 varnum);
	void set_var(uint8 varnum, uint32 val) {
		if (varnum <= U6TALK_VAR__LAST_) variables[varnum].cv = val;
	}
	void set_svar(uint8 varnum, const char *set);
	void init_variables();
	void delete_variables();

	ConverseSpeech *get_speech() {
		return speech;
	};

	bool conversations_stop_music;
private:
	void print_prompt();
};


/* Conversation script container. Maintains current position in the script. The
 * object only exists if it has data loaded. Different classes with an identical
 * interface can be created to handle different games' file formats.
 */
class ConvScript {
	friend class Converse;

	convscript_buffer buf;
	uint32 buf_len;
	convscript_buffer buf_pt; // pointer into script (current location)

	U6Lib_n *src;
	uint32 src_index;
	bool compressed; // was the original file (LZW) compressed?

	uint8 ref; // Multiple objects can use the same buffer
	ConvScript *cpy;

public:
	ConvScript(U6Lib_n *s, uint32 idx);
	ConvScript(ConvScript *orig);
	~ConvScript();

	void read_script();
	bool loaded() {
		return ((buf && buf_len));    // script is loaded?
	}

	/* Reading */
	converse_value read(uint32 advance = 1);
	converse_value read2();
	converse_value read4();
	converse_value peek(uint32 displacement = 0) {
		return ((converse_value) * (buf_pt + displacement));
	}

	/* Writing */
	void write2(converse_value val);

	/* Seeking - update script pointer */
	void rewind() {
		buf_pt = buf;
	}
	void skip(uint32 bytes = 1) {
		buf_pt += bytes;
	}
	void seek(uint32 offset = 0) {
		rewind();
		skip(offset);
	}

	uint32 pos()                    {
		return (buf_pt - buf);
	}
	bool overflow(uint32 ptadd = 0) {
		return (((pos() + ptadd) >= buf_len));
	}
	convscript_buffer get_buffer(uint32 ptadd = 0) {
		return ((!ptadd || (ptadd < buf_len)) ? buf + ptadd : NULL);
	}
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
