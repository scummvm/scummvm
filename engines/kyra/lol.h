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
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_LOL_H
#define KYRA_LOL_H

#include "kyra/kyra_v1.h"
#include "kyra/script_tim.h"
#include "kyra/script.h"

#include "common/list.h"

namespace Kyra {

class Screen_LoL;
class WSAMovie_v2;
struct Button;

struct LoLCharacter {
	uint16 flags;
	char name[11];
	uint8 raceClassSex;
	int16 id;
	uint8 curFaceFrame;
	uint8 nextFaceFrame;
	uint16 field_12;
	uint16 field_14;
	uint8 field_16;
	uint16 field_17[5];
	uint16 field_21;
	uint16 field_23;
	uint16 field_25;
	uint16 field_27[2];
	uint8 field_2B;
	uint16 field_2C;
	uint16 field_2E;
	uint16 field_30;
	uint16 field_32;
	uint16 field_34;
	uint8 field_36;
	uint16 field_37;
	uint16 hitPointsCur;
	uint16 hitPointsMax;
	uint16 magicPointsCur;
	uint16 magicPointsMax;
	uint8 field_41;
	uint16 damageSuffered;
	uint16 weaponHit;
	uint16 field_46;
	uint16 field_48;
	uint16 field_4A;
	uint16 field_4C;
	uint16 rand;
	uint16 items[11];
	uint8 field_66[3];
	uint8 field_69[3];
	uint8 field_6C;
	uint8 field_6D;
	uint16 field_6E;
	uint16 field_70;
	uint16 field_72;
	uint16 field_74;
	uint16 field_76;
	uint8 arrayUnk2[5];
	uint8 arrayUnk1[5];
};

struct SpellProperty {
	uint16 field_0;
	uint16 unkArr[4];
	uint16 field_A;
	uint16 field_C;
	uint16 field_E;
	uint16 spellNameCode;
	uint16 mpRequired[4];
	uint16 field_1A;
};

struct LevelBlockProperty {
	uint8 walls[4];
	uint16 itemIndex;
	uint8 field_6;
	uint8 field_7;
	uint8 field_8;
	uint8 flags;
};

struct LVL {
	uint16 itemIndexUnk;
	uint8 field_2;
	uint16 field_3;
	uint16 blockPropertyIndex;
	uint16 p_1a;
	uint16 p_1b;
	int8 level;
	uint16 p_2a;
	uint16 p_2b;
	uint8 field_10;
	uint8 field_11;
	uint8 field_12;
	uint8 field_13;
	uint8 field_14;
	uint8 field_15;
	uint8 field_16;
	uint8 field_17;
	uint8 field_18;
	uint16 field_19;
	uint8 field_1B;
	uint8 field_1C;
	int16 field_1D;
	uint8 field_1F;
	uint8 field_20;
	uint8 *offs_lvl415;
	uint8 field_25;
	uint8 field_26;
	uint8 field_27;
	uint8 field_28;
	uint8 field_29;
	uint8 field_2A;
	uint8 field_2B;
	uint8 field_2C;
	uint8 field_2D;
	uint8 field_2E;
};

struct LevelShapeProperty {
	uint16 shapeIndex[10];
	uint8 scaleFlag[10];
	uint16 shapeX[10];
	uint16 shapeY[10];
	int8 next;
	uint8 flags;
};

struct CompassDef {
	uint8 shapeIndex;
	int8 x;
	int8 y;
	uint8 flags;
};

struct ScriptOffsUnkStruct {
	uint8 field_0;
	uint8 field_1;
	uint8 field_2;
	uint8 field_3;
	uint8 field_4;
	uint8 field_5;
	uint8 field_6;
	uint8 field_7;
	uint8 field_8;
};

class LoLEngine : public KyraEngine_v1 {
public:
	LoLEngine(OSystem *system, const GameFlags &flags);
	~LoLEngine();

	Screen *screen();
private:
	Screen_LoL *_screen;
	TIMInterpreter *_tim;

	Common::Error init();
	Common::Error go();

	// initialization
	void initStaticResource();
	void preInit();

	void initializeCursors();
	int mainMenu();

	void startup();
	void startupNew();

	void runLoop();

	// mouse
	void setMouseCursorToIcon(int icon);
	void setMouseCursorToItemInHand();
	uint8 *getItemIconShapePtr(int index);

