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
#include "freescape/gfx.h"
#include "freescape/objects/entrance.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/sensor.h"

namespace Common {
class RandomSource;
}

namespace Freescape {

class Renderer;

#define FREESCAPE_DATA_BUNDLE Common::String("freescape.dat")

enum CameraMovement {
	kForwardMovement,
	kBackwardMovement,
	kLeftMovement,
	kRightMovement
};

typedef Common::HashMap<uint16, Area *> AreaMap;
typedef Common::Array<byte *> ColorMap;
typedef Common::HashMap<uint16, int32> StateVars;
typedef Common::HashMap<uint16, uint32> StateBits;

enum {
	kFreescapeDebugMove = 1 << 0,
	kFreescapeDebugParser = 1 << 1,
	kFreescapeDebugCode = 1 << 2,
	kFreescapeDebugMedia = 1 << 4,
};

struct entrancesTableEntry {
	int id;
	int position[3];
};

struct soundFx {
	int size;
	int sampleRate;
	byte *data;
};

class SizedPCSpeaker : public Audio::PCSpeaker {
public:
	bool endOfStream() const override { return !isPlaying(); }
};

class FreescapeEngine : public Engine {

public:
	FreescapeEngine(OSystem *syst, const ADGameDescription *gd);
	~FreescapeEngine();

	const ADGameDescription *_gameDescription;
	bool isDemo() const;

	// Game selection
	uint32 _variant;
	bool isDriller() { return _targetName.hasPrefix("driller") || _targetName.hasPrefix("spacestationoblivion"); }
	bool isDark() { return _targetName.hasPrefix("darkside"); }
	bool isEclipse() { return _targetName.hasPrefix("totaleclipse"); }
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

	virtual void loadBorder();
	virtual void processBorder();
	void drawBorder();
	void drawTitle();
	void drawBackground();
	virtual void drawUI();
	virtual void drawInfoMenu();

	virtual void drawCrossair(Graphics::Surface *surface);
	Graphics::ManagedSurface *_border;
	Graphics::ManagedSurface *_title;
	Texture *_borderTexture;
	Texture *_titleTexture;
	Texture *_uiTexture;
	Common::HashMap<uint16, Texture *> _borderCGAByArea;
	Common::HashMap<uint16, byte *> _paletteCGAByArea;

	// Parsing assets
	uint8 _binaryBits;
	void loadAssets();
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
	void loadBundledImages();
	byte *getPaletteFromNeoImage(Common::SeekableReadStream *stream, int offset);
	Graphics::ManagedSurface *loadAndConvertNeoImage(Common::SeekableReadStream *stream, int offset, byte *palette = nullptr);
	Graphics::ManagedSurface *loadAndCenterScrImage(Common::SeekableReadStream *stream);
	void loadPalettes(Common::SeekableReadStream *file, int offset);
	void swapPalette(uint16 areaID);
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
	Common::Array<uint8> readArray(Common::SeekableReadStream *file, int size);

	// 8-bit
	void load8bitBinary(Common::SeekableReadStream *file, int offset, int ncolors);
	Area *load8bitArea(Common::SeekableReadStream *file, uint16 ncolors);
	Object *load8bitObject(Common::SeekableReadStream *file);
	void renderPixels8bitBinImage(Graphics::ManagedSurface *surface, int &i, int &j, uint8 pixels, int color);

	void renderPixels8bitBinCGAImage(Graphics::ManagedSurface *surface, int &i, int &j, uint8 pixels, int color);
	void renderPixels8bitBinEGAImage(Graphics::ManagedSurface *surface, int &i, int &j, uint8 pixels, int color);

	Graphics::ManagedSurface *load8bitBinImage(Common::SeekableReadStream *file, int offset);

	// Areas
	uint16 _startArea;
	AreaMap _areaMap;
	Area *_currentArea;
	Math::Vector3d _scale;

	virtual void gotoArea(uint16 areaID, int entranceID);
	// Entrance
	uint16 _startEntrance;
	Common::HashMap<int, const struct entrancesTableEntry *> _entranceTable;

	// Input
	bool _demoMode;
	bool _disableDemoMode;
	bool _flyMode;
	bool _shootMode;
	bool _noClipMode;
	void processInput();
	void resetInput();
	void generateDemoInput();
	virtual void pressedKey(const int keycode);
	virtual bool onScreenControls(Common::Point mouse);
	void move(CameraMovement direction, uint8 scale, float deltaTime);
	virtual void checkIfStillInArea();
	void changePlayerHeight(int index);
	void increaseStepSize();
	void decreaseStepSize();
	void rise();
	void lower();
	bool checkFloor(Math::Vector3d currentPosition);
	bool tryStepUp(Math::Vector3d currentPosition);
	bool tryStepDown(Math::Vector3d currentPosition);
	bool _hasFallen;

