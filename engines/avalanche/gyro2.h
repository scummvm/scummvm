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

/* GYRO		It all revolves around this bit! */

#ifndef GYRO2_H
#define GYRO2_H

#include "common/str.h"
#include "common/scummsys.h"
#include "common/file.h"

#include "graphics/surface.h"

#include "avalanche/roomnums.h"
#include "avalanche/color.h"

namespace Avalanche {
class AvalancheEngine;


static const byte numobjs = 18; /* always preface with a # */
static const int16 maxobjs = 12;  /* carry limit */

static const int16 numlockCode = 32;  /* Code for Num Lock */
static const int16 mouse_size = 134;

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
	byte wearing; /* what you're wearing */
	byte swore; /* number of times you've sworn */
	byte saves; /* number of times this game has been saved */
	byte rooms[100]; /* Add one to each every time you enter a room */
	byte alcohol; /* Your blood alcohol level. */
	byte playednim; /* How many times you've played Nim. */
	bool wonnim; /* Have you *won* Nim? (That's harder.) */
	byte winestate; /* 0=good (Notts), 1=passable(Argent) ... 3=vinegar.*/
	bool cwytalot_gone; /* Has Cwytalot rushed off to Jerusalem yet?*/

	byte pass_num; /* Number of the passw for this game. */
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

	byte box_contents; /* 0 = money (sixpence), 254 = empty, any
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

	uint32 total_time; /* Your total time playing this game, in ticks.*/

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


struct PointType {
	int16 x, y;
};

struct vmctype { /* Virtual Mouse Cursor */
	byte *andpic, *xorpic;
	byte *backpic[2];
	PointType wherewas[2];
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

class Gyro {
public:
	static const char *vernum;
	static const char *copyright;
	static const int16 thisvercode = 130;
	/* as "vernum", but numerically & without the ".". */
	static const int16 thisgamecode = 2; /* Avalot's code number */

	/* Objects you can hold: */
	static const char wine = 1;
	static const char money = 2;
	static const char bodkin = 3;
	static const char potion = 4;
	static const char chastity = 5;
	static const char bolt = 6;
	static const char crossbow = 7;
	static const char lute = 8;
	static const char badge = 9;
	static const char mushroom = 10;
	static const char key = 11;
	static const char bell = 12;
	static const char prescription = 13;
	static const char pen = 14;
	static const char ink = 15;
	static const char clothes = 16;
	static const char habit = 17;
	static const char onion = 18;

	/* People who hang around this game. */

	/* Boys: */
	static const uint16 pavalot = 150;
	static const uint16 pspludwick = 151;
	static const uint16 pcrapulus = 152;
	static const uint16 pdrduck = 153;
	static const uint16 pmalagauche = 154;
	static const uint16 pfriartuck = 155;
	static const uint16 probinhood = 156;
	static const uint16 pcwytalot = 157;
	static const uint16 pdulustie = 158;
	static const uint16 pduke = 159;
	static const uint16 pdogfood = 160;
	static const uint16 ptrader = 161;
	static const uint16 pibythneth = 162;
	static const uint16 payles = 163;
	static const uint16 pport = 164;
	static const uint16 pspurge = 165;
	static const uint16 pjacques = 166;

	/* Girls: */
	static const uint16 parkata = 175;
	static const uint16 pgeida = 176;
	static const uint16 pwisewoman = 178;

	static const int16 xw = 30;
	static const int16 yw = 36; /* x width & y whatsit */

	static const int16 margin = 5;

	static const mp mps[9];

	static const Common::String lads[17];

	static const Common::String lasses[4];

	static const char ladchar[];

	static const char lasschar[];

	static const int16 numtr = 2; /* current max no. of sprites */

	static const bool a_thing = true;
	static const bool a_person = false; /* for Thinkabout */

	/* Magic/portal commands are */

	/*N*/ static const int16 nix = 0; /* ignore it if this line is touched */
	/*B*/ static const int16 bounces = 1; /* bounce off this line. Not valid for portals. */
	/*E*/ static const int16 exclaim = 2; /* put up a chain of scrolls */
	/*T*/ static const int16 transport = 3; /* enter new room */
	/*U*/ static const int16 unfinished = 4; /* unfinished connection */
	/*S*/ static const int16 special = 5; /* special function. */
	/*O*/ static const int16 mopendoor = 6; /* opening door. */

	/* These following static constants should be included in CFG when it's written. */

	static const bool slow_computer = false; /* stops walking when mouse touches toolbar */

	/* --- */

	static const int16 border = 1; /* size of border on shadowboxes */

	static const int32 pagetop = 81920;

