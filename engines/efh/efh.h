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

#ifndef EFH_EFH_H
#define EFH_EFH_H

#include "efh/detection.h"
#include "efh/constants.h"

#include "common/file.h"
#include "common/rect.h"
#include "common/events.h"

#include "engines/engine.h"
#include "graphics/surface.h"


namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the Efh engine.
 *
 * Status of this engine:
 * - Skeletton
 *
 * Games using this engine:
 * - Escape From Hell
 * 
 * Escape From Hell is based on a modified Wasteland engine, so this engine could eventually, one day, also support:
 * - Wasteland
 * - Fountain of Dreams
 */
namespace Efh {

static const int kSavegameVersion = 1;

enum AccessDebugChannels {
	kDebugEngine = 1 << 0,
	kDebugUtils = 1 << 1,
	kDebugGraphics = 1 << 2
};

struct EfhGameDescription;

class EfhGraphicsStruct {
public:
	EfhGraphicsStruct();
	EfhGraphicsStruct(int8 **lineBuf, int16 x, int16 y, int16 width, int16 height);

	int8 **_vgaLineBuffer;
	uint16 _shiftValue;
	uint16 _width;
	uint16 _height;
	Common::Rect _area;

	void copy(EfhGraphicsStruct *src);
};

struct InvObject {
	int16 _ref;
	uint8 _stat1;
	uint8 _stat2;

	void init();
};

struct UnkMapStruct {
	uint8 _placeId;
	uint8 _posX;
	uint8 _posY;
	uint8 _field3;
	uint8 _field4;
	uint16 _field5;
	uint16 _field7;

	void init();
};

struct UnkAnimStruct {
	int8 _field[4];

	void init();
};
struct AnimInfo {
	UnkAnimStruct _unkAnimArray[15];
	uint8 _field3C_startY[10];
	uint16 _field46_startX[10];

	void init();
};

struct ItemStruct {
	char _name[15];
	uint8 _damage;
	uint8 _defense;
	uint8 _attacks;
	uint8 _uses;
	int8 field_13; // data contains values from -8 to +8
	uint8 _range;
	uint8 _attackType;
	uint8 field_16;
	uint8 field17_attackTypeDefense;
	uint8 field_18;
	uint8 field_19;
	uint8 field_1A;

	void init();
};

struct NPCStruct {
	char _name[11];
	uint8 field_B;
	uint8 field_C;
	uint8 field_D;
	uint8 field_E;
	uint8 field_F;
	uint8 field_10;
	uint8 field_11;
	uint16 field_12;
	uint16 field_14;
	uint32 _xp;
	uint8 _activeScore[15];
	uint8 _passiveScore[11];
	uint8 _infoScore[11];
	uint8 field_3F;
	uint8 field_40;
	InvObject _inventory[10];
	uint8 _possessivePronounSHL6;
	uint8 _speed;
	uint8 field_6B;
	uint8 field_6C;
	uint8 field_6D;
	uint8 _unkItemId;
	uint8 field_6F;
	uint8 field_70;
	uint8 field_71;
	uint8 field_72;
	uint8 field_73;
	int16 _hitPoints;
	int16 _maxHP;
	uint8 field_78;
	uint16 field_79;
	uint16 field_7B;
	uint8 field_7D;
	uint8 field_7E;
	uint8 field_7F;
	uint8 field_80;
	uint8 field_81;
	uint8 field_82;
	uint8 field_83;
	uint8 field_84;
	uint8 field_85;

	void init();
};

struct FontDescr {
	const uint8 *_widthArray;
	const uint8 *_extraLines;
	const Font  *_fontData;
	uint8 _charHeight;
	uint8 _extraVerticalSpace;
	uint8 _extraHorizontalSpace;
};

struct BufferBM {
	uint8 *_dataPtr;
	uint16 _width;
	uint16 _startX;
	uint16 _startY;
	uint16 _height;
	uint16 _lineDataSize;
	uint8 _paletteTransformation;
	uint16 _fieldD;
};

struct CharStatus {
	int16 _status;
	int16 _duration;
};

struct MapMonster {
	uint8 _possessivePronounSHL6;
	uint8 _field_1;
	uint8 _guess_fullPlaceId; // unsigned? Magic values are 0xFF and 0xFE
	uint8 _posX;
	uint8 _posY;
	uint8 _itemId_Weapon;
	uint8 _field_6;
	uint8 _monsterRef;
	uint8 _field_8;
	uint8 _field_9;
	uint8 _groupSize;
	int16 _pictureRef[9];
};

struct Stru32686 {
	int16 _field0[9];
	int16 _field2[9];

