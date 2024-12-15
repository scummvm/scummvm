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

	uint32 _initialEnergy;
	uint32 _initialShield;
	uint32 _jetFuelSeconds;
	void addSkanner(Area *area);

	void initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) override;
	void initGameState() override;
	void borderScreen() override;
	bool checkIfGameEnded() override;
	void endGame() override;

	void gotoArea(uint16 areaID, int entranceID) override;
	void pressedKey(const int keycode) override;
	void executePrint(FCLInstruction &instruction) override;

	void initDOS();
	void initAmigaAtari();
	void initZX();
	void initCPC();

	void loadAssets() override;
	void loadAssetsDOSFullGame() override;
	void loadAssetsDOSDemo() override;
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
	void drawZXUI(Graphics::Surface *surface) override;
	void drawCPCUI(Graphics::Surface *surface) override;
	void drawAmigaAtariSTUI(Graphics::Surface *surface) override;

	Font _fontBig;
	Font _fontMedium;
	Font _fontSmall;
	int _soundIndexRestoreECD;
	int _soundIndexDestroyECD;

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
};

}
