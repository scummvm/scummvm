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

void Hugo::RunDo() {
	long skip, enterptr;

	enterptr = ++codeptr;
	skip = PeekWord(codeptr);       /* remember the skip distance */
	codeptr+=2;

	SetStackFrame(stack_depth, DOWHILE_BLOCK, skip+enterptr, codeptr);

#if defined (DEBUGGER)
	dbnest++;
#endif
}

void Hugo::RunEvents() {
	int i, tempundo, flag, temp_ret;
	int eventin, tempself;
	int templocals[MAXLOCALS];
	int temp_stack_depth;
	int temp_parse_location;
	long tempptr, eventaddr;
#if defined (DEBUGGER)
	int tempdbnest;
#endif

	tempundo = undorecord;
	undorecord = true;

	tempptr = codeptr;
	tempself = var[self];
	temp_ret = ret;
	temp_parse_location = parse_location;

	parse_location = var[location];	/* for Available() */

	temp_stack_depth = stack_depth;

	for (i=0; i<MAXLOCALS; i++)
		templocals[i] = var[MAXGLOBALS+i];

	for (i=0; i<events; i++)
	{
		defseg = eventtable;

		eventin = PeekWord(2 + i * 4);
		eventaddr = (long)PeekWord(2 + i * 4 + 2)*address_scale;
		var[self] = eventin;

		domain = 0, flag = 0;

#if defined (DEBUGGER)
		/* Prevent premature stopping */
		if (debugger_step_over && !debugger_finish)
			debugger_run = true;
#endif
		if (eventin==0 || GrandParent(eventin)==GrandParent(var[player]))
			flag = 1;

		/* true is to signal a non-grammar call */
		else if (Available(eventin, true))
			flag = 1;

		if (flag)
		{
			PassLocals(0);

			SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);
#if defined (DEBUGGER)
			tempdbnest = dbnest;
			DebugRunRoutine(eventaddr);
			dbnest = tempdbnest;
#else
			RunRoutine(eventaddr);
#endif
			stack_depth = temp_stack_depth;

			retflag = 0;
			if (var[endflag]) break;
		}
	}

	for (i=0; i<MAXLOCALS; i++)
		var[MAXGLOBALS+i] = templocals[i];

	codeptr = tempptr;
	parse_location = temp_parse_location;
	var[self] = tempself;
	undorecord = (char)tempundo;
	ret = temp_ret;
}

