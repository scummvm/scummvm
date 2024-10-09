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

#ifndef FREESCAPE_H
#define FREESCAPE_H

#include "common/bitarray.h"
#include "common/events.h"
#include "engines/advancedDetector.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"
#include "graphics/framelimiter.h"

#include "freescape/area.h"
#include "freescape/font.h"
#include "freescape/gfx.h"
#include "freescape/objects/entrance.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/sensor.h"
#include "freescape/sound.h"

namespace Common {
class RandomSource;
}

namespace Freescape {

class Renderer;

#define FREESCAPE_DATA_BUNDLE "freescape.dat"

enum CameraMovement {
	kForwardMovement,
	kBackwardMovement,
	kLeftMovement,
	kRightMovement
};

enum FREESCAPEAction {
	kActionNone,
	kActionEscape,
	kActionSave,
	kActionLoad,
	kActionToggleSound,
	kActionMoveUp,
	kActionMoveDown,
	kActionMoveLeft,
	kActionMoveRight,
	kActionShoot,
	kActionRunMode,
	kActionChangeAngle,
	kActionChangeStepSize,
	kActionToggleRiseLower,
	kActionRiseOrFlyUp,
	kActionLowerOrFlyDown,
	kActionChangeMode,
	kActionSkip,
	kActionFaceForward,
	kActionRotateUp,
	kActionRotateDown,
	kActionRotateLeft,
	kActionRotateRight,
	kActionTurnBack,
	kActionInfoMenu,
	kActionIncreaseStepSize,
	kActionDecreaseStepSize,
	kActionToggleFlyMode,
	kActionToggleClipMode,
	// Driller
	kActionDeployDrillingRig,
	kActionCollectDrillingRig,
	// Total Eclipse
	kActionRest,
	// Castle
	kActionSelectPrince,
	kActionSelectPrincess,
};

typedef Common::HashMap<uint16, Area *> AreaMap;
typedef Common::Array<byte *> ColorMap;
typedef Common::HashMap<uint16, int32> StateVars;

enum {
	kFreescapeDebugMove = 1 << 0,
	kFreescapeDebugParser = 1 << 1,
	kFreescapeDebugCode = 1 << 2,
	kFreescapeDebugMedia = 1 << 4,
	kFreescapeDebugGroup = 1 << 5,
};

enum GameStateControl {
	kFreescapeGameStateStart,
	kFreescapeGameStatePlaying,
	kFreescapeGameStateDemo,
	kFreescapeGameStateEnd,
	kFreescapeGameStateRestart,
};

struct CGAPaletteEntry {
	int areaId;
	byte *palette;
};

class EventManagerWrapper {
public:
	EventManagerWrapper(Common::EventManager *delegate);
	// EventManager API
	bool pollEvent(Common::Event &event);
	void purgeKeyboardEvents();
	void purgeMouseEvents();
	void pushEvent(Common::Event &event);
	void clearExitEvents();

private:
	// for continuous events (keyDown)
	enum {
		kKeyRepeatInitialDelay = 400,
		kKeyRepeatSustainDelay = 100
	};

	Common::EventManager *_delegate;

	Common::KeyState _currentKeyDown;
	Common::CustomEventType _currentActionDown;
	uint32 _keyRepeatTime;
};

class FreescapeEngine : public Engine {

public:
	FreescapeEngine(OSystem *syst, const ADGameDescription *gd);
	~FreescapeEngine();

	const ADGameDescription *_gameDescription;
	GameStateControl _gameStateControl;
	bool isDemo() const;

	// Game selection
	uint32 _variant;
	Common::Language _language;
	bool isSpaceStationOblivion() { return _targetName.hasPrefix("spacestationoblivion"); }
	bool isDriller() { return _targetName.hasPrefix("driller") || _targetName.hasPrefix("spacestationoblivion"); }
	bool isDark() { return _targetName.hasPrefix("darkside"); }
	bool isEclipse() { return _targetName.hasPrefix("totaleclipse"); } // This will match Total Eclipse 1 and 2.
	bool isEclipse2() { return _targetName.hasPrefix("totaleclipse2"); }
	bool isCastle() { return _targetName.hasPrefix("castle"); }
	bool isAmiga() { return _gameDescription->platform == Common::kPlatformAmiga; }
	bool isAtariST() { return _gameDescription->platform == Common::kPlatformAtariST; }
	bool isDOS() { return _gameDescription->platform == Common::kPlatformDOS; }
	bool isSpectrum() { return _gameDescription->platform == Common::kPlatformZX; }
	bool isCPC() { return _gameDescription->platform == Common::kPlatformAmstradCPC; }
	bool isC64() { return _gameDescription->platform == Common::kPlatformC64; }

