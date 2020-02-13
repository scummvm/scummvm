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

static char EMPTY[1] = { 0 };

void Hugo::AP(const char *a) {
	char sticky = false, skipspchar = false, startofline = 0;
	int i, alen, plen, cwidth;
	char c = 0;			/* current character */
#ifdef USE_SMARTFORMATTING
	char lastc = 0;			/* for smart formatting */
#endif

	static int lastfcolor = 16, lastbgcolor = 17;
	static int lastfont = NORMAL_FONT;
	static int thisline = 0;	/* width in pixels or characters */
	static int linebreaklen = 0, linebreak = 0;
	int tempfont;
	char printed_something = false;
#ifdef USE_TEXTBUFFER
	int bufferfont = currentfont;
#endif


	/* Shameless little trick to override control characters in engine-
	   printed text, such as MS-DOS filenames that contain '\'s:
	*/
	if (a[0]==NO_CONTROLCHAR)
	{
		skipspchar = true;
		a++;
	}

	/* Semi-colon overrides LF */
	if ((strlen(a)>=2) && a[strlen(a)-1]==';' && a[strlen(a)-2]=='\\')
	{
		sticky = true;
	}

	if (hugo_strlen(pbuffer))
		printed_something = true;

	plen = strlen(pbuffer);
	if (plen==0)
	{
		thisline = 0;
		linebreak = 0;
		linebreaklen = 0;
		lastfont = currentfont;
		startofline = true;
#ifdef USE_TEXTBUFFER
		bufferbreak = 0;
		bufferbreaklen = 0;
#endif
#ifdef USE_SMARTFORMATTING
		leftquote = true;
#endif
	}

	/* Check for color changes */
	if ((a[0]) && (lastfcolor!=fcolor || lastbgcolor!=bgcolor || startofline))
	{
		if (plen >= MAXBUFFER*2-3) FatalError(OVERFLOW_E);
		pbuffer[plen++] = COLOR_CHANGE;
		pbuffer[plen++] = (char)(fcolor+1);
		pbuffer[plen++] = (char)(bgcolor+1);
		pbuffer[plen] = '\0';
		lastfcolor = fcolor;
		lastbgcolor = bgcolor;
	}

	/* Check for font changes--since fonts can only get changed
	   by printing, we don't check lastfont */
	if ((a[0]) && startofline)
	{
		if (plen >= MAXBUFFER*2-2) FatalError(OVERFLOW_E);
		pbuffer[plen++] = FONT_CHANGE;
		pbuffer[plen++] = (char)(currentfont+1);
		pbuffer[plen] = '\0';
		lastfont = currentfont;
	}

	/* Begin by looping through the entire provided string: */

	alen = (int)strlen(a);
	if (sticky)
		alen -= 2;

	/* Not printing any actual text, so we won't need to go through
	   queued font changes
	*/
	if (alen==0)
		lastfont = currentfont;

	for (i=0; i<alen; i++)
	{
		c = a[i];
		
		/* Left-justification */
		if (thisline==0 && c==' ' && !textto && currentpos==0)
			continue;

		/* First check control characters */
		if (c=='\\' && !skipspchar)
		{
			c = a[++i];

			switch (c)
			{
				case 'n':
				{
					c = '\n';
					break;
				}
				case 'B':
				{
					currentfont |= BOLD_FONT;
					goto AddFontCode;
				}
				case 'b':
				{
					currentfont &= ~BOLD_FONT;
					goto AddFontCode;
				}
				case 'I':
				{
					currentfont |= ITALIC_FONT;
					goto AddFontCode;
				}
				case 'i':
				{
					currentfont &= ~ITALIC_FONT;
					goto AddFontCode;
				}
				case 'P':
				{
					currentfont |= PROP_FONT;
					goto AddFontCode;
				}
				case 'p':
				{
					currentfont &= ~PROP_FONT;
					goto AddFontCode;
				}
				case 'U':
				{
					currentfont |= UNDERLINE_FONT;
					goto AddFontCode;
				}
				case 'u':
				{
					currentfont &= ~UNDERLINE_FONT;
AddFontCode:
					if (!textto)
					{
						int m, n;
						int newfull;
						double ratio;

						if (!printed_something)
						{
							for (m=0; m<plen;)
							{
								if (pbuffer[m]==FONT_CHANGE)
								{
									for (n=m; n<plen-2; n++)
									{
										pbuffer[n] = pbuffer[n+2];
									}
									plen-=2;
									pbuffer[plen] = '\0';
									lastfont = currentfont;
								}
								else if (pbuffer[m]==COLOR_CHANGE)
									m+=3;
								else
									break;
							}
						}
#ifdef USE_TEXTBUFFER
						if (hugo_strlen(pbuffer+bufferbreak)==0)
							bufferfont = currentfont;
#endif
						if (plen >= MAXBUFFER*2-2) FatalError(OVERFLOW_E);
						pbuffer[plen+2] = '\0';
						pbuffer[plen+1] = (char)(currentfont+1);
						pbuffer[plen] = FONT_CHANGE;
						plen+=2;

						/* Convert full if font height changes, since
						   the amount of used screen real estate for
						   this particular font (in terms of lines)
						   will have changed:
						*/
						ratio = (double)lineheight;
						hugo_font(currentfont);
						ratio /= (double)lineheight;
						newfull = (int)(((double)full)*ratio+0.5);
						if (newfull) full = newfull;
					}
					continue;
				}
				case '_':       /* forced space */
				{
					if (textto) c = ' ';
					else c = FORCED_SPACE;
					break;
				}
				default:
					c = SpecialChar(a, &i);
			}
		}
		else if (game_version<=22)
		{
			if (c=='~')
				c = '\"';
			else if (c=='^')
				c = '\n';
		}

	/* Add the new character */

		/* Text may be sent to an address in the array table instead
		   of being output to the screen
		*/
		if (textto)
		{
			/* space for array length */
			int n = (game_version>23)?2:0;

			if (c=='\n')
			{
				SETMEM(arraytable*16L + textto*2 + n, 0);
				textto++;
			}			
			else if ((unsigned char)c >= ' ')
			{
				SETMEM(arraytable*16L + textto*2 + n, c);
				textto++;
			}
			/* Add a terminating zero in case we don't
			   print any more to the array */
			SETMEM(arraytable*16L + textto*2 + n, 0);

			if (i >= alen) return;

			continue;       /* back to for (i=0; i<slen; i++) */
		}

		printed_something = true;

		/* Handle in-text newlines */
		if (c=='\n')
		{
			hugo_font(currentfont = lastfont);
#ifdef USE_TEXTBUFFER
			TB_AddWord(pbuffer+bufferbreak,
				current_text_x+bufferbreaklen,
				current_text_y,
				current_text_x+thisline-1,
				current_text_y+lineheight-1);
#endif
			Printout(pbuffer, 0);
			lastfont = currentfont;
#ifdef USE_TEXTBUFFER
			bufferfont = currentfont;
			bufferbreak = 0;
			bufferbreaklen = 0;
#endif
			strcpy(pbuffer, "");
			plen = 0;
			linebreak = 0;
			linebreaklen = 0;
			thisline = 0;
#ifdef USE_SMARTFORMATTING
			leftquote = true;
			lastc = '\n';
#endif
			pbuffer[plen++] = COLOR_CHANGE;
			pbuffer[plen++] = (char)(fcolor+1);
			pbuffer[plen++] = (char)(bgcolor+1);
			pbuffer[plen] = '\0';


			continue;
		}

#ifdef USE_SMARTFORMATTING
		/* Smart formatting only for non-fixed fonts */
		if ((currentfont & PROP_FONT) && smartformatting)
		{
			if ((!strncmp(a+i, "--", 2)) && lastc!='-' && strncmp(a+i, "---", 3))
			{
				lastc = '-';
				c = (char)151;
				i++;
				leftquote = false;
			}
			else if (c=='\"')
			{
				if (leftquote)
					c = (char)147;
				else
					c = (char)148;
				leftquote = false;
				lastc = c;
			}
			else if (c=='\'')
			{
				if (leftquote)
					c = (char)145;
				else
					c = (char)146;
				leftquote = false;
				lastc = c;
			}
			else
			{
				if (c==' ')
					leftquote = true;
				else
					leftquote = false;
				lastc = c;
			}
		}
#endif

		/* Add the new character to the printing buffer */
		if (plen >= MAXBUFFER*2-1) FatalError(OVERFLOW_E);
		pbuffer[plen+1] = '\0';
		pbuffer[plen] = c;
		plen++;

		cwidth = hugo_charwidth(c);

	/* Check to see if we've overrun the current line */

		if (thisline+cwidth+currentpos > physical_windowwidth)
		{
			char t;

			if (!linebreak)
			{
				linebreak = plen-1;
				linebreaklen = thisline;
			}

			t = pbuffer[linebreak];
			pbuffer[linebreak] = '\0';

			tempfont = currentfont;
			hugo_font(currentfont = lastfont);
			Printout(pbuffer, linebreak);
			lastfont = currentfont;
			hugo_font(currentfont = tempfont);

			pbuffer[linebreak] = t;
			plen = strlen(pbuffer+linebreak);
			memmove(pbuffer, pbuffer+linebreak, plen + 1);
			thisline = thisline - linebreaklen;
			linebreak = 0;
			linebreaklen = 0;
			startofline = 0;
#ifdef USE_TEXTBUFFER
			bufferbreak = 0;
			bufferbreaklen = 0;
#endif
		}

		thisline += cwidth;

#ifdef USE_TEXTBUFFER
		if ((c==' ' || c==FORCED_SPACE) ||
			(c=='/' && a[i+1]!='/') || (c=='-' && a[i+1]!='-'))
		{
			TB_AddWord(pbuffer+bufferbreak,
				current_text_x+bufferbreaklen,
				current_text_y,
				current_text_x+thisline-1,
				current_text_y+lineheight-1);

			bufferbreak = plen;
			bufferbreaklen = thisline;
			bufferfont = currentfont;
		}
#endif
		if ((c==' ') || (c=='/' && a[i+1]!='/') || (c=='-' && a[i+1]!='-'))
		{
			linebreak = plen, linebreaklen = thisline;
		}
	}

#ifdef USE_TEXTBUFFER
	if (!sticky || alen > 1)
	{
		tempfont = currentfont;
		currentfont = bufferfont;

		TB_AddWord(pbuffer+bufferbreak,
			current_text_x+bufferbreaklen,
			current_text_y,
			current_text_x+thisline-1,
			current_text_y+lineheight-1);

		bufferbreak = plen;
		bufferbreaklen = thisline;
		currentfont = tempfont;
	}
#endif
	if (!sticky)
	{
		hugo_font(currentfont = lastfont);
		Printout(pbuffer, 0);
		lastfont = currentfont;
		strcpy(pbuffer, "");
		linebreak = 0;
		linebreaklen = 0;
		thisline = 0;
		plen = 0;
#ifdef USE_TEXTBUFFER
		bufferbreak = 0;
		bufferbreaklen = 0;
#endif
#ifdef USE_SMARTFORMATTING
		leftquote = true;
#endif
	}
}