	void init();
};

struct Stru3244C {
	int16 _field0;
	int16 _field2;

	void init();
};

struct TileFactStruct {
	uint8 _field0;
	uint8 _field1;

	void init();
};

class EfhEngine : public Engine {
public:
	EfhEngine(OSystem *syst, const EfhGameDescription *gd);
	~EfhEngine() override;

	OSystem *_system;
	Graphics::Surface *_mainSurface;
	Common::RandomSource *_rnd;

	const EfhGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void initGame(const EfhGameDescription *gd);
	GameType getGameType() const;
	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const override;
	const char *getCopyrightString() const;

	Common::String getSavegameFilename(int slot);
	void syncSoundSettings() override;

	bool _shouldQuit;

protected:
	Common::EventManager *_eventMan;
	int _lastTime;
	void saveGame();
	// Engine APIs
	Common::Error run() override;
	void handleMenu();

private:
	static EfhEngine *s_Engine;

	GameType _gameType;
	Common::Platform _platform;

	void initialize();
	void readAnimInfo();
	void findMapFile(int16 mapId);
	void loadNewPortrait();
	void loadAnimImageSet();
	void loadHistory();
	void loadTechMapImp(int16 fileId);
	void loadPlacesFile(uint16 fullPlaceId, bool forceReloadFl);
	void readTileFact();
	void readItems();
	void loadNPCS();
	Common::KeyCode playSong(uint8 *buffer);
	void readImpFile(int16 id, bool techMapFl);
	void playIntro();
	void initEngine();
	void initMapMonsters();
	void loadMapArrays();
	void saveAnimImageSetId();
	int16 getEquipmentDefense(int16 charId, bool flag);
	uint16 sub1C80A(int16 charId, int16 field18, bool flag);
	void displayLowStatusScreen(bool flag);
	void loadImageSetToTileBank(int16 tileBankId, int16 imageSetId);
	void restoreAnimImageSetId();
	void checkProtection();
	void loadEfhGame();
	void saveEfhGame();
	void copyCurrentPlaceToBuffer(int16 id);
	uint8 getMapTileInfo(int16 mapPosX, int16 mapPosY);
	void displayNextAnimFrame();
	void writeTechAndMapFiles();
	uint16 getStringWidth(const char *buffer);
	void setTextPos(int16 textPosX, int16 textPosY);
	void drawGameScreenAndTempText(bool flag);
	void drawMap(bool largeMapFl, int16 mapPosX, int16 mapPosY, int16 mapSize, bool drawHeroFl, bool drawMonstersFl);
	void displaySmallMap(int16 posX, int16 posY);
	void displayLargeMap(int16 posX, int16 posY);
	void drawScreen();
	uint8 *script_readNumberArray(uint8 *buffer, int16 destArraySize, int16 *destArray);
	uint8 *script_getNumber(uint8 *srcBuffer, int16 *retval);
	void removeObject(int16 charId, int16 objectId);
	void totalPartyKill();
	void removeCharacterFromTeam(int16 teamMemberId);
	void refreshTeamSize();
	bool isCharacterATeamMember(int16 id);
	bool isTPK();
	void handleWinSequence();
	bool giveItemTo(int16 charId, int16 objectId, int16 altCharId);
	int16 chooseCharacterToReplace();
	int16 handleCharacterJoining();
	int16 script_parse(uint8 *str, int16 posX, int16 posY, int16 maxX, int16 maxY, bool flag);
	void drawText(uint8 *impPtr, int16 posX, int16 posY, int16 maxX, int16 maxY, bool flag);
	void displayMiddleLeftTempText(uint8 *impArray, bool flag);
	void sub15A28(int16 arg0, int16 arg2);
	void sub2455E(int16 arg0, int16 arg1, int16 arg2);
	int16 sub1C219(uint8 *str, int16 menuType, int16 arg4, bool displayTeamWindowFl);
	int16 sub151FD(int16 posX, int16 posY);
	bool isPosOutOfMap(int16 mapPosX, int16 mapPosY);
	void goSouth();
	void goNorth();
	void goEast();
	void goWest();
	void goNorthEast();
	void goSouthEast();
	void goNorthWest();
	void goSouthWest();
	void handleNewRoundEffects();
	bool handleDeathMenu();
	void computeMapAnimation();
	void unkFct_anim();
	int8 sub16B08(int16 monsterId);
	bool moveMonsterAwayFromTeam(int16 monsterId);
	bool moveMonsterTowardsTeam(int16 monsterId);
	bool moveMonsterGroupOther(int16 monsterId, int16 direction);
	bool moveMonsterGroup(int16 monsterId);
	int16 computeMonsterGroupDistance(int16 monsterId);
	bool checkWeaponRange(int16 monsterId, int16 weaponId);
	bool unkFct_checkMonsterField8(int16 id, bool teamFlag);
	bool checkTeamWeaponRange(int16 monsterId);
	bool checkIfMonsterOnSameLargeMapPlace(int16 monsterId);
	bool checkMonsterWeaponRange(int16 monsterId);
	void sub174A0();
	bool checkPictureRefAvailability(int16 monsterId);
	void displayMonsterAnim(int16 monsterId);
	int16 countPictureRef(int16 id, bool teamMemberFl);
	bool checkMonsterGroupDistance1OrLess(int16 monsterId);
	bool sub21820(int16 monsterId, int16 arg2, int16 itemId);
	void sub221D2(int16 monsterId);
	void sub22AA8(int16 arg0);
	bool sub22293(int16 mapPosX, int16 mapPosY, int16 charId, int16 itemId, int16 arg8, int16 imageSetId);
	int8 sub15581(int16 mapPosX, int16 mapPosY, int16 arg4);
	bool sub1BC74(int16 monsterId, int16 teamMonsterId);
	void sub1BCA7(int16 monsterTeamId);
	void reset_stru32686();
	void sub1BE89(int16 monsterId);
	void resetTeamMonsterIdArray();
	bool isTeamMemberStatusNormal(int16 id);
	void sub1CDFA();
	void redrawScreenForced();
	int16 selectMonsterGroup();
	int16 sub1C956(int16 charId, int16 unkFied18Val, bool arg4);
	void sub1CAB6(int16 charId);
	bool sub1CB27();
	void sub1BE9A(int16 monsterId);
	int16 getTeamMonsterAnimId();
	int16 countMonsterGroupMembers(int16 monsterGroup);
	void sub1C4CA(bool WhiteFl);
	void displayCombatMenu(int16 charId);
	void drawCombatScreen(int16 charId, bool whiteFl, bool forceDrawFl);
	void handleFight_checkEndEffect(int16 charId);
	int16 sub1DEC8(int16 groupNumber);
	int16 getCharacterScore(int16 charId, int16 itemId);
	bool checkSpecialItemsOnCurrentPlace(int16 itemId);
	void generateSound(int16 soundType);
	void genericGenerateSound(int16 soundType, int16 repeatCount);
	bool hasAdequateDefense(int16 monsterId, uint8 attackType);
	bool hasAdequateDefense_2(int16 charId, uint8 attackType);
	void getDeathTypeDescription(int16 attackerId, int16 victimId);
	bool characterSearchesMonsterCorpse(int16 charId, int16 monsterId);
	void getXPAndSearchCorpse(int16 charId, char* namePt1, char* namePt2, int16 monsterId);
	void addReactionText(int16 id);
	void handleFight_lastAction_A(int16 teamCharId);
	void handleFight_lastAction_D(int16 teamCharId);
	void handleFight_lastAction_H(int16 teamCharId);
	void handleFight_lastAction_U(int16 teamCharId);
	char getFightMessageLastCharacter(char *message);
	void sub1D8C2(int16 charId, int16 damage);
	bool handleFight(int16 monsterId);
	void displayMenuItemString(int16 menuBoxId, int16 thisBoxId, int16 minX, int16 maxX, int16 minY, const char *str);
	void displayStatusMenu(int16 windowId);
	void countRightWindowItems(int16 menuId, int16 charId);
	int16 getXPLevel(int32 xp);
	void displayCharacterSummary(int16 curMenuLine, int16 npcId);
	void displayCharacterInformationOrSkills(int16 curMenuLine, int16 npcId);
	void displayStatusMenuActions(int16 menuId, int16 curMenuLine, int16 npcId);
	void unk_StatusMenu(int16 windowId, int16 menuId, int16 curMenuLine, int16 charId, bool unusedFl, bool refreshFl);
	void sub18E80(int16 charId, int16 windowId, int16 menuId, int16 curMenuLine);
	int16 displayString_3(const char *str, bool animFl, int16 charId, int16 windowId, int16 menuId, int16 curMenuLine);
	bool isItemCursed(int16 itemId);
	bool hasObjectEquipped(int16 charId, int16 objectId);
	void equipCursedItem(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine);
	void sub191FF(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine);
	void sub1E028(int16 id, uint8 mask, int16 groupFl);
	bool isMonsterActive(int16 groupId, int16 id);
	int16 sub15538(int16 mapPosX, int16 mapPosY);
	void setCharacterObjectToBroken(int16 charId, int16 objectId);
	int16 selectOtherCharFromTeam();
	int16 sub19E2E(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine, int16 argA);
	int16 handleStatusMenu(int16 gameMode, int16 charId);
	bool checkMonsterCollision();