void Hugo::playGame() {
	char jw = 0;                    /* just wrote undo info */
	char wasxverb = 0, newinput;
	int i, flag, mc, lastspeaking = 0, startlocation;
	
#ifdef USE_TEXTBUFFER
	TB_Init();
#endif

#ifdef PALMOS
	if (AutoResume())
	{
		goto FreshInput;
	}
#endif

	/* Set up initial screen position */
	hugo_settextpos(1, physical_windowheight/lineheight);
	display_needs_repaint = false;
	full = 0;

	/* Load globals */
	defseg = arraytable;
	for (i=0; i<MAXGLOBALS; i++)
		var[i] = PeekWord(i*2);

	/* Reset the speaking-to variable */
	speaking = 0;

	if (game_version < 22)
	{
		passlocal[0] = objects;
#if defined (ACTUAL_LINELENGTH)
		passlocal[1] = ACTUAL_LINELENGTH();
#else
		passlocal[1] = physical_windowwidth/FIXEDCHARWIDTH;
#endif
	}

#if defined (DEBUGGER)
RestartDebugger:

	dictcount = original_dictcount;		/* see hd.c */
	
	/* If no gamefile is loaded, jump immediately to the debugger
	   interrupt function.
	*/
	if (game==nullptr) Debugger();
#endif

	stack_depth = RESET_STACK_DEPTH;

	InitGame();

	undoptr = 0;
	undoturn = 0;
	undoinvalid = 1;
	undorecord = 0;

Start:
	stack_depth = 0;
	strcpy(errbuf, "");
	strcpy(oops, "");

#if defined (GLK)
	// Handle any savegame selected directly from the ScummVM launcher
	if (_savegameSlot != -1) {
		if (loadGameState(_savegameSlot).getCode() != Common::kNoError) {
			GUIErrorMessage("Loading failed");
			_savegameSlot = -1;
		}
	}
#endif

	do
	{
		if (shouldQuit())
			return;

		if (xverb==0)
		{
			undorecord = true;

			SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

			PassLocals(0);

#if defined (DEBUGGER)
			currentroutine = mainaddr;
			window[VIEW_CALLS].count = 0;

			/* If there is no Main routine */
			if (currentroutine==0) goto NormalTermination;

			DebugRunRoutine((long)mainaddr*address_scale);
#else
			RunRoutine((long)mainaddr*address_scale);
#endif
			undorecord = false;
			if (retflag)
				break;
		}

		if (!undoinvalid && !wasxverb)
		{
			undorecord = true;
			SaveUndo(0, undoturn, 0, 0, 0);
			undorecord = false;
			undoturn = 0;
		}
		else if (undoinvalid)
		{
			undoptr = 0;
			undoinvalid = 0;
			undoturn = 0;
			undorecord = true;
			SaveUndo(0, 0, 0, 0, 0);
			SaveUndo(0, 0, 0, 0, 0);
			undorecord = false;
		}

		xverb = true;

		jw = 1;
		if (var[endflag])
			break;
		jw = 0;

		full_buffer = 0;

	/* Endless game loop begins here */

		do
		{
			PassLocals(0);

			/* If there's nothing waiting to be finished in the
			   input buffer:
			*/
			if (!remaining)
			{
				newinput = true;
				speaking = 0;
				do
				{
FreshInput:
					if (full_buffer != 1)
					{
						newinput = true;
						speaking = 0;
						var[actor] = var[player];
#if defined (DEBUGGER)
						AddStringtoCodeWindow("[Waiting for input]");
						buffered_code_lines = FORCE_REDRAW;
						debugger_has_stepped_back = false;
						window[VIEW_LOCALS].changed = true;
#endif
#if defined (GLK)
						if (_savegameSlot != -1) {
							// Trigger a "look" command so that players will get some initial text
							// after loading a savegame directly from the launcher
							_savegameSlot = -1;
							strcpy(buffer, "look");
						}
						else
#endif
					if (!playback)
						{
							GetCommand();
						}
						else
						{
							if (!hugo_fgets(buffer, MAXBUFFER, *playback))
							{
								if (hugo_fclose(playback))
									FatalError(READ_E);
								playback = nullptr;
								GetCommand();
							}
							else
							{
								/* Remove CR/LF */
/*
								buffer[strlen(buffer)-1] = '\0';
								if (buffer[strlen(buffer)-1]==0x0d)
									buffer[strlen(buffer)-1] = '\0';
*/
								while (buffer[strlen(buffer)-1]==0x0d || buffer[strlen(buffer)-1]==0x0a)
									buffer[strlen(buffer)-1] = '\0';
								sprintf(line, "\n%s%s", GetWord(var[prompt]), buffer);
								if (script)
									/* fprintf() this way for Glk */
									script->putBuffer("\n", 1);
#if defined (SCROLLBACK_DEFINED)
								hugo_sendtoscrollback("\n");
#endif
								AP(line);
							}
						}
#if defined (DEBUGGER)
						if (debugger_collapsing)
							goto NormalTermination;
						runaway_counter = 0;
#endif
						if (shouldQuit())
							return;

						SeparateWords();

						if (record)
						{
							for (i=1; i<=words; i++)
							{
								if (!strcmp(word[i], "."))
								{
									/* fprintf() this way for Glk */
									if (hugo_fprintf(record, "%s", "\n")<0)
										FatalError(WRITE_E);
									if (i==words) goto RecordedNewline;
								}
								else if (hugo_fputs(word[i], record)<0
									|| hugo_fprintf(record, "%s", " ")<0)
								{
									FatalError(WRITE_E);
								}
							}
							if (hugo_fprintf(record, "%s", "\n")<0) FatalError(WRITE_E);
RecordedNewline:;
						}
					}
					else full_buffer = 0;

					if (!strcmp(buffer, "") || buffer[0]=='.')
					{
						strcpy(parseerr, "");

						/* "What?" */
						ParseError(0, 0);
						goto FreshInput;
					}
				}

				/* Loop until valid input */
				while (Parse()==false && strcmp(buffer, ""));
			}


			/* Else if there's something left in the input buffer */
			else
			{
				newinput = false;

				/* Erase the just-parsed command, and check to
				   to see if what's left is just blanks
				*/
				while (words > remaining)
					KillWord(1);
				flag = false;
				for (i=1; i<=words; i++)
					if (wd[i]!=0) flag = true;
				if (!flag)
					goto FreshInput;

				if (words) AP("");

				if (Parse()==false)
				{
					mc = false;
					goto Skipmc;
				}
			}

			/* Run the user Parse routine if one exists */
			CallLibraryParse();

			reparse_everything = false;
			do
			{
				mc = MatchCommand();
				if (mc==false)
				{
					remaining = 0;
				}
			} while (reparse_everything && !mc);
Skipmc:;
		}
		while (!mc);

		if (!xverb) undorecord = true;

		wasxverb = xverb;

		/* If there's an unknown string to be put in parse$ */
		if (parsestr[0]!='\0')
		{
			if (parsestr[0]=='\"')
			{
				strcpy(parseerr, Right(parsestr, strlen(parsestr)-1));
				if (parseerr[strlen(parseerr)-1]=='\"')
					parseerr[strlen(parseerr)-1] = '\0';
			}
		}
		else
			strcpy(parseerr, "");

		/* default actor */
		var[actor] = var[player];

		if (!newinput && lastspeaking) speaking = lastspeaking;

		if (var[verbroutine]!=0 || (speaking))
		{
			/* If command addresses an object/char. */
			if (speaking)
			{
				lastspeaking = speaking;
				var[actor] = speaking;

				/* If user Speakto routine exists */
				if (speaktoaddr)
				{
					if (objcount) var[object] = objlist[0];
					ret = 0;

					SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

					passlocal[0] = speaking;
					PassLocals(1);
#if defined (DEBUGGER)
					DebugRunRoutine((long)speaktoaddr*address_scale);
#else
					RunRoutine((long)speaktoaddr*address_scale);
#endif
					if (ret==0)
					{
						remaining = 0;
						xverb = true;
					}
					retflag = 0;
				}

				else
				{
					/* "...start with a verb..." */
					ParseError(2, 0);
					xverb = true;
				}
				
				/* reset actor */
				var[actor] = var[player];
			}

			/* Regular old vanilla command: */
			else
			{
				speaking = 0;
				lastspeaking = 0;

	/* As of v2.5, the Perform junction routine takes care of calling the
	   before routines, verbroutine, etc.
	*/
				if (game_version>=25 && performaddr!=0)
				{
					i = 0;
NextPerform:
					if (objcount) var[object] = objlist[i];

					/* Have to do this before passing locals, in case
					   Name() ends up calling a routine (which would
					   trash passlocal[])
					*/
					if (parseerr[0]=='\0' && parsestr[0]=='\0')
						strcpy(parseerr, Name(objlist[i]));

					/* Set up arguments for Perform */
					passlocal[0] = var[verbroutine];
					passlocal[1] = var[object];
					passlocal[2] = var[xobject];

					/* 'queue' argument, >1 if objcount > 1, or if
					   "all" has been used to refer to object(s) */
					passlocal[3] = (objcount>1)?(i+1):(parse_allflag?1:0);
					/* -1 if object is a digit */
					if (object_is_number) passlocal[3] = (short)-1;

					/* 'isxverb' argument */
					if (game_version>=31 && xverb)
						passlocal[4] = 1;

					obj_match_state = -1;
					startlocation = var[location];
					ret = 0;

					SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

					PassLocals(4);
#if defined (DEBUGGER)
					DebugRunRoutine((long)performaddr*address_scale);
#else
					RunRoutine((long)performaddr*address_scale);
#endif
					if (ret==0)
					{
						remaining = 0;
						xverb = true;
					}
					retflag = 0;

					/* Break if endflag is set or if the location has
					   changed from the first call to Perform
					*/
					if (var[endflag] || startlocation!=var[location])
						goto EndofCommand;

					if (objcount>1 && ++i<objcount)
						goto NextPerform;
			    	}

	/* v2.4 or earlier had to call the verb loop via the engine 
	   (as does v2.5 with no Perform routine */

				/* One or more objects specified */
				else if (objcount > 0)  /* "if (objcount > 0" for pre-v2.5 */
				{
					obj_match_state = 1;
					startlocation = var[location];
					for (i=0; i<objcount; i++)
					{
						if (parseerr[0]=='\0' && parsestr[0]=='\0')
							strcpy(parseerr, Name(objlist[i]));

						if (ValidObj(objlist[i]) &&
							((objcount>1 && objlist[i]!=var[xobject]) || objcount==1))
						{
							var[object] = objlist[i];
							if (GetProp(var[player], before, 1, 0)==0)
							if (GetProp(var[location], before, 1, 0)==0)
							if (GetProp(var[xobject], before, 1, 0)==0)
							{
								/* If multiple objects are specified, print
								"name:  " for each:
								*/
								if (objcount > 1)
								{
									sprintf(line, "%s:  \\;", Name(var[object]));
									AP(line);
								}

								obj_match_state = 0;

								if ((object_is_number) || GetProp(var[object], before, 1, 0)==0)
								{
									obj_match_state = -1;
									ret = 0;

									SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

									PassLocals(0);
#if defined (DEBUGGER)
									DebugRunRoutine((long)var[verbroutine]*address_scale);
#else
									RunRoutine((long)var[verbroutine]*address_scale);
#endif
									if (ret==0)
									{
										remaining = 0;
										xverb = true;
									}
									retflag = 0;

									GetProp(var[player], after, 1, 0);
									GetProp(var[location], after, 1, 0);
								}
							}
						}
						if (var[endflag] || var[location]!=startlocation)
						break;
					}
				}

				/* No object(s) specified */
				else
				{
					if (GetProp(var[player], before, 1, 0)==0)
					{
						if (GetProp(var[location], before, 1, 0)==0)
						{
							ret = 0;

							SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

							PassLocals(0);
#if defined (DEBUGGER)
							DebugRunRoutine((long)var[verbroutine]*address_scale);
#else
							RunRoutine((long)var[verbroutine]*address_scale);
#endif
							if (ret==0)
							{
								remaining = 0;
								xverb = true;
							}
							retflag = 0;

							GetProp(var[player], after, 1, 0);
							GetProp(var[location], after, 1, 0);
						}
		        		}
				}

	/* (end of pre-v2.5 verbroutine-calling) */

			}
		}
EndofCommand:
		if (var[endflag])
			break;

		undorecord = false;
	}
	while (true);   /* endless loop back to start */

	undorecord = false;

	if (var[endflag]==-1)
#if defined (DEBUGGER)
		goto NormalTermination;
#else
		return;
#endif

	if (!jw)
	{
		undorecord = true;
		SaveUndo(0, undoturn, 0, 0, 0);
		undorecord = false;
		undoturn = 0;
	}

	if (playback)
	{
		if (hugo_fclose(playback)) FatalError(READ_E);
		playback = nullptr;
	}

	Flushpbuffer();


	/* Run the user Endgame routine if one exists */

#if defined (DEBUGGER)
	if (endgameaddr && !debugger_collapsing)
#else
	if (endgameaddr)
#endif
	{
		passlocal[0] = var[endflag];

		SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

		ret = 0;
		var[endflag] = 0;
		PassLocals(0);
#if defined (DEBUGGER)
		DebugRunRoutine((long)endgameaddr*address_scale);
#else
		RunRoutine((long)endgameaddr*address_scale);
#endif
		retflag = false;
	}
	else
		ret = 0;

	xverb = true;
	wasxverb = true;
	if (ret) goto Start;

	/* Stop all audio after running EndGame */
	hugo_stopmusic();
	hugo_stopsample();
	
	/* The debugger will reset endflag anyway, but we need it to signal ports
	   (like Palm) that the game loop has exited.
	*/
	var[endflag] = -1;

#if defined (DEBUGGER)

NormalTermination:

	/* Normal program termination doesn't exit the debugger. */

	debugger_interrupt = true;
	debugger_run = false;
	var[endflag] = false;

	xverb = false;

	SwitchtoDebugger();
	UpdateDebugScreen();

	if (debugger_collapsing!=2)
	{
		DebugMessageBox("Program Exiting", "Normal program termination");

	}

	debugger_collapsing = false;

	if ((game!=nullptr) && !RunRestart())
		DebugMessageBox("Restart Error", "Unable to restart");

	SwitchtoGame();

	history_count = 0;
	window[VIEW_CALLS].count = 0;

	for (i=0; i<(int)window[CODE_WINDOW].count; i++)
		free(codeline[i]);
	window[CODE_WINDOW].count = 0;

	/* Force Code window redraw */
	buffered_code_lines = FORCE_REDRAW;

	goto RestartDebugger;
#endif
}