int Hugo::CallRoutine(unsigned int addr) {
	int arg, i;
	int val;
	int templocals[MAXLOCALS], temppass[MAXLOCALS];
	int temp_stack_depth;
	long tempptr;
	int potential_tail_recursion = tail_recursion;
#if defined (DEBUGGER)
	int tempdbnest;
#endif
	arg = 0;
	tail_recursion = 0;
	Common::fill(&temppass[0], &temppass[MAXLOCALS], 0);

	/* Pass local variables to routine, if specified */
	if (MEM(codeptr)==OPEN_BRACKET_T)
	{
		codeptr++;
		while (MEM(codeptr) != CLOSE_BRACKET_T)
		{
			if (arg)
			{
				for (i=0; i<arg; i++)
					temppass[i] = passlocal[i];
			}

			passlocal[arg++] = GetValue();

			if (arg > 1)
			{
				for (i=0; i<arg-1; i++)
					passlocal[i] = temppass[i];
			}

			if (MEM(codeptr)==COMMA_T) codeptr++;
		}
		codeptr++;
	}

	/* TAIL_RECURSION_ROUTINE if we came from a routine call immediately
	   following a 'return' statement...
	*/
	tail_recursion = potential_tail_recursion;
	if (tail_recursion==TAIL_RECURSION_ROUTINE && MEM(codeptr)==EOL_T)
	{
		tail_recursion_addr = (long)addr*address_scale;
		PassLocals(arg);
		return 0;
	}
	/* ...but if we're not immediately followed by and end-of-line marker,
	   cancel the pending tail-recursion
	*/
	else
	{
		tail_recursion = 0;
	}

	for (i=0; i<MAXLOCALS; i++)
		templocals[i] = var[MAXGLOBALS+i];
	PassLocals(arg);

	temp_stack_depth = stack_depth;

	SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);

	tempptr = codeptr;      /* store calling address */
	ret = 0;

#if defined (DEBUGGER)
	tempdbnest = dbnest;
	DebugRunRoutine((long)addr*address_scale);
	dbnest = tempdbnest;
#else
	RunRoutine((long)addr*address_scale);
#endif
	retflag = 0;
	val = ret;
	codeptr = tempptr;

	stack_depth = temp_stack_depth;

	for (i=0; i<MAXLOCALS; i++)
		var[MAXGLOBALS+i] = templocals[i];

	return val;
}

void Hugo::ContextCommand() {
	unsigned int n;

ContextCommandLoop:

	codeptr++;
	
	n = GetValue();
#if !defined (COMPILE_V25)
	if (n==0)
	{
		context_commands = 0;
	}
	else if (context_commands < MAX_CONTEXT_COMMANDS)
	{
		const char *cc;

		strncpy(context_command[context_commands], cc = GetWord(n), 64);
		context_command[context_commands][63] = '\0';
		if (strlen(cc)>=64)
			sprintf(context_command[context_commands]+60, "...");
		context_commands++;
	}
#endif
	if (Peek(codeptr)==COMMA_T) goto ContextCommandLoop;
	codeptr++;
}

unsigned int Hugo::Dict() {
	int i, len = 256;
	unsigned int arr;
	unsigned int pos = 2, loc;

	codeptr += 2;                           /* "(" */

	if (MEM(codeptr)==PARSE_T || MEM(codeptr)==WORD_T)
		strcpy(line, GetWord(GetValue()));
	else
	{
		/* Get the array address to read the to-be-
		   created dictionary entry from:
		*/
		arr = GetValue();
		if (game_version>=22)
		{
			/* Convert the address to a word
			   value:
			*/
			arr*=2;

			if (game_version>=23)
				/* space for array length */
				arr+=2;
		}

		defseg = arraytable;
		for (i=0; i<len && PeekWord(arr+i*2)!=0; i++)
			line[i] = (char)PeekWord(arr+i*2);
		defseg = gameseg;
		line[i] = '\0';
	}

	if (Peek(codeptr)==COMMA_T) codeptr++;
	len = GetValue();

	if ((loc = FindWord(line))!=UNKNOWN_WORD) return loc;

	defseg = dicttable;

	for (i=1; i<=dictcount; i++)
		pos += Peek(pos) + 1;

	loc = pos - 2;
	
	if ((long)(pos+strlen(line)) > (long)(codeend-dicttable*16L))
	{
#ifdef DEBUGGER
		sprintf(debug_line, "$MAXDICTEXTEND dictionary space exceeded");
		RuntimeWarning(debug_line);
#endif
		defseg = gameseg;
		return 0;
	}

	Poke(pos++, (unsigned char)strlen(line));
	for (i=0; i<(int)strlen(line) && i<len; i++)
		Poke(pos++, (unsigned char)(line[i]+CHAR_TRANSLATION));
	PokeWord(0, ++dictcount);

	defseg = gameseg;

	SaveUndo(DICT_T, strlen(line), 0, 0, 0);

	return loc;
}