	// intro
	void setupPrologueData(bool load);

	void showIntro();

	struct CharacterPrev {
		const char *name;
		int x, y;
		int attrib[3];
	};

	static const CharacterPrev _charPreviews[];

	WSAMovie_v2 *_chargenWSA;
	static const uint8 _chargenFrameTable[];
	int chooseCharacter();

	void kingSelectionIntro();
	void kingSelectionReminder();
	void kingSelectionOutro();
	void processCharacterSelection();
	void updateSelectionAnims();
	int selectionCharInfo(int character);
	void selectionCharInfoIntro(char *file);

	int getCharSelection();
	int selectionCharAccept();

	int _charSelection;
	int _charSelectionInfoResult;

	uint32 _selectionAnimTimers[4];
	uint8 _selectionAnimFrames[4];
	static const uint8 _selectionAnimIndexTable[];

	static const uint16 _selectionPosTable[];

	static const uint8 _selectionChar1IdxTable[];
	static const uint8 _selectionChar2IdxTable[];
	static const uint8 _selectionChar3IdxTable[];
	static const uint8 _selectionChar4IdxTable[];

	static const uint8 _reminderChar1IdxTable[];
	static const uint8 _reminderChar2IdxTable[];
	static const uint8 _reminderChar3IdxTable[];
	static const uint8 _reminderChar4IdxTable[];

	static const uint8 _charInfoFrameTable[];

	// timer
	void setupTimers() {}

	// sound
	void loadTalkFile(int index);
	void snd_playVoiceFile(int);
	void snd_playSoundEffect(int track, int volume);
	void snd_loadSoundFile(int track);
	int snd_playTrack(int track);
	int snd_stopMusic();

	int _lastSfxTrack;
	int _lastMusicTrack;
	int _curMusicFileIndex;
	char _curMusicFileExt;
	int _curTlkFile;	

	int _unkAudioSpecOffs;
	bool _unkLangAudio;

	char **_ingameSoundList;
	int _ingameSoundListSize;

	const uint8 *_musicTrackMap;
	int _musicTrackMapSize;
	const uint16 *_ingameSoundIndex;
	int _ingameSoundIndexSize;
	const uint8 *_ingameGMSoundIndex;
	int _ingameGMSoundIndexSize;
	const uint8 *_ingameMT32SoundIndex;
	int _ingameMT32SoundIndexSize;

	// gui
	void gui_drawPlayField();
	void gui_drawScene(int pageNum);
	void gui_drawAllCharPortraitsWithStats();
	void gui_drawCharPortraitWithStats(int charNum);
	void gui_drawPortraitBox(int x, int y, int w, int h, int frameColor1, int frameColor2, int fillColor);
	void gui_drawCharFaceShape(int charNum, int x, int y, int pageNum);
	void gui_drawLiveMagicBar(int x, int y, int curPoints, int unk, int maxPoints, int w, int h, int col1, int col2, int flag);
	void gui_drawMoneyBox(int pageNum);
	void gui_drawInventory();
	void gui_drawInventoryItem(int index);
	void gui_drawCompass();
	void gui_drawScroll();

	bool _weaponsDisabled;
	int _unkDrawPortraitIndex;
	int _updateUnk2;
	int _compassDirectionIndex;

	const CompassDef *_compassDefs;
	int _compassDefsSize;

	// emc scripts
	void runInitScript(const char *filename, int func);
	void runInfScript(const char *filename);
	void runResidentScript(int func, int reg0);
	void runResidentScriptCustom(int func, int reg0, int reg1, int reg2, int reg3, int reg4);
	bool checkScriptUnk(int func);
	
	EMCData _scriptData;
	bool _scriptBoolSkipExec;
	uint8 _unkScriptByte;
	uint16 _currentDirection;
	uint16 _currentBlock;
	bool _boolScriptFuncDone;
	int16 _scriptExecutedFuncs[18];
	uint16 _gameFlags[15];
	uint16 _unkEMC46[16];

