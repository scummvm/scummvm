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

#ifndef PRINCE_PRINCE_H
#define PRINCE_PRINCE_H

#include "common/random.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/events.h"
#include "common/endian.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "image/bmp.h"

#include "gui/debugger.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "audio/mixer.h"

#include "video/flic_decoder.h"

#include "prince/mob.h"
#include "prince/object.h"
#include "prince/pscr.h"
#include "prince/detection.h"

namespace Prince {

struct SavegameHeader;

class PrinceEngine;
class GraphicsMan;
class Script;
class Interpreter;
class InterpreterFlags;
class Debugger;
class MusicPlayer;
class VariaTxt;
class Cursor;
class MhwanhDecoder;
class Font;
class Hero;
class Animation;
class Room;
class Pscr;

struct SavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int16 saveYear, saveMonth, saveDay;
	int16 saveHour, saveMinutes;
	uint32 playTime;
};

#define kSavegameStrSize 14
#define kSavegameStr "SCUMMVM_PRINCE"

struct Text {
	const char *_str;
	uint16 _x, _y;
	uint16 _time;
	uint32 _color;

	Text() : _str(nullptr), _x(0), _y(0), _time(0), _color(255){
	}
};

struct AnimListItem {
	uint16 _type; // type of animation - for background anims RND of frame
	uint16 _fileNumber;
	uint16 _startPhase; // first phase number
	uint16 _endPhase;
	uint16 _loopPhase;
	int16 _x;
	int16 _y;
	uint16 _loopType;
	uint16 _nextAnim; // number of animation to do for loop = 3
	uint16 _flags; // byte 0 - draw masks, byte 1 - draw in front of mask, byte 2 - load but turn off drawing
	bool loadFromStream(Common::SeekableReadStream &stream);
};

struct BAS {
	int32 _type; // type of sequence
	int32 _data; // additional data
	int32 _anims; // number of animations
	int32 _current; // actual number of animation
	int32 _counter; // time counter for animation
	int32 _currRelative; //actual relative number for animation
	int32 _data2; // additional data for measurements
};

const int kStructSizeBAS = 28;

struct BASA {
	int16 _num;	// animation number
	int16 _start;	// initial frame
	int16 _end;	// final frame
	//int16 _pad;	// fulfilment to 8 bytes
};

const int kStructSizeBASA = 8;

// background and normal animation
struct Anim {
	BASA _basaData;
	int32 _addr; //animation adress
	int16 _usage;
	int16 _state; // state of animation: 0 - turning on, 1 - turning off
	int16 _flags;
	int16 _frame; // number of phase to show
	int16 _lastFrame; // last phase
	int16 _loopFrame; // first frame of loop
	int16 _showFrame; // actual visible frame of animation
	int16 _loopType;	 // type of loop (0 - last frame; 1 - normal loop (begin from _loopFrame); 2 - no loop; 3 - load new animation)
	int16 _nextAnim; // number of next animation to load after actual
	int16 _x;
	int16 _y;
	int32 _currFrame;
	int16 _currX;
	int16 _currY;
	int16 _currW;
	int16 _currH;
	int16 _packFlag;
	int32 _currShadowFrame;
	int16 _packShadowFlag;
	int32 _shadowBack;
	int16 _relX;
	int16 _relY;
	Animation *_animData;
	Animation *_shadowData;

	enum AnimOffsets {
		kAnimState = 10,
		kAnimFrame = 14,
		kAnimLastFrame = 16,
		kAnimX = 26
	};

	int16 getAnimData(Anim::AnimOffsets offset) {
		switch (offset) {
		case kAnimState:
			return _state;
		case kAnimFrame:
			return _frame + 1; // fix for location 30 - man with a dog animation
		case kAnimX:
			return _x;
		default:
			error("getAnimData() - Wrong offset type: %d", (int)offset);
		}
	}

	void setAnimData(Anim::AnimOffsets offset, int16 value) {
		if (offset == kAnimX) {
			_x = value;
		} else {
			error("setAnimData() - Wrong offset: %d, value: %d", (int)offset, value);
		}
	}
};

