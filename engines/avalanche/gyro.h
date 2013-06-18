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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef __gyro_h__
#define __gyro_h__


#include "graph.h"
/*#include "Dos.h"*/
/*#include "Crt.h"*/

namespace Avalanche {

const char numobjs = '\22'; /* always preface with a # */
const integer maxobjs = 12;  /* carry limit */
const byte howlong = 1/*8*/; /* 18 ticks. */

const boolean oncandopageswap = true; /* Variable constant for overriding the
  ability of On to switch pages. You may know better than On which page
  to switch to. */

const integer num = 32;  /* Code for Num Lock */

const integer mouse_size = 134;

typedef void(*proc)();

struct postype {
	word x, y, datapos;
	byte length;
};

struct mp { /* mouse-pointer */
	matrix<0, 1, 0, 15, word> mask;
	integer horzhotspot, verthotspot;
};

struct dnatype {
	/* here goes... */ /* Ux, uy, & ww now all belong to Trip5 */
	byte rw; /* Realway- just for convenience! */
	byte carrying; /* how many objects you're carrying... */
	array < '\1', numobjs, boolean > obj; /* ...and which ones they are. */
	integer score; /* your score, of course */
	longint pence; /* your current amount of dosh */
	byte room; /* your current room */
	char wearing; /* what you're wearing */
	byte swore; /* number of times you've sworn */
	byte saves; /* number of times this game has been saved */
	array<1, 100, byte> rooms;     /* Add one to each every time
                                            you enter a room */
	byte alcohol; /* Your blood alcohol level. */
	byte playednim; /* How many times you've played Nim. */
	boolean wonnim; /* Have you *won* Nim? (That's harder.) */
	byte winestate; /* 0=good (Notts), 1=passable(Argent) ... 3=vinegar.*/
	boolean cwytalot_gone; /* Has Cwytalot rushed off to Jerusalem yet?*/

	byte pass_num; /* Number of the password for this game. */
	boolean ayles_is_awake; /* pretty obvious! */
	byte drawbridge_open; /* Between 0 (shut) and 4 (open). */
	byte avaricius_talk; /* How much Avaricius has said to you. */
	boolean bought_onion; /* Have you bought an onion yet? */
	boolean rotten_onion; /* And has it rotted? */
	boolean onion_in_vinegar; /* Is the onion in the vinegar? */

	byte given2spludwick; /* 0 = nothing given, 1 = onion... */
	byte brummie_stairs; /* Progression through the stairs trick. */
	byte cardiff_things; /* Things you get asked in Cardiff. */

	boolean cwytalot_in_herts; /* Have you passed Cwytalot in Herts?*/

	boolean avvy_is_awake; /* Well? Is Avvy awake? (Screen 1 only.) */
	boolean avvy_in_bed; /* True if Avvy's in bed, but awake. */

	boolean user_moves_avvy; /* If this is false, the user has no
                                        control over Avvy's movements. */

	byte dogfoodpos; /* Which way Dogfood's looking in the pub. */

	boolean givenbadgetoiby; /* Have you given the badge to Iby yet? */

	boolean friar_will_tie_you_up; /* If you're going to get tied up. */
	boolean tied_up; /* You ARE tied up! */

	char box_contents; /* 0 = money (sixpence), 254 = empty, any
             other number implies the contents of the box. */

	boolean talked_to_crapulus; /* Pretty self-explanatory. */

	byte jacques_awake; /* 0=asleep, 1=awake, 2=gets up, 3=gone. */

	boolean ringing_bells; /* Is Jacques ringing the bells? */

	boolean standing_on_dais; /* In room 71, inside Cardiff Castle. */
	boolean taken_pen; /* Have you taken the pen (in Cardiff?) */
	boolean arrow_triggered; /* And has the arrow been triggered? */
	boolean arrow_in_the_door;  /* Did the arrow hit the wall? */

	varying_string<77> like2drink,
	               favourite_song,
	               worst_place_on_earth,
	               spare_evening;            /* Personalisation str's */

	longint total_time; /* Your total time playing this game, in ticks.*/

