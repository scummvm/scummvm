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
#include "common/events.h"
#include "common/random.h"
#include "engines/engine.h"

#include "audio/mixer.h"

#include "graphics/transparent_surface.h"

namespace Griffon {

class Console;

#define kSoundHandles 16
#define kMaxNPC      32
#define kMaxFloat    32
#define kMaxSpell    32

// spells
enum {
	kSpellIce = 0,
	kSpellSteel,
	kSpellWood,
	kSpellRock,
	kSpellFire
};

// inventory items
enum {
	kInvFlask = 0,
	kInvDoubleFlask,
	kInvShock,
	kInvNormalKey,
	kInvMasterKey
};

enum {
	kSndBite = 0,
	kSndCrystal,
	kSndDoor,
	kSndEnemyHit,
	kSndIce,
	kSndLever,
	kSndLightning,
	kSndMetalHit,
	kSndPowerUp,
	kSndRocks,
	kSndSwordHit,
	kSndThrow,
	kSndChest,
	kSndFire,
	kSndBeep
};

//  in game scripts
enum {
	kScriptMasterKey = 2, 		// find master key
	kScriptFindCtystal = 3,		// find crystal
	kScriptFindShield = 4,		// find shield - obj 8
	kScriptFindSword = 5,		// find sword - obj 9
	kScriptKeyChest = 6,		// regular key chest
	kScriptBlueFlask = 7,		// blue flask
	kScriptGardenMasterKey = 8,	// garden's master key
	kScriptLightningBomb = 9,	// lightning bomb
	kScriptBlueFlaskChest = 10,	// blue flask chest
	kScriptLightningChest = 11,	// lightning chest
	kScriptArmourChest = 12,	// armour chest
	kScriptCitadelMasterKey = 13,	// citadel master key
	kScriptEndOfGame = 14,		// end of game
	kScriptGetSword3 = 15,		// get sword3
	kScriptShield3 = 16,		// shield3
	kScriptArmour3 = 17,		// armour3
	kScriptKeyChest1 = 20,		// key chest 1
	kScriptLever = 60			// lever
};


#define kEpsilon 0.001

struct Player {
	float   px;
	float   py;
	float   opx;
	float   opy;
	int     walkdir;
	float   walkframe;
	float   walkspd;
	float   attackframe;
	float   attackspd;

	int hp;
	int maxhp;
	float hpflash;
	int hpflashb;
	int level;
	int maxlevel;
	int sword;
	int shield;
	int armour;
	int foundspell[5];
	float spellcharge[5];
	int inventory[5];
	float attackstrength;
	float spellstrength;
	int spelldamage;
	int sworddamage;

	int exp;
	int nextlevel;

	int pause;

	float itemselshade;
	int ysort;
};

struct BodySection {
	float   x;
	float   y;
	int parentID;
	int isbase;
	int sprite;
	int bonelength; // the 'bone' that connects the body sections
};

struct NPC {
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
	BodySection bodysection[31];
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

struct Spell {
	int spellnum;
	float   homex;
	float   homey;
	float   enemyx;
	float   enemyy;

	float   frame;

	int damagewho;  // 0 = npc, 1 = player

	// for earthslide
	float   rocky[9]; // CHECKME: Looks unused
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

struct AnimSet {
	int x;  // xyloc on spriteimageset
	int y;
	int xofs;   // the actual place to paste the sprite in reference to the bodypart loc on screen
	int yofs;
	int w;  // w/h of the sprite in the imageset
	int h;
};

struct DataChunk {
	byte *data;
	int size;
};

class GriffonEngine : public Engine {
public:
	GriffonEngine(OSystem *syst);
	~GriffonEngine();

	virtual Common::Error run();

private:
	Common::RandomSource *_rnd;
	bool _shouldQuit;

	Console *_console;

private:

	// combat.cpp
	void attack();
	void castSpell(int spellnum, float homex, float homey, float enemyx, float enemyy, int damagewho);
	void checkHit();
	void damageNPC(int npcnum, int damage, int spell);
	void damagePlayer(int damage);

	// cutscenes.cpp
	void showLogos();
	void intro();
	void endOfGame();
	void theEnd();

	// dialogs.cpp
	void title(int mode);
	void configMenu();
	void saveLoadNew();

	// draw.cpp
	void drawAnims(int Layer);
	void drawHud();
	void drawNPCs(int mode);
	void drawOver(int modx, int mody);
	void drawPlayer();
	void drawView();
	void swash();

	float RND();

	void addFloatIcon(int ico, float xloc, float yloc);
	void addFloatText(const char *stri, float xloc, float yloc, int col);
	void checkInputs();
	void checkTrigger();
	void eventText(const char *stri);
	void handleWalking();
	void loadMap(int mapnum);
	void newGame();
	void mainLoop();
	void processTrigger(int trignum);
	void updateAnims();
	void updateY();
	void updateMusic();
	void updateNPCs();
	void updateSpells();
	void updateSpellsUnder();