void Hugo::RunIf(char ovrride) {
	char t, tempinexpr;
	long enterptr, skip;

	switch (t = MEM(codeptr))
	{
		case CASE_T:
		case IF_T:
		case ELSEIF_T:
		case WHILE_T:
		case FOR_T:
		{
			codeptr++;
			enterptr = codeptr;

			/* Remember the skip distance */
			skip = PeekWord(codeptr);
			codeptr += 2;

			/* Check if we've already done an elseif */
			if (ovrride && (t == ELSEIF_T)) {
				codeptr = skip+enterptr;
				return;
			}

			/* Read the expression */
			tempinexpr = inexpr;
			inexpr = 1;
			SetupExpr();
			inexpr = tempinexpr;

			/* If the expression is false, skip the
			   conditional block
			*/
			if (EvalExpr(0)==0)
			{
				codeptr = skip+enterptr;
				return;
			}

			/* Protect the stack if jumping backward */
			if (MEM(codeptr)==JUMP_T)
			{
				if ((long)(PeekWord(codeptr+1)*address_scale) < codeptr)
					if (--stack_depth < 0) stack_depth = 0;
			}

			/* Continue on into the conditional block if
			   the expression evaluated to non-zero
			*/
PasstoBlock:
			if (t==WHILE_T || t==FOR_T)
				SetStackFrame(stack_depth, CONDITIONAL_BLOCK, skip+enterptr, 0);

			else	/* no 'break' parameter */
				SetStackFrame(stack_depth, CONDITIONAL_BLOCK, 0, 0);
#if defined (DEBUGGER)
			dbnest++;
#endif
			return;
		}
		case ELSE_T:
		{
			skip = PeekWord(++codeptr);
			enterptr = codeptr;
			codeptr += 2;

			if (ovrride)
			{
				codeptr = skip+enterptr;
				return;
			}

			if (MEM(codeptr)==JUMP_T)
			{
				if ((long)(PeekWord(codeptr+1)*address_scale) < codeptr)
					if (--stack_depth < 0) stack_depth = 0;
			}

			goto PasstoBlock;
		}
	}
}

void Hugo::RunInput() {
	int i;

	strcpy(parseerr, "");

	Flushpbuffer();

	if (icolor==-1) icolor = fcolor;	/* check unset input color */

	hugo_getline("");

#if defined (DEBUGGER)
	if (debugger_collapsing) return;
#endif

	strcpy(buffer, Rtrim(strlwr(buffer)));

	SeparateWords();

	for (i=1; i<=words; i++)
	{
		wd[i] = FindWord(word[i]);

		/* If a word isn't in the dictionary */
		if (wd[i]==UNKNOWN_WORD)
		{
			wd[i] = 0;
			strcpy(parseerr, word[i]);
			if (parseerr[0]=='\"')
			{
				strcpy(parseerr, Right(parseerr, strlen(parseerr)-1));
				if (parseerr[strlen(parseerr)-1]=='\"')
					parseerr[strlen(parseerr)-1] = '\0';
			}
		}
	}
	currentpos = 0;	/* left margin */
	remaining = 0;
}

void Hugo::RunMove() {
	int obj, p;
#if defined (DEBUGGER)
	char out_of_range = 0;
#endif

	switch (MEM(codeptr))
	{
		case MOVE_T:
		{
			codeptr++;
			obj = GetValue();

#if defined (DEBUGGER)
			if (!CheckinRange(obj, objects, "object"))
				out_of_range = true;
			else
#endif
			SaveUndo(MOVE_T, obj, Parent(obj), 0, 0);

			codeptr++;              /* skip "to" */
			p = GetValue();

#if defined (DEBUGGER)
			if (!CheckinRange(p, objects, "object"))
				out_of_range = true;

			if (!out_of_range)
#endif
			MoveObj(obj, p);
			break;
		}

		case REMOVE_T:
		{
			codeptr++;
			obj = GetValue();

#if defined (DEBUGGER)
			if (!CheckinRange(obj, objects, "object"))
				out_of_range = true;

			else
#endif
			SaveUndo(MOVE_T, obj, Parent(obj), 0, 0);

#if defined (DEBUGGER)
			if (!out_of_range)
#endif
			MoveObj(obj, 0);        /* move to parent 0 */
			break;
		}
	}

	if (game_version>=23) codeptr++; /* eol */
}

void Hugo::RunPrint() {
	char number = 0, hexnumber = 0;
	int a;
	int i, l;

	codeptr++;

	while (MEM(codeptr) != EOL_T)
	{
		strcpy(line, "");

		switch (MEM(codeptr))
		{
			case NEWLINE_T:
			{
				codeptr++;
				if (currentpos+hugo_textwidth(pbuffer)!=0)
					AP("");
				if (MEM(codeptr)==SEMICOLON_T) codeptr++;
				continue;
			}

			case TO_T:
			{
				codeptr++;

#ifdef GLK
				// WORKAROUND: Glk uses a non-fixed width font for displaying
				// text, so get the length, but don't allow long runs of spaces
				if ((a = GetValue()) > 20)
					a = 0;

#elif !defined (ACTUAL_LINELENGTH)
				if ((a = GetValue()) > physical_windowwidth/FIXEDCHARWIDTH)
					a = physical_windowwidth/FIXEDCHARWIDTH;
#else
				if ((a = GetValue()) > ACTUAL_LINELENGTH())
				{
					double ratio;

					ratio = (physical_windowwidth/FIXEDCHARWIDTH) / a;
					a = (int)(ACTUAL_LINELENGTH() / ratio);
				}
#endif
				strcpy(line, "");
				l = 0;
				if (a*FIXEDCHARWIDTH >
					hugo_textwidth(pbuffer)+currentpos-hugo_charwidth(' '))
				{
					for (i=hugo_textwidth(pbuffer)+currentpos;
#ifdef NO_TERMINAL_LINEFEED
				        	i<a*FIXEDCHARWIDTH;
#else
				        	i<a*FIXEDCHARWIDTH && i<physical_windowright;
#endif
						i+=hugo_charwidth(' '))
					{
						line[l++] = FORCED_SPACE;
						line[l] = '\0';
					}

				}
				break;
			}

			case CAPITAL_T:
			{
				codeptr++;
				capital = 1;
				continue;
			}

			case NUMBER_T:
			{
				codeptr++;
				number = 1;
				continue;
			}

			case HEX_T:
			{
				codeptr++;
				number = 1;
				hexnumber = 1;
				continue;
			}

			case STRINGDATA_T:
			{
				codeptr++;
				if (game_version >= 24)
					l = PeekWord(codeptr++);
				else
					l = Peek(codeptr);
				for (i=0; i<l; i++)
					line[i] = (char)(MEM(++codeptr) - CHAR_TRANSLATION);
				line[i] = '\0';
				codeptr++;
				break;
			}

			/* Anything else is treated as a value */
			default:
			{
				a = GetValue();
				if (!number)
				{
					strcpy(line, GetWord(a));
				}
				else
				{
					if (!hexnumber)
					{
						if (capital)
							itoa((unsigned int)a, line, 10);
						else
							itoa(a, line, 10);
						capital = 0;
					}
					else
						sprintf(line, "%X", a);

					number = 0;
					hexnumber = 0;
				}
				break;
			}
		}

		if (MEM(codeptr)==SEMICOLON_T)
		{
			codeptr++;
			strcat(line, "\\;");
		}
		if (capital)
		{
			capital = 0;
			if ((unsigned)line[0]<128)
				line[0] = (char)toupper((int)line[0]);
			else
			{
				/* Special conversion for non-Latin1
				   (>127) lowercase characters
				*/
				char diff;
				diff = 'a'-'A';
				if ((unsigned)line[0]+diff<=255 && (unsigned)line[0]-diff>127)
					line[0] -= diff;
			}
		}

		AP(line);
	}

	codeptr++;
}