	static const int16 up = 0;
	static const int16 right = 1;
	static const int16 down = 2;
	static const int16 left = 3;
	static const int16 ur = 4;
	static const int16 dr = 5;
	static const int16 dl = 6;
	static const int16 ul = 7;
	static const int16 stopped = 8;

	static const int16 walk = 3;
	static const int16 run = 5;

	/* Art gallery at 2,1; notice about this at 2,2. */

	static const int32 catamap[8][8];

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

	static const bool demo = false; /* If this is true, we're in a demo of the game. */

	static const char spludwick_order[3];
	 
	static const quasiped_type quasipeds[16];

	static const int16 lower = 0;
	static const int16 same = 1;
	static const int16 higher = 2;

	static const char keys[];
	static const uint16 notes[12];

	static const tunetype tune;

	/* special run-time errors */

	static const int16 runerr_getset_overflow = 50;






	byte interrogation;
	/* If this is greater than zero, the next line you type is stored in
		the DNA in a position dictated by the value. If a scroll comes up,
		or you leave the room, it's automatically set to zero. */

	static byte whereis[29];

	bool oncandopageswap; 
	/* Variable static constant for overriding the ability of On to switch pages.
		* You may know better than On which page to switch to. */



	/* previous:^previoustype;*/
	dnatype dna;
	byte lineNum; // Number of lines.
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
	bool weirdword;
	byte to_do;
	bool lmo, mousemade;
	Common::String scroll[15];
	byte scrolln, score, whichwas;
	byte thinks;
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
	bool ontoolbar, seescroll; // TODO: maybe this means we're interacting with the toolbar / a scroll?

	char objlist[10];

	::Graphics::Surface digit[10]; // digitsize and rwlitesize are defined in Lucerna::load_digits() !!!
	::Graphics::Surface rwlite[9]; // Maybe it will be needed to move them to the class itself instead.
	// Called .free() for them in ~Gyro().

	byte oldrw;
	int8 lastscore[3];
	byte cmp; /* current mouse-void **/
	Common::String verbstr; /* what you can do with your object. :-) */

	Common::String *also[31][2];
	pedtype peds[15];
	magictype magics[15];
	magictype portals[7];
	fieldtype fields[30];
	byte numfields;
	Common::String flags;
	Common::String listen;

	uint16 oh, onh, om, hour, minutes, seconds;

	Common::String atkey; /* For XTs, set to "alt-". For ATs, set to "f1". */

	byte cp, ledstatus, defaultled;
	fontType characters;
	bool alive;
	byte buffer[2000];
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
	bool log_epson;

	bool cl_override;

	byte locks; /*ABSOLUTE $40:$17;*/

	Common::String subject; /* What you're talking to them about. */
	byte subjnumber; /* The same thing. */

	bool keyboardclick; /* Is a keyboard click noise wanted? */

	byte him, her, it;
	int32 roomtime; /* Set to 0 when you enter a room, added to in every loop.*/

	bool after_the_scroll;

	/* For the demo: */
	demo_type demo_rec;
	Common::File demofile; // of demo_type
	Common::DumpFile demofile_save; // uruk added it - first use located in constructor of Basher

	byte last_person; /* Last person to have been selected using the People
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
	bool isLoaded; // Is it a loaded gamestate?

	bool super_was_virtual, super_was_off; /* Used by Super_Off and Super_On */

	Common::String enid_filename;

	joysetup js;
	uint16 cxmin, cxmax, cymin, cymax;
	bool use_joy_a;


	Gyro(AvalancheEngine *vm);

	~Gyro();

	Common::String strf(int32 x);

	void newpointer(byte id);

	void wait();    // Makes hourglass.

	void on();

	void off();

	void xycheck(); // Only updates mx & my, not all other mouse vars.

	void check();

	void note(uint16 hertz);

	void blip();

	void shbox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String t);

	void newgame(); // This sets up the DNA for a completely new game.

	void click(); // "Audio keyboard feedback"

	void slowdown();

	bool flagset(char x);

	void force_numlock();

	bool pennycheck(uint16 howmuchby);

	Common::String getname(byte whose);

	byte getnamechar(byte whose);

	Common::String get_thing(byte which);

	char get_thingchar(byte which);

	Common::String get_better(byte which);

	Common::String f5_does(); // This procedure determines what f5 does.

	void load_a_mouse(byte which);

	void background(byte x);

	void hang_around_for_a_while();

	bool mouse_near_text();

private:
	AvalancheEngine *_vm;
	
	static const Common::String things[numobjs];

	static const char thingchar[];

	static const Common::String better[numobjs];

	static const char betterchar[];

	void shadow(int16 x1, int16 y1, int16 x2, int16 y2, byte hc, byte sc);
};

} // End of namespace Avalanche

#endif // GYRO2_H