	// Graphics
	void initPalette();
	void drawLeftCenterBox();
	void displayAnimFrame();
	void displayAnimFrames(int16 animId, bool displayMenuBoxFl);
	void displayFctFullScreen();
	void copyDirtyRect(int16 minX, int16 minY, int16 maxX, int16 maxY);
	void copyGraphicBufferFromTo(EfhGraphicsStruct *efh_graphics_struct, EfhGraphicsStruct *efh_graphics_struct1, const Common::Rect &rect, int16 min_x, int16 min_y);
	void displayBufferBmAtPos(BufferBM *bufferBM, int16 posX, int16 posY);
	void drawRect(int16 minX, int16 minY, int16 maxX, int16 maxY);
	void drawColoredRect(int16 minX, int16 minY, int16 maxX, int16 maxY, int16 color);
	void clearScreen(int16 color);
	void displayRawDataAtPos(uint8 *imagePtr, int16 posX, int16 posY);
	void drawString(const char *str, int16 startX, int16 startY, uint16 textColor);
	void displayCenteredString(const char *str, int16 minX, int16 maxX, int16 posY);
	void displayMenuAnswerString(const char *str, int16 minX, int16 maxX, int16 posY);
	void drawMapWindow();
	void displayGameScreen();
	void drawUpperLeftBorders();
	void drawUpperRightBorders();
	void drawBottomBorders();
	void drawChar(uint8 curChar, int16 posX, int16 posY);
	void setTextColorWhite();
	void setTextColorRed();
	void setTextColorGrey();
	void displayStringAtTextPos(const char *message);
	void clearBottomTextZone(int16 color);
	void clearBottomTextZone_2(int16 color);
	void setNextCharacterPos();
	void displayCharAtTextPos(char character);
	void displayWindow(uint8 *buffer, int16 posX, int16 posY, uint8 *dest);
	void displayColoredMenuBox(int16 minX, int16 minY, int16 maxX, int16 maxY, int16 color);

