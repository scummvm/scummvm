/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TOT_H
#define TOT_H

#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "tot/chrono.h"
#include "tot/detection.h"
#include "tot/graphics.h"
#include "tot/mouse.h"
#include "tot/sound.h"
#include "tot/types.h"

namespace Tot {

struct TotGameDescription;

class TotEngine : public Engine {

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	void handleEvent(Common::Event e);
	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	int engineStart();
	int startGame();
	void newGame();
	void resumeGame();

	void changeRoom();
	void loadScrollData(uint roomCode, bool rightScroll, uint horizontalPos, int scrollCorrection);
	void drawText(uint number);
	void displayLoading();
	void runaroundRed();
	void sprites(bool drawCharacter);
	void saveGameToRegister();
	void originalSaveLoadScreen();
	void loadGame(SavedGame game);

	RoomFileRegister *readScreenDataFile(Common::SeekableReadStream *screenDataFile);
	void lookAtObject(byte objectNumber);
	void useInventoryObjectWithInventoryObject(uint obj1, uint obj2);
	void pickupScreenObject();
	void useScreenObject();
	void openScreenObject();
	void closeScreenObject();
	void action();
	void handleAction(byte invPos);
	void loadInventory();
	void obtainName(Common::String &playerName);
	void calculateRoute(byte zone1, byte zone2, bool extraCorrection = false, bool zonavedada = false);
	void wcScene();
	void advanceAnimations(bool barredZone, bool animateMouse);
	void updateSecondaryAnimationDepth();
	void updateMainCharacterDepth();
	void actionLineText(const Common::String &actionLine);
	void initializeObjectFile();
	void saveObjectsData(ScreenObject object, Common::SeekableWriteStream *objectDataStream);
	void scrollLeft(uint horizontalPos);
	void scrollRight(uint horizontalPos);
	TextEntry readTextRegister(uint numRegister);

	void readConversationFile();

	void adjustKey();
	void adjustKey2();
	void animateBat();
	void updateVideo();
	void nicheAnimation(byte nicheDir, int32 bitmap);
	void replaceBackpack(byte obj1, uint obj2);
	void dropObjectInScreen(ScreenObject replacementObject);
	void calculateTrajectory(uint finalX, uint finalY);
	void animatedSequence(uint numSequence);
	void initScreenPointers();

	void loadAnimation(const Common::String &animation);
	void loadCharAnimation();
	void animateGive(uint dir, uint height);
	void animatePickup1(uint dir, uint height);
	void animatePickup2(uint dir, uint height);
	void animateOpen2(uint dir, uint height);

	void loadTV();
	void loadScreen();
	void loadScreenLayerWithDepth(uint coordx, uint coordy, uint bitmapSize, int32 bitmapIndex, uint depth);
	void loadScreenLayer(uint coordx, uint coordy, uint picSize, int32 pic, uint prof);
	void readBitmap(int32 bitmapOffset, byte *bitmap, uint bitmapSize, uint errorCode);
	void updateAltScreen(byte altScreenNumber);

	void freeInventory();
	void updateInventory(byte index);
	void updateObject(uint filePos);
	void readObject(Common::SeekableReadStream *stream, uint objPos, ScreenObject &thisRegObj);
	void saveObject(ScreenObject object, Common::SeekableWriteStream *stream);
	void saveItemRegister();

	void saveTemporaryGame();
	void drawLookAtItem(RoomObjectListEntry obj);
	void putIcon(uint posX, uint posY, uint iconNumber);
	void drawInventory(byte direction, byte limit);
	void generateDiploma(Common::String &photoName);
	void credits();
	void checkMouseGrid();
	void introduction();
	void firstIntroduction();
	void readAlphaGraph(Common::String &data, int length, int x, int y, byte barColor);
	void readAlphaGraphSmall(Common::String &data, int length, int x, int y, byte barColor, byte textColor, char startChar = '\0');
	void displayObjectDescription(const Common::String &text);
	void copyProtection();
	void initialLogo();
	void mainMenu(bool fade);
	void exitToDOS();
	void soundControls();
	void sacrificeScene();
	void ending();
	void loadBat();
	void loadDevil();
	void assembleCompleteBackground(byte *image, uint coordx, uint coordy);
	void assembleScreen(bool scroll = false);
	void disableSecondAnimation();
	void clearGame();

	// vars
	void clearCurrentInventoryObject();
	void initVars();
	void resetGameState();
	void clearVars();

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;
	Tot::GraphicsManager *_graphics = nullptr;
	SoundManager *_sound = nullptr;
	MouseManager *_mouse = nullptr;
	ChronoManager *_chrono = nullptr;