	Common::Error run() override;

	// UI
	Common::Rect _viewArea;
	Common::Rect _fullscreenViewArea;
	void centerCrossair();

	virtual void borderScreen();
	virtual void titleScreen();

	void drawFullscreenSurface(Graphics::Surface *surface);
	virtual void loadBorder();
	virtual void processBorder();
	void waitInLoop(int maxWait);
	void drawBorder();
	void drawTitle();
	virtual void drawBackground();
	void clearBackground();
	virtual void drawUI();
	virtual void drawInfoMenu();
	void drawBorderScreenAndWait(Graphics::Surface *surface, int maxWait = INT_MAX);

	virtual void drawCrossair(Graphics::Surface *surface);
	Graphics::ManagedSurface *_border;
	Graphics::ManagedSurface *_title;
	Graphics::ManagedSurface *_background;

	Texture *_borderTexture;
	Texture *_titleTexture;
	Texture *_uiTexture;
	Texture *_skyTexture;

	Common::Array<Graphics::Surface *>_indicators;
	Common::HashMap<uint16, Texture *> _borderCGAByArea;
	Common::HashMap<uint16, byte *> _paletteCGAByArea;

	// Parsing assets
	uint8 _binaryBits;
	virtual void loadAssets();
	virtual void loadAssetsDemo();
	virtual void loadAssetsFullGame();

	virtual void loadAssetsAtariFullGame();
	virtual void loadAssetsAtariDemo();

	virtual void loadAssetsAmigaFullGame();
	virtual void loadAssetsAmigaDemo();

	virtual void loadAssetsDOSFullGame();
	virtual void loadAssetsDOSDemo();

	virtual void loadAssetsZXFullGame();
	virtual void loadAssetsZXDemo();

	virtual void loadAssetsCPCFullGame();
	virtual void loadAssetsCPCDemo();

	virtual void loadAssetsC64FullGame();
	virtual void loadAssetsC64Demo();

	virtual void drawDOSUI(Graphics::Surface *surface);
	virtual void drawZXUI(Graphics::Surface *surface);
	virtual void drawCPCUI(Graphics::Surface *surface);
	virtual void drawC64UI(Graphics::Surface *surface);
	virtual void drawAmigaAtariSTUI(Graphics::Surface *surface);

	Common::Archive *_dataBundle;
	void loadDataBundle();
	Graphics::Surface *loadBundledImage(const Common::String &name);
	byte *getPaletteFromNeoImage(Common::SeekableReadStream *stream, int offset);
	Graphics::ManagedSurface *loadAndConvertNeoImage(Common::SeekableReadStream *stream, int offset, byte *palette = nullptr);
	Graphics::ManagedSurface *loadAndCenterScrImage(Common::SeekableReadStream *stream);
	void loadPalettes(Common::SeekableReadStream *file, int offset);
	byte *loadPalette(Common::SeekableReadStream *file);
	void swapPalette(uint16 areaID);
	virtual byte *findCGAPalette(uint16 levelID);
	const CGAPaletteEntry *_rawCGAPaletteByArea;
	Common::HashMap<uint16, byte *> _paletteByArea;
	void loadColorPalette();

	// Demo
	Common::Array<byte> _demoData;
	int _demoIndex;
	int _currentDemoInputCode;
	int _currentDemoInputRepetition;
	Common::Array<Common::Event> _demoEvents;
	Common::Point _currentDemoMousePosition;
	void loadDemoData(Common::SeekableReadStream *file, int offset, int size);
	int decodeAmigaAtariKey(int code);
	int decodeDOSKey(int code);
	Common::Event decodeDOSMouseEvent(int code, int repetition);

	uint16 readField(Common::SeekableReadStream *file, int nbits);
	uint16 readPtr(Common::SeekableReadStream *file);
	Common::Array<uint16> readArray(Common::SeekableReadStream *file, int size);

	// 8-bit
	void load8bitBinary(Common::SeekableReadStream *file, int offset, int ncolors);
	Area *load8bitArea(Common::SeekableReadStream *file, uint16 ncolors);
	Object *load8bitObject(Common::SeekableReadStream *file);
	Group *load8bitGroup(Common::SeekableReadStream *file, byte rawFlagsAndType);
	Group *load8bitGroupV1(Common::SeekableReadStream *file, byte rawFlagsAndType);
	Group *load8bitGroupV2(Common::SeekableReadStream *file, byte rawFlagsAndType);