	byte jumpstatus; /* Fixes how high you're jumping. */

	boolean mushroom_growing; /* Is the mushroom growing in 42? */

	boolean spludwicks_here; /* Is Spludwick at home? */

	byte last_room;
	byte last_room_not_map;

	boolean crapulus_will_tell; /* Will Crapulus tell you about
                        Spludwick being away? */

	boolean enter_catacombs_from_lusties_room;
	boolean teetotal; /* Are we touching any more drinks? */
	byte malagauche; /* Position of Malagauche. See Celer for more info. */
	char drinking; /* What's he getting you? */

	boolean entered_lusties_room_as_monk;

	byte cat_x, cat_y;   /* XY coords in the catacombs. */

	boolean avvys_in_the_cupboard; /* On screen 22. */

	boolean geida_follows; /* Is Geida following you? */

	byte geida_spin, geida_time; /* For the making "Geida dizzy" joke. */

	byte nextbell; /* For the ringing. */

	boolean geida_given_potion; /* Does Geida have the potion? */
	boolean lustie_is_asleep; /* Is BDL asleep? */

	byte flip_to_where, flip_to_ped; /* For the sequencer. */

	boolean been_tied_up; /* In r__Robins. */

	boolean sitting_in_pub; /* Are you sitting down in the pub? */
	byte spurge_talk; /* Count for talking to Spurge. */

	boolean met_avaroid;

	boolean taken_mushroom,
	        given_pen_to_ayles,
	        asked_dogfood_about_nim;
};

struct pedtype {
	integer x, y;
	byte dir;
};

struct magictype {
	byte op; /* one of the operations */
	word data; /* data for them */
};

class fieldtype {
public:
	integer x1, y1, x2, y2;
};

struct bytefield {
	byte x1, y1, x2, y2;
};

class linetype : public fieldtype {
public:
	byte col;
};

struct adxtype {
	varying_string<12> name; /* name of character */
	byte num; /* number of pictures */
	byte xl, yl; /* x & y lengths of pictures */
	byte seq; /* how many in one stride */
	word size; /* the size of one picture */
	byte fgc, bgc; /* foreground & background bubble colours */
};

typedef matrix < '\0', '\377', 0, 15, byte > raw; /* raw_font_type */

enum controllers {cjoy, ckey, last_controllers};

typedef array<1, 20, varying_string<77> > previoustype;

struct corridor_type { /* Decarations for the corridors. */
	word doors; /* Door styles are calc'ed from this word.
                   Assign a different number to each one! */
};

struct demo_type {
	word delay;
	char key, extd;
};

struct quasiped_type {
	byte whichped, fgc, room, bgc;
	char who;
};
/* A quasiped defines how people who aren't sprites talk. For example,
   quasiped "A" is Dogfood. The rooms aren't stored because I'm leaving
   that to context. */

typedef array<1, 31, byte> tunetype;

struct vmctype { /* Virtual Mouse Cursor */
	pointer andpic, xorpic;
	array<0, 1, pointer> backpic;
	array<0, 1, pointtype> wherewas;
	byte picnumber;
	shortint ofsx, ofsy;
};

struct sundry {
	/* Things which must be saved over a backtobootstrap,
	                   outside DNA. */
	pathstr qenid_filename;
	boolean qsoundfx;
	char qthinks;
	boolean qthinkthing;
};

struct joysetup {
	word xmid, ymid, xmin, ymin, xmax, ymax;
	byte centre; /* Size of centre in tenths */
};

struct ednahead { /* Edna header */
	/* This header starts at byte offset 177 in the .ASG file. */
	array<1, 9, char> id;   /* signature */
	word revision; /* EDNA revision, here 2 (1=dna256) */
	varying_string<50> game; /* Long name, eg Lord Avalot D'Argent */
	varying_string<15> shortname; /* Short name, eg Avalot */
	word number; /* Game's code number, here 2 */
	word ver; /* Version number as integer (eg 1.00 = 100) */
	varying_string<5> verstr; /* Vernum as string (eg 1.00 = "1.00" */
	varying_string<12> filename; /* Filename, eg AVALOT.EXE */
	byte osbyte; /* Saving OS (here 1=DOS. See below for others.*/
	varying_string<5> os; /* Saving OS in text format. */

