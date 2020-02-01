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
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/usecode/u6_usecode.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/converse_interpret.h"
#include "ultima/nuvie/core/converse_speech.h"

namespace Ultima {
namespace Nuvie {

//#define CONVERSE_DEBUG

ConverseInterpret::ConverseInterpret(Converse *owner) {
	converse = owner;
	b_frame = NULL;
	decl_v = decl_t = 0x00;
	in_start = 0;

	unwait();
	answer_mode = ANSWER_NO;
	stopped = false;
	db_lvar = false;
	db_loc = 0;
	db_offset = 0;
}


ConverseInterpret::~ConverseInterpret() {
	leave_all(); // deletes b_frame when empty
}


/* Returns a value from the current location in the ConvScript, respecting any
 * present data-size definition.
 */
struct ConverseInterpret::in_val_s ConverseInterpret::read_value() {
	ConvScript *cs = converse->script;
	converse_value nval = cs->read();
	uint8 dtype = nval;
	if (nval == 0xd3)
		nval = cs->read();
	else if (nval == 0xd2)
		nval = cs->read4();
	else if (nval == 0xd4)
		nval = cs->read2();
	else
		dtype = 0;
	struct in_val_s ival;
	ival.v = nval;
	ival.d = dtype;
	return (ival);
}


/* Read a control statement from the script.
 */
void ConverseInterpret::collect_input() {
	ConvScript *cs = converse->script;
	converse_value cval = cs->peek();
	// special cases
	if (cval == U6OP_JUMP) { // 0xb0 (location)
		add_val(cs->read()); // OPC
		add_val(cs->read4());
		return;
	} else if (cval == U6OP_SIDENT) { // 0xff, get NPC identity
		add_val(cs->read()); // OPC
		add_val(cs->read()); // number
		add_text(); // name
		return;
	} else if (cval == U6OP_KEYWORDS // 0xef, get keyword list
	           || cval == U6OP_ASKC   // ...0xf8, or characters allowed as input
	           || cval == U6OP_SLOOK) { // ...0xf1, or description
		add_val(cs->read()); // OPC
		add_text();
		return;
	}
	// standard
	uint32 arg_num = 1; // which arg. of control statement will be evaluated
	do {
		if (is_print(cs->peek()) && (cs->overflow(1) || !is_valop(cs->peek(1))))
			break;
		if (cs->peek() == U6OP_EVAL) { // 0xa7
			cs->skip();
			// HACK forced by this input-collection design, don't eval if not
			// running (but this was supposed to be checked only in exec())
			if (!top_frame() || top_frame()->run) // "no frame" == run
				eval(arg_num++);
		} else {
			struct in_val_s nval = read_value();
			add_val(nval.v, nval.d);
		}
	} while (!cs->overflow() && !is_ctrl(cs->peek()));
}


/* Interpret control codes and text from the ConvScript.
 */
void ConverseInterpret::step() {
	ConvScript *cs = converse->script;

	flush();

	while (!waiting() && !cs->overflow() && !converse->is_waiting_for_scroll()) {
		if (is_print(cs->peek())) {
			add_text(); // collect
			converse->set_output(get_text());
		} else if (is_ctrl(cs->peek())) {
			in_start = cs->pos();
			collect_input(); // get opcode + data (1 statement/instruction)
		} else {
#ifdef CONVERSE_DEBUG
			DEBUG(1, LEVEL_DEBUGGING, "Converse: skipped 0x%02x at %04x\n", cs->peek(), cs->pos());
#endif
			converse->print("[Tried to print a control char.]\n");
			cs->skip();
		}
		exec(); // run it
	}

	if (cs->overflow() && !stopped) {
		converse->print("\n[EOF]\n");
		stop();
	}
}


/* Collect text from script and add to end of collected text statement, or add
 * one character to text.
 */
void ConverseInterpret::add_text(unsigned char c) {
	ConvScript *cs = converse->script;
	do {
		text.append(1, (unsigned char)cs->read());
	} while (!cs->overflow() && is_print(cs->peek()));
}


/* Add some code or data to end of collected control statement.
 */
void ConverseInterpret::add_val(converse_value c, uint8 d) {
	struct in_val_s ivs;
	ivs.v = c;
	ivs.d = d;
	in.push_back(ivs);
}


/* Delete the top-level frame. If no frames are left, run==true for all code.
 */
void ConverseInterpret::leave() {
	if (top_frame()) {
		struct convi_frame_s *fp = b_frame->top();
#ifdef CONVERSE_DEBUG
		DEBUG(1, LEVEL_DEBUGGING, "Converse: ...leave %02x...\n", fp->start_c);
#endif
		delete fp;
		fp = NULL;
		b_frame->pop();
		if (b_frame->empty()) {
			delete b_frame;
			b_frame = NULL;
		}
	}
}


/* Create new top-level frame for a code block starting at control code `c'.
 */
void ConverseInterpret::enter(converse_value c) {
	struct convi_frame_s *ef = new convi_frame_s;
	ef->start = in_start; // start location
	ef->run = top_frame() ? top_frame()->run : true; // run if parent is
	ef->break_c = 0x00;
	ef->start_c = c;
	if (!b_frame)
		b_frame = new stack<struct convi_frame_s *>;
	b_frame->push(ef);
#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, "Converse: ...enter %02x...\n", ef->start_c);
#endif
}


/* Leave the top-frame for end-code matching `c', and enter a new frame for
 * other codes. Toggle run mode if `c' is break-code.
 */
void ConverseInterpret::do_frame(converse_value c) {
	switch (c) {
	case U6OP_IF: // 0xa1
		enter(U6OP_IF);
		break;
	case U6OP_ENDIF: // 0xa2
		leave(); // (0xa1)
		break;
	case U6OP_KEYWORDS: // 0xef
		if (!top_frame() || top_frame()->start_c != U6OP_KEYWORDS)
			enter(U6OP_KEYWORDS); // 0xef...0xef...0xee, can't be recursive
		break;
	case U6OP_ENDANSWER: // 0xee
		leave(); // (0xef)
		break;
	}
	if (c != 0x00 && c == get_break())
		set_run(!get_run());
}


/* Run (or skip) the stored control and text statements, and clear them.
 */
void ConverseInterpret::exec() {
	do_frame(get_val(0));
	if (!top_frame() || top_frame()->run) { // "no frame" == run
		if (val_count())
			do_ctrl();
		if (!converse->get_output().empty())
			do_text();
	}
#ifdef CONVERSE_DEBUG
	else if (get_val(0) != 0x00) // show stepped over ctrl code (not text)
		DEBUG(1, LEVEL_DEBUGGING, "Converse: %04x ----: %02x\n", in_start, get_val(0));
#endif
	flush();
	converse->set_output(""); // clear output
}


/* Print script text, resolving special symbols as they are encountered.
 */
void ConverseInterpret::do_text() {
	string output = get_formatted_text(converse->get_output().c_str());
	converse->print(output.c_str());
}

/* Print script text, resolving special symbols as they are encountered.
 */
string ConverseInterpret::get_formatted_text(const char *c_str) {
	unsigned int i = 0;
	char symbol[3] = { '\0', '\0', '\0' },
	                 intval[16];
	string output;
	const uint32 len = strlen(c_str);
	uint32 last_value = 0;

	while (i < len) {
		switch (c_str[i]) {

		case '$' :
		case '#' : // copy symbol
			strncpy(symbol, &c_str[i], 2);
			// evaluate
			if (!strcmp(symbol, "$G")) // gender title
				output.append(converse->player->get_gender_title());
			else if (!strcmp(symbol, "$N")) // npc name
				output.append(converse->_name.c_str());
			else if (!strcmp(symbol, "$P")) // player name
				output.append(converse->player->get_name());
			else if (!strcmp(symbol, "$T")) // time of day
				output.append(converse->clock->get_time_of_day_string());
			else if (!strcmp(symbol, "$Y")) // Y-string
				output.append(get_ystr());
			else if (!strcmp(symbol, "$Z")) // previous input
				output.append(converse->get_svar(U6TALK_VAR_INPUT));
			else if (symbol[0] == '$' // value of a string variable
			         && Common::isDigit(symbol[1]))
				output.append(converse->get_svar(strtol(&symbol[1], NULL, 10)));
			else if (symbol[0] == '#' // value of a variable
			         && Common::isDigit(symbol[1])) {
				last_value = converse->get_var(strtol(&symbol[1], NULL, 10));
				snprintf(intval, 16, "%u", last_value);

				output.append((char *)intval);
				output.append("");
			} else
				output.append(symbol);

			i += 2;
			break;

		case '{' :
			i++;
			break; //FIXME what does this FM-Towns command do?

		case '~' :
			if (i + 3 <= len) {
				i++;
				if (c_str[i] == 'P')
					converse->get_speech()->play_speech(converse->script_num, (int)strtol(&c_str[i + 1], NULL, 10));

				for (i++; Common::isDigit(c_str[i]) && i < len;)
					i++;
			}
			break;

		case '/' : // singular
		case '\\' : { // plural
			bool show = false;
			if ((last_value == 1 && c_str[i] == '/') ||
			        (last_value != 1 && c_str[i] == '\\')) show = true;
			++i;
			unsigned int count = 0;
			while (i + count < len &&
			        c_str[i + count] >= 'a' && c_str[i + count] <= 'z')
				// NOTE: the above check might not work for non-english
				count++;
			if (show) output.append(count, c_str[i]);
			i += count;
			break;
		}

		default :
			//skip fm-towns keyword commands when in original ui mode.
			// They take the following form. '+actor_numberKeyword+' eg. '+3leave+'
			if (c_str[i] == '+' && !Game::get_game()->is_new_style()) {
				if (i + 3 <= len) {
					i++;

					for (; c_str[i] != 0x2b && i < len;) //0x2b = '+'
						i++;

					i++;
				}
			} else {
				output.append(1, c_str[i]);
				i += 1;
			}
			break;
		}
	}

	return output;
}


/* Execute a control statement/instruction (control code + optional arguments
 * which must have been evaluated already.)
 */
void ConverseInterpret::do_ctrl() {
	stack<converse_typed_value> st;
#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, "Converse: %04x INSTR:", in_start);
	for (uint32 v = 0; v < val_count(); v++)
		DEBUG(1, LEVEL_DEBUGGING, " 0x%02x", get_val(v));
	DEBUG(1, LEVEL_DEBUGGING, "\n");
#endif

	converse_typed_value v = {U6OP_VAR, 0};
	while (val_count()) {
		v.val = pop_val();
		st.push(v);
	}
	op(st);
}


/* Returns last item value from input list (and removes it from the same.)
 */
converse_value ConverseInterpret::pop_val() {
	converse_value ret = 0;
	if (!in.empty()) {
		ret = get_val(val_count() - 1);
		in.resize(val_count() - 1);
	}
	return (ret);
}


/* Returns last item data-size/type from value list (and removes it from the
 * same.)
 */
uint8 ConverseInterpret::pop_val_size() {
	converse_value ret = 0;
	if (!in.empty()) {
		ret = get_val_size(val_count() - 1);
		in.resize(val_count() - 1);
	}
	return (ret);
}


/* Returns any item value from the input list (and leaves the list alone.)
 */
converse_value ConverseInterpret::get_val(uint32 vi) {
	if (vi < in.size())
		return (in[vi].v);
	return (0);
}


/* Returns any item data-size/type from the input list (and leaves the list
 * alone.)
 */
uint8 ConverseInterpret::get_val_size(uint32 vi) {
	if (vi < in.size())
		return (in[vi].d);
	return (0);
}

void ConverseInterpret::set_ystr(const char *s) {
	ystring = s ? s : "";
	converse->set_svar(U6TALK_VAR_YSTRING, ystring.c_str());
}

/* Set the return string `sn' to `s'.
 */
void ConverseInterpret::set_rstr(uint32 sn, const char *s) {
	if (sn >= rstrings.size())
		rstrings.resize(rstrings.size() + 1);
	rstrings[sn] = s ? s : "";
}


/* Add a return string.
 * Returns the index into the list, which is used by an instruction.
 */
converse_value ConverseInterpret::add_rstr(const char *s) {
	rstrings.push_back(s ? s : "");
	return (rstrings.size() - 1);
}


/* Returns and removes top item from a value stack.
 */
converse_value ConverseInterpret::pop_arg(stack<converse_typed_value> &vs) {
	converse_value ret = 0;
	if (!vs.empty()) {
		converse_typed_value val = vs.top();
		ret = val.val;
		vs.pop();
	}
	return (ret);
}

converse_typed_value ConverseInterpret::pop_typed_arg(stack<converse_typed_value> &vs) {
	converse_typed_value ret = {0, 0};
	if (!vs.empty()) {
		ret = vs.top();
		vs.pop();
	}
	return (ret);
}

#if 0
bool MDTalkInterpret::op(stack<converse_value> &i) {
	bool success = true;
	converse_value v[4], in; // args

	switch (in == pop_arg(i)) {
	default:
		i.push(in);
		success = ConverseInterpret::op(i);
	}
	return (success);
}
#endif

bool ConverseInterpret::op_create_new(stack<converse_typed_value> &i) {
	converse_value v[4];
	Actor *cnpc = NULL;

	v[0] = pop_arg(i); // npc
	v[1] = pop_arg(i); // obj
	v[2] = pop_arg(i); // qual
	v[3] = pop_arg(i); // quant
	cnpc = converse->actors->get_actor(npc_num(v[0]));
	if (cnpc) {
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6 && v[1] == 76) { // Amulet of Submission
			cnpc->remove_readied_object(ACTOR_NECK);
			Obj *cnpc_obj = cnpc->inventory_new_object(76, 1, 0);
			cnpc->add_readied_object(cnpc_obj);
		} else
			cnpc->inventory_new_object(v[1], v[3], v[2]);
	}

