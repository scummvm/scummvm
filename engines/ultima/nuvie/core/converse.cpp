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
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/converse_interpret.h"
#include "ultima/nuvie/core/converse_speech.h"
#include "ultima/nuvie/gui/widgets/converse_gump.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/widgets/background.h"

namespace Ultima {
namespace Nuvie {

//#define CONVERSE_DEBUG


Converse::Converse() {
	config = NULL;
	clock = NULL;
	actors = NULL;
	objects = NULL;
	player = NULL;
	views = NULL;
	last_view = NULL;
	scroll = NULL;

	conv_i = NULL;
	script = NULL;
	npc = NULL;
	npc_num = 0;
	script_num = 0;
	src = NULL;
	src_num = 0;

	allowed_input = NULL;

	active = false;
	variables = NULL;
	party_all_the_time = false;
	speech = NULL;
	using_fmtowns = false;
	need_input = false;
	aname[15] = '\0';
	gametype = NUVIE_GAME_NONE;
}


/* Initialize global classes from the game.
 */
void
Converse::init(Configuration *cfg, nuvie_game_t t, MsgScroll *s, ActorManager *a,
               GameClock *c, Player *p, ViewManager *v, ObjManager *o) {
	Std::string townsdir;

	config = cfg;
	scroll = s;
	actors = a;
	clock = c;
	player = p;
	views = v;
	objects = o;
	gametype = t;

	cfg->value("config/cheats/party_all_the_time", party_all_the_time);
	cfg->value("config/audio/conversations_stop_music", conversations_stop_music, false);

	cfg->value("config/townsdir", townsdir, "");
	if (townsdir != "" && directory_exists(townsdir.c_str()))
		using_fmtowns = true;

	speech = new ConverseSpeech();
	speech->init(config);
}


Converse::~Converse() {
	if (running()) {
		reset();
		DEBUG(0, LEVEL_INFORMATIONAL, "End conversation\n");
	}
	unload_conv();

	delete speech;
}


/* Free up allocated memory, reset values for new conversation. (call only when
 * ending a conversation or quitting)
 */
void Converse::reset() {
	delete conv_i;
	conv_i = NULL;
	set_input("");	// delete
	set_output("");	// clear output
	_name = "";		// clear name

	if (script) {
		delete script;
		script = NULL;
	}

	if (allowed_input) {
		free(allowed_input);
		allowed_input = NULL;
	}

	player->set_quest_flag((uint8)get_var(U6TALK_VAR_QUESTF));
	player->set_gargish_flag((uint8)get_var(U6TALK_VAR_GARGF));

	delete_variables();
}


/* Load `convfilename' as src.
 */
void Converse::load_conv(const Std::string &convfilename) {
	string conv_lib_str;
	if (gametype == NUVIE_GAME_U6 && using_fmtowns) {
		config->pathFromValue("config/townsdir", convfilename, conv_lib_str);
	} else {
		config_get_path(config, convfilename, conv_lib_str);
	}

	unload_conv();
	src_num = 0;
	if (gametype == NUVIE_GAME_U6) {
		src = new U6Lib_n;
		src->open(conv_lib_str, 4);
		src_num = (convfilename == "converse.a") ? 1 : (convfilename == "converse.b") ? 2 : 0;
	} else { // MD or SE gametype
		src = new U6Lib_n;
		src->open(conv_lib_str, 4, gametype);
		src_num = 1;
	}

#ifdef CONVERSE_DEBUG
	DEBUG(0, LEVEL_DEBUGGING, "Converse: load \"%s\"\n", convfilename.c_str());
#endif
}

uint32 Converse::get_script_num(uint8 a) {
	if (gametype == NUVIE_GAME_U6) {
		if (a > 200) { // (quick fix for U6: anything over 200 is a temporary npc)
			Actor *npcP = actors->get_actor(a);
			if (npcP->get_obj_n() == 373) // OBJ_U6_WISP
				a = 201;
			else if (npcP->get_obj_n() == 382) // OBJ_U6_GUARD
				a = 202;
		}
		//else if(a == 188)  // U6: temp. fix for shrines
		//   a = 191; // ??? -> Exodus
		//else if(a >= 191 && a <= 197) // shrines except spirituality & humility
		//   a += 2;
		//else if(a == 198)
		//   a = 192; // Spirituality -> Honesty
		//else if(a == 199)
		//   a = 200; // Humility -> Singularity
	}

	return a;
}

/* Check that loaded converse library (if any) has script for npc `a'. Load
 * another file if it doesn't.
 * Returns the real item number in the source.
 */
uint32 Converse::load_conv(uint8 a) {
	if (gametype == NUVIE_GAME_U6) {
		if (a <= 98) {
			if (src_num != 1)
				load_conv("converse.a");
		} else { // a >= 99
			if (src_num != 2)
				load_conv("converse.b");
		}
	} else {
		if (src_num != 1)
			load_conv("talk.lzc");
	}
// we want to return the real item number in the converse file.
	if (gametype == NUVIE_GAME_U6 && a > 98) {
		a -= 99;
	} else if (gametype == NUVIE_GAME_SE) {
		a -= 2;
	}
	return a;
}


/* Returns name of loaded source file, identified by `src_num'.
 */
const char *Converse::src_name() {
	if (src_num == 0)
		return ("");
	if (gametype == NUVIE_GAME_U6)
		return ((src_num == 1) ? "converse.a" : "converse.b");
	if (gametype == NUVIE_GAME_MD)
		return ("talk.lzc");
	if (gametype == NUVIE_GAME_SE)
		return ("talk.lzc");

	return ("");
}


/* Get an NPC conversation from the source file.
 * Returns new ConvScript object.
 */
ConvScript *Converse::load_script(uint32 n) {
	ConvScript *loaded = new ConvScript(src, n);
	if (!loaded->loaded()) {
		delete loaded;
		loaded = NULL;
	} else
		DEBUG(0, LEVEL_INFORMATIONAL, "Read %s npc script (%s:%d)\n",
		      loaded->compressed ? "encoded" : "unencoded", src_name(), (unsigned int)n);
	return (loaded);
}


/* Initialize Converse variable list, and set globals from game.
 */
void Converse::init_variables() {
	if (variables)
		delete_variables();

	variables = new converse_variables_s[U6TALK_VAR__LAST_ + 1];
	for (uint32 v = 0; v <= U6TALK_VAR__LAST_; v++) {
		variables[v].cv = 0;
		variables[v].sv = NULL;
	}
	set_var(U6TALK_VAR_SEX, player->get_gender());
	set_var(U6TALK_VAR_KARMA, player->get_karma());
	set_var(U6TALK_VAR_GARGF, player->get_gargish_flag());
	set_var(U6TALK_VAR_PARTYLIVE, player->get_party()->get_party_size() - 1);
	// FIXME: count dead party members in PARTYALL, not in PARTYLIVE
	set_var(U6TALK_VAR_PARTYALL, get_var(U6TALK_VAR_PARTYLIVE));
	set_var(U6TALK_VAR_HP, player->get_actor()->get_hp());
	set_svar(U6TALK_VAR_NPC_NAME, npc_name(npc_num));
	set_svar(U6TALK_VAR_PLAYER_NAME, player->get_name());
	set_var(U6TALK_VAR_QUESTF, player->get_quest_flag());
	set_var(U6TALK_VAR_WORKTYPE, npc->get_worktype());
}


/* Free memory used by Converse variable list.
 */
void Converse::delete_variables() {
	for (uint32 v = 0; v <= U6TALK_VAR__LAST_; v++)
		if (variables[v].sv)
			free(variables[v].sv);
	delete [] variables;
	variables = NULL;
}


/* Create new script interpreter for the current game.
 * Returns pointer to object which is derived from ConverseInterpret.
 */
ConverseInterpret *Converse::new_interpreter() {
	ConverseInterpret *ci = NULL;
	switch (gametype) {
	case NUVIE_GAME_U6:
		ci = (ConverseInterpret *)new U6ConverseInterpret(this);
		break;
	case NUVIE_GAME_MD:
		ci = (ConverseInterpret *)new MDTalkInterpret(this);
		break;
	case NUVIE_GAME_SE:
		ci = (ConverseInterpret *)new SETalkInterpret(this);
		break;
	}
	return (ci);
}


/* Returns false if a conversation cannot be started with the NPC. This
 * represents an internal error, and doesn't have anything to do with the NPC
 * not wanting/being able to talk to the Avatar.
 */
bool Converse::start(uint8 n) {
	uint32 real_script_num = 0; // The script number in the converse file.

	// load, but make sure previous script is unloaded first
	if (running())
		stop();
	if (!(npc = actors->get_actor(n)))
		return (false);
	// get script num for npc number (and open file)
	script_num = get_script_num(n);
	real_script_num = load_conv(script_num);
	if (!src)
		return (false);

	script = load_script(real_script_num);

	// begin
	if (script) {
		active = true;
		last_view = views->get_current_view();
		if (!(conv_i = new_interpreter())) {
			DEBUG(0, LEVEL_CRITICAL, "Can't talk: Unimplemented or unknown game type\n");
			return (false);
		}
		views->close_all_gumps();
		// set current NPC and start conversation
		npc_num = n;
		init_variables();
		scroll->set_talking(true, actors->get_actor(npc_num));
		Game::get_game()->get_map_window()->set_walking(false);
		Game::get_game()->get_map_window()->set_looking(false);
		if (conversations_stop_music)
			Game::get_game()->get_sound_manager()->musicStop();
		//Game::get_game()->get_event()->set_mode(WAIT_MODE); // ignore player actions
		Game::get_game()->pause_user();
		Game::get_game()->get_gui()->unblock();
		scroll->set_autobreak(true);
		/* moved into ConverseGump::set_talking()
		if(Game::get_game()->is_new_style())
		{
		    scroll->Show();
		    scroll->set_input_mode(false);
		    scroll->clear_scroll();
		    ((ConverseGump *)scroll)->set_found_break_char(true);
		    //scroll->grab_focus();
		}
		*/
		show_portrait(npc_num);
		unwait();
		DEBUG(0, LEVEL_INFORMATIONAL, "Begin conversation with \"%s\" (npc %d)\n", npc_name(n), n);
		return (true);
	}
	DEBUG(0, LEVEL_ERROR, "Failed to load npc %d from %s:%d\n",
	      n, src_name(), script_num);
	return (false);
}


/* Stop execution of the current script.
 */
void Converse::stop() {
	scroll->set_talking(false);
	MsgScroll *system_scroll = Game::get_game()->get_scroll();

	if ((Game::get_game()->using_new_converse_gump() || scroll != system_scroll) && !scroll->is_converse_finished()) {
		return;
	}

	reset(); // free memory

	if (Game::get_game()->using_new_converse_gump()) {
		scroll->Hide();
		if (!Game::get_game()->is_new_style()) {
			Game::get_game()->get_event()->endAction(true);
			GUI::get_gui()->force_full_redraw(); // need to remove converse background
		}
	} else {


		system_scroll->set_autobreak(false);
		system_scroll->display_string("\n");
		system_scroll->display_prompt();

		if (scroll != system_scroll) { //if using an alternate scroll eg wou fullmap scroll.
			scroll->Hide();
		}
	}
	if (!Game::get_game()->is_new_style()) {
		if (last_view->set_party_member(last_view->get_party_member_num()) == false) // set party member left party
			last_view->prev_party_member(); // seems only needed with new converse gump but will leave here just in case
		views->set_current_view(last_view);
	}

	Game::get_game()->unpause_user();
	if (conversations_stop_music) {
		SoundManager *sm = Game::get_game()->get_sound_manager();
		if (sm->is_audio_enabled() && sm->is_music_enabled())
			sm->musicPlay();
	}
	Game::get_game()->get_event()->set_mode(MOVE_MODE); // return control to player

	active = false;
	DEBUG(0, LEVEL_INFORMATIONAL, "End conversation\n");
}


/* Returns true if there is input available (placed at `in_str'.)
 */
bool Converse::input() {
	if (scroll->has_input()) {
		Std::string s = scroll->get_input();
		set_input(s);
#ifdef CONVERSE_DEBUG
		DEBUG(0, LEVEL_DEBUGGING, "Converse: INPUT \"%s\"\n\n", get_input().c_str());
#endif
		return (true);
	}
	return (false);
}


/* Output string `s' or the current set output to the scroll view.
 */
void Converse::print(const char *s) {
#ifdef CONVERSE_DEBUG
	DEBUG(0, LEVEL_DEBUGGING, "Converse: PRINT \"%s\"\n\n", s ? s : get_output().c_str());
#endif
	if (s)
		scroll->display_string(s, MSGSCROLL_NO_MAP_DISPLAY);
	else
		scroll->display_string(get_output(), MSGSCROLL_NO_MAP_DISPLAY);
}

void Converse::print_prompt() {
	scroll->display_converse_prompt();
}

/* Get string value of variable `varnum'.
 */
const char *Converse::get_svar(uint8 varnum) {
	if (varnum <= U6TALK_VAR__LAST_ && variables[varnum].sv)
		return ((const char *)variables[varnum].sv);
	return ("");
}


/* Set string value of variable `varnum'.
 */
void Converse::set_svar(uint8 varnum, const char *set) {
	if (varnum <= U6TALK_VAR__LAST_)
		variables[varnum].sv = scumm_strdup(set);
}


/* Show portrait for npc `n'. The name will be shown for actors in the player
 * party, or those the player/avatar has met. The look-string will be shown for
 * anyone else.
 */
void Converse::show_portrait(uint8 n) {
	Game *game = Game::get_game();
	Actor *actor = (n == npc_num) ? npc : actors->get_actor(n);
	const char *nameret = 0;
	if (!actor)
		return;
	bool statue = (gametype == NUVIE_GAME_U6 && n >= 189 && n <= 191);
	if (gametype == NUVIE_GAME_U6 && n == 0) { // Pushme Pullyu
		Actor *real_actor = game->get_actor_manager()->get_actor(130);
		if (real_actor->is_met() || player->get_party()->contains_actor(real_actor))
			nameret = npc_name(130);
		else
			nameret = actors->look_actor(real_actor, false);
	} else if ((actor->is_met() || player->get_party()->contains_actor(actor))
	           && !statue) // they need to display statue of names
		nameret = npc_name(n);
	else
		nameret = actors->look_actor(actor, false);
	if (game->using_new_converse_gump()) {
		if ((game->is_original_plus() && game->get_converse_gump()->W() > game->get_game_width() - game->get_background()->get_border_width())
		        || game->is_orig_style())
			views->close_current_view();
		((ConverseGump *)scroll)->set_actor_portrait(actor);
	} else
		views->set_portrait_mode(actor, nameret);
}


/* Copy the NPC num's name from their conversation script. This is very U6
 * specific.
 * Returns the name as a non-modifiable string of 16 characters maximum. */
const char *Converse::npc_name(uint8 num) {
	ConvScript *temp_script;
	convscript_buffer s_pt;
	aname[15] = '\0';

// FIX (crashing)
//    if(actors->get_actor(num))
//        actors->get_actor(num)->set_name(name);

	if ((num == npc_num) && !_name.empty()) // use NPC name
		strncpy(aname, _name.c_str(), 15);
	else {                            // or load another script
//        uint32 temp_num = num;
		num = load_conv(get_script_num(num)); // get idx number; won't actually reload file
		temp_script = new ConvScript(src, num);
		s_pt = temp_script->get_buffer();
		if (!s_pt)
			return (NULL);

		// read name up to LOOK section, convert "_" to "."
		uint32 c;
		for (c = 0; s_pt[c + 2] != 0xf1 && s_pt[c + 2] != 0xf3 && c <= 14; c++)
			aname[c] = s_pt[c + 2] != '_' ? s_pt[c + 2] : '.';

		aname[c] = '\0';
		delete temp_script;
	}
	return (aname);
}


/* Start checking i/o object for some input, (optionally block all but allowed
 * input) and tell interpreter to wait.
 */
void Converse::poll_input(const char *allowed, bool nonblock) {
	if (allowed_input)
		free(allowed_input);
	allowed_input = NULL;
	allowed_input = (allowed && strlen(allowed)) ? scumm_strdup(allowed) : NULL;

	scroll->set_input_mode(true, allowed_input, nonblock);
	need_input = true;
	conv_i->wait();
}


/* Stop polling i/o, tell interpreter to stop waiting.
 */
void Converse::unwait() {
	need_input = false;
	conv_i->unwait();
}


/* Check talk input and determine if it needs to be handled before being passed
 * to the interpreter.
 * Returns false if the conversation should be stopped.
 */
bool Converse::override_input() {
	bool overide_cheat = Game::get_game()->are_cheats_enabled() && party_all_the_time;
	if (in_str.empty())
		in_str = "bye";
	else if (in_str == "look") {
		print("You see ");
		print(_desc.c_str());
		script->seek(script->pos() - 1); // back to ASK command
	} else if (overide_cheat && in_str == "join") {
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6 // altars and statues
		        && (npc->get_actor_num() >= 189 && npc->get_actor_num() <= 200))
			return true;
		else if (!npc->is_alive()) {
			print("\"How can I join you when I'm dead?\"\n*");
			return true;
		}
		if (!player->get_party()->contains_actor(npc))
			player->get_party()->add_actor(npc);
		print("\"Friends of Nuvie? Sure, I'll come along!\"\n*");
		return (false);
	} else if (overide_cheat && in_str == "leave") {
		if (player->get_party()->contains_actor(npc))
			player->get_party()->remove_actor(npc);
		print("\"For Nuvie!\"\n*");
		return (false);
	}
	return (true);
}

void Converse::collect_input() {
	if (!Game::get_game()->using_new_converse_gump()) {
		print_prompt();
	}
	poll_input();
}

/* If not waiting, continue the active script. If waiting for input, check i/o
 * object (scroll), taking the input if available. Else wait until the scroll's
 * page is unbroken.
 */
void Converse::continue_script() {
	speech->update();

	if (running()) {
		if (!conv_i->waiting())
			conv_i->step();
		else if (need_input && input()) {
			print("\n\n");
			if (!override_input()) {
				need_input = false;
				conv_i->stop();
				stop();
				return;
			}
			// assign value to declared input variable
			if (conv_i->var_input())
				conv_i->assign_input();
			set_svar(U6TALK_VAR_INPUT, get_input().c_str()); // set $Z
			unwait();
		} else if (!need_input && !scroll->get_page_break() && scroll->is_converse_finished()) {
			// if page unbroken, unpause script
			unwait();
		}
		// interpreter has stopped itself
		if (conv_i->end())
			stop();
	}
}


/*** ConvScript ***/

/* Init. and read data from U6Lib.
 */
ConvScript::ConvScript(U6Lib_n *s, uint32 idx) {
	buf = NULL;
	buf_len = 0;
	src = s;
	src_index = idx;

	ref = 0;
	cpy = NULL;

	read_script();

	rewind();
}


/* Init. and use data from another ConvScript.
 */
ConvScript::ConvScript(ConvScript *orig) {
	src = NULL;
	buf = NULL;
	buf_len = 0;
	src_index = 0;
	compressed = false;

	cpy = orig;
	ref = 1;
	cpy->ref += 1;

	rewind();
}


ConvScript::~ConvScript() {
	if (ref == 0)
		free(buf);
	else if (cpy)
		cpy->ref -= 1;
}


/* Read (decode if necessary) the script data (with the pre-set item index) from
 * the loaded converse library.
 */
void ConvScript::read_script() {
	unsigned char *undec_script = 0; // item as it appears in library
	unsigned char *dec_script = 0; // decoded
	uint32 undec_len = 0, dec_len = 0;
	U6Lzw decoder;
	uint8 gametype = src->get_game_type();

	undec_len = src->get_item_size(src_index);
	if (undec_len > 4) {
		undec_script = src->get_item(src_index);
		if (gametype == NUVIE_GAME_U6) {
			// decode
			if (!(undec_script[0] == 0 && undec_script[1] == 0
			        && undec_script[2] == 0 && undec_script[3] == 0)) {
				compressed = true;
				dec_script =
				    decoder.decompress_buffer(undec_script, undec_len, dec_len);
				free(undec_script);
			} else {
				compressed = false;
				dec_len = undec_len - 4;
				dec_script = (unsigned char *)malloc(dec_len);
				memcpy(dec_script, undec_script + 4, dec_len);
				free(undec_script);
			}
		} else {
			// MD/SE compression handled by lzc library
			compressed = false;
			dec_len = undec_len;
			dec_script = undec_script;
		}
	}
	if (dec_len) {
		buf = (convscript_buffer)dec_script;
		buf_len = dec_len;
	}
}


/* Returns 8bit value from current script location in LSB-first form.
 */
converse_value ConvScript::read(uint32 advance) {
	uint8 val = 0;
	while (advance--) {
		val = *buf_pt;
		++buf_pt;
	}
	return (val);
}


/* Returns 16bit value from current script location in LSB-first form.
 */
converse_value ConvScript::read2() {
	uint16 val = 0;
	val = *(buf_pt++);
	val += *(buf_pt++) << 8;
	return (val);
}


/* Returns 32bit value from current script location in LSB-first form.
 */
converse_value ConvScript::read4() {
	uint32 val = 0;
	val = *(buf_pt++);
	val += *(buf_pt++) << 8;
	val += *(buf_pt++) << 16;
	val += *(buf_pt++) << 24;
	return (val);
}

void ConvScript::write2(converse_value val) {
	*(buf_pt++) = val & 0xff;
	*(buf_pt++) = (val >> 8) & 0xff;
	return;
}


uint8 get_converse_gump_type_from_config(Configuration *config) {
	Std::string configvalue;
	config->value("config/general/converse_gump", configvalue, "default");

	if (string_i_compare(configvalue, "default")) {
		return CONVERSE_GUMP_DEFAULT;
	} else if (string_i_compare(configvalue, "u7style")) {
		return CONVERSE_GUMP_U7_STYLE;
	} else if (string_i_compare(configvalue, "wou")) {
		return CONVERSE_GUMP_WOU_STYLE;
	}

	return CONVERSE_GUMP_DEFAULT;
}

} // End of namespace Nuvie
} // End of namespace Ultima
