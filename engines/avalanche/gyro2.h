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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#ifndef GYRO2_H
#define GYRO2_H

#include "common/system.h"
#include "common/str.h"
#include "common/scummsys.h"
#include "avalanche/roomnums.h"
#include "avalanche/color.h"

namespace Avalanche {

	namespace Gyro {

	///////////////////////////// CONSTANTS ///////////////////////////// 

	const char numobjs = 18; /* always preface with a # */
	const int16 maxobjs = 12;  /* carry limit */
	const byte howlong = 1/*8*/; /* 18 ticks. */

	const bool oncandopageswap = true; 
	/* Variable constant for overriding the ability of On to switch pages.
	 * You may know better than On which page to switch to. */

	const int16 num = 32;  /* Code for Num Lock */

	const int16 mouse_size = 134;



	///////////////////////////// TYPEDEFS ///////////////////////////// 

	typedef void (*proc)();

	struct postype {
		uint16 x, y, datapos;
		byte length;
	};

	struct mp { /* mouse-void **/
		uint16 mask[2][16];
		int16 horzhotspot, verthotspot;
	};

	struct dnatype { /* here goes... */ /* Ux, uy, & ww now all belong to Trip5 */
		byte rw; /* Realway- just for convenience! */
		byte carrying; /* how many objects you're carrying... */
		bool obj[numobjs]; /* ...and which ones they are. */
		int16 score; /* your score, of course */
		int32 pence; /* your current amount of dosh */
		byte room; /* your current room */
		char wearing; /* what you're wearing */
		byte swore; /* number of times you've sworn */
		byte saves; /* number of times this game has been saved */
		byte rooms[100];     /* Add one to each every time
												you enter a room */
		byte alcohol; /* Your blood alcohol level. */
		byte playednim; /* How many times you've played Nim. */
		bool wonnim; /* Have you *won* Nim? (That's harder.) */
		byte winestate; /* 0=good (Notts), 1=passable(Argent) ... 3=vinegar.*/
		bool cwytalot_gone; /* Has Cwytalot rushed off to Jerusalem yet?*/

		byte pass_num; /* Number of the passuint16 for this game. */
		bool ayles_is_awake; /* pretty obvious! */
		byte drawbridge_open; /* Between 0 (shut) and 4 (open). */
		byte avaricius_talk; /* How much Avaricius has said to you. */
		bool bought_onion; /* Have you bought an onion yet? */
		bool rotten_onion; /* And has it rotted? */
		bool onion_in_vinegar; /* Is the onion in the vinegar? */

		byte given2spludwick; /* 0 = nothing given, 1 = onion... */
		byte brummie_stairs; /* Progression through the stairs trick. */
		byte cardiff_things; /* Things you get asked in Cardiff. */

		bool cwytalot_in_herts; /* Have you passed Cwytalot in Herts?*/

		bool avvy_is_awake; /* Well? Is Avvy awake? (Screen 1 only.) */
		bool avvy_in_bed; /* True if Avvy's in bed, but awake. */

		bool user_moves_avvy; /* If this is false, the user has no
											control over Avvy's movements. */

		byte dogfoodpos; /* Which way Dogfood's looking in the pub. */

		bool givenbadgetoiby; /* Have you given the badge to Iby yet? */

		bool friar_will_tie_you_up; /* If you're going to get tied up. */
		bool tied_up; /* You ARE tied up! */

		char box_contents; /* 0 = money (sixpence), 254 = empty, any
				 other number implies the contents of the box. */

		bool talked_to_crapulus; /* Pretty self-explanatory. */

		byte jacques_awake; /* 0=asleep, 1=awake, 2=gets up, 3=gone. */

		bool ringing_bells; /* Is Jacques ringing the bells? */

		bool standing_on_dais; /* In room 71, inside Cardiff Castle. */
		bool taken_pen; /* Have you taken the pen (in Cardiff?) */
		bool arrow_triggered; /* And has the arrow been triggered? */
		bool arrow_in_the_door;  /* Did the arrow hit the wall? */

		Common::String like2drink,
					   favourite_song,
					   worst_place_on_earth,
					   spare_evening;            /* Personalisation str's */

		int32 total_time; /* Your total time playing this game, in ticks.*/

		byte jumpstatus; /* Fixes how high you're jumping. */

