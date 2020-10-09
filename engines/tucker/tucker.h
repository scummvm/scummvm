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

#ifndef TUCKER_TUCKER_H
#define TUCKER_TUCKER_H

#include "common/file.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/stream.h"

#include "video/flic_decoder.h"

#include "audio/mixer.h"

#include "engines/engine.h"

#include "tucker/console.h"
#include "tucker/detection.h"

namespace Audio {
class RewindableAudioStream;
} // End of namespace Audio

/**
 * This is the namespace of the Tucker engine.
 *
 * Status of this engine: Complete
 *
 * Games using this engine:
 * - Bud Tucker in Double Trouble
 */
namespace Tucker {

enum CursorStyle {
	kCursorNormal     = 0,
	kCursorTalk       = 1,
	kCursorArrowRight = 2,
	kCursorArrowUp    = 3,
	kCursorArrowLeft  = 4,
	kCursorArrowDown  = 5,
	kCursorMap        = 6
};

enum CursorState {
	kCursorStateNormal         = 0,
	kCursorStateDialog         = 1,
	kCursorStateDisabledHidden = 2
};

enum PanelState {
	kPanelStateNormal    = 0,
	kPanelStateShrinking = 1,
	kPanelStateExpanding = 2
};

enum PanelStyle {
	kPanelStyleVerbs = 0,
	kPanelStyleIcons = 1
};

enum PanelType {
	kPanelTypeNormal           = 0,
	kPanelTypeEmpty            = 1,
	kPanelTypeLoadSavePlayQuit = 2,
	kPanelTypeLoadSaveSavegame = 3
};

enum Verb {
	kVerbWalk  = 0,
	kVerbLook  = 1,
	kVerbTalk  = 2,
	kVerbOpen  = 3,
	kVerbClose = 4,
	kVerbGive  = 5,
	kVerbTake  = 6,
	kVerbMove  = 7,
	kVerbUse   = 8,

	kVerbFirst = kVerbWalk,
	kVerbLast  = kVerbUse
};

enum VerbPreposition {
	kVerbPrepositionNone = 0,

	kVerbPrepositionWith = 11,
	kVerbPrepositionTo   = 12
};

enum Part {
	kPartInit  = 0,
	kPartOne   = 1,
	kPartTwo   = 2,
	kPartThree = 3
};

enum Location {
	kLocationNone                      =  0,

	kLocationHotelRoom                 =  1,
	kLocationBackAlley                 =  2,
	kLocationSeedyStreet               =  3,
	kLocationBakersShop                =  4,
	kLocationBakersKitchen             =  5,
	kLocationStripJoint                =  6,
	kLocationPoliceHQ                  =  7,
	kLocationPoliceCell                =  8,
	kLocationMall                      =  9,
	kLocationFishShop                  = 10,
	kLocationBurgerJoint               = 11,
	kLocationRecordShop                = 12,
	kLocationDentist                   = 13,
	kLocationPlugShop                  = 14,
	kLocationTouristInfo               = 15,
	kLocationPark                      = 16,
	kLocationRoystonsHomeHallway       = 17,
	kLocationRoystonsHomeBoxroom       = 18,
	kLocationDocks                     = 19,
	kLocationOutsideMuseum             = 20,
	kLocationInsideMuseum              = 21,
	kLocationFishingTrawler            = 22,
	kLocationWarehouseCutscene         = 23,
	kLocationStoreRoom                 = 24,
	kLocationVentSystem                = 25,
	kLocationSubwayTunnel              = 26,
	kLocationStrangeRoom               = 27,
	kLocationTopCorridor               = 28,
	kLocationSubmarineHangar           = 29,
	kLocationBunkRoom                  = 30,
	kLocationBottomCorridor            = 31,
	kLocationKitchen                   = 32,
	kLocationCommandCentre             = 33,
	kLocationSubmarineHatch            = 34,
	kLocationSubmarineWalkway          = 35,
	kLocationSubmarineBridge           = 36,
	kLocationSubmarineOffice           = 37,
	kLocationSubmarineEngineRoom       = 38,
	kLocationLuxuryApartment           = 39,
	kLocationFarDocks                  = 40,
	kLocationAlleyway                  = 41,
	kLocationBasement                  = 42,
	kLocationTateTowerEntrance         = 43,
	kLocationRooftop                   = 44,
	kLocationConferenceRoom            = 45,
	kLocationAnteChamber               = 46,
	kLocationHelipad                   = 47,
	kLocationCorridor                  = 48,
	kLocationWaitingRoom               = 49,
	kLocationkLocationCorridorCutscene = 50,
	kLocationCells                     = 51,
	kLocationMachineRoom               = 52,
	kLocationRecordShopPartThree       = 53,
	kLocationPlugShopPartThree         = 54,
	kLocationTouristInfoPartThree      = 55,
	kLocationDentistPartThree          = 56,
	kLocationFishShopPartThree         = 57,
	kLocationInsideMuseumPartThree     = 58,
	kLocationBakersShopPartThree       = 59,
	kLocationStripJointPartThree       = 60,
	kLocationParkPartThree             = 61,
	kLocationDocksPartThree            = 62,
	kLocationTV                        = 63,
	kLocationSewer                     = 64,
	kLocationSeedyStreetPartThree      = 65,
	kLocationMallPartThree             = 66,
	kLocationBurgerJointPartThree      = 67,
	kLocationOutsideMuseumPartThree    = 68,
	kLocation69Cutscene                = 69,
	kLocationComputerScreen            = 70,
	kLocationParkCutscene              = 71,
	kLocationSeedyStreetCutscene       = 72,
	kLocationJesusCutscene1            = 73,
	kLocationCredits                   = 74,
	kLocation75Cutscene                = 75,
	kLocationBeachCutscene             = 76,
	kLocationHospitalCutscene          = 77,
	kLocation78Cutscene                = 78,
	kLocationElvisCutscene             = 79,
	kLocationPyramidCutscene           = 80,
	kLocationCleopatraCutscene         = 81,
	kLocationJesusCutscene2            = 82,