void Hugo::FatalError(int n) {
#if defined (DEBUGGER)
	hugo_stopmusic();
	hugo_stopsample();

	/* If the Debugger has already issued an error, it will
	   try to recover instead of issuing a stream of
	   identical errors.
	*/
	if (runtime_error) return;
#else
	hugo_cleanup_screen();
#endif

	switch (n)
	{
		case MEMORY_E:
			{sprintf(line, "Out of memory\n");
			break;}

		case OPEN_E:
			{sprintf(line, "Cannot open file\n");
			break;}

		case READ_E:
			{sprintf(line, "Cannot read from file\n");
			break;}

		case WRITE_E:
			{sprintf(line, "Cannot write to save file\n");
			break;}

		case EXPECT_VAL_E:
			{sprintf(line, "Expecting value at $%s\n", PrintHex(codeptr));
			break;}

		case UNKNOWN_OP_E:
			{sprintf(line, "Unknown operation at $%s\n", PrintHex(codeptr));
			break;}

		case ILLEGAL_OP_E:
			{sprintf(line, "Illegal operation at $%s\n", PrintHex(codeptr));
			break;}

		case OVERFLOW_E:
			{sprintf(line, "Overflow at $%s\n", PrintHex(codeptr));
			break;}

		case DIVIDE_E:
			{sprintf(line, "Divide by zero at $%s\n", PrintHex(codeptr));
			break;}
	}

#if defined (DEBUGGER)

	if (routines > 0)
        {
		SwitchtoDebugger();

		if (n==MEMORY_E) DebuggerFatal(D_MEMORY_ERROR);

		RuntimeWarning(line);
		debugger_interrupt = true;
		debugger_skip = true;
		runtime_error = true;

		if (n!=EXPECT_VAL_E)
			RecoverLastGood();

		codeptr = this_codeptr;

		return;
	}

	hugo_cleanup_screen();
#endif

/* crash dump */
/*
if (n==UNKNOWN_OP_E || n==ILLEGAL_OP_E || n==EXPECT_VAL_E || n==OVERFLOW_E)
{
	for (n=-8; n<0; n++)
		fprintf(stderr, " %c%2X", (n==-8)?'$':' ', MEM(codeptr+n));
	fprintf(stderr, "\n");
	for (n=-8; n<0; n++)
		fprintf(stderr, " %3d", MEM(codeptr+n));
	fprintf(stderr, "\n\n> %2X", MEM(codeptr));
	for (n=1; n<8; n++)
		fprintf(stderr, "  %2X", MEM(codeptr+n));
	fprintf(stderr, "\n");
	fprintf(stderr, ">%3d", MEM(codeptr));
	for (n=1; n<8; n++)
		fprintf(stderr, " %3d", MEM(codeptr+n));
	fprintf(stderr, "\n");
}
*/
	Common::String msg = Common::String::format("\nFatal Error:  %s", line);
	PRINTFATALERROR(msg.c_str());

	hugo_closefiles();
	hugo_blockfree(mem);
	mem = nullptr;
	error("Error code: %d", (int)n);
}

void Hugo::FileIO() {
#ifndef GLK
	char drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILENAME], ext[MAXEXT];
#endif
	char fileiopath[MAXPATH];
	char iotype;
	unsigned int fnameval;
	long skipaddr;
	int i, temp_stack_depth = stack_depth;
#if defined (DEBUGGER)
	int tempdbnest;
#endif

	iotype = MEM(codeptr++);
	skipaddr = (long)PeekWord(codeptr)*address_scale;
	codeptr+=2;
	fnameval = GetValue();
	if (game_version>=23) codeptr++; /* eol */

	ioerror = 0;
	
	/* Make sure the filename is legal, 8 alphanumeric characters or less */
	strcpy(line, GetWord(fnameval));
	if (strlen(line) > 8) goto LeaveFileIO;
	for (i=0; i<(int)strlen(line); i++)
	{
		if ((line[i]>='0' && line[i]<='9') || (line[i]>='A' && line[i]<='Z') ||
			(line[i]>='a' && line[i]<='z'))
		{
			continue;
		}
		else
			goto LeaveFileIO;
	}

	if (ioblock) goto LeaveFileIO;  /* can't nest file operations */

#if !defined (GLK)
	hugo_splitpath(program_path, drive, dir, fname, ext);
	hugo_makepath(fileiopath, drive, dir, GetWord(fnameval), "");
#else
	strcpy(fileiopath, GetWord(fnameval));
#endif

	if (iotype==WRITEFILE_T)        /* "writefile" */
	{
#if !defined (GLK)
		/* stdio implementation */
		if ((io = HUGO_FOPEN(fileiopath, "wb"))==nullptr) goto LeaveFileIO;
#else
		/* Glk implementation */
		frefid_t fref = nullptr;

		fref = glk_fileref_create_by_name(fileusage_Data | fileusage_BinaryMode,
			fileiopath, 0);
		io = glk_stream_open_file(fref, filemode_Write, 0);
		glk_fileref_destroy(fref);
		if (io==nullptr) goto LeaveFileIO;
#endif
		ioblock = 1;
	}
	else                            /* "readfile"  */
	{
#if !defined (GLK)
		/* stdio implementation */
		if ((io = HUGO_FOPEN(fileiopath, "rb"))==nullptr) goto LeaveFileIO;
#else
		/* Glk implementation */
		frefid_t fref = nullptr;

		fref = glk_fileref_create_by_name(fileusage_Data | fileusage_BinaryMode,
			fileiopath, 0);
		io = glk_stream_open_file(fref, filemode_Read, 0);
		glk_fileref_destroy(fref);
		if (io==nullptr) goto LeaveFileIO;
#endif
		ioblock = 2;
	}

#if defined (DEBUGGER)
	tempdbnest = dbnest++;
#endif

	SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);

	RunRoutine(codeptr);

#if defined (DEBUGGER)
	dbnest = tempdbnest;
#endif

	stack_depth = temp_stack_depth;

	if (ioerror) retflag = 0;

	hugo_fclose(io);
	io = nullptr;
	ioblock = 0;

LeaveFileIO:
	ioerror = 0;
	codeptr = skipaddr;
}

void Hugo::Flushpbuffer() {
	if (pbuffer[0]=='\0') return;

#ifdef USE_TEXTBUFFER
	/* For (single) characters left over from AP(), when Flushpbuffer() gets
	   called, say, from RunWindow()
	*/
	if (bufferbreaklen && pbuffer+bufferbreaklen && !(currentfont&PROP_FONT))
	{
		TB_AddWord(pbuffer+bufferbreak,
			current_text_x+bufferbreaklen,
			current_text_y,
			current_text_x+bufferbreaklen+FIXEDCHARWIDTH,
			current_text_y+lineheight-1);
	}
#endif

	pbuffer[strlen(pbuffer)+1] = '\0';
	pbuffer[strlen(pbuffer)] = (char)NO_NEWLINE;
	Printout(Ltrim(pbuffer), 0);
	currentpos = hugo_textwidth(pbuffer);	/* -charwidth; */
	strcpy(pbuffer, "");
}

void Hugo::GetCommand() {
	char a[256];
#ifdef USE_TEXTBUFFER
	int start, width, i, y;
#endif
	Flushpbuffer();
	AP("");

	hugo_settextcolor(fcolor);
	hugo_setbackcolor(bgcolor);
	if (icolor==-1)
		icolor = fcolor;	/* check unset input color */

	strncpy(a, GetWord(var[prompt]), 255);
	during_player_input = true;
	full = 0;
#ifdef USE_TEXTBUFFER
	/* Add the prompt to the textbuffer (using TB_Add()) */
	y = current_text_y;
	width = hugo_textwidth(GetWord(var[prompt]));
	TB_AddWord(GetWord(var[prompt]), physical_windowleft, y,
		physical_windowleft+width, y+lineheight-1);

	hugo_getline(a);
	
	/* If hugo_scrollwindowup() called by hugo_getline() shifted things */
	if (current_text_y > y)
	{
		y += (current_text_y - y);
	}

	/* Add each word in the input buffer */
	start = 0;
	for (i=0; i<(int)strlen(buffer); i++)
	{
		if (buffer[i]==' ')
		{
			buffer[i] = '\0';
			TB_AddWord(buffer+start, physical_windowleft+width, y-lineheight, 
				physical_windowleft+width+hugo_textwidth(buffer+start), y-1);
			width += hugo_textwidth(buffer+start) + hugo_textwidth(" ");
			start = i+1;
			buffer[i] = ' ';
		}
	}
	/* Add the final word */
	TB_AddWord(buffer+start, physical_windowleft+width, y-lineheight, 
		physical_windowleft+width+hugo_textwidth(buffer+start), y-1);
#else
	hugo_getline(a);
#endif
	during_player_input = false;
	strcpy(buffer, Rtrim(buffer));

	strcpy(parseerr, "");

	full = 1;
	remaining = 0;
}

