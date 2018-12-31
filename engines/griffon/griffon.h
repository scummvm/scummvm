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
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#ifndef GRIFFON_H
#define GRIFFON_H

#include "common/scummsys.h"
#include "common/random.h"
#include "engines/engine.h"

#include "graphics/transparent_surface.h"

namespace Griffon {

#define kMaxNPC      32
#define kMaxFloat    32
#define kMaxSpell    32

// spells
#define ice     0
#define steel       1
#define wood        2
#define rock        3
#define fire        4

// inventory items
#define INV_FLASK   0
#define INV_DOUBLEFLASK 1
#define INV_SHOCK   2
#define INV_NORMALKEY   3
#define INV_MASTERKEY   4

#define sndbite     0
#define sndcrystal  1
#define snddoor     2
#define sndenemyhit 3
#define sndice      4
#define sndlever    5
#define sndlightning    6
#define sndmetalhit 7
#define sndpowerup  8
#define sndrocks    9
#define sndswordhit 10
#define sndthrow    11
#define sndchest    12
#define sndfire     13
#define sndbeep     14

struct PLAYERTYPE {
	float   px;
	float   py;
	float   opx;
	float   opy;
	int walkdir;
	float   walkframe;
	float   walkspd;
	float   attackframe;
	float   attackspd;

	int hp;
	int maxhp;
	float   hpflash;
	int hpflashb;
	int level;
	int maxlevel;
	int sword;
	int shield;
	int armour;
	int foundspell[5];
	float   spellcharge[5];
	int inventory[5];
	float   attackstrength;
	float   spellstrength;
	int spelldamage;
	int sworddamage;

	int exp;
	int nextlevel;

	int pause;

	float   itemselshade;
	int ysort;
};

struct BODYSECTIONTYPE {
	float   x;
	float   y;
	int parentID;
	int isbase;
	int sprite;
	int bonelength; // the 'bone' that connects the body sections
};

struct NPCTYPE {
	float   x;
	float   y;
	int spriteset;
	int x1;     // patrol area
	int y1;
	int x2;
	int y2;
	int attitude;
	int hp;

	int maxhp;
	int item1;
	int item2;
	int item3;
	int script;
	float   frame;
	float   frame2;     // end boss specific
	int cframe;
	int onmap;      // is this npc set to be genned in the mapfile

	int ticks;
	int pause;
	int shake;

	int movementmode;
	int walkdir;
	float   walkspd;
	int movingdir;
	int moving;

	int attacking;
	float   attackframe;
	int cattackframe;
	float   attackspd;
	int attackdelay;
	int attacknext;
	int attackattempt;

	int spelldamage;
	int attackdamage;


	// one wing and firehydra specific
	BODYSECTIONTYPE bodysection[31];
	float   swayangle;
	float   swayspd;
	float   headtargetx[4];
	float   headtargety[4];
	int castpause;

	// firehydra specific
	int attacknext2[4];
	int attacking2[4];
	int attackframe2[4];

	// dragon2 specific
	float   floating;
};

struct SPELLTYPE {
	int spellnum;
	float   homex;
	float   homey;
	float   enemyx;
	float   enemyy;

	float   frame;

	int damagewho;  // 0 = npc, 1 = player

	// for earthslide
	float   rocky[9];
	int rockimg[9];
	int rockdeflect[9];

	float   strength;

	// fire
	int legalive[5];

	// spell 6 specific
	float   fireballs[7][4];    // x,y,targetx, targety
	int nfballs;
	int ballon[7];

	int npc;
};

struct ANIMSET2TYPE {
	int x;  // xyloc on spriteimageset
	int y;
	int xofs;   // the actual place to paste the sprite in reference to the bodypart loc on screen
	int yofs;
	int w;  // w/h of the sprite in the imageset
	int h;
};

extern PLAYERTYPE player;
extern int curmap;
extern int scriptflag[100][10];
extern int objmapf[1000][21][15];
extern int roomlocks[201];
extern int secsingame, secstart;


class GriffonEngine : public Engine {
public:
	GriffonEngine(OSystem *syst);
	~GriffonEngine();

	virtual Common::Error run();

private:
	Common::RandomSource *_rnd;
	bool _shouldQuit;

private:
	void griffon_main();

	float RND();

	void game_addFloatIcon(int ico, float xloc, float yloc);
	void game_addFloatText(const char *stri, float xloc, float yloc, int col);
	void game_attack();
	void game_castspell(int spellnum, float homex, float homey, float enemyx, float enemyy, int damagewho);
	void game_checkhit();
	void game_checkinputs();
	void game_checktrigger();
	void game_configmenu();
	void game_damagenpc(int npcnum, int damage, int spell);
	void game_damageplayer(int damage);
	void game_drawanims(int Layer);
	void game_drawhud();
	void game_drawnpcs(int mode);
	void game_drawover(int modx, int mody);
	void game_drawplayer();
	void game_drawview();
	void game_endofgame();
	void game_eventtext(const char *stri);
	void game_handlewalking();
	void game_loadmap(int mapnum);
	void game_main();
	void game_newgame();
	void game_playgame();
	void game_processtrigger(int trignum);
	void game_saveloadnew();
	void game_showlogos();
	void game_swash();
	void game_theend();
	void game_title(int mode);
	void game_updanims();
	void game_updatey();
	void game_updmusic();
	void game_updnpcs();
	void game_updspells();
	void game_updspellsunder();

	void sys_initialize();
	void sys_line(Graphics::TransparentSurface *buffer, int x1, int y1, int x2, int y2, int col);
	void sys_LoadAnims();
	void sys_LoadFont();
	void sys_LoadItemImgs();
	void sys_LoadTiles();
	void sys_LoadTriggers();
	void sys_LoadObjectDB();
	void sys_print(Graphics::TransparentSurface *buffer, const char *stri, int xloc, int yloc, int col);
	void sys_progress(int w, int wm);
	void sys_setupAudio();
	void sys_update();
};

}

#endif