	void initialize();
	void drawLine(Graphics::TransparentSurface *buffer, int x1, int y1, int x2, int y2, int col);
	void loadAnims();
	void loadFont();
	void loadItemImgs();
	void loadTiles();
	void loadTriggers();
	void loadObjectDB();
	void drawString(Graphics::TransparentSurface *buffer, const char *stri, int xloc, int yloc, int col);
	void drawProgress(int w, int wm);
	void setupAudio();
	void updateEngine();

	Common::String makeSaveGameName(int slot);
	int loadState(int slotnum);
	int loadPlayer(int slotnum);
	int saveState(int slotnum);

	void setChannelVolume(int channel, int volume);
	int getSoundHandle();
	int playSound(DataChunk *chunk, bool looped = false);
	void pauseSoundChannel(int channel);
	void haltSoundChannel(int channel);
	void resumeSoundChannel(int channel);
	bool isSoundChannelPlaying(int channel);

	Graphics::TransparentSurface *loadImage(const char *name, bool colorkey = false);

private:
	Graphics::TransparentSurface *_video, *_videobuffer, *_videobuffer2, *_videobuffer3;

	// system
	Graphics::TransparentSurface *_titleimg, *_titleimg2, *_inventoryimg;
	Graphics::TransparentSurface *_logosimg, *_theendimg;
	Common::Event _event;

	Graphics::TransparentSurface *_mapbg, *_clipbg, *_clipbg2;
	unsigned int _clipsurround[4][4];

	float _animspd; // CHECKME: it seems to always be 0.5
	int _rampdata[40][24];

	int _curmap;
	Graphics::TransparentSurface *_fontchr[224][5]; // 256 - 32
	Graphics::TransparentSurface *_itemimg[21], *_windowimg;
	Graphics::TransparentSurface *_spellimg;

	int _itemselon, _curitem, _itemticks;
	float _itemyloc;
	int _selenemyon, _curenemy;
	bool _forcepause;
	bool _roomlock; // set to disable any room jumps while in the room
	int _scriptflag[100][10], _saveslot;  // script, flag

	// timer related - move to local later
	int _ticks, _tickspassed, _nextticks;
	float _fp, _fps, _fpsr; // CHECKME: _fp and _fps seems to be integers
	int _secsingame, _secstart;

	Graphics::TransparentSurface *mapimg[4];

	Common::Rect rcSrc, rcDest;

	// -----------special case
	int dontdrawover;   // used in map24 so that the candles don't draw over the boss, default set to 0

	// saveload info
	Graphics::TransparentSurface *saveloadimg;


	// post info
	float postinfo[21][3];
	int nposts;

	// cloud info
	Graphics::TransparentSurface *cloudimg;
	float clouddeg;
	int cloudson;

	// spell info
	Spell spellinfo[kMaxSpell];

	// player info
	bool movingup, movingdown, movingleft, movingright;
	Player _player;
	bool attacking;
	Player _playera;
	int _asecstart;

	// tile info
	Graphics::TransparentSurface *_tiles[4];
	int _tileinfo[3][40][24][3]; // maplayer, x, y, tiledata (tile, tilelayer)

	// animation info
	Graphics::TransparentSurface *_anims[100];
	// id number 0&1 = players
	Graphics::TransparentSurface *_animsa[100];
	// attack anims
	float _playerattackofs[4][16][3];
	// [dir] [frame] [x,y ofs, completed(0/1)]

	float _floattext[kMaxFloat][4]; // [id] [framesleft, x, y, col]
	char *_floatstri[kMaxFloat];
	float  _floaticon[kMaxFloat][4]; // [id] [framesleft, x, y, ico]

	// special for animset2
	AnimSet _animset2[7], _animset9[7];

	// object info
	float _objectFrame[256][2];
	int _lastObj;
	// frame!, curframe
	int _objectInfo[33][6];
	// nframes,xtiles,ytiles,speed,type,script, update?
	int _objectTile[33][9][3][3][2];
	// [objnum] [frame] [x] [y] [tile/layer]
	int _objectMap[21][15];

	int _objmapf[1000][21][15];
	// [mapnum] x, y  set to 1 to make this objmap spot stay at -1

	// trigger info
	int _triggers[10000][9];
	// [map#][index], [var]
	// map#,x,y
	int _triggerloc[320][240], _ntriggers;

	// npc info
	NPC _npcinfo[kMaxNPC];
	int _lastnpc;

	// music info
	DataChunk *_mgardens, *_mgardens2, *_mgardens3, *_mgardens4, *_mboss, *_mmenu, *_mendofgame;
	int _musicchannel, _menuchannel;
	bool _ptown, _pacademy, _pcitadel; // CHECKME: They look unused.
	bool _pboss;
	bool _pgardens;
	int _loopseta;

	DataChunk *_sfx[21];
	Audio::SoundHandle _handles[kSoundHandles];
	Audio::Mixer *_mixer;

	// set to 1 for normal key, set to 2 for master, set to 0 if unlocked
	int _roomLocks[201], _lockType;
	int _roomToUnlock;

	bool _canUseKey;
	bool _saidLocked;
	bool _saidJammed;


	// ysort
	int _ysort[2401], _lasty, _firsty;

	bool _pmenu;
};

}

#endif
