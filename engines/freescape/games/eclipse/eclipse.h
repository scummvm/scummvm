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

#include "freescape/sound.h"

namespace Freescape {

enum EclipseReleaseFlags {
	GF_ZX_DEMO_CRASH = (1 << 0),
	GF_ZX_DEMO_MICROHOBBY = (1 << 1),
};

enum {
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
	int _lastThirtySeconds;
	int _lastFiveSeconds;

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
	void drawEclipseIndicator(Graphics::Surface *surface, int x, int y, uint32 color1, uint32 color2, uint32 color3 = 0);
	Common::String getScoreString(int score);
	void drawScoreString(int score, int x, int y, uint32 front, uint32 back, Graphics::Surface *surface);

	Common::Array<byte> _musicData; // TEMUSIC.ST TEXT segment (Atari ST)

	// Atari ST UI sprites (extracted from binary, pre-converted to target format)
	Font _fontScore; // Font B (10 score digit glyphs, 4-plane at $249BE)
	Common::Array<Graphics::ManagedSurface *> _eclipseSprites; // 2 eclipse animation frames (16x13)
	Common::Array<Graphics::ManagedSurface *> _eclipseProgressSprites;  // 16 eclipse animation frames (16x16)
	Common::Array<Graphics::ManagedSurface *> _compassSprites; // 37 pre-composited compass frames (32x27)
	Common::Array<Graphics::ManagedSurface *> _lanternLightSprites;  // 6 lantern light animation frames (32x6)
	Common::Array<Graphics::ManagedSurface *> _lanternSwitchSprites; // 2 lantern on/off frames (32x23)
	Common::Array<Graphics::ManagedSurface *> _shootSprites;         // 2 shooting crosshair frames (32x25, 48x25)
	Common::Array<Graphics::ManagedSurface *> _ankhSprites;          // 5 ankh fade-in frames (16x15)
	Common::Array<Graphics::ManagedSurface *> _waterSprites;         // 9 water ripple frames (32x9)
	Common::Array<Graphics::ManagedSurface *> _soundToggleSprites;   // 5 sound on/off toggle frames (16x11)
	byte _compassLookup[72];  // direction-to-needle-frame lookup table

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

	bool checkIfGameEnded() override;
	void endGame() override;
	void playSoundFx(int index, bool sync, Sound::Type type = Sound::kTypeNormal) override;

	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;

private:
	Sound *_soundFx;
};

}