		bool mushroom_growing; /* Is the mushroom growing in 42? */

		bool spludwicks_here; /* Is Spludwick at home? */

		byte last_room;
		byte last_room_not_map;

		bool crapulus_will_tell; /* Will Crapulus tell you about
							Spludwick being away? */

		bool enter_catacombs_from_lusties_room;
		bool teetotal; /* Are we touching any more drinks? */
		byte malagauche; /* Position of Malagauche. See Celer for more info. */
		char drinking; /* What's he getting you? */

		bool entered_lusties_room_as_monk;

		byte cat_x, cat_y;   /* XY coords in the catacombs. */

		bool avvys_in_the_cupboard; /* On screen 22. */

		bool geida_follows; /* Is Geida following you? */

		byte geida_spin, geida_time; /* For the making "Geida dizzy" joke. */

		byte nextbell; /* For the ringing. */

		bool geida_given_potion; /* Does Geida have the potion? */
		bool lustie_is_asleep; /* Is BDL asleep? */

		byte flip_to_where, flip_to_ped; /* For the sequencer. */

		bool been_tied_up; /* In r__Robins. */

		bool sitting_in_pub; /* Are you sitting down in the pub? */
		byte spurge_talk; /* Count for talking to Spurge. */

		bool met_avaroid;

		bool taken_mushroom,
				given_pen_to_ayles,
				asked_dogfood_about_nim;
	};

	struct pedtype {
		int16 x, y;
		byte dir;
	};

	struct magictype {
		byte op; /* one of the operations */
		uint16 data; /* data for them */
	};

	class fieldtype {
	public:
		int16 x1, y1, x2, y2;
	};

	struct bytefield {
		byte x1, y1, x2, y2;
	};

	class linetype : public fieldtype {
	public:
		byte col;
	};

	struct adxtype {
		Common::String name; /* name of character */
		byte num; /* number of pictures */
		byte xl, yl; /* x & y lengths of pictures */
		byte seq; /* how many in one stride */
		uint16 size; /* the size of one picture */
		byte fgc, bgc; /* foreground & background bubble colours */
	};

	typedef byte raw[256][16]; /* raw_font_type */

	enum controllers {cjoy, ckey};

	typedef Common::String previoustype[20];

	struct corridor_type { /* Decarations for the corridors. */
		uint16 doors; /* Door styles are calc'ed from this uint16.
					   Assign a different number to each one! */
	};

	struct demo_type {
		uint16 delay;
		char key, extd;
	};

	struct quasiped_type {
		byte whichped, fgc, room, bgc;
		uint16 who;
	};
	/* A quasiped defines how people who aren't sprites talk. For example,
	   quasiped "A" is Dogfood. The rooms aren't stored because I'm leaving
	   that to context. */

	typedef byte tunetype[31];

	struct vmctype { /* Virtual Mouse Cursor */
		void *andpic, *xorpic;
		void *backpic[2];
		void* wherewas[2];
		byte picnumber;
		int8 ofsx, ofsy;
	};

	struct sundry {
		/* Things which must be saved over a backtobootstrap,
						   outside DNA. */
		Common::String qenid_filename;
		bool qsoundfx;
		char qthinks;
		bool qthinkthing;
	};

	struct joysetup {
		uint16 xmid, ymid, xmin, ymin, xmax, ymax;
		byte centre; /* Size of centre in tenths */
	};

	struct ednahead { /* Edna header */
		/* This header starts at byte offset 177 in the .ASG file. */
		char id[9];   /* signature */
		uint16 revision; /* EDNA revision, here 2 (1=dna256) */
		Common::String game; /* Long name, eg Lord Avalot D'Argent */
		Common::String shortname; /* Short name, eg Avalot */
		uint16 number; /* Game's code number, here 2 */
		uint16 ver; /* Version number as int16 (eg 1.00 = 100) */
		Common::String verstr; /* Vernum as Common::String (eg 1.00 = "1.00" */
		Common::String filename; /* Filename, eg AVALOT.EXE */
		byte osbyte; /* Saving OS (here 1=DOS. See below for others.*/
		Common::String os; /* Saving OS in text format. */

		/* Info on this particular game */