	// emc opcode
	int o2_setGameFlag(EMCState *script);
	int o2_testGameFlag(EMCState *script);
	int o2_loadLevelGraphics(EMCState *script);
	int o2_loadCmzFile(EMCState *script);
	int o2_loadMonsterShapes(EMCState *script);
	int o2_allocItemPropertiesBuffer(EMCState *script);
	int o2_setItemProperty(EMCState *script);
	int o2_makeItem(EMCState *script);	
	int o2_getItemPara(EMCState *script);
	int o2_getCharacterStat(EMCState *script);
	int o2_setCharacterStat(EMCState *script);
	int o2_loadLevelShapes(EMCState *script);
	int o2_closeLevelShapeFile(EMCState *script);
	int o2_loadDoorShapes(EMCState *script);
	int o2_setMusicTrack(EMCState *script);
	int o2_getUnkArrayVal(EMCState *script);
	int o2_setUnkArrayVal(EMCState *script);
	int o2_setGlobalVar(EMCState *script);
	int o2_mapShapeToBlock(EMCState *script);
	int o2_resetBlockShapeAssignment(EMCState *script);
	int o2_loadLangFile(EMCState *script);
	int o2_loadSoundFile(EMCState *script);
	int o2_setPaletteBrightness(EMCState *script);
	int o2_assignCustomSfx(EMCState *script);

	// tim opcode
	void setupOpcodeTable();

	Common::Array<const TIMOpcode*> _timIntroOpcodes;
	int tlol_setupPaletteFade(const TIM *tim, const uint16 *param);
	int tlol_loadPalette(const TIM *tim, const uint16 *param);
	int tlol_setupPaletteFadeEx(const TIM *tim, const uint16 *param);
	int tlol_processWsaFrame(const TIM *tim, const uint16 *param);
	int tlol_displayText(const TIM *tim, const uint16 *param);

	// translation
	int _lang;

	uint8 *_landsFile;
	uint8 *_levelLangFile;

	int _lastUsedStringBuffer;
	char _stringBuffer[5][512];	// TODO: The original used a size of 512, it looks a bit large.
								// Maybe we can someday reduce the size.
	const char *getLangString(uint16 id);
	uint8 *getTableEntry(uint8 *buffer, uint16 id);

	static const char * const _languageExt[];

	// graphics
	void setupScreenDims();

	uint8 **_itemIconShapes;
	int _numItemIconShapes;
	uint8 **_itemShapes;
	int _numItemShapes;
	uint8 **_gameShapes;
	int _numGameShapes;
	uint8 **_thrownShapes;
	int _numThrownShapes;
	uint8 **_iceShapes;
	int _numIceShapes;
	uint8 **_fireballShapes;
	int _numFireballShapes;

	const int8 *_gameShapeMap;
	int _gameShapeMapSize;

	uint8 *_characterFaceShapes[40][3];

	// characters
	bool addCharacter(int id);
	void initCharacter(int charNum, int firstFaceFrame, int unk2, int redraw);
	void initCharacterUnkSub(int charNum, int unk1, int unk2, int unk3);
	int countActiveCharacters();
	void loadCharFaceShapes(int charNum, int id);
	void calcCharPortraitXpos();

	void updatePortraitWithStats();
	void updatePortraits();
	void updatePortraitUnkTimeSub(int unk1, int unk2);

	void setCharFaceFrame(int charNum, int frameNum);
	void faceFrameRefresh(int charNum);

	LoLCharacter *_characters;
	uint16 _activeCharsXpos[3];
	int _charFlagUnk;
	int _updateCharNum;
	int _updateCharV1;
	int _updateCharV2;
	int _updateCharV3;
	int _updateCharV4;
	int _updateCharV5;
	int _updateCharV6;
	uint32 _updateCharTime;
	uint32 _updatePortraitNext;

	int _loadLevelFlag;
	int _levelFlagUnk;

	uint8 **_monsterShapes;
	uint8 **_monsterPalettes;
	uint8 **_buf4;
	uint8 _monsterUnk[3];

	const LoLCharacter *_charDefaults;
	int _charDefaultsSize;

	// level
	void loadLevel(int index);
	void addLevelItems();
	int initCmzWithScript(int block);
	void initCMZ1(LVL *l, int a);
	void initCMZ2(LVL *l, uint16 a, uint16 b);
	int cmzS1(uint16 a, uint16 b, uint16 c, uint16 d);
	void cmzS2(LVL *l, int a);
	void cmzS3(LVL *l);
	void cmzS4(uint16 &itemIndex, int a);
	int cmzS5(uint16 a, uint16 b);
	void cmzS6(uint16 &itemIndex, int a);
	void cmzS7(int itemIndex, int a);
	void loadLevelWLL(int index, bool mapShapes);
	void moveItemToBlock(uint16 *cmzItemIndex, uint16 item);
	int assignLevelShapes(int index);
	uint8 *getLevelShapes(int index);
	void loadLevelCmzFile(int index);
	void loadCMZ_Sub(int index1, int index2);
	void loadCmzFile(const char *file);
	void loadMonsterShapes(const char *file, int monsterIndex, int b);
	void releaseMonsterShapes(int monsterIndex);
	void loadLevelShpDat(const char *shpFile, const char *datFile, bool flag);
	void loadLevelGraphics(const char *file, int specialColor, int weight, int vcnLen, int vmpLen, const char *palFile);