	bool _showMouseGrid = false;
	bool _showScreenGrid = false;
	bool _showGameGrid = false;
	bool _drawObjectAreas = false;

	Common::Language _lang = Common::ES_ESP;

	Common::MemorySeekableReadWriteStream *_conversationData = nullptr;
	Common::MemorySeekableReadWriteStream *_rooms = nullptr;
	Common::MemorySeekableReadWriteStream *_sceneObjectsData = nullptr;

	bool _roomChange = false;
	bool _isTVOn = false;
	bool _isVasePlaced = false;
	bool _isScytheTaken = false;
	bool _isTridentTaken = false;
	bool _isPottersWheelDelivered = false;
	bool _isMudDelivered = false;
	bool _isGreenDevilDelivered = false;
	bool _isRedDevilCaptured = false;
	bool _isPottersManualDelivered = false;
	bool _isCupboardOpen = false;
	bool _isChestOpen = false;
	bool _isTrapSet = false;
	bool _isPeterCoughing = false;

	bool _isSealRemoved = false;
	bool _inGame = false;
	bool _firstTimeDone = false; // Flag for first time run of the game.
	bool _isIntroSeen = false;
	bool _shouldQuitGame = false;
	bool _startNewGame = false; // Flag to initialize game
	bool _continueGame = false; // Flag to resume game
	bool _isSavingDisabled = false;
	bool _isDrawingEnabled = false; // true if sprites should be drawn
	bool _isSecondaryAnimationEnabled = false; // Whether room has secondary animation

	InventoryEntry _inventory[kInventoryIconCount]; // These are the icons currnetly in the inventory
	/**
	 * Keeps an array of all inventory icon bitmaps
	 */
	byte *_inventoryIconBitmaps[kInventoryIconCount];
	/**
	 * Position within inventory
	 */
	byte _inventoryPosition = 0;
	/**
	 * Animation sequences
	 */
	CharacterAnim _mainCharAnimation;
	uint _mainCharFrameSize;
	SecondaryAnim _secondaryAnimation;
	uint _secondaryAnimFrameSize;
	/**
	 * Currently selected action.
	 */
	byte _actionCode = 0;
	/**
	 * Previously selected action.
	 */
	byte _oldActionCode = 0;
	/**
	 * Number of trajectory changes
	 */
	byte _trajectorySteps = 0;
	/**
	 * index of currently selected door.
	 */
	byte _doorIndex = 0;
	/**
	 * 1 first part, 2 second part
	 */
	byte _gamePart = 1;
	/**
	 * Number of frames of secondary animation
	 */
	byte _secondaryAnimationFrameCount = 0;
	/**
	 * Number of directions of the secondary animation
	 */
	byte _secondaryAnimDirCount = 0;
	/**
	 * Data protection control
	 */
	byte _cpCounter = 0, _cpCounter2 = 0;
	/**
	 * Coordinates of target step
	 */
	byte _destinationX = 0, _destinationY = 0;
	/**
	 * Current character facing direction
	 * 0: upwards
	 * 1: right
	 * 2: downwards
	 * 3: left
	 */
	byte _charFacingDirection = 0;

	/**
	 * Width and height of secondary animation
	 */
	uint _secondaryAnimWidth = 0, _secondaryAnimHeight = 0;
	/**
	 * Code of selected object in the backpack
	 */
	uint _backpackObjectCode = 0;
	/**
	 * Auxiliary vars for grid update
	 */
	uint _oldposx = 0, _oldposy = 0;

	/**
	 * Amplitude of movement
	 */
	int _element1 = 0, _element2 = 0;
	/**
	 * Current position of the main character
	 */
	int _characterPosX = 0, _characterPosY = 0;
	/**
	 * Target position of the main character?
	 */
	int _xframe2 = 0, _yframe2 = 0;

	/**
	 * Text map
	 */
	Common::File _verbFile;
	/**
	 * Auxiliary vars with current inventory object name.
	 */
	Common::String _oldInventoryObjectName, _inventoryObjectName;

	/**
	 * Name of player
	 */
	Common::String _characterName = "";

	Common::String _decryptionKey;

	/**
	 * State of the niches in part 2
	 */
	uint _niche[2][4];

	RoomFileRegister *_currentRoomData = nullptr;

	ScreenObject _curObject;
	/**
	 * New movement to execute.
	 */
	Route _mainRoute;

	/**
	 * Matrix of positions for a trajectory between two points
	 */
	Common::Point _trajectory[300];