		Common::String fn; /* Filename (not extension ('cos that's .ASG)) */
		byte d, m; /* D, M, Y are the Day, Month & Year this game was... */
		uint16 y;  /* ...saved on. */
		Common::String desc; /* Description of game (same as in Avaricius!) */
		uint16 len; /* Length of DNA (it's not going to be above 65535!) */

		/* Quick reference & miscellaneous */

		uint16 saves; /* no. of times this game has been saved */
		int16 cash; /* contents of your wallet in numerical form */
		Common::String money; /* ditto in Common::String form (eg 5/-, or 1 denarius)*/
		uint16 points; /* your score */

		/* DNA values follow, then footer (which is ignored) */
	};

	/* Possible values of edhead.os:
	   1 = DOS        4 = Mac
	   2 = Windows    5 = Amiga
	   3 = OS/2       6 = ST
	   7 = Archimedes */



	///////////////////////////// CONSTANTS ///////////////////////////// 

	const char vernum[] = "1.30";
	const char copyright[] = "1995";
	const int16 thisvercode = 130;
	/* as "vernum", but numerically & without the ".". */
	const int16 thisgamecode = 2; /* Avalot's code number */

	/* Objects you can hold: */
	const char wine = 1;
	const char money = 2;
	const char bodkin = 3;
	const char potion = 4;
	const char chastity = 5;
	const char bolt = 6;
	const char crossbow = 7;
	const char lute = 10;
	const char badge = 11;
	const char mushroom = 12;
	const char key = 13;
	const char bell = 14;
	const char prescription = 15;
	const char pen = 16;
	const char ink = 17;
	const char clothes = 20;
	const char habit = 21;
	const char onion = 22;

	/* People who hang around this game. */

	/* Boys: */
	const uint16 pavalot = 226;
	const uint16 pspludwick = 227;
	const uint16 pcrapulus = 230;
	const uint16 pdrduck = 231;
	const uint16 pmalagauche = 232;
	const uint16 pfriartuck = 233;
	const uint16 probinhood = 234;
	const uint16 pcwytalot = 235;
	const uint16 pdulustie = 236;
	const uint16 pduke = 237;
	const uint16 pdogfood = 240;
	const uint16 ptrader = 241;
	const uint16 pibythneth = 242;
	const uint16 payles = 243;
	const uint16 pport = 244;
	const uint16 pspurge = 245;
	const uint16 pjacques = 246;

	/* Girls: */
	const uint16 parkata = 257;
	const uint16 pgeida = 260;
	const uint16 pwisewoman = 262;

	const int16 xw = 30;
	const int16 yw = 36; /* x width & y whatsit */

	const int16 margin = 5;

	const mp mps[9] = {
		   {
				/* 1 - up-arrow */
				{	{65151, 64575, 64575, 63519, 63519, 61455, 61455, 57351, 57351, 49155, 49155, 64575, 64575, 64575, 64575, 64575},
					{0, 384, 384, 960, 960, 2016, 2016, 4080, 4080, 8184, 384, 384, 384, 384, 384, 0}
				},	
				8,
				0
			},

			{
				/* 2 - screwdriver */
				{	{8191, 4095, 2047, 34815, 50175, 61951, 63743, 64543, 65039, 65031, 65027, 65281, 65408, 65472, 65505, 65523},
					{0, 24576, 28672, 12288, 2048, 1024, 512, 256, 224, 176, 216, 96, 38, 10, 12, 0}
					
				},
				0,
				0
			},

			{
				/* 3 - right-arrow */
				{	{65535, 65535, 64639, 64543, 7, 1, 0, 1, 7, 64543, 64639, 65535, 65535, 65535, 65535, 65535},
					{0, 0, 0, 384, 480, 32760, 32766, 32760, 480, 384, 0, 0, 0, 0, 0, 0}	
				},
				15,
				6
			},

			{
				/* 4 - fletch */
				{	{255, 511, 1023, 2047, 1023, 4607, 14591, 31871, 65031, 65283, 65281, 65280, 65280, 65409, 65473, 65511},
					{0, 10240, 20480, 24576, 26624, 17408, 512, 256, 128, 88, 32, 86, 72, 20, 16, 0}
				},
				0,
				0
			},

			{
				/* 5 - hourglass */
				{	{0, 0, 0, 34785, 50115, 61455, 61455, 63519, 63519, 61839, 61455, 49155, 32769, 0, 0, 0},
					{0, 32766, 16386, 12300, 2064, 1440, 1440, 576, 576, 1056, 1440, 3024, 14316, 16386, 32766, 0}
				},
				8,
				7
			},

			{
				/* 6 - TTHand */
				{	{62463, 57855, 57855, 57855, 57471, 49167, 32769, 0, 0, 0, 0, 32768, 49152, 57344, 61441, 61443},
					{3072, 4608, 4608, 4608, 4992, 12912, 21070, 36937, 36873, 36865, 32769, 16385, 8193, 4097, 2050, 4092}
				},
				4,
				0
			},

			{
				/* 7- Mark's crosshairs */
				{	{65535, 65151, 65151, 65151, 65151, 0, 65151, 65151, 65151, 65151, 65535, 65535, 65535, 65535, 65535, 65535},
					{0, 384, 384, 384, 384, 65535, 384, 384, 384, 384, 0, 0, 0, 0, 0, 0}
				},
				8,
				5
			},

			{
				/* 8- I-beam. */
				{	{65535, 65535, 63631, 63503, 63503, 65087, 65087, 65087, 65087, 65087, 63503, 63503, 63631, 65535, 65535, 65535},
					{0, 0, 0, 864, 128, 128, 128, 128, 128, 128, 128, 864, 0, 0, 0, 0}
				},
				8,
				7
			},

			{
				/* 9- Question mark. */
				{	{511, 1023, 2047, 31, 15, 8199, 32647, 65415, 63503, 61471, 61503, 61695, 63999, 63999, 61695, 61695},
					{65024, 33792, 34816, 34784, 40976, 57224, 32840, 72, 1936, 2080, 2496, 2304, 1536, 1536, 2304, 3840}
				},
				0,
				0
			}
		
	};

