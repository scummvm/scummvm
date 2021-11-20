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

#include "common/file.h"
#include "common/rect.h"
#include "common/events.h"

#include "engines/engine.h"
#include "graphics/palette.h"
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
	EfhGraphicsStruct(int16 *lineBuf, int16 x, int16 y, int16 width, int16 height);

	int16 *_vgaLineBuffer;
	uint16 _shiftValue;
	uint16 _width;
	uint16 _height;
	Common::Rect _area;

	void copy(EfhGraphicsStruct *src);
};

struct Font {
	uint8 _lines[8];
};

struct FontDescr {
	uint8 *_widthArray;
	uint8 *_extraLines;
	Font  *_fontData;
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
	uint16 _fieldA;
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
	uint16 _pictureRef[9];
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
	void loadPlacesFile(uint16 fullPlaceId, int16 unused, bool forceReloadFl);
	void drawUnknownMenuBox();
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
	void initEngine();
	void initMapMonsters();
	void loadMapMonsters();
	void saveAnimImageSetId();
	void displayLowStatusScreen(int i);
	void loadImageSet(int imageSetId, uint8 *buffer, uint8 **subFilesArray, char CGAVal, char EGAVal, uint8 *destBuffer, uint8 *transfBuffer);
	void rImageFile(Common::String filename, uint8 *targetBuffer, uint8 **subFilesArray, char CGAVal, char EGAVal, uint8 *packedBuffer, uint8 *transformedBuffer);
	void displayFctFullScreen();
	void displayBitmapAtPos(int16 minX, int16 minY, int16 maxX, int16 maxY);
	void displayBitmap(EfhGraphicsStruct *efh_graphics_struct, EfhGraphicsStruct *efh_graphics_struct1, const Common::Rect &rect, int16 min_x, int16 min_y);
	void loadImageSetToTileBank(int16 tileBankId, int16 imageSetId);
	void restoreAnimImageSetId();
	void checkProtection();
	void loadGame();
	uint32 uncompressBuffer(uint8 *compressedBuf, uint8 *destBuf);
	void copyCurrentPlaceToBuffer(int id);
	uint8 getMapTileInfo(int16 mapPosX, int16 mapPosY);
	void drawBox(int minX, int minY, int maxX, int maxY);
	void drawMenuBox(int minX, int minY, int maxX, int maxY, int color);
	void displayFullScreenColoredMenuBox(int color);

	void sub15150(bool flag);
	void sub12A7F();
	void sub10B77_unkDisplayFct1(uint8 *imagePtr, int16 posX, int16 posY, uint8 guess_paletteTransformation);
	void sub24D92(BufferBM *bufferBM, int16 posX, int16 posY);
	void sub133E5(uint8 *impPtr, int posX, int posY, int maxX, int maxY, int argC);
	void sub1512B();
	void sub221FA(uint8 *impArray, bool flag);
	void sub15094();
	void sub150EE();
	void sub15018();

	uint8 _videoMode;
	uint8 _bufferCharBM[128];
	int16 _vgaLineBuffer[200];
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
	uint8 _npcBuf[13400];
	uint8 _imp1[13000];
	uint8 _imp2[10000];
	uint8 _titleSong[1024];
	uint8 _items[8100];
	uint8 _tileFact[864];
	uint8 _animInfo[9000];
	uint8 _history[256];
	uint8 _techData[4096];

	uint8 *_mapBitmapRef;
	uint8 *_mapUnknownPtr;
	uint8 *_mapMonstersPtr;
	MapMonster _mapMonsters[64];
	uint8 *_mapGameMapPtr;

	uint8 _defaultBoxColor;
	FontDescr _fontDescr;

	uint16 _word31E9E;
	uint16 _unkVideoRelatedWord1;

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
	int16 _teamCharIdArray;
	int16 _charId;
	int16 _word2C8B8;
	
	Common::Rect _initRect;
	bool _engineInitPending;
	bool _protectionPassed;

	CharStatus _teamCharStatus[3];
	int16 _unkArray2C8AA[3];
	int16 _teamSize;
	int16 _word2C872;
	int16 _imageSetSubFilesIdx;

	int16 _mapPosX, _mapPosY;
	int16 _oldMapPosX, _oldMapPosY;
	int16 _techDataId_MapPosX, _techDataId_MapPosY;
	uint16 _lastMainPlaceId;

	uint8 _word2C86E;
	uint8 *_dword2C856;
};

} // End of namespace Efh

#endif