	// Utils
	int32 readFileToBuffer(Common::String &filename, uint8 *destBuffer);
	void setDefaultNoteDuration();
	void decryptImpFile(bool techMapFl);
	void loadImageSet(int16 imageSetId, uint8 *buffer, uint8 **subFilesArray, uint8 *destBuffer);
	void rImageFile(Common::String filename, uint8 *targetBuffer, uint8 **subFilesArray, uint8 *packedBuffer);	
	uint32 uncompressBuffer(uint8 *compressedBuf, uint8 *destBuf);
	int16 getRandom(int16 maxVal);
	Common::KeyCode getLastCharAfterAnimCount(int16 delay);
	Common::KeyCode getInput(int16 delay);
	Common::KeyCode waitForKey();
	Common::KeyCode mapInputCode(Common::KeyCode input);
	Common::KeyCode handleAndMapInput(bool animFl);
	Common::KeyCode getInputBlocking();
	void setNumLock();
	void copyString(char *srcStr, char *destStr);
	bool getValidationFromUser();


	uint8 _videoMode;
	uint8 _bufferCharBM[128];
	int8 *_vgaLineBuffer[200];
	EfhGraphicsStruct *_vgaGraphicsStruct1;
	EfhGraphicsStruct *_vgaGraphicsStruct2;
	EfhGraphicsStruct *_graphicsStruct;
	uint8 _tileBank[3][12000];
	uint8 _circleImageBuf[40100];
	uint8 _portraitBuf[25000];
	uint8 _hiResImageBuf[40100];
	uint8 _loResImageBuf[40100];
	uint8 _menuBuf[12500];
	uint8 _windowWithBorderBuf[1500];
	uint8 _map[7000];
	uint8 _places[12000];
	uint8 _curPlace[24][24];
	NPCStruct _npcBuf[100];
	uint8 _imp1[13000];
	uint8 _imp2[10000];
	uint8 _titleSong[1024];
	ItemStruct _items[300];
	TileFactStruct _tileFact[432];
	AnimInfo _animInfo[100];
	uint8 _history[256];
	uint8 _techData[4096];
	char _enemyNamePt1[5];
	char _enemyNamePt2[20];
	char _characterNamePt1[5];
	char _characterNamePt2[20];
	char _nameBuffer[20];
	char _attackBuffer[20];
	uint8 _messageToBePrinted[400];
	
