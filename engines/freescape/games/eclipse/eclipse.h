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

#include "common/file.h"

#include "freescape/music.h"
#include "freescape/sound.h"

namespace Freescape {

class EclipseC64SFXPlayer;

enum EclipseReleaseFlags {
	GF_ZX_DEMO_CRASH = (1 << 0),
	GF_ZX_DEMO_MICROHOBBY = (1 << 1),
};

enum {
	kVariableEclipse2SphinxParts = 1,
	kVariableEclipseAnkhs = 32,
};

class EclipseEngine : public FreescapeEngine {
public:
	EclipseEngine(OSystem *syst, const ADGameDescription *gd);
	~EclipseEngine() override;

	void gotoArea(uint16 areaID, int entranceID) override;

	void borderScreen() override;
	void titleScreen() override;
	void drawInfoMenu() override;
	void drawIndicator(Graphics::Surface *surface, int xPosition, int yPosition, int separation);

	void drawSensorShoot(Sensor *sensor) override;

	void loadAssets() override;
	void loadAssetsDOSFullGame() override;
	void pressedKey(const int keycode) override;
	void releasedKey(const int keycode) override;

	uint32 _initialEnergy;
	uint32 _initialShield;

	int _soundIndexStartFalling;
	int _soundIndexEndFalling;

	bool _resting;
	bool _flashlightOn;
	bool _atariAreaDark;
	int _lastThirtySeconds;
	int _lastFiveSeconds;
	int _lastHeartbeatSoundTick;
	int _lastHeartIndicatorFrame;

	int _lastSecond;
	void updateTimeVariables() override;

	void initDOS();
	void initCPC();
	void initZX();
	void initC64();
	void initAmigaAtari();

	void loadAssetsZXFullGame() override;
	void loadAssetsCPCFullGame() override;
	void loadAssetsC64FullGame() override;
	void loadAssetsAmigaFullGame() override;
	void loadAssetsAtariFullGame() override;
	void loadAssetsCPCDemo() override;
	void loadAssetsZXDemo() override;

	void initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) override;
	void initGameState() override;
	void executePrint(FCLInstruction &instruction) override;

	void drawBackground() override;
	void drawDOSUI(Graphics::Surface *surface) override;
	void drawCPCUI(Graphics::Surface *surface) override;
	void drawC64UI(Graphics::Surface *surface) override;
	void drawZXUI(Graphics::Surface *surface) override;
	void drawAmigaAtariSTUI(Graphics::Surface *surface) override;
	void drawAnalogClock(Graphics::Surface *surface, int x, int y, uint32 colorHand1, uint32 colorHand2, uint32 colorBack);
	void drawAnalogClockHand(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color);
	void drawCompass(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color);
	int atariCompassPhaseFromRotationY(float rotationY) const;
	int atariCompassTargetPhaseFromYaw(float yaw, int referencePhase) const;
	bool isAtariDarkArea(uint16 areaID) const;
	void onRotate(float xoffset, float yoffset, float zoffset) override;
	void drawEclipseIndicator(Graphics::Surface *surface, int x, int y, uint32 color1, uint32 color2, uint32 color3 = 0);
	Common::String getScoreString(int score);
	void drawScoreString(int score, int x, int y, uint32 front, uint32 back, Graphics::Surface *surface);

	soundFx *load1bPCM(Common::SeekableReadStream *file, int offset);
	void loadHeartFramesCPC(Common::SeekableReadStream *file, int restOffset, int beatOffset);
	void loadHeartFramesZX(Common::SeekableReadStream *file, int restOffset, int beatOffset);
	void loadHeartFramesDOS(Common::SeekableReadStream *file, int restOffset, int beatOffset);
	void drawHeartIndicator(Graphics::Surface *surface, int x, int y);

	// CPC heart frames stored as indexed (CLUT8) for per-area re-paletting
	Common::Array<Graphics::ManagedSurface *> _heartFramesCPCIndexed;
	void updateHeartFramesCPC();

