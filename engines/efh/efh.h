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

struct UnkAnimStruct {
	uint8 field0;
	uint8 field1;
	uint8 field2;
	uint8 field3;

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
	uint8 field_13;
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
	char _name[9];
	uint8 field_9;
	uint8 field_A;
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
	uint8 _MonsterRef;
	uint8 _field_8;
	uint8 _field_9;
	uint8 _groupSize;
	int16 _pictureRef[9];
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
	// Engine APIs
	Common::Error run() override;
	void handleMenu();

private:
	static EfhEngine *s_Engine;

	GameType _gameType;
	Common::Platform _platform;

	void initPalette();
	void initialize();
	int32 readFileToBuffer(Common::String &filename, uint8 *destBuffer);
	void readAnimInfo();
	void findMapFile(int16 mapId);
	void loadNewPortrait();
	void loadAnimImageSet();
	void loadHistory();
	void loadTechMapImp(int16 fileId);
	void loadPlacesFile(uint16 fullPlaceId, bool forceReloadFl);
	void drawLeftCenterBox();
	void displayAnimFrame();
	void displayAnimFrames(int16 animId, bool displayMenuBoxFl);
	void readTileFact();
	void readItems();
	void loadNPCS();
	void setDefaultNoteDuration();
	Common::KeyCode playSong(uint8 *buffer);
	void decryptImpFile(bool techMapFl);
	void readImpFile(int16 id, bool techMapFl);
	Common::KeyCode getLastCharAfterAnimCount(int16 delay);
	void playIntro();
	void initEngine();
	void initMapMonsters();
	void loadMapMonsters();
	void saveAnimImageSetId();
	int16 getEquipmentDefense(int16 charId, bool flag);
	uint16 sub1C80A(int16 charId, int field18, bool flag);
	void displayLowStatusScreen(bool flag);
	void loadImageSet(int imageSetId, uint8 *buffer, uint8 **subFilesArray, uint8 *destBuffer);
	void rImageFile(Common::String filename, uint8 *targetBuffer, uint8 **subFilesArray, uint8 *packedBuffer);
	void displayFctFullScreen();
	void copyDirtyRect(int16 minX, int16 minY, int16 maxX, int16 maxY);
	void copyGraphicBufferFromTo(EfhGraphicsStruct *efh_graphics_struct, EfhGraphicsStruct *efh_graphics_struct1, const Common::Rect &rect, int16 min_x, int16 min_y);
	void loadImageSetToTileBank(int16 tileBankId, int16 imageSetId);
	void restoreAnimImageSetId();
	void checkProtection();
	void loadGame();
	uint32 uncompressBuffer(uint8 *compressedBuf, uint8 *destBuf);
	void copyCurrentPlaceToBuffer(int id);
	uint8 getMapTileInfo(int16 mapPosX, int16 mapPosY);
	void drawRect(int minX, int minY, int maxX, int maxY);
	void drawColoredRect(int minX, int minY, int maxX, int maxY, int color);
	void clearScreen(int color);
	Common::KeyCode handleAndMapInput(bool animFl);
	void displayNextAnimFrame();
	void writeTechAndMapFiles();
	uint16 getStringWidth(const char *buffer);
	void setTextPos(int16 textPosX, int16 textPosY);

	void sub15150(bool flag);
	void drawMap(bool largeMapFl, int16 mapPosX, int16 mapPosY, int mapSize, bool drawHeroFl, bool drawMonstersFl);
	void displaySmallMap(int16 posX, int16 posY);
	void displayLargeMap(int16 posX, int16 posY);
	void redrawScreen();
	void displayRawDataAtPos(uint8 *imagePtr, int16 posX, int16 posY);
	void displayBufferBmAtPos(BufferBM *bufferBM, int16 posX, int16 posY);
	uint8 *script_readNumberArray(uint8 *buffer, int16 destArraySize, int16 *destArray);
	uint8 *script_getNumber(uint8 *srcBuffer, int16 *retval);
	void removeObject(int16 charId, int16 objectId);
	void totalPartyKill();
	int16 getRandom(int16 maxVal);
	void removeCharacterFromTeam(int16 teamMemberId);
	void emptyFunction(int i);
	void refreshTeamSize();
	bool isCharacterATeamMember(int16 id);
	bool isTPK();
	Common::KeyCode getInput(int16 delay);
	void handleWinSequence();
	bool giveItemTo(int16 charId, int16 objectId, int altCharId);
	void drawString(const char *str, int16 startX, int16 startY, uint16 unkFl);
	void displayCenteredString(const char *str, int16 minX, int16 maxX, int16 posY);
	int16 chooseCharacterToReplace();
	int16 handleCharacterJoining();
	void drawMapWindow();
	void copyString(char *srcStr, char *destStr);
	int16 script_parse(uint8 *str, int posX, int posY, int maxX, int maxY, int argC);
	void sub133E5(uint8 *impPtr, int posX, int posY, int maxX, int maxY, int argC);
	void displayGameScreen();
	void sub221FA(uint8 *impArray, bool flag);
	void drawUpperLeftBorders();
	void drawUpperRightBorders();
	void drawBottomBorders();
	void sub15A28(int16 arg0, int16 arg2);
	void sub2455E(int16 arg0, int16 arg1, int16 arg2);
	int16 sub1C219(const char *str, int menuType, int arg4, int displayTeamWindowFl);
	int16 sub151FD(int16 posX, int16 posY);
	void drawChar(uint8 curChar, int16 posX, int posY);
	void setTextColorWhite();
	void setTextColorRed();
	void setTextColorGrey();
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