	kLocationNewPart                   = 98,
	kLocationMap                       = 99,

	kLocationInit                      =  1,
	kLocationInitDemo                  =  9
};

struct Action {
	int _key;
	int _testFlag1Num;
	int _testFlag1Value;
	int _testFlag2Num;
	int _testFlag2Value;
	int _speech;
	int _flipX;
	int _index;
	int _delay;
	int _setFlagNum;
	int _setFlagValue;
	int _fxNum;
	int _fxDelay;
};

struct Sprite {
	int _state;
	int _gfxBackgroundOffset;
	int _updateDelay;
	int _backgroundOffset;
	int _stateIndex;
	int _counter;
	int _colorType;
	int _animationFrame;
	int _firstFrame;
	uint8 *_animationData;
	int _prevState;
	int _defaultUpdateDelay;
	int _xSource;
	int _yMaxBackground;
	bool _disabled;
	bool _flipX;
	bool _needUpdate;
	bool _nextAnimationFrame;
	bool _prevAnimationFrame;
};

struct CharPos {
	int _xPos;
	int _yPos;
	int _xSize;
	int _ySize;
	int _xWalkTo;
	int _yWalkTo;
	int _flagNum;
	int _flagValue;
	int _direction;
	int _name;
	int _description;
};

struct SpriteFrame {
	int _sourceOffset;
	int _xOffset;
	int _yOffset;
	int _xSize;
	int _ySize;
};

struct SpriteAnimation {
	int _numParts;
	int _rotateFlag;  // Useless variable
	int _firstFrameIndex;
};

struct Data {
	int _sourceOffset;
	int _xSize;
	int _ySize;
	int _xDest;
	int _yDest;
	int _index;
};

struct LocationAnimation {
	int _graphicNum;
	int _animInitCounter;
	int _animCurrentCounter;
	int _animLastCounter;
	int _getFlag;
	int _inventoryNum;
	int _flagNum;
	int _flagValue;
	int _selectable;
	int _standX;
	int _standY;
	bool _drawFlag;
};

struct LocationObject {
	int _xPos;
	int _yPos;
	int _xSize;
	int _ySize;
	int _textNum;
	Location _location;
	int _toX;
	int _toY;
	int _toX2;
	int _toY2;
	int _toWalkX2;
	int _toWalkY2;
	int _standX;
	int _standY;
	CursorStyle _cursorStyle;
};

struct LocationSound {
	int _startFxSpriteState;
	int _startFxSpriteNum;
	int _updateType;
	int _stopFxSpriteState;
	int _stopFxSpriteNum;
	int _offset;
	int _type;
	int _volume;
	int _flagValueStartFx;
	int _flagValueStopFx;
	int _flagNum;
	int _num;
};

struct LocationMusic {
	int _flag;
	int _offset;
	int _volume;
	int _num;
};

enum {
	kScreenWidth = 320,
	kScreenHeight = 200,
	kScreenPitch = 640,
	kFadePaletteStep = 5,
	kDefaultCharSpeechSoundCounter = 1,
	kMaxSoundVolume = 127,
	kLastSaveSlot = 99,
	kAutoSaveSlot = kLastSaveSlot
};

enum InputKey {
	kInputKeyPause = 0,
	kInputKeyEscape,
	kInputKeyTogglePanelStyle,
	kInputKeyToggleTextSpeech,
	kInputKeyHelp,
	kInputKeySkipSpeech,

	kInputKeyCount
};

enum CompressedSoundType {
	kSoundTypeFx,
	kSoundTypeMusic,
	kSoundTypeSpeech,
	kSoundTypeIntro
};

class CompressedSound {
public:

	CompressedSound() : _compressedSoundType(-1), _compressedSoundFlags(0) {}

	void openFile();
	void closeFile();
	Audio::RewindableAudioStream *load(CompressedSoundType type, int num);

private:

	int _compressedSoundType;
	uint16 _compressedSoundFlags;
	Common::File _fCompressedSound;
};

inline int scaleMixerVolume(int volume, int max = 100) {
	if (volume > max) {
		// This happens for instance for Bud's line, "Is that the
		// great mystery invention you had hidden away?" in the intro,
		// which is played at volume 110 out of 100. This made it very
		// hard to hear. I'm not sure if this was a bug in the original
		// game, or if it had the ability to amplify sounds.
		warning("scaleMixerVolume: Adjusting volume %d to %d", volume, max);
		volume = max;
	}
	return volume * Audio::Mixer::kMaxChannelVolume / max;
}

Common::String generateGameStateFileName(const char *target, int slot, bool prefixOnly = false);

class AnimationSequencePlayer;

class TuckerEngine: public Engine {
public:

	enum {
		kLocationAnimationsTableSize = 20,
		kLocationObjectsTableSize = 10,
		kActionsTableSize = 100,
		kFlagsTableSize = 300,
		kLocationSoundsTableSize = 30,
		kLocationMusicsTableSize = 4,
		kSpriteFramesTableSize = 200,
		kSprA02TableSize = 20,
		kSprC02TableSize = 40,
		kDataTableSize = 500,
		kSpeechHistoryTableSize = 5,
		kMaxCharacters = 8,
		kMaxDirtyRects = 32
	};

	struct SavegameHeader {
		uint16 version;
		uint32 flags;
		Common::String description;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	enum SavegameError {
		kSavegameNoError = 0,
		kSavegameInvalidTypeError,
		kSavegameInvalidVersionError,
		kSavegameNotFoundError,
		kSavegameIoError
	};

	TuckerEngine(OSystem *system, Common::Language language, uint32 flags);
	~TuckerEngine() override;

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

	WARN_UNUSED_RESULT static SavegameError readSavegameHeader(Common::InSaveFile *file, SavegameHeader &header, bool skipThumbnail = true);
	WARN_UNUSED_RESULT static SavegameError readSavegameHeader(const char *target, int slot, SavegameHeader &header);
	virtual bool canSaveAutosaveCurrently() override;

	static bool isAutosaveAllowed(const char *target);
protected:

	int getRandomNumber();
	void allocateBuffers();
	void freeBuffers();
	void resetVariables();
	void mainLoop();
	void waitForTimer(int ticksCount);
	void parseEvents();
	void updateCursorPos(int x, int y);
	void setCursorStyle(CursorStyle style);
	void setCursorState(CursorState state);
	void showCursor(bool visible);
	void setupNewLocation();
	void copyLocBitmap(const char *filename, int offset, bool isMask);
	void updateMouseState();
	void updateCharPositionHelper();
	void updateCharPosition();
	void updateFlagsForCharPosition();
	void fadeOutPalette(int colorsCount = 256);
	void fadeInPalette(int colorsCount = 256);
	void fadePaletteColor(int color, int step);
	void setBlackPalette();
	void updateCursor();
	void stopSounds();
	void playSounds();
	void updateCharactersPath();
	void setSoundVolumeDistance();
	void updateData3DrawFlag();
	void updateData3();
	void updateSfxData3_1();
	void updateSfxData3_2();
	void saveOrLoad();
	void handleMouseOnPanel();
	void togglePanelStyle();
	void redrawPanelOverBackground();
	void drawConversationTexts();
	void updateScreenScrolling();
	void updateGameHints();
	void startCharacterSounds();
	void updateSoundsTypes3_4();
	void drawData3();
	void execData3PreUpdate();
	void drawBackgroundSprites();
	void drawCurrentSprite();
	void setVolumeSound(int index, int volume);
	void setVolumeMusic(int index, int volume);
	void startSound(int offset, int index, int volume);
	void stopSound(int index);
	bool isSoundPlaying(int index);
	void startMusic(int offset, int index, int volume);
	void stopMusic(int index);
	void startSpeechSound(int num, int volume);
	void stopSpeechSound();
	bool isSpeechSoundPlaying();
	void rememberSpeechSound();
	void redrawPanelItems();
	void redrawPanelItemsHelper();
	void drawSprite(int i);
	void clearItemsGfx();
	void drawPausedInfoBar();
	const uint8 *getStringBuf(int type) const;
	void drawInfoString();
	void drawGameHintString();
	void updateCharacterAnimation();
	void execData3PostUpdate();
	void addObjectToInventory(int num);
	void removeObjectFromInventory(int num);
	void handleMap();
	void clearSprites();
	void updateSprites();
	void updateSprite(int i);
	void drawStringInteger(int num, int x, int y, int digits);
	void drawStringAlt(int x, int y, int color, const uint8 *str, int strLen = -1);
	void drawItemString(int x, int num, const uint8 *str);
	void drawCreditsString(int x, int y, int num);
	void updateCharSpeechSound(bool displayText);
	void updateItemsGfxColors(int bit0, int bit7);
	bool testLocationMask(int x, int y);
	int getStringWidth(int num, const uint8 *ptr);
	int getPositionForLine(int num, const uint8 *ptr);
	void resetCharacterAnimationIndex(int count);
	int readTableInstructionCode(int *index);
	int readTableInstructionParam(int len);
	int executeTableInstruction();
	void moveUpInventoryObjects();
	void moveDownInventoryObjects();
	void setActionVerbUnderCursor();
	int getObjectUnderCursor();
	void setSelectedObjectKey();
	void setCharacterAnimation(int count, int spr);
	bool testLocationMaskArea(int xBase, int yBase, int xPos, int yPos);
	void handleMouseClickOnInventoryObject();
	int setCharacterUnderCursor();
	int setLocationAnimationUnderCursor();
	void setActionForInventoryObject();
	void setActionState();
	void playSpeechForAction(int i);
	void drawSpeechText(int xStart, int y, const uint8 *dataPtr, int num, int color);
	bool splitSpeechTextLines(const uint8 *dataPtr, int pos, int x, int &lineCharsCount, int &lineWidth);
	void drawSpeechTextLine(const uint8 *dataPtr, int pos, int count, int x, int y, uint8 color);
	void redrawScreen(int offset);
	void redrawScreenRect(const Common::Rect &clip, const Common::Rect &dirty);
	void addDirtyRect(int x, int y, int w, int h);