char *Hugo::GetString(long addr) {
	static char a[256];
	int i, length;

	length = Peek(addr);

	for (i=1; i<=length; i++)
		a[i-1] = (char)(Peek(addr + i) - CHAR_TRANSLATION);
	a[i-1] = '\0';

	return a;
}

char *Hugo::GetText(long textaddr) {
	static char g[1025];
	int i, a;
	int tdatal, tdatah, tlen;       /* low byte, high byte, length */


	/* Read the string from memory... */
	if (loaded_in_memory)
	{
		tlen = MEM(codeend+textaddr) + MEM(codeend+textaddr+1)*256;
		for (i=0; i<tlen; i++)
		{
			g[i] = (char)(MEM(codeend+textaddr+2+i) - CHAR_TRANSLATION);
		}
		g[i] = '\0';

		return g;
	}

	/* ...Or load the string from disk */
	if (hugo_fseek(game, codeend+textaddr, SEEK_SET)) FatalError(READ_E);

	tdatal = hugo_fgetc(game);
	tdatah = hugo_fgetc(game);
	if (tdatal==EOF || tdatah==EOF || hugo_ferror(game)) FatalError(READ_E);

	tlen = tdatal + tdatah * 256;

	for (i=0; i<tlen; i++)
	{
		if ((a = hugo_fgetc(game))==EOF) FatalError(READ_E);
		g[i] = (char)(a - CHAR_TRANSLATION);
	}
	g[i] = '\0';

	return g;
}

char *Hugo::GetWord(unsigned int w) {
	char *b;
	unsigned short a;

	a = w;

	if (a==0) return EMPTY;

	if (a==PARSE_STRING_VAL) return parseerr;
	if (a==SERIAL_STRING_VAL) return serial;

	/* bounds-checking to avoid some sort of memory arena error */
	if ((long)(a+dicttable*16L) > codeend)
	{
		return EMPTY;
	}

	defseg = dicttable;
	b = GetString((long)a + 2);
	defseg = gameseg;

	return b;
}

void Hugo::HandleTailRecursion(long addr) {
	codeptr = addr;

	/* Set up proper default return value for property or routine */
	if (tail_recursion==TAIL_RECURSION_PROPERTY)
		ret = 1;
	else
		ret = 0;

	/* Unstack until we get to any routine call that got us here */
	while (stack_depth)
	{
		if (code_block[stack_depth].type==RUNROUTINE_BLOCK)
			break;
		stack_depth--;
	}

#ifdef DEBUGGER
	currentroutine = (unsigned int)(codeptr/address_scale);
	call[window[VIEW_CALLS].count-1].addr = currentroutine;
	call[window[VIEW_CALLS].count-1].param = true;

	sprintf(debug_line, "Calling:  %s", RoutineName(currentroutine));
	/* Don't duplicate blank separator line in code window */
	if (codeline[window[CODE_WINDOW].count-1][0] != 0)
		AddStringtoCodeWindow("");
	AddStringtoCodeWindow(debug_line);

	/* Adjust for very long runs */
	dbnest--;
#endif

	tail_recursion = 0;
	tail_recursion_addr = 0;
}

void Hugo::InitGame() {
	int i;

	/* Stop any audio if this is a restart */
	hugo_stopsample();
	hugo_stopmusic();

#if !defined (COMPILE_V25)
	hugo_stopvideo();
	context_commands = 0;
#endif
	game_reset = false;
	
	SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);

	/* Figure out which objects have either a noun or an adjective property;
	   store it in obj_parselist, one bit per object */
	if ((!obj_parselist) && (obj_parselist = (char *)hugo_blockalloc(sizeof(char)*((objects+7)/8))))
	{
		
		for (i=0; i<objects; i++)
		{
			if (i%8==0) obj_parselist[i/8] = 0;
			
			if (PropAddr(i, adjective, 0) || PropAddr(i, noun, 0))
				obj_parselist[i/8] |= 1 << (i%8);
			else
				obj_parselist[i/8] &= ~(1 << (i%8));
		}
	}
	
#if defined (GLK)
	if (_savegameSlot == -1) {
#endif
#if defined (DEBUGGER)
	for (i=0; i<MAXLOCALS; i++) strcpy(localname[i], "");
	window[VIEW_LOCALS].count = current_locals = 0;

	PassLocals(0);
	DebugRunRoutine((long)initaddr*address_scale);
#else
	PassLocals(0);
	RunRoutine((long)initaddr*address_scale);
#endif
#if defined (GLK)
	}
#endif
	ret = 0;
	retflag = 0;
	var[actor] = var[player];
}