	void loadGlobalObjects(Common::SeekableReadStream *file, int offset, int size);
	void renderPixels8bitBinImage(Graphics::ManagedSurface *surface, int row, int column, int bit, int count);

	void renderPixels8bitBinCGAImage(Graphics::ManagedSurface *surface, int &i, int &j, uint8 pixels, int color);
	void renderPixels8bitBinEGAImage(Graphics::ManagedSurface *surface, int &i, int &j, uint8 pixels, int color);

	Graphics::ManagedSurface *load8bitBinImage(Common::SeekableReadStream *file, int offset);
	void load8bitBinImageRow(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row);
	void load8bitBinImageRowIteration(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int bit);
	int execute8bitBinImageCommand(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int pixels, int bit);
	int execute8bitBinImageSingleCommand(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int pixels, int bit, int count);
	int execute8bitBinImageMultiCommand(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int row, int pixels, int bit, int count);

	void parseAmigaAtariHeader(Common::SeekableReadStream *file);
	Common::SeekableReadStream *decryptFileAmigaAtari(const Common::Path &packed, const Common::Path &unpacker, uint32 unpackArrayOffset);

	// Areas
	uint16 _startArea;
	uint16 _endArea;
	AreaMap _areaMap;
	Area *_currentArea;
	bool _gotoExecuted;
	Math::Vector3d _scale;

	virtual void gotoArea(uint16 areaID, int entranceID);
	// Entrance
	uint16 _startEntrance;
	uint16 _endEntrance;
	Common::HashMap<int, const struct entrancesTableEntry *> _entranceTable;

	// Input
	bool _demoMode;
	bool _disableDemoMode;
	bool _flyMode;
	bool _shootMode;
	bool _noClipMode;
	bool _invertY;
	virtual void initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target);
	EventManagerWrapper *_eventManager;
	void processInput();
	void resetInput();
	void generateDemoInput();
	virtual void pressedKey(const int keycode);
	virtual void releasedKey(const int keycode);
	Common::Point getNormalizedPosition(Common::Point position);
	virtual bool onScreenControls(Common::Point mouse);
	void move(CameraMovement direction, uint8 scale, float deltaTime);
	void resolveCollisions(Math::Vector3d newPosition);
	virtual void checkIfStillInArea();
	void changePlayerHeight(int index);
	void increaseStepSize();
	void decreaseStepSize();
	void changeStepSize();

	void changeAngle();
	void rise();
	void lower();
	bool checkFloor(Math::Vector3d currentPosition);
	bool tryStepUp(Math::Vector3d currentPosition);
	bool tryStepDown(Math::Vector3d currentPosition);
	bool _hasFallen;
	int _maxFallingDistance;
	int _maxShield;
	int _maxEnergy;

	void rotate(float xoffset, float yoffset);
	// Input state
	float _lastFrame;

	// Interaction
	void activate();
	void shoot();
	void traverseEntrance(uint16 entranceID);

	// Euler Angles
	float _yaw;
	float _pitch;
	int _angleRotationIndex;
	Common::Array<float> _angleRotations;

	Math::Vector3d directionToVector(float pitch, float heading, bool useTable);
	void updateCamera();

	// Camera options
	Common::Point _crossairPosition;
	float _mouseSensitivity;
	Math::Vector3d _upVector; // const
	Math::Vector3d _cameraFront, _cameraRight;
	// Spacial attributes
	Math::Vector3d _position, _rotation, _velocity;
	Math::Vector3d _lastPosition;
	int _playerHeightNumber;
	int _playerHeightMaxNumber;
	uint16 _playerHeight;
	uint16 _playerWidth;
	uint16 _playerDepth;
	uint16 _stepUpDistance;

	int _playerStepIndex;
	Common::Array<int> _playerSteps;

	// Effects
	Common::Array<Common::String> _conditionSources;
	Common::Array<FCLInstructionVector> _conditions;

	bool runCollisionConditions(Math::Vector3d const lastPosition, Math::Vector3d const newPosition);
	Math::Vector3d _objExecutingCodeSize;
	bool _executingGlobalCode;
	virtual void executeMovementConditions();
	bool executeObjectConditions(GeometricObject *obj, bool shot, bool collided, bool activated);
	void executeEntranceConditions(Entrance *entrance);
	void executeLocalGlobalConditions(bool shot, bool collided, bool timer);
	bool executeCode(FCLInstructionVector &code, bool shot, bool collided, bool timer, bool activated);