	void execData3PreUpdate_locationNum1();
	void execData3PreUpdate_locationNum1Helper1();
	void execData3PreUpdate_locationNum1Helper2();
	int execData3PreUpdate_locationNum1Helper3(int dx, int dy);
	void execData3PostUpdate_locationNum1();
	void updateSprite_locationNum2();
	void execData3PreUpdate_locationNum2();
	void execData3PreUpdate_locationNum2Helper();
	void updateSprite_locationNum3_0(int i);
	void updateSprite_locationNum3_1(int i);
	void updateSprite_locationNum3_2(int i);
	void execData3PreUpdate_locationNum3();
	void updateSprite_locationNum4(int i);
	void execData3PreUpdate_locationNum4();
	void updateSprite_locationNum5_0();
	void updateSprite_locationNum5_1(int i);
	void updateSprite_locationNum6_0(int i);
	void updateSprite_locationNum6_1(int i);
	void updateSprite_locationNum6_2(int i);
	void execData3PreUpdate_locationNum6();
	void execData3PreUpdate_locationNum6Helper1();
	void execData3PreUpdate_locationNum6Helper2(int dstOffset, const uint8 *src);
	void execData3PreUpdate_locationNum6Helper3(int dstOffset, const uint8 *src);
	void execData3PostUpdate_locationNum6();
	void updateSprite_locationNum7_0(int i);
	void updateSprite_locationNum7_1(int i);
	void updateSprite_locationNum8_0(int i);
	void updateSprite_locationNum8_1(int i);
	void execData3PostUpdate_locationNum8();
	void updateSprite_locationNum9_0(int i);
	void updateSprite_locationNum9_1(int i);
	void updateSprite_locationNum9_2(int i);
	void execData3PreUpdate_locationNum9();
	void execData3PostUpdate_locationNum9();
	void updateSprite_locationNum10();
	void execData3PreUpdate_locationNum10();
	void updateSprite_locationNum11_0(int i);
	void updateSprite_locationNum11_1(int i);
	void updateSprite_locationNum11_2(int i);
	void updateSprite_locationNum11_3(int i);
	void updateSprite_locationNum11_4(int i);
	void updateSprite_locationNum12_0(int i);
	void updateSprite_locationNum12_1(int i);
	void execData3PreUpdate_locationNum12();
	void updateSprite_locationNum13(int i);
	void execData3PreUpdate_locationNum13();
	void updateSprite_locationNum14(int i);
	void execData3Update_locationNum14();
	void execData3PreUpdate_locationNum14();
	void execData3PreUpdate_locationNum14Helper1(int i);
	void execData3PreUpdate_locationNum14Helper2(int i);
	void execData3PostUpdate_locationNum14();
	void updateSprite_locationNum15_0(int i);
	void updateSprite_locationNum15_1(int i);
	void updateSprite_locationNum15_2(int i);
	void execData3PreUpdate_locationNum15();
	void updateSprite_locationNum16_0(int i);
	void updateSprite_locationNum16_1(int i);
	void updateSprite_locationNum16_2(int i);
	void execData3PreUpdate_locationNum16();
	void updateSprite_locationNum17();
	void updateSprite_locationNum18();
	void updateSprite_locationNum19_0(int i);
	void updateSprite_locationNum19_1(int i);
	void updateSprite_locationNum19_2(int i);
	void updateSprite_locationNum19_3(int i);
	void execData3PreUpdate_locationNum19();
	void updateSprite_locationNum21();
	void execData3PreUpdate_locationNum21();
	void execData3PostUpdate_locationNum21();
	void updateSprite_locationNum22();
	void execData3PreUpdate_locationNum22();
	void updateSprite_locationNum23_0(int i);
	void updateSprite_locationNum23_1(int i);
	void updateSprite_locationNum23_2(int i);
	void updateSprite_locationNum23_3(int i);
	void updateSprite_locationNum24_0(int i);
	void updateSprite_locationNum24_1(int i);
	void updateSprite_locationNum24_2(int i);
	void updateSprite_locationNum24_3(int i);
	void execData3PreUpdate_locationNum24();
	void execData3PostUpdate_locationNum24();
	void execData3PreUpdate_locationNum25();
	void updateSprite_locationNum26_0(int i);
	void updateSprite_locationNum26_1(int i);
	void execData3PreUpdate_locationNum26();
	void updateSprite_locationNum27(int i);
	void execData3PreUpdate_locationNum27();
	void execData3PostUpdate_locationNum27();
	void updateSprite_locationNum28_0(int i);
	void updateSprite_locationNum28_1(int i);
	void updateSprite_locationNum28_2(int i);
	void execData3PreUpdate_locationNum28();
	void execData3PostUpdate_locationNum28();
	void updateSprite_locationNum29_0(int i);
	void updateSprite_locationNum29_1(int i);
	void updateSprite_locationNum29_2(int i);
	void execData3PreUpdate_locationNum29();
	void updateSprite_locationNum30_34(int i);
	void execData3PreUpdate_locationNum30();
	void updateSprite_locationNum31_0(int i);
	void updateSprite_locationNum31_1(int i);
	void execData3PreUpdate_locationNum31();
	void updateSprite_locationNum32_0(int i);
	void execData3PreUpdate_locationNum32();
	void execData3PostUpdate_locationNum32();
	void updateSprite_locationNum33_0(int i);
	void updateSprite_locationNum33_1(int i);
	void updateSprite_locationNum33_2(int i);
	void execData3PreUpdate_locationNum33();
	void execData3PreUpdate_locationNum34();
	void execData3PreUpdate_locationNum35();
	void updateSprite_locationNum36(int i);
	void execData3PreUpdate_locationNum36();
	void updateSprite_locationNum37(int i);
	void execData3PreUpdate_locationNum38();
	void updateSprite_locationNum41(int i);
	void execData3PreUpdate_locationNum41();
	void updateSprite_locationNum42(int i);
	void execData3PreUpdate_locationNum42();
	void updateSprite_locationNum43_2(int i);
	void updateSprite_locationNum43_3(int i);
	void updateSprite_locationNum43_4(int i);
	void updateSprite_locationNum43_5(int i);
	void updateSprite_locationNum43_6(int i);
	void execData3PreUpdate_locationNum43();
	void execData3PreUpdate_locationNum44();
	void updateSprite_locationNum48(int i);
	void updateSprite_locationNum49(int i);
	void execData3PreUpdate_locationNum49();
	void updateSprite_locationNum50(int i);
	void updateSprite_locationNum51(int i);
	void execData3PreUpdate_locationNum52();
	void updateSprite_locationNum53_0(int i);
	void updateSprite_locationNum53_1(int i);
	void execData3PreUpdate_locationNum53();
	void updateSprite_locationNum54(int i);
	void updateSprite_locationNum55(int i);
	void updateSprite_locationNum56(int i);
	void updateSprite_locationNum57_0(int i);
	void updateSprite_locationNum57_1(int i);
	void execData3PreUpdate_locationNum57();
	void updateSprite_locationNum58(int i);
	void execData3PreUpdate_locationNum58();
	void updateSprite_locationNum59(int i);
	void updateSprite_locationNum60_0(int i);
	void updateSprite_locationNum60_1(int i);
	void execData3PostUpdate_locationNum60();
	void updateSprite_locationNum61_0(int i);
	void updateSprite_locationNum61_1(int i);
	void updateSprite_locationNum61_2(int i);
	void execData3PreUpdate_locationNum61();
	void updateSprite_locationNum63_0(int i);
	void updateSprite_locationNum63_1(int i);
	void updateSprite_locationNum63_2(int i);
	void updateSprite_locationNum63_3(int i);
	void updateSprite_locationNum63_4(int i);
	void execData3PreUpdate_locationNum63();
	void execData3PreUpdate_locationNum64();
	void updateSprite_locationNum65(int i);
	void execData3PreUpdate_locationNum65();
	void updateSprite_locationNum66_0(int i);
	void updateSprite_locationNum66_1(int i);
	void updateSprite_locationNum66_2(int i);
	void updateSprite_locationNum66_3(int i);
	void updateSprite_locationNum66_4(int i);
	void execData3PreUpdate_locationNum66();
	void execData3PostUpdate_locationNum66();
	void updateSprite_locationNum69_1(int i);
	void updateSprite_locationNum69_2(int i);
	void updateSprite_locationNum69_3(int i);
	void execData3PreUpdate_locationNum70();
	void execData3PreUpdate_locationNum70Helper();
	void updateSprite_locationNum71(int i);
	void updateSprite_locationNum72(int i);
	void updateSprite_locationNum74(int i);
	void updateSprite_locationNum79(int i);
	void updateSprite_locationNum81_0(int i);
	void updateSprite_locationNum81_1(int i);
	void updateSprite_locationNum82(int i);

