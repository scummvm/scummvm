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

#include "glk/agt/agility.h"
#include "glk/agt/interp.h"

namespace Glk {
namespace AGT {

/* This module contains a miscellany of things that are somewhat */
/* system dependent but not enough so to justify being put in */
/* OS_<whatever>.c */
/* --writestr() and writeln().*/
/* --Hooks for sound, pictures, and fonts.  */
/* --yesno() and wait_return() */
/* --Some lower level file stuff */
/* --main() and command line parseing stuff */

#ifndef REPLACE_BNW

/* #define DEBUG_BELLS_AND_WHISTLES */

/* Warning for fontcmd, pictcmd, musiccmd:
  These all extract filenames from fontlist, pictlist, pixlist, songlist.
 Any of these are allowed to be NULL and this should be checked
 before accessing them.  */

#ifdef DEBUG_BELLS_AND_WHISTLES
void bnw_report(char *cmdstr, filename *list, int index) {
	writeln("");
	writestr(">** ");
	writestr(cmdstr);
	writestr(" ");
	if (list != NULL) {
		writestr(list[index]);
		writestr(" ");
	}
	writeln("**<");
}
#endif /* DEBUG_BELLS_AND_WHISTLES */

void fontcmd(int cmd, int font)
/* 0=Load font, name is fontlist[font]
   1=Restore original (pre-startup) font
   2=Set startup font. (<gamename>.FNT)
*/
{
#ifdef DEBUG_BELLS_AND_WHISTLES
	if (cmd == 0) bnw_report("Loading Font", fontlist, font);
	else if (cmd == 1) bnw_report("Restoring original font", NULL, 0);
#endif
	return;
}

void pictcmd(int cmd, int pict)
/* 1=show global picture, name is pictlist[pict]
   2=show room picture, name is pixlist[pict]
   3=show startup picture <gamename>.P..
  */
{
#ifdef DEBUG_BELLS_AND_WHISTLES
	if (cmd == 1) bnw_report("Showing picture", pictlist, pict);
	else if (cmd == 2) bnw_report("Showing pix", pixlist, pict);
	agt_waitkey();
#endif
	return;
}



int musiccmd(int cmd, int song)
/* For cmd=1 or 2, the name of the song is songlist[song]
  The other commands don't take an additional argument.
   1=play song
   2=repeat song
   3=end repeat
   4=end song
   5=suspend song
   6=resume song
   7=clean-up
   8=turn sound on
   9=turn sound off
   -1=Is a song playing? (0=false, -1=true)
   -2=Is the sound on?  (0=false, -1=true)
*/
{
	if (cmd == 8) sound_on = 1;
	else if (cmd == 9) sound_on = 0;
#ifdef DEBUG_BELLS_AND_WHISTLES
	switch (cmd) {
	case 1:
		bnw_report("Play song", songlist, song);
		break;
	case 2:
		bnw_report("Repeat song", songlist, song);
		break;
	case 3:
		bnw_report("End repeat", NULL, 0);
		break;
	case 4:
		bnw_report("End song", NULL, 0);
		break;
	case 5:
		bnw_report("Suspend song", NULL, 0);
		break;
	case 6:
		bnw_report("Resume song", NULL, 0);
		break;
	case 7:
		bnw_report("Clean up", NULL, 0);
		break;
	case 8:
		bnw_report("Sound On", NULL, 0);
		break;
	case 9:
		bnw_report("Sound Off", NULL, 0);
		break;
	case -1:
		return yesno("Is song playing?");
	case -2:
		return 1;
	}
#endif
	return 0;
}

#endif /* REPLACE_BNW */

static char linebuff[100];
static int lp;  /* Line pointer */
static rbool savenl = 0;
static rbool needfill; /* Used for paragraph filling */
static rbool quotemode = 0;

void debugout(const char *s) {
	int i;

	if (DEBUG_OUT) {
		debugfile->write(s, strlen(s));
	} else {
		lp = 0;
		for (; *s != 0; s++) {
			if (curr_x + lp >= screen_width || lp > 80) {
				if (lp + curr_x >= screen_width)
					lp = screen_width - curr_x - 1;
				linebuff[lp] = 0;
				agt_puts(linebuff);
				agt_newline();
				lp = 0;
			}
			if (*s == '\n') {
				linebuff[lp] = 0;
				agt_puts(linebuff);
				agt_newline();
				lp = 0;
			} else if (*s == '\t') {
				for (i = 0; i < 3; i++) linebuff[lp++] = ' ';
			} else if (*s >= 0 && *s <= 9) linebuff[lp++] = ' ';
			else linebuff[lp++] = *s;
		}
		linebuff[lp] = 0;
		agt_puts(linebuff);
	}
}


int close_pfile(genfile f, int ft)
/* ft=0 for script, 4 for log_in, 5 for log_out */
{
	delete f;
	return 0;
}



static char *get_log(void)
/* Read string from logfile_in */
{
	char *s;
	static int dead_log;

	if (!filevalid(log_in, fLOG)) { /* We are finishing up */
		if (++dead_log > 100) fatal("Internal error: LOG.");
		assert(BATCH_MODE);
		s = (char *)rmalloc(2);
		s[0] = ' ';
		s[1] = 0;
		return s;
	}

	s = (char *)rmalloc(1000);
	s[0] = ' ';
	s[1] = 0;
	(void)textgets(log_in, s, 1000);
	if (texteof(log_in)) {  /* Reached end of logfile */
		close_pfile(log_in, 1);
		log_in = BAD_TEXTFILE;
		if (BATCH_MODE) {
			writeln("");
			writeln("ERROR: Unexpected end of log file.");
			agt_quit(); /* This doesn't actually quit; it just sets things
             up so we *will* quit. */
			dead_log = 0;
		} else {
			logflag &= ~2;
			fast_replay = 0;
		}
	} else { /* Need to delay or wait for keypress */
		if (logdelay == -1) agt_waitkey();
		else agt_delay(logdelay);
		if (s[0] != 0) writeln(s);
	}
	return s;
}


static void put_log(const char *s)
/* Write s to logfile_out */
{
	textputs(log_out, s);
	if (s[strlen(s) - 1] != '\n')
		textputs(log_out, "\n");
}


char *agt_readline(int in_type) {
	char *s;

	if (PURE_INPUT) agt_textcolor(-1);
	if (logflag & 2)
		s = get_log();
	else
		s = agt_input(in_type);

	if (g_vm->shouldQuit())
		return nullptr;

	if (PURE_INPUT)
		agt_textcolor(-2);

	if (logflag & 1)
		put_log(s);

	return s;
}

char agt_getchar(void) {
	char c, *s, buff[2];

	if (PURE_INPUT) agt_textcolor(-1);
	if (logflag & 2) {
		s = get_log();
		c = s[0];
		rfree(s);
	} else
		c = agt_getkey(1);
	if (PURE_INPUT) agt_textcolor(-2);
	if (logflag & 1) {
		buff[0] = c;
		buff[1] = 0;
		put_log(buff);
	}
	return c;
}

void agt_center(rbool b)
/* 1=turn on text centering, 0=turn off */
/* At the moment, this is only used for game end messages */
/* When it is on, text output with writeln() should be 'centered'; */
/* it is up to the interface to decide what that means. */
/* writestr() should not be called while centering is on. */
{
	center_on = b;
}

void agt_par(rbool b)
/* This has been added for the sake of anyone trying to get this to */
/*  work on less-than-80 column screens. My personal opinion is that this */
/* is probably hopeless; many AGT games assume 80-column format for   */
/* creating tables and ascii graphics. Nevertheless... */
/*  Text between an agt_par(1) and an agt_par(0) is logically connected */
/* (all part of one description) and so it *might* be possible to reformat */
/* it, treating multiple lines as being one paragraph. */
/*    At the very least, you should look for blank lines and indentation */
/* since a single section of text could contain multiple paragraphs. */
/* Sections of text _not_ between an agt_par(1) and an agt_par(0) should */
/* be treated as though each line were a new paragraph */
{
	par_fill_on = b;
	if (b == 0 && savenl) agt_newline();
	savenl = 0;
	needfill = 0;
}

/* This handles the various format code. They all show up after
   '\r'; unrecogonized codes are just ignored */
static uchar xlat_format_code(uchar c) {
	if (c == 0xFF) {
		if (fix_ascii) return trans_ibm[0xFF - 0x80];
		else return 0xFF;
	}
	return 0;
}

#define FMT_CODE_CNT 15

static void run_format_code(uchar c) {
	if (c < FMT_CODE_CNT)
		agt_textcolor(c - 3);
}

#define format_code(c) ((c>0 && c<=LAST_TEXTCODE)||((uchar)c==FORMAT_CODE))

void writestr(const char *s) {
	int i, j;
	char c;
	int endmark, old_x;

	if (savenl) {
		assert(par_fill_on);
		if (!isalnum(s[0])) agt_newline();
		else agt_puts(" ");
		/* If combining two lines, insert a space between them. */
	}
	savenl = 0;
	i = 0;
	lp = 0;

	while (s[i] != 0) {
		for (; s[i] != 0 && lp < 90 && curr_x + lp < screen_width; i++)
			if (s[i] == '\t')
				for (j = 0; j < TAB_SIZE && curr_x + lp < screen_width; j++) linebuff[lp++] = ' ';
			else if (format_code(s[i])) {
				linebuff[lp++] = ' ';    /* Color code */
				break;
			} else if (s[i] == '\r') { /* New format code */
				if (s[i + 1] == 0) continue; /* Bogus format code */
				if (((uchar)s[i + 1]) < FMT_CODE_CNT) break;
				c = (char)xlat_format_code((uchar)s[++i]);
				if (c != 0) linebuff[lp++] = c;
			} else if (s[i] == '\n') {
				break;
			} else linebuff[lp++] = s[i];

		linebuff[lp] = 0;

		/* Backtrack to last space; in case of formatting codes, we should
		 already have one */
		endmark = lp;

		if (!isspace(s[i]) && !format_code(s[i]) && s[i] != 0) {
			/* If we aren't conveniently at a break...*/
			do {    /* Find last space */
				endmark--;
			} while (endmark > 0 && !isspace(linebuff[endmark]));
		}

		if (endmark == 0 && !isspace(linebuff[endmark])) { /* Can't find a break */
			if (curr_x + lp < screen_width) /* Not a line break */
				endmark = lp; /* Break at end; it doesn't matter that much */
			else  /* We _need_ a line break but are having trouble finding one */
				if (curr_x > 0) /* already stuff on this line printed previously */
					endmark = 0; /* i.e. print out nothing; move it to next line */
				else   /* We have a single word that is longer than our line */
					endmark = screen_width; /* Give up */
		}

		c = linebuff[endmark];
		linebuff[endmark] = 0;
		old_x = curr_x;

		agt_puts(linebuff);

		linebuff[endmark] = c;

		if (old_x + lp >= screen_width)
			/* Need to insert line break and skip any spaces */
		{
			if (!quotemode) agt_newline();
			else return; /* In quote mode, just truncate */

			/* Now set up beginning of next line: skip over whitespace */
			while (endmark < lp && isspace(linebuff[endmark]))
				endmark++;  /* Eliminate EOL whitespace */
			if (endmark == lp) {
				/* Nothing left; eliminate whitespace at beginning
				            of next line */
				while (isspace(s[i]) && s[i] != '\r') i++;
				lp = endmark = 0;
			}
			needfill = 1;
			if (endmark == lp && s[i] == 0) {
				needfill = 2;
				return; /* If only spaces left, don't print them */
			}
		}

		/* Now copy remaining text */
		for (j = 0; endmark < lp; j++, endmark++) linebuff[j] = linebuff[endmark];
		lp = j;

		/* Now to deal with format codes */
		if ((unsigned char)s[i] == FORMAT_CODE) {
			i++;
			if (bold_mode) { /* Translate as BOLD toggle */
				if (textbold)
					agt_textcolor(-2);  /* Turn bold off */
				else agt_textcolor(-1); /* Turn bold on */
				textbold = !textbold;
			} else /* translate as BLACK */
				agt_textcolor(0);
		} else if (s[i] > 0 && s[i] <= LAST_TEXTCODE)
			agt_textcolor(s[i++]);
		else if (s[i] == '\r') {
			run_format_code((uchar)s[i + 1]);
			i += 2;
		} else if (s[i] == '\n') {
			i += 1;
			agt_newline();
		}
	}
}



void writeln(const char *s) {
	int i, pad;
	char *padstr;

	if (center_on && (int)strlen(s) + curr_x < screen_width) {
		pad = (screen_width - strlen(s)) / 2;
		padstr = (char *)rmalloc((pad + 1) * sizeof(char));
		for (i = 0; i < pad; i++) padstr[i] = ' ';
		padstr[i] = 0;
		agt_puts(padstr);
		rfree(padstr);
	}
	writestr(s);
	/* needfill=2 if writestr ended with a line that wrapped only
	   because of excess spaces (which have been discarded); needfill==1
	   if writestr wrapped a line for any reason */
	/* If needfill==2, we've already issued a new-line, so don't issue a
	   second one. */
	/* If needfill==1, set savenl rather than wrapping (writestr will
	   then decide to wrap or not depending on whether the next line starts
	   with text or nontext), *unless* we are version magx, in which case
	   the game author presumably knew what they were doing, so honor their
	   wishes. */
	if (par_fill_on && needfill == 1)
		if (aver >= AGX00) agt_newline();
		else savenl = 1;
	else if (needfill != 2)
		agt_newline();
	needfill = 0;
}


static char fixstatchar(uchar c)
/* Eliminate formating characters in the status line */
{
	if (c == '\t' || c <= LAST_TEXTCODE ||
	        (c == FORMAT_CODE) || c == '\r' || c == '\n')
		return ' ';
	return c;
}

void print_statline(void)
/* Use strings in l_stat and r_stat */
{
	int i, j;
	char *s, *t;
	static rbool lastline = 0; /* Was a non-empty status line printed  last time? */

	s = (char *)rmalloc((status_width + 1) * sizeof(char));

	/* If both strings empty, don't print the status line */
	if (l_stat[0] == 0 && r_stat[0] == 0 && !lastline) return;
	lastline = (l_stat[0] || r_stat[0]);

	i = status_width - strlen(l_stat) - strlen(r_stat);

	j = 0;
	if (r_stat[0] == 0) { /* Center the status line */
		while (j < i / 2) s[j++] = ' ';
		i -= j;
	} else if (i > 6) {
		s[j++] = ' ';
		i -= 2;
	}  /* If statline is wide enough, put a
                       space on each side */

	if ((int)strlen(l_stat) < status_width)  /* Copy left side of status line into s*/
		for (t = l_stat; *t != 0; t++) s[j++] = fixstatchar(*t);

	for (; i > 0; i--) s[j++] = ' '; /* Insert space between left and right sides */

	if (j + (int)strlen(r_stat) <= status_width) /*Copy right side into s */
		for (t = r_stat; *t != 0; t++) s[j++] = fixstatchar(*t);

	while (j < status_width) s[j++] = ' '; /* Pad any extra width with spaces */
	s[j] = 0; /* Put end of string marker */
	agt_statline(s); /* Output it */
	rfree(s);
}


void padout(int padleng) {
	int i;
	char *pstr;

	if (padleng <= 0) return;
	pstr = (char *)rmalloc(padleng + 1);
	for (i = 0; i < padleng; i++) pstr[i] = ' ';
	pstr[padleng] = 0;
	writestr(pstr);
	free(pstr);
}

static int textwidth(char *s) {
	int n;

	n = 0;
	for (; *s != 0; s++) n += (*s == '\t') ? TAB_SIZE : 1;
	return n;
}

void textbox(char *(txt[]), int len, unsigned long flags)
/* TB_TTL, TB_BOLD, TB_BORDER, TB_CENTER */
{
	int i, width, padwidth;
	int *linewidth;

	agt_textcolor(7);
	if (flags & TB_BOLD) agt_textcolor(-1);
	else agt_textcolor(-2);

	linewidth = (int *)rmalloc(len * sizeof(int));

	width = 0; /* This contains the maximum width of any line */
	for (i = 0; i < len; i++) {
		linewidth[i] = textwidth(txt[i]);
		if (linewidth[i] > width) width = linewidth[i];
	}

	agt_makebox(width, len, flags & ~(TB_BOLD | TB_CENTER));
	quotemode = 1;  /* So newlines will cause truncation rather than a
           real newline */
	for (i = 0; i < len; i++) {
		padwidth = width - linewidth[i]; /* Amount of padding we need */
		if (flags & TB_CENTER) {
			padout(padwidth / 2);
			padwidth -= padwidth / 2;
		}
		writestr(txt[i]);
		padout(padwidth);
		if (i != len - 1) agt_qnewline();
	}
	agt_endbox();
	quotemode = 0; /* Back to normal */

	agt_textcolor(7);
	textbold = 0;
}


#ifndef REPLACE_MENU

int agt_menu(const char *header, int size, int width, menuentry *menu)
/* This is _very_ minimal as it stands */
{
	int i, j;
	char sbuff[10];
	int numcol, colheight;

	if (size == 0) return 0;

	width = width + 5;
	numcol = screen_width / width;
	colheight = size / numcol;
	if (size % numcol != 0) colheight++;

	writeln(header);
	for (i = 0; i < colheight; i++) {
		for (j = 0; j < numcol; j++) {
			if (j * colheight + i >= size) break;
			sprintf(sbuff, "%2d.", j * colheight + i + 1);
			writestr(sbuff);
			writestr(menu[j * colheight + i]);
			if (j < numcol - 1) padout(width - 3 - strlen(menu[j * colheight + i]));
		}
		writeln("");
	}
	do {
		writestr("Choice:");
		i = read_number() - 1;
		if (i < 0 || i >= size)
			writeln("Please choose an option from the menu.");
	} while (i < 0 || i >= size);
	return i;
}

#endif /* REPLACE_MENU */



void prompt_out(int n)
/* n=1 standard prompt
   n=2 question prompt */
{
	agt_textcolor(7);
	if (PURE_INPUT && n == 1) agt_textcolor(-1);
	if (n == 1) {
		agt_newline();
		gen_sysmsg(1, ">", MSG_MAIN, NULL);
	}
	if (n == 2) agt_puts("? ");
	agt_textcolor(7);
}

void agt_waitkey(void) {
	if (BATCH_MODE || fast_replay)
		return;
	agt_getkey(0);
}


void wait_return(void) {
	writeln("          --- HIT ANY KEY ---");
	agt_waitkey();
}


rbool yesno(const char *s)
/* True for yes, false for no. */
{
	char c;

	writestr(s);
	writestr(" ");
	c = 'y';
	do {
		if (c != 'y')
			writestr("Please answer <y>es or <n>o. ");
		c = tolower(agt_getchar());
	} while (c != 'y' && c != 'n' && !quitflag);
	return (c == 'y');
}


void set_test_mode(fc_type fc) {
	const char *errstr;

	log_in = readopen(fc, fLOG, &errstr);

	if (make_test) {
		if (errstr == NULL)
			fatal("Log file already exists.");
		log_out = writeopen(fc, fLOG, NULL, &errstr);
		if (errstr != NULL)
			fatal("Couldn't create log file.");
		logflag = 1;
		return;
	}

	logdelay = 0;
	if (errstr != NULL)
		fatal("Couldn't open log file.");
	logflag = 2;

	script_on = 1;
	scriptfile = writeopen(fc, fSCR, NULL, &errstr);
	if (errstr != NULL)
		fatal("Couldn't open script file.");
}


#ifndef REPLACE_GETFILE

/* This opens the file refered to by fname and returns it */
static genfile uf_open(fc_type fc, filetype ext, rbool rw) {
	char *errstr;
	genfile f;

	if (rw) { /* Check to see if we are overwriting... */
		if (fileexist(fc, ext) && ext != fSCR) {
			if (!yesno("This file already exists; overwrite?"))
				/* That is, DON'T overwrite */
				return badfile(ext);
		}
		f = writeopen(fc, ext, NULL, &errstr);
	} else
		f = readopen(fc, ext, &errstr);
	if (errstr != NULL) writeln(errstr);
	return f;
}

static fc_type last_save = NULL;
static fc_type last_log = NULL;
static fc_type last_script = NULL;


genfile get_user_file(int ft)
/* ft= 0:script, 1:save 2:restore, 3:log(read) 4:log(write)  */
/* Should return file in open state, ready to be read or written to,
   as the case may be */
{
	/* int extlen;*/
	rbool rw;  /* True if writing, false if reading */
	filetype ext;
	genfile fd;
	fc_type def_fc, fc;
	char *fname;
	char *ftype;
	char *p, *q;

	switch (ft) {
	case 0:
		ftype = "script ";
		def_fc = last_script;
		rw = 1;
		ext = fSCR;
		break;
	case 1:
		ftype = "save ";
		def_fc = last_save;
		rw = 1;
		ext = fSAV;
		break;
	case 2:
		ftype = "restore ";
		def_fc = last_save;
		rw = 0;
		ext = fSAV;
		break;
	case 3:
		ftype = "log ";
		def_fc = last_log;
		rw = 0;
		ext = fLOG;
		break;
	case 4:
		ftype = "log ";
		def_fc = last_log;
		rw = 1;
		ext = fLOG;
		break;
	default:
		writeln("<INTERNAL ERROR: invalid file type>");
		return badfile(fSAV);
	}

	writestr(" ");
	writestr("Enter ");
	if (ftype != NULL) writestr(ftype);
	writestr("file name");
	if (def_fc != NULL) {
		char *s;
		s = formal_name(def_fc, ext);
		writestr(" (");
		writestr(s);
		writestr(")");
		rfree(s);
	}
	writestr(": ");

	if (PURE_INPUT) agt_textcolor(-1);
	fname = agt_input(4);
	if (PURE_INPUT) agt_textcolor(-2);

	/* Delete whitespace before and after the file name. */
	for (p = fname; isspace(*p); p++);
	if (*p == 0) { /* Line is all whitespace; use default if there is one */
		if (def_fc == NULL) {
			writeln("Never mind.");
			rfree(fname);
			return badfile(ext);
		} else {
			rfree(fname);
			fc = def_fc;
		}
	} else {   /* Line is _not_ all whitespace: we have a file name */
		for (q = fname; *p != 0; p++, q++)
			*q = *p;
		q--;
		while (isspace(*q)) q--;
		q++;
		*q = 0;
		fc = init_file_context(fname, ext);
	}

	fd = uf_open(fc, ext, rw);

	if (!filevalid(fd, ext)) {
		if (fc != def_fc) release_file_context(&fc);
		return fd;
	}

	switch (ft) {
	case 0:
		last_script = fc;
		break;
	case 1:
		last_save = fc;
		break;
	case 2:
		last_save = fc;
		break;
	case 3:
		last_log = fc;
		break;
	case 4:
		last_log = fc;
		break;
	}
	if (fc != def_fc) release_file_context(&def_fc);
	return fd;
}


void set_default_filenames(fc_type fc) {
	last_save = convert_file_context(fc, fSAV, NULL);
	last_log = convert_file_context(fc, fLOG, NULL);
	last_script = convert_file_context(fc, fSCR, NULL);
}



#endif  /* REPLACE_GETFILE */




void script(uchar onp) {
	if (onp == script_on)
		if (onp == 0) writeln("Scripting wasn't on.");
		else writeln("Scripting is already on.");
	else if (onp == 1) {
		scriptfile = get_user_file(0);
		if (filevalid(scriptfile, fSCR)) script_on = 1;
	} else if (filevalid(scriptfile, fSCR)) {
		close_pfile(scriptfile, 0);
		scriptfile = BAD_TEXTFILE;
		script_on = 0;
	}
}


void logon(void) {
	if (logflag & 1) {
		writeln("Already logging");
		return;
	}
	log_out = get_user_file(4);
	if (filevalid(log_out, fLOG))
		logflag |= 1;
}

void replay(int delay) {
	if (logflag & 2) return; /* Nested replays are meaningless */
	log_in = get_user_file(3);
	if (filevalid(log_in, fLOG)) {
		logflag |= 2;
		logdelay = delay;
	}
}


/* These two are intended to be called by the platform-dependent
   interface (e.g. if the user had chosen these from some general purpose
   menu) */
/* They're never called from the rest of the code */

void agt_save(void) {
	g_vm->saveGame();
}

void agt_restore(void) {
	doing_restore = 1;
}

void agt_restart(void) {
	doing_restore = 2;
}

void agt_quit(void) {
	doing_restore = 4;
}


/* This should be rmalloc'd */
static fc_type newgame_fc;

fc_type new_game(void) {
	return newgame_fc;
}

void agt_newgame(fc_type fc) {
	newgame_fc = fc;
	doing_restore = 3;
}

#if 0
static rbool end_cmd_options;
#endif

void set_default_options(void) {
	init_flags();
	flag = (rbool *)rmalloc(sizeof(rbool));
	debug_parse = 0;
	DEBUG_AGT_CMD = 0;
	DEBUG_EXEC_VERB = 0;
	DEBUG_DISAMBIG = 0;
	DEBUG_SMSG = 0;
}

void helpmsg(void) {
	/*
	  printf(" -i Try to use IBM character set.\n");
	  printf(" -1 IRUN Mode: Print messages in first person\n");
	  printf(" -h Print out this message\n");
	  printf(" -d Debug metacommand execution\n");
	  printf(" -t Test mode; see accompanying documentation. Implies -r.\n");
	  printf(" -c Create test file.\n");
	  printf(" -m Force descriptions to be loaded from disk.\n");
	#ifdef OPEN_AS_TEXT
	  printf(" -b Open data files as binary files.\n");
	#endif
	  printf("\nTechnical options (intended for debugging AGiliTy itself).\n");
	  printf(" -p Debug parser\n");
	  printf(" -x Debug verb execution loop\n");
	  printf(" -a Debug disambiguation system\n");
	  printf(" -s Debug STANDARD message handler\n");
	*/
}

#if 0
static rbool setarg(char **optptr) {
	if ((*optptr)[1] == '+') {
		(*optptr)++;
		return 1;
	}
	if ((*optptr)[1] == '-') {
		(*optptr)++;
		return 0;
	}
	return 1;
}
#endif

#define fixcase(c) tolower(c)

#if 0
void parse_options(char *opt, char *next) {
	/*
	if (opt[0]=='-' && opt[1]==0)
	{end_cmd_options=1;return;}
	for(;*opt!=0;opt++)
	switch(fixcase(*opt))
	  {
	  case 'p': debug_parse=setarg(&opt);break;
	  case 'a': DEBUG_DISAMBIG=setarg(&opt);break;
	  case 'd': DEBUG_AGT_CMD=setarg(&opt);break;
	  case 'x':DEBUG_EXEC_VERB=setarg(&opt);break;
	  case 's':DEBUG_SMSG=setarg(&opt);break;
	#ifdef MEM_INFO
	  case 'M': DEBUG_MEM=setarg(&opt);break;
	#endif
	  case 'm': descr_maxmem=0; break;
	  case 'i': fix_ascii_flag=!setarg(&opt);break;
	  case 't': BATCH_MODE=setarg(&opt); break;
	  case 'c': make_test=setarg(&opt); break;
	  case '1': irun_mode=setarg(&opt);break;
	#ifdef OPEN_FILE_AS_TEXT
	  case 'b': open_as_binary=setarg(&opt);break;
	#endif
	  default:printf("Do not recognize option %c\n",*opt);
	helpmsg();
	exit(EXIT_FAILURE);
	  }
	*/
}
#endif

#ifndef REPLACE_MAIN

int main(int argc, char *argv[]) {
	int i;
	char *gamefile;

	set_default_options();
	end_cmd_options = 0;
	gamefile = NULL;
	for (i = 1; i < argc; i++)
		if (argv[i][0] == '-' && !end_cmd_options)
			parse_options(argv[i] + 1, argv[i + 1]);
		else if (gamefile == NULL)
			gamefile = argv[i];
		else fatal("Please specify only one game\n");
	if (gamefile == NULL)  {
		helpmsg();
		exit(EXIT_FAILURE);
	}

	init_interface(argc, argv);
	/* From this point on, MUST use writestr/writeln or may
	   cause problems w/ the interfaces on some platforms
	   that have to keep track of cursor position */

	run_game(init_file_context(gamefile, fDA1));
	return EXIT_SUCCESS;
}

#endif /* REPLACE_MAIN */

} // End of namespace AGT
} // End of namespace Glk