	uint8 *_mapBitmapRef;
	UnkMapStruct _mapUnknown[100];
	MapMonster _mapMonsters[64];
	uint8 _mapGameMap[64][64];

	uint8 _defaultBoxColor;
	FontDescr _fontDescr;

	bool _word31E9E;
	uint16 _textColor;

	int16 _oldAnimImageSetId;
	int16 _animImageSetId;
	uint8 _paletteTransformationConstant;
	uint8 *_circleImageSubFileArray[12];
	uint8 *_imageSetSubFilesArray[214]; // CHECKME : logically it should be 216
	BufferBM _imageDataPtr;
	int16 _currentTileBankImageSetId[3];
	int16 _unkRelatedToAnimImageSetId;
	int16 _techId;
	int16 _currentAnimImageSetId;
	uint8 *_portraitSubFilesArray[20];
	int16 _unkAnimRelatedIndex;
	uint8 *_imp1PtrArray[100];
	uint8 *_imp2PtrArray[432];
	uint16 _fullPlaceId;
	int16 _guessAnimationAmount;
	uint16 _largeMapFlag; // CHECKME: bool?
	int16 _teamCharId[3];
	int16 _textPosX;
	int16 _textPosY;
	
	Common::Rect _initRect;
	bool _engineInitPending;
	bool _protectionPassed;

	int16 _teamMonsterIdArray[5];
	CharStatus _teamCharStatus[3];
	int16 _unkArray2C8AA[3];
	int16 _teamLastAction[3];
	int16 _teamSize;
	int16 _word2C872;
	bool _word2C880;
	bool _redrawNeededFl;
	bool _drawHeroOnMapFl;
	bool _drawMonstersOnMapFl;
	bool _word2C87A;

	int16 _imageSetSubFilesIdx;
	int16 _oldImageSetSubFilesIdx;

	int16 _mapPosX, _mapPosY;
	int16 _oldMapPosX, _oldMapPosY;
	int16 _techDataId_MapPosX, _techDataId_MapPosY;
	uint16 _lastMainPlaceId;

	uint16 _tempTextDelay;
	uint8 *_tempTextPtr;
	bool _dbgForceDisplayUpperRightBorder; // Original debug flag? Always false.
	bool _dbgForceMonsterBlock; // Original debug flag? Always false.
	bool _word2C8D7; // Original debug flag? Always true.
	bool _ongoingFightFl;
	bool _statusMenuActive;
	int16 _menuDepth;
	int16 _word2D0BA;
	int16 _word32680[3];
	int16 _word32482[3];
	int16 _teamNextAttack[3];
	int16 _word31780[3];

	int16 _word3273A[15];
	Stru32686 _stru32686[5];
	Stru3244C _stru3244C[8];
};

} // End of namespace Efh

#endif