	template<class S> SavegameError saveOrLoadGameStateData(S &s);
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	SavegameError writeSavegameHeader(Common::OutSaveFile *file, SavegameHeader &header);
	virtual int getAutosaveSlot() const override { return kAutoSaveSlot; }
	virtual Common::String getSaveStateName(int slot) const override {
		return Common::String::format("%s.%d", _targetName.c_str(), slot);
	}

	bool canLoadOrSave() const;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	virtual bool existsSavegame();

	void handleIntroSequence();
	void handleCreditsSequence();
	void handleCongratulationsSequence();
	void handleNewPartSequence();
	void handleMeanwhileSequence();
	void handleMapSequence();
	void copyMapRect(int x, int y, int w, int h);
	bool handleSpecialObjectSelectionSequence();

	uint8 *loadFile(const char *filename, uint8 *p);
	void loadImage(const char *filename, uint8 *dst, int a);
	void loadCursor();
	void loadCharset();
	void loadCharset2();
	void loadCharsetHelper();
	void loadCharSizeDta();
	void loadPanel();
	void loadBudSpr();
	int  loadCTable01(int *framesCount);
	void loadCTable02();
	void loadLoc();
	void loadObj();
	void loadData();
	int  loadDataHelper(int offset, int index);
	void loadPanObj();
	void loadData3();
	void loadData4();
	void loadActionFile();
	void loadCharPos();
	void loadSprA02_01();
	void unloadSprA02_01();
	void loadSprC02_01();
	void unloadSprC02_01();
	void loadFx();
	void loadSound(Audio::Mixer::SoundType type, int num, int volume, bool loop, Audio::SoundHandle *handle);
	void loadActionsTable();