struct BackgroundAnim {
	BAS _seq;
	Common::Array<Anim> backAnims;
};

enum AnimType {
	kBackgroundAnimation,
	kNormalAnimation
};

// Nak (PL - Nakladka)
struct Mask {
	uint16 _state; // visible / invisible
	int16 _flags; // turning on / turning off of an mask
	int16 _x1;
	int16 _y1;
	int16 _x2;
	int16 _y2;
	int16 _z;
	int16 _number; // number of mask for background recreating
	int16 _width;
	int16 _height;
	byte *_data;

	int16 getX() const {
		return READ_LE_UINT16(_data);
	}

	int16 getY() const {
		return READ_LE_UINT16(_data + 2);
	}

	int16 getWidth() const {
		return READ_LE_UINT16(_data + 4);
	}

	int16 getHeight() const {
		return READ_LE_UINT16(_data + 6);
	}

	byte *getMask() const {
		return (byte *)(_data + 8);
	}
};

struct InvItem {
	int _x;
	int _y;
	Graphics::Surface *_surface;
	Graphics::Surface *getSurface() const { return _surface; }
};

struct DrawNode {
	int posX;
	int posY;
	int posZ;
	int32 width;
	int32 height;
	int32 scaleValue;
	Graphics::Surface *s;
	Graphics::Surface *originalRoomSurface;
	void *data;
	void (*drawFunction)(Graphics::Surface *, DrawNode *);
};

struct DebugChannel {

enum Type {
	kScript,
	kEngine
};

};

