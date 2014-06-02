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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_H
#define CGE2_H

#include "common/random.h"
#include "engines/engine.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "cge2/fileio.h"

namespace CGE2 {

class Vga;
class Sprite;
class MusicPlayer;
class Fx;
class Sound;
class Text;
struct HeroTab;
class V3D;
class V2D;
struct Dac;
class Spare;
class CommandHandler;
class InfoLine;
class Mouse;
class Keyboard;
class Talk;
class Hero;
class Bitmap;
class System;
class EventManager;

#define kScrWidth      320
#define kScrHeight     240
#define kScrDepth      480
#define kPanHeight     40
#define kWorldHeight   (kScrHeight - kPanHeight)
#define kMaxFile       128
#define kPathMax       128
#define kDimMax          8
#define kWayMax         10
#define kPocketMax       4
#define kCaveMax       100
#define kMaxPoint        4
#define kInfoX         160
#define kInfoY         -11
#define kInfoW         180

enum CallbackType {
	kNullCB = 0, kQGame, kMiniStep, kXScene, kSoundSetVolume
};

enum Action { kNear, kMTake, kFTake, kActions };

class Font {
	char _path[kPathMax];
	void load();
	CGE2Engine *_vm;
public:
	uint8  *_widthArr;
	uint16 *_pos;
	uint8  *_map;
	Font(CGE2Engine *vm, const char *name);
	~Font();
	uint16 width(const char *text);
	void save();
};

class CGE2Engine : public Engine {
private:
	uint32 _lastFrame, _lastTick;
	void tick();
public:
	CGE2Engine(OSystem *syst, const ADGameDescription *gameDescription);
	virtual bool hasFeature(EngineFeature f) const;
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual Common::Error run();

	bool showTitle(const char *name);
	void cge2_main();
	char *mergeExt(char *buf, const char *name, const char *ext);
	void inf(const char *text, bool wideSpace = false);
	void movie(const char *ext);
	void runGame();
	void loadGame();
	void loadScript(const char *fname);
	void loadSprite(const char *fname, int ref, int scene, V3D &pos);
	void badLab(const char *fn);
	void caveUp(int cav);
	void switchCave(int cav);
	void showBak(int ref);
	void loadTab();
	int newRandom(int range);
	void loadMap(int cav);
	void openPocket();
	void selectPocket(int n);
	void busy(bool on);
	void feedSnail(Sprite *spr, Action snq, Hero *hero);
	int freePockets(int sx);
	int findActivePocket(int ref);
	void pocFul();
	void killText();
	void mainLoop();
	void handleFrame();
	Sprite *locate(int ref);
	Sprite *spriteAt(int x, int y);
	bool isHero(Sprite *spr);
	void loadUser();
	void checkSaySwitch();
	void qGame();
	void loadPos();
	void releasePocket(Sprite *spr);
	void switchHero(bool sex);
	void optionTouch(int opt, uint16 mask);
	void offUse();

	void setEye(const V3D &e);
	void setEye(const V2D& e2, int z = -kScrWidth);
	void setEye(const char *s);

	int number(char *s);
	char *token(char *s);
	char *tail(char *s);
	int takeEnum(const char **tab, const char *text);
	ID ident(const char *s);
	bool testBool(char *s);

	void snKill(Sprite *spr);
	void snHide(Sprite *spr, int val);
	void snMidi(int val);
	void snSetDlg(int clr, int set);
	void snMskDlg(int clr, int set);
	void snSeq(Sprite *spr, int val);
	void snRSeq(Sprite *spr, int val);
	void snSend(Sprite *spr, int val);
	void snSwap(Sprite *spr, int val);
	void snCover(Sprite *spr, int val);
	void snUncover(Sprite *spr, Sprite *spr2);
	void snFocus(int val);
	void snKeep(Sprite *spr, int val);
	void snGive(Sprite *spr, int val);
	void snGoto(Sprite *spr, int val);
	void snMove(Sprite *spr, V3D pos);
	void snSlave(Sprite *spr, int val);
	void snTrans(Sprite *spr, int val);
	void snPort(Sprite *spr, int val);
	void snMouse(int val);
	void snNNext(Sprite *spr, Action act, int val);
	void snRNNext(Sprite *spr, int val);
	void snRMTNext(Sprite *spr, int val);
	void snRFTNext(Sprite *spr, int val);
	void snRmNear(Sprite *spr);
	void snRmMTake(Sprite *spr);
	void snRmFTake(Sprite *spr);
	void snFlag(int ref, int val);
	void snSetRef(Sprite *spr, int val);
	void snBackPt(Sprite *spr, int val);
	void snFlash(int val);
	void snLight(int val);
	void snWalk(Sprite *spr, int val);
	void snReach(Sprite *spr, int val);
	void snSound(Sprite *spr, int wav);
	void snRoom(Sprite *spr, int val);
	void snGhost(Bitmap *bmp);
	
	void hide1(Sprite *spr);

	const ADGameDescription *_gameDescription;

	Common::RandomSource _randomSource;

	bool _quitFlag;
	Dac *_bitmapPalette;
	int _mode;
	bool _music;
	int _startupMode;
	int _now;
	bool _sex;
	int _mouseTop;
	bool _dark;
	int _waitSeq;
	int _waitRef;
	struct CommandStat {
		int *_wait;
		int _ref[2];
	} _commandStat;
	bool _taken;
	bool _endGame;
	bool _flag[4];

	ResourceManager *_resman;
	Vga *_vga;
	Sprite *_sprite;
	MusicPlayer *_midiPlayer;
	Fx *_fx;
	Sound *_sound;
	Text *_text;
	HeroTab *_heroTab[2];
	V3D *_eye;
	V3D *_eyeTab[kCaveMax];
	Spare *_spare;
	CommandHandler *_commandHandler;
	CommandHandler *_commandHandlerTurbo;
	InfoLine *_infoLine;
	Mouse *_mouse;
	Keyboard *_keyboard;
	Talk *_talk;
	V3D *_point[kMaxPoint];
	System *_sys;
	Sprite *_busyPtr;
	Sprite *_vol[2];
	EventManager *_eventManager;
	Sprite *_blinkSprite;
private:
	void init();
	void deinit();
};

} // End of namespace CGE2

#endif // CGE2_H