	Common::RandomSource _rnd;
	AnimationSequencePlayer *_player;
	CompressedSound _compressedSound;
	Common::Language _gameLang;
	uint32 _gameFlags;
	int _startSlot;

	bool _quitGame;
	bool _fastMode;
	int _syncCounter;
	uint32 _lastFrameTime;
	int _mainLoopCounter1;
	int _mainLoopCounter2;
	int _timerCounter2;
	int _flagsTable[kFlagsTableSize];
	Part _part;
	Part _currentPart;
	Location _location;
	Location _nextLocation;
	bool _gamePaused;
	bool _gameDebug;
	bool _displayGameHints;
	int _execData3Counter;
	int _currentSaveLoadGameState;
	int _gameHintsIndex;
	int _gameHintsCounter;
	int _gameHintsStringNum;

	bool _displaySpeechText;
	bool _displayHintsText;

	int _fileLoadSize;
	uint8 *_loadTempBuf;
	uint8 *_cursorGfxBuf;
	uint8 *_charsetGfxBuf;
	uint8 *_panelGfxBuf;
	uint8 *_itemsGfxBuf;
	uint8 *_spritesGfxBuf;
	uint8 *_locationBackgroundGfxBuf;
	uint8 *_data5Buf;
	uint8 *_data3GfxBuf;
	uint8 *_quadBackgroundGfxBuf;
	uint8 *_objTxtBuf;
	uint8 *_panelObjectsGfxBuf;
	uint8 *_ptTextBuf;
	uint8 *_infoBarBuf;
	uint8 *_bgTextBuf;
	uint8 *_charNameBuf;
	uint8 *_locationBackgroundMaskBuf;
	uint8 *_csDataBuf;
	int _csDataSize;
	uint8 _charWidthTable[256];
	const uint8 *_locationWidthTable;
	uint8 *_locationHeightTable;

	int _mousePosX, _mousePosY;
	int _prevMousePosX, _prevMousePosY;
	int _mouseButtonsMask;
	int _mouseClick;
	int _saveOrLoadGamePanel;
	int _mouseIdleCounter;
	bool _leftMouseButtonPressed;
	bool _rightMouseButtonPressed;
	bool _mouseWheelUp;
	bool _mouseWheelDown;
	int _lastKeyPressed;
	bool _inputKeys[kInputKeyCount];
	CursorStyle _cursorStyle;
	CursorState _cursorState;
	bool _updateCursorFlag;

	PanelStyle _panelStyle;
	PanelState _panelState;
	PanelType  _panelType;
	bool _forceRedrawPanelItems;
	int _redrawPanelItemsCounter;
	int _panelObjectsOffsetTable[50];
	int _switchPanelCounter;
	int _conversationOptionsCount;
	bool _fadedPanel;
	bool _panelLockedFlag;
	int _conversationOptionLinesCount;
	int _inventoryItemsState[50];
	int _inventoryObjectsList[40];
	int _inventoryObjectsOffset;
	int _inventoryObjectsCount;
	int _lastInventoryObjectIndex;

	uint8 *_sprA02Table[kSprA02TableSize];
	uint8 *_sprC02Table[kSprC02TableSize];
	Action _actionsTable[kActionsTableSize];
	int _actionsCount;
	LocationObject _locationObjectsTable[kLocationObjectsTableSize];
	int _locationObjectsCount;
	Sprite _spritesTable[kMaxCharacters];
	int _spritesCount;
	LocationAnimation _locationAnimationsTable[kLocationAnimationsTableSize];
	int _locationAnimationsCount;
	Data _dataTable[kDataTableSize];
	int _dataCount;
	CharPos _charPosTable[kMaxCharacters];
	int _charPosCount;
	LocationSound _locationSoundsTable[kLocationSoundsTableSize];
	int _locationSoundsCount;
	LocationMusic _locationMusicsTable[kLocationMusicsTableSize];
	int _locationMusicsCount;