void Hugo::LoadGame() {
	int i, data;
	unsigned int synptr;
	size_t ccount;
	int check_version;
	long textbank, filelength;
#ifndef LOADGAMEDATA_REPLACED
	long c;
#endif

#if defined (DEBUGGER)
	if (!strcmp(gamefile, ""))
	{
		game = nullptr;
		strcpy(gamefile, "(no file)");
		return;
	}
#endif

#if !defined (GLK) /* since in Glk the game stream is always open */
	if ((game = TrytoOpen(gamefile, "rb", "games"))==nullptr)
	{
		if ((game = TrytoOpen(gamefile, "rb", "object"))==nullptr)
			FatalError(OPEN_E);
	}
#else
	game = &_gameFile;
#endif

	hugo_fseek(game, 0, SEEK_END);
	filelength = hugo_ftell(game);
	hugo_fseek(game, 0, SEEK_SET);

	if (hugo_ferror(game)) FatalError(READ_E);
	if ((game_version = hugo_fgetc(game))==EOF) FatalError(OPEN_E);

	/* Earlier versions of the compiler wrote the version code as
	   1 or 2 instead of 10 or 20.
	*/
	if (game_version==1 || game_version==2)
		game_version*=10;

	if (game_version < 21) object_size = 12;

	if (game_version < 31) address_scale = 4;

	check_version = HEVERSION*10 + HEREVISION;
#if defined (COMPILE_V25)
	if (check_version==25 && (game_version>=30 && game_version<=39))
		check_version = game_version;
#endif

	defseg = gameseg;

	if (game_version < HEVERSION)
	{
#if defined (DEBUGGER)
		hugo_cleanup_screen();
		hugo_clearfullscreen();
#endif
		sprintf(line, "Hugo Compiler v%d.%d or later required.\n", HEVERSION, HEREVISION);
		if (game_version>0)
			sprintf(line+strlen(line), "File \"%s\" is v%d.%d.\n", gamefile, game_version/10, game_version%10);

#if defined (DEBUGGER_PRINTFATALERROR)
		DEBUGGER_PRINTFATALERROR(line);
#else
		//debug("%s", line);
#endif
		hugo_closefiles();
		hugo_blockfree(mem);
		mem = nullptr;

		hugo_exit(line);
	}
	else if (game_version > check_version)
	{
#if defined (DEBUGGER)
		hugo_cleanup_screen();
		hugo_clearfullscreen();
#endif
		sprintf(line, "File \"%s\" is incorrect or unknown version.\n", gamefile);

#if defined (DEBUGGER_PRINTFATALERROR)
		DEBUGGER_PRINTFATALERROR(line);
#else
		//debug("%s", line);
#endif
		hugo_closefiles();
		hugo_blockfree(mem);
		mem = nullptr;
		hugo_exit(line);           /* ditto */
	}

	hugo_settextpos(1, physical_windowheight/lineheight);

	if (game_version>=25)
		hugo_fseek(game, H_TEXTBANK, SEEK_SET);
	else
		/* Because pre-v2.5 didn't have performaddr in the header */
		hugo_fseek(game, H_TEXTBANK-2L, SEEK_SET);

	data = hugo_fgetc(game);
	textbank = hugo_fgetc(game);
	if (data==EOF || textbank==EOF || hugo_ferror(game)) FatalError(READ_E);
	textbank = (textbank*256L + (long)data) * 16L;
	codeend = textbank;

	/* Use a 1024-byte read block */
	ccount = 1024;

	if (hugo_fseek(game, 0, SEEK_SET)) FatalError(READ_E);

#ifndef LOADGAMEDATA_REPLACED
	/* Allocate as much memory as is required */
	if ((!loaded_in_memory) || (mem = (unsigned char *)hugo_blockalloc(filelength))==nullptr)
	{
		loaded_in_memory = false;
		if ((mem = (unsigned char *)hugo_blockalloc(codeend))==nullptr)
			FatalError(MEMORY_E);
	}

	c = 0;

	/* Load either the entire file or just up to the start of the
	   text bank
	*/
	while (c < (loaded_in_memory ? filelength:codeend))
	{
		/* Complicated, but basically just makes sure that
		   the last read (whether loaded_in_memory or not)
		   doesn't override the end of the file.  Shouldn't
		   normally be a problem for fread(), but it caused
		   a crash under MSVC++.
		*/
		i = hugo_fread((unsigned char *)&mem[c], sizeof(unsigned char),
			(loaded_in_memory)?
				((filelength-c>(long)ccount)?ccount:(size_t)(filelength-c)):
				((codeend-c>(long)ccount)?ccount:(size_t)(codeend-c)),
			game);

		if (!i) break;
		c += i;
	}
#else
	if (!LoadGameData(false)) FatalError(READ_E);
#endif

	if (hugo_ferror(game)) FatalError(READ_E);

	defseg = gameseg;

	/* Read header: */

	id[0] = Peek(H_ID);
	id[1] = Peek(H_ID+1);
	id[2] = '\0';

	for (i=0; i<8; i++)
		serial[i] = Peek(H_SERIAL+i);
	serial[8] = '\0';

	codestart  = PeekWord(H_CODESTART);
	objtable   = PeekWord(H_OBJTABLE) + gameseg;
	proptable  = PeekWord(H_PROPTABLE) + gameseg;
	eventtable = PeekWord(H_EVENTTABLE) + gameseg;
	arraytable = PeekWord(H_ARRAYTABLE) + gameseg;
	dicttable  = PeekWord(H_DICTTABLE) + gameseg;
	syntable   = PeekWord(H_SYNTABLE) + gameseg;

	initaddr       = PeekWord(H_INIT);
	mainaddr       = PeekWord(H_MAIN);
	parseaddr      = PeekWord(H_PARSE);
	parseerroraddr = PeekWord(H_PARSEERROR);
	findobjectaddr = PeekWord(H_FINDOBJECT);
	endgameaddr    = PeekWord(H_ENDGAME);
	speaktoaddr    = PeekWord(H_SPEAKTO);
	performaddr    = PeekWord(H_PERFORM);


	/* Read totals: */

	defseg = objtable;
	objects = PeekWord(0);

	defseg = eventtable;
	events = PeekWord(0);

	defseg = dicttable;
	dictcount = PeekWord(0);

	defseg = syntable;
	syncount = PeekWord(0);


	/* Additional information to be found: */

	/* display object */
	if (game_version >= 24)
	{
		data = FindWord("(display)");

		for (i=0; i<objects; i++)
		{
			if (GetProp(i, 0, 1, true)==data)
			{
				display_object = i;
				break;
			}
		}
	}
	
	/* build punctuation string (additional user-specified punctuation) */
	synptr = 2;
	strcpy(punc_string, "");
	for (i=1; i<=syncount; i++)
	{
		defseg = syntable;
		if (Peek(synptr)==3)	/* 3 = punctuation */
		{
			strcpy(line, GetWord(PeekWord(synptr+1)));
			if (strlen(line) + strlen(punc_string) > 63) break;
			strcat(punc_string, line);
		}
		synptr+=5;
	}
}

#if !defined (GLK)	/* ParseCommandLine() is omitted for Glk */

signed char def_fcolor    = DEF_FCOLOR;
signed char def_bgcolor   = DEF_BGCOLOR;
signed char def_slfcolor  = DEF_SLFCOLOR;
signed char def_slbgcolor = DEF_SLBGCOLOR;

void ParseCommandLine(int argc, char *argv[])
{
	char drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILENAME], ext[MAXEXT];
	char* game_file_arg = nullptr;

#if defined(GCC_UNIX) && defined(DO_COLOR)
        int ch;
	/* Parse comand line options (colour switches) */
	while ((ch = getopt(argc, argv, "f:b:F:B:?h")) != -1) {
	  switch (ch) {
	    case 'f':
	      def_fcolor = atoi(optarg);
	      break;
	    case 'b':
	      def_bgcolor = atoi(optarg);
	      break;
	    case 'F':
	      def_slfcolor = atoi(optarg);
	      break;
	    case 'B':
	      def_slbgcolor = atoi(optarg);
	      break;
	    case 'h':
	    case '?':
	    default:
	      Banner();
	      if (mem) hugo_blockfree(mem);
	      mem = nullptr;
	      exit(0);
	  }
	}
	if ( optind < argc ) {
	  game_file_arg = argv[optind];
	}
#else
	if (argc>1) {
	  game_file_arg = argv[1];
	}
#endif

	if (game_file_arg==nullptr)
	{
		Banner();
		if (mem) hugo_blockfree(mem);
		mem = nullptr;
		exit(0);
	}

	hugo_splitpath(game_file_arg, drive, dir, fname, ext);

	if (strcmp(ext, ""))
		strcpy(gamefile, game_file_arg);
	else
		hugo_makepath(gamefile, drive, dir, fname,
#if defined (DEBUGGER)
			"hdx");
#else
			"hex");
#endif

	if (getenv("HUGO_SAVE"))
		hugo_makepath(savefile, "", getenv("HUGO_SAVE"), fname, "sav");
	else
		hugo_makepath(savefile, drive, dir, fname, "sav");

	if (getenv("HUGO_RECORD"))
		hugo_makepath(recordfile, "", getenv("HUGO_RECORD"), fname, "rec");
	else
		hugo_makepath(recordfile, drive, dir, fname, "rec");

	strcpy(scriptfile, DEF_PRN);

	hugo_makepath(gamepath, drive, dir, "", "");
}

#endif	/* GLK */

void Hugo::PassLocals(int n) {
	int i;

	for (i=0; i<MAXLOCALS; i++)
	{
		var[MAXGLOBALS+i] = passlocal[i];
		passlocal[i] = 0;
	}
	arguments_passed = n;
}

#ifdef NO_INLINE_MEM_FUNCTIONS

/* PEEK */

unsigned char Peek(long a)
{
	return MEM(defseg * 16L + a);
}


/* PEEKWORD */

unsigned int PeekWord(long a)
{
	return (unsigned char)MEM(defseg*16L+a) + (unsigned char)MEM(defseg*16L+a+1)*256;
}


/* POKE */

void Poke(unsigned int a, unsigned char v)
{
	SETMEM(defseg * 16L + a, v);
}


/* POKEWORD */

void PokeWord(unsigned int a, unsigned int v)
{
	SETMEM(defseg * 16L + a, (char)(v%256));
	SETMEM(defseg * 16L + a + 1, (char)(v/256));
}

#endif	/* NO_INLINED_MEM_FUNCTIONS */

const char *Hugo::PrintHex(long a) {
	static char hex[7];
	int h = 0;

	strcpy(hex, "");

	if (a < 0L) a = 0;

	hex[h++] = '0';
	if (a < 65536L) hex[h++] = '0';
	if (a < 4096L) hex[h++] = '0';
	if (a < 256L) hex[h++] = '0';
	if (a < 16L) hex[h++] = '0';

	sprintf(hex+h, "%lX", a);

	return hex;
}

