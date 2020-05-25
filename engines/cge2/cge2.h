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
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_CGE2_H
#define CGE2_CGE2_H

#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "common/system.h"
#include "cge2/fileio.h"
#include "cge2/console.h"
#include "audio/mixer.h"

struct ADGameDescription;

namespace CGE2 {

class Vga;
class Sprite;
class MusicPlayer;
class Fx;
class Sound;
class Text;
struct HeroTab;
class FXP;
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
class Font;
class Map;
struct SavegameHeader;

#define kScrWidth         320
#define kScrHeight        240
#define kScrDepth         480
#define kPanHeight         40
#define kWorldHeight      (kScrHeight - kPanHeight)
#define kMaxFile          128
#define kPathMax          128
#define kDimMax             8
#define kWayMax            10
#define kPocketMax          4
#define kSceneMax         100
#define kMaxPoint           4
#define kInfoX            160
#define kInfoY            -11
#define kInfoW            180
#define kPocketsWidth      59
#define kLineMax          512

#define kIntroExt         ".I80"
#define kTabName          "CGE.TAB"
#define kPocketFull       170
#define kGameFrameDelay   (750 / 50)
#define kGameTickDelay    (750 / 62)

#define kMusicRef         122
#define kPowerRef         123
#define kDvolRef          124
#define kMvolRef	      125
#define kBusyRef	      127

#define kOffUseCount      130
#define kOffUseText       131

#define kSysTimeRate        6 // 12 Hz
#define kBlinkRate          4 //  3 Hz

#define kQuitTitle        200
#define kQuitText         201
#define kNoQuitText       202

#define kSavegameVersion    2
#define kSavegameStrSize   12
#define kSavegameStr       "SCUMMVM_CGE2"

#define kColorNum           6

struct SavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int16 saveYear, saveMonth, saveDay;
	int16 saveHour, saveMinutes;
	uint32 playTime;
};

enum ColorBank { kCBRel, kCBStd, kCBSay, kCBInf, kCBMnu, kCBWar };

enum GamePhase { kPhaseInGame, kPhaseIntro, kPhaseOver };

// our engine debug channels
enum {
	kCGE2DebugOpcode = 1 << 0
};

enum CallbackType {
	kNullCB = 0, kQGame, kXScene
};

enum Action { kNear, kMTake, kFTake, kActions };

typedef void (CGE2Engine::*NotifyFunctionType)();

class CGE2Engine : public Engine {
private:
	uint32 _lastFrame, _lastTick;
	void tick();

	void init();
	void deinit();