	return true;
}

bool WOUConverseInterpret::op_create_new(stack<converse_typed_value> &i) {
	converse_value v[4];
	Actor *cnpc = NULL;

	v[0] = pop_arg(i); // npc
	v[1] = pop_arg(i); // obj
	v[2] = pop_arg(i); // qual
	v[3] = pop_arg(i); // quant
	cnpc = converse->actors->get_actor(npc_num(v[0]));
	if (cnpc) {
		if (cnpc->can_carry_object(v[1], v[3])) {
			cnpc->inventory_new_object(v[1], v[3], v[2]);
			converse->set_var(WOUTALK_VAR_ADD_TO_INVENTORY_FAILED, 0);
		} else {
			converse->set_var(WOUTALK_VAR_ADD_TO_INVENTORY_FAILED, 1); //This var is set to true when the object cannot be added to the actor's inventory
		}
	}

	return true;
}

/* Run control code with arguments/operands (arranged on a stack from
 * top to bottom.)
 */
bool ConverseInterpret::op(stack<converse_typed_value> &i) {
	bool success = true;
	converse_value v[4] = { 0, 0, 0, 0 }; // args
	converse_value inVal;
	ConvScript *cs = converse->script;
	Actor *cnpc = NULL;
	Obj *cnpc_obj = NULL;
	Player *player = converse->player;
//    converse_db_s *cdb;

	inVal = pop_arg(i);

#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, "op %s(%x)\n", op_str(inVal), inVal);
#endif

	switch (inVal) {
	case U6OP_SLEEP: // 0x9e
		// Note: It's usually unecessary to wait for the effect, as it
		// pauses input and the user can't continue the conversation until
		// the effect is complete.
		new SleepEffect(5); // sleep until sunrise
		break;
	case U6OP_HORSE: // 0x9c
		// FIXME: probably need to do more real actor/object set-up here
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		cnpc_obj = cnpc->make_obj();
		cnpc_obj->frame_n = 0; // FIX for actors orginal direction.
		cnpc_obj->obj_n = OBJ_U6_HORSE_WITH_RIDER; // mount up.
		cnpc->init_from_obj(cnpc_obj);
		delete_obj(cnpc_obj);
		break;
	case U6OP_IF: // 0xa1 (test val)
		set_break(U6OP_ELSE);
		set_run(pop_arg(i) ? true : false);
		break;
	case U6OP_ENDIF: // 0xa2
		break; // (frame only)
	case U6OP_ELSE: // 0xa3
		break; // (frame only_
	case U6OP_SETF: // 0xa4 (npc, flagnum)
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc)
			cnpc->set_flag(pop_arg(i));
		break;
	case U6OP_CLEARF: // 0xa5 (npc, flagnum)
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc)
			cnpc->clear_flag(pop_arg(i));
		break;
	case U6OP_DECL: // 0xa6
		db_lvar = false;
		v[0] = pop_arg(i); // variable
		v[1] = pop_arg(i); // type