void Hugo::Printout(char *a, int no_scrollback_linebreak) {
	char b[2], sticky = 0, trimmed = 0;
	char tempfcolor;
	int i, l;
	int n;
	int last_printed_font = currentfont;

	/* hugo_font() should do this if necessary, but just in case */
	if (lineheight < FIXEDLINEHEIGHT)
		lineheight = FIXEDLINEHEIGHT;
	
	tempfcolor = fcolor;

	/* The before-check of the linecount: */
	if (full)
	{
		/* -1 here since it's before printing */
		if (full >= physical_windowheight/lineheight-1)
			PromptMore();
	}

	if ((a[0]!='\0') && a[strlen(a)-1]==(char)NO_NEWLINE)
	{
		a[strlen(a)-1] = '\0';
		sticky = true;
	}

	b[0] = b[1] = '\0';


	/* The easy part is just skimming <a> and processing each code
	   or printed character, as the case may be:
	*/
	
	l = 0;	/* physical length of string */

	for (i=0; i<(int)strlen(a); i++)
	{
		if ((a[i]==' ') && !trimmed && currentpos==0)
		{
			continue;
		}

		if ((unsigned char)a[i] > ' ' || a[i]==FORCED_SPACE)
		{
			trimmed = true;
			last_printed_font = currentfont;
		}

		switch (b[0] = a[i])
		{
			case FONT_CHANGE:
				n = (int)(a[++i]-1);
				if (currentfont != n)
					hugo_font(currentfont = n);
				break;

			case COLOR_CHANGE:
				fcolor = (char)(a[++i]-1);
				hugo_settextcolor((int)fcolor);
				hugo_setbackcolor((int)(a[++i]-1));
				hugo_font(currentfont);
				break;

			default:
				if (b[0]==FORCED_SPACE) b[0] = ' ';
				l += hugo_charwidth(b[0]);

				/* A minor adjustment for font changes and RunWindow() to make
				   sure we're not printing unnecessarily downscreen
				*/
				if ((just_left_window) && current_text_y > physical_windowbottom-lineheight)
				{
					current_text_y = physical_windowbottom-lineheight;
				}
				just_left_window = false;

				hugo_print(b);
		}

		if (script && (unsigned char)b[0]>=' ')
			if (hugo_fprintf(script, "%s", b) < 0) FatalError(WRITE_E);

#if defined (SCROLLBACK_DEFINED)
		if (!inwindow && (unsigned char)b[0]>=' ')
		{
#ifdef USE_SMARTFORMATTING
			/* Undo smart-formatting for ASCII scrollback */
			switch ((unsigned char)b[0])
			{
				case 151:
					hugo_sendtoscrollback("--");
					continue;
				case 145:
				case 146:
					b[0] = '\'';
					break;
				case 147:
				case 148:
					b[0] = '\"';
			}
#endif
			hugo_sendtoscrollback(b);
		}
#endif
	}

	/* If we've got a linefeed and didn't hit the right edge of the
	   window
	*/
#ifdef NO_TERMINAL_LINEFEED
	if (!sticky)
#else
	if (!sticky && currentpos+l < physical_windowwidth)
#endif
	{
		/* The background color may have to be temporarily set if we're
		   not in a window--the reason is that full lines of the
		   current background color might be printed by the OS-specific
		   scrolling function.  (This behavior is overridden by the
		   Hugo Engine for in-window printing, which always adds new
		   lines in the current background color when scrolling.)
		*/
		hugo_setbackcolor((inwindow)?bgcolor:default_bgcolor);
		hugo_print("\r");

		i = currentfont;
		hugo_font(currentfont = last_printed_font);

#ifndef GLK
		if (currentline > physical_windowheight/lineheight)
		{
			int full_limit = physical_windowheight/lineheight;

			hugo_scrollwindowup();

			if ((current_text_y)
				&& full >= full_limit-3
				&& physical_windowbottom-current_text_y-lineheight > lineheight/2)
			{
				PromptMore();
			}
			currentline = full_limit;
		}

		/* Don't scroll single-line windows before PromptMore() */
		else if (physical_windowheight/lineheight > 1)
#endif
		{
			hugo_print("\n");
		}

		hugo_font(currentfont = i);
		hugo_setbackcolor(bgcolor);
	}

#if defined (AMIGA) && !defined (GLK)
	else
	{
		if (currentpos + l >= physical_windowwidth)
			AmigaForceFlush();
	}
#endif
	just_left_window = false;

	/* If no newline is to be printed after the current line: */
	if (sticky)
	{
		currentpos += l;
	}

	/* Otherwise, take care of all the line-feeding, line-counting,
	   etc.
	*/
	else
	{
		currentpos = 0;
		if (currentline++ > physical_windowheight/lineheight)
			currentline = physical_windowheight/lineheight;

		if (!playback) skipping_more = false;

		++full;
		
		/* The after-check of the linecount: */
		if ((full) && full >= physical_windowheight/lineheight)
		{
			PromptMore();
		}

		if (script && !no_scrollback_linebreak)
		{
			if (hugo_fprintf(script, "%s", "\n")<0)
				FatalError(WRITE_E);
		}

#if defined (SCROLLBACK_DEFINED)
		if (!inwindow && !no_scrollback_linebreak) hugo_sendtoscrollback("\n");
#endif
	}

	fcolor = tempfcolor;
}

#ifndef PROMPTMORE_REPLACED

void Hugo::PromptMore() {
	char temp_during_player_input;
	int k, tempcurrentfont;
	int temp_current_text_y = current_text_y;
	
	if (playback && skipping_more)
	{
		full = 0;
		return;
	}
	skipping_more = false;
	
	/* Clear the key buffer */
	while (hugo_iskeywaiting()) hugo_getkey();

	temp_during_player_input = during_player_input;
	during_player_input = false;

	tempcurrentfont = currentfont;
	hugo_font(currentfont = NORMAL_FONT);

	hugo_settextpos(1, physical_windowheight/lineheight);

#ifdef NO_TERMINAL_LINEFEED
	/* For ports where it's possible, do a better "MORE..." prompt
	   without a flashing caret */
	if (default_bgcolor!=DEF_SLBGCOLOR)
	{
		/* system statusline colors */
		hugo_settextcolor(DEF_SLFCOLOR);
		hugo_setbackcolor(DEF_SLBGCOLOR);
	}
	else
	{
		/* system colors */
		if (DEF_FCOLOR < 8)
			hugo_settextcolor(DEF_FCOLOR+8); /* bright */
		else
			hugo_settextcolor(DEF_FCOLOR);
		hugo_setbackcolor(DEF_BGCOLOR);
	}

	if (current_text_y)
		current_text_y = physical_windowbottom - lineheight;

	/* Make sure we fit in a window */
	if (physical_windowwidth/FIXEDCHARWIDTH >= 19)
		hugo_print("     [MORE...]     ");
	else
		hugo_print("[MORE...]");

	if (!inwindow)
		hugo_setbackcolor(default_bgcolor);
	else
		hugo_setbackcolor(bgcolor);

	/* Note that hugo_iskeywaiting() must flush the printing buffer,
	   if one is being employed */
	while (!hugo_iskeywaiting())
	{
		hugo_timewait(100);
	}

	k = hugo_waitforkey();

#else
	/* The normal, with-caret way of doing it */
	/* system colors */
	hugo_settextcolor(16); /* DEF_FCOLOR);  */
	hugo_setbackcolor(17); /* DEF_BGCOLOR); */
	hugo_print("[MORE...]");

	k = hugo_waitforkey();

	if (!inwindow)
		hugo_setbackcolor(default_bgcolor);
	else
		hugo_setbackcolor(bgcolor);
#endif

	if (playback && k==27)         /* if ESC is pressed during playback */
	{
		if (hugo_fclose(playback))
			FatalError(READ_E);
		playback = nullptr;
	}
	else if (playback && k=='+')
		skipping_more = true;

	hugo_settextpos(1, physical_windowheight/lineheight);
#ifdef NO_TERMINAL_LINEFEED
	current_text_y = physical_windowbottom - lineheight;
	/* Make sure we fit in a window */
	if (physical_windowwidth/FIXEDCHARWIDTH >= 19)
		hugo_print("                    ");
	else
		hugo_print("         ");
#else
	hugo_print("         ");
#endif
	hugo_font(currentfont = tempcurrentfont);

	hugo_settextpos(1, physical_windowheight/lineheight);
	current_text_y = temp_current_text_y;
	full = 0;

	hugo_settextcolor(fcolor);
	hugo_setbackcolor(bgcolor);

	during_player_input = temp_during_player_input;
}

