/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef INSANE_H
#define INSANE_H

#include "base/engine.h"
#include "scumm/scumm.h"
#include "scumm/nut_renderer.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/chunk.h"

#ifdef INSANE

namespace Scumm {

#define INV_CHAIN    0
#define INV_CHAINSAW 1
#define INV_MACE     2
#define INV_2X4      3
#define INV_WRENCH   4
#define INV_BOOT     5
#define INV_HAND     6
#define INV_DUST     7

#define EN_ROTT1     0 // rottwheeler
#define EN_ROTT2     1 // rottwheeler
#define EN_ROTT3     2 // rottwheeler
#define EN_VULTF1    3 // vulture (redhead female1)
#define EN_VULTM1    4 // vulture (male with glasses)
#define EN_VULTF2    5 // vulture (redhead female2)
#define EN_VULTM2    6 // vulture (initialized as rottwheeler) (male)
#define EN_CAVEFISH  7 // Cavefish Maximum Fish
#define EN_TORQUE    8 // Father Torque
#define EN_BEN       9 // used only with handler

class Insane {
 public:
	Insane(ScummEngine *scumm);
	~Insane();

	void setSmushParams(int speed, bool subtitles);
	void runScene(int arraynum);

	void procPreRendering(void);
	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
						   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Chunk &b, int32 size, int32 flags);
	void procSKIP(Chunk &b);
	void escapeKeyHandler(void);

 private:
	
	ScummEngine *_scumm;
	SmushPlayer *_player;
	
	int32 _speed;
	bool _subtitles;
	bool _insaneIsRunning;
	
	int32 _numberArray;
   	int32 _emulTimerId;
	int32 _emulateInterrupt;
	byte _errbuf[0x7d0];
	int32 _flag1d;
	int32 _mainTimerId;
	int32 _objArray1Idx;
	int32 _objArray1Idx2;
	int32 _objArray1[101];
	int32 _objArray2Idx;
	int32 _objArray2Idx2;
	int32 _objArray2[101];
	byte _currSceneId;
	int32 _timer1Flag;
	int32 _timer3Id;
	int32 _timer4Id;
	int32 _timer6Id;
	int32 _timer7Id;
	int32 _timerSpriteId;
	byte _temp2SceneId;
	byte _tempSceneId;
	int32 _currEnemy;
	int32 _currScenePropIdx;
	int32 _currScenePropSubIdx;
	char *_currTrsMsg;
	int16 _sceneData2Loaded;
	int16 _sceneData1Loaded;
	int16 _keyboardDisable;
	bool _needSceneSwitch;
	int32 _idx2Exceeded;
	bool _memoryAllocatedNotOK;
	int32 _lastKey;
	bool _beenCheated;
	bool _tiresRustle;
	int _keybOldDx;
	int _keybOldDy;
	int _velocityX;
	int _velocityY;
	int _keybX;
	int _keybY;
	int32 _firstBattle;
	bool _weaponBenJustSwitched;
	bool _kickBenProgress;
	int32 _battleScene;
	bool _kickEnemyProgress;
	bool _weaponEnemyJustSwitched;
	int32 _enHdlVar[9][9];
	int32 _trsFilePtr; // FIXME: we don't need it
	int32 _smlayer_room;
	int32 _smlayer_room2;
	byte *_smush_roadrashRip; // FIXME: combine them in array
	byte *_smush_roadrsh2Rip;
	byte *_smush_roadrsh3Rip;
	byte *_smush_goglpaltRip;
	byte *_smush_tovista1Flu;
	byte *_smush_tovista2Flu;
	byte *_smush_toranchFlu;
	byte *_smush_minedrivFlu;
	byte *_smush_minefiteFlu;
	NutRenderer *_smush_bencutNut;
	NutRenderer *_smush_bensgoggNut;
	NutRenderer *_smush_iconsNut;
	NutRenderer *_smush_icons2Nut;
	bool _smush_isSanFileSetup;
	bool _isBenCut;
	int _smush_smushState;
	bool _smush_isPauseImuse;
	int _continueFrame;
	int _continueFrame1;
	int _counter1;
	int _iactSceneId;
	int _iactSceneId2;
	int _smush_setupsan17;
	int32 _smush_setupsan1;
	int16 _smush_setupsan2;
	int32 _smush_setupsan4;
	int16 _smush_numFrames;
	int16 _smush_frameStep;
	int16 _smush_curFrame;
	int16 _smush_frameNum1;
	int16 _smush_frameNum2;
	byte _smush_earlyFluContents[0x31a];
	int16 _enemyState[9][10];
	byte _iactBits[0x80];
	int32 _val8d;
	byte _val10b;
	int32 _val11d;
	int32 _val32d;
	int32 _val50d;
	int32 _val51d;
	int32 _val52d;
	int32 _val53d;
	int32 _val54d;
	int32 _val55d;
	int32 _val56d;
	int32 _val57d;
	int16 _val109w;
	int16 _val110w;
	int16 _val111w;
	int16 _val112w;
	int32 _val113d;
	int32 _val114d16[16];
	int16 _val115w;
	int16 _val116w;
	bool _val119_;
	bool _val120_;
	bool _val121_;
	bool _val122_;
	bool _val123_;
	bool _val124_;
	int32 _val128d;
	int32 _val211d;
	int32 _val212_;
	int32 _val213d;
	int32 _val214d;
	int32 _val215d;
	int32 _val216d[12];

