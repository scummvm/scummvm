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

namespace Freescape {

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

	void initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) override;
	void initGameState() override;
	bool checkIfGameEnded() override;
	void endGame() override;

	void gotoArea(uint16 areaID, int entranceID) override;

	void drawInfoMenu() override;
	void drawSensorShoot(Sensor *sensor) override;
	void drawCompass(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, double fov, uint32 color);

	void pressedKey(const int keycode) override;
	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;

private:
	bool drillDeployed(Area *area);
	GeometricObject *_drillBase;
	Math::Vector3d drillPosition();
	void addDrill(const Math::Vector3d position, bool gasFound);
	bool checkDrill(const Math::Vector3d position);
	void removeDrill(Area *area);
	void addSkanner(Area *area);

	void loadAssets() override;
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

	void updateTimeVariables() override;

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
	Graphics::ManagedSurface *_borderExtra;
	Texture *_borderExtraTexture;

	Common::SeekableReadStream *decryptFileAtari(const Common::Path &filename);
};

enum DrillerReleaseFlags {
	GF_AMIGA_MAGAZINE_DEMO = (1 << 0),
	GF_ATARI_MAGAZINE_DEMO = (1 << 1),
};

}