	int _currentFxSet;
	int _currentFxDist;
	int _currentFxScale;
	int _currentFxVolume;
	int _currentFxIndex;
	int _speechSoundNum;
	int _speechVolume;
	Audio::SoundHandle _sfxHandles[kLocationSoundsTableSize];
	Audio::SoundHandle _musicHandles[kLocationMusicsTableSize];
	Audio::SoundHandle _speechHandle;
	int _miscSoundFxNum[2];
	int _speechHistoryTable[kSpeechHistoryTableSize];
	int _charSpeechSoundCounter;
	int _miscSoundFxDelayCounter[2];
	int _characterSoundFxDelayCounter;
	int _characterSoundFxNum;
	int _speechSoundBaseNum;

	int _pendingActionIndex;
	int _pendingActionDelay;
	int _charPositionFlagNum;
	int _charPositionFlagValue;
	Verb _actionVerb;
	Verb _currentActionVerb;
	Verb _previousActionVerb;
	int _nextAction;
	int _selectedObjectNum;
	int _selectedObjectType;
	int _selectedCharacterNum;
	int _actionObj1Type, _actionObj2Type;
	int _actionObj1Num, _actionObj2Num;
	bool _actionRequiresTwoObjects;
	bool _actionVerbLocked;
	int _actionPosX;
	int _actionPosY;
	bool _selectedObjectLocationMask;
	struct {
		int _xDefaultPos;
		int _yDefaultPos;
		int _xPos;
		int _yPos;
		Location _locationObjectLocation;
		int _locationObjectToX;
		int _locationObjectToY;
		int _locationObjectToX2;
		int _locationObjectToY2;
		int _locationObjectToWalkX2;
		int _locationObjectToWalkY2;
	} _selectedObject;
	int _selectedCharacterDirection;
	int _selectedCharacter2Num;
	int _currentActionObj1Num, _currentActionObj2Num;
	int _currentInfoString1SourceType, _currentInfoString2SourceType;
	int _speechActionCounterTable[9];
	int _actionCharacterNum;

	bool _csDataLoaded;
	bool _csDataHandled;
	bool _stopActionOnSoundFlag;
	bool _stopActionOnSpeechFlag;
	bool _stopActionOnPanelLock;
	int _csDataTableCount;
	int _stopActionCounter;
	int _actionTextColor;
	int _nextTableToLoadIndex;
	int _nextTableToLoadTable[6];
	int _soundInstructionIndex;
	const uint8 *_tableInstructionsPtr;
	int _tableInstructionObj1Table[6];
	int _tableInstructionObj2Table[6];
	bool _tableInstructionFlag;
	int _tableInstructionItemNum1, _tableInstructionItemNum2;
	int _instructionsActionsTable[6];
	bool _validInstructionId;

	SpriteFrame _spriteFramesTable[kSpriteFramesTableSize];
	SpriteAnimation _spriteAnimationsTable[200];
	int _spriteAnimationFramesTable[500];
	int _spriteAnimationFrameIndex;
	int _backgroundSpriteCurrentFrame;
	int _backgroundSpriteLastFrame;
	int _backgroundSpriteCurrentAnimation;
	bool _disableCharactersPath;
	bool _skipCurrentCharacterDraw;
	int _xPosCurrent;
	int _yPosCurrent;
	const uint8 *_characterSpeechDataPtr;
	int _ptTextOffset;
	int _characterAnimationsTable[200];
	int _characterStateTable[200];
	int _backgroundSprOffset;
	int _mainSpritesBaseOffset;
	int _currentSpriteAnimationLength;
	int _currentSpriteAnimationFrame;
	int _currentSpriteAnimationFrame2;
	int _characterAnimationIndex;
	int _characterFacingDirection;
	int _characterPrevFacingDirection;
	bool _characterBackFrontFacing;
	bool _characterPrevBackFrontFacing;
	int _characterAnimationNum;
	int _noCharacterAnimationChange;
	int _characterSpriteAnimationFrameCounter;
	bool _locationMaskIgnore;
	int _locationMaskType;
	int _locationMaskCounter;
	int _handleMapCounter;
	bool _noPositionChangeAfterMap;
	bool _changeBackgroundSprite;
	bool _updateSpriteFlag1;
	bool _updateSpriteFlag2;

	bool _mirroredDrawing;
	uint8 *_loadLocBufPtr;
	uint8 *_backgroundSpriteDataPtr;
	int _locationHeight;
	int _scrollOffset;
	int _currentGfxBackgroundCounter;
	uint8 *_currentGfxBackground;
	int _fadePaletteCounter;
	uint8 _currentPalette[768];
	bool _fullRedraw;
	int _dirtyRectsPrevCount, _dirtyRectsCount;
	Common::Rect _dirtyRectsTable[kMaxDirtyRects];