	void setNumLock();
	void computeMapAnimation();
	void unkFct_anim();
	void setNextCharacterPos();
	void displayStringAtTextPos(const char *message);
	void unkFct_displayMenuBox_2(int16 color);
	int8 sub16B08(int16 monsterId);
	bool moveMonsterAwayFromTeam(int16 monsterId);
	bool moveMonsterTowardsTeam(int16 monsterId);
	bool moveMonsterGroupOther(int16 monsterId, int16 direction);
	bool moveMonsterGroup(int16 monsterId);
	int16 computeMonsterGroupDistance(int monsterId);
	bool checkWeaponRange(int16 monsterId, int weaponId);
	bool unkFct_checkMonsterField8(int id, bool teamFlag);
	bool checkTeamWeaponRange(int16 monsterId);
	bool checkIfMonsterOnSameLargelMapPlace(int16 monsterId);
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
	bool handleFight(int16 monsterId);
	void displayMenuItemString(int16 menuBoxId, int thisBoxId, int minX, int maxX, int minY, const char *str);
	void displayStatusMenu(int16 windowId);
	void countRightWindowItems(int16 menuId, int16 charId);
	void displayCharacterSummary(int16 curMenuLine, int16 npcId);
	void displayCharacterInformationOrSkills(int16 curMenuLine, int16 npcId);
	void displayStatusMenuActions(int16 menuId, int16 curMenuLine, int16 npcId);
	void unk_StatusMenu(int16 windowId, int16 menuId, int16 curMenuLine, int16 charId, bool unusedFl, bool refreshFl);
	void displayWindow(uint8 *buffer, int16 posX, int16 posY, uint8 *dest);
	void sub18E80(int16 charId, int16 windowId, int16 menuId, int16 curMenuLine);
	int16 displayString_3(const char * str, bool animFl, int16 charId, int16 windowId, int16 menuId, int16 curMenuLine);
	bool isItemCursed(int16 itemId);
	bool hasObjectEquipped(int16 charId, int16 _objectId);
	void sub191FF(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine);
	int16 sub19E2E(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine, int16 argA);
	bool getValidationFromUser();
	int16 handleStatusMenu(int16 gameMode, int16 charId);
	Common::KeyCode waitForKey();
	Common::KeyCode mapInputCode(Common::KeyCode input);
	bool sub16E14();

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
	uint8 _curPlace[600];
	NPCStruct _npcBuf[100];
	uint8 _imp1[13000];
	uint8 _imp2[10000];
	uint8 _titleSong[1024];
	ItemStruct _items[300];
	uint8 _tileFact[864];
	AnimInfo _animInfo[100];
	uint8 _history[256];
	uint8 _techData[4096];
	char _ennemyNamePt2[20];
	char _characterNamePt2[20];
	char _nameBuffer[20];
	uint8 _messageToBePrinted[400];
	
	uint8 *_mapBitmapRef;
	uint8 *_mapUnknownPtr;
	uint8 *_mapMonstersPtr;
	MapMonster _mapMonsters[64];
	uint8 *_mapGameMapPtr;

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
	int16 _teamSize;
	int16 _word2C872;
	bool _word2C880;
	bool _redrawNeededFl;
	bool _word2C8D7;
	bool _drawHeroOnMapFl;
	bool _drawMonstersOnMapFl;
	bool _word2C87A;
	int16 _unk_sub26437_flag;

	int16 _imageSetSubFilesIdx;
	int16 _oldImageSetSubFilesIdx;

	int16 _mapPosX, _mapPosY;
	int16 _oldMapPosX, _oldMapPosY;
	int16 _techDataId_MapPosX, _techDataId_MapPosY;
	uint16 _lastMainPlaceId;

	uint16 _word2C86E;
	uint8 *_dword2C856;
	bool _word2C8D9;
	bool _word2C8D5; // CHECKME: always 0?
	bool _word2D0BC;
	bool _word2C8D2;
	int16 _menuDepth;
	int16 _word2D0BA;

	int16 _word3273A[15];
};

} // End of namespace Efh

#endif