		if (!i.empty()) {
			converse_typed_value top = i.top();
			if ((top.val == 0xb2 || top.val == 0xb4)) {
				if (top.val == 0xb2) { // eg DB1[#2] = ...
					pop_arg(i); //0xb2
					pop_arg(i); //0xb4 DATA op.
					db_lvar = true;
					db_loc = v[0];
					db_offset = converse->get_var(v[1]);
				} else if (top.val == 0xb4) { // eg DB1[0] = ...
					pop_arg(i); //0xb4 DATA op.
					db_lvar = true;
					db_loc = v[0];
					db_offset = v[1];
				}
#ifdef CONVERSE_DEBUG
				DEBUG(1, LEVEL_DEBUGGING, "DB lvar found. location = %x, offset = %x", db_loc, db_offset);
#endif
			}
		}
		let(v[0], v[1]);
		break;
	case U6OP_ASSIGN: // 0xa8
	case U6OP_SETNAME: // 0xd8
		v[0] = pop_arg(i); // value
		if (inVal == U6OP_ASSIGN) { // 0xa8
			if (db_lvar) {
				set_db_integer(db_loc, db_offset, v[0]);
			} else if (decl_v <= U6TALK_VAR__LAST_) {
				if (decl_t == 0xb3)
					converse->set_svar(decl_v, get_rstr(v[0]));
				else if (decl_t == 0xb2)
					converse->set_var(decl_v, v[0]);
			} else {
				converse->print("[Illegal variable]\n");
				DEBUG(0, LEVEL_ERROR, "[Illegal variable] decl_v = %x decl_t = %x\n", decl_v, decl_t);
			}
		} else { // // 0xd8, assign name of npc `result' to ystr
			v[0] = npc_num(v[0]);
			// get name from party information
			if (player->get_party()->contains_actor(v[0])) {
				v[0] = player->get_party()->get_member_num(v[0]);
				set_ystr(player->get_party()->get_actor_name(v[0]));
			} else
				set_ystr(converse->npc_name(v[0])); // read from script
		}
		let(); // clear
		break;
	case U6OP_JUMP: // 0xb0
		v[0] = pop_arg(i);
#ifdef CONVERSE_DEBUG
		DEBUG(1, LEVEL_DEBUGGING, "Converse: JUMP 0x%04x\n", v[0]);
#endif
		cs->seek(v[0]);
		leave_all(); // always run
		break;
	case U6OP_DPRINT: { // 0xb5
		v[0] = pop_arg(i); // db location
		v[1] = pop_arg(i); // index
		char *dstring = get_db_string(v[0], v[1]);
		if (dstring) {
			converse->set_output(dstring); // data may have special symbols
//                converse->print(dstring); // data can have no special symbols -- wrong
//                converse->print("\n");
			free(dstring);
		}
		break;
	}
	case U6OP_BYE: // 0xb6
		stop();
		break;
	case U6OP_NEW: // 0xb9 (npc, obj, qual, quant)
		op_create_new(i);
		break;
	case U6OP_DELETE: // 0xba
//bool Actor::inventory_del_object(uint16 obj_n, uint8 qty, uint8 quality);
		v[0] = pop_arg(i); // npc
		v[1] = pop_arg(i); // obj
		v[2] = pop_arg(i); // qual
		v[3] = pop_arg(i); // quant
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc)
			cnpc->inventory_del_object(v[1], v[3], v[2]);
		break;
	case U6OP_GIVE: // 0xc9
		v[0] = pop_arg(i); // obj
		v[1] = pop_arg(i); // qual
		v[2] = pop_arg(i); // from
		v[3] = pop_arg(i); // to
		cnpc = converse->actors->get_actor(npc_num(v[2]));
		if (!cnpc)
			break;
		cnpc->inventory_del_object(v[0], 1, v[1]);
		cnpc = converse->actors->get_actor(npc_num(v[3]));
		if (cnpc)
			cnpc->inventory_new_object(v[0], 1, v[1]);
		break;
	case U6OP_ADDKARMA: // 0xc4
		player->add_karma(pop_arg(i));
		break;
	case U6OP_SUBKARMA: // 0xc5
		player->subtract_karma(pop_arg(i));
		break;
	case MDOP_MISC_ACTION: // 0xd1
		v[0] = pop_arg(i); // action number
		Game::get_game()->get_script()->call_talk_script(v[0]);
		break;
	case U6OP_RESURRECT: { // 0xd6
		v[0] = npc_num(pop_arg(i));
		uint16 body = OBJ_U6_DEAD_BODY;

		if (Game::get_game()->get_game_type() == NUVIE_GAME_SE)
			body = OBJ_SE_DEAD_BODY;
		else if (Game::get_game()->get_game_type() == NUVIE_GAME_MD)
			body = OBJ_MD_DEAD_BODY;

		if (v[0] == 0) // Party
			cnpc = converse->player->get_party()->who_has_obj(body , 0, false);
		else
			cnpc = converse->actors->get_actor(v[0]);

		cnpc_obj = cnpc->inventory_get_object(body, 0, false);
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6 && !cnpc_obj) {
			if (v[0] == 0)
				cnpc = converse->player->get_party()->who_has_obj(OBJ_U6_MOUSE, 0, false);
			cnpc_obj = cnpc->inventory_get_object(OBJ_U6_MOUSE, 0, false);
		}
		if (cnpc_obj != NULL) {
			if (converse->actors->resurrect_actor(cnpc_obj, converse->player->get_actor()->get_location())) {
				converse->objects->unlink_from_engine(cnpc_obj);
				delete_obj(cnpc_obj);
			}
		}
		break;
	}
	case U6OP_HEAL: // 0xd9
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc)
			cnpc->heal();
		break;
	case U6OP_CURE: // 0xdb
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc)
			cnpc->set_poisoned(false);
		break;
	case U6OP_WORKTYPE: // 0xcd
		v[0] = pop_arg(i); // npc
		v[1] = pop_arg(i); // worktype
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc)
			cnpc->set_worktype(v[1]);
		break;
	case U6OP_INVENTORY: // 0xbe
		converse->print("!inventory\n");
		break;
	case U6OP_PORTRAIT: // 0xbf
		converse->show_portrait(npc_num(pop_arg(i)));
		break;
	case U6OP_WAIT: // 0xcb, set page-break on scroller and wait
		converse->print("*");
		wait();
		break;
	case U6OP_ENDANSWER: // 0xee
		break; // (frame only)
	case U6OP_KEYWORDS: // 0xef (text:keywords)
		if (answer_mode != ANSWER_DONE) { // havn't already answered
			answer_mode = ANSWER_NO;
			if (check_keywords(get_text(), converse->get_input()))
				answer_mode = ANSWER_YES;
		}
		break; // (frame only)
	case U6OP_SIDENT: // 0xff, arg 1 is id number, name follows
		v[0] = pop_arg(i);
		if (v[0] != converse->npc_num)
			DEBUG(0, LEVEL_WARNING,
			      "Converse: npc number inVal script (%d) does not"
			      " match actor number (%d)\n", v[0], converse->npc_num);
		converse->_name = get_text(); // collected
		break;
	case U6OP_SLOOK: // 0xf1, description follows
		converse->_desc = get_formatted_text(get_text().c_str()); // collected
		converse->print("\nYou see ");
		converse->print(converse->_desc.c_str());
		converse->print("\n");
		break;
	case U6OP_SCONVERSE: // 0xf2
		break;
	case U6OP_SPREFIX: // 0xf3, Unknown
		break;
	case U6OP_ANSWER: // 0xf6
		set_break(U6OP_KEYWORDS); // run or skip to next 0xef
		if (answer_mode == ANSWER_YES) { // depending on input comparison
			set_run(true);
			answer_mode = ANSWER_DONE; // don't do further comparisons
		} else
			set_run(false);
		break;
	case U6OP_ASK: // 0xf7
		answer_mode = ANSWER_NO; // reset answer switch
		converse->collect_input();
		break;
	case U6OP_ASKC: // 0xf8 (blocking, single character input)
		answer_mode = ANSWER_NO; // reset answer switch
		converse->poll_input(get_text().c_str(), false); // collected=allowed input
		break;
	case U6OP_INPUTSTR: // 0xf9 (string or integer)
		answer_mode = ANSWER_NO; // reset answer switch
		// fall through
	case U6OP_INPUT: // 0xfb (integer)
		v[0] = pop_arg(i); // var
		v[1] = pop_arg(i); // type (should be 0xb2)
		let(v[0], v[1]);
		converse->poll_input();
		break;
	case U6OP_INPUTNUM: // 0xfc
		v[0] = pop_arg(i); // var
		v[1] = pop_arg(i); // type
		let(v[0], v[1]);
		converse->poll_input("0123456789");
		break;
	default:
		converse->print("[Unknown command]\n");
		DEBUG(0, LEVEL_ERROR, "Converse: UNK OP=%02x\n", inVal);
		success = false;
	}
	return (success);
}