	void writeSavegameHeader(Common::OutSaveFile *out, SavegameHeader &header);
	void saveGame(int slotNumber, const Common::String &desc);
	bool loadGame(int slotNumber);
	void syncHeader(Common::Serializer &s);
	void syncGame(Common::SeekableReadStream *readStream, Common::WriteStream *writeStream);
	void resetGame();
public:
	CGE2Engine(OSystem *syst, const ADGameDescription *gameDescription);
	bool hasFeature(EngineFeature f) const override;
	bool canSaveGameStateCurrently() override;
	bool canLoadGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error loadGameState(int slot) override;
	Common::Error run() override;

	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header, bool skipThumbnail = true);

	bool showTitle(const char *name);
	void cge2_main();
	char *mergeExt(char *buf, const char *name, const char *ext);
	void inf(const char *text, ColorBank col = kCBInf);
	void movie(const char *ext);
	void runGame();
	void loadHeroes();
	void loadScript(const char *fname, bool onlyToolbar = false);
	Sprite *loadSprite(const char *fname, int ref, int scene, V3D &pos);
	void badLab(const char *fn);
	void sceneUp(int cav);
	void sceneDown();
	void closePocket();
	void switchScene(int scene);
	void storeHeroPos();
	void showBak(int ref);
	void loadTab();
	int newRandom(int range);
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
	bool isHero(Sprite *spr);
	void loadUser();
	void loadPos();
	void releasePocket(Sprite *spr);
	void switchHero(int sex);
	void offUse();
	void setAutoColors();
	bool cross(const V2D &a, const V2D &b, const V2D &c, const V2D &d);
	bool contain(const V2D &a, const V2D &b, const V2D &p);
	long det(const V2D &a, const V2D &b, const V2D &c);
	int sgn(long n);
	int mapCross(const V2D &a, const V2D &b);
	Sprite *spriteAt(V2D pos);
	void keyClick();
	void swapInPocket(Sprite *spr, Sprite *xspr);
	void busyStep();

	void optionTouch(int opt, uint16 mask);
	void switchColorMode();
	void switchMusic();
	void quit();
	void setVolume(int idx, int cnt);
	void checkVolumeSwitches();
	void switchCap();
	void switchVox();
	void switchSay();
	void initToolbar();
	void initVolumeSwitch(Sprite *volSwitch, int val);
	void checkMute();

	void checkSounds();

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
	void snSeq(Sprite *spr, int val);
	void snRSeq(Sprite *spr, int val);
	void snSend(Sprite *spr, int val);
	void snSwap(Sprite *spr, int val);
	void snCover(Sprite *spr, int val);
	void snUncover(Sprite *spr, Sprite *spr2);
	void snKeep(Sprite *spr, int val);
	void snGive(Sprite *spr, int val);
	void snGoto(Sprite *spr, int val);
	void snPort(Sprite *spr, int port);
	void snMouse(bool on);
	void snNNext(Sprite *spr, Action act, int val);
	void snRNNext(Sprite *spr, int val);
	void snRMTNext(Sprite *spr, int val);
	void snRFTNext(Sprite *spr, int val);
	void snRmNear(Sprite *spr);
	void snRmMTake(Sprite *spr);
	void snRmFTake(Sprite *spr);
	void snSetRef(Sprite *spr, int val);
	void snFlash(bool on);
	void snCycle(int cnt);
	void snWalk(Sprite *spr, int val);
	void snReach(Sprite *spr, int val);
	void snSound(Sprite *spr, int wav, Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType);
	void snRoom(Sprite *spr, bool on);
	void snGhost(Bitmap *bmp);
	void snSay(Sprite *spr, int val);

	void hide1(Sprite *spr);
	Sprite *expandSprite(Sprite *spr);
	void qGame();
	void xScene();

	const ADGameDescription *_gameDescription;

	Common::RandomSource _randomSource;

	bool _quitFlag;
	Dac *_bitmapPalette;
	GamePhase _gamePhase; // Original name: startupmode
	int _now;
	int _sex;
	int _mouseTop;
	bool _dark;
	int _waitSeq;
	int _waitRef;

	struct {
		int *_wait;
		int _ref[2];
	} _soundStat;

	bool _taken;
	bool _endGame;
	int _req;
	NotifyFunctionType _midiNotify;
	NotifyFunctionType _spriteNotify;
	int _startGameSlot;

	bool _sayCap;
	bool _sayVox;
	int _oldMusicVolume;
	int _oldSfxVolume;
	bool _music;
	bool _muteAll;

	ResourceManager *_resman;
	Vga *_vga;
	MusicPlayer *_midiPlayer;
	Fx *_fx;
	Sound *_sound;
	Text *_text;
	HeroTab *_heroTab[2];
	V3D *_eye;
	V3D *_eyeTab[kSceneMax];
	Spare *_spare;
	CommandHandler *_commandHandler;
	CommandHandler *_commandHandlerTurbo;
	Font *_font;
	InfoLine *_infoLine;
	Mouse *_mouse;
	Keyboard *_keyboard;
	Talk *_talk;
	V3D *_point[kMaxPoint];
	System *_sys;
	Sprite *_busyPtr;
	Sprite *_vol[2];
	EventManager *_eventManager;
	Map *_map;
};

} // End of namespace CGE2

#endif // CGE2_CGE2_H
