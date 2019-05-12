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

#include "glk/hugo/hugo.h"

namespace Glk {
namespace Hugo {

Hugo::Hugo(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		mainwin(nullptr), currentwin(nullptr), secondwin(nullptr), auxwin(nullptr), 
		runtime_warnings(false), dbnest(0), address_scale(16),
		SCREENWIDTH(0), SCREENHEIGHT(0), FIXEDCHARWIDTH(0), FIXEDLINEHEIGHT(0),
		// heexpr
		evalcount(0), incdec(0), getaddress(0), inexpr(0), inobj(0), last_precedence(0),
		// hemedia
		mchannel(nullptr), schannel(nullptr),
		// hemisc
		game_version(0), object_size(0), game(nullptr), script(nullptr), save(nullptr),
		playback(nullptr), record(nullptr), io(nullptr), ioblock('\0'), ioerror('\0'),
		codestart(0), objtable(0), eventtable(0), proptable(0), arraytable(0), dicttable(0), 
		syntable(0), initaddr(0), mainaddr(0), parseaddr(0), parseerroraddr(0), 
		findobjectaddr(0), endgameaddr(0), speaktoaddr(0), performaddr(0), 
		objects(0), events(0), dictcount(0), syncount(0), mem(nullptr), loaded_in_memory(true),
		defseg(0), gameseg(0), codeptr(0), codeend(0), currentpos(0), currentline(0), full(0),
		def_fcolor(0), def_bgcolor(0), def_slfcolor(0), def_slbgcolor(0), fcolor(0), bgcolor(0),
		icolor(0), default_bgcolor(0), currentfont(0), capital(0), textto(0),
		physical_windowwidth(0), physical_windowheight(0), physical_windowtop(0),
		physical_windowleft(0), physical_windowbottom(0), physical_windowright(0),
		inwindow(0), charwidth(0), lineheight(0), current_text_x(0), current_text_y(0),
		skipping_more(false), undoptr(0), undoturn(0), undoinvalid(0), undorecord(0),
		context_commands(0), in_valid_window(false), glk_fcolor(DEF_FCOLOR), glk_bgcolor(DEF_BGCOLOR),
		mainwin_bgcolor(0), glk_current_font(0), just_cleared_screen(false), secondwin_bottom(0),
		// heobject
		display_object(-1), display_needs_repaint(0), display_pointer_x(0), display_pointer_y(0),
		// heparse
		words(0), parsed_number(0), remaining(0), xverb(0), starts_with_verb(0),
		grammaraddr(0), obj_parselist(nullptr), domain(0), odomain(0), objcount(0),
		parse_allflag(false), pobjcount(0), pobj(0), obj_match_state(0), object_is_number(0),
		objgrammar(0), objstart(0), objfinish(0), addflag(false), speaking(0), oopscount(0),
		parse_called_twice(0), reparse_everything(0), full_buffer(false), recursive_call(false),
		parse_location(0),
		// heres
		resource_file(nullptr), extra_param(0), resource_type(0),
		// herun
		arguments_passed(0), ret(0), retflag(0), during_player_input(false), override_full(0),
		game_reset(false), stack_depth(0), tail_recursion(0), tail_recursion_addr(0),
		last_window_top(0), last_window_bottom(0), last_window_left(0), last_window_right(0),
		lowest_windowbottom(0), physical_lowest_windowbottom(0), just_left_window(false),
		// heset
		arrexpr(0), multiprop(0), set_value(0)
#if defined (DEBUGGER)
		, debug_eval(false), debug_eval_error(false), debugger_step_over(false),
		debugger_finish(false), debugger_run(false), debugger_interrupt(false),
		debugger_skip(false), runtime_error(false), currentroutine(false),
		complex_prop_breakpoint(false), trace_complex_prop_routine(false), routines(0),
		properties(0), current_locals(0), this_codeptr(0), debug_workspace(0), attributes(0),
		original_dictcount(0), buffered_code_lines(0), debugger_has_stepped_back(false),
		debugger_step_back(false), debugger_collapsing(0), runaway_counter(0), history_count(0),
		active_screen(0), step_nest(0), history_last(0)
#endif
		{
	// heexpr
	Common::fill(&eval[0], &eval[MAX_EVAL_ELEMENTS], 0);
	Common::fill(&var[0], &var[MAXLOCALS + MAXGLOBALS], 0);

	// hemedia
	Common::fill(&resids[0][0], &resids[2][0], 0);
	numres[0] = numres[1] = 0;

	// hemisc		
	Common::fill(&context_command[0][0], &context_command[MAX_CONTEXT_COMMANDS][0], 0);
	Common::fill(&id[0], &id[3], '\0');
	Common::fill(&serial[0], &serial[9], '\0');
	Common::fill(&pbuffer[0], &pbuffer[MAXBUFFER * 2 + 1], 0);
	Common::fill(&undostack[0][0], &undostack[MAXUNDO][0], 0);
	
	// heparse
	Common::fill(&buffer[0], &buffer[MAXBUFFER + MAXWORDS], '\0');
	Common::fill(&errbuf[0], &errbuf[MAXBUFFER + 1], 0);
	Common::fill(&line[0], &line[1025], 0);
	Common::fill(&word[0], &word[MAXWORDS + 1], (char *)nullptr);
	Common::fill(&wd[0], &wd[MAXWORDS + 1], 0);
	Common::fill(&parseerr[0], &parseerr[MAXBUFFER + 1], '\0');
	Common::fill(&parsestr[0], &parsestr[MAXBUFFER + 1], '\0');
	Common::fill(&objlist[0], &objlist[MAXOBJLIST], 0);
	Common::fill(&objword_cache[0], &objword_cache[MAXWORDS], 0);
	Common::fill(&oops[0], &oops[MAXBUFFER + 1], '\0');
	Common::fill(&punc_string[0], &punc_string[64], '\0');

	// heres
	Common::fill(&loaded_filename[0], &loaded_filename[MAX_RES_PATH], '\0');
	Common::fill(&loaded_resname[0], &loaded_resname[MAX_RES_PATH], '\0');

	// herun
	Common::fill(&passlocal[0], &passlocal[MAXLOCALS], 0);

	// heset
	game_title[0] = '\0';

#ifdef DEBUGGER
	debug_line[0] = '\0';
	Common::fill(&objectname[0], &objectname[MAX_OBJECT], (char *)nullptr);
	Common::fill(&propertyname[0], &propertyname[MAX_PROPERTY], (char *)nullptr);
	Common::fill(&codeline[0][0], &codeline[9][0], 0);
	Common::fill(&localname[0][0], &localname[9][0], 0);
	Common::fill(&code_history[0], &code_history[MAX_CODE_HISTORY], 0);
	Common::fill(&dbnest_history[0], &dbnest_history[MAX_CODE_HISTORY], 0);
#endif
}

void Hugo::runGame() {
	hugo_init_screen();

	SetupDisplay();

	strcpy(gamefile, getFilename().c_str());
	strcpy(pbuffer, "");

	gameseg = 0;

	LoadGame();

	playGame();

	hugo_cleanup_screen();

	hugo_blockfree(mem);
	mem = nullptr;
	hugo_closefiles();
}

Common::Error Hugo::loadGameData(strid_t file) {
	// TODO
	return Common::kNoError;
}

Common::Error Hugo::saveGameData(strid_t file, const Common::String &desc) {
	// TODO
	return Common::kNoError;
}

} // End of namespace Hugo
} // End of namespace Glk