/* The other set of codes, these operate on the input values, so call them
 * valops. Output goes back to the stack.
 */
bool ConverseInterpret::evop(stack<converse_typed_value> &i) {
	bool success = true;
	converse_value v[4]; // input
	converse_typed_value inVal;
	converse_typed_value out;
	Actor *cnpc = NULL;
	Obj *cnpc_obj = NULL;
//    converse_db_s *cdb;
	Player *player = converse->player;

	out.type = U6OP_VAR;
	out.val = 0;

	inVal.val = pop_arg(i);

#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, "evop %s(%x)\n", evop_str(inVal.val), inVal.val);
#endif

	switch (inVal.val) {
	case U6OP_GT: // 0x81
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		if (v[0] > v[1])
			out.val = 1;
		break;
	case U6OP_GE: // 0x82
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		if (v[0] >= v[1])
			out.val = 1;
		break;
	case U6OP_LT: // 0x83
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		if (v[0] < v[1])
			out.val = 1;
		break;
	case U6OP_LE: // 0x84
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		if (v[0] <= v[1])
			out.val = 1;
		break;
	case U6OP_NE: // 0x85
		if (pop_arg(i) != pop_arg(i))
			out.val = 1;
		break;
	case U6OP_EQ: // 0x86
		out.val = evop_eq(i);
		break;
	case U6OP_ADD: // 0x90
		out.val = pop_arg(i) + pop_arg(i);
		break;
	case U6OP_SUB: // 0x91
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		out.val = v[0] - v[1];
		break;
	case U6OP_MUL: // 0x92
		out.val = pop_arg(i) * pop_arg(i);
		break;
	case U6OP_DIV: // 0x93
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		if (v[1] == 0) {
			DEBUG(0, LEVEL_ERROR, "Converse: Divide error\n");
			success = false;
			stop();
		} else
			out.val = v[0] / v[1];
		break;
	case U6OP_LOR: // 0x94
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		if (v[0] || v[1])
			out.val = 1;
		break;
	case U6OP_LAND: // 0x95
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		if (v[0] && v[1])
			out.val = 1;
		break;
	case U6OP_CANCARRY: // 0x9a
		cnpc = converse->actors->get_actor(pop_arg(i));
		if (cnpc)
			out.val = (converse_value)((cnpc->inventory_get_max_weight()
			                            - cnpc->get_inventory_weight()) * 10);
		break;
	case U6OP_WEIGHT: { // 0x9b
		v[1] = pop_arg(i); // quantity
		v[0] = pop_arg(i); // object
		float weight = (float)converse->objects->get_obj_weight_unscaled(v[0]) * v[1]; //FIXME should this be scaled down by 10???
		if (converse->objects->has_reduced_weight(v[0]))
			weight /= 10;
		out.val = weight;
		break;
	}
	case U6OP_HORSED: // 0x9d
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		cnpc_obj = cnpc->make_obj();
		if (cnpc_obj->obj_n == OBJ_U6_HORSE_WITH_RIDER)
			out.val = 1;
		delete_obj(cnpc_obj);
		break;
	case U6OP_HASOBJ: // 0x9f
		v[2] = pop_arg(i); // quality
		v[1] = pop_arg(i); // object
		v[0] = pop_arg(i); // npc
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc && cnpc->inventory_has_object(v[1], v[2], (v[2] != 0 ? true : false))) //don't search quality if quality is 0
			out.val = 1;
		break;
	case U6OP_RAND: // 0xa0
		v[1] = pop_arg(i); // max.
		v[0] = pop_arg(i); // min.
		if (v[0] >= v[1])
			out.val = v[0];
		else
			out.val = v[0] + (NUVIE_RAND() % (v[1] - v[0] + 1));
		break;
	case U6OP_FLAG: // 0xab (npc, flag)
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
#ifdef CONVERSE_DEBUG
		DEBUG(1, LEVEL_DEBUGGING, "npc=%d bit=%d\n", v[0], v[1]);
#endif
		if (v[1] <= 7) {
			cnpc = converse->actors->get_actor(npc_num(v[0]));
			if (cnpc && (cnpc->get_talk_flags() & (1 << v[1])))
				out.val = 1;
		}
		break;
	case U6OP_VAR: // 0xb2
		out.val = converse->get_var(pop_arg(i));
		break;
	case U6OP_SVAR: // 0xb3 (using new rstring)
		out.val = add_rstr(converse->get_svar(pop_arg(i))); // rstr num
		out.type = U6OP_SVAR;
		break;
#if 0 /* old read style tries to detect if data is string or integer */
	case U6OP_DATA: // 0xb4
		v[1] = pop_arg(i); // index
		v[0] = pop_arg(i); // db location
		cdb = get_db(v[0], v[1]);
		if (!cdb)
			out.val = 0;
		else if (cdb->type == 0)
			out.val = add_rstr(cdb->s);
		else
			out.val = cdb->i;
		delete cdb;
		break;
#else /* new read assumes DATA may be assigned, and depend on variable type */
	case U6OP_DATA: // 0xb4
		v[1] = pop_arg(i); // index
		v[0] = pop_arg(i); // db location

		if (decl_t == 0xb3) {
			char *dstring = get_db_string(v[0], v[1]);
			if (dstring) {
				out.val = add_rstr(dstring);
				free(dstring);
			} else
				out.val = add_rstr("");

			out.type = U6OP_SVAR;
		} else // this will work for 0xB2 assignment and normal stack ops
			out.val = get_db_integer(v[0], v[1]);
		break;
#endif
	case U6OP_INDEXOF: // 0xb7
		v[1] = pop_arg(i); // string variable
		v[0] = pop_arg(i); // db location
		out.val = find_db_string(v[0], converse->get_svar(v[1]));
		break;
	case U6OP_OBJCOUNT: // 0xbb
		v[1] = pop_arg(i); // object
		v[0] = pop_arg(i); // npc
		cnpc = converse->actors->get_actor(npc_num(v[0])); //FIXME should this be party?
		if (cnpc)
			out.val = cnpc->inventory_count_object(v[1]);
		break;
	case U6OP_INPARTY: // 0xc6
		if (player->get_party()->contains_actor(npc_num(pop_arg(i))))
			out.val = 1;
		break;
	case U6OP_OBJINPARTY: { // 0xc7 ?? check if party has object
		v[1] = pop_arg(i); // qual
		v[0] = pop_arg(i); // obj
		bool has_mouse = false; // resurrect others first
		if (!player->get_party()->has_obj(v[0], v[1], false) && (v[0] != OBJ_U6_DEAD_BODY
		        || Game::get_game()->get_game_type() != NUVIE_GAME_U6
		        || !(has_mouse = player->get_party()->has_obj(OBJ_U6_MOUSE, v[1], false))))
			out.val = 0x8001; // something OR'ed or u6val version of "no npc"?
		else {
			cnpc = player->get_party()->who_has_obj(has_mouse ? OBJ_U6_MOUSE : v[0], v[1], false);
			assert(cnpc);
			out.val = cnpc->get_actor_num(); // first NPC that has object (sometimes 0xFFFF?)
		}
		break;
	}
	case U6OP_JOIN: // 0xca
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc) {
			if (player->get_party()->contains_actor(cnpc))
				out.val = 3; // 3: ALREADY IN PARTY
			else if (player->get_party()->get_party_size() >=
			         player->get_party()->get_party_max())
				out.val = 2; // 2: PARTY TOO LARGE
			else if (player->get_actor()->get_actor_num() == 0) // vehicle
				out.val = 1; // 1: NOT ON LAND
			else
				player->get_party()->add_actor(cnpc);
			// 0: SUCCESS
		}
		break;
	case U6OP_LEAVE: // 0xcc
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc) {
			if (!player->get_party()->contains_actor(cnpc))
				out.val = 2; // 2: NOT IN PARTY
			else if (player->get_actor()->get_actor_num() == 0) // vehicle
				out.val = 1; // 1: NOT ON LAND
			else
				player->get_party()->remove_actor(cnpc);
			// 0: SUCCESS
		}
		break;
	case U6OP_NPCNEARBY: // 0xd7
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc)
			out.val = converse->npc->is_nearby(cnpc) ? 1 : 0;
		break;
	case U6OP_WOUNDED: // 0xda
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc)
			if (cnpc->get_hp() < cnpc->get_maxhp())
				out.val = 1;
		break;
	case U6OP_POISONED: // 0xdc
		cnpc = converse->actors->get_actor(npc_num(pop_arg(i)));
		if (cnpc)
			out.val = cnpc->is_poisoned() ? 1 : 0;
		break;
	case U6OP_NPC: // 0xdd (val2=??)
		v[1] = pop_arg(i);
		v[0] = pop_arg(i); // 0+
		cnpc = player->get_party()->get_actor(v[0]);
		out.val = cnpc ? cnpc->get_actor_num() : 0;
		break;
	case U6OP_EXP: // 0xe0
		v[1] = pop_arg(i); // add value
		v[0] = pop_arg(i); // npc
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc) {
			out.val = cnpc->get_exp() + v[1];
			cnpc->set_exp(out.val);
		}
		break;
	case U6OP_LVL: // 0xe1
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc) {
			out.val = cnpc->get_level() + v[1];
			cnpc->set_level(out.val);
		}
		break;
	case U6OP_STR: // 0xe2
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc) {
			out.val = cnpc->get_strength() + v[1];
			cnpc->set_strength(out.val);
		}
		break;
	case U6OP_INT: // 0xe3
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc) {
			out.val = cnpc->get_intelligence() + v[1];
			cnpc->set_intelligence(out.val);
		}
		break;
	case U6OP_DEX: // 0xe4
		v[1] = pop_arg(i);
		v[0] = pop_arg(i);
		cnpc = converse->actors->get_actor(npc_num(v[0]));
		if (cnpc) {
			out.val = cnpc->get_dexterity() + v[1];
			cnpc->set_dexterity(out.val);
		}
		break;
	default:
		DEBUG(0, LEVEL_ERROR, "Converse: UNK EVOP=%02x\n", inVal);
		success = false;
	}
	i.push(out);
	return (success);
}

