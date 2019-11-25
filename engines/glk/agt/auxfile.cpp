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
#include "common/str.h"

namespace Glk {
namespace AGT {

/* ------------------------------------------------------------------- */
/* Purity flag initialization                                          */
/*    Logically, these belong in agtdata.c, but I wanted to keep them  */
/*    near the CFG reading routines.                                   */
/* ------------------------------------------------------------------- */
/*   The following are AGT 'purity' flags; they turn off features of */
/* my interpreter that are not fully consistent with the original AGT */
/* and so could break some games. Some of these are trivial improvements; */
/* some are more radical and should be used with caution. Several are */
/* only useful if a game was designed with them in mind. */
/*   In all cases, setting the flag to 1 more closely follows the */
/* behavior of the original interpreters */
/* WARNING: Many of these haven't been tested extenstivly in the non-default
   state. */


rbool PURE_ANSWER = 0; /* For ME questions, requires that AND-separated
              answers be in the same order in the player's
             answer as they are in the game file. According
             to the AGT documentation, AND should ignore
             the order, but the original AGT interpreters
             (at least the one I've tested) don't conform
             to this. */

rbool PURE_TIME = 1; /* Set to 0 causes time to always be increased
                by delta_time rather than by a random amount
            between 0 and delta_time. Only really of any use
            to a game author who wanted to write a game
            explicitly for AGiliTy. */

/* rbool PURE_BOLD=1; Set to 0 causes the backslash to toggle bold on and
            off for all versions of AGT, not just 1.8x.
            I can think of no reason to do this unless
            you are an AGT author who wants to use the 1.8x
            bold feature with the Master's Edition compiler. */

rbool PURE_AND = 1; /* increment the turn counter for each noun in a
                chain of <noun> AND <noun> AND ... If 0, the turn
             counter will only be incremented by one in such a case.
             (need to do something about metacommands, as well...) */

rbool PURE_METAVERB = 1; /* If set, ANY and AFTER commands are run even
               if you type in a metaverb (SAVE, RESTORE,...
               that is, any verb that doesn't cause time to
               pass). Verb specific metacommands are _always_
               run. */

rbool PURE_ROOMTITLE = 1;  /* If 0, the interpreter will print out room
               names before room descriptions even for
               pre-ME games */

rbool PURE_SYN = 0; /* Treats synonyms as nouns when parsing: that is, they
               must show up only as the last word and they have the
               same priority as noun matches during disambiguation.
              If this is 0, then synonyms can appear anywhere in
               the name the player types in but are still
               disambiguated as nouns. */

rbool PURE_NOUN = 0; /* _Requires_ a noun to end a word. This is only
             imperfectly supported: if there are no other
             possible matches the parser will take the adjective-
             only one anyhow. Frankly, I can't think of any reason
             to set this to 1, but it's included for completeness
             sake (and for any AGT Purists out there :-) ) */

rbool PURE_ADJ = 1; /* Picks noun/syn-matches over pure adj matches
               when disambiguating. This is redundant if PURE_NOUN=1
               since in that case pure adjective matches will
               be rejected anyhow. */

rbool PURE_DUMMY = 0;  /* If set, the player can running dummy verbs
             in the game by typing 'dummy_verb3'; otherwise,
             this will produce an error message */

rbool PURE_SUBNAME = 0; /* If set, the player can run subroutines from
              the parse line by typing (e.g.) 'subroutine4'
              (yes, the original AGT interpreters actually
              allow this). If cleared, this cheat isn't
              available */
rbool PURE_PROSUB = 0;  /* If clear, then $you$ substitutions are done
            everywhere $$ substitutions are, even in
            messages written by the game author.
            If set, these substitutions are only made
            in internal game messages */

rbool PURE_HOSTILE = 1;  /* =0 Will allow you to leave a room with a hostile
               creature if you go back the way you came */
rbool PURE_ALL = 1;      /* =0 will cause the parser to expand ALL */
rbool PURE_DISAMBIG = 1; /* =0 will cause intelligent disambiguation */
rbool PURE_GETHOSTILE = 1;  /* =0 will prevent the player from picking things
                up in a room with a hostile creature */

rbool PURE_OBJ_DESC = 1;    /* =0 prevents [providing light] messages
              from being shown */

rbool PURE_ERROR = 0;    /* =1 means no GAME ERROR messages will be printed
             out */

rbool PURE_SIZE = 1;  /* =0 eliminates size/weight limits on how many
              things the player can wear or carry. (But it's
              still impossible to pick things up that are
              in themselves larger than the player's capacity) */

rbool PURE_GRAMMAR = 1; /* =0 prints error messages if the player uses a
            built in verb with an extra object.
            (e.g. YELL CHAIR). Otherwise, the extra object
            will just be ignored. */

rbool PURE_SYSMSG = 1; /* =0 causes AGiliTy to always use the default
               messages even if the game file has its own
               standard error messages. */

rbool PURE_AFTER = 1; /* =0 causes LOOK and other end-of-turn events
               to happen *before* AFTER commands run.  */

rbool PURE_PROPER = 1; /* Don't automatically treat creatures as proper nouns */

rbool TWO_CYCLE = 0; /* AGT 1.83-style two-cycle metacommand execution. */
rbool FORCE_VERSION = 0; /* Load even if the version is wrong. */


/*-------------------------------------------------------------------------*/
/* .CFG reading routines                                                   */
/*-------------------------------------------------------------------------*/

/* The main interpreter handles configuration in this order:
   1) Global configuration file
   2) First pass through game specific CFG to get the settings for
       SLASH_BOLD and IBM_CHAR which we need to know _before_ reading
       in the game.
   3) Read in the game.
   4) Main pass through game specific CFG. Doing it here ensures that
      its settings will override those in the gamefile.
  Secondary programs (such as agt2agx) usually only call this once, for
  the game specific configuration file.
      */

#define opt(s) (strcasecmp(optstr[0],s)==0)

static void cfg_option(int optnum, char *optstr[], rbool lastpass)
/* This is passed each of the options; it is responsible for parsing
   them or passing them on to the platform-specific option handler
   agt_option() */
/* lastpass is set if it is the last pass through this configuration
   file; it is false only on the first pass through the game specific
   configuration file during the run of the main interpreter */
{
	rbool setflag;

	if (optnum == 0 || optstr[0] == NULL) return;

	if (strncasecmp(optstr[0], "no_", 3) == 0) {
		optstr[0] += 3;
		setflag = 0;
	} else setflag = 1;

	if (opt("slash_bold")) bold_mode = setflag;
	else if (!lastpass) {
		/* On the first pass, we ignore all but a few options */
		agil_option(optnum, optstr, setflag, lastpass);
		return;
	} else if (opt("irun")) irun_mode = setflag;
	else if (opt("block_hostile")) PURE_HOSTILE = setflag;
	else if (opt("get_hostile")) PURE_GETHOSTILE = setflag;
	else if (opt("debug")) {
		if (!agx_file && aver <= AGTME10) debug_mode = setflag;
		if (setflag == 0) debug_mode = 0; /* Can always turn debugging support off */
	} else if (opt("pure_answer")) PURE_ANSWER = setflag;
	else if (opt("const_time")) PURE_TIME = !setflag;
	else if (opt("fix_multinoun")) PURE_AND = !setflag;
	else if (opt("fix_metaverb")) PURE_METAVERB = !setflag;
	else if (opt("roomtitle")) PURE_ROOMTITLE = !setflag;
	else if (opt("pure_synonym")) PURE_SYN = setflag;
	else if (opt("adj_noun")) PURE_ADJ = !setflag;
	else if (opt("pure_dummy")) PURE_DUMMY = setflag;
	else if (opt("pure_subroutine")) PURE_SUBNAME = setflag;
	else if (opt("pronoun_subs")) PURE_PROSUB = !setflag;
	else if (opt("verbose")) verboseflag = setflag;
	else if (opt("fixed_font")) font_status = 1 + !setflag;
	else if (opt("alt_any")) mars_fix = setflag;
	else if (opt("smart_disambig")) PURE_DISAMBIG = !setflag;
	else if (opt("expand_all")) PURE_ALL = !setflag;
	else if (opt("object_notes")) PURE_OBJ_DESC = setflag;
	else if (opt("error")) PURE_ERROR = !setflag;
	else if (opt("ignore_size")) PURE_SIZE = !setflag;
	else if (opt("check_grammar")) PURE_GRAMMAR = !setflag;
	else if (opt("default_errors")) PURE_SYSMSG = !setflag;
	else if (opt("pure_after")) PURE_AFTER = !setflag;
	else if (opt("proper_creature")) PURE_PROPER = !setflag;
	else agil_option(optnum, optstr, setflag, lastpass);
}

#undef opt

/* Returns false if it there are too many tokens on the line */
rbool parse_config_line(char *buff, rbool lastpass) {
	char *opt[50], *p;
	int optc;

	optc = 0;
	opt[0] = NULL;
	for (p = buff; *p; p++) {
		if (isspace(*p)) {  /* Whitespace */
			if (opt[optc] != NULL) { /*... which means this is the first whitespace */
				if (optc == 50) return 0; /* Too many */
				opt[++optc] = NULL;
			}
			*p = 0;
		} else  /* No whitespace */
			if (opt[optc] == NULL) /* ...this is the first non-whitespace */
				opt[optc] = p;
	}
	if (opt[optc] != NULL) opt[++optc] = NULL;
	cfg_option(optc, opt, lastpass);
	return 1;
}


/* For the meaning of lastpass, see comments to cfg_option() above */
void read_config(genfile cfgfile, rbool lastpass) {
	char buff[100];

	if (!filevalid(cfgfile, fCFG)) return;

	while (readln(cfgfile, buff, 99)) {
		if (buff[0] == '#') continue; /* Comments */
		/* Now we parse the line into words, with opt[] pointing at the words
		   and optc counting how many there are. */
		if (!parse_config_line(buff, lastpass))
			rprintf("Too many tokens on configuration line.\n");
	}
	readclose(cfgfile);
}



/*-------------------------------------------------------------------------*/
/* Read OPT file                                                          */
/*  (most of these routines used to be in agil.c)                          */
/*-------------------------------------------------------------------------*/

/* .OPT reading routines */
/* I've put the comments on the format here because they don't really
   belong anywhere else. (Maybe in agility.h, but I don't want to further
   clutter that already quite cluttered file with something as peripheral
   as this) */
/* OPT file format:  the .OPT file consists of 14 bytes. They are:
   0  Screen size(0=43/50 rows, 1=25 rows)
   1  Status line(1=top, 0=none, -1=bottom)
   2  Unknown, always seems to be 0
   3  Put box around status line?
   4  Sound on?
   5  Menus on?
   6  Fixed input line?
   7  Print transcript?
   8  Height of menus (3, 4, 5, 6, 7, or 8)
   9  Unknown, always seems to be 0
   10-13  Color scheme: output/status/input/menu, specified in DOS attribute
      format (Bbbbffff,  B=blink, b=backround, f=foreground,
      MSB of foreground specifies intensity ("bold") ). */
/* The interpreter ignores almost all of this. */

void read_opt(fc_type fc) {
	const char *errstr;
	genfile optfile;

	have_opt = 0;
	optfile = openbin(fc, fOPT, NULL, 0);
	if (filevalid(optfile, fOPT)) {
		if (!binread(optfile, opt_data, 14, 1, &errstr))
			fatal("Invalid OPT file.");
		have_opt = 1;
		readclose(optfile);
	}
}


/*-------------------------------------------------------------------------*/
/* Read and process TTL                                                    */
/*  (most of these routines used to be in agil.c)                          */
/*-------------------------------------------------------------------------*/

/* Shades of Gray uses a custom interpreter that prints out the names
   of the authors as the program loads. */
/* Normally I wouldn't bother with this, but Shades of Gray is probably
   the best known of all AGT games */

#define SOGCREDIT 7
static const char *sogauthor[SOGCREDIT] = {
	"Mark \"Sam\" Baker",
	"Steve \"Aaargh\" Bauman",
	"Belisana \"The\" Magnificent",
	"Mike \"of Locksley\" Laskey",
	"Judith \"Teela Brown\" Pintar",
	"Hercules \"The Loyal\" SysOp",
	"Cindy \"Nearly Amelia\" Yans"
};

static rbool check_dollar(char *s)
/* Determines if s consists of an empty string with a single dollar sign
 and possibly whitespace */
{
	rbool dfound;
	dfound = 0;
	for (; *s != 0; s++)
		if (*s == '$' && !dfound) dfound = 1;
		else if (!rspace(*s)) return 0;
	return dfound;
}

descr_line *read_ttl(fc_type fc) {
	genfile ttlfile;
	int i, j, height;
	descr_line *buff;

	ttlfile = openfile(fc, fTTL, NULL, 0);
	/* "Warning: Could not open title file '%s'." */
	if (!filevalid(ttlfile, fTTL)) return NULL;
	build_fixchar();

	buff = (descr_line *)rmalloc(sizeof(descr_line));
	i = 0;
	while (NULL != (buff[i] = readln(ttlfile, NULL, 0))) {
		if (strncmp(buff[i], "END OF FILE", 11) == 0) break;
		else if (aver >= AGT18 && aver <= AGT18MAX && check_dollar(buff[i]))
			statusmode = 4;
		else {
			for (j = 0; buff[i][j] != 0; j++)
				buff[i][j] = fixchar[(uchar)buff[i][j]];
			/* Advance i and set the next pointer to NULL */
			buff = (descr_line *)rrealloc(buff, sizeof(descr_line) * (++i + 1));
			buff[i] = NULL;
		}
		rfree(buff[i]);
	}
	readclose(ttlfile);

	rfree(buff[i]);
	while (buff[i] == NULL || strlen(buff[i]) <= 1) { /* Discard 'empty' lines */
		if (i == 0) break;
		rfree(buff[i]);
		i--;
	}
	height = i;

	if (aver == AGTCOS && ver == 4 && height >= 17) /* SOGGY */
		for (i = 0; i < SOGCREDIT; i++)
			if (strlen(sogauthor[i]) + 9 + i < strlen(buff[i + 7]))
				memcpy(buff[i + 7] + 9 + i, sogauthor[i], strlen(sogauthor[i]));

	return buff;
}

void free_ttl(descr_line *title) {
	int i;
	if (title == NULL) return;
	for (i = 0; title[i] != NULL; i++)
		rfree(title[i]);
	rfree(title);
}


/*-------------------------------------------------------------------------*/
/* Read and convert VOC                                                    */
/*  (most of these routines used to be in agil.c)                          */
/*-------------------------------------------------------------------------*/


static const char *newvoc[] = { "1 Menu", "1 Restart", "1 Undo" };
static int newindex = 0; /* Points into newvoc */

void add_verbrec(const char *verb_line, rbool addnew) {
	char s[3];
	Common::String verbStr(verb_line);

	while (!verbStr.empty() && rspace(verbStr.firstChar()))
		verbStr.deleteChar(0);

	if (verbStr.empty() || verbStr.hasPrefix("!"))
		return;		/* Comment or empty line */

	/* The following guarentees automatic initialization of the verbrec structures */
	if (!addnew)
		while (newindex < 3 && strcasecmp(verbStr.c_str() + 2, newvoc[newindex] + 2) > 0)
			add_verbrec(newvoc[newindex++], 1);

	verbinfo = (verbentry_rec *)rrealloc(verbinfo, (vm_size + 1) * sizeof(verbentry_rec));

	s[0] = verbStr.firstChar();
	s[1] = 0;
	verbinfo[vm_size].objnum = strtol(s, NULL, 10) - 1;

	verbStr.deleteChar(0);
	verbStr.deleteChar(0);

	verbinfo[vm_size].verb = verbinfo[vm_size].prep = 0;

	uint idx = 0;
	while (idx < verbStr.size()) {
		while (idx < verbStr.size() && !rspace(verbStr[idx]))
			++idx;
		if (idx < verbStr.size()) {
			verbStr.setChar('\0', idx);
			++idx;
		}

		verbinfo[vm_size].verb = search_dict(verbStr.c_str());
		if (verbinfo[vm_size].verb == -1) {
			verbinfo[vm_size].verb = 0;
			return;
		}
		if (idx < verbStr.size()) {
			verbinfo[vm_size].prep = search_dict(verbStr.c_str() + idx);
			if (verbinfo[vm_size].prep == -1)
				verbinfo[vm_size].prep = 0;
		}
	}

	vm_size++;
}

void init_verbrec(void)
/* Need to insert special verbs into verbinfo */
/* Fill in vnum field */
/* UNDO, RESTART, MENU  */
{
	verbinfo = NULL;
	vm_size = 0;
	newindex = 0;
	if (freeze_mode) newindex = 1;  /* Don't include MENU option if we can't
                   use it. */
}

void finish_verbrec(void) {
	for (; newindex < 3; newindex++) add_verbrec(newvoc[newindex], 1);
}


void read_voc(fc_type fc) {
	char linbuf[80];
	genfile vocfile;

	init_verbrec();
	vocfile = openfile(fc, fVOC, NULL, 0);
	if (filevalid(vocfile, fVOC)) { /* Vocabulary file exists */
		while (readln(vocfile, linbuf, 79))
			add_verbrec(linbuf, 0);
		readclose(vocfile);
		finish_verbrec();
	}
}




/*-------------------------------------------------------------------------*/
/* Read INS file                                                           */
/*  (most of these routines used to be in agil.c)                          */
/*-------------------------------------------------------------------------*/


static genfile insfile = BAD_TEXTFILE;
static char *ins_buff;

static descr_line *ins_descr = NULL;
static int ins_line;  /* Current instruction line */


/* Return 1 on success, 0 on failure */
rbool open_ins_file(fc_type fc, rbool report_error) {
	ins_buff = NULL;
	ins_line = 0;

	if (ins_descr != NULL) return 1;

	if (filevalid(insfile, fINS)) {
		textrewind(insfile);
		return 1;
	}

	if (agx_file) {
		ins_descr = read_descr(ins_ptr.start, ins_ptr.size);
		if (ins_descr != NULL) return 1;

		/* Note that if the AGX file doesn't contain an INS block, we
		   don't immediatly give up but try opening <fname>.INS */
	}

	insfile = openfile(fc, fINS,
	                   report_error
	                   ? "Sorry, Instructions aren't available for this game"
	                   : NULL,
	                   0);
	return (filevalid(insfile, fINS));
}

char *read_ins_line(void) {
	if (ins_descr) {
		if (ins_descr[ins_line] != NULL)
			return ins_descr[ins_line++];
		else return NULL;
	} else {
		rfree(ins_buff);
		ins_buff = readln(insfile, NULL, 0);
		return ins_buff;
	}
}

void close_ins_file(void) {
	if (ins_descr) {
		free_descr(ins_descr);
		ins_descr = NULL;
	} else if (filevalid(insfile, fINS)) {
		rfree(ins_buff);
		readclose(insfile);
		insfile = BAD_TEXTFILE;
	}
}



descr_line *read_ins(fc_type fc) {
	descr_line *txt;
	char *buff;
	int i;

	i = 0;
	txt = NULL;
	if (open_ins_file(fc, 0)) {  /* Instruction file exists */
		while (NULL != (buff = read_ins_line())) {
			/* Enlarge txt; we use (i+2) here to leave space for the trailing \0 */
			txt = (descr_line *)rrealloc(txt, sizeof(descr_ptr) * (i + 2));
			txt[i++] = rstrdup(buff);
		}
		if (txt != NULL)
			txt[i] = 0; /* There is space for this since we used (i+2) above */
		close_ins_file();
	}
	return txt;
}


void free_ins(descr_line *instr) {
	int i;
	if (instr == NULL) return;
	for (i = 0; instr[i] != NULL; i++)
		rfree(instr[i]);
	rfree(instr);
}



/* Character translation routines, used by agtread.c and read_ttl() */
void build_fixchar(void) {
	int i;
	for (i = 0; i < 256; i++) {
		if (i == '\r' || i == '\n') fixchar[i] = ' ';
		else if (i == '\\' && bold_mode) fixchar[i] = FORMAT_CODE;
		else if (i >= 0x80 && fix_ascii_flag)
			fixchar[i] = trans_ibm[i & 0x7f];
		else if (i == 0) /* Fix color and blink codes */
			fixchar[i] = FORMAT_CODE;
		else fixchar[i] = i;
	}
}

} // End of namespace AGT
} // End of namespace Glk
