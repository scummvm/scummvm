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

#ifndef GRIFFON_GRIFFON_H
#define GRIFFON_GRIFFON_H

#include "common/scummsys.h"
#include "common/error.h"
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
	kScriptFlask = 0,			// get flask
	kScriptMasterKey = 2, 		// find master key
	kScriptFindCrystal = 3,		// find crystal
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

// monsters
enum {
	kMonsterBabyDragon = 1, 	// baby dragon
 	kMonsterOneWing = 2,		// one wing
 	kMonsterBoss1 = 3, 			// boss 1
 	kMonsterBlackKnight = 4,	// black knight
 	kMonsterFireHydra = 5,		// fire hydra
 	kMonsterRedDragon = 6,		// red dragon
 	kMonsterPriest = 7,			// priest
 	kMonsterYellowDragon = 8,	// yellow fire dragon
 	kMonsterTwoWing = 9,		// two wing
	kMonsterDragon2 = 10,		// dragon2
	kMonsterFinalBoss = 11,		// final boss
	kMonsterBatKitty = 12		// bat kitty
};

// engine actions
enum GriffonActions {
	kGriffonLeft,
	kGriffonRight,
	kGriffonUp,
	kGriffonDown,
	kGriffonAttack,
	kGriffonInventory,
	kGriffonMenu,
	kGriffonConfirm,
	kGriffonCutsceneSpeedUp
};

#define kEpsilon 0.001

struct Player {
	float   px;
	float   py;
	float   opx;
	float   opy;
	int     walkDir;
	float   walkFrame;
	float   walkSpeed;
	float   attackFrame;
	float   attackSpeed;

	int hp;
	int maxHp;
	float hpflash;
	int hpflashb;
	int level;
	int maxLevel;
	int sword;
	int shield;
	int armour;
	int foundSpell[5];
	float spellCharge[5];
	int inventory[5];
	float attackStrength;
	float spellStrength;
	int spellDamage;
	int swordDamage;

	int exp;
	int nextLevel;

	int pause;

	float itemselshade;
	int ysort;

	void reset();
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
	float x;
	float y;
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
	float frame;
	float frame2;     // end boss specific
	int cframe;
	bool onMap;      // is this npc set to be genned in the mapfile

	int ticks;
	int pause;
	int shake;

	int movementmode;
	int walkdir;
	float walkspd;
	int movingdir;
	bool moving;

	bool attacking;
	float attackframe;
	int cattackframe;
	float attackspd;
	int attackdelay;
	int attacknext;
	int attackattempt;

	int spellDamage;
	int attackDamage;


	// one wing and firehydra specific
	BodySection bodysection[31];
	float swayAngle;
	float swaySpeed;
	float headTargetX[4];
	float headTargetY[4];
	int castPause;

	// firehydra specific
	int attacknext2[4];
	bool attacking2[4];
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

struct FloatTextStruct {
	float framesLeft;
	float x;
	float y;
	int col;
	char *text;
};

struct FloatIconStruct {
	float framesLeft;
	float x;
	float y;
	int ico;
};

struct AttackOffsetStruct {
	float x;
	float y;
	bool completed;
};

struct Config {
	bool music;
	int musicVol;
	bool effects;
	int effectsVol;
};

struct ObjectInfoStruct {
	int nFrames;
	int xTiles;
	int yTiles;
	int speed;
	int type;
	int script;
};

enum {
	kGameModeIntro,
	kGameModePlay,
	kGameModeNewGame,
	kGameModeLoadGame
};

class GriffonEngine : public Engine {
public:
	GriffonEngine(OSystem *syst);
	~GriffonEngine() override;

	Common::Error run() override;
	void syncSoundSettings() override;

private:
	Common::RandomSource *_rnd;
	bool _shouldQuit;
	int _gameMode;

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
	void renderSaveStates();

	// draw.cpp
	void drawAnims(int Layer);
	void drawHud();
	void drawNPCs(int mode);
	void drawOver(int modx, int mody);
	void drawPlayer();
	void drawView();
	void swash();

	// engine.cpp
	float RND();

	void mainLoop();
	void updateEngine();
	void newGame();

	// gfx.cpp
	void addFloatIcon(int ico, float xloc, float yloc);
	void addFloatText(const char *stri, float xloc, float yloc, int col);
	void eventText(const char *stri);
	void drawLine(Graphics::TransparentSurface *buffer, int x1, int y1, int x2, int y2, int col);
	void drawString(Graphics::TransparentSurface *buffer, const char *stri, int xloc, int yloc, int col);
	void drawProgress(int w, int wm);

	// input.cpp
	void checkInputs();
	void handleWalking();
	void checkTrigger();
	void processTrigger(int trignum);

	// logic.cpp
	void updateAnims();
	void updateY();
	void updateNPCs();
	void updateSpells();
	void updateSpellsUnder();