	const Common::String lads[17] = {
			"Avalot", "Spludwick", "Crapulus", "Dr. Duck", "Malagauche", "Friar Tuck",
			"Robin Hood", "Cwytalot", "du Lustie", "the Duke of Cardiff", "Dogfood",
			"A trader", "Ibythneth", "Ayles", "Port", "Spurge", "Jacques"
	};

	const Common::String lasses[4] =
	{"Arkata", "Geida", "±", "the Wise Woman"};

	const char ladchar[] = "ASCDMTRwLfgeIyPu";

	const char lasschar[] = "kG±o";

	const int16 numtr = 2; /* current max no. of sprites */

	const bool a_thing = true;
	const bool a_person = false; /* for Thinkabout */

	/* Magic/portal commands are */

	/*N*/ const int16 nix = 0; /* ignore it if this line is touched */
	/*B*/ const int16 bounces = 1; /* bounce off this line. Not valid for portals. */
	/*E*/ const int16 exclaim = 2; /* put up a chain of scrolls */
	/*T*/ const int16 transport = 3; /* enter new room */
	/*U*/ const int16 unfinished = 4; /* unfinished connection */
	/*S*/ const int16 special = 5; /* special function. */
	/*O*/ const int16 mopendoor = 6; /* opening door. */

	/* These following constants should be included in CFG when it's written. */

	const bool slow_computer = false; /* stops walking when mouse touches toolbar */

	/* --- */

	const int16 border = 1; /* size of border on shadowboxes */

	const int32 pagetop = 81920;

	const int16 up = 0;
	const int16 right = 1;
	const int16 down = 2;
	const int16 left = 3;
	const int16 ur = 4;
	const int16 dr = 5;
	const int16 dl = 6;
	const int16 ul = 7;
	const int16 stopped = 8;

	const int16 walk = 3;
	const int16 run = 5;

	/* Art gallery at 2,1; notice about this at 2,2. */