int Hugo::RunRestart() {
	unsigned int a;
	long i = 0;
	Common::SeekableReadStream *file;

#ifndef LOADGAMEDATA_REPLACED

	remaining = 0;

#if !defined (GLK)	/* with Glk, game is never closed */
	/* Use file instead of game, just in case the call fails */
	if (!(file = HUGO_FOPEN(gamefile, "rb"))) goto RestartError;
#else
	file = game;
#endif

	if (hugo_fseek(file, (objtable-gameseg)*16, SEEK_SET)) goto RestartError;

	i = (objtable-gameseg)*16L;
	do
	{
		int val;

		val = hugo_fgetc(file);
		SETMEM(i++, (unsigned char)val);
		if (val==EOF || hugo_ferror(file)) goto RestartError;
	}
	while (i < codeend);

#if !defined (GLK)
	if (fclose(file)) FatalError(READ_E);
#endif

#else
	if (!(file = HUGO_FOPEN(gamefile, "rb"))) goto RestartError;
	LoadGameData(true);
	fclose(file);
#endif	/* LOADGAMEDATA_REPLACED */

	defseg = arraytable;
	for (a=0; a<MAXGLOBALS; a++)
		var[a] = PeekWord(a*2);

	i = codeptr;

	if (game_version < 22)
	{
		passlocal[0] = objects;
#if defined (ACTUAL_LINELENGTH)
		passlocal[1] = ACTUAL_LINELENGTH();
#else
		passlocal[1] = physical_windowwidth/FIXEDCHARWIDTH;
#endif
	}

#if defined (DEBUGGER)
	/* A restart can happen mid-playback from the debugger */
	if (playback)
	{
		if (hugo_fclose(playback))
			FatalError(READ_E);
		playback = nullptr;
	}

	if (active_screen!=DEBUGGER)
#endif
	{
		InitGame();

		SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);

		PassLocals(0);
#if defined (DEBUGGER)
		DebugRunRoutine((long)mainaddr*address_scale);
#else
		RunRoutine((long)mainaddr*address_scale);
#endif
		retflag = 0;

		codeptr = i;

		undoptr = 0;
		undoturn = 0;
		undoinvalid = 1;
	}

	return 1;

RestartError:
#if !defined (GLK)
	if (fclose(file)) FatalError(READ_E);
#endif

	return 0;
}

int Hugo::RunRestore() {
#if !defined (GLK)
	save = nullptr;

	/* stdio implementation */
	hugo_getfilename("to restore", savefile);

#if defined (DEBUGGER)
	if (debugger_collapsing) return 1;
#endif
	if (!strcmp(line, "")) return 0;
	if (!(save = HUGO_FOPEN(line, "r+b"))) return 0;

	if (!RestoreGameData()) goto RestoreError;

	if (hugo_fclose(save)) FatalError(READ_E);
	save = nullptr;

	strcpy(savefile, line);
#else
	/* Glk implementation */
	if (loadGame().getCode() != Common::kNoError)
		goto RestoreError;

#endif	/* GLK */

	game_reset = true;

	return 1;

RestoreError:
#if !defined (GLK)
	if ((save) && hugo_fclose(save)) FatalError(READ_E);
	save = nullptr;
#endif
	game_reset = false;
	return 0;
}