	// Instructions
	bool checkConditional(FCLInstruction &instruction, bool shot, bool collided, bool timer, bool activated);
	bool checkIfGreaterOrEqual(FCLInstruction &instruction);
	void executeExecute(FCLInstruction &instruction);
	void executeIncrementVariable(FCLInstruction &instruction);
	void executeDecrementVariable(FCLInstruction &instruction);
	void executeSetVariable(FCLInstruction &instruction);
	void executeGoto(FCLInstruction &instruction);
	void executeIfThenElse(FCLInstruction &instruction);
	virtual void executeMakeInvisible(FCLInstruction &instruction);
	void executeMakeVisible(FCLInstruction &instruction);
	void executeToggleVisibility(FCLInstruction &instruction);
	virtual void executeDestroy(FCLInstruction &instruction);
	virtual void executeRedraw(FCLInstruction &instruction);
	void executeSound(FCLInstruction &instruction);
	void executeDelay(FCLInstruction &instruction);
	bool executeEndIfNotEqual(FCLInstruction &instruction);
	void executeSetBit(FCLInstruction &instruction);
	void executeClearBit(FCLInstruction &instruction);
	void executeToggleBit(FCLInstruction &instruction);
	bool executeEndIfBitNotEqual(FCLInstruction &instruction);
	bool executeEndIfVisibilityIsEqual(FCLInstruction &instruction);
	void executeSwapJet(FCLInstruction &instruction);
	virtual void executePrint(FCLInstruction &instruction);
	void executeSPFX(FCLInstruction &instruction);
	void executeStartAnim(FCLInstruction &instruction);

	// Sound
	Audio::SoundHandle _soundFxHandle;
	Audio::SoundHandle _musicHandle;
	Freescape::SizedPCSpeaker *_speaker;

	bool _syncSound;
	bool _firstSound;
	bool _usePrerecordedSounds;
	void waitForSounds();
	void stopAllSounds();
	bool isPlayingSound();
	void playSound(int index, bool sync);
	void playWav(const Common::Path &filename);
	void playMusic(const Common::Path &filename);
	void queueSoundConst(double hzFreq, int duration);
	void playSilence(int duration, bool sync);
	void playSoundConst(double hzFreq, int duration, bool sync);
	void playSoundSweepIncWL(double hzFreq1, double hzFreq2, double wlStepPerMS, int resolution, bool sync);
	uint16 playSoundDOSSpeaker(uint16 startFrequency, soundSpeakerFx *speakerFxInfo);
	void playSoundDOS(soundSpeakerFx *speakerFxInfo, bool sync);

	virtual void playSoundFx(int index, bool sync);
	virtual void loadSoundsFx(Common::SeekableReadStream *file, int offset, int number);
	Common::HashMap<uint16, soundFx *> _soundsFx;
	void loadSpeakerFxDOS(Common::SeekableReadStream *file, int offsetFreq, int offsetDuration);
	void loadSpeakerFxZX(Common::SeekableReadStream *file, int sfxTable, int sfxData);
	Common::HashMap<uint16, soundSpeakerFx *> _soundsSpeakerFx;

	void playSoundZX(Common::Array<soundUnitZX> *data);
	Common::HashMap<uint16, Common::Array<soundUnitZX>*> _soundsSpeakerFxZX;
	int _soundIndexShoot;
	int _soundIndexCollide;
	int _soundIndexFall;
	int _soundIndexClimb;
	int _soundIndexMenu;
	int _soundIndexStart;
	int _soundIndexAreaChange;
	int _soundIndexHit;

	int _soundIndexNoShield;
	int _soundIndexNoEnergy;
	int _soundIndexFallen;
	int _soundIndexTimeout;
	int _soundIndexForceEndGame;
	int _soundIndexCrushed;
	int _soundIndexMissionComplete;

	// Rendering
	int _screenW, _screenH;
	Renderer *_gfx;
	Graphics::FrameLimiter *_frameLimiter;
	bool _vsyncEnabled;
	Common::RenderMode _renderMode;
	ColorMap _colorMap;
	int _underFireFrames;
	int _avoidRenderingFrames;
	int _shootingFrames;
	GeometricObject *_delayedShootObject;
	void drawFrame();
	void flashScreen(int backgroundColor);
	uint8 _colorNumber;
	Math::Vector3d _scaleVector;
	float _nearClipPlane;
	float _farClipPlane;
	float _yminValue;
	float _ymaxValue;

