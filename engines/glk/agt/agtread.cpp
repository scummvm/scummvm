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

namespace Glk {
namespace AGT {

static void read_da6(fc_type fc);
static void check_cmd_version(void);
static void build_cmd_table(void);
static void fixcmd(integer *, int);

/* This parses the block of integers of a command to standardize
   the command numbers */
static short *cmd_table;  /* Holds the command translation table used
                 by fixcmd */
int topcmd;    /* The highest legal opcode in the current AGT version. */



static genfile fd_desc;  /* File pointer for description file. */
static long desc_size;  /* Size of description file. */

static int top_quest; /* Highest question actually referenced */
/* This is computed by fixcmd */

static int SL_NAME, SL_TEXT, SL_ROOM, SL_WORD;
static integer MAX_CMD_SIZE;

static rbool encrypt_desc = 1; /* Are descriptions encrypted? */


/* This translates v1.8 status mode codes into ME statue mode codes */
const uchar agt18_statmode[] = {0, 4, 3, 1};


/*-------------------------------------------------------------------------*/
/* Utilities to convert strings, do ASCII translation, etc. and to add     */
/*   words to the dictionary (the actual dictionary routines are in        */
/*   agtdata.c; these routines here are just wrappers that prepare words   */
/*   to be added).                                                         */
/*-------------------------------------------------------------------------*/



static void fatals(const char *msg, const char *fname) {
	Common::String str = Common::String::format(msg, fname);
	error("%s", str.c_str());
}


static word add_dic1(uchar *buff, int n) {
	char nbuff[100];

	int i;
	if (n > 100) n = 100;
	for (i = 0; i < buff[0] && i < n; i++)
		nbuff[i] = buff[i + 1];
	nbuff[i] = 0;
	return add_dict(nbuff);
}


static slist add_slist(uchar *buff) {
	int j, k;
	slist start_ptr;
	char nbuff[100];

	k = 0;
	start_ptr = synptr;
	if (buff[0] > 80) fatal("Invalid game file format");
	for (j = 1; j <= buff[0]; j++)
		if (rspace(buff[j]) && k > 0) {
			nbuff[k] = 0;
			addsyn(add_dict(nbuff));
			k = 0;
		} else nbuff[k++] = buff[j];
	if (k > 0) {
		nbuff[k] = 0;
		addsyn(add_dict(nbuff));
	}
	addsyn(-1);  /* End of list marker */
	return start_ptr;
}




/*-------------------------------------------------------------------------*/
/* Description file manipulation routines: routines to open and close the  */
/*   description file and read in individual descriptions.                 */
/*-------------------------------------------------------------------------*/

/* The memory-based stuff is not done yet */


void convert_agt_descr(uchar *s)
/* Convert encrypted pascal string into plaintext C string */
{
	int j, n;

	n = s[0];
	if (n > 80) {
		s[0] = 0;
		return;
	}

	if (encrypt_desc)
		for (j = 0; j < n; j++)
			if (s[j + 1] != ' ')
				s[j] = fixchar[(s[j + 1] - (j + 1) + 0x100) & 0xFF];
			else s[j] = ' ';
	else
		for (j = 0; j < n; j++)
			s[j] = fixchar[s[j + 1]];

	s[n] = 0;
}


void open_descr(fc_type fc) {
	const char *errstr;
	long i;
	int alpha, cnt;
	tline buff;

	fd_desc = readopen(fc, fDSS, &errstr);
	if (errstr != NULL)  fatal(errstr);
	desc_size = binsize(fd_desc);
	if (DIAG) {
		char *s;
		s = formal_name(fc, fDSS);
		rprintf("Opened file %s (size:%ld)\n", s, desc_size);
		rfree(s);
	}

	/* <Sigh> Now need to figure out if the input is encoded. Do this by
	     reading a few random lines and seeing if they "look" encoded */
	alpha = cnt = 0;
	if (aver > AGT135 || aver == AGTCOS)
		encrypt_desc = 1;
	else {
		binread(fd_desc, buff, 81, 1, &errstr); /* Throw away first line */
		while (cnt < 300) {
			if (!binread(fd_desc, buff, 81, 1, &errstr)) { /* EOF */
				writeln("");
				agtwarn("EOF while analyzing descriptions", 0);
				rprintf("......assuming type ");
				break;
			}
			if (buff[0] > 0 && buff[1] != ' ' && buff[1] > 0) /* To avoid "banner"
                               lines */
			{
				for (i = 1; i <= buff[0]; i++) {
					if (buff[i] >= 'A' && buff[i] <= 'z') alpha++;
					if (buff[i] != ' ') cnt++;
				}
			}
		}
		if (3 * cnt < 4 * alpha) {
			encrypt_desc = 0;
			if (aver == AGT135) aver = AGT12;
		} else encrypt_desc = 1;
	}
	if (DIAG) {
		if (encrypt_desc) rprintf(" [encrypted]\n");
		else rprintf("  [plaintext: %d/%d]\n", alpha, cnt);
	}

	mem_descr = NULL;
	if (desc_size <= descr_maxmem) {
		/* This is where we need to read the data in and convert it:
		   encrypted Pascal strings --> plaintext C strings */
		binseek(fd_desc, 0);
		mem_descr = (char *)rmalloc(desc_size);
		/* Read in the whole file */
		binread(fd_desc, mem_descr, desc_size, 1, &errstr);
		if (errstr != NULL) fatal(errstr);
		for (i = 0; i < desc_size; i += sizeof(tline))
			convert_agt_descr((uchar *)(mem_descr + i));
		/* Decode and convert to C string */
	}
}


void close_descr(void) {
	if (mem_descr != NULL)
		rfree(mem_descr);
	else {
		readclose(fd_desc);
		fd_desc = NULL;
	}
}


descr_line *agt_read_descr(long start, long len) {
	tline *d;
	descr_line *lines;
	long i;
	const char *errstr;

	if (len == -1 || start == -1) return NULL;
	lines = (descr_line *)rmalloc(sizeof(descr_line) * (len + 1));

	if (mem_descr != NULL) {
		d = ((tline *)mem_descr) + start;
		for (i = 0; i < len; i++)
			lines[i] = (char *)(d + i);
	} else {
		d = (tline *)rmalloc(sizeof(tline) * len);
		binseek(fd_desc, start * sizeof(tline));
		binread(fd_desc, d, sizeof(tline), len, &errstr);
		if (errstr != NULL) fatal(errstr);
		for (i = 0; i < len; i++) {
			lines[i] = (char *)(d + i);
			convert_agt_descr((uchar *)(d + i));
		}
	}
	lines[len] = NULL; /* Mark end of array */
	return lines;
}



/*-------------------------------------------------------------------------*/
/* Read DA2: The Room File.                                                */
/*-------------------------------------------------------------------------*/


#define seti(a) (room[i].a=buff[bp] | (buff[bp+1]<<8),bp+=2)
#define set32(a) (room[i].a=buff[bp] | (buff[bp+1]<<8) | (buff[bp+2]<<16)|\
                            (buff[bp+3]<<24), bp+=4)
#define setb(a) (room[i].a=buff[bp],bp++)

#define setstr(leng) (bp+=(leng),new_str((char*)buff+bp-(leng),(leng),1))
#define setd(leng) (bp+=(leng),add_dic1(buff+bp-(leng),(leng)))
#define setsl() (bp+=sizeof(tline),add_slist(buff+bp-sizeof(tline)))
#define nonecheck(leng) (memcmp(buff+bp,nonestr,5)==0)


static void read_da2(fc_type fc) {
	int i, j, numroom;
	uchar *buff; /* [FRS_ROOM];*/
	long bp;

	numroom = maxroom - first_room + 1;
	if (numroom < 0) return;
	room_name = (long *)rmalloc(numroom * sizeof(long));

	buffopen(fc, fDA2, FRS_ROOM, "room", numroom);

	bp = 0;
	for (i = 0; i < numroom; i++) {
		buff = buffread(i);
		bp = 0;
		if (nonecheck(SL_ROOM))
			room[i].unused = 1;
		else room[i].unused = 0;
		room_name[i] = setstr(SL_ROOM);
		room[i].replace_word = setd(SL_WORD);
		room[i].replacing_word = setsl();
		for (j = 0; j < 12; j++) seti(path[j]);

		if (aver >= AGT15) set32(flag_noun_bits); /* Menu flags */
		else room[i].flag_noun_bits = 0;

		if (aver >= AGTME10) set32(PIX_bits); /* PIX bits */
		else room[i].PIX_bits = 0;

		seti(path[12]); /* Special */

		/* There's a small possibility that the 1.5/Hotel flag_noun_bits
		goes here, rather than above; 1.5/F is known to go above */

		setb(seen);
		seti(key);
		setb(locked_door);
		if (room_inside != NULL)
			room_inside[i] = fixsign16(buff[bp], buff[bp + 1]);
		bp += 2; /* Skip # of nouns in this room */

		seti(points);
		seti(light);
		setb(end);
		setb(win);
		if (aver != AGT10) setb(killplayer); /* I'm guessing here */
		else room[i].killplayer = room[i].end;

		if (aver >= AGTME10) {
			seti(initdesc);
			seti(pict);
		} else {
			room[i].initdesc = 0;
			room[i].pict = 0;
		}
		if (aver >= AGTME15) room[i].autoverb = setd(SL_WORD);
		else room[i].autoverb = 0;
		room[i].oclass = 0;
		room[i].seen = 0;
	}
	if (DIAG)
		rprintf("   Internal:%ld\n", bp);
	buffclose();
}



/*-------------------------------------------------------------------------*/
/* Read DA3: The Noun File.                                                */
/*-------------------------------------------------------------------------*/

#undef seti
#undef setb
#define seti(a) (noun[i].a=buff[bp] | (buff[bp+1]<<8),bp+=2)
#define setb(a) (noun[i].a=buff[bp],bp++)

static void read_da3(fc_type fc) {
	int i, numnoun;
	long recsize;
	uchar *buff; /* [FRS_NOUN];*/
	long bp;

	numnoun = maxnoun - first_noun + 1;
	if (numnoun < 0) return;
	noun_sdesc = (long *)rmalloc(numnoun * sizeof(long));
	noun_pos = (long *)rmalloc(numnoun * sizeof(long));

	recsize = buffopen(fc, fDA3, FRS_NOUN, "noun", numnoun);
	if (aver == AGT15 && recsize > 263) aver = AGT15F;

	bp = 0;
	for (i = 0; i < numnoun; i++) {
		buff = buffread(i);
		bp = 0;
		if (nonecheck(SL_NAME)) {
			bp += SL_NAME;
			noun[i].name = 0;
			noun[i].unused = 1;
		} else {
			noun[i].name = setd(SL_NAME);
			noun[i].unused = 0;
		}
		noun_sdesc[i] = setstr(SL_TEXT);
		noun[i].adj = setd(SL_NAME);

		if (aver >= AGT15F) seti(initdesc);
		else noun[i].initdesc = 0;

		setb(plural);
		/* The following is a guess for ME games */
		noun_pos[i] = setstr((ver == 3) ? SL_ROOM : SL_NAME);
		setb(something_pos_near_noun); /* These may not be valid */
		seti(nearby_noun);          /* in masters ed. */

		setb(has_syns);
		noun[i].syns = setsl(); /*,SL_TEXT);*/
		if (aver >= AGT15)
			noun[i].related_name = setd(SL_NAME);
		else
			noun[i].related_name = 0;
		seti(location);
		seti(weight);
		seti(size);
		seti(key);
		/* All of following flags known to be valid except
		   pullable, on, and win */
		setb(pushable);
		setb(pullable);
		setb(turnable);
		setb(playable);
		setb(readable);
		setb(on);
		setb(closable);
		setb(open);
		setb(lockable);
		setb(locked);
		setb(edible);
		setb(wearable);
		setb(drinkable);
		setb(poisonous);
		setb(movable);
		setb(light);
		setb(shootable);
		seti(num_shots);
		seti(points);
		if (noun_inside != NULL)
			noun_inside[i] = fixsign16(buff[bp], buff[bp + 1]);
		bp += 2; /* Skip # of nouns contained in this one */
		setb(win);
		if (ver == 3) seti(pict);
		else noun[i].pict = 0;
		noun[i].oclass = 0; /* AGT games don't support classes */
		noun[i].isglobal = 0;
		noun[i].flagnum = 0;
		noun[i].seen = 0;
		noun[i].proper = 0;
	}
	if (DIAG)
		rprintf("   Internal:%ld\n", bp);
	buffclose();
}


#undef seti
#undef setb
#define seti(a) (creature[i].a=buff[bp] | (buff[bp+1]<<8),bp+=2)
#define setb(a) (creature[i].a=buff[bp],bp++)



/*-------------------------------------------------------------------------*/
/* Read DA4: The Creature File.                                            */
/*-------------------------------------------------------------------------*/

static void read_da4(fc_type fc) {
	int i, numcreat;
	uchar *buff; /* [FRS_CREAT];*/
	long bp;

	numcreat = maxcreat - first_creat + 1;
	if (numcreat <= 0) return;
	creat_sdesc = (long *)rmalloc(numcreat * sizeof(long));

	buffopen(fc, fDA4, FRS_CREAT, "creature", numcreat);

	bp = 0;
	for (i = 0; i < numcreat; i++) {
		buff = buffread(i);
		bp = 0;
		if (nonecheck(SL_NAME)) {
			bp += SL_NAME;
			creature[i].name = 0;
			creature[i].unused = 1;
		} else {
			creature[i].name = setd(SL_NAME);
			creature[i].unused = 0;
		}
		creat_sdesc[i] = setstr(SL_TEXT);
		creature[i].adj = setd(SL_NAME);
		if (ver == 3) seti(initdesc);
		else creature[i].initdesc = 0;
		setb(has_syns);
		creature[i].syns = setsl();
		setb(groupmemb);
		seti(location);
		seti(weapon);
		setb(hostile);
		seti(points);
		if (creat_inside != NULL)
			creat_inside[i] = fixsign16(buff[bp], buff[bp + 1]);
		bp += 2; /* Skip # of nouns the creature is carrying */
		seti(counter);
		seti(threshold);
		seti(timethresh);
		seti(timecounter);
		setb(gender);
		if (ver == 3) seti(pict);
		else creature[i].pict = 0;
		creature[i].oclass = 0; /* AGT games don't support classes */
		creature[i].isglobal = 0;
		creature[i].flagnum = 0;
		creature[i].seen = 0;
		creature[i].proper = 0;
	}
	if (DIAG)
		rprintf("   Internal:%ld\n", bp);
	buffclose();
}

#undef seti
#undef setb




/*-------------------------------------------------------------------------*/
/* Read Commands (DA5 and DA6) and convert them to a uniform internal      */
/*   format.                                                               */
/*-------------------------------------------------------------------------*/


static int translate_vnum(int vnum)
/* actor is a numerical index occuring at the beginning of each command.
 In general, it contains the verb number of the verb associated with
   this command; because of AGiliTy's dictionary organization, we don't
   really need this (the verb itself will be converted to a number anyhow),
   but the field contains other useful information as well:
 i)If this command header is really the object of a redirection command,
   then the actor will have 1000 or 2000 added to it, depending on
   AGT version.
 ii)If this command is directed at an actor, then the creature number
   will be in this field instead of the verb number.
     Commands directed to ANYBODY have one plus the maximum verb number
   in this field (a hassle since the maximum verb number depends on
   AGT version: Classic:106, Master's:123); EVERYONE is the next
   code after ANYBODY.
 What this routine does is rationalize the differences between AGT versions.
  --Verb values (ie. not referring to creatures) are converted to 1.
  --Redirections are marked by multiplying by negative one and setting
          cmdsize to 0.
  --ANYBODY is set to 2
  --EVERYBODY is set to 3
*/
{
	rbool redir; /* Is this command redirected? */
	integer anycode;
	int redir_val;

	/* Earlier games use 1000 as redirect value, later games use 2000: */
	/* We strip it off, but remember whether it was there or not so we
	   can restore this information later. */
	redir_val = (aver <= AGT18MAX ? 1000 : 2000);
	if (vnum >= redir_val) {
		vnum = vnum % redir_val;
		redir = 1;
	} else redir = 0;

	anycode = (aver <= AGT18MAX) ? 106 : 123;

	/* Now to correct ANYBODY to something consistent and set verb
	   references to 1 since we don't need them and they just confuse things */
	if (vnum < anycode) vnum = 1; /* "player" */
	else if (vnum == anycode) vnum = 2; /* ANYBODY */
	else if (vnum == anycode + 1) vnum = 3; /* EVERYBODY */

	/* Finally restore redirection info. We now use the sign of vnum
	   to indicate this.*/
	if (redir) vnum = -vnum;

	return vnum;
}


#define CREC_SIZE (FRS_CMD)

static long badtokcnt;

static void read_da5(fc_type fc) {
	long i, j;
	uchar *buff; /* [CREC_SIZE];*/
	long bp;

	if (!have_meta) return;
	if (last_cmd <= 0)
		fatal("Bogus last_cmd");

	buffopen(fc, fDA5, CREC_SIZE, "command", last_cmd);

	if (aver >= AGT15F) cmd_ptr = (long *)rmalloc(sizeof(long) * last_cmd);
	else cmd_ptr = NULL;

	bp = 0;
	for (i = 0; i < last_cmd; i++) {
		buff = buffread(i);
		command[i].actor = translate_vnum(buff[0] + (buff[1] << 8));
		bp = 2;
		command[i].verbcmd = setd(SL_WORD);
		command[i].nouncmd = setd(SL_WORD);
		if (aver >= AGTME10)
			command[i].prep = setd(SL_WORD);
		else command[i].prep = 0;
		command[i].objcmd = setd(SL_WORD);
		command[i].noun_adj = command[i].obj_adj = 0;
		command[i].noun_obj = command[i].obj_obj = 0;
		if (aver < AGT15F) {
			command[i].data = (integer *)rmalloc(MAX_CMD_SIZE * sizeof(integer));
			for (j = 0; j < MAX_CMD_SIZE; j++)
				command[i].data[j] = fixsign16(buff[bp + 2 * j], buff[bp + 2 * j + 1]);
			bp += 2 * MAX_CMD_SIZE;
			command[i].cmdsize = MAX_CMD_SIZE;
		} else {
			cmd_ptr[i] = (long)buff[bp] + (((long)buff[bp + 1]) << 8);
			bp += 2;
		}
	}
	if (DIAG)
		rprintf("     Internal:%ld\n", bp);
	buffclose();

	/* Now to read in DA6 for versions that have it */
	if (aver >= AGT15F) read_da6(fc);
	check_cmd_version();  /* This uses the opcodes to check gamefile
               version information and change it if neccesary. */
	build_cmd_table();   /* Create the command translation table for
             this version of AGT. */

	badtokcnt = 0;
	if (!RAW_CMD_OUT)
		for (i = 0; i < last_cmd; i++)
			fixcmd(command[i].data, command[i].cmdsize);
	rfree(cmd_table);
	if (badtokcnt > MAX_BADTOK)
		agtnwarn("Total number of bad opcodes:", badtokcnt, 1);
}



static void read_da6(fc_type fc)
/* This will only be called for versions with a DA6 file--
   i.e. Master's Edition and proto-ME games. */
{
	genfile fda6;
	char *fullname;
	const char *errstr;
	long fsize; /* Size of the file */
	long frame; /* The first element of the file that is in the buffer. */
	uchar *cbuf;  /* Buffer */
	long cfile_size, cbuf_size; /* Number of tokens in file and in buffer */
	long i, j;
	long cmdstart, cmdend;   /* Marks the start and end of the current command */
	long ip;  /* Points to instruction in cmd.data[] that we are writing to */
	long bp;  /* Pointer into buffer */
	long endp;  /* Used to indicate end of current read loop
           (with an infinite buffer, this would always be an adjusted
           cmdend) */
	long adj_cbuf_size;  /* Stores number of bytes actually read in to cbuf */

	fda6 = openbin(fc, fDA6, "Could not open code file '%s'.", 1);
	fsize = binsize(fda6);
	fullname = formal_name(fc, fDA6);
	if (DIAG) rprintf("Reading code file %s (size:%ld)\n", fullname, fsize);

	if (aver == AGT15F && fsize == 20000) aver = AGT16;
	if (aver >= AGTME10) cfile_size = 20000;
	else if (aver == AGT16) cfile_size = 10000;
	else cfile_size = 5000;

	if (fsize != 2 * cfile_size)
		fatals("Code file %s is the wrong size.", fullname);

	cbuf_size = (cfile_size < CBUF_SIZE) ? cfile_size : CBUF_SIZE;
	cbuf = (uchar *)rmalloc(2 * cbuf_size);
	frame = cfile_size + 1; /* Guarentee frame will be wrong */

	for (i = 0; i < last_cmd; i++)
		if (cmd_ptr[i] >= 2) {
			for (j = i + 1; j < last_cmd && cmd_ptr[j] <= cmd_ptr[i]; j++);
			if (j < last_cmd) cmdend = cmd_ptr[j];
			else cmdend = cfile_size;
			if (cmdend > cfile_size) fatals("Code file overrun(%s)", fullname);
			--cmdend;
			cmdstart = cmd_ptr[i] - 1;
			command[i].cmdsize = cmdend - cmdstart;
			command[i].data = (integer *)rmalloc(command[i].cmdsize * sizeof(integer));

			ip = 0;
			bp = cmdstart - frame;
			adj_cbuf_size = cbuf_size;

			while (ip < command[i].cmdsize) {
				if (bp < 0 || bp >= adj_cbuf_size) { /* Read in new block */
					frame = frame + bp;
					binseek(fda6, frame * 2);
					if (frame + cbuf_size <= cfile_size)
						adj_cbuf_size = cbuf_size;
					else
						adj_cbuf_size = cfile_size - frame;
					if (adj_cbuf_size <= 0) fatal("Unexpected end of file.");
					if (!binread(fda6, cbuf, 2, adj_cbuf_size, &errstr))
						fatal(errstr);
					bp = 0;
				}
				endp = cmdend - frame;
				if (endp > cbuf_size) endp = cbuf_size;
				for (; bp < endp; ip++, bp++)
					command[i].data[ip] = fixsign16(cbuf[bp * 2L], cbuf[bp * 2L + 1]);
			}
		} else {
			command[i].data = NULL;
			command[i].cmdsize = 0;
		}
	rfree(cbuf);
	readclose(fda6);
	rfree(fullname);
}





static int check_endcmd(void)
/* What is the most common last byte for metacommands? Except
 under very abnormal situations, this should be the EndCmd opcode */
{
	int count[MAX_TOKEN_ID + 1];
	int i, tok, maxcnt, maxtok;
	/* int nextcnt; */

	for (i = 0; i <= MAX_TOKEN_ID; i++) count[i] = 0;
	for (i = 0; i < last_cmd; i++)
		if (command[i].cmdsize > 0) {
			tok = command[i].data[command[i].cmdsize - 1];
			if (tok >= 0 && tok <= MAX_TOKEN_ID) count[tok]++;
		}
	maxcnt = maxtok = 0; /* nextcnt=0;*/
	for (i = 0; i <= MAX_TOKEN_ID; i++)
		if (count[i] >= maxcnt) {
			/*      nextcnt=maxcnt; */
			maxcnt = count[i];
			maxtok = i;
		}
	return maxtok;
}

static int compute_endcode(int ver_)
/* Computes the correct endcode for a given gamefile version  */
{
	int i;

	for (i = 0; FIX_LIST[ver_][i].tnew != -1; i++);
	return (FIX_LIST[ver_][i].told - 3); /* -3 to get to EndCmd */
}


static void check_cmd_version(void)
/* Run through the commands looking at the last legal byte. This is
   normally the EndCmd token code, which can give us info on which
   token encoding scheme is being used. */
{
	int endcode;

	endcode = check_endcmd();
	if (DIAG) rprintf("  (EndCmd=%d)\n", endcode);
	if (endcode < 150) return; /* No metacommands, or something else is wrong. */
	if (endcode == compute_endcode(aver)) return; /* We're okay */

	/* Check for the special cases we know about */
	if (aver == AGT135) {
		if (endcode == compute_endcode(AGT182)) {
			aver = AGT182;
			return;
		} else if (endcode == compute_endcode(AGT118)) {
			aver = AGT118;
			return;
		}
	}
	if (aver == AGTME10)
		if (endcode == compute_endcode(AGTME10A)) {
			aver = AGTME10A;
			return;
		}
	if (aver == AGTMAST)
		if (endcode == compute_endcode(AGTME155)) {
			aver = AGTME155;
			return;
		}

	/* If we still haven't fixed the problem, print out a warning and
	 pray. */
	agtnwarn("Game has invalid EndCmd: ", endcode, 1);
}



static void build_cmd_table(void) {
	int told, tnew, fp;
	const cmd_fix_rec *fixtbl;

	topcmd = compute_endcode(aver) + 3;
	cmd_table = (short *)rmalloc(sizeof(short) * topcmd);

	fixtbl = FIX_LIST[aver];
	fp = 0; /* Pointer into fix table */
	tnew = 0; /* This shouldn't be neccessary */
	for (told = 0; told < topcmd;) {
		if (told == fixtbl[fp].told) tnew = fixtbl[fp++].tnew;
		cmd_table[told++] = tnew++;
	}
}



static void badtokerr(const char *s, int tok) {
	if (++badtokcnt <= MAX_BADTOK) agtnwarn(s, tok, 1);
}

static void fixcmd(integer *clist, int cnt)
/* Okay, we need to go through the elements of clist (which is an array,
  actually), figure out which ones are commands (as opposed to arguments)
  and tweak them to hide version differences. */
{
	long ip;

	/* Now need to go through and adjust opcodes. */
	for (ip = 0; ip < cnt; ip++)
		if (clist[ip] >= topcmd || clist[ip] < 0)
			badtokerr("Invalid token found: ", clist[ip]);
		else {

			clist[ip] = cmd_table[clist[ip]]; /* Translate */

			/* Now increment ip by the length of the instruction */
			/* Remember that we are already incrementing by one automatically */

			if (clist[ip] >= END_ACT) break; /* CMD end marker */
			if (clist[ip] <= MAX_COND)
				ip += cond_def[clist[ip]].argnum;
			else if (clist[ip] < WIN_ACT) {
				if (clist[ip] == 1087 && ip + 1 < cnt) /* AskQuestion: Adjust top_quest */
					if (top_quest < clist[ip + 1]) top_quest = clist[ip + 1];
				ip += act_def[clist[ip] - START_ACT].argnum;
			}
			/* else do nothing */
		}
}





/*-------------------------------------------------------------------------*/
/* DA1 Reading Utilites: routines to read the various lines of the DA1 file */
/*-------------------------------------------------------------------------*/

static void chop_newline(char *s)
/* Remove trailing \r,\n, etc. characters */
{
	char *t;

	for (t = s; *t != 0; t++); /* Find the end of the string */
	for (; t >= s && (*t == 0 || *t == '\r' || *t == '\n'); t--);
	*(t + 1) = 0;
}

static void fix_answer(char *s)
/* Put answer s into standard format: all lower case and with trailing/
   following whitespace removed */
{
	char *t, *p;

	for (t = s; *t != 0; t++)
		*t = tolower(*t);
	/* Eliminate trailing space and newlines */
	for (; t >= s && (*t == 0 || rspace(*t)); t--);
	*(t + 1) = 0;
	/* Eliminate leading space and newlines */
	for (t = s; rspace(*t); t++);
	if (t != s) {
		for (p = s; *t != 0;)
			*(p++) = *(t++);
		*p = 0;
	}
}


static char linebuffer[81];
static int bhold;
static int linenum;
static rbool unexpected_eof;

static void read_line(genfile fd, const char *typestr)
/* Read a line into buffer, unless bhold=1 in which case we want
   to use the last line read */
{
	if (bhold == 0) {
		readln(fd, linebuffer, 80);
		if (linebuffer[0] == 0 && texteof(fd)) {
			unexpected_eof = 1;
			strcpy(linebuffer, ">End Of File<");
		} else chop_newline(linebuffer);
		linenum++;
	}
	if (debug_da1 && typestr != NULL) {
		rprintf("%s %4d:%s", typestr, linenum, linebuffer);
		if (bhold) rprintf("     *");
		writeln("");
	}
	bhold = 0;
}


static void report(const char *s, genfile fd) {
	if (DIAG) rprintf("REPORT:%s at %d\n", s, linenum);
}

static int isbool(genfile fd) {
	read_line(fd, NULL);
	bhold = 1;
	return (strncasecmp(linebuffer, "TRUE", 4) == 0 ||
	        strncasecmp(linebuffer, "FALSE", 5) == 0);
}

static int isnum(genfile fd) {
	char *errstr;

	read_line(fd, NULL);
	bhold = 1;
	(void)strtol(linebuffer, &errstr, 10);
	while (*errstr == '\n' || *errstr == '\r') errstr++;
	if (debug_da1)
		rprintf("NUMCHK: %s==>%c\n", linebuffer, *errstr);
	return (*errstr == '\0');
}

static rbool readrbool(genfile fd) {
	read_line(fd, "BOOL");
	return (strncasecmp(linebuffer, "TRUE", 4) == 0);
}


static long readnum(genfile fd) {
	read_line(fd, "NUM ");
	return strtol(linebuffer, NULL, 10);
}


static void readptr(genfile fd, descr_ptr *desc) {
	read_line(fd, "PTR ");
	desc->start = strtol(linebuffer, NULL, 10);
	read_line(fd, "LEN");
	desc->size = strtol(linebuffer, NULL, 10);
}


static void readjunk(genfile fd) {
	read_line(fd, "JUNK");
}

static void readtext(genfile fd, tline s) {
	read_line(fd, "TEXT");
	strncpy((char *)s, linebuffer, 80);
	s[80] = 0;
}

static long readfname(genfile fd) {
	read_line(fd, "FILE");
	return new_str(linebuffer, 0, 0);
	/* Copy filename string to static string space and return index */
}

static word readdict(genfile fd) {
	read_line(fd, "DICT");
	return add_dict(linebuffer);
}


static slist readslist(genfile fd) { /* Read in synonym list line */
	slist start_ptr;
	char nbuff[50];
	int j, k;

	start_ptr = synptr;
	read_line(fd, "SYN ");
	/* Need to see if it is none or * terminated.  */
	for (j = 0; linebuffer[j] != 0 && linebuffer[j] != '*'; j++);
	linebuffer[j] = 0;
	k = 0;
	for (j = 0; linebuffer[j] != 0; j++)
		if (rspace(linebuffer[j]) && k > 0) {
			nbuff[k] = 0;
			addsyn(add_dict(nbuff));
			k = 0;
		} else if (!rspace(linebuffer[j]))
			nbuff[k++] = linebuffer[j];
	if (k > 0) {
		nbuff[k] = 0;
		addsyn(add_dict(nbuff));
	}
	addsyn(-1);
	return start_ptr;
}



/*-------------------------------------------------------------------------*/
/* Version analysis: Utilities to analyse the file format version and      */
/*   deduce sizes.                                                         */
/*-------------------------------------------------------------------------*/

static int soggy_test(fc_type fc) {
	genfile fda3;
	long fsize;

	if (DIAG) {
		char *s;
		s = formal_name(fc, fDA3);
		rprintf("Testing %s for abnormal noun organization....", s);
		rfree(s);
	}
	fda3 = openbin(fc, fDA3, "Could not find room file '%s'.", 1);
	fsize = binsize(fda3);
	readclose(fda3);

	if (fsize % (maxnoun - 300 + 1) != 0) {
		if (DIAG) rprintf("FOUND!\n");
		return 1;
	}
	if (fsize / (maxnoun - 300 + 1) > 300) {
		if (DIAG) rprintf("FOUND!\n");
		return 1;
	}
	if (DIAG) rprintf("nope.\n");
	return 0;
}


static void deduce_sizes(fc_type fc, rbool diag)
/* If diag is true, we will also allocate space for
noun inside information; this is used by agtout */
{
	if (ver == 0) {
		ver = 1;
		if (maxroom >= 200) ver = 2;
		else if (maxnoun != 0)
			if (maxnoun < 300)
				if (maxcreat != 0)
					if (maxcreat >= 500) ver = 4; /* SOGGY */
					else ver = 1;  /* Small */
				else if (aver == AGTCOS) ver = 4; /* SOGGY */
				else ver = 1; /* Small */
			else if (aver != AGTCOS) ver = 2; /* Large */
			else if (soggy_test(fc)) ver = 4;
			else ver = 2;
		else if (maxcreat != 0)
			if (maxcreat >= 500)
				if (aver != AGTCOS) ver = 2; /* Large */
				else if (soggy_test(fc)) ver = 4; /* Either large or SOGGY */
				else ver = 2;
			else ver = 1; /* Small */
		else
			agtwarn("No nouns or creatures: unable to determine version."
			        "\nAssuming AGT Small", 0);
	}

	if (aver < AGTME15)
		MaxQuestion = 25;
	else
		MaxQuestion = 100; /* This is a guess. */
	if (aver == AGTCOS)
		MaxQuestion = 10;
	if (aver == AGT15 || aver == AGT15F)
		MaxQuestion = 57;
	first_room = 2;
	if (ver == 1) {
		first_noun = 200;
		first_creat = 300;
		last_obj = 399;
		last_message = 250;
	} else { /* ver 2 or 3 or 4 */
		if (ver != 4)
			first_noun = 300;
		else first_noun = 200;
		first_creat = 500;
		last_obj = 699;
		if (aver <= AGT12) last_message = 500;
		else if (aver < AGTME155) last_message = 600;
		else last_message = 800;
	}
	if (aver == AGTCOS) {
		if (ver == 4) last_obj = 610;
		else last_obj = 599;
		if (ver == 4) last_message = 810; /* Soggy case */
		else last_message = 700;
	}

	if (aver >= AGT18 && aver <= AGT18MAX) {
		bold_mode = 1;
		build_fixchar();
		fixchar[(int)'\\'] = FORMAT_CODE;
	}

	if (aver < AGTME10) {
		SL_TEXT = 81;
		SL_NAME = SL_WORD = 23;
		SL_ROOM = 31;
	} else {
		SL_TEXT = 81;
		SL_NAME = SL_WORD = 16;
		SL_ROOM = 31;
	}
	if (aver == AGT15 || aver == AGT15F) SL_NAME = SL_WORD = 16;

	if (aver >= AGTME10) {
		MAX_USTR = 25;
		MAX_SUB = 15;
	} else MAX_SUB = MAX_USTR = 0;

	if (aver >= AGT15)
		NUM_ERR = 185; /* Number of standard error messages */
	else
		NUM_ERR = 0;

	DVERB = 50;
	FLAG_NUM = 255;
	CNT_NUM = VAR_NUM = 50;
	exitmsg_base = 1000;

	num_rflags = num_nflags = num_cflags = 0;
	num_rprops = num_nprops = num_cprops = 0;
	objflag = NULL;
	objprop = NULL;
	attrtable = NULL;
	proptable = NULL;
	oflag_cnt = 0;
	oprop_cnt = 0;
	propstr = NULL;
	propstr_size = 0;
	vartable = NULL;
	flagtable = NULL;



	/* Now to allocate space for all of the 'immortal' data structures */
	/* We do this all at once to avoid fragmentation; all of the following
	   will be around for the life of the program (unless we restart) and so
	   should be allocated first */

	synlist = (slist *)rmalloc(sizeof(slist) * TOTAL_VERB);
	comblist = NULL; /* The original AGT didn't support multi-word verbs */
	num_comb = 0;
	userprep = NULL; /* ... nor did it allow user-defined prepostions */
	num_prep = 0;

	if (numglobal > 0)
		globalnoun = (word *)rmalloc(numglobal * sizeof(word));

	if (aver < AGTME15 && aver != AGT10) {
		question = (tline *)rmalloc(MaxQuestion * sizeof(tline));
		answer = (tline *)rmalloc(MaxQuestion * sizeof(tline));
	} else if (aver >= AGTME15) {
		quest_ptr = (descr_ptr *)rmalloc(MaxQuestion * sizeof(descr_ptr));
		ans_ptr = (descr_ptr *)rmalloc(MaxQuestion * sizeof(descr_ptr));
	}
	msg_ptr = (descr_ptr *)rmalloc((last_message) * sizeof(descr_ptr));

	if (maxroom >= first_room) {
		room = (room_rec *)rmalloc((maxroom - first_room + 1) * sizeof(room_rec));
		room_ptr = (descr_ptr *)rmalloc((maxroom - first_room + 1) * sizeof(descr_ptr));
		help_ptr = (descr_ptr *)rmalloc((maxroom - first_room + 1) * sizeof(descr_ptr));
		special_ptr = (descr_ptr *)rmalloc((maxroom - first_room + 1) * sizeof(descr_ptr));
		if (diag) room_inside = (integer *)rmalloc((maxroom - first_room + 1) * sizeof(integer));
	}

	if (maxnoun >= first_noun) {
		noun = (noun_rec *)rmalloc((maxnoun - first_noun + 1) * sizeof(noun_rec));
		noun_ptr = (descr_ptr *)rmalloc((maxnoun - first_noun + 1) * sizeof(descr_ptr));
		push_ptr = (descr_ptr *)rmalloc((maxnoun - first_noun + 1) * sizeof(descr_ptr));
		pull_ptr = (descr_ptr *)rmalloc((maxnoun - first_noun + 1) * sizeof(descr_ptr));
		text_ptr = (descr_ptr *)rmalloc((maxnoun - first_noun + 1) * sizeof(descr_ptr));
		turn_ptr = (descr_ptr *)rmalloc((maxnoun - first_noun + 1) * sizeof(descr_ptr));
		play_ptr = (descr_ptr *)rmalloc((maxnoun - first_noun + 1) * sizeof(descr_ptr));
		if (diag) noun_inside = (integer *)rmalloc((maxnoun - first_noun + 1) * sizeof(integer));
	}

	if (maxcreat >= first_creat) {
		creature = (creat_rec *)rmalloc((maxcreat - first_creat + 1) * sizeof(creat_rec));
		creat_ptr = (descr_ptr *)rmalloc((maxcreat - first_creat + 1) * sizeof(descr_ptr));
		ask_ptr = (descr_ptr *)rmalloc((maxcreat - first_creat + 1) * sizeof(descr_ptr));
		talk_ptr = (descr_ptr *)rmalloc((maxcreat - first_creat + 1) * sizeof(descr_ptr));
		if (diag) creat_inside = (integer *)rmalloc((maxcreat - first_creat + 1) * sizeof(integer));
	}

	if (aver >= AGTME10) {
		userstr = (tline *)rmalloc(MAX_USTR * sizeof(tline));
		sub_name = (word *)rmalloc(MAX_SUB * sizeof(word));
	}
	command = (cmd_rec *)rmalloc(sizeof(cmd_rec) * last_cmd);

	if (aver >= AGT15)
		err_ptr = (descr_ptr *)rmalloc(NUM_ERR * sizeof(descr_ptr));


	reinit_dict(); /* The dictionary grows dynamically so we want to
            allocate it AFTER we have allocated all the permenent
            things */
}




/*-------------------------------------------------------------------------*/
/* Read DA1: The Info file; this is a text file containing a miscellany of */
/*   game information that wouldn't fit elsewhere                          */
/*-------------------------------------------------------------------------*/

static int try_read_da1(fc_type fc, genfile fda1, rbool diag)
/* Returns new aver value to try, or 0 on success. */
/* diag determines if noun inside info will be read */
/* VER values: 1=Small
               2=Large
           3=Master's Edition
           4="Soggy Large", with a larger last_room
*/
/* AVER values: see agility.h for the current values */
/* NOTE:  This routine is allowed to set *ver*, but is not allowed to
   change *aver*; should it be neccessary to change *aver*, then the routine
   should return the new *aver* value.
  (The only exception to this is in the very beginning-- and that may get
     changed)
  [This is done to allow the user to force a version number]
*/
{
	int i;

	MAX_CMD_SIZE = 30;
	maxpict = maxpix = maxfont = maxsong = 0;
	linenum = 0;
	bhold = 0;
	game_sig = 0;
	unexpected_eof = 0;

	if (aver == 0 && isbool(fda1)) aver = AGTMAST;
	/* From this point on can assume ME detected */

	freeze_mode = 0; /* The default values */
	if (aver >= AGTME10) { /* 2 rbool */
		debug_mode = readrbool(fda1); /* DEBUG */
		if (aver >= AGTME15) {
			if (!isbool(fda1)) aver = AGTME10;
			else freeze_mode = readrbool(fda1);
		} /* FREEZE */
		ver = 3;
	}

	start_room = readnum(fda1);
	treas_room = readnum(fda1);
	if (aver != AGT10) resurrect_room = readnum(fda1);
	else resurrect_room = start_room;
	if (aver >= AGTME10) { /* 4 int */
		score_mode = readnum(fda1); /* Score option */
		statusmode = readnum(fda1); /* Status option */
		startup_time = readnum(fda1); /* Starting time */
		delta_time = readnum(fda1); /* Delta_time */
	} else {
		score_mode = statusmode = 0;
		startup_time = delta_time = 0;
	}
	max_lives = readnum(fda1);
	if (aver != AGT10) max_score = readnum(fda1);
	else max_score = 0;
	maxroom = readnum(fda1);
	maxnoun = readnum(fda1);
	maxcreat = readnum(fda1);
	if (aver >= AGTME10) numglobal = readnum(fda1); /* # of global nouns? */
	else numglobal = 0;
	last_cmd = readnum(fda1);
	readjunk(fda1);  /* Number of items being carried */
	readjunk(fda1);  /* Number of items being worn */
	if (isbool(fda1)) return AGT10; /* AGT v1.0 */
	/* From this point on, can assume AGT v1.0 is detected. */
	readptr(fda1, &intro_ptr);

	deduce_sizes(fc, diag);

	if (aver >= AGTME10)  {

		(void)readdict(fda1);  /* ?!?! Not sure what this is */

		report("Reading global and flag nouns", fda1);

		for (i = 0; i < MAX_FLAG_NOUN; i++)
			flag_noun[i] = readdict(fda1); /* Read in flag nouns; may be NONE */
		for (; i < 32; i++)
			readjunk(fda1);

		for (i = 0; i < numglobal; i++)
			globalnoun[i] = readdict(fda1); /* Global nouns */
	} else
		for (i = 0; i < MAX_FLAG_NOUN; i++)
			flag_noun[i] = 0;

	report("Reading questions and junk", fda1);

	if (aver < AGTME15 && aver != AGT10) {
		for (i = 0; i < MaxQuestion; i++) {
			readtext(fda1, question[i]); /* Question[i]== question #(i+1) */
			chop_newline(question[i]);
			readtext(fda1, answer[i]);
			fix_answer(answer[i]);
		}
	} else if (aver >= AGTME15) {
		/* There are 400 lines of description pointers, meaning
		   200 descriptions. I'm guessing they're all questions and
		   answers, which means that there are 100 questions here. */
		for (i = 0; i < MaxQuestion; i++) {
			readptr(fda1, &quest_ptr[i]);
			readptr(fda1, &ans_ptr[i]);
		}
	}

	if (!isbool(fda1)) {  /* Something is wrong... */
		if (aver == AGTMAST)
			return AGTME10;
		else if (aver != AGTCOS && aver != AGT15 && aver != AGT15F) return AGTCOS;
		else return AGT15;
	}
	report("Reading have_meta", fda1);
	have_meta = readrbool(fda1);

	if (have_meta) {
		for (i = 0; i <= last_obj; i++) { /* i.e. iterate over all objects */
			readjunk(fda1);
			readjunk(fda1);
		}
	}

	/* The Master's Edition apparently _always_ sets have_meta,
	   even if there are no metacommands. The only way to determine
	   if there are really metacommands is to check last_cmd */
	if (aver >= AGTME10 && last_cmd == 0) have_meta = 0;

	report("Reading synonyms", fda1);

	for (i = 0; i < TOTAL_VERB; i++)
		synlist[i] = synptr; /* Is this correct? */
	addsyn(-1); /* Put an end-of-list marker in place */

	for (i = 0; i < 56; i++)
		synlist[i] = readslist(fda1); /* May read <none> */

	if (aver >= AGTME10) { /* Unknown verbs */
		synlist[56] = readslist(fda1); /* VIEW */
		synlist[57] = synlist[14];    /* AFTER */
		synlist[14] = readslist(fda1); /* THROW */
	}

	if (aver == AGT183) {
		/* Eliminate LIST_EXITS and add INSTRUCTIONS */
		synlist[58] = synlist[52];
		/* Move 'REMOVE'-- the last thing before INS in 1.83 verblist --
		up to INSTRUCTIONS where it belongs */
		for (i = 52; i > 42; i--) /* i:=Remove to Brief (above List Exits) */
			synlist[i] = synlist[i - 1];
		synlist[41] = synptr;  /* LIST_EXITS, which doesn't exist in 1.83 and so
               can't have synonyms */
		addsyn(-1);
	}

	report("Starting dummy verbs", fda1);
	for (i = 0; i < 25; i++) {
		if (i != 0 || aver < AGTME10)
			synlist[i + BASE_VERB] = readslist(fda1);
		synlist[i + BASE_VERB + 25] = readslist(fda1);
	}
	if (aver >= AGTME10) {
		synlist[BASE_VERB] = readslist(fda1);
		for (i = 0; i < 15; i++) /* Subroutines */
			synlist[i + BASE_VERB + 50] = readslist(fda1);
	}
	report("Reading DESC ptrs", fda1);
	if (aver >= AGT15)
		for (i = 0; i < NUM_ERR; i++)
			readptr(fda1, &err_ptr[i]); /* Read in "standard" error messages. */
	else /* Otherwise need to initialize them to nothing */
		for (i = 0; i < NUM_ERR; i++) {
			err_ptr[i].start = 0;
			err_ptr[i].size = -1;
		}

	report("Reading messages", fda1);
	if (DIAG) rprintf("  MSGS:1..%ld [%ld]\n", last_message, last_message);
	for (i = 0; i < last_message; i++)
		readptr(fda1, &msg_ptr[i]);

	report("Reading room descs", fda1);
	for (i = 0; i <= maxroom - first_room; i++) {
		readptr(fda1, &room_ptr[i]);
		readptr(fda1, &help_ptr[i]);
		readptr(fda1, &special_ptr[i]);
	}

	report("Reading noun descs", fda1);
	for (i = 0; i <= maxnoun - first_noun; i++) {
		readptr(fda1, &noun_ptr[i]);
		readptr(fda1, &push_ptr[i]);
		readptr(fda1, &pull_ptr[i]);
		readptr(fda1, &text_ptr[i]);
		readptr(fda1, &turn_ptr[i]);
		readptr(fda1, &play_ptr[i]);
	}

	report("Reading creatures", fda1);
	if (maxcreat >= first_creat) {
		for (i = 0; i <= maxcreat - first_creat; i++) {
			readptr(fda1, &creat_ptr[i]);
			if (aver != 0 && aver <= AGTCLASS) {
				ask_ptr[i].start = talk_ptr[i].start = 0;
				ask_ptr[i].size = talk_ptr[i].size = -1;
			} else {
				readptr(fda1, &talk_ptr[i]);
				readptr(fda1, &ask_ptr[i]);
				if (aver == 0 && (talk_ptr[i].size == 0 || ask_ptr[i].size == 0 ||
				                  unexpected_eof)) return AGT135;
			}
		}
	}
	if (aver == AGT135 && unexpected_eof) return AGT12;

	if (aver >= AGTME10) {
		if (aver >= AGTME155 && !isnum(fda1)) return AGTME15;

		maxpict = rangefix(readnum(fda1)); /* Number of pictures */
		maxpix = rangefix(readnum(fda1)); /* Numper of PIXs */
		maxsong = rangefix(readnum(fda1)); /* Number of sounds */
		maxfont = rangefix(readnum(fda1)); /* Number of fonts. */

		if (maxpix > MAX_PIX) {
			rprintf("Invalid MAXPIX value?!?\n");
			maxpix = MAX_PIX;
		}

		t_pictlist = (long *)rmalloc(sizeof(long) * maxpict);
		t_pixlist = (long *)rmalloc(sizeof(long) * maxpix);
		t_songlist = (long *)rmalloc(sizeof(long) * maxsong);
		t_fontlist = (long *)rmalloc(sizeof(long) * maxfont);

		for (i = 0; i < maxpict; i++)
			t_pictlist[i] = readfname(fda1); /* picture file names */
		for (i = 0; i < maxpix; i++)
			pix_name[i] = readdict(fda1); /* PIX names */
		for (i = 0; i < maxpix; i++)
			t_pixlist[i] = readfname(fda1); /* PIX filenames */
		for (i = 0; i < maxsong; i++)
			t_songlist[i] = readfname(fda1); /* Sound filenames */
		for (i = 0; i < maxfont; i++)
			t_fontlist[i] = readfname(fda1); /* Font filenames */

		for (i = 0; i < MAX_USTR; i++)
			readtext(fda1, userstr[i]); /* This is just a guess-- should be
                 tested. */
	} else {
		for (i = 0; i < maxpix; i++) pix_name[i] = 0;
		maxpict = maxpix = maxsong = maxfont = 0;
	}
	if ((aver == AGT135 || aver == 0) && isnum(fda1)) return AGT183;
	if (aver == AGT183) {
		long tval;
		tval = readnum(fda1); /* Needs to be translated */
		if (tval >= 1 && tval <= 4)
			statusmode = agt18_statmode[tval - 1];
		else statusmode = 0;
		tval = readnum(fda1);     /* Hours */
		startup_time = readnum(fda1); /* Minutes */
		tval += startup_time / 60;
		startup_time = (startup_time % 60) + 100 * tval;
		if (readrbool(fda1) && startup_time < 1200)
			startup_time += 1200;
		milltime_mode = readrbool(fda1); /* Military time */
		delta_time = readnum(fda1);
	}
	if (DIAG) rprintf("Read in  %d lines\n", linenum);
	return 0;
}



static void set_da1_null(void)
/* Set pointers that are malloc'd by try_read_da1 to NULL, to clear
 the way for free_da1_stuff to recover them */
{
	static_str = NULL;
	ss_end = ss_size = 0;
	command = NULL;
	cmd_ptr = NULL;
	synlist = NULL;
	userstr = NULL;
	sub_name = NULL;
	globalnoun = NULL;
	err_ptr = NULL;
	quest_ptr = ans_ptr = NULL;
	question = answer = NULL;
	msg_ptr = room_ptr = help_ptr = special_ptr = NULL;
	noun_ptr = push_ptr = pull_ptr = text_ptr = turn_ptr = play_ptr = NULL;
	room_inside = noun_inside = creat_inside = NULL;
	creat_ptr = ask_ptr = talk_ptr = NULL;
	pictlist = pixlist = fontlist = songlist = NULL;
	room = NULL;
	noun = NULL;
	creature = NULL;
	command = NULL;
	t_pictlist = t_pixlist = t_fontlist = t_songlist = NULL;
}



static void free_da1_stuff(void)
/* Free all data structures malloc'd by try_read_da1 */
/* (This is neccessary since try_read_da1 may have to restart) */
/* Note that if a pointer is NULL, rfree does nothing */
/* Recall that rfree() is a macro that sets its argument to NULL */
/* after freeing it */
{
	rfree(static_str);
	ss_end = ss_size = 0;
	rfree(userstr);
	rfree(sub_name);
	rfree(globalnoun);
	rfree(err_ptr);
	rfree(synlist);
	rfree(quest_ptr);
	rfree(ans_ptr);
	rfree(question);
	rfree(answer);
	rfree(msg_ptr);
	rfree(room_ptr);
	rfree(help_ptr);
	rfree(special_ptr);
	rfree(noun_ptr);
	rfree(push_ptr);
	rfree(pull_ptr);
	rfree(text_ptr);
	rfree(room_inside);
	rfree(noun_inside);
	rfree(creat_inside);
	rfree(turn_ptr);
	rfree(play_ptr);
	rfree(creat_ptr);
	rfree(ask_ptr);
	rfree(talk_ptr);
	rfree(t_pictlist);
	rfree(t_pixlist);
	rfree(t_songlist);
	rfree(t_fontlist);
	rfree(room);
	rfree(noun);
	rfree(creature);
	rfree(command);
	free_dict();
}

static rbool read_da1(fc_type fc, rbool diag)
/* diag is set by agtout to save extra diagnostic information */
/* It has nothing to do with DIAG */
{
	genfile fda1;
	int i;

	ver = 0;
	aver = 0;
	top_quest = 0; /* Highest question actually referenced; set by fixcmd */
	fda1 = openfile(fc, fDA1, NULL, 0);
	if (!filevalid(fda1, fDA1)) return 0;

	if (DIAG) {
		char *s;
		s = formal_name(fc, fDA1);
		rprintf("Reading info file %s\n", s);
		rfree(s);
	}
	set_da1_null();
	while ((i = try_read_da1(fc, fda1, diag)) != 0) {
		if (aver == i) {
			rprintf("[Recoginiton loop: AVER=%d]\n", aver);
			fatal("AGT version not recognized\n");
		}
		aver = i;
		/* fseek(fda1,0,SEEK_SET);  Go back to beginning... */
		textrewind(fda1);
		if (DIAG)
			rprintf("...Found incompatibility; restarting, w/ AVER=%d\n", aver);
		free_da1_stuff();
		/* set_da1_null();*/
		ver = 0;
	}
	if (aver == 0) aver = AGTSTD; /* i.e. if we didn't notice any differences from
              standard format, it must be a standard file. */
	readclose(fda1);
	return 1; /* Success */
}



/*-------------------------------------------------------------------------*/
/* Miscellaneous routines to tie up loose ends and clean up afterwards.    */
/*-------------------------------------------------------------------------*/

static void finish_read(rbool cleanup)
/* cleanup=0 means it will leave cmd_ptr, 1=it cleans up cmd_ptr */
/*  The only reason to set cleanup==0 is if we are writing a diagnostic
    program of some sort */
{
	int i;

	if (aver >= AGT18 && aver <= AGT18MAX) {
		intro_first = 1;
		max_lives = 1;
		TWO_CYCLE = 1;
		PURE_AFTER = 0;
	} else {
		intro_first = 0;
		TWO_CYCLE = 0;
		PURE_AFTER = 1;
	}

	min_ver = 0;  /* All original AGT games will run with any version of
         AGiliTy. */

	if (aver >= AGTME10)
		PURE_ROOMTITLE = 0;

	if (aver >= AGT15)
		box_title = 1;
	else box_title = 0;

	/* Compute max_score if it isn't already computed */
	if (max_score == 0) {
		for (i = 0; i < maxroom - first_room + 1; i++)
			if (!room[i].unused) max_score += room[i].points;
		for (i = 0; i < maxnoun - first_noun + 1; i++)
			if (!noun[i].unused) max_score += noun[i].points;
		for (i = 0; i < maxcreat - first_creat + 1; i++)
			if (!creature[i].unused) max_score += creature[i].points;
	}

	if (cleanup) rfree(cmd_ptr);
	if (ss_end > 0)
		static_str = (char *)rrealloc(static_str, sizeof(char) * ss_end);

	/* Now convert string handles into honest pointers */
	for (i = 0; i <= maxroom - first_room; i++)
		room[i].name = static_str + room_name[i];
	for (i = 0; i <= maxnoun - first_noun; i++) {
		noun[i].shortdesc = static_str + noun_sdesc[i];
		noun[i].position = static_str + noun_pos[i];
	}
	for (i = 0; i <= maxcreat - first_creat; i++)
		creature[i].shortdesc = static_str + creat_sdesc[i];

	if (aver >= AGTME10) {
		pictlist = (filename *)rmalloc(sizeof(filename) * maxpict);
		pixlist = (filename *)rmalloc(sizeof(filename) * maxpix);
		songlist = (filename *)rmalloc(sizeof(filename) * maxsong);
		fontlist = (filename *)rmalloc(sizeof(filename) * maxfont);

		for (i = 0; i < maxpict; i++)
			pictlist[i] = static_str + t_pictlist[i];
		for (i = 0; i < maxpix; i++)
			pixlist[i] = static_str + t_pixlist[i];
		for (i = 0; i < maxsong; i++)
			songlist[i] = static_str + t_songlist[i];
		for (i = 0; i < maxfont; i++)
			fontlist[i] = static_str + t_fontlist[i];
	}

	/* Free the various temporary arrays */
	rfree(room_name);
	rfree(noun_sdesc);
	rfree(noun_pos);
	rfree(creat_sdesc);
	rfree(t_pictlist);
	rfree(t_pixlist);
	rfree(t_songlist);
	rfree(t_fontlist);

	/* Reallocate questions and asnwers to only use the space that they need */
	if (!RAW_CMD_OUT && top_quest < MaxQuestion) {
		MaxQuestion = top_quest; /* top_quest is computed by fixcmd */
		if (top_quest == 0) {
			rfree(question);
			rfree(answer);
			rfree(quest_ptr);
			rfree(ans_ptr);
		} else {
			if (question != NULL)
				question = (tline *)rrealloc(question, top_quest * sizeof(tline));
			if (answer != NULL)
				answer = (tline *)rrealloc(answer, top_quest * sizeof(tline));
			if (quest_ptr != NULL)
				quest_ptr = (descr_ptr *)rrealloc(quest_ptr, top_quest * sizeof(descr_ptr));
			if (ans_ptr != NULL)
				ans_ptr = (descr_ptr *)rrealloc(ans_ptr, top_quest * sizeof(descr_ptr));
		}
	}
}

void free_all_agtread() {
	int i;

	if (!agx_file)
		for (i = 0; i < last_cmd; i++)
			rfree(command[i].data);
	free_da1_stuff();
	/* userstr, globalnoun, quest_ptr, ans_ptr, question, answer, msg_ptr,
	   room_ptr, help_ptr, special_ptr, noun_ptr, push_ptr, pull_ptr,
	   text_ptr, turn_ptr, play_ptr, creat_ptr, ask_ptr, talk_ptr,
	   room_inside, noun_inside, creat_inside
	   pictlist, pixlist, songlist, fontlist,
	   room, noun, creature, command,
	   dictionary data structures */
}

rbool readagt(fc_type fc, rbool diag)
/* If diag==1, then extra diagnostic information is preserved */
{
	agx_file = 0;
	mem_descr = NULL;
	build_fixchar();
	init_dict();
	if (!read_da1(fc, diag)) return 0; /* Couldn't open DA1 file */
	read_da2(fc);
	read_da3(fc);
	read_da4(fc);
	read_da5(fc);
	read_voc(fc);
	read_opt(fc);
	finish_read(!diag);
	return 1;
}

} // End of namespace AGT
} // End of namespace Glk