	/* Info on this particular game */

	varying_string<8> fn; /* Filename (not extension ('cos that's .ASG)) */
	byte d, m; /* D, M, Y are the Day, Month & Year this game was... */
	word y;  /* ...saved on. */
	varying_string<40> desc; /* Description of game (same as in Avaricius!) */
	word len; /* Length of DNA (it's not going to be above 65535!) */

	/* Quick reference & miscellaneous */

	word saves; /* no. of times this game has been saved */
	integer cash; /* contents of your wallet in numerical form */
	varying_string<20> money; /* ditto in string form (eg 5/-, or 1 denarius)*/
	word points; /* your score */

	/* DNA values follow, then footer (which is ignored) */
};

/* Possible values of edhead.os:
   1 = DOS        4 = Mac
   2 = Windows    5 = Amiga
   3 = OS/2       6 = ST
   7 = Archimedes */

const char vernum[] = "1.30";
const char copyright[] = "1995";
const integer thisvercode = 130;
/* as "vernum", but numerically & without the ".". */
const integer thisgamecode = 2; /* Avalot's code number */

/* Objects you can hold: */
const char wine = '\1';
const char money = '\2';
const char bodkin = '\3';
const char potion = '\4';
const char chastity = '\5';
const char bolt = '\6';
const char crossbow = '\7';
const char lute = '\10';
const char badge = '\11';
const char mushroom = '\12';
const char key = '\13';
const char bell = '\14';
const char prescription = '\15';
const char pen = '\16';
const char ink = '\17';
const char clothes = '\20';
const char habit = '\21';
const char onion = '\22';

/* People who hang around this game. */

/* Boys: */
const char pavalot = '\226';
const char pspludwick = '\227';
const char pcrapulus = '\230';
const char pdrduck = '\231';
const char pmalagauche = '\232';
const char pfriartuck = '\233';
const char probinhood = '\234';
const char pcwytalot = '\235';
const char pdulustie = '\236';
const char pduke = '\237';
const char pdogfood = '\240';
const char ptrader = '\241';
const char pibythneth = '\242';
const char payles = '\243';
const char pport = '\244';
const char pspurge = '\245';
const char pjacques = '\246';

/* Girls: */
const char parkata = '\257';
const char pgeida = '\260';
const char pwisewoman = '\262';

const integer xw = 30;
const integer yw = 36; /* x width & y whatsit */

const integer margin = 5;

const array<1, 9, mp> mps = {
	{   {
			/* 1 - up-arrow */
			{	{	{{65151, 64575, 64575, 63519, 63519, 61455, 61455, 57351, 57351, 49155, 49155, 64575, 64575, 64575, 64575, 64575}},
					{{0, 384, 384, 960, 960, 2016, 2016, 4080, 4080, 8184, 384, 384, 384, 384, 384, 0}}
				}
			},
			8,
			0
		},

		{
			/* 2 - screwdriver */
			{	{	{{8191, 4095, 2047, 34815, 50175, 61951, 63743, 64543, 65039, 65031, 65027, 65281, 65408, 65472, 65505, 65523}},
					{{0, 24576, 28672, 12288, 2048, 1024, 512, 256, 224, 176, 216, 96, 38, 10, 12, 0}}
				}
			},
			0,
			0
		},

		{
			/* 3 - right-arrow */
			{	{	{{65535, 65535, 64639, 64543, 7, 1, 0, 1, 7, 64543, 64639, 65535, 65535, 65535, 65535, 65535}},
					{{0, 0, 0, 384, 480, 32760, 32766, 32760, 480, 384, 0, 0, 0, 0, 0, 0}}
				}
			},
			15,
			6
		},

		{
			/* 4 - fletch */
			{	{	{{255, 511, 1023, 2047, 1023, 4607, 14591, 31871, 65031, 65283, 65281, 65280, 65280, 65409, 65473, 65511}},
					{{0, 10240, 20480, 24576, 26624, 17408, 512, 256, 128, 88, 32, 86, 72, 20, 16, 0}}
				}
			},
			0,
			0
		},

		{
			/* 5 - hourglass */
			{	{	{{0, 0, 0, 34785, 50115, 61455, 61455, 63519, 63519, 61839, 61455, 49155, 32769, 0, 0, 0}},
					{{0, 32766, 16386, 12300, 2064, 1440, 1440, 576, 576, 1056, 1440, 3024, 14316, 16386, 32766, 0}}
				}
			},
			8,
			7
		},

		{
			/* 6 - TTHand */
			{	{	{{62463, 57855, 57855, 57855, 57471, 49167, 32769, 0, 0, 0, 0, 32768, 49152, 57344, 61441, 61443}},
					{{3072, 4608, 4608, 4608, 4992, 12912, 21070, 36937, 36873, 36865, 32769, 16385, 8193, 4097, 2050, 4092}}
				}
			},
			4,
			0
		},

		{
			/* 7- Mark's crosshairs */
			{	{	{{65535, 65151, 65151, 65151, 65151, 0, 65151, 65151, 65151, 65151, 65535, 65535, 65535, 65535, 65535, 65535}},
					{{0, 384, 384, 384, 384, 65535, 384, 384, 384, 384, 0, 0, 0, 0, 0, 0}}
				}
			},
			8,
			5
		},

		{
			/* 8- I-beam. */
			{	{	{{65535, 65535, 63631, 63503, 63503, 65087, 65087, 65087, 65087, 65087, 63503, 63503, 63631, 65535, 65535, 65535}},
					{{0, 0, 0, 864, 128, 128, 128, 128, 128, 128, 128, 864, 0, 0, 0, 0}}
				}
			},
			8,
			7
		},

		{
			/* 9- Question mark. */
			{	{	{{511, 1023, 2047, 31, 15, 8199, 32647, 65415, 63503, 61471, 61503, 61695, 63999, 63999, 61695, 61695}},
					{{65024, 33792, 34816, 34784, 40976, 57224, 32840, 72, 1936, 2080, 2496, 2304, 1536, 1536, 2304, 3840}}
				}
			},
			0,
			0
		}
	}
};

const array < '\226', '\246', varying_string<19> > lads = {
	{
		"Avalot", "Spludwick", "Crapulus", "Dr. Duck", "Malagauche", "Friar Tuck",
		"Robin Hood", "Cwytalot", "du Lustie", "the Duke of Cardiff", "Dogfood",
		"A trader", "Ibythneth", "Ayles", "Port", "Spurge", "Jacques"
	}
};

const array < '\257', '\262', varying_string<14> > lasses =
{{"Arkata", "Geida", '±', "the Wise Woman"}};

const array < '\226', '\245', char > ladchar = "ASCDMTRwLfgeIyPu";

const array < '\257', '\262', char > lasschar = "kG±o";

const integer numtr = 2; /* current max no. of sprites */

const boolean a_thing = true;
const boolean a_person = false; /* for Thinkabout */

/* Magic/portal commands are */

/*N*/ const integer nix = 0; /* ignore it if this line is touched */
/*B*/ const integer bounces = 1; /* bounce off this line. Not valid for portals. */
/*E*/ const integer exclaim = 2; /* put up a chain of scrolls */
/*T*/ const integer transport = 3; /* enter new room */
/*U*/ const integer unfinished = 4; /* unfinished connection */
/*S*/ const integer special = 5; /* special function. */
/*O*/ const integer mopendoor = 6; /* opening door. */

/* These following constants should be included in CFG when it's written. */

const boolean slow_computer = false; /* stops walking when mouse touches toolbar */

/* --- */

const integer border = 1; /* size of border on shadowboxes */

const integer pagetop = 81920;

const integer up = 0;
const integer right = 1;
const integer down = 2;
const integer left = 3;
const integer ur = 4;
const integer dr = 5;
const integer dl = 6;
const integer ul = 7;
const integer stopped = 8;

const integer walk = 3;
const integer run = 5;

/*$I ROOMNUMS.INC - Room number constants (r__xxx) */

const array < '\226', '\262', byte > whereis =
    /* The Lads */
{   {
		r__yours, /* Avvy */
		r__spludwicks, /* Spludwick */
		r__outsideyours, /* Crapulus */
		r__ducks, /* Duck - r__DucksRoom's not defined yet. */
		r__argentpub, /* Malagauche */
		r__robins, /* Friar Tuck. */
		177, /* Robin Hood - can't meet him at the start. */
		r__brummieroad, /* Cwytalot */
		r__lustiesroom, /* Baron du Lustie. */
		r__outsidecardiffcastle, /* The Duke of Cardiff. */
		r__argentpub, /* Dogfood */
		r__outsideducks, /* Trader */
		r__argentpub, /* Ibythneth */
		r__aylesoffice, /* Ayles */
		r__nottspub, /* Port */
		r__nottspub, /* Spurge */
		r__musicroom, /* Jacques */
		0, 0, 0, 0, 0, 0, 0, 0,
		/* The Lasses */
		r__yours, /* Arkata */
		r__geidas, /* Geida */
		177, /* nobody allocated here! */
		r__wisewomans
	}
}; /* The Wise Woman. */

/* Art gallery at 2,1; notice about this at 2,2. */

const matrix<1, 8, 1, 8, longint> catamap =
    /* Geida's room */
    /*  1     2     3   | 4     5     6     7     8*/
{	{	{{0x204, 0x200, 0xd0f0, 0xf0ff, 0xff, 0xd20f, 0xd200, 0x200}},
		{{0x50f1, 0x20ff, 0x2ff, 0xff, 0xe0ff, 0x20ff, 0x200f, 0x7210}},
		{{0xe3f0, 0xe10f, 0x72f0, 0xff, 0xe0ff, 0xff, 0xff, 0x800f}},
		{{0x2201, 0x2030, 0x800f, 0x220, 0x20f, 0x30, 0xff, 0x23f}}, /* >> Oubliette */
		{{0x5024, 0xf3, 0xff, 0x200f, 0x22f0, 0x20f, 0x200, 0x7260}},
		{{0xf0, 0x2ff, 0xe2ff, 0xff, 0x200f, 0x50f0, 0x72ff, 0x201f}},
		{{0xf6, 0x220f, 0x22f0, 0x30f, 0xf0, 0x20f, 0x8200, 0x2f0}}, /* <<< In here */
		{{0x34, 0x200f, 0x51f0, 0x201f, 0xf1, 0x50ff, 0x902f, 0x2062}}
	}
};
/* vv Stairs trap. */

/* Explanation: $NSEW.
   Nibble N: North.
    0     = no connection,
    2     = (left,) middle(, right) door with left-hand handle,
    5     = (left,) middle(, right) door with right-hand handle,
    7     = arch,
    8     = arch and 1 north of it,
    9     = arch and 2 north of it,
    D     = no connection + WINDOW,
    E     = no connection + TORCH,
    F     = recessed door (to Geida's room.)

   Nibble S: South.
    0     = no connection,
    1,2,3 = left, middle, right door.

   Nibble E: East.
    0     = no connection (wall),
    1     = no connection (wall + window),
    2     = wall with door,
    3     = wall with door and window,
    6     = wall with candles,
    7     = wall with door and candles,
    F     = straight-through corridor.

   Nibble W: West.
    0     = no connection (wall),
    1     = no connection (wall + shield),
    2     = wall with door,
    3     = wall with door and shield,
    4     = no connection (window),
    5     = wall with door and window,
    6     = wall with candles,
    7     = wall with door and candles,
    F     = straight-through corridor. */

const byte interrogation = 0;
/* If this is greater than zero, the next line you type is stored in
   the DNA in a position dictated by the value. If a scroll comes up,
   or you leave the room, it's automatically set to zero. */

const boolean demo = false; /* If this is true, we're in a demo of the game. */

const array<0, 2, char> spludwick_order = {{onion, ink, mushroom}};

const array<10, 25, quasiped_type> quasipeds = {
	{	{2, lightgray, 19, brown, pdogfood},                       /* A: Dogfood (screen 19). */
		{3, green,     19, white, pibythneth},                         /* B: Ibythneth (screen 19). */
		{3, white,     1, magenta, parkata},                        /* C: Arkata (screen 1). */
		{3, black,     23, red, '\261'},                         /* D: Hawk (screen 23). */
		{3, lightgreen, 50, brown, ptrader},                        /* E: Trader (screen 50). */
		{6, yellow,    42, red, pavalot},                           /* F: Avvy, tied up (scr.42) */
		{2, blue,      16, white, payles},                         /* G: Ayles (screen 16). */
		{2, brown,     7, white, pjacques},                          /* H: Jacques (screen 7). */
		{2, lightgreen, 47, green, pspurge},                        /* I: Spurge (screen 47). */
		{3, yellow,    47, red, pavalot},                           /* J: Avalot (screen 47). */
		{2, lightgray, 23, black, pdulustie},                         /* K: du Lustie (screen 23). */
		{2, yellow,    27, red, pavalot},                           /* L: Avalot (screen 27). */
		{3, white,     27, red, '\261'},                         /* M: Avaroid (screen 27). */
		{4, lightgray, 19, darkgray, pmalagauche},                        /*N: Malagauche (screen 19). */
		{5, lightmagenta, 47, red, pport},                           /* O: Port (screen 47). */
		{2, lightgreen, 51, darkgray, pdrduck}
	}
};                         /*P: Duck (screen 51). */

const integer lower = 0;
const integer same = 1;
const integer higher = 2;

const array<1, 12, char> keys = "QWERTYUIOP[]";
const array<1, 12, word> notes =
{{196, 220, 247, 262, 294, 330, 350, 392, 440, 494, 523, 587}};

const tunetype tune = {
	{
		higher, higher, lower, same, higher, higher, lower, higher, higher, higher,
		lower, higher, higher,
		same, higher, lower, lower, lower, lower, higher, higher, lower, lower, lower,
		lower, same, lower, higher, same, lower, higher
	}
};

/* special run-time errors */

const integer runerr_getset_overflow = 50;


#ifdef __gyro_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN varying_string<77> current;
EXTERN byte curpos;
EXTERN boolean cursoron;
/* previous:^previoustype;*/
EXTERN varying_string<77> last;
EXTERN dnatype dna;
EXTERN array<1, 50, linetype> lines; /* For Also. */
EXTERN integer c;
EXTERN registers r;
EXTERN enum { m_no , m_yes , m_virtual } visible;
EXTERN boolean dropsok, screturn, soundfx, cheat;
EXTERN word mx, my; /* mouse x & y now */
EXTERN word mpx, mpy; /* mouse x & y when pressed */
EXTERN word mrx, mry; /* mouse x & y when released */
EXTERN byte mpress, mrelease; /* times left mouse button has been pressed/released */
EXTERN byte keystatus; /* Mouse key status */
EXTERN array<1, 10, varying_string<20> > un;
EXTERN byte unn;
EXTERN string mousetext;
/* which:array[0..5] of byte;*/
EXTERN pointer p;
EXTERN boolean weirdword;
EXTERN byte to_do;
EXTERN boolean lmo, mousemade;
EXTERN array<1, 15, varying_string<50> > scroll;
EXTERN byte scrolln, score, whichwas;
EXTERN char thinks;
EXTERN boolean thinkthing;

/* pp:array[1..1000] of postype;
 bb:array[1..9000] of byte;*/
EXTERN word pptr, bptr;
EXTERN matrix<0, 0, 0, 1, integer> ppos;
EXTERN array<1, 24, word> pozzes;
EXTERN byte anim;
EXTERN pointer copier;
EXTERN integer talkx, talky;
EXTERN byte talkb, talkf;
EXTERN byte scrollbells; /* no. of times to ring the bell */
EXTERN boolean ontoolbar, seescroll;

EXTERN array<1, 10, char> objlist;
EXTERN array < '0', '9', pointer > digit;
EXTERN array<0, 8, pointer> rwlite;
EXTERN byte oldrw;
EXTERN varying_string<3> lastscore;
EXTERN byte cmp; /* current mouse-pointer */
EXTERN varying_string<10> verbstr; /* what you can do with your object. :-) */

EXTERN matrix<0, 30, 0, 1, string *> also;
EXTERN array<1, 15, pedtype> peds;
EXTERN array<1, 15, magictype> magics;
EXTERN array<9, 15, magictype> portals;
EXTERN array<1, 30, fieldtype> fields;
EXTERN byte numfields;
EXTERN varying_string<26> flags;
EXTERN string listen;

EXTERN word oh, onh, om, h, m, s, s1;

EXTERN varying_string<4> atkey; /* For XTs, set to "alt-". For ATs, set to "f1". */

EXTERN byte cp, ledstatus, defaultled;
EXTERN raw little;
EXTERN boolean quote; /* 66 or 99 next? */
EXTERN boolean alive;
EXTERN array<1, 2000, char> buffer;
EXTERN word bufsize;

EXTERN byte oldjw; /* Old joystick-way */
EXTERN controllers ctrl;

EXTERN integer underscroll; /* Y-coord of just under the scroll text. */

/* TSkellern is only temporary, and I'll replace it
   with a local version when it's all fixed up. */

/* tskellern:longint absolute $0:244; { Over int $61 }*/

EXTERN boolean ddmnow; /* Kludge so we don't have to keep referring to Dropdown */
EXTERN varying_string<40> roomname; /* Name of this room */

EXTERN text logfile;
EXTERN boolean logging, log_epson;

EXTERN boolean cl_override;

EXTERN byte locks; /*ABSOLUTE $40:$17;*/

EXTERN varying_string<20> subject; /* What you're talking to them about. */
EXTERN byte subjnumber; /* The same thing. */

EXTERN boolean keyboardclick; /* Is a keyboard click noise wanted? */

EXTERN char him, her, it;
EXTERN longint roomtime; /* Set to 0 when you enter a room, added to in every loop.*/

EXTERN boolean after_the_scroll;

/* For the demo: */
EXTERN demo_type demo_rec;
EXTERN file<demo_type> demofile;

EXTERN char last_person; /* Last person to have been selected using the People
                     menu. */

EXTERN boolean doing_sprite_run; /* Only set to True if we're doing a sprite_run
  at this moment. This stops the trippancy system from moving any of the
  sprites. */

EXTERN vmctype vmc;
EXTERN string filetoload;

EXTERN boolean holdthedawn; /* If this is true, calling Dawn will do nothing.
  It's used, for example, at the start, to stop Load from dawning. */

EXTERN word storage_seg, storage_ofs; /* Seg and ofs of the Storage area. */
EXTERN word skellern; /* Offset of the timer variable - 1 more than storage_OFS */
EXTERN boolean reloaded; /* Is this NOT the primary loading? */

EXTERN boolean super_was_virtual, super_was_off; /* Used by Super_Off and Super_On */

EXTERN pathstr enid_filename;

EXTERN joysetup js;
EXTERN word cxmin, cxmax, cymin, cymax;
EXTERN boolean use_joy_a;
#undef EXTERN
#define EXTERN extern


void newpointer(byte m);

void wait();    /* makes hourglass */

void on();

void off();

void on_virtual();

void off_virtual();

void xycheck();

void hopto(integer x, integer y); /* Moves mouse pointer to x,y */

void check();

void note(word hertz);

void blip();

string strf(longint x);

void shbox(integer x1, integer y1, integer x2, integer y2, string t);

void newgame();

void click();

void slowdown();

boolean flagset(char x);

void force_numlock();

boolean pennycheck(word howmuchby);

string getname(char whose);

char getnamechar(char whose);

string get_thing(char which);

char get_thingchar(char which);

string get_better(char which);

string f5_does();

void plot_vmc(integer xx, integer yy, byte page_);

void wipe_vmc(byte page_);

void setup_vmc();

void clear_vmc();

void load_a_mouse(byte which);

void background(byte x);

void hang_around_for_a_while();

void super_off();

void super_on();

boolean mouse_near_text();

} // End of namespace Avalanche.

#endif