void Hugo::RunRoutine(long addr) {
	int null_count;        /* for reading to next address boundary     */
	char tempinexpr;
	int i, t, len, xpos, ypos;
	int initial_stack_depth, tempret;
	unsigned int routineptr = 0;
	long textaddr;
#if defined (DEBUGGER)
	char wascalled = 0;     /* distinguish routine calls from 'window'
				   blocks, etc. */
	const char *called_from;
	char trace_comp_prop = 0;
	unsigned char param_type;
	long param_start;
	unsigned int old_currentroutine = 0;

	/* Because <debugdata#> and <label#> in-code data is
	   not decompiled
	*/
	int broke_on_nonstatement = 0;
#endif

	/* If routine doesn't exist */
	if (addr==0L) return;
	
	initial_stack_depth = stack_depth;
	inexpr = 0;

#if !defined (DEBUGGER)
#if defined (DEBUG_CODE)
/*
	if (trce)
	{
		if (codeptr != addr)
		{
			sprintf(line, "[ROUTINE:  $%6s]", PrintHex(addr));
			AP(line);
			wascalled = 1;
		}
	}
*/
#endif
#endif

#if defined (DEBUGGER)

/*
 *      First see what debugger information has to be set up upon calling
 *      this block of code
 */

	/* If this is a routine call vs. other block, codeptr will be
	   different from addr.
	*/
	if (codeptr != addr)
	{
		wascalled = 1;

		/* Checking to see if currentroutine is 0 is a way of seeing
		   if the debugger has started properly.  If, for example, a
		   property routine runs while LoadGame() is searching for
		   the display object, it may corrupt the uninitialized
		   debugger arrays.
		*/
		if ((old_currentroutine = currentroutine)==0) return;

		currentroutine = (unsigned int)(addr/address_scale);

		if (debugger_step_over)
		{
			step_nest++;
			debugger_interrupt = false;
		}
		else
		{
			/* Add a blank line if one hasn't been added
			   already:
			*/
			if ((window[CODE_WINDOW].count) && (codeline[window[CODE_WINDOW].count-1][0]&0x0FF)!='\0')
				AddStringtoCodeWindow("");

			/* If this is a property routine, the debug_line array
			   already holds the calling information
			*/
			if (!trace_complex_prop_routine)
				sprintf(debug_line, "Calling:  %s", RoutineName(currentroutine));
			else
				trace_comp_prop = true;
			trace_complex_prop_routine = false;

			call[window[VIEW_CALLS].count].addr = currentroutine;
			call[window[VIEW_CALLS].count++].param = (char)arguments_passed;
			window[VIEW_CALLS].changed = true;

			/* Revise call history */
			if (window[VIEW_CALLS].count==MAXCALLS)
			{
				for (i=0; i<MAXCALLS-1; i++)
				{
					call[i].addr = call[i+1].addr;
					call[i].param = call[i+1].param;
				}
				call[0].addr = 0;
			}

			/* If not object.property or an event */
			if (strchr(debug_line, '.')==nullptr && strstr(debug_line, "vent ")==nullptr)
			{
				strcat(debug_line, "(");
				for (i=0; i<arguments_passed; i++)
				{
					sprintf(debug_line+strlen(debug_line), "%d", var[MAXGLOBALS+i]);
					if (i<arguments_passed-1)
						strcat(debug_line, ", ");
				}
				strcat(debug_line, ")");
			}
			AddStringtoCodeWindow(debug_line);
		}
	}

#endif  /* defined (DEBUGGER) */

	defseg = gameseg;
	codeptr = addr;

/*
 *      Get the next token, so long as it isn't a CLOSE_BRACE_T ('}')
 *      marking the end of this block of code
 */

ContinueRunning:

	while (MEM(codeptr) != CLOSE_BRACE_T)   /* until "}" */
	{
		if (shouldQuit())
			return;
#if defined (DEBUGGER)
		/* Check if we're stepping over, and if we've returned to
		   the original level of nesting:
		*/
		if (debugger_step_over && step_nest==0)
		{
			debugger_step_over = false;
			debugger_interrupt = true;
		}
#endif
		if (var[endflag]) return;

		null_count = 0;

		/* Read the next token */
		while ((t = MEM(codeptr))==0)
		{
			codeptr++;

			/* Allow for padding zeroes.  If address_scale
			   zeroes are processed, we can't simply be
			   eating up the null space before an address
			   boundary.
			*/
			if (++null_count > address_scale)
				FatalError(UNKNOWN_OP_E);
		}
		
#if !defined (DEBUGGER)
#if defined (DEBUG_CODE)
		if (!inwindow)
		{
			sprintf(line, "[%6s:  %s]", PrintHex(codeptr), token[t]);
			AP(line);
		}
#endif
#endif
		if (game_version < 22) if (t==TEXT_T) t = TEXTDATA_T;

#if defined (DEBUGGER)
		if (++runaway_counter>=65535 && runtime_warnings)
		{
			sprintf(debug_line, "Possible runaway loop (65535 unchecked steps)");
			RuntimeWarning(debug_line);
			buffered_code_lines = FORCE_REDRAW;
			runaway_counter = 0;
		}

		if (t!=DEBUGDATA_T && t!=LABEL_T && !debugger_step_over)
			AddLinetoCodeWindow(codeptr);

		if ((i = IsBreakpoint(codeptr)))
		{
			if (t==DEBUGDATA_T || t==LABEL_T)
				broke_on_nonstatement = i;

			debugger_interrupt = true;

			/* '<' for "<Unknown>" */
			if (breakpoint[--i].in[0]=='<')
			{
				breakpoint[i].in = RoutineName(currentroutine);
				window[VIEW_BREAKPOINTS].changed = true;
			}
		}

		/* Don't add in-code data to the code window */
		if (t==DEBUGDATA_T || t==LABEL_T) goto ProcessToken;

		/* Evaluate (only) any watch expressions set to break
		   when true:
		*/
		for (i=0; i<(int)window[VIEW_WATCH].count; i++)
		{
			if (watch[i].isbreak)
			{
				SetupWatchEval(i);
				if (EvalWatch())
				{
					debugger_interrupt = true;
					break;
				}
			}
		}

		/* Always update the watch window */
		window[VIEW_WATCH].changed = true;

/*
 *      Immediately following is the main Debugger interrupt point.
 *      No matter where the engine is while running, if debugger_interrupt
 *      is ever set during the execution of a command, execution is
 *      paused and control passed to the Debugger.
 */

		if (debugger_interrupt)
		{
			if (broke_on_nonstatement)
			{
				broke_on_nonstatement--;
				breakpoint[broke_on_nonstatement].addr = codeptr;
				window[VIEW_BREAKPOINTS].changed = true;
				broke_on_nonstatement = false;
			}

			if (debugger_step_over)
			{
				AddStringtoCodeWindow("...");
				if (t!=DEBUGDATA_T && t!=LABEL_T)
					AddLinetoCodeWindow(codeptr);
			}
			Debugger();
		}


		/* Now, additional processing for flags that may have been
		   set while execution was suspended:
		*/

		/* Collapsing the RunRoutine() call stack */
		if (debugger_collapsing) return;


		/* May be necessary to reset this if, for some
		   reason, the line array was altered (see above)
		*/
		if (!trace_complex_prop_routine)
			sprintf(debug_line, "Calling:  %s", RoutineName(currentroutine));
		trace_complex_prop_routine = false;


		/* Add this statement to the code history */
		if (!debugger_step_back && !(debugger_step_over && step_nest>0))
		{
			if (++history_count>=MAX_CODE_HISTORY)
				 history_count = MAX_CODE_HISTORY;
			code_history[history_last] = codeptr;
			dbnest_history[history_last] = dbnest;
			if (++history_last >= MAX_CODE_HISTORY)
				history_last = 0;
		}

		/* If skipping next or stepping back */
		if (debugger_skip || debugger_step_back)
		{
			/* Debugger() has reset codeptr to next_codeptr */
			debugger_skip = false;
			continue;
		}

#endif  /* defined (DEBUGGER) */


/*
 *      This is the heart of RunRoutine():  the switch statement
 *      that executes the next engine operation based on what token
 *      has been read
 */
#if defined (DEBUGGER)
ProcessToken:
#endif
		switch (t)
		{
			/* First process any encoded, non-executable data: */

			/* If this is v2.5 or later, the compiler will have
			   noted the nesting level of this label to
			   reconcile stack_depth
			*/
			case LABEL_T:
				stack_depth = initial_stack_depth + MEM(++codeptr);
				codeptr++;
				break;

			case DEBUGDATA_T:
			{
			  switch (MEM(++codeptr))
			  {
				case VAR_T:     /* local variable name */
				{
					len = MEM(++codeptr);
#if defined (DEBUGGER)
					if (!debugger_has_stepped_back)
					{
						/* Read the local variable name */
						for (i=0; i<len; i++)
							line[i] = MEM(codeptr+i+1);
						line[len] = '\0';

						/* Check to make sure it doesn't already exist,
						   for instance, if we've looped back to it
						*/
						for (i=0; i<current_locals; i++)
							if (!strcmp(line, localname[i])) break;
						
						/* If it doesn't exist, add it */
						if (i==current_locals)
						{
							strcpy(localname[current_locals], line);
							if (++current_locals==MAXLOCALS)
								current_locals--;
							window[VIEW_LOCALS].count = current_locals;
						}
					}
#endif
					codeptr+=(len+1);
					break;
				}
			  }
			  break;
			}


			/* Then the executable statements: */

			case TEXTDATA_T:        /* printed text from file */
			{
				textaddr = Peek(codeptr+1)*65536L+(long)PeekWord(codeptr+2);
				strcpy(line, GetText(textaddr));
				codeptr += 4;
				if (Peek(codeptr)==SEMICOLON_T)
					{strcat(line, "\\;");
					codeptr++;}
				if (capital)
					{line[0] = (char)toupper((int)line[0]);
					capital = 0;}
				AP(line);
				break;
			}

			case TEXT_T:
			{
				if (MEM(++codeptr)==TO_T)
				{
					codeptr++;
#if defined (DEBUGGER)
					param_type = MEM(codeptr);
					param_start = codeptr;
#endif
					textto = GetValue();
					if (game_version>=23) codeptr++; /* eol */
#if defined (DEBUGGER)
					/* Check if textto is 0 but was not
					   really "text to 0", but rather
					   something that evaluated to 0
					*/
					if (textto==0 && runtime_warnings)
					{
					  if (param_type!=VALUE_T || param_start!=codeptr-4)
					    RuntimeWarning("Text array address evaluates to zero");
					}
#endif
				}
				else
				{
					SetupDisplay();
				}
				break;
			}

			case MINUS_T:           /* "--" */
			case PLUS_T:            /* "++" */
				GetValue();
				codeptr++; /* eol */
				break;

			case PRINT_T:
				RunPrint();
				break;

			case PRINTCHAR_T:
			{
Printcharloop:
				codeptr++;
				i = GetValue();
				if (capital) sprintf(line, "%c\\;", toupper(i));
				else sprintf(line, "%c\\;", i);
				capital = 0;
				AP(line);
				if (Peek(codeptr)==COMMA_T)
					goto Printcharloop;
				if (game_version>=23) codeptr++; /* eol */
				break;
			}

			case STRING_T:
				RunString();
				break;

			case WINDOW_T:
				RunWindow();
				break;

			case LOCATE_T:
			{
				char adhere_to_bottom = false;

				codeptr++;

				Flushpbuffer();

				xpos = GetValue();
				if (xpos > physical_windowwidth/FIXEDCHARWIDTH)
					xpos = physical_windowwidth/FIXEDCHARWIDTH;

				if (Peek(codeptr)==COMMA_T)
				{
					codeptr++;
					ypos = GetValue();
				}
				else
					ypos = currentline;

				full = ypos - 1;


				if (ypos >= physical_windowheight/lineheight)
					full = 0;
					
				if (ypos > physical_windowheight/lineheight)
				{
					ypos = physical_windowheight/lineheight;

					if (!inwindow && current_text_y && (currentfont & PROP_FONT))
						adhere_to_bottom = true;
				}

				hugo_settextpos(xpos, ypos);

				/* An adjustment for non-fixed-width font lineheight */
				if (adhere_to_bottom)
					current_text_y = physical_windowbottom - lineheight;

				currentpos = (xpos-1)*FIXEDCHARWIDTH;
				currentline = ypos;

				codeptr++;      /* skip EOL */
				break;
			}

			case SELECT_T:
				codeptr++;
				break;

			case CASE_T:
			case IF_T:
			case ELSEIF_T:
			case ELSE_T:
			case WHILE_T:
			case FOR_T:
				RunIf(0);
				break;

			case DO_T:
				RunDo();
				break;

			case RUN_T:
				codeptr++;
				tempret = ret;
				GetValue();     /* object.property to run */
				ret = tempret;
				if (game_version>=23) codeptr++; /* eol */
				break;

			case BREAK_T:
			{
				for (; stack_depth>0; stack_depth--)
				{
					if (code_block[stack_depth].brk)
					{
						codeptr = code_block[stack_depth].brk;
#if defined (DEBUGGER)
						dbnest = code_block[stack_depth].dbnest;
#endif
						--stack_depth;
						goto LeaveBreak;
					}
				}
				codeptr++;
LeaveBreak:
				break;
			}

			case RETURN_T:
			{
				codeptr++;
				i = inexpr;	/* don't reuse tempinexpr */
				inexpr = 1;
				
				/* Let 'return Routine()' or 'return obj.prop'
				   set tail_recursion
				*/
				tail_recursion = 0;
				tail_recursion_addr = 0;
				
				SetupExpr();
				inexpr = (char)i;
				
				/* If either a routine or property routine call has
				   determined it's valid, we can use tail-recursion
				   (with tail_recursion_addr having been set up)
				*/
				if (tail_recursion)
				{
					HandleTailRecursion(tail_recursion_addr);
					break;
				}
				else
				{
					/* Clear these to be safe */
					tail_recursion = 0;
					tail_recursion_addr = 0;

					ret = EvalExpr(0);
					retflag = true;
					goto LeaveRunRoutine;
				}
			}

			case JUMP_T:
			{
				codeptr = (long)PeekWord(codeptr + 1)*address_scale;
#if defined (DEBUGGER)
				if (MEM(codeptr)==LABEL_T)
					dbnest = 0;	/* prevent "false" nesting */
#endif
				break;
			}

			case PARENT_T:
			case SIBLING_T:
			case CHILD_T:
			case YOUNGEST_T:
			case ELDEST_T:
			case YOUNGER_T:
			case ELDER_T:
			{
				inobj = true;

				/* Note:  GetValue() would actually get
				   the property/attribute to be set
				*/
				RunSet(GetVal());

				inobj = false;
				break;
			}

			case VAR_T:
			case OBJECTNUM_T:
			case VALUE_T:
			case WORD_T:
			case ARRAYDATA_T:
			case ARRAY_T:
				RunSet(-1);
				break;

			case ROUTINE_T:
			case CALL_T:
			{
				switch (t)
				{
					case ROUTINE_T:
					{
						codeptr++;
						routineptr = PeekWord(codeptr);
						codeptr += 2;
						break;
					}
					case CALL_T:
					{
						codeptr++;
						routineptr = GetValue();
					}
				}

				tempret = ret;
				CallRoutine(routineptr);

				if (MEM(codeptr)==DECIMAL_T || MEM(codeptr)==IS_T)
					RunSet(ret);
				else if ((t==CALL_T) && game_version>=23)
					codeptr++; /* eol */

				ret = tempret;

				break;
			}

			case MOVE_T:
			case REMOVE_T:
				RunMove();
				break;

			case COLOR_T:
			case COLOUR_T:
			{
				codeptr++;

				/* Get foreground color */
				fcolor = (char)GetValue();
				/* If background color is given */
				if (Peek(codeptr)==COMMA_T)
				{
					codeptr++;
					bgcolor = (char)GetValue();

					/* If input color is given */
					if (Peek(codeptr)==COMMA_T)
					{
						codeptr++;
						icolor = (char)GetValue();
					}
					else
						icolor = fcolor;
				}
				else
					icolor = fcolor;

				/* Only set the actual pen color now if
				   there is no text buffered
				*/
				if (pbuffer[0]=='\0')
				{
					hugo_settextcolor(fcolor);
					hugo_setbackcolor(bgcolor);
				}
				
				if (inwindow)
					default_bgcolor = bgcolor;

				codeptr++;      /* skip EOL */
				break;
			}

			case PAUSE_T:
			{
				full = 0;
				override_full = true;
				codeptr++;
				Flushpbuffer();
				/* Flush the key buffer first */
				while (hugo_iskeywaiting()) hugo_getkey();
				wd[0] = (unsigned int)hugo_waitforkey();
#if defined (DEBUGGER)
				runaway_counter = 0;
#endif
				break;
			}

			case RUNEVENTS_T:
				codeptr++;
				RunEvents();
				break;

			case QUIT_T:
				var[endflag] = -1;
				break;

			case INPUT_T:
				RunInput();
				full = 1;
				override_full = true;
				codeptr++;
				break;

			case SYSTEM_T:
				RunSystem();
				if (game_version>=23) codeptr++; /* eol */
				break;

			case CLS_T:
			{
				hugo_settextcolor(fcolor);
				hugo_setbackcolor(bgcolor);
				hugo_clearwindow();
				hugo_settextpos(1, physical_windowheight/lineheight); /*+1);*/

				if (!inwindow)
				{
					full = 0;
				}
				default_bgcolor = bgcolor;

				codeptr++;
				pbuffer[0] = '\0';
				break;
			}

			case WRITEFILE_T:
			case READFILE_T:
				FileIO();
				break;

			case WRITEVAL_T:
			{
Writevalloop:
				codeptr++;
				i = GetValue();
				if (ioblock)
				{
					if ((ioblock==2)
						|| hugo_fputc(i%256, io)==EOF
						|| hugo_fputc(i/256, io)==EOF)
					{
						ioerror = true;
						retflag = true;
						break;
					}
				}

				if (Peek(codeptr)==COMMA_T)
					goto Writevalloop;

				if (game_version>=23) codeptr++; /* eol */
				break;
			}

			case PICTURE_T:
				DisplayPicture();
				break;

			case MUSIC_T:
				PlayMusic();
				break;

			case SOUND_T:
				PlaySample();
				break;

			case VIDEO_T:
				PlayVideo();
				break;

			case ADDCONTEXT_T:
				ContextCommand();
				break;

			/* Didn't match a command token, so throw up an
			   "Unknown operation" error.
			*/
			default:
				FatalError(UNKNOWN_OP_E);
		}

		defseg = gameseg;

		if (retflag) goto LeaveRunRoutine;
	}


	/* Process the closing '}': */

	codeptr++;

#if defined (DEBUGGER)
	if (--dbnest < 0) dbnest = 0;
#endif
	/* Continue executing this iteration of RunRoutine() if the
	   '}' marks the end of a conditional block, i.e., one that
	   didn't call RunRoutine() the way, e.g., 'window' does.
	   Otherwise, get out of RunRoutine().
	*/
	if (code_block[stack_depth--].type > RUNROUTINE_BLOCK)
	{
		/* Skip a following 'elseif', 'else', or 'case'  */
		t = MEM(codeptr);
		while (t==ELSEIF_T || t==ELSE_T || t==CASE_T)
		{
			RunIf(1);
			t = MEM(codeptr);
		}

		if (t==WHILE_T && code_block[stack_depth+1].type==DOWHILE_BLOCK)
		{
			codeptr+=3;
			tempinexpr = inexpr;
			inexpr = 1;
			SetupExpr();
			inexpr = tempinexpr;

			if (EvalExpr(0))
				codeptr = code_block[++stack_depth].returnaddr;
			else
				codeptr = code_block[stack_depth+1].brk;
		}

		/* Since this isn't a RUNROUTINE_BLOCK, keep running this
		   iteration of RunRoutine()
		*/
		goto ContinueRunning;

	}

	if (stack_depth<0) stack_depth = 0;

	if (var[endflag]) return;


LeaveRunRoutine:

#if defined (DEBUGGER)

/*
 *      Finally, do any debugger-required cleaning-up
 */

	/* As noted above, wascalled is true if this was a routine call
	   as opposed to, e.g., a conditional block.  In the former case,
	   it is necessary to print the "Returning from..." message.
	*/
	if (wascalled)
	{
		if (debugger_step_over)
		{
			if (--step_nest<=0)
			{
				debugger_step_over = false;
				debugger_interrupt = true;

				if (debugger_finish || step_nest < 0)
				{
					debugger_finish = false;
					goto ReturnfromRoutine;
				}
			}
		}

		else if (!debugger_step_over)
		{
ReturnfromRoutine:
			sprintf(debug_line, "(Returning %d", ret);

			/* Since a complex property routine will give "<Routine>" as the
			   routine name, skip those
			*/
			called_from = RoutineName(currentroutine);
			if (!trace_comp_prop && called_from[0]!='<')
				sprintf(debug_line+strlen(debug_line), " from %s", called_from);

			if (old_currentroutine!=mainaddr && old_currentroutine!=initaddr
				&& currentroutine!=mainaddr && currentroutine!=initaddr)
			{
				sprintf(debug_line+strlen(debug_line), " to %s", RoutineName(old_currentroutine));
			}
			strcat(debug_line, ")");
			AddStringtoCodeWindow(debug_line);
			AddStringtoCodeWindow("");

			if ((signed)--window[VIEW_CALLS].count < 0)
				window[VIEW_CALLS].count = 0;
			window[VIEW_CALLS].changed = true;
		}

		currentroutine = old_currentroutine;
	}

/*#elif defined (DEBUG_CODE)
	if (wascalled)
		{sprintf(line, "[RETURNING %d]", ret);
		AP(line);}
*/
#endif

	return;
}