	struct enemy {
		int32 handler;
		int32 initializer;
		int32 field_8;
		int32 maxdamage;
		int32 field_10;
		int32 weapon;
		int32 sound;
		char  filename[20];
		int32 costume4;
		int32 costume6;
		int32 costume5;
		int16 costumevar;
		int32 maxframe;
		int32 field_34;
	};
  
	struct enemy _enemy[9];
  
	struct fluConf {
		int sceneId;
		byte **fluPtr;
		const char *filenamePtr;
		int startFrame;
		int numFrames;
	};

	struct fluConf _fluConf[21];

	struct sceneProp {
		int32 actor; // main actor number, -1 if not applicable
		int32 sound;
		int32 trsId;
		byte r;
		byte g;
		byte b;
		int32 counter;
		int32 maxCounter;
		int32 index;
	};

	struct sceneProp _sceneProp[139];

	struct act {
		int32 actor;
		byte  state;
		int32 room;
		int32 animTilt;
		int32 tilt;
		int32 frame;
	};
  
	struct actor {
		int32 damage;
		int32 maxdamage;
		int32 field_8;
		int32 frame;
		int32 tilt;
		int32 cursorX;
		int32 speed;
		int32 x;
		int32 y;
		int32 y1;
		int32 x1;
		int32 weaponClass;
		int32 animWeaponClass;
		int32 field_34;
		int32 field_38;
		bool  lost;
		bool  kicking;
		bool  field_44;
		int32 field_48;
		bool  defunct;
		int32 scenePropSubIdx;
		int32 field_54;
		int32 runningSound;
		int32 weapon;
		bool inventory[8];
		int32 probability;
		int32 enemyHandler;
		struct act act[4];
	};

	struct actor _actor[2];