	const int32 catamap[8][8] = {
		/* Geida's room */
			/*  1     2			3   | 4     5		6		7     8*/
			{0x204, 0x200, 0xd0f0, 0xf0ff, 0xff, 0xd20f, 0xd200, 0x200},
			{0x50f1, 0x20ff, 0x2ff, 0xff, 0xe0ff, 0x20ff, 0x200f, 0x7210},
			{0xe3f0, 0xe10f, 0x72f0, 0xff, 0xe0ff, 0xff, 0xff, 0x800f},
			{0x2201, 0x2030, 0x800f, 0x220, 0x20f, 0x30, 0xff, 0x23f}, /* >> Oubliette */
			{0x5024, 0xf3, 0xff, 0x200f, 0x22f0, 0x20f, 0x200, 0x7260},
			{0xf0, 0x2ff, 0xe2ff, 0xff, 0x200f, 0x50f0, 0x72ff, 0x201f},
			{0xf6, 0x220f, 0x22f0, 0x30f, 0xf0, 0x20f, 0x8200, 0x2f0}, /* <<< In here */
			{0x34, 0x200f, 0x51f0, 0x201f, 0xf1, 0x50ff, 0x902f, 0x2062}
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

	const bool demo = false; /* If this is true, we're in a demo of the game. */

	const char spludwick_order[3] = {onion, ink, mushroom};
	 
	const quasiped_type quasipeds[16] = {
		{2, lightgray, 19, brown, pdogfood},                       /* A: Dogfood (screen 19). */
		{3, green,     19, white, pibythneth},                         /* B: Ibythneth (screen 19). */
		{3, white,     1, magenta, parkata},                        /* C: Arkata (screen 1). */
		{3, black,     23, red, 177},                         /* D: Hawk (screen 23). */
		{3, lightgreen, 50, brown, ptrader},                        /* E: Trader (screen 50). */
		{6, yellow,    42, red, pavalot},                           /* F: Avvy, tied up (scr.42) */
		{2, blue,      16, white, payles},                         /* G: Ayles (screen 16). */
		{2, brown,     7, white, pjacques},                          /* H: Jacques (screen 7). */
		{2, lightgreen, 47, green, pspurge},                        /* I: Spurge (screen 47). */
		{3, yellow,    47, red, pavalot},                           /* J: Avalot (screen 47). */
		{2, lightgray, 23, black, pdulustie},                         /* K: du Lustie (screen 23). */
		{2, yellow,    27, red, pavalot},                           /* L: Avalot (screen 27). */
		{3, white,     27, red, 177},                         /* M: Avaroid (screen 27). */
		{4, lightgray, 19, darkgray, pmalagauche},                        /*N: Malagauche (screen 19). */
		{5, lightmagenta, 47, red, pport},                           /* O: Port (screen 47). */
		{2, lightgreen, 51, darkgray, pdrduck}
	};                         /*P: Duck (screen 51). */

	const int16 lower = 0;
	const int16 same = 1;
	const int16 higher = 2;

	const char keys[] = "QWERTYUIOP[]";
	const uint16 notes[12] =
	{196, 220, 247, 262, 294, 330, 350, 392, 440, 494, 523, 587};

	const tunetype tune = {
			higher, higher, lower, same, higher, higher, lower, higher, higher, higher,
			lower, higher, higher,
			same, higher, lower, lower, lower, lower, higher, higher, lower, lower, lower,
			lower, same, lower, higher, same, lower, higher
	};

	/* special run-time errors */

	const int16 runerr_getset_overflow = 50;


	///////////////////////////// VARIABLES ///////////////////////////// 

	byte interrogation = 0;
	/* If this is greater than zero, the next line you type is stored in
	   the DNA in a position dictated by the value. If a scroll comes up,
	   or you leave the room, it's automatically set to zero. */

	byte whereis[29] = {
		/* The Lads */
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
	}; /* The Wise Woman. */



	Common::String current;
	byte curpos;
	bool cursoron;
	/* previous:^previoustype;*/
	Common::String last;
	dnatype dna;
	linetype lines[50]; /* For Also. */
	int16 c;
	//registers r; // http://www.freepascal.org/docs-html/rtl/dos/registers.html
	enum { m_no , m_yes , m_virtual } visible;
	bool dropsok, screturn, soundfx, cheat;
	uint16 mx, my; /* mouse x & y now */
	uint16 mpx, mpy; /* mouse x & y when pressed */
	uint16 mrx, mry; /* mouse x & y when released */
	byte mpress, mrelease; /* times left mouse button has been pressed/released */
	byte keystatus; /* Mouse key status */
	Common::String un[10];
	byte unn;
	Common::String mousetext;
	/* which:array[0..5] of byte;*/
	void *p;
	bool weirduint16;
	byte to_do;
	bool lmo, mousemade;
	Common::String scroll[15];
	byte scrolln, score, whichwas;
	char thinks;
	bool thinkthing;

	/* pp:array[1..1000] of postype;
	bb:array[1..9000] of byte;*/
	uint16 pptr, bptr;
	int16 ppos[1][2];
	uint16 pozzes[24];
	byte anim;
	void *copier;
	int16 talkx, talky;
	byte talkb, talkf;
	byte scrollbells; /* no. of times to ring the bell */
	bool ontoolbar, seescroll;

	char objlist[10];
	void * digit[10];
	void *rwlite[9];
	byte oldrw;
	Common::String lastscore;
	byte cmp; /* current mouse-void **/
	Common::String verbstr; /* what you can do with your object. :-) */

	Common::String * also[31][2];
	pedtype peds[15];
	magictype magics[15];
	magictype portals[7];
	fieldtype fields[30];
	byte numfields;
	Common::String flags;
	Common::String listen;

	uint16 oh, onh, om, h, m, s, s1;

	Common::String atkey; /* For XTs, set to "alt-". For ATs, set to "f1". */

	byte cp, ledstatus, defaultled;
	raw little;
	bool quote; /* 66 or 99 next? */
	bool alive;
	char buffer[2000];
	uint16 bufsize;

	byte oldjw; /* Old joystick-way */
	controllers ctrl;

	int16 underscroll; /* Y-coord of just under the scroll text. */

	/* TSkellern is only temporary, and I'll replace it
	with a local version when it's all fixed up. */

	/* tskellern:int32 absolute $0:244; { Over int $61 }*/

	bool ddmnow; /* Kludge so we don't have to keep referring to Dropdown */
	Common::String roomname; /* Name of this room */

	//text logfile; // http://wiki.freepascal.org/Text
	bool logging, log_epson;

	bool cl_override;

	byte locks; /*ABSOLUTE $40:$17;*/

	Common::String subject; /* What you're talking to them about. */
	byte subjnumber; /* The same thing. */

	bool keyboardclick; /* Is a keyboard click noise wanted? */

	char him, her, it;
	int32 roomtime; /* Set to 0 when you enter a room, added to in every loop.*/

	bool after_the_scroll;

	/* For the demo: */
	demo_type demo_rec;
	//file<demo_type> demofile; // http://www.freepascal.org/docs-html/ref/refsu21.html#x45-520003.3.4

	char last_person; /* Last person to have been selected using the People
						menu. */

	bool doing_sprite_run; /* Only set to True if we're doing a sprite_run
	at this moment. This stops the trippancy system from moving any of the
	sprites. */

	vmctype vmc;
	Common::String filetoload;

	bool holdthedawn; /* If this is true, calling Dawn will do nothing.
	It's used, for example, at the start, to stop Load from dawning. */

	uint16 storage_seg, storage_ofs; /* Seg and ofs of the Storage area. */
	uint16 skellern; /* Offset of the timer variable - 1 more than storage_OFS */
	bool reloaded; /* Is this NOT the primary loading? */

	bool super_was_virtual, super_was_off; /* Used by Super_Off and Super_On */

	Common::String enid_filename;

	joysetup js;
	uint16 cxmin, cxmax, cymin, cymax;
	bool use_joy_a;



	///////////////////////////// FUNCTIONS ///////////////////////////// 

	void newpointer(byte m);

	void wait();    /* makes hourglass */

	void on();

	void off();

	void on_virtual();

	void off_virtual();

	void xycheck();

	void hopto(int16 x, int16 y); /* Moves mouse pointer to x,y */

	void check();

	void note(uint16 hertz);

	void blip();

	Common::String strf(int32 x);

	void shbox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String t);

	void newgame();

	void click();

	void slowdown();

	bool flagset(char x);

	void force_numlock();

	bool pennycheck(uint16 howmuchby);

	Common::String getname(byte whose);

	char getnamechar(byte whose);

	Common::String get_thing(byte which);

	char get_thingchar(byte which);

	Common::String get_better(byte which);

	Common::String f5_does();

	void plot_vmc(int16 xx, int16 yy, byte page_);

	void wipe_vmc(byte page_);

	void setup_vmc();

	void clear_vmc();

	void load_a_mouse(byte which);

	void background(byte x);

	void hang_around_for_a_while();

	void super_off();

	void super_on();

	bool mouse_near_text();



	} // End of namespace Gyro

} // End of namespace Avalanche

#endif // GYRO2_H