	int _updateLocationFadePaletteCounter;
	int _updateLocationCounter;
	int _updateLocationPos;
	int _updateLocationXPosTable[5], _updateLocationYPosTable[5];
	int _updateLocationFlagsTable[5];
	int _updateLocationXPosTable2[10], _updateLocationYPosTable2[10];
	int _updateLocationYMaxTable[3];
	int _updateLocation14Step[10];
	int _updateLocation14ObjNum[10];
	int _updateLocation14Delay[10];
	int _updateLocationCounter2;
	bool _updateLocationFlag;
	int _updateLocation70StringLen;
	uint8 _updateLocation70String[20];

	static const uint8 _locationWidthTableGame[85];
	static const uint8 _locationWidthTableDemo[70];
	static const uint8 _sprA02LookupTable[88];
	static const uint8 _sprC02LookupTable[100];
	static const uint8 _sprC02LookupTable2[100];
	static const int _staticData3Table[1600];
	static uint8 _locationHeightTableGame[80];
	static uint8 _locationHeightTableDemo[70];
	static int _objectKeysPosXTable[80];
	static int _objectKeysPosYTable[80];
	static int _objectKeysLocationTable[80];
	static int _mapSequenceFlagsLocationTable[70];
	static const uint8 _charWidthCharset1[224];
	static const uint8 _charWidthCharset2[58];
};

enum {
	kFirstAnimationSequenceGame = 17,
	kFirstAnimationSequenceDemo = 13
};

enum AnimationSoundType {
	kAnimationSoundType8BitsRAW,
	kAnimationSoundType16BitsRAW,
	kAnimationSoundTypeWAV
};

enum {
	kSoundsList_Seq3_4,
	kSoundsList_Seq9_10,
	kSoundsList_Seq21_20,
	kSoundsList_Seq13_14,
	kSoundsList_Seq15_16,
	kSoundsList_Seq27_28,
	kSoundsList_Seq17_18,
	kSoundsList_Seq19_20
};

struct SoundSequenceData {
	int timestamp;
	int index;
	int num;
	int volume;
	int opcode;
};

struct SoundSequenceDataList {
	int musicIndex;
	int musicVolume;
	int soundList1Count;
	int soundList2Count;
	int soundSeqDataCount;
	const SoundSequenceData *soundSeqData;
};

class AnimationSequencePlayer {
public:

	enum {
		kSequenceFrameTime = 55
	};

	struct SequenceUpdateFunc {
		int num;
		int frameTime;
		void (AnimationSequencePlayer::*load)();
		void (AnimationSequencePlayer::*play)();
	};

	AnimationSequencePlayer(OSystem *system, Audio::Mixer *mixer, Common::EventManager *event, CompressedSound *sound, int num);
	~AnimationSequencePlayer();

	void mainLoop();

private:

	void syncTime();
	void loadSounds(int num);
	Audio::RewindableAudioStream *loadSound(int index, AnimationSoundType type);
	void updateSounds();
	void fadeInPalette();
	void fadeOutPalette();
	void unloadAnimation();
	uint8 *loadPicture(const char *fileName);
	void openAnimation(int index, const char *fileName);
	bool decodeNextAnimationFrame(int index, bool copyDirtyRects = true);
	void loadIntroSeq17_18();
	void playIntroSeq17_18();
	void loadIntroSeq19_20();
	void playIntroSeq19_20();
	void displayLoadingScreen();
	void initPicPart4();
	void drawPicPart4();
	void loadIntroSeq3_4();
	void playIntroSeq3_4();
	void drawPic2Part10();
	void drawPic1Part10();
	void loadIntroSeq9_10();
	void playIntroSeq9_10();
	void loadIntroSeq21_22();
	void playIntroSeq21_22();
	void loadIntroSeq13_14();
	void playIntroSeq13_14();
	void loadIntroSeq15_16();
	void playIntroSeq15_16();
	void loadIntroSeq27_28();
	void playIntroSeq27_28();
	void getRGBPalette(int index);

	OSystem *_system;
	Audio::Mixer *_mixer;
	Common::EventManager *_event;
	CompressedSound *_compressedSound;

	int _seqNum;
	bool _changeToNextSequence;
	const SequenceUpdateFunc *_updateFunc;
	int _updateFuncIndex;
	Video::FlicDecoder _flicPlayer[2];
	uint8 _animationPalette[256 * 3];
	int _soundSeqDataCount;
	int _soundSeqDataIndex;
	const SoundSequenceData *_soundSeqData;
	uint8 *_offscreenBuffer;
	int _updateScreenWidth;
	bool _updateScreenPicture;
	int _updateScreenCounter;
	int _updateScreenIndex;
	int _frameCounter;
	int _frameTime;
	uint32 _lastFrameTime;
	uint8 *_picBufPtr, *_pic2BufPtr;
	Audio::SoundHandle _soundsHandle[15];
	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _musicHandle;

	static const SoundSequenceDataList _soundSeqDataList[];
	static const char *const _audioFileNamesTable[];
};

} // namespace Tucker

#endif