	Common::Array<byte> _musicData; // TEMUSIC.ST TEXT segment (Atari ST)
	Common::Array<byte> _c64MusicData;
	EclipseC64SFXPlayer *_playerC64Sfx;
	bool _c64UseSFX;
	void playSoundC64(int index) override;
	void toggleC64Sound();

	MusicPlayer *_playerMusic;
	void restartBackgroundMusic();
	void stopBackgroundMusic();

	// Atari ST UI sprites (extracted from binary, pre-converted to target format)
	Font _fontScore; // Font B (10 score digit glyphs, 4-plane at $249BE)
	Common::Array<Graphics::ManagedSurface *> _eclipseSprites; // 2 eclipse animation frames (16x13)
	Common::Array<Graphics::ManagedSurface *> _eclipseProgressSprites;  // 16 eclipse animation frames (16x16)
	Graphics::ManagedSurface *_compassBackground; // Atari ST compass background at $20986
	Graphics::ManagedSurface *_atariWaterBody; // Atari ST water body bitmap at $2003C
	Common::Array<Graphics::ManagedSurface *> _compassSprites; // signed Atari compass needle bank addressed by the $1542 lookup table
	Common::Array<Graphics::ManagedSurface *> _lanternLightSprites;  // 6 lantern light animation frames (32x6)
	Common::Array<Graphics::ManagedSurface *> _lanternSwitchSprites; // 2 lantern on/off frames (32x23)
	Common::Array<Graphics::ManagedSurface *> _shootSprites;         // 2 shooting crosshair frames (32x25, 48x25)
	Common::Array<Graphics::ManagedSurface *> _ankhSprites;          // 5 ankh fade-in frames (16x15)
	Common::Array<uint16> _atariDarkAreas;                           // Atari ST dark areas from the $2A520 runtime table
	byte _eclipseFadePalettes[6][16 * 3];                            // 6 brightness levels from prog $10EB6 (post LSR.L #1)
	void applyEclipseFadePalette(uint16 areaID, int brightnessLevel);
	Common::Array<uint16> _atariWaterSurfacePixels;                  // Atari ST water surface row words at $2024C
	Common::Array<uint16> _atariWaterSurfaceMask;                    // Atari ST water surface mask words at $2025C
	Common::Array<Graphics::ManagedSurface *> _soundToggleSprites;   // 5 sound on/off toggle frames (16x11)
	int8 _compassLookup[72];  // signed Atari ST phase-to-frame lookup table
	int _atariCompassPhase;
	int _atariCompassTargetPhase;
	float _atariCompassTargetRemainder;
	int _atariCompassLastUpdateTick;
	bool _atariCompassPhaseInitialized;
	int _atariLanternLightFrame;
	int _atariLanternAnimationDirection;
	int _atariLanternLastUpdateTick;
	int _lanternBatteryLevel; // 5=full, 0=nearly dead, -1=dead (non-rechargeable)

	// Atari ST on-screen control hotspots (from binary hotspot table at prog $869A)
	bool onScreenControls(Common::Point mouse) override;

	Common::Rect _lookUpArea;
	Common::Rect _lookDownArea;
	Common::Rect _turnLeftArea;
	Common::Rect _turnRightArea;
	Common::Rect _uTurnArea;
	Common::Rect _faceForwardArea;
	Common::Rect _moveBackwardArea;
	Common::Rect _stepBackwardArea;
	Common::Rect _interactArea;
	Common::Rect _infoDisplayArea;
	Common::Rect _lanternArea;
	Common::Rect _restArea;
	Common::Rect _stepSizeArea;
	Common::Rect _heightArea;
	Common::Rect _saveGameArea;
	Common::Rect _loadGameArea;

	bool triggerWinCondition() override;
	bool checkIfGameEnded() override;
	void endGame() override;
	void loadSoundsFx(Common::SeekableReadStream *file, int offset, int number) override;
	void playSoundFx(int index, bool sync) override;

	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;
};

}