class PrinceEngine : public Engine {
protected:
	Common::Error run() override;

public:
	PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc);
	~PrinceEngine() override;

	bool scummVMSaveLoadDialog(bool isSave);

	bool hasFeature(EngineFeature f) const override;
	void pauseEngineIntern(bool pause) override;
	bool canSaveGameStateCurrently() override;
	bool canLoadGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error loadGameState(int slot) override;

	void playVideo(Common::String videoFilename);

	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header, bool skipThumbnail = true);
	void writeSavegameHeader(Common::OutSaveFile *out, SavegameHeader &header);
	void syncGame(Common::SeekableReadStream *readStream, Common::WriteStream *writeStream);
	bool loadGame(int slotNumber);
	void resetGame();

	int32 _creditsDataSize;
	byte *_creditsData;
	void scrollCredits();

	int getGameType() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	const PrinceGameDescription *_gameDescription;
	Video::FlicDecoder _flicPlayer;
	const Graphics::Surface *_flcFrameSurface;
	VariaTxt *_variaTxt;

	uint32 _talkTxtSize;
	byte *_talkTxt;

	uint32 _mobTranslationSize;
	byte *_mobTranslationData;

	bool _missingVoice;

	bool loadLocation(uint16 locationNr);
	bool loadAnim(uint16 animNr, bool loop);
	bool loadVoice(uint32 textSlot, uint32 sampleSlot, const Common::String &name);
	bool loadSample(uint32 sampleSlot, const Common::String &name);
	bool loadZoom(byte *zoomBitmap, uint32 dataSize, const char *resourceName);
	bool loadShadow(byte *shadowBitmap, uint32 dataSize, const char *resourceName1, const char *resourceName2);
	bool loadTrans(byte *transTable, const char *resourceName);
	bool loadMobPriority(const char *resourceName);

	void loadMobTranslationTexts();
	void setMobTranslationTexts();

	bool loadMusic(int musNumber);
	void stopMusic();

	void playSample(uint16 sampleId, uint16 loopType);
	void stopSample(uint16 sampleId);
	void stopAllSamples();
	void freeSample(uint16 sampleId);
	void freeAllSamples();

	void setVoice(uint16 slot, uint32 sampleSlot, uint16 flag);

	void changeCursor(uint16 curId);
	void printAt(uint32 slot, uint8 color, char *s, uint16 x, uint16 y);
	int calcTextLines(const char *s);
	int calcTextTime(int numberOfLines);
	void correctStringDEU(char *s);

	static const uint8 kMaxTexts = 32;
	Text _textSlots[kMaxTexts];

	Hero *_mainHero;
	Hero *_secondHero;

	enum HeroId {
		kMainHero,
		kSecondHero
	};

	int _mouseFlag;
	uint32 _currentTime;
	uint16 _locationNr;
	uint16 _sceneWidth;
	int32 _picWindowX;
	int32 _picWindowY;

	Image::BitmapDecoder *_roomBmp;
	MhwanhDecoder *_suitcaseBmp;
	Room *_room;
	Script *_script;
	InterpreterFlags *_flags;
	Interpreter *_interpreter;
	GraphicsMan *_graph;
	uint8 _currentMidi;
	byte *_zoomBitmap;
	byte *_shadowBitmap;
	byte *_transTable;

	int16 _scaleValue; // scale for hero or special shadow animation
	int16 _lightX; // for hero shadow
	int16 _lightY;
	int32 _shadScaleValue;
	int32 _shadLineLen;
	byte *_shadowLine;
	void setShadowScale(int32 shadowScale);
	static void plotShadowLinePoint(int x, int y, int color, void *data);

	static const int16 kFPS = 15;
	static const int32 kIntMax = 2147483647;

	static const int16 kMaxPicWidth = 1280;
	static const int16 kMaxPicHeight = 480;
	static const int16 kZoomStep = 4;
	static const int32 kZoomBitmapLen = kMaxPicHeight / kZoomStep * kMaxPicWidth / kZoomStep;
	static const int32 kShadowBitmapSize = kMaxPicWidth * kMaxPicHeight / 8;
	static const int16 kShadowLineArraySize = 2 * 1280 * 4;
	static const int16 kZoomBitmapWidth = kMaxPicWidth / kZoomStep;
	static const int16 kZoomBitmapHeight = kMaxPicHeight / kZoomStep;
	static const int16 kNormalWidth = 640;
	static const int16 kNormalHeight = 480;
	static const uint32 kTransTableSize = 256 * 256;

	static const int kMaxNormAnims = 64;
	static const int kMaxBackAnims = 64;
	static const int kMaxObjects = 64;
	static const int kMaxMobs = 64;

	Common::Array<DrawNode> _drawNodeList;
	Common::Array<AnimListItem> _animList;
	Common::Array<BackgroundAnim> _backAnimList;
	Common::Array<Anim> _normAnimList;
	Common::Array<Mob> _mobList;
	Common::Array<uint32> _mobPriorityList;
	Common::Array<Mask> _maskList;
	Common::Array<Object *> _objList;
	uint16 *_objSlot;

	void freeNormAnim(int slot);
	void freeAllNormAnims();
	void removeSingleBackAnim(int slot);

	Common::RandomSource _randomSource;

	void checkMasks(int x1, int y1, int sprWidth, int sprHeight, int z);
	void insertMasks(Graphics::Surface *originalRoomSurface);
	void showMask(int maskNr, Graphics::Surface *originalRoomSurface);
	void clsMasks();

	void grabMap();

	int _selectedMob; // number of selected Mob / inventory item
	int _selectedItem; // number of item on mouse cursor
	int _selectedMode;
	int _currentPointerNumber;

	static const int16 kMaxInv = 90; // max amount of inventory items in whole game
	static const uint16 kMaxItems = 30; // size of inventory

	uint32 _invTxtSize;
	byte *_invTxt;

	Graphics::Surface *_optionsPic;
	Graphics::Surface *_optionsPicInInventory;

	bool _optionsFlag;
	int _optionEnabled;
	int _optionsMob;
	int _optionsX;
	int _optionsY;
	int _optionsWidth;
	int _optionsHeight;
	int _invOptionsWidth;
	int _invOptionsHeight;
	int _optionsStep;
	int _invOptionsStep;
	int _optionsNumber;
	int _invOptionsNumber;
	int _optionsColor1; // color for non-selected options
	int _optionsColor2; // color for selected option

	bool _showInventoryFlag;
	int _invExamY;
	bool _inventoryBackgroundRemember;
	int _invLineX;
	int _invLineY;
	int _invLine;  // number of items in one line
	int _invLines; // number of lines with inventory items
	int _invLineW;
	int _invLineH;
	int _maxInvW;
	int _maxInvH;
	int _invLineSkipX;
	int _invLineSkipY;
	int _invX1;
	int _invY1;
	int _invWidth;
	int _invHeight;
	bool _invCurInside;
	int _mst_shadow;
	int _mst_shadow2; // blinking after adding new item
	int _candleCounter; // special counter for candle inventory item
	int _invMaxCount; // time to turn inventory on
	int _invCounter; // turning on counter

	void inventoryFlagChange(bool inventoryState);
	bool loadAllInv();
	void rememberScreenInv();
	void prepareInventoryToView();
	void drawInvItems();
	void displayInventory();
	void addInv(int heroId, int item, bool addItemQuiet);
	void remInv(int heroId, int item);
	void clearInv(int heroId);
	void swapInv(int heroId);
	void addInvObj();
	void makeInvCursor(int itemNr);
	void enableOptions(bool checkType);
	void checkOptions();
	void checkInvOptions();
	void openInventoryCheck();

	void leftMouseButton();
	void rightMouseButton();
	void inventoryLeftMouseButton();
	void inventoryRightMouseButton();
	void dialogLeftMouseButton(byte *string, int dialogSelected);

	uint32 _dialogDatSize;
	byte *_dialogDat;
	byte *_dialogData; // on, off flags for lines of dialog text

	byte *_dialogBoxAddr[32]; // adresses of dialog windows
	byte *_dialogOptAddr[32]; // adresses of dialog options
	int _dialogOptLines[4 * 32]; // numbers of initial dialog lines

	byte *_dialogText;
	int _dialogLines;
	bool _dialogFlag;
	int _dialogWidth;
	int _dialogHeight;
	int _dialogLineSpace;
	int _dialogColor1; // color for non-selected options
	int _dialogColor2; // color for selected option
	Graphics::Surface *_dialogImage;

	void createDialogBox(int dialogBoxNr);
	void dialogRun();
	void talkHero(int slot);
	void doTalkAnim(int animNumber, int slot, AnimType animType);

	static const uint8 zoomInStep = 8;
	void initZoomIn(int slot);
	void initZoomOut(int slot);
	void doZoomIn(int slot);
	void doZoomOut(int slot);
	void freeZoomObject(int slot);

	static const uint8 kFadeStep = 4;
	void blackPalette();
	void setPalette(const byte *palette);

	int getMob(Common::Array<Mob> &mobList, bool usePriorityList, int posX, int posY);

	// 'Throw a rock' mini-game:
	static const int16 kCurveLen = 17;
	static const int kCelStep = 4;
	int16 *_curveData;
	int _curvPos;
	void makeCurve();
	void getCurve();
	void mouseWeirdo();

	static const uint16 kPowerBarPosX = 288;
	static const uint16 kPowerBarPosY = 430;
	static const uint8 kPowerBarWidth = 64;
	static const uint8 kPowerBarHeight = 16;
	static const uint8 kPowerBarBackgroundColor = 0;
	static const uint16 kPowerBarGreenPosY = 434;
	static const uint8 kPowerBarGreenColor1 = 202;
	static const uint8 kPowerBarGreenColor2 = 235;
	static const uint8 kPowerBarGreenHeight = 8;
	void showPower();

	// Pathfinding
	static const int16 kPathGridStep = 2;
	static const uint32 kPathBitmapLen = (kMaxPicHeight / kPathGridStep * kMaxPicWidth / kPathGridStep) / 8;
	static const int32 kTracePts = 8000;
	static const int32 kPBW = kMaxPicWidth / 16; // PathBitmapWidth
	static const int kMinDistance = 2500;

	byte *_roomPathBitmap; // PL - Sala
	byte *_roomPathBitmapTemp; // PL - SSala
	byte *_coordsBufEnd;
	byte *_coordsBuf; // optimal path
	byte *_coords; // last path point adress from coordsBuf
	byte *_coordsBuf2;
	byte *_coords2;
	byte *_coordsBuf3;
	byte *_coords3;
	int _traceLineLen;
	bool _traceLineFirstPointFlag; // if plotTraceLine after first point
	bool _tracePointFirstPointFlag; // if plotTracePoint after first point
	byte *_directionTable;
	int _shanLen;

	byte *_checkBitmapTemp;
	byte *_checkBitmap;
	int _checkMask;
	int _checkX;
	int _checkY;

	byte *_rembBitmapTemp;
	byte *_rembBitmap;
	int _rembMask;
	int _rembX;
	int _rembY;

	int _fpX;
	int _fpY;

	int drawLine(int x0, int y0, int x1, int y1, int (*plotProc)(int, int, void *), void *data);
	bool loadPath(const char *resourceName);
	byte *makePath(int heroId, int currX, int currY, int destX, int destY);
	void findPoint(int x, int y);
	int getPixelAddr(byte *pathBitmap, int x, int y);
	static int plotTraceLine(int x, int y, void *data);
	void specialPlotInside(int x, int y);
	bool tracePath(int x1, int y1, int x2, int y2);
	Direction makeDirection(int x1, int y1, int x2, int y2);
	void specialPlot(int x, int y);
	void specialPlot2(int x, int y);
	void allocCoords2();
	void freeCoords2();
	void freeCoords3();
	static int plotTracePoint(int x, int y, void *data);
	void specialPlotInside2(int x, int y);
	void approxPath();
	void freeDirectionTable();
	void scanDirections();
	int scanDirectionsFindNext(byte *coords, int xDiff, int yDiff);
	void moveShandria();
	void walkTo();
	void moveRunHero(int heroId, int x, int y, int dir, bool runHeroFlag);

	int leftDownDir();
	int leftDir();
	int leftUpDir();
	int rightDownDir();
	int rightDir();
	int rightUpDir();
	int upLeftDir();
	int upDir();
	int upRightDir();
	int downLeftDir();
	int downDir();
	int downRightDir();

	int cpe();
	int checkLeftDownDir();
	int checkLeftDir();
	int checkDownDir();
	int checkUpDir();
	int checkRightDir();
	int checkLeftUpDir();
	int checkRightDownDir();
	int checkRightUpDir();