int Hugo::RunSave() {
#ifdef PALMOS
	/* Prevent simultaneous access to the same db record */
	int dummy = MEM(objtable*16L);
#endif

#if !defined (GLK)
	save = nullptr;

	/* stdio implementation */
	hugo_getfilename("to save", savefile);

#if defined (DEBUGGER)
	if (debugger_collapsing) return 1;
#endif
	if (!strcmp(line, gamefile)) return 0;
	if (!strcmp(line, "")) return 0;
	if (!hugo_overwrite(line)) return 0;
	if (!(save = HUGO_FOPEN(line, "w+b"))) return 0;

	if (!SaveGameData()) goto SaveError;

	if (hugo_fclose(save)) FatalError(WRITE_E);
	save = nullptr;

	strcpy(savefile, line);
#else
	/* Glk implementation */
	if (saveGame().getCode() != Common::kNoError)
		goto SaveError;
#endif	/* GLK */

	return 1;

SaveError:
#if !defined (GLK)
	if ((save) && hugo_fclose(save)) FatalError(WRITE_E);
	save = nullptr;
#endif
	return 0;
}

int Hugo::RunScriptSet() {
	remaining = 0;

	switch (Peek(codeptr))
	{
		case SCRIPTON_T:
		{
			if (!script)
			{
#if !defined (GLK)
				/* stdio implementation */
				hugo_getfilename("to begin transcription (or printer name)", scriptfile);
#if defined (DEBUGGER)
				if (debugger_collapsing) return 1;
#endif
				if (!strcmp(line, "")) return 0;
				if (!hugo_overwrite(line)) return 0;
				if (!(script = HUGO_FOPEN(line, "wt")))
					return (0);
				strcpy(scriptfile, line);

#else
				/* Glk implementation */
				frefid_t fref;

				fref = glk_fileref_create_by_prompt(fileusage_Transcript | fileusage_TextMode,
					filemode_Write, 0);
				script = glk_stream_open_file(fref, filemode_Write, 0);
				glk_fileref_destroy(fref);
				if (!script) return (0);
#endif	/* GLK */
				return 1;
			}
			break;
		}

		case SCRIPTOFF_T:
		{
			if (script)
			{
				if (hugo_fclose(script)) return (0);
				script = nullptr;
				return 1;
			}
			break;
		}
	}
	return 0;
}