	// Text messages and Fonts
	void insertTemporaryMessage(const Common::String message, int deadline);
	void getLatestMessages(Common::String &message, int &deadline);
	void clearTemporalMessages();
	Common::StringArray _temporaryMessages;
	Common::Array<int> _temporaryMessageDeadlines;
	Common::StringArray _messagesList;
	Common::String _noShieldMessage;
	Common::String _noEnergyMessage;
	Common::String _fallenMessage;
	Common::String _timeoutMessage;
	Common::String _forceEndGameMessage;
	Common::String _crushedMessage;
	Common::String _outOfReachMessage;
	Common::String _noEffectMessage;

	void loadMessagesFixedSize(Common::SeekableReadStream *file, int offset, int size, int number);
	virtual void loadMessagesVariableSize(Common::SeekableReadStream *file, int offset, int number);
	void drawFullscreenMessageAndWait(Common::String message);
	void drawFullscreenMessage(Common::String message, uint32 front, Graphics::Surface *surface);

	// Font loading and rendering
	void loadFonts(Common::SeekableReadStream *file, int offset);
	void loadFonts(byte *font, int charNumber);
	Common::Array<Graphics::ManagedSurface *> getChars(Common::SeekableReadStream *file, int offset, int charsNumber);
	Common::Array<Graphics::ManagedSurface *> getCharsAmigaAtari(Common::SeekableReadStream *file, int offset, int charsNumber);
	Common::StringArray _currentAreaMessages;
	Common::StringArray _currentEphymeralMessages;
	Font _font;
	bool _fontLoaded;
	virtual void drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface, int offset = 0);
	virtual void drawStringInSurface(const Common::String &str, int x, int y, uint32 primaryFontColor, uint32 secondaryFontColor, uint32 backColor, Graphics::Surface *surface, int offset = 0);
	Graphics::Surface *drawStringsInSurface(const Common::Array<Common::String> &lines, Graphics::Surface *surface);

	// Game state
	virtual void initGameState();
	void setGameBit(int index);
	void clearGameBit(int index);
	void toggleGameBit(int index);
	uint16 getGameBit(int index);

	StateVars _gameStateVars;
	uint32 _gameStateBits;
	virtual bool checkIfGameEnded();
	virtual void endGame();
	int _endGameDelayTicks;
	bool _endGameKeyPressed;
	bool _endGamePlayerEndArea;
	bool _forceEndGame;
	bool _playerWasCrushed;
	Common::HashMap<uint16, bool> _exploredAreas;
	ObjectArray _sensors;
	virtual void checkSensors();
	virtual void drawSensorShoot(Sensor *sensor);
	void takeDamageFromSensor();

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override { return true; }
	bool canSaveAutosaveCurrently() override { return false; }
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override { return _gameStateControl == kFreescapeGameStatePlaying && _currentArea; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	virtual Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false);
	virtual Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream);
	Graphics::Surface *_savedScreen;

	void pauseEngineIntern(bool pause) override;

	// Timers
	bool startCountdown(uint32 delay);
	void removeTimers();
	bool _timerStarted;
	int _initialCountdown;
	int _countdown;
	int _ticks;
	int _ticksFromEnd;
	int _lastTick;
	int _lastMinute;

	void getTimeFromCountdown(int &seconds, int &minutes, int &hours);
	virtual void updateTimeVariables();

	// Cheats
	bool _useExtendedTimer;
	bool _disableSensors;
	bool _disableFalling;

	// Random
	Common::RandomSource *_rnd;
};

enum GameReleaseFlags {
	GF_AMIGA_RETAIL = (1 << 0),
	GF_AMIGA_BUDGET = (1 << 1),
	GF_ZX_RETAIL = (1 << 2),
	GF_ZX_BUDGET = (1 << 3),
	GF_ZX_DISC = (1 << 4),
	GF_CPC_RETAIL = (1 << 5),
	GF_CPC_RETAIL_ALT = (1 << 6),
	GF_CPC_BUDGET = (1 << 7),
	GF_CPC_VIRTUALWORLDS = (1 << 8),
	GF_ATARI_RETAIL = (1 << 9),
	GF_ATARI_BUDGET = (1 << 10)
};

extern FreescapeEngine *g_freescape;

} // namespace Freescape

#endif // FREESCAPE_H