	void resetItems(int flag);
	void resetLvlBuffer();
	void resetBlockProperties();
	bool testWallInvisibility(int block, int direction);

	void drawScene(int pageNum);

	void generateBlockDrawingBuffer(int block, int direction);
	void generateBlockDrawingBufferF0(int16 wllOffset, uint8 wllIndex, uint8 wllVmpIndex, int16 vmpOffset, uint8 len, uint8 numEntries);
	void generateBlockDrawingBufferF1(int16 wllOffset, uint8 wllIndex, uint8 wllVmpIndex, int16 vmpOffset, uint8 len, uint8 numEntries);
	bool hasWall(int index);
	void assignBlockCaps(int block, int direction);

	void drawVcnBlocks(uint8 *vcnBlocks, uint16 *blockDrawingBuffer, uint8 *vcnShift, int pageNum);
	void drawSceneShapes();
	void setLevelShapesDim(int index, int16 &x1, int16 &x2, int dim);
	void scaleLevelShapesDim(int index, int16 &y1, int16 &y2, int dim);
	void drawLevelModifyScreenDim(int dim, int16 x1, int16 y1, int16 x2, int16 y2);
	void drawDecorations(int index);
	void drawIceShapes(int index, int iceShapeIndex);
	void drawMonstersAndItems(int index);
	void drawDoor(uint8 *shape, uint8 *table, int index, int unk2, int w, int h, int flags);
	void drawDoorOrMonsterShape(uint8 *shape, uint8 *table, int x, int y, int flags, const uint8 *ovl);
	void drawScriptShapes(int pageNum);
	void updateSceneWindow();

	void turnOnLamp();
	void updateLampStatus();

	void moveParty(uint16 direction, int unk1, int unk2, int unk3);
	uint16 calcNewBlockPostion(uint16 curBlock, uint16 direction);

	void setLF1(uint16 & a, uint16 & b, int block, uint16 d, uint16 e);
	void setLF2(int block);
	
	int _unkFlag;
	int _scriptFuncIndex;
	uint8 _currentLevel;
	bool _loadLevelFlag2;
	int _lvlBlockIndex;
	int _lvlShapeIndex;
	bool _unkDrawLevelBool;
	uint8 *_vcnBlocks;
	uint8 *_vcnShift;
	uint8 *_vcnExpTable;
	uint16 *_vmpPtr;
	uint16 *_blockDrawingBuffer;
	uint8 *_sceneWindowBuffer;
	LevelShapeProperty *_levelShapeProperties;
	uint8 **_levelShapes;

	char _lastSuppFile[12];
	char _lastOverridePalFile[12];
	char *_lastOverridePalFilePtr;
	int _lastSpecialColor;
	int _lastSpecialColorWeight;

	int _sceneDrawVar1;
	int _sceneDrawVar2;
	int _sceneDrawVar3;
	int _wllProcessFlag;

	uint8 *_tlcTable2;
	uint8 *_tlcTable1;

	int _loadSuppFilesFlag;

	int _lampOilStatus;
	int _brightness;
	int _lampStatusUnk;
	uint32 _lampStatusTimer;

	uint8 *_wllVmpMap;
	int8 *_wllShapeMap;
	uint8 *_wllBuffer3;
	uint8 *_wllBuffer4;
	uint8 *_wllWallFlags;

	int16 *_lvlShapeTop;
	int16 *_lvlShapeBottom;
	int16 *_lvlShapeLeftRight;

	LevelBlockProperty *_levelBlockProperties;
	LevelBlockProperty *_curBlockCaps[18];
	LVL *_lvlBuffer;
	uint8 *_lvl415;

	uint16 _unkCmzU1;
	uint16 _unkCmzU2;
	