	/**
	 * Longitude of the trajectory matrix.
	 */
	uint _trajectoryLength = 0;
	/**
	 * Position within the trajectory matrix
	 */
	uint _currentTrajectoryIndex = 0;
	/**
	 * Position within the trajectory matrix for secondary animation
	 */
	uint _currentSecondaryTrajectoryIndex = 0;
	/**
	 * Screen areas
	 */
	byte _currentZone = 0, _targetZone = 0, _oldTargetZone = 0;

	/**
	 * Amplitude of grid slices
	 */
	byte _maxXGrid = 0, _maxYGrid = 0;

	/**
	 * capture of movement grid of secondary animation
	 */
	byte _movementGridForSecondaryAnim[10][10];
	/**
	 * capture of mouse grid of secondary animation
	 */
	byte _mouseGridForSecondaryAnim[10][10];
	/**
	 * movement mask for grid of secondary animation
	 */
	byte _maskGridSecondaryAnim[10][10];
	/**
	 * mouse mask for grid of secondary animation
	 */
	byte _maskMouseSecondaryAnim[10][10];

	bool _list1Complete = false,
		_list2Complete = false,
		_obtainedList1 = false, // whether we've been given list 1
		_obtainedList2 = false; // whether we've been given list 2

	/** Conversation topic unlocks */
	bool _firstTimeTopicA[kCharacterCount] = { false },
		_firstTimeTopicB[kCharacterCount] = { false },
		_firstTimeTopicC[kCharacterCount] = { false },
		_bookTopic[kCharacterCount] = { false },
		_mintTopic[kCharacterCount] = { false };

	bool _caves[5] = { false };

	/**
	 * First and second lists of objects to retrieve in the game
	 */
	uint16 _firstList[5] = { 0 }, _secondList[5] = { 0 };

	int32 _screenSize = 65520;

	/**
	 * Frame number for the animations
	 */
	byte _iframe = 0, _iframe2 = 0;

	/**
	 * Depth of screenobjects
	 */
	ObjectInfo _depthMap[kNumScreenOverlays];
	/**
	 * Bitmaps of screenobjects
	 */
	byte *_screenLayers[kNumScreenOverlays];
	/**
	 * Current frame of main character
	 */
	byte *_curCharacterAnimationFrame;
	/**
	 * Current frame of secondary animation
	 */
	byte *_curSecondaryAnimationFrame;

	/**
	 * Pointer storing the screen as it displays on the game
	 */
	byte *_sceneBackground;

	/**
	 * Dirty patch of screen to repaint on every frame
	 */
	byte *_characterDirtyRect;
	/**
	 * Stores a copy of the background bitmap
	 */
	byte *_backgroundCopy;

	uint _currentRoomNumber = 0;

	bool _isLoadingFromLauncher = false;

	bool _saveAllowed = false;

	/**
	 * Previous positions of the mouse within the screen grid
	 */
	uint _oldGridX = 0, _oldGridY = 0;

	SavedGame _savedGame;

	uint _curDepth = 0;
	/**
	 * Point of origin of the area surrounding the main character.
	 * Calculated using the position of the character.
	 */
	uint _dirtyMainSpriteX = 0, _dirtyMainSpriteY = 0;
	/**
	 * End point of origin of the area surrounding the main character.
	 * Calculated using the position of the character + dimension
	 */
	uint _dirtyMainSpriteX2 = 0, _dirtyMainSpriteY2 = 0;
	byte *_spriteBackground;
public:
	TotEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~TotEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher);
	};

	void drawFontSurface(Graphics::BgiFont &litt);

	// Save/Load
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	void loadScreenData(uint screenNumber);
	void clearScreenLayers();
	void clearAnimation();
	void buttonBorder(uint x1, uint y1, uint x2, uint y2, byte color1, byte color2, byte color3, byte color4, byte color5);
	void drawMenu(byte menuNumber);
	void readTextFile();
	void loadAnimationForDirection(Common::SeekableReadStream *stream, int direction);
	void sayLine(uint textRef, byte textColor, byte shadowColor, uint &responseNumber, bool isWithinConversation);
	void goToObject(byte zone1, byte zone2);
	void readObject(uint objPos);
	TextEntry readTextRegister();
	void drawInventory();
	void drawInventoryMask();
	void setRoomTrajectories(int height, int width, TRAJECTORIES_OP op, bool fixGrids = true);
	void saveRoomData(RoomFileRegister *room, Common::SeekableWriteStream *stream);
	//vars
	void initializeScreenFile();
};

extern TotEngine *g_engine;
#define SHOULD_QUIT ::Tot::g_engine->shouldQuit()

} // End of namespace Tot

#endif // TOT_H