converse_value ConverseInterpret::evop_eq(stack<converse_typed_value> &vs) {
	converse_typed_value operand1 = pop_typed_arg(vs);
	converse_typed_value operand2 = pop_typed_arg(vs);
	converse_value out = 0;

	if (operand1.type == U6OP_VAR) {
		if (operand1.val == operand2.val)
			out = 1;
	} else {
		if (scumm_stricmp(get_rstr(operand1.val), get_rstr(operand2.val)) == 0)
			out = 1;
	}

	return out;
}

/* Take the values collected so far as input (starting with value `vi') and run
 * any assign/compare value ops that are mixed with the data. The result goes
 * back to the input list, and replaces everything after and including `vi'.
 */
void ConverseInterpret::eval(uint32 vi) {
	stack<converse_typed_value> op_stk, r_stk;
	uint32 v = vi;
#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, "Converse: EVAL");
	for (uint32 w = 0; w < val_count(); w++)
		DEBUG(1, LEVEL_DEBUGGING, " %s0x%02x%s", (w == vi) ? "(" : "", get_val(w), (w == val_count() - 1) ? ")" : "");
#endif

	while (v < val_count()) {
		converse_typed_value a = {U6OP_VAR, 0};
		a.val = get_val(v);
		uint8 ds = get_val_size(v++);
		op_stk.push(a);
		if (is_valop(a.val) && !ds)
			evop(op_stk);
	}
	in.resize(vi);
	if (op_stk.empty()) // took all parameters, no return
		add_val(0x00);
	else {
		while (!op_stk.empty()) {
			r_stk.push(op_stk.top()); // reverse
			op_stk.pop();
		}
		while (!r_stk.empty())
			add_val(pop_arg(r_stk)); // return(s)
	}

#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, " ->");
	if (val_count()) {
		for (uint32 w = 0; w < val_count(); w++)
			DEBUG(1, LEVEL_DEBUGGING, " 0x%02x", get_val(w));
		DEBUG(1, LEVEL_DEBUGGING, "\n");
	}
#endif
}