private:
	bool playNextFLCFrame();
	void keyHandler(Common::Event event);
	int checkMob(Graphics::Surface *screen, Common::Array<Mob> &mobList, bool usePriorityList);
	void drawScreen();
	void showTexts(Graphics::Surface *screen);
	void init();
	void showLogo();
	void showAnim(Anim &anim);
	void showNormAnims();
	void setBackAnim(Anim &backAnim);
	void showBackAnims();
	void clearBackAnimList();
	bool spriteCheck(int sprWidth, int sprHeight, int destX, int destY);
	void showSprite(Graphics::Surface *spriteSurface, int destX, int destY, int destZ);
	void showSpriteShadow(Graphics::Surface *shadowSurface, int destX, int destY, int destZ);
	void showObjects();
	void showParallax();
	static bool compareDrawNodes(DrawNode d1, DrawNode d2);
	void runDrawNodes();
	void makeShadowTable(int brightness);
	void pausePrinceEngine(int fps = kFPS);

	uint32 getTextWidth(const char *s);
	void debugEngine(const char *s, ...);

	uint8 _cursorNr;

	Common::RandomSource *_rnd;
	Cursor *_cursor1;
	Graphics::Surface *_cursor2;
	Cursor *_cursor3;
	Debugger *_debugger;
	Font *_font;
	MusicPlayer *_midiPlayer;

	static const int kMaxSamples = 60;
	Audio::RewindableAudioStream *_audioStream[kMaxSamples];
	Audio::SoundHandle _soundHandle[kMaxSamples];

	Common::Array<PScr *> _pscrList;
	Common::Array<InvItem> _allInvList;
	Common::Array<Mob> _invMobList;

	bool _flicLooped;

	void mainLoop();

};

} // End of namespace Prince

#endif