int Hugo::RunString() {
	int i, pos;
	unsigned int aaddr;                     /* array address   */
	unsigned int dword;                     /* dictionary word */
	unsigned int maxlen = 32767;

	codeptr += 2;                           /* skip "(" */

	aaddr = GetValue();
	if (game_version>=22)
	{
		/* Convert to 16-bit word value */
		aaddr*=2;

		if (game_version>=23)
		{
			defseg = arraytable;
			maxlen = PeekWord(aaddr);
			defseg = gameseg;

			/* Space for array length */
			aaddr+=2;
		}
	}

	if (Peek(codeptr)==COMMA_T) codeptr++;

	dword = GetValue();

	if (Peek(codeptr)==COMMA_T) codeptr++;

	if (Peek(codeptr)!=CLOSE_BRACKET_T)
		maxlen = GetValue();
	if (Peek(codeptr)==CLOSE_BRACKET_T) codeptr++;

	strcpy(line, GetWord(dword));

	defseg = arraytable;
	pos = 0;
	for (i=0; i<(int)strlen(line) && i<(int)maxlen; i++, pos++)
	{
		char a;

		SaveUndo(ARRAYDATA_T, aaddr, i, PeekWord(aaddr+i*2), 0);

		a = line[i];
		if (a=='\\')
			++i, a = SpecialChar(line, &i);
		PokeWord(aaddr+pos*2, a);
	}
	PokeWord(aaddr+pos*2, 0);

	defseg = gameseg;

	return (i);
}

int Hugo::RunSystem() {
	codeptr++;

	/* Since the obsolete form of the system command is unimplemented, 
	   simply get the parameter (in order to skip it), and exit the
	   function.
	*/
	if (game_version < 25)
	{
		GetValue();
		return 0;
	}
	
	/* Otherwise, process the following system calls: */
	
	codeptr++;			/* skip opening bracket */
	
	var[system_status] = 0;

	Flushpbuffer();
	
	switch (GetValue())
	{
		case 11:		/* READ_KEY */
			if (!hugo_iskeywaiting())
				return 0;
			else
			{
				full = 0;
				return hugo_getkey();
			}

		case 21:		/* NORMALIZE_RANDOM */
#if !defined (RANDOM)
			_random.setSeed(1);
#else
			SRANDOM(1);
#endif
			break;
		case 22:		/* INIT_RANDOM */
		{
#if !defined (RANDOM)
			_random.setSeed(g_system->getMillis());
#else
			time_t seed;
			SRANDOM((unsigned int)time((time_t *)&seed));
#endif
			break;
		}			
		case 31:		/* PAUSE_SECOND */
			if (!hugo_timewait(1))
				var[system_status] = STAT_UNAVAILABLE;
			break;
			
		case 32:		/* PAUSE_100TH_SECOND */
			if (!hugo_timewait(100))
				var[system_status] = STAT_UNAVAILABLE;
			break;

		case 41:		/* GAME_RESET */
		{
			if (game_reset)
			{
				game_reset = 0;
				return true;
			}
			return false;
		}

		case 51:		/* SYSTEM_TIME */
		{
#ifndef NO_STRFTIME
			TimeDate td;
			g_system->getTimeAndDate(td);
			sprintf(parseerr, "%d-%.2d-%.2d %d:%.2d:%.2d", td.tm_year, td.tm_mon, td.tm_mday,
				td.tm_hour, td.tm_min, td.tm_sec);
#else
			hugo_gettimeformatted(parseerr);
#endif
			return true;
		}

		case 61:		/* MINIMAL_INTERFACE */
#ifdef MINIMAL_INTERFACE
			return true;
#else
			return false;
#endif

		default:
			var[system_status] = STAT_UNAVAILABLE;
	}
	
	return 0;
}