	void rotate(float xoffset, float yoffset);
	// Input state
	float _lastFrame;

	// Interaction
	void shoot();
	void traverseEntrance(uint16 entranceID);

	// Euler Angles
	float _yaw;
	float _pitch;
	int _angleRotationIndex;
	Common::Array<float> _angleRotations;

	Math::Vector3d directionToVector(float pitch, float heading);
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
	Common::Array<int> _playerHeights;
	uint16 _playerHeight;
	uint16 _playerWidth;
	uint16 _playerDepth;

	int _playerStepIndex;
	Common::Array<int> _playerSteps;

	// Effects
	Common::Array<Common::String> _conditionSources;
	Common::Array<FCLInstructionVector> _conditions;

	bool checkCollisions(bool executeCode);
	Math::Vector3d _objExecutingCodeSize;
	virtual void executeMovementConditions();
	void executeObjectConditions(GeometricObject *obj, bool shot, bool collided);
	void executeLocalGlobalConditions(bool shot, bool collided, bool timer);
	void executeCode(FCLInstructionVector &code, bool shot, bool collided, bool timer);

	// Instructions
	void executeIncrementVariable(FCLInstruction &instruction);
	void executeDecrementVariable(FCLInstruction &instruction);
	void executeSetVariable(FCLInstruction &instruction);
	void executeGoto(FCLInstruction &instruction);
	void executeIfThenElse(FCLInstruction &instruction);
	void executeMakeInvisible(FCLInstruction &instruction);
	void executeMakeVisible(FCLInstruction &instruction);
	void executeToggleVisibility(FCLInstruction &instruction);
	void executeDestroy(FCLInstruction &instruction);
	void executeRedraw(FCLInstruction &instruction);
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
	void playWav(const Common::String filename);
	void playMusic(const Common::String filename);
	void queueSoundConst(double hzFreq, int duration);
	void playSilence(int duration, bool sync);
	void playSoundConst(double hzFreq, int duration, bool sync);
	void playSoundSweepIncWL(double hzFreq1, double hzFreq2, double wlStepPerMS, int resolution, bool sync);
	void playTeleporter(int totalIters, bool sync);

	void playSoundFx(int index, bool sync);
	void loadSoundsFx(Common::SeekableReadStream *file, int offset, int number);
	Common::HashMap<uint16, soundFx *> _soundsFx;

	// Rendering
	int _screenW, _screenH;
	Renderer *_gfx;
	Graphics::FrameLimiter *_frameLimiter;
	bool _vsyncEnabled;
	Common::RenderMode _renderMode;
	ColorMap _colorMap;
	int _underFireFrames;
	int _shootingFrames;
	void drawFrame();
	void flashScreen(int backgroundColor);
	uint8 _colorNumber;
	Math::Vector3d _scaleVector;
	float _nearClipPlane;
	float _farClipPlane;

	// Text messages and Fonts
	void insertTemporaryMessage(const Common::String message, int deadline);
	void getLatestMessages(Common::String &message, int &deadline);
	void clearTemporalMessages();
	Common::StringArray _temporaryMessages;
	Common::Array<int> _temporaryMessageDeadlines;
	Common::StringArray _messagesList;

	void loadMessagesFixedSize(Common::SeekableReadStream *file, int offset, int size, int number);
	void loadMessagesVariableSize(Common::SeekableReadStream *file, int offset, int number);

	void loadFonts(Common::SeekableReadStream *file, int offset);
	Common::StringArray _currentAreaMessages;
	Common::StringArray _currentEphymeralMessages;
	Common::BitArray _font;
	bool _fontLoaded;
	void drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface, int offset = 0);

	// Game state
	virtual void initGameState();
	void setGameBit(int index);
	void clearGameBit(int index);
	void toggleGameBit(int index);

	StateVars _gameStateVars;
	StateBits _gameStateBits;
	virtual bool checkIfGameEnded();
	bool _forceEndGame;
	ObjectArray _sensors;
	void checkSensors();
	void drawSensorShoot(Sensor *sensor);
	void takeDamageFromSensor();

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveAutosaveCurrently() override { return false; }
	bool canSaveGameStateCurrently() override { return true; }
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

enum DrillerReleaseFlags {
		GF_AMIGA_RETAIL = (1 << 0),
		GF_AMIGA_BUDGET = (1 << 1),
		GF_ZX_RETAIL = (1 << 2),
		GF_ZX_BUDGET = (1 << 3),
		GF_ZX_DISC = (1 << 4),
		GF_CPC_RETAIL = (1 << 5),
		GF_CPC_RETAIL2 = (1 << 6),
		GF_CPC_BUDGET = (1 << 7),
		GF_CPC_VIRTUALWORLDS = (1 << 8),
};

class DrillerEngine : public FreescapeEngine {
public:
	DrillerEngine(OSystem *syst, const ADGameDescription *gd);
	~DrillerEngine();