#endif	/* ifndef PROMPTMORE_REPLACED */

int Hugo::RecordCommands() {
	remaining = 0;
	skipping_more = false;

	switch (Peek(codeptr))
	{
		case RECORDON_T:
		{
			if (!record && !playback)
			{
#if !defined (GLK)
				/* stdio implementation */
				hugo_getfilename("for command recording", recordfile);
				if (!strcmp(line, ""))
					return 0;
				if (!hugo_overwrite(line))
					return 0;
				if (!(record = HUGO_FOPEN(line, "wt")))
					return 0;
				strcpy(recordfile, line);
#else
				/* Glk implementation */
				frefid_t fref = nullptr;

				fref = glk_fileref_create_by_prompt(fileusage_Transcript | fileusage_TextMode,
					filemode_Write, 0);
				record = glk_stream_open_file(fref, filemode_Write, 0);
				glk_fileref_destroy(fref);
				if (!record)
					return 0;
#endif
				return 1;
			}
			break;
		}

		case RECORDOFF_T:
		{
			if (playback) return 1;

			if (record)
			{
				if (hugo_fclose(record)) return (0);

				record = nullptr;
				return 1;
			}
			break;
		}

		case PLAYBACK_T:
		{
			if (!playback)
			{
#if !defined (GLK)
				/* stdio implementation */
				hugo_getfilename("for command playback", recordfile);
				if (!strcmp(line, ""))
					return 0;
				if (!(playback = HUGO_FOPEN(line, "rt")))
					return 0;
				strcpy(recordfile, line);
#else
				/* Glk implementation */
				frefid_t fref = nullptr;

				fref = glk_fileref_create_by_prompt(fileusage_InputRecord | fileusage_TextMode,
					filemode_Read, 0);
				playback = glk_stream_open_file(fref, filemode_Read, 0);
				glk_fileref_destroy(fref);
				if (!playback)
					return 0;
#endif
				return 1;
			}
			break;
		}
	}
	return 0;
}

void Hugo::SaveUndo(int a, int b, int c, int d, int e) {
	int tempptr;

	if (undorecord)
	{
		undostack[undoptr][0] = a;      /* save the operation */
		undostack[undoptr][1] = b;
		undostack[undoptr][2] = c;
		undostack[undoptr][3] = d;
		undostack[undoptr][4] = e;

		/* Put zeroes at end of this operation in case
		   the stack wraps around */
		tempptr = undoptr;
		if (++undoptr==MAXUNDO) undoptr = 0;
		undostack[undoptr][0] = 0;
		undostack[undoptr][1] = 0;
		undoptr = tempptr;

		if (++undoturn==MAXUNDO)        /* turn too complex */
			{undoptr = 0;
			undoturn = MAXUNDO;
			undoinvalid = 1;}

		if (++undoptr==MAXUNDO) undoptr = 0;
	}
}

void Hugo::SetStackFrame(int depth, int type, long brk, long returnaddr) {
	if (depth==RESET_STACK_DEPTH) stack_depth = 0;
	else if (++stack_depth>=MAXSTACKDEPTH) FatalError(MEMORY_E);

	code_block[stack_depth].type = type;
	code_block[stack_depth].brk = brk;
	code_block[stack_depth].returnaddr = returnaddr;

#if defined (DEBUGGER)
	code_block[stack_depth].dbnest = dbnest;
#endif
}

void Hugo::SetupDisplay() {
	hugo_settextmode();

	hugo_settextwindow(1, 1,
		SCREENWIDTH/FIXEDCHARWIDTH, SCREENHEIGHT/FIXEDLINEHEIGHT);

	last_window_left = 1;
	last_window_top = 1;
	last_window_right = SCREENWIDTH/FIXEDCHARWIDTH;
	last_window_bottom = SCREENHEIGHT/FIXEDLINEHEIGHT;

	hugo_settextcolor(16);
	hugo_setbackcolor(17);
	hugo_clearfullscreen();
}

char Hugo::SpecialChar(const char *a, int *i) {
	char r, s, skipbracket = 0;

	r = a[*i];
	s = r;

	if (r=='\"') return r;

	/* For a couple of versions, Hugo allowed Inform-style
	   punctuation control characters; I don't remember
	   exactly why.
	*/
	if (game_version <= 22)
		if (r=='~' || r=='^') return r;

	if (r=='(')
		{r = a[++*i];
		skipbracket = true;}

	switch (r)
	{
		case '`':               /* accent grave */
		{
			/* Note that the "s = '...'" characters are
			   Latin-1 and may not display properly under,
			   e.g., DOS */

			s = a[++*i];
#ifndef NO_LATIN1_CHARSET
			switch (s)
			{
				case 'a':  s = (char)0xe0; break;
				case 'e':  s = (char)0xe8; break;
				case 'i':  s = (char)0xec; break;
				case 'o':  s = (char)0xf2; break;
				case 'u':  s = (char)0xf9; break;
				case 'A':  s = (char)0xc0; break;
				case 'E':  s = (char)0xc8; break;
				case 'I':  s = (char)0xcc; break;
				case 'O':  s = (char)0xd2; break;
				case 'U':  s = (char)0xd9; break;
			}
#endif
			break;
		}
		case '\'':              /* accent acute */
		{
			s = a[++*i];
#ifndef NO_LATIN1_CHARSET
			switch (s)
			{
				case 'a':  s = (char)0xe1; break;
				case 'e':  s = (char)0xe9; break;
				case 'i':  s = (char)0xed; break;
				case 'o':  s = (char)0xf3; break;
				case 'u':  s = (char)0xfa; break;
				case 'y':  s = (char)0xfd; break;
				case 'A':  s = (char)0xc1; break;
				case 'E':  s = (char)0xc9; break;
				case 'I':  s = (char)0xcd; break;
				case 'O':  s = (char)0xd3; break;
				case 'U':  s = (char)0xda; break;
				case 'Y':  s = (char)0xdd; break;
			}
#endif
			break;
		}
		case '~':               /* tilde */
		{
			s = a[++*i];
#ifndef NO_LATIN1_CHARSET
			switch (s)
			{
				case 'a':  s = (char)0xe3; break;
				case 'n':  s = (char)0xf1; break;
				case 'o':  s = (char)0xf5; break;
				case 'A':  s = (char)0xc3; break;
				case 'N':  s = (char)0xd1; break;
				case 'O':  s = (char)0xd5; break;
			}
#endif
			break;
		}
		case '^':               /* circumflex */
		{
			s = a[++*i];
#ifndef NO_LATIN1_CHARSET
			switch (s)
			{
				case 'a':  s = (char)0xe2; break;
				case 'e':  s = (char)0xea; break;
				case 'i':  s = (char)0xee; break;
				case 'o':  s = (char)0xf4; break;
				case 'u':  s = (char)0xfb; break;
				case 'A':  s = (char)0xc2; break;
				case 'E':  s = (char)0xca; break;
				case 'I':  s = (char)0xce; break;
				case 'O':  s = (char)0xd4; break;
				case 'U':  s = (char)0xdb; break;
			}
#endif
			break;
		}
		case ':':               /* umlaut */
		{
			s = a[++*i];
#ifndef NO_LATIN1_CHARSET
			switch (s)
			{
				case 'a':  s = (char)0xe4; break;
				case 'e':  s = (char)0xeb; break;
				case 'i':  s = (char)0xef; break;
				case 'o':  s = (char)0xf6; break;
				case 'u':  s = (char)0xfc; break;
				/* case 'y':  s = (char)0xff; break; */
				case 'A':  s = (char)0xc4; break;
				case 'E':  s = (char)0xcb; break;
				case 'I':  s = (char)0xcf; break;
				case 'O':  s = (char)0xd6; break;
				case 'U':  s = (char)0xdc; break;
			}
#endif
			break;
		}
		case ',':               /* cedilla */
		{
			s = a[++*i];
#ifndef NO_LATIN1_CHARSET
			switch (s)
			{
				case 'C':  s = (char)0xc7; break;
				case 'c':  s = (char)0xe7; break;
			}
#endif
			break;
		}
		case '<':               /* Spanish left quotation marks */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xab;
#endif
			break;
		case '>':               /* Spanish right quotation marks */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xbb;
			break;
#endif
		case '!':               /* upside-down exclamation mark */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xa1;
#endif
			break;
		case '?':               /* upside-down question mark */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xbf;
#endif
			break;
		case 'a':               /* ae ligature */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xe6; ++*i;
#else
			s = 'e'; ++*i;
#endif
			break;
		case 'A':               /* AE ligature */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xc6; ++*i;
#else
			s = 'E'; ++*i;
#endif
			break;
		case 'c':               /* cents symbol */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xa2;
#endif
			break;
		case 'L':               /* British pound */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xa3;
#endif
			break;
		case 'Y':               /* Japanese Yen */
#ifndef NO_LATIN1_CHARSET
			s = (char)0xa5;
#endif
			break;
		case '-':               /* em dash */
#ifndef NO_LATIN1_CHARSET
			/* s = (char)0x97; */
#endif
			break;
		case '#':               /* 3-digit decimal code */
		{
			s = (char)((a[++*i]-'0')*100);
			s += (a[++*i]-'0')*10;
			s += (a[++*i]-'0');
#ifdef NO_LATIN1_CHARSET
			if ((unsigned)s>127) s = '?';
#endif
		}
	}

	if (skipbracket)
	{
		++*i;
		if (a[*i+1]==')') ++*i;
		if (s==')') s = r;
	}

	return s;
}