	void initvars(void);
	void readFileToMem(const char *name, byte **buf);
	void startVideo(const char *filename, int num, int argC, int frameRate, int doMainLoop);
	void startVideo1(const char *filename, int num, int argC, int frameRate, 
					 int doMainLoop, byte *fluPtr, int32 numFrames);
	void smush_proc39(void);
	void smush_proc40(void);
	void smush_proc41(void);
	void imuseCode04(void);
	void smush_warpMouse(int x, int y, int buttons);
	void putActors(void);
	void readState(void);
	void setTrsFile(int file);	// FIXME: we don't need it
	void resetTrsFilePtr(void);	 // FIXME: we don't need it
	int initScene(int sceneId);
	void stopSceneSounds(int sceneId);
	void shutCurrentScene(void);
	int loadSceneData(int scene, int flag, int phase);
	void setSceneCostumes(int sceneId);
	void setupValues(void);
	void setEnemyCostumes (void);
	void smlayer_stopSound (int idx);
	int smlayer_loadSound(int id, int flag, int phase);
	int smlayer_loadCostume(int id, int phase);
	void smlayer_setActorCostume(int actornum, int act, int costume);
	void smlayer_putActor(int actornum, int act, int x, int y, byte room);
	void smlayer_setActorLayer(int actornum, int act, int layer);
	void smlayer_setFluPalette(byte *pal, int shut_flag);
	int32 readArray (int field, int number);
	void setWordInString(int field, int number, int value);
	int smlayer_mainLoop(void);
	void mainLoop(void);
	bool idx1Compare(void);
	bool idx2Compare(void);
	int32 idx1Tweak(void);
	int32 idx2Tweak(void);
	void smush_setToFinish(void);
	void postCase11(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase0(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase17(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase16(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase2(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase20(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase3(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase5(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase6(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase8(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase9(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				   int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase10(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase12(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase23(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCase14(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCaseAll(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					 int32 setupsan13, int32 curFrame, int32 maxFrame);
	void postCaseMore(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, int32 curFrame, int32 maxFrame);
	void switchSceneIfNeeded(void);
	int smush_changeState(int state);
	void init_actStruct(int actornum, int actnum, int32 actorval, byte state, 
						  int32 room, int32 animtilt, int32 tilt, int32 frame);
	void init_enemyStruct(int n, int32 handler, int32 initializer,
							   int32 field_8, int32 maxdamage, int32 field_10,
							   int32 field_14, int32 sound, const char *filename,
							   int32 costume4, int32 costume6, int32 costume5,
							   int16 field_2C, int32 field_30, int32 field_34);
	int32 enemyHandler(int n, int32, int32, int32);
	int32 enemyInitializer(int n, int32, int32, int32);
	int32 enemy0handler(int32, int32, int32);
	int32 enemy0initializer(int32, int32, int32);
	int32 enemy1handler(int32, int32, int32);
	int32 enemy1initializer(int32, int32, int32);
	int32 enemy2handler(int32, int32, int32);
	int32 enemy2initializer(int32, int32, int32);
	int32 enemy3handler(int32, int32, int32);
	int32 enemy3initializer(int32, int32, int32);
	int32 enemy4handler(int32, int32, int32);
	int32 enemy4initializer(int32, int32, int32);
	int32 enemy5handler(int32, int32, int32);
	int32 enemy5initializer(int32, int32, int32);
	int32 enemy6handler(int32, int32, int32);
	int32 enemy6initializer(int32, int32, int32);
	int32 enemy7handler(int32, int32, int32);
	int32 enemy7initializer(int32, int32, int32);
	int32 enemy8handler(int32, int32, int32);
	int32 enemy8initializer(int32, int32, int32);
	int32 enemyBenHandler(int32, int32, int32);
	void IMUSE_shutVolume(void);
	void IMUSE_restoreVolume(void);
	bool smlayer_isSoundRunning(int32 sound);
	bool smlayer_startSound1(int32 sound);
	bool smlayer_startSound2(int32 sound);
	void smlayer_soundSetPan(int32 sound, int32 pan);
	void smlayer_soundSetPriority(int32 sound, int32 priority);
	void smlayer_drawSomething(byte *renderBitmap, int32 codecparam, 
			   int32 arg_8, int32 arg_C, int32 arg_10, NutRenderer *nutfileptr, 
			   int32 arg_18, int32 arg_1C, int32 arg_20);
	void smlayer_overrideDrawActorAt(byte *, byte, byte);
	void queueSceneSwitch(int32 sceneId, byte *fluPtr, const char *filename, 
						  int32 arg_C, int32 arg_10, int32 startFrame, int32 numFrames);
	void turnBen(bool battle);
	void smush_rewindCurrentSan(int arg_0, int arg_4, int arg_8);
	void smlayer_showStatusMsg(int32 arg_0, byte *renderBitmap, int32 codecparam, 
							   int32 x, int32 y, int32 arg_14, int32 arg_18, 
							   int32 arg_1C, const char *formatString, char *str);
	void init_fluConfStruct(int n, int sceneId, byte **fluPtr, 
							const char *filenamePtr, int startFrame, int numFrames);
	int32 processBenOnRoad(bool flag);
	void mineChooseRoad(int32 arg_0);
	void actor02Reaction(int32 buttons);
	void actor00Reaction(int32 buttons);
	void actor01Reaction(int32 buttons);
	void actor03Reaction(int32 buttons);
	void turnEnemy(bool battle);
	int32 actionBen(void);
	void chooseBenWeaponAnim(int buttons);
	void setBenAnimation(int32 actornum, int anim);
	int calcTilt(int speed);
	bool smush_eitherNotStartNewFrame(void);
	void smlayer_setActorFacing(int actornum, int actnum, int frame, int direction);
	int32 weaponMaxRange(int32 actornum);
	int32 weaponMinRange(int32 actornum);
	void switchBenWeapon(void);
	void prepareScenePropScene(int32 scenePropNum, bool arg_4, bool arg_8);
	int32 calcBenDamage(bool arg_0, bool arg_4);
	int32 weaponDamage(int32 actornum);
	void proc47(int32 actornum, int32 val);
	bool weaponBenIsEffective(void);
	bool actor1StateFlags(int state);
	bool actor0StateFlags1(int state);
	bool actor0StateFlags2(int state);
	bool loadScenePropSounds(int32 scenePropNum);
	void init_scenePropStruct(int32 n, int32 n1, int32 actornum, int32 sound, int32 trsId,
							  byte r, byte g, byte b, int32 counter, int32 maxCounter,
							  int32 index);
	int32 setBenState(void);
	bool smlayer_actorNeedRedraw(int actornum, int actnum);
	void reinitActors(void);
	void smush_setPaletteValue(int where, int r, int g, int b);
	char *handleTrsTag(int32 trsFilePtr, int32 trsId);
	void ouchSoundBen(void);
	void smush_setupSanWithFlu(const char *filename, int32 setupsan2, int32 step1, 
							   int32 step2, int32 setupsan1, byte *fluPtr, int32 numFrames);
	void smush_setupSanFromStart(const char *filename, int32 setupsan2, int32 step1, 
								 int32 step2, int32 setupsan1);
	void smush_setFrameSteps(int32 step1, int32 step2);
	void smush_setupSanFile(const char *filename, int32 offset);
	int32 getLastKey(bool arg_0);
	void drawSpeedyActor(int32 arg_0);
	void actor11Reaction(int32 buttons);
	void actor12Reaction(int32 buttons);
	void actor13Reaction(int32 buttons);
	void actor10Reaction(int32 buttons);
	int32 actionEnemy(void);
	int32 processKeyboard(void);
	int32 processMouse(void);
	void setEnemyAnimation(int32 actornum, int anim);
	void chooseEnemyWeaponAnim(int32 buttons);
	void switchEnemyWeapon(void);
	void setEnemyState(void);
	int32 calcEnemyDamage(bool arg_0, bool arg_4);
	void ouchSoundEnemy(void);
	bool weaponEnemyIsEffective(void);
	void iactScene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Chunk &b, int32 size, int32 flags);
	void iactScene3(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Chunk &b, int32 size, int32 flags);
	void iactScene4(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Chunk &b, int32 size, int32 flags);
	void iactScene6(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Chunk &b, int32 size, int32 flags);
	void iactScene17(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Chunk &b, int32 size, int32 flags);
	void iactScene21(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Chunk &b, int32 size, int32 flags);
	bool isBitSet(int n);
	void setBit(int n);
	void clearBit(int n);
	void proc62(void);
	void proc63(void);
	void proc64(int32);
};
} // End of namespace Insane

#endif // INSANE

#endif