	Common::SeekableReadStream *_lvlShpFileHandle;
	uint16 _lvlShpNum;
	uint32 *_lvlShpHeader;
	uint16 _levelFileDataSize;
	LevelShapeProperty *_levelFileData;

	uint8 *_doorShapes[2];
	int16 _shpDmX;
	int16 _shpDmY;
	int16 _dmScaleW;
	int16 _dmScaleH;

	uint8 _unkGameFlag;

	uint8 *_tempBuffer5120;
	uint8 *_tmpData136;
	
	const char *const * _levelDatList;
	int _levelDatListSize;
	const char *const * _levelShpList;
	int _levelShpListSize;

	const int8 *_dscUnk1;
	int _dscUnk1Size;
	const int8 *_dscShapeIndex;
	int _dscShapeIndexSize;
	const uint8 *_dscOvlMap;
	int _dscOvlMapSize;
	const uint16 *_dscShapeScaleW;
	int _dscShapeScaleWSize;
	const uint16 *_dscShapeScaleH;
	int _dscShapeScaleHSize;
	const int16 *_dscShapeX;
	int _dscShapeXSize;
	const int8 *_dscShapeY;
	int _dscShapeYSize;
	const uint8 *_dscTileIndex;
	int _dscTileIndexSize;
	const uint8 *_dscUnk2;
	int _dscUnk2Size;
	const uint8 *_dscDoorShpIndex;
	int _dscDoorShpIndexSize;
	const int8 *_dscDim1;
	int _dscDim1Size;
	const int8 *_dscDim2;
	int _dscDim2Size;
	const uint8 *_dscBlockMap;
	int _dscBlockMapSize;
	const uint8 *_dscDimMap;
	int _dscDimMapSize;
	const uint16 *_dscDoorMonsterScaleTable;
	int _dscDoorMonsterScaleTableSize;
	const uint16 *_dscDoor4;
	int _dscDoor4Size;
	const uint8 *_dscShapeOvlIndex;
	int _dscShapeOvlIndexSize;
	const int8 *_dscBlockIndex;
	int _dscBlockIndexSize;
	const uint8 *_dscDoor1;
	int _dscDoor1Size;
	const int16 *_dscDoorMonsterX;
	int _dscDoorMonsterXSize;
	const int16 *_dscDoorMonsterY;
	int _dscDoorMonsterYSize;
	
	int _sceneDrawPage1;
	int _sceneDrawPage2;

	// items
	struct ItemInPlay {
		uint16 itemIndexUnk;
		uint8 unk2;
		uint16 unk3;
		uint16 blockPropertyIndex;
		uint16 unk7;
		uint16 anonymous_4;
		int8 level;
		uint16 itemPropertyIndex;
		uint16 shpCurFrame_flg;
		uint8 field10;
		uint16 anon8;
		uint8 anon9;
	};

	struct ItemProperty {
		uint16 nameStringId;
		uint8 shpIndex;
		uint16 flags;
		uint16 unk5;
		uint8 itemScriptFunc;
		uint8 unk8;
		uint8 unk9;
		uint8 unkA;
		uint16 unkB;
		uint8 unkD;
	};

	void giveCredits(int credits, int redraw);
	int makeItem(int itemIndex, int curFrame, int flags);
	bool testUnkItemFlags(int itemIndex);
	void clearItemTableEntry(int itemIndex);
	void *cmzGetItemOffset(uint16 index);
	void runItemScript(int reg1, int item, int reg0, int reg3, int reg4);

	uint8 _moneyColumnHeight[5];
	uint16 _credits;

	ItemInPlay *_itemsInPlay;
	ItemProperty *_itemProperties;

	int _itemInHand;
	uint16 _inventoryItemIndex[48];
	int _inventoryCurItem;
	int _unkInventFlag;

	EMCData _itemScript;

	// spells
	int8 _availableSpells[7];
	int _selectedSpell;
	const SpellProperty *_spellProperties;
	int _spellPropertiesSize;

	// unneeded
	void setWalkspeed(uint8) {}
	void setHandItem(uint16) {}
	void removeHandItem() {}
	bool lineIsPassable(int, int) { return false; }

	// save
	Common::Error loadGameState(int slot) { return Common::kNoError; }
	Common::Error saveGameState(int slot, const char *saveName, const Graphics::Surface *thumbnail) { return Common::kNoError; }
};

} // end of namespace Kyra

#endif