#if !defined (GLK)	/* not used for Glk */

HUGO_FILE TrytoOpen(char *f, char *p, char *d)
{
	char drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILENAME], ext[MAXEXT];
	char envvar[32];
	HUGO_FILE tempfile; char temppath[MAXPATH];

	/* Try to open the given, vanilla filename */
	if ((strcmp(f, "")) && (tempfile = HUGO_FOPEN(f, p)))
	{
		return tempfile;
	}

	hugo_splitpath(f, drive, dir, fname, ext);      /* file to open */

	/* If the given filename doesn't already specify where to find it */
	if (!strcmp(drive, "") && !strcmp(dir, ""))
	{
		/* Check gamefile directory */
		hugo_makepath(temppath, "", gamepath, fname, ext);

		if ((tempfile = HUGO_FOPEN(temppath, p)))
		{
			strcpy(f, temppath);    /* the new pathname */
			return tempfile;
		}

		/* Check environment variables */
		strcpy(envvar, "hugo_");        /* the actual var. name */
		strcat(envvar, d);

		if (getenv(strupr(envvar)))
		{
			hugo_makepath(temppath, "", getenv(strupr(envvar)), fname, ext);

			if ((tempfile = HUGO_FOPEN(temppath, p)))
			{
				strcpy(f, temppath);  /* the new pathname */
				return tempfile;
			}
		}
	}

	return nullptr;            /* return nullptr if not openable */
}

#endif	/* GLK */

int Hugo::Undo() {
	int count = 0, n;
	int turns, turncount, tempptr;
	int obj, prop, attr, v;
	unsigned int addr;

	if (--undoptr < 0) undoptr = MAXUNDO-1;

	if (undostack[undoptr][1]!=0)
	{
		/* Get the number of operations to be undone for
		   the last turn.
		*/
		if ((turns = undostack[undoptr][1]) >= MAXUNDO)
			goto CheckUndoFailed;

		turns--;

		turncount = 0;
		tempptr = undoptr;

		/* Count the number of operations available to see if there
		   are enough to undo the last turn (as per the number
		   required in <turns>.
		*/
		do
		{
			if (--undoptr < 0) undoptr = MAXUNDO-1;
			turncount++;

			/* if end of turn */
			if (undostack[undoptr][0]==0)
				break;
		}
		while (true);

		if (turncount<turns) goto CheckUndoFailed;

		undoptr = tempptr;

		if (--undoptr < 0) undoptr = MAXUNDO-1;

		while (undostack[undoptr][0] != 0)
		{
			switch (undostack[undoptr][0])
			{
				case MOVE_T:
				{
					MoveObj(undostack[undoptr][1], undostack[undoptr][2]);
					count++;
					break;
				}

				case PROP_T:
				{
					obj = undostack[undoptr][1];
					prop = undostack[undoptr][2];
					n = undostack[undoptr][3];
					v = undostack[undoptr][4];

					if ((addr = PropAddr(obj, prop, 0))!=0)
					{
						defseg = proptable;

						if (n==PROP_ROUTINE)
						{
							Poke(addr+1, PROP_ROUTINE);
							n = 1;
						}

						/* Use this new prop count number if the
						   existing one is too low or a prop routine
						*/
						else if (Peek(addr+1)==PROP_ROUTINE || Peek(addr+1)<(unsigned char)n)
							Poke(addr+1, (unsigned char)n);

						/* property length */
						if (n<=(int)Peek(addr+1))
							PokeWord(addr+2+(n-1)*2, v);
					}
					count++;
					break;
				}

				case ATTR_T:
				{
					obj = undostack[undoptr][1];
					attr = undostack[undoptr][2];
					n = undostack[undoptr][3];
					SetAttribute(obj, attr, n);
					count++;
					break;
				}

				case VAR_T:
				{
					n = undostack[undoptr][1];
					v = undostack[undoptr][2];
					var[n] = v;
					count++;
					break;
				}

				case ARRAYDATA_T:
				{
					defseg = arraytable;
					addr = undostack[undoptr][1];
					n = undostack[undoptr][2];
					v = undostack[undoptr][3];

				/* The array length was already accounted for before calling
				   SaveUndo(), so there is no adjustment of
				   +2 here.
				*/
					PokeWord(addr+n*2, v);
					count++;
					break;
				}

				case DICT_T:
				{
					defseg = dicttable;
					PokeWord(0, --dictcount);
					count++;
					break;
				}
				case WORD_T:
				{
					n = undostack[undoptr][1];
					v = undostack[undoptr][2];
					wd[n] = v;
					word[n] = GetWord(wd[n]);
					count++;
				}
			}
			defseg = gameseg;

			if (--undoptr < 0) undoptr = MAXUNDO-1;
		}
	}

CheckUndoFailed:
	if (!count)
	{
		undoinvalid = 1;
		game_reset = false;
		return 0;
	}

	game_reset = true;

	undoptr++;
	return 1;
}

/*
 * Random-number generator replacement by Andrew Plotkin:
 *
 */

#if defined (BUILD_RANDOM)

static unsigned int rand_table[55];	/* state for the RNG */
static int rand_index1, rand_index2;

int random()
{
    rand_index1 = (rand_index1 + 1) % 55;
    rand_index2 = (rand_index2 + 1) % 55;
    rand_table[rand_index1] = rand_table[rand_index1] - rand_table[rand_index2];
    return rand_table[rand_index1];
}

void srandom(int seed)
{
	int k = 1;
	int i, loop;

	rand_table[54] = seed;
	rand_index1 = 0;
	rand_index2 = 31;
	
	for (i = 0; i < 55; i++)
	{
		int ii = (21 * i) % 55;

		rand_table[ii] = k;
		k = seed - k;
		seed = rand_table[ii];
	}
	
	for (loop = 0; loop < 4; loop++)
	{
		for (i = 0; i < 55; i++)
			rand_table[i] = rand_table[i] - rand_table[ (1 + i + 30) % 55];
	}
}

#endif /* BUILD_RANDOM */

} // End of namespace Hugo
} // End of namespace Glk
