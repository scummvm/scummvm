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
#include "glk/agt/exec.h"

namespace Glk {
namespace AGT {

#define SAVE_UNDO
#define DEBUG_SAVE_SIZE 0

long state_size;


/*-------------------------------------------------------------------*/
/*  INITIALISATION ROUTINES  */
/* These initialize all of the values that can be derived from */
/*   other data in the game file or that are reset when a game */
/*   is restored */
/* See parser.c for the interpreter's main initialisation routines */

void init_vals(void)
/* Compute quantities that can be deduced from existing data */
{
	int i;

	quitflag = winflag = deadflag = endflag = 0;
	cmd_saveable = 0;
	last_he = last_she = last_it = 0;
	totwt = totsize = 0;
	for (i = 0; i <= maxroom - first_room; i++)
		room[i].contents = 0;
	player_contents = player_worn = 0;
	for (i = 0; i <= maxnoun - first_noun; i++) {
		if (player_has(i + first_noun)) totwt += noun[i].weight;
		if (noun[i].location == 1) totsize += noun[i].size;
		noun[i].something_pos_near_noun = 0;
		noun[i].contents = noun[i].next = 0;
	}
	for (i = 0; i <= maxcreat - first_creat; i++)
		creature[i].contents = creature[i].next = 0;
	for (i = 0; i <= maxnoun - first_noun; i++) {
		add_object(noun[i].location, i + first_noun);
		if (noun[i].nearby_noun >= first_noun &&
		        noun[i].nearby_noun <= maxnoun)
			noun[noun[i].nearby_noun - first_noun].something_pos_near_noun = 1;
	}
	for (i = 0; i <= maxcreat - first_creat; i++)
		add_object(creature[i].location, i + first_creat);
	objscore = 0; /* Will need to recompute this ... */
}




/*-------------------------------------------------------------------*/
/*  ROUTINES TO SAVE/RESTORE THE GAME STATE */
/* These are used by RESTART and UNDO as well as SAVE and RESTORE */

/* Game State format: */
/* The first two bytes indicate the length of the block (unsigned).*/
/* The next two bytes indicate the game file somehow (so we don't try to */
/* restore to a different game). */
/* After this comes the game information itself. */
/* All values are still little-endian (that is, LSB first) */

/* These are the macros for writing game information to the state block */
/* There is no difference between signed and unsigned when storing them;
 there will be problems when recovering them again. */

#define g(ft,var) {ft,DT_DEFAULT,&var,0}
#define r(ft,str,f) {ft,DT_DEFAULT,NULL,offsetof(str,f)}
#define dptype   {FT_DESCPTR,DT_DESCPTR,NULL,0}

static file_info fi_savehead[] = {
	g(FT_INT16, loc), g(FT_INT32, tscore), g(FT_INT16, turncnt),
	g(FT_BYTE, statusmode),
	g(FT_BOOL, first_visit_flag), g(FT_BOOL, newlife_flag),
	g(FT_BOOL, room_firstdesc), g(FT_BOOL, verboseflag),
	g(FT_BOOL, notify_flag), g(FT_BOOL, listexit_flag),
	g(FT_BOOL, menu_mode), g(FT_BOOL, sound_on),
	g(FT_BOOL, agt_answer), g(FT_INT32, agt_number),
	g(FT_INT16, curr_time), g(FT_INT16, curr_lives),
	g(FT_INT16, delta_time),
	endrec
};

static file_info fi_saveroom[] = {
	dptype,
	r(FT_BOOL, room_rec, seen),
	r(FT_BOOL, room_rec, locked_door),
	r(FT_INT16, room_rec, oclass),
	r(FT_INT16, room_rec, points),
	r(FT_INT16, room_rec, light),
	r(FT_PATHARRAY, room_rec, path),
	r(FT_UINT32, room_rec, flag_noun_bits),
	endrec
};

static file_info fi_savenoun[] = {
	dptype,
	r(FT_INT16, noun_rec, location),
	r(FT_INT16, noun_rec, nearby_noun),
	r(FT_INT16, noun_rec, num_shots),
	r(FT_INT16, noun_rec, initdesc),
	r(FT_INT16, noun_rec, oclass),
	r(FT_INT16, noun_rec, points),
	r(FT_INT16, noun_rec, weight),
	r(FT_INT16, noun_rec, size),
	r(FT_BOOL, noun_rec, on),
	r(FT_BOOL, noun_rec, open),
	r(FT_BOOL, noun_rec, locked),
	r(FT_BOOL, noun_rec, movable),
	r(FT_BOOL, noun_rec, seen),
	r(FT_WORD, noun_rec, pos_prep),
	r(FT_WORD, noun_rec, pos_name),
	endrec
};

static file_info fi_savecreat[] = {
	dptype,
	r(FT_INT16, creat_rec, location),
	r(FT_INT16, creat_rec, counter),
	r(FT_INT16, creat_rec, timecounter),
	r(FT_INT16, creat_rec, initdesc),
	r(FT_INT16, creat_rec, oclass),
	r(FT_INT16, creat_rec, points),
	r(FT_BOOL, creat_rec, groupmemb),
	r(FT_BOOL, creat_rec, hostile),
	r(FT_BOOL, creat_rec, seen),
	endrec
};

static file_info fi_saveustr[] = {
	{FT_TLINE, DT_DEFAULT, NULL, 0},
	endrec
};



uchar *getstate(uchar *gs)
/* Returns block containing game state.
  If gs!=NULL, uses that space as a buffer;
  if gs==NULL, we malloc a new block and return it */
{
	rbool new_block; /* True if we allocate a new block */
	long bp;

	if (gs == NULL) {
		rm_trap = 0; /* Don't exit on out-of-memory condition */
		gs = (uchar *)rmalloc(state_size); /* This should be enough. */
		rm_trap = 1;
		if (gs == NULL) /* This is why we set rm_trap to 0 before calling rmalloc */
			return NULL;
		new_block = 1;
	} else new_block = 0;

	/* First two bytes reserved for block size, which we don't know yet.*/
	gs[4] = game_sig & 0xFF;
	gs[5] = (game_sig >> 8) & 0xFF;

	tscore -= objscore;  /* Only include "permanent" part of score;
            objscore we can recompute on RESTORE */

	/* Need to setup here */
	set_internal_buffer(gs);
	fi_saveroom[0].ptr = room_ptr;
	fi_savenoun[0].ptr = noun_ptr;
	fi_savecreat[0].ptr = creat_ptr;

	bp = 6;
	bp += write_globalrec(fi_savehead, bp);
	bp += write_recblock(flag, FT_BYTE, FLAG_NUM + 1, bp);
	bp += write_recblock(agt_counter, FT_INT16, CNT_NUM + 1, bp);
	bp += write_recblock(agt_var, FT_INT32, VAR_NUM + 1, bp);
	bp += write_recarray(room, sizeof(room_rec), rangefix(maxroom - first_room + 1),
	                     fi_saveroom, bp);
	bp += write_recarray(noun, sizeof(noun_rec), rangefix(maxnoun - first_noun + 1),
	                     fi_savenoun, bp);
	bp += write_recarray(creature, sizeof(creat_rec),
	                     rangefix(maxcreat - first_creat + 1),
	                     fi_savecreat, bp);
	if (userstr != NULL)
		bp += write_recarray(userstr, sizeof(tline), MAX_USTR, fi_saveustr, bp);
	if (objflag != NULL)
		bp += write_recblock(objflag, FT_BYTE, objextsize(0), bp);
	if (objprop != NULL)
		bp += write_recblock(objprop, FT_INT32, objextsize(1), bp);
	set_internal_buffer(NULL);
	gs[0] = bp & 0xFF;
	gs[1] = (bp >> 8) & 0xFF;
	gs[2] = (bp >> 16) & 0xFF;
	gs[3] = (bp >> 24) & 0x7F; /* Don't trust top bit */
	if (new_block)
		gs = (uchar *)rrealloc(gs, bp);
	tscore += objscore;
	return gs;
}



void putstate(uchar *gs) { /* Restores games state. */
	long size, bp, numrec, i;


	size = gs[0] + (((long)gs[1]) << 8) + (((long)gs[2]) << 16) + (((long)gs[3]) << 24);
	if (size != state_size) {
		writeln("Size difference in save files!");
		agt_delay(3);
		return;
	}
	if (gs[4] + (((long)gs[5]) << 8) != game_sig) {
		writestr("This appears to be a save file for a different game. Is this"
		         " from an earlier chapter in a multi-part game such as"
		         " Klaustrophobia");
		if (yesno("?"))
			skip_descr = 1; /* We don't want to overwrite the descriptions
               with the pointers from the save file. */
		else {
			writestr("Do you want to try using it anyhow (WARNING: This could"
			         " crash the interpreter)");
			if (!(yesno("?"))) {
				writeln("Command cancelled!");
				agt_delay(3);
				return;
			}
		}
	}


	/* setup... */
	set_internal_buffer(gs);
	fi_saveroom[0].ptr = room_ptr;
	fi_savenoun[0].ptr = noun_ptr;
	fi_savecreat[0].ptr = creat_ptr;
	bp = 6;

	read_globalrec(fi_savehead, 0, bp, 0);
	bp += compute_recsize(fi_savehead);
	read_recblock(flag, FT_BYTE, FLAG_NUM + 1, bp, 0);
	bp += ft_leng[FT_BYTE] * (FLAG_NUM + 1);
	read_recblock(agt_counter, FT_INT16, CNT_NUM + 1, bp, 0);
	bp += ft_leng[FT_INT16] * (CNT_NUM + 1);
	read_recblock(agt_var, FT_INT32, VAR_NUM + 1, bp, 0);
	bp += ft_leng[FT_INT32] * (VAR_NUM + 1);

	numrec = rangefix(maxroom - first_room + 1);
	read_recarray(room, sizeof(room_rec), numrec, fi_saveroom, 0, bp, 0);
	bp += compute_recsize(fi_saveroom) * numrec;
	numrec = rangefix(maxnoun - first_noun + 1);
	read_recarray(noun, sizeof(noun_rec), numrec, fi_savenoun, 0, bp, 0);
	bp += compute_recsize(fi_savenoun) * numrec;
	numrec = rangefix(maxcreat - first_creat + 1);
	read_recarray(creature, sizeof(creat_rec), numrec, fi_savecreat, 0, bp, 0);
	bp += compute_recsize(fi_savecreat) * numrec;
	if (userstr != NULL) {
		read_recarray(userstr, sizeof(tline), MAX_USTR, fi_saveustr, 0, bp, 0);
		bp += ft_leng[FT_TLINE] * MAX_USTR;
	}
	if (objflag != NULL) {
		i = objextsize(0);
		read_recblock(objflag, FT_BYTE, i, bp, 0);
		bp += ft_leng[FT_BYTE] * i;
	}
	if (objprop != NULL) {
		i = objextsize(1);
		read_recblock(objprop, FT_INT32, i, bp, 0);
		bp += ft_leng[FT_INT32] * i;
	}
	set_internal_buffer(NULL);

	if (skip_descr)   /* Need to "fix" position information. This is a hack. */
		/* Basically, this sets the position of each object to its default */
		/* The problem here is that the usual position info is invalid-- we've
		   changed games, and hence dictionaries */
		for (i = 0; i < maxnoun - first_noun; i++) {
			if (noun[i].position != NULL && noun[i].position[0] != 0)
				noun[i].pos_prep = -1;
			else noun[i].pos_prep = 0;
		}
	else   /* Rebuild position information */
		for (i = 0; i < maxnoun - first_noun; i++)
			if (noun[i].pos_prep == -1)
				noun[i].position = noun[i].initpos;
			else
				noun[i].position = NULL;

	init_vals();
	skip_descr = 0; /* If we set this to 1, restore it to its original state */
	/* Now do some simple consistancy checking on major variables */
	if (loc > maxroom || loc < 0 || turncnt < 0 ||
	        curr_lives < 0 || curr_lives > max_lives) {
		error("Error: Save file inconsistent.");
	}
}

void init_state_sys(void)
/* Initializes the state saving mechanisms */
/* Mainly it just computes the size of a state block */
{
	state_size = compute_recsize(fi_savehead)
	             + compute_recsize(fi_saveroom) * rangefix(maxroom - first_room + 1)
	             + compute_recsize(fi_savenoun) * rangefix(maxnoun - first_noun + 1)
	             + compute_recsize(fi_savecreat) * rangefix(maxcreat - first_creat + 1)
	             + ft_leng[FT_BYTE] * (FLAG_NUM + 1)
	             + ft_leng[FT_INT16] * (CNT_NUM + 1)
	             + ft_leng[FT_INT32] * (VAR_NUM + 1)
	             + ft_leng[FT_BYTE] * objextsize(0)
	             + ft_leng[FT_INT32] * objextsize(1)
	             + 6;  /* Six bytes in header */
	if (userstr != NULL) state_size += ft_leng[FT_TLINE] * MAX_USTR;
}


/*-------------------------------------------------------------------*/
/*  SAVE FILE ROUTINES    */

extern Common::Error savegame(Common::WriteStream *savefile) {
	uchar *gs;
	long size;

#ifndef UNDO_SAVE
	gs = getstate(NULL);
#else
	gs = undo_state;
#endif
	if (gs == NULL) {
		writeln("Insufficiant memory to support SAVE.");
		return Common::kWritingFailed;
	}

	if (!filevalid(savefile, fSAV)) {
		writeln("That is not a valid save file.");
		return Common::kWritingFailed;
	}
	size = gs[0] + (((long)gs[1]) << 8) + (((long)gs[2]) << 16) + (((long)gs[3]) << 24);
	bool result = binwrite(savefile, gs, size, 1, 0);
#ifndef UNDO_SAVE
	rfree(gs);
#endif
	if (!result) {
		warning("Error writing save file.");
		return Common::kWritingFailed;
	} else {
		return Common::kNoError;
	}
}

/* 1=success, 0=failure */
Common::Error loadgame(Common::SeekableReadStream *loadfile) {
	long size;
	uchar *gs;
	const char *errstr;

	if (!filevalid(loadfile, fSAV)) {
		warning("Unable to open file.");
		return Common::kReadingFailed;
	}
	size = binsize(loadfile);
	if (size == -1) {
		warning("Could not access file.");
		return Common::kReadingFailed;
	}

	gs = (uchar *)rmalloc(size);
	if (!binread(loadfile, gs, size, 1, &errstr)) {
		warning("Error reading file.");
		rfree(gs);
		return Common::kReadingFailed;
	}

	if (size != gs[0] + (((long)gs[1]) << 8) + (((long)gs[2]) << 16) + (((long)gs[3]) << 24)) {
		if (size == gs[0] + (((long)gs[1]) << 8)) {
			/* Old save file format; patch to look like new format */
			gs = (uchar *)rrealloc(gs, size + 2);
			memmove(gs + 4, gs + 2, size - 2);
			gs[2] = gs[3] = 0;
		} else {
			warning("Save file corrupted or invalid.");
			rfree(gs);
			return Common::kReadingFailed;
		}
	}

	putstate(gs);
	rfree(gs);
	set_statline();
	look_room();
	return Common::kNoError;
}

void restart_game(void) {
	putstate(restart_state);
	agt_clrscr();
	set_statline();
	do_look = do_autoverb = 1;
	if (intro_ptr.size > 0) {
		print_descr(intro_ptr, 1);
		wait_return();
		agt_clrscr();
	}
	newroom();
}

} // End of namespace AGT
} // End of namespace Glk