void Hugo::SaveWindowData(SAVED_WINDOW_DATA *spw) {
	spw->left = physical_windowleft;
	spw->top = physical_windowtop;
	spw->right = physical_windowright;
	spw->bottom = physical_windowbottom;
	spw->width = physical_windowwidth;
	spw->height = physical_windowheight;
	spw->currentfont = currentfont;
	spw->charwidth = charwidth;
	spw->lineheight = lineheight;
	spw->currentpos = currentpos;
	spw->currentline = currentline;
}

void Hugo::RestoreWindowData(SAVED_WINDOW_DATA *spw) {
	physical_windowleft = spw->left;
	physical_windowtop = spw->top;
	physical_windowright = spw->right;
	physical_windowbottom = spw->bottom;
	physical_windowwidth = spw->width;
	physical_windowheight = spw->height;

	charwidth = spw->charwidth;
	lineheight = spw->lineheight;
	currentpos = spw->currentpos;
	currentline = spw->currentline;

/*	if (currentfont!=spw->currentfont) hugo_font((currentfont = spw->currentfont)); */
}

void Hugo::RunWindow() {
	int top, bottom, left, right;
	struct SAVED_WINDOW_DATA restorewindow;
	int temp_current_text_y;
	char restore_default_bgcolor;
	int tempfull;
	int temp_stack_depth = stack_depth;
	HUGO_FILE tempscript;
#ifdef MINIMAL_WINDOWING
	int last_lowest_windowbottom = lowest_windowbottom;
#endif

#if defined (DEBUGGER)
	unsigned char param_type;
	int tempdbnest;
	long param_start;
#endif

	Flushpbuffer();
	tempfull = full;
	full = 0;
	override_full = false;

	temp_current_text_y = current_text_y;

	tempscript = script;
	script = nullptr;
	restore_default_bgcolor = default_bgcolor;

	/* v2.4 is the first version to support proper windowing */
	if (game_version>=24)
	{
		/* Set up default top, left, etc. as character coordinates,
		   and save the current physical window data
		*/
		left = physical_windowleft/FIXEDCHARWIDTH + 1;
		top = physical_windowtop/FIXEDLINEHEIGHT + 1;
		right = physical_windowright/FIXEDCHARWIDTH + 1;
		bottom = physical_windowbottom/FIXEDLINEHEIGHT + 1;

		SaveWindowData(&restorewindow);

		/* if "window x1, y1, x2, y2" or "window n"... */
		if (MEM(++codeptr)!=EOL_T)
		{
#if defined (DEBUGGER)
			param_type = MEM(codeptr);
			param_start = codeptr;
#endif
			left = GetValue();
			if (MEM(codeptr++)==COMMA_T)
			{
				top = GetValue();
				if (MEM(codeptr++)==COMMA_T)
				{
					right = GetValue();
					if (MEM(codeptr++)==COMMA_T)
					{
						bottom = GetValue();
						codeptr++;
					}
				}
			}

			/* if only one parameter, i.e., "window n" */
			else
			{
				if (left!=0)
				{
					bottom = left;
					top = 1;
					left = 1;
					right = SCREENWIDTH/FIXEDCHARWIDTH;
				}

				/* "window 0" restores full screen without
				   running a code block
				*/
				else
				{
#if defined (DEBUGGER)
					/* Here, check to see if left was 0 but the
					   statement wasn't really "window 0", but
					   rather something that evaluated to zero
					*/
					if (runtime_warnings)
					{
					  if (param_type!=VALUE_T || param_start!=codeptr-4)
					    RuntimeWarning("Window size evaluates to zero");
					}
#endif
					left = 1, top = 1;
					right = SCREENWIDTH/FIXEDCHARWIDTH;
					bottom = SCREENHEIGHT/FIXEDLINEHEIGHT;
					physical_lowest_windowbottom = lowest_windowbottom = 0;
					hugo_settextwindow(left, top, right, bottom);
					goto LeaveWindow;
				}
			}
		}

		/* ...or just "window", so use last window defaults */
		else
		{
			codeptr++;      /* skip EOL */

			left = last_window_left;
			top = last_window_top;
			right = last_window_right;
			bottom = last_window_bottom;
		}

		/* Remember, these are character/text coordinates */
		if (top < 1) top = 1;
		if (left < 1) left = 1;
		if (bottom < 1) bottom = 1;
		if (right < 1) right = 1;
		if (top > SCREENHEIGHT/FIXEDLINEHEIGHT)
			top = SCREENHEIGHT/FIXEDLINEHEIGHT;
		if (left > SCREENWIDTH/FIXEDCHARWIDTH)
			left = SCREENWIDTH/FIXEDCHARWIDTH;
		if (bottom > SCREENHEIGHT/FIXEDLINEHEIGHT)
			bottom = SCREENHEIGHT/FIXEDLINEHEIGHT;
		if (right > SCREENWIDTH/FIXEDCHARWIDTH)
			right = SCREENWIDTH/FIXEDCHARWIDTH;

		/* Set the new text window */
		inwindow = true;
		hugo_settextwindow(left, top, right, bottom);
		hugo_settextpos(1, 1);

#if defined (DEBUGGER)
		tempdbnest = dbnest++;
#endif
		SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);

		RunRoutine(codeptr);

#if defined (DEBUGGER)
		dbnest = tempdbnest;
#endif
		stack_depth = temp_stack_depth;

		Flushpbuffer();

		/* Restore the old window parameters */
		last_window_top = top;
		last_window_bottom = bottom;
		last_window_left = left;
		last_window_right = right;

		/* Figure out what the lowest window bottom is that we need
		   to protect from scrolling
		*/
		if (bottom > lowest_windowbottom)
			lowest_windowbottom = bottom;

#ifdef MINIMAL_WINDOWING
		if (minimal_windowing && illegal_window)
			lowest_windowbottom = last_lowest_windowbottom;
#endif
		/* (error situation--shouldn't happen) */
		if (lowest_windowbottom>=SCREENHEIGHT/FIXEDLINEHEIGHT)
			lowest_windowbottom = 0;

		/* Restore the old text window */
		RestoreWindowData(&restorewindow);

		inwindow = false;
		hugo_settextwindow(physical_windowleft/FIXEDCHARWIDTH + 1,
			lowest_windowbottom + 1,
			physical_windowright/FIXEDCHARWIDTH + 1,
			physical_windowbottom/FIXEDLINEHEIGHT + 1);

		physical_lowest_windowbottom = lowest_windowbottom*FIXEDLINEHEIGHT;
	}

	/* v2.3 and earlier supported a very simple version of
	   windowing:  mainly just moving the top/scroll-off line
	   of the printable area to the bottom of the text printed
	   in the "window" block
	*/
	else
	{
		inwindow = true;
		hugo_settextwindow(1, 1,
	       		SCREENWIDTH/FIXEDCHARWIDTH,
		        SCREENHEIGHT/FIXEDLINEHEIGHT);
		hugo_settextpos(1, 1);

		SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);
		RunRoutine(++codeptr);
		Flushpbuffer();

		inwindow = false;

		stack_depth = temp_stack_depth;

		hugo_settextwindow(1, full+1,
		        SCREENWIDTH/FIXEDCHARWIDTH,
			SCREENHEIGHT/FIXEDLINEHEIGHT);

		physical_lowest_windowbottom = full*lineheight;
	}

LeaveWindow:

	current_text_y = temp_current_text_y;

#ifndef PALMOS
	if (!current_text_y)
		hugo_settextpos(1, physical_windowheight/lineheight);
#endif
	current_text_x = 0;
	currentpos = 0;

	default_bgcolor = restore_default_bgcolor;
	script = tempscript;

	if (!override_full)
		full = tempfull;
	override_full = false;

	just_left_window = true;
}

} // End of namespace Hugo
} // End of namespace Glk
