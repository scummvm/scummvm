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
#include "glk/hugo/resource_archive.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {
namespace Hugo {

Hugo *g_vm;

Hugo::Hugo(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		mainwin(nullptr), currentwin(nullptr), secondwin(nullptr), auxwin(nullptr), 
		runtime_warnings(false), dbnest(0), address_scale(16),
		SCREENWIDTH(0), SCREENHEIGHT(0), FIXEDCHARWIDTH(0), FIXEDLINEHEIGHT(0),
		// heexpr
		evalcount(0), incdec(0), getaddress(0), inexpr(0), inobj(0), last_precedence(0),
		// hemedia
		mchannel(nullptr), schannel(nullptr),
		// hemisc
		game_version(0), object_size(24), game(nullptr), script(nullptr),
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
		parse_called_twice(0), reparse_everything(0), full_buffer(0), recursive_call(false),
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
	g_vm = this;
	strcpy(gamefile, "");

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

	// Miscellaneous
	_savegameSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

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

Hugo::~Hugo() {
	g_vm = nullptr;
}

void Hugo::runGame() {
	hugo_init_screen();

	SetupDisplay();

	strcpy(gamefile, getFilename().c_str());
	strcpy(pbuffer, "");

	ResourceArchive *res = new ResourceArchive();
	SearchMan.add("Resouces", res);

	gameseg = 0;

	LoadGame();

	playGame();

	hugo_cleanup_screen();

	hugo_blockfree(mem);
	mem = nullptr;
	hugo_closefiles();
}

Common::Error Hugo::readSaveData(Common::SeekableReadStream *rs) {
	char testid[3], testserial[9];
	int lbyte, hbyte;
	int j;
	unsigned int k, undosize;
	long i;

	/* Check ID */
	testid[0] = (char)hugo_fgetc(rs);
	testid[1] = (char)hugo_fgetc(rs);
	testid[2] = '\0';
	if (hugo_ferror(rs)) goto RestoreError;

	if (strcmp(testid, id)) {
		GUIErrorMessage(_("Incorrect rs file."));
		goto RestoreError;
	}

	/* Check serial number */
	if (!hugo_fgets(testserial, 9, rs)) goto RestoreError;
	if (strcmp(testserial, serial)) {
		GUIErrorMessage(_("Save file created by different version."));
		goto RestoreError;
	}

	/* Restore variables */
	for (k = 0; k<MAXGLOBALS + MAXLOCALS; k++) {
		if ((lbyte = hugo_fgetc(rs))==EOF || (hbyte = hugo_fgetc(rs))==EOF)
			goto RestoreError;
		var[k] = lbyte + hbyte * 256;
	}

	/* Restore objtable and above */

	if (hugo_fseek(game, objtable*16L, SEEK_SET)) goto RestoreError;
	i = 0;

	while (i<codeend-(long)(objtable*16L)) 	{
		if ((hbyte = hugo_fgetc(rs))==EOF && hugo_ferror(rs)) goto RestoreError;

		if (hbyte == 0) {
			if ((lbyte = hugo_fgetc(rs))==EOF && hugo_ferror(rs)) goto RestoreError;
			SETMEM(objtable*16L+i, (unsigned char)lbyte);
			i++;

			/* Skip byte in game file */
			if (hugo_fgetc(game)==EOF) goto RestoreError;
		} else {
			while (hbyte--) {
				/* Get unchanged game file byte */
				if ((lbyte = hugo_fgetc(game))==EOF) goto RestoreError;
				SETMEM(objtable*16L+i, (unsigned char)lbyte);
				i++;
			}
		}
	}

	/* Restore undo data */
	if ((lbyte = hugo_fgetc(rs))==EOF || (hbyte = hugo_fgetc(rs))==EOF)
		goto RestoreError;
	undosize = lbyte + hbyte*256;

	/* We can only restore undo data if it was saved by a port with
	   the same MAXUNDO as us */
	if (undosize == MAXUNDO) {
		for (k = 0; k < MAXUNDO; k++) {
			for (j=0; j<5; j++) {
				if ((lbyte = hugo_fgetc(rs))==EOF || (hbyte = hugo_fgetc(rs))==EOF)
					goto RestoreError;
				undostack[k][j] = lbyte + hbyte*256;
			}
		}
		if ((lbyte = hugo_fgetc(rs))==EOF || (hbyte = hugo_fgetc(rs))==EOF) goto RestoreError;
		undoptr = lbyte + hbyte*256;
		if ((lbyte = hugo_fgetc(rs))==EOF || (hbyte = hugo_fgetc(rs))==EOF) goto RestoreError;
		undoturn = lbyte + hbyte*256;
		if ((lbyte = hugo_fgetc(rs))==EOF || (hbyte = hugo_fgetc(rs))==EOF) goto RestoreError;
		undoinvalid = (unsigned char)lbyte, undorecord = (unsigned char)hbyte;
	}
	else undoinvalid = true;

	return Common::kNoError;
	
RestoreError:
	return Common::kReadingFailed;
}

Common::Error Hugo::writeGameData(Common::WriteStream *ws) {
	int c, j;
	int lbyte, hbyte;
	long i;
	int samecount = 0;

	/* Write ID */
	if (hugo_fputc(id[0], ws) == EOF || hugo_fputc(id[1], ws) == EOF) goto SaveError;

	/* Write serial number */
	if (hugo_fputs(serial, ws) == EOF) goto SaveError;

	/* Save variables */
	for (c = 0; c<MAXGLOBALS + MAXLOCALS; c++) {
		hbyte = (unsigned int)var[c] / 256;
		lbyte = (unsigned int)var[c] - hbyte * 256;
		if (hugo_fputc(lbyte, ws) == EOF || hugo_fputc(hbyte, ws) == EOF) goto SaveError;
	}

	/* Save objtable to end of code space */

	if (hugo_fseek(game, objtable * 16L, SEEK_SET)) goto SaveError;

	for (i = 0; i <= codeend - (long)(objtable * 16L); i++) {
		if ((lbyte = hugo_fgetc(game)) == EOF) goto SaveError;
		hbyte = MEM(objtable * 16L + i);

		/* If memory same as original game file */
		if (lbyte == hbyte && samecount < 255)
			samecount++;

		/* If memory differs (or samecount exceeds 1 byte) */
		else {
			if (samecount)
				if (hugo_fputc(samecount, ws) == EOF) goto SaveError;

			if (lbyte != hbyte) {
				if (hugo_fputc(0, ws) == EOF) goto SaveError;
				if (hugo_fputc(hbyte, ws) == EOF) goto SaveError;
				samecount = 0;
			}
			else samecount = 1;
		}
	}
	if (samecount)
		if (hugo_fputc(samecount, ws) == EOF) goto SaveError;

	/* Save undo data */

	/* Save the number of turns in this port's undo stack */
	hbyte = (unsigned int)MAXUNDO / 256;
	lbyte = (unsigned int)MAXUNDO - hbyte * 256;
	if (hugo_fputc(lbyte, ws) == EOF || hugo_fputc(hbyte, ws) == EOF)
		goto SaveError;
	for (c = 0; c < MAXUNDO; c++) {
		for (j = 0; j < 5; j++) {
			hbyte = (unsigned int)undostack[c][j] / 256;
			lbyte = (unsigned int)undostack[c][j] - hbyte * 256;
			if (hugo_fputc(lbyte, ws) == EOF || hugo_fputc(hbyte, ws) == EOF)
				goto SaveError;
		}
	}
	if (hugo_fputc(undoptr - (undoptr / 256) * 256, ws) == EOF || hugo_fputc(undoptr / 256, ws) == EOF)
		goto SaveError;
	if (hugo_fputc(undoturn - (undoturn / 256) * 256, ws) == EOF || hugo_fputc(undoturn / 256, ws) == EOF)
		goto SaveError;
	if (hugo_fputc(undoinvalid, ws) == EOF || hugo_fputc(undorecord, ws) == EOF)
		goto SaveError;

	return Common::kNoError;

SaveError:
	return Common::kWritingFailed;
}

} // End of namespace Hugo
} // End of namespace Glk
