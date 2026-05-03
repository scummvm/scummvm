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

#include "audio/mixer.h"
#include "common/array.h"
#include "freescape/music.h"
#include "freescape/games/dark/c64.music.h"
#include "freescape/games/dark/c64.sfx.h"

namespace Freescape {

enum {
	kVariableDarkEnding = 28,
	kVariableDarkECD = 29,
	kVariableActiveECDs = 60,
};

enum {
	kDarkEndingEvathDestroyed = 1,
	kDarkEndingECDsDestroyed = 2,
};

struct ECD {
	uint16 _area;
	int _id;
};

enum DarkFontSize {
	kDarkFontSmall,
	kDarkFontMedium,
	kDarkFontBig,
};

class DarkEngine : public FreescapeEngine {
public:
	DarkEngine(OSystem *syst, const ADGameDescription *gd);
	~DarkEngine();

	uint32 _initialEnergy;
	uint32 _initialShield;
	uint32 _jetFuelSeconds;
	void addSkanner(Area *area);

	void initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) override;
	void initGameState() override;
	void borderScreen() override;
	bool triggerWinCondition() override;
	bool checkIfGameEnded() override;
	void endGame() override;

	void gotoArea(uint16 areaID, int entranceID) override;
	void pressedKey(const int keycode) override;
	void executePrint(FCLInstruction &instruction) override;

	void initDOS();
	void initC64();
	void initAmigaAtari();
	void initZX();
	void initCPC();

	void loadAssets() override;
	void loadAssetsDOSFullGame() override;
	void loadAssetsDOSDemo() override;
	void loadAssetsC64FullGame() override;
	void loadAssetsAmigaFullGame() override;
	void loadAssetsAtariFullGame() override;

	void loadAssetsCPCFullGame() override;

	void loadAssetsZXDemo() override;
	void loadAssetsZXFullGame() override;
	void loadMessagesVariableSize(Common::SeekableReadStream *file, int offset, int number) override;

	int _lastTenSeconds;
	int _lastSecond;
	void updateTimeVariables() override;

	void drawBinaryClock(Graphics::Surface *surface, int xPosition, int yPosition, uint32 front, uint32 back);
	void drawIndicator(Graphics::Surface *surface, int xPosition, int yPosition);

	void drawSensorShoot(Sensor *sensor) override;
	void drawDOSUI(Graphics::Surface *surface) override;
	void drawC64UI(Graphics::Surface *surface) override;
	void drawZXUI(Graphics::Surface *surface) override;
	void drawCPCUI(Graphics::Surface *surface) override;
	void drawAmigaAtariSTUI(Graphics::Surface *surface) override;
	void drawC64Compass(Graphics::Surface *surface);

	Font _fontBig;
	Font _fontMedium;
	Font _fontSmall;
	Common::Array<Graphics::ManagedSurface *> _cpcIndicators;
	Common::Array<Graphics::ManagedSurface *> _cpcJetpackIndicators;
	Common::Array<Graphics::ManagedSurface *> _cpcActionIndicators;
	uint32 _cpcActionIndicatorUntilMillis;
	Common::Array<Graphics::ManagedSurface *> _c64ModeFrames;

	// Dark Side Amiga stores the grounded jetpack indicator states as raw
	// 4-plane bitplane data. The executable drives those frames through a tiny
	// fixed color ramp, so the renderer keeps the raw planes and applies a
	// hardcoded palette at draw time.
	Common::Array<Common::Array<byte>> _jetpackTransitionFrames;
	Common::Array<byte> _jetpackCrouchFrame;
	Common::Array<Graphics::ManagedSurface *> _amigaCompassYawFrames;
	Graphics::ManagedSurface *_amigaCompassPitchMarker;
	Common::Array<Graphics::ManagedSurface *> _amigaCompassNeedleFrames;
	Common::Array<Graphics::ManagedSurface *> _amigaCompassLeftFrames;
	Common::Array<Graphics::ManagedSurface *> _amigaCompassRightFrames;
	bool _amigaCompassYawPhaseInitialized;
	int _amigaCompassYawPhase;
	int _amigaCompassYawLastUpdateTick;
	bool _jetpackIndicatorStateInitialized;
	bool _jetpackIndicatorLastFlyMode;
	int _jetpackIndicatorTransitionFrame;
	int _jetpackIndicatorTransitionDirection;
	uint32 _jetpackIndicatorNextFrameMillis;
	void loadJetpackRawFrames(Common::SeekableReadStream *file);
	void loadAmigaIndicatorSprites(Common::SeekableReadStream *file, byte *palette);
	void loadAmigaCompass(Common::SeekableReadStream *file, byte *palette);
	void drawAmigaCompass(Graphics::Surface *surface);
	void drawAmigaAmbientIndicators(Graphics::Surface *surface);
	void drawJetpackIndicator(Graphics::Surface *surface);

	int _soundIndexRestoreECD;
	int _soundIndexDestroyECD;
	Audio::SoundHandle _soundFxHandleJetpack;

	DarkSideC64SFXPlayer *_playerC64Sfx;
	MusicPlayer *_playerMusic;
	bool _c64UseSFX;
	bool _c64CompassInitialized;
	int _c64CompassPosition;
	Common::Array<byte> _c64CompassTable;
	void playSoundC64(int index) override;
	void toggleC64Sound();

	Common::Array<byte> _musicData; // HDSMUSIC.AM TEXT segment (Amiga)

	void drawString(const DarkFontSize size, const Common::String &str, int x, int y, uint32 primaryColor, uint32 secondaryColor, uint32 backColor, Graphics::Surface *surface);
	void drawInfoMenu() override;

	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;

private:
	void addECDs(Area *area);
	void addECD(Area *area, const Math::Vector3d position, int index);
	void restoreECD(Area &area, int index);
	bool checkECD(uint16 areaID, int index);
	bool tryDestroyECD(int index);
	bool tryDestroyECDFullGame(int index);
	void addWalls(Area *area);
	void loadCPCIndicator(Common::SeekableReadStream *file, uint32 offset, Common::Array<Graphics::ManagedSurface *> &target);
	void loadCPCIndicatorData(const byte *data, int widthBytes, int height, Common::Array<Graphics::ManagedSurface *> &target);
	void loadCPCIndicators(Common::SeekableReadStream *file);
	void drawC64ModeIndicator(Graphics::Surface *surface);
	void drawCPCSprite(Graphics::Surface *surface, const Graphics::ManagedSurface *indicator, int xPosition, int yPosition);
	void drawCPCIndicator(Graphics::Surface *surface, int xPosition, int yPosition);
	void drawVerticalCompass(Graphics::Surface *surface, int x, int y, float angle, uint32 color);
	void drawHorizontalCompass(int x, int y, float angle, uint32 front, uint32 back, Graphics::Surface *surface);
};

}