	// resources.cpp
	void initialize();
	Graphics::TransparentSurface *loadImage(const char *name, bool colorkey = false);
	void loadMap(int mapnum);
	void loadAnims();
	void loadFont();
	void loadItemImgs();
	void loadTiles();
	void loadTriggers();
	void loadObjectDB();

	// saveload.cpp
	Common::String getSaveStateName(int slot) const override;
	int loadPlayer(int slotnum);
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *file) override;
	Common::Error saveGameStream(Common::WriteStream *file, bool isAutosave) override;

	// sound.cpp
	void setChannelVolume(int channel, int volume);
	int getSoundHandle();
	int playSound(DataChunk *chunk, bool looped = false);
	void pauseSoundChannel(int channel);
	void haltSoundChannel(int channel);
	void resumeSoundChannel(int channel);
	bool isSoundChannelPlaying(int channel);
	void setupAudio();
	void updateMusic();

	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return _gameMode == kGameModePlay; }
	int getAutosaveSlot() const override { return 4; }
	bool hasFeature(EngineFeature f) const override;

private:
	Graphics::TransparentSurface *_video, *_videoBuffer, *_videoBuffer2, *_videoBuffer3;

	// system
	Graphics::TransparentSurface *_titleImg, *_titleImg2, *_inventoryImg;
	Graphics::TransparentSurface *_logosImg, *_theEndImg;
	Common::Event _event;

	Graphics::TransparentSurface *_mapBg, *_clipBg, *_clipBg2;
	unsigned int _clipSurround[4][4];

	float _animSpeed; // CHECKME: it seems to always be 0.5
	int _rampData[40][24];

	int _curMap;
	Graphics::TransparentSurface *_fontChr[224][5]; // 256 - 32
	Graphics::TransparentSurface *_itemImg[21], *_windowImg;
	Graphics::TransparentSurface *_spellImg;

	bool _itemSelOn;
	int _curItem, _itemTicks;
	float _itemyloc;
	bool _selEnemyOn;
	int _curEnemy;
	bool _forcePause;
	bool _roomLock; // set to disable any room jumps while in the room
	int _scriptFlag[100][10], _saveSlot;  // script, flag

	// timer related - move to local later
	int _ticks, _ticksPassed, _nextTicks;
	float _fp, _fps, _fpsr; // CHECKME: _fp and _fps seems to be integers
	int _secsInGame, _secStart;

	Graphics::TransparentSurface *mapImg[4];

	Common::Rect rcSrc, rcDest;

	// -----------special case
	bool _dontDrawOver;   // used in map24 so that the candles don't draw over the boss, default set to 0

	// saveload info
	Graphics::TransparentSurface *_saveLoadImg;

	// post info
	float _postInfo[21][3];
	int _postInfoNbr;

	// cloud info
	Graphics::TransparentSurface *_cloudImg;
	float _cloudAngle;
	int _cloudsOn;

	// spell info
	Spell _spellInfo[kMaxSpell];

	// player info
	Player _player;
	Player _playera;
	bool _movingUp, _movingDown, _movingLeft, _movingRight;
	bool _attacking;
	int _asecstart;

	// tile info
	Graphics::TransparentSurface *_tiles[4];
	int _tileinfo[3][40][24][3]; // maplayer, x, y, tiledata (tile, tilelayer)

	// animation info
	Graphics::TransparentSurface *_anims[100];
	// id number 0&1 = players
	Graphics::TransparentSurface *_animsAttack[100];
	// attack anims
	AttackOffsetStruct _playerAttackOfs[4][16];

	FloatTextStruct _floatText[kMaxFloat];
	FloatIconStruct _floatIcon[kMaxFloat];

	// special for animset2
	AnimSet _animSet2[7], _animSet9[7];

	// object info
	float _objectFrame[256][2];
	int _lastObj;
	// frame!, curframe
	ObjectInfoStruct _objectInfo[33];

	int _objectTile[33][9][3][3][2];
	// [objnum] [frame] [x] [y] [tile/layer]
	int _objectMap[21][15];

	int _objectMapFull[1000][21][15];
	// [mapnum] x, y  set to 1 to make this objmap spot stay at -1

	// trigger info
	int _triggers[10000][9];
	// [map#][index], [var]
	// map#,x,y
	int _triggerLoc[320][240], _triggerNbr;

	// npc info
	NPC _npcInfo[kMaxNPC];
	int _lastNpc;

	// music info
	DataChunk *_musicGardens1, *_musicGardens2, *_musicGardens3, *_musicGardens4, *_musicBoss, *_musicMenu, *_musicEndOfGame;
	int _musicChannel, _menuChannel;
	bool _playingBoss, _playingGardens;

	DataChunk *_sfx[15];
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

	Config config;
	void saveConfig();
};

}

#endif // GRIFFON_GRIFFON_H