	uint32 _initialJetEnergy;
	uint32 _initialJetShield;

	uint32 _initialTankEnergy;
	uint32 _initialTankShield;

	bool _useAutomaticDrilling;

	Common::HashMap<uint16, uint32> _drillStatusByArea;
	Common::HashMap<uint16, uint32> _drillMaxScoreByArea;
	Common::HashMap<uint16, uint32> _drillSuccessByArea;

	void initGameState() override;
	bool checkIfGameEnded() override;

	void gotoArea(uint16 areaID, int entranceID) override;

	void borderScreen() override;
	void titleScreen() override;

	void processBorder() override;
	void drawInfoMenu() override;

	void pressedKey(const int keycode) override;
	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;

private:
	void loadGlobalObjects(Common::SeekableReadStream *file, int offset);
	bool drillDeployed(Area *area);
	GeometricObject *_drillBase;
	Math::Vector3d drillPosition();
	void addDrill(const Math::Vector3d position, bool gasFound);
	bool checkDrill(const Math::Vector3d position);
	void removeDrill(Area *area);
	void addSkanner(Area *area);

	void loadAssetsFullGame() override;

	void loadAssetsAtariFullGame() override;
	void loadAssetsAtariDemo() override;

	void loadAssetsAmigaFullGame() override;
	void loadAssetsAmigaDemo() override;

	void loadAssetsDOSFullGame() override;
	void loadAssetsDOSDemo() override;

	void loadAssetsZXFullGame() override;

	void loadAssetsCPCFullGame() override;

	void loadAssetsC64FullGame() override;

	void drawDOSUI(Graphics::Surface *surface) override;
	void drawZXUI(Graphics::Surface *surface) override;
	void drawCPCUI(Graphics::Surface *surface) override;
	void drawC64UI(Graphics::Surface *surface) override;
	void drawAmigaAtariSTUI(Graphics::Surface *surface) override;
	bool onScreenControls(Common::Point mouse) override;
	void initAmigaAtari();
	void initDOS();
	void initZX();
	void initCPC();
	void initC64();

	Common::Rect _moveFowardArea;
	Common::Rect _moveLeftArea;
	Common::Rect _moveRightArea;
	Common::Rect _moveBackArea;
	Common::Rect _moveUpArea;
	Common::Rect _moveDownArea;
	Common::Rect _deployDrillArea;
	Common::Rect _infoScreenArea;
	Common::Rect _saveGameArea;
	Common::Rect _loadGameArea;

	Graphics::ManagedSurface *load8bitTitleImage(Common::SeekableReadStream *file, int offset);
	Graphics::ManagedSurface *load8bitDemoImage(Common::SeekableReadStream *file, int offset);

	uint32 getPixel8bitTitleImage(int index);
	void renderPixels8bitTitleImage(Graphics::ManagedSurface *surface, int &i, int &j, int pixels);
};

class DarkEngine : public FreescapeEngine {
public:
	DarkEngine(OSystem *syst, const ADGameDescription *gd);

	uint32 _initialFuel;
	uint32 _initialShield;

	void initGameState() override;
	void borderScreen() override;
	void titleScreen() override;

	void gotoArea(uint16 areaID, int entranceID) override;
	void checkIfStillInArea() override;
	void pressedKey(const int keycode) override;
	void executePrint(FCLInstruction &instruction) override;

	void loadAssetsDOSFullGame() override;
	void loadAssetsDOSDemo() override;

	int _lastTenSeconds;
	void updateTimeVariables() override;

	void drawDOSUI(Graphics::Surface *surface) override;
	void drawFullscreenMessage(Common::String message);
	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;

private:
	void loadGlobalObjects(Common::SeekableReadStream *file, int offset);
};

class EclipseEngine : public FreescapeEngine {
public:
	EclipseEngine(OSystem *syst, const ADGameDescription *gd);

	void titleScreen() override;
	void gotoArea(uint16 areaID, int entranceID) override;

	void loadAssetsDOSFullGame() override;
	void drawUI() override;

	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;
};

class CastleEngine : public FreescapeEngine {
public:
	CastleEngine(OSystem *syst, const ADGameDescription *gd);

	void loadAssetsDOSFullGame() override;

	void gotoArea(uint16 areaID, int entranceID) override;
	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;
private:
	Common::SeekableReadStream *decryptFile(const Common::String filename);
};

extern FreescapeEngine *g_freescape;

} // namespace Freescape

#endif // FREESCAPE_H