/* Return `n' as-is or convert to number of NPC from Converse.
 */
uint8 ConverseInterpret::npc_num(uint32 n) {
	return ((n != 0xeb) ? n : converse->npc_num);
}


/* Returns true if the keywords list contains the input string, or contains an
 * asterisk (matching any input).
 */
bool ConverseInterpret::check_keywords(string keystr, string instr) {
	const char *strt_s = NULL;
	char *tok_s = NULL, *cmp_s = NULL;
	if (keystr == "*")
		return (true);
	// check each comma-separated keyword
	strt_s = keystr.c_str();
	for (uint32 c = 0; c < strlen(strt_s); c++) {
		// check at start of string and each keyword
		if (c == 0 || strt_s[c] == ',') {
			// copy from keyword start to end of string/keyword
			uint32 l;
			tok_s = scumm_strdup(&strt_s[(c == 0) ? c : c + 1]);
			for (l = 0; l < strlen(tok_s) && tok_s[l] != ','; l++);
			tok_s[l] = '\0';
			cmp_s = scumm_strdup(instr.c_str());
			// trim input to keyword size
			if (l < strlen(cmp_s))
				cmp_s[l] = '\0';
			// compare
			if (!scumm_stricmp(tok_s, cmp_s)) {
				free(cmp_s);
				free(tok_s);
				return (true);
			}
			free(cmp_s);
			free(tok_s);
		}
	}
	return (false);
}


/* Assign input from Converse to the declared variable.
 */
void ConverseInterpret::assign_input() {
	// FIXME: Nuvie treats 0xF9-INPUTSTR & 0xFB-INPUT as identical, but in U6
	//        0xFB-INPUT could not input strings.
	if (decl_t == 0xb2)
		converse->set_var(decl_v, strtol(converse->get_input().c_str(), NULL, 10));
	if (decl_t == 0xb3)
		converse->set_svar(decl_v, converse->get_input().c_str());
}


/* Collect data from section at `loc', index `i'.
 * Returns pointer to data, which can be 8bit integer or a character string.
 * FIXME: there is no checking for overflow beyond the initial location
 *        Deprecated function that guesses the data type, and reads incorrectly.
 */
struct ConverseInterpret::converse_db_s *
ConverseInterpret::get_db(uint32 loc, uint32 i) {
	convscript_buffer db = converse->script->get_buffer(loc);
	struct converse_db_s *item = NULL;
	uint32 d = 0, dbuf_len = 0, p = 0, e = 0;
	if (!db)
		return (NULL);

//    item = (struct converse_db_s *)malloc(sizeof(struct converse_db_s));
	item = new struct converse_db_s;
	item->type = 0;
	item->s = NULL;
	item->i = 0;
	while (e++ <= i) {
		if (is_print(db[p]) && is_print(db[p + 1])) {
			d = 0;
			dbuf_len = 0;
			item->type = 0;
			do {
				if ((d + 1) >= dbuf_len)
					dbuf_len += 16;
				item->s = (char *)realloc(item->s, dbuf_len);
				item->s[d++] = (char)(db[p]);
				item->s[d] = '\0';
			} while (is_print(db[++p]));
		} else {
			free(item->s);
			item->s = 0;
			item->type = 1;
			item->i = (uint8)(db[p++]);
		}
		++p; // skip 0
	}
	return (item);
}


/* Collect data from section at `loc', index `i', as a string.
 * Returns pointer to NEW data, or NULL if only integer data is found.
 */
char *ConverseInterpret::get_db_string(uint32 loc, uint32 i) {
	convscript_buffer db = converse->script->get_buffer(loc);
	char *item = NULL;
	uint32 d = 0, dbuf_len = 0, /* string pointer & length */
	       p = 0; /* pointer into db */
	if (!db)
		return (NULL);
	/* skip to index */
	uint32 e = 0;
	while (e++ < i) {
		if (db[p] == U6OP_ENDDATA)
			return (NULL);
		while (is_print(db[p++]));
	}

	d = 0;
	dbuf_len = 0;
	do {
		if ((d + 1) >= dbuf_len) // resize buffer as needed
			dbuf_len += 16;
		item = (char *)nuvie_realloc(item, dbuf_len);
		item[d++] = (char)(db[p]); // copy
		item[d] = '\0';
	} while (is_print(db[++p]));
	return (item);
}


/* Collect data from section at `loc', index `i', as an integer.
 * Returns the two-byte integer value.
 */
converse_value ConverseInterpret::get_db_integer(uint32 loc, uint32 i) {
	uint16 item = 0;
	uint32 p = 0; /* pointer into db */

	/* skip to index */
	uint32 e = 0;
	while (e++ < i)
		p += 2;

	/* use ConvScript functions to check overflow and read data correctly */
	uint32 old_pos = converse->script->pos();
	converse->script->seek(loc + p);
	if (!converse->script->overflow(+1))
		item = converse->script->read2();
	converse->script->seek(old_pos);

	return ((converse_value)item);
}

void ConverseInterpret::set_db_integer(uint32 loc, uint32 i, converse_value val) {
#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, "set_db_integer(%x, %x, %d)\n", loc, i, val);
#endif
	uint32 p = 0; /* pointer into db */

	/* skip to index */
	uint32 e = 0;
	while (e++ < i)
		p += 2;

	/* use ConvScript functions to check overflow and read data correctly */
	uint32 old_pos = converse->script->pos();
	converse->script->seek(loc + p);
	if (!converse->script->overflow(+1))
		converse->script->write2(val);
	converse->script->seek(old_pos);

	return;
}

/* Scan data section `loc' for `dstring'. Stop at ENDDATA.
 * Returns the index of the string or the index of the ENDDATA marker.
 */
converse_value ConverseInterpret::find_db_string(uint32 loc, const char *dstring) {
	convscript_buffer db = converse->script->get_buffer(loc);
	char *item = NULL; /* item being checked */
	uint32 d = 0, dbuf_len = 0, /* string pointer & length */
	       p = 0, /* pointer into db */
	       i = 0; /* item index */
#ifdef CONVERSE_DEBUG
	DEBUG(1, LEVEL_DEBUGGING, "\nConverse: find_db_string(0x%04x, \"%s\")\n", loc, dstring);
#endif
	while ((converse_value)(db[p]) != U6OP_ENDDATA) {
		if (is_print(db[p])) {
			item = NULL;
			d = 0;
			dbuf_len = 0;
			do {
				if ((d + 1) >= dbuf_len) // resize buffer as needed
					dbuf_len += 16;
				item = (char *)nuvie_realloc(item, dbuf_len);
				item[d++] = (char)(db[p]); // copy
				item[d] = '\0';
			} while (is_print(db[++p]));
			++p; // skip this unprintable now so it's not counted as an item
			if (item) {
				string item_str = item;
				string find_str = dstring;
				free(item);
				// match keywords format: clamp item to 4 characters
				if (item_str.size() > 4)
					item_str.resize(4);
				if (check_keywords(item_str, find_str))
					return (i);
			}
		} else ++p;
		++i;
	}
#ifdef CONVERSE_DEBUG
	DEBUG(0, LEVEL_DEBUGGING, "\nConverse: find_db_string: not found; returning %d\n", i);
#endif
	return (i);
}

const char *ConverseInterpret::evop_str(converse_value op) {
	switch (op) {
	case U6OP_GT:
		return "U6OP_GT";
	case U6OP_GE:
		return "U6OP_GE";
	case U6OP_LT:
		return "U6OP_LT";
	case U6OP_LE:
		return "U6OP_LE";
	case U6OP_NE:
		return "U6OP_NE";
	case U6OP_EQ:
		return "U6OP_EQ";
	case U6OP_ADD:
		return "U6OP_ADD";
	case U6OP_SUB:
		return "U6OP_SUB";
	case U6OP_MUL:
		return "U6OP_MUL";
	case U6OP_DIV:
		return "U6OP_DIV";
	case U6OP_LOR:
		return "U6OP_LOR";
	case U6OP_LAND:
		return "U6OP_LAND";
	case U6OP_CANCARRY:
		return "U6OP_CANCARRY";
	case U6OP_WEIGHT:
		return "U6OP_WEIGHT";
	case U6OP_HORSED:
		return "U6OP_HORSED";
	case U6OP_HASOBJ:
		return "U6OP_HASOBJ";
	case U6OP_RAND:
		return "U6OP_RAND";
	case U6OP_FLAG:
		return "U6OP_FLAG";
	case U6OP_VAR:
		return "U6OP_VAR";
	case U6OP_SVAR:
		return "U6OP_SVAR";
	case U6OP_DATA:
		return "U6OP_DATA";
	case U6OP_INDEXOF:
		return "U6OP_INDEXOF";
	case U6OP_OBJCOUNT:
		return "U6OP_OBJCOUNT";
	case U6OP_INPARTY:
		return "U6OP_INPARTY";
	case U6OP_OBJINPARTY:
		return "U6OP_OBJINPARTY";
	case U6OP_JOIN:
		return "U6OP_JOIN";
	case U6OP_LEAVE:
		return "U6OP_LEAVE";
	case U6OP_NPCNEARBY:
		return "U6OP_NPCNEARBY";
	case U6OP_WOUNDED:
		return "U6OP_WOUNDED";
	case U6OP_POISONED:
		return "U6OP_POISONED";
	case U6OP_NPC:
		return "U6OP_NPC";
	case U6OP_EXP:
		return "U6OP_EXP";
	case U6OP_LVL:
		return "U6OP_LVL";
	case U6OP_STR:
		return "U6OP_STR";
	case U6OP_INT:
		return "U6OP_INT";
	case U6OP_DEX:
		return "U6OP_DEX";
	default:
		break;
	}

	return "U6OP_UNKNOWN";
}

const char *ConverseInterpret::op_str(converse_value op) {
	switch (op) {
	case U6OP_SLEEP:
		return "U6OP_SLEEP";
	case U6OP_HORSE:
		return "U6OP_HORSE";
	case U6OP_IF:
		return "U6OP_IF";
	case U6OP_ENDIF:
		return "U6OP_ENDIF";
	case U6OP_ELSE:
		return "U6OP_ELSE";
	case U6OP_SETF:
		return "U6OP_SETF";
	case U6OP_CLEARF:
		return "U6OP_CLEARF";
	case U6OP_DECL:
		return "U6OP_DECL";
	case U6OP_ASSIGN:
		return "U6OP_ASSIGN";
	case U6OP_SETNAME:
		return "U6OP_SETNAME";
	case U6OP_JUMP:
		return "U6OP_JUMP";
	case U6OP_DPRINT:
		return "U6OP_DPRINT";
	case U6OP_BYE:
		return "U6OP_BYE";
	case U6OP_NEW:
		return "U6OP_NEW";
	case U6OP_DELETE:
		return "U6OP_DELETE";
	case U6OP_GIVE:
		return "U6OP_GIVE";
	case U6OP_ADDKARMA:
		return "U6OP_ADDKARMA";
	case U6OP_SUBKARMA:
		return "U6OP_SUBKARMA";
	case U6OP_RESURRECT:
		return "U6OP_RESURRECT";
	case U6OP_HEAL:
		return "U6OP_HEAL";
	case U6OP_CURE:
		return "U6OP_CURE";
	case U6OP_WORKTYPE:
		return "U6OP_WORKTYPE";
	case U6OP_INVENTORY:
		return "U6OP_INVENTORY";
	case U6OP_PORTRAIT:
		return "U6OP_PORTRAIT";
	case U6OP_WAIT:
		return "U6OP_WAIT";
	case U6OP_ENDANSWER:
		return "U6OP_ENDANSWER";
	case U6OP_KEYWORDS:
		return "U6OP_KEYWORDS";
	case U6OP_SIDENT:
		return "U6OP_SIDENT";
	case U6OP_SLOOK:
		return "U6OP_SLOOK";
	case U6OP_SCONVERSE:
		return "U6OP_SCONVERSE";
	case U6OP_SPREFIX:
		return "U6OP_SPREFIX";
	case U6OP_ANSWER:
		return "U6OP_ANSWER";
	case U6OP_ASK:
		return "U6OP_ASK";
	case U6OP_ASKC:
		return "U6OP_ASKC";
	case U6OP_INPUTSTR:
		return "U6OP_INPUTSTR";
	case U6OP_INPUT:
		return "U6OP_INPUT";
	case U6OP_INPUTNUM:
		return "U6OP_INPUTNUM";
	default :
		break;
	}

	return "U6OP_UNKNOWN";
}

} // End of namespace Nuvie
} // End of namespace Ultima
