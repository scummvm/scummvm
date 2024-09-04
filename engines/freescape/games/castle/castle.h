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

class CastleEngine : public FreescapeEngine {
public:
	CastleEngine(OSystem *syst, const ADGameDescription *gd);
	~CastleEngine();

	// Only in DOS
	Graphics::ManagedSurface *_option;
	Graphics::ManagedSurface *_menuButtons;
	Graphics::ManagedSurface *_menuCrawlIndicator;
	Graphics::ManagedSurface *_menuWalkIndicator;
	Graphics::ManagedSurface *_menuRunIndicator;
	Graphics::ManagedSurface *_menuFxOnIndicator;
	Graphics::ManagedSurface *_menuFxOffIndicator;
	Graphics::ManagedSurface *_something;
	Graphics::ManagedSurface *_menu;

	void initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) override;
	void initGameState() override;
	void endGame() override;

	void drawInfoMenu() override;
	void loadAssets() override;
	void loadAssetsDOSFullGame() override;
	void loadAssetsDOSDemo() override;
	void loadAssetsAmigaDemo() override;
	void loadAssetsZXFullGame() override;
	void borderScreen() override;
	void selectCharacterScreen();
	void drawOption();

	void initZX();

	void drawDOSUI(Graphics::Surface *surface) override;
	void drawZXUI(Graphics::Surface *surface) override;
	void drawEnergyMeter(Graphics::Surface *surface);
	void pressedKey(const int keycode) override;
	void checkSensors() override;
	void updateTimeVariables() override;

	bool checkIfGameEnded() override;

	void executePrint(FCLInstruction &instruction) override;
	void executeMakeInvisible(FCLInstruction &instruction) override;
	void executeRedraw(FCLInstruction &instruction) override;
	void gotoArea(uint16 areaID, int entranceID) override;
	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;

	Common::StringArray _riddleList;
	Common::BitArray _fontPlane1;
	Common::BitArray _fontPlane2;
	Common::BitArray _fontPlane3;

	void drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface, int offset = 0) override;
	//void drawStringInSurface(const Common::String &str, int x, int y, uint32 primaryFontColor, uint32 secondaryFontColor, uint32 backColor, Graphics::Surface *surface, int offset = 0) override;
	Common::Array<Graphics::ManagedSurface *> loadFramesWithHeader(Common::SeekableReadStream *file, int pos, int numFrames, uint32 front, uint32 back);
	Graphics::ManagedSurface *loadFrameWithHeader(Common::SeekableReadStream *file, int pos, uint32 front, uint32 back);
	Graphics::ManagedSurface *loadFrame(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height, uint32 back);
	Graphics::ManagedSurface *loadFrameFromPlanes(Common::SeekableReadStream *file, int widthInBytes, int height);
	Graphics::ManagedSurface *loadFrameFromPlanesInternal(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height);

	Graphics::ManagedSurface *_keysFrame;
	Graphics::ManagedSurface *_spiritsMeterIndicatorFrame;
	Graphics::ManagedSurface *_strenghtBackgroundFrame;
	Graphics::ManagedSurface *_strenghtBarFrame;
	Common::Array<Graphics::ManagedSurface *> _strenghtWeightsFrames;
	Graphics::ManagedSurface *_flagFrames[4];
	Graphics::ManagedSurface *_thunderFrame;

	int _numberKeys;
	bool _useRockTravel;
	int _spiritsDestroyed;
	int _spiritsMeter;
	int _spiritsMeterPosition;
	int _spiritsMeterMax;
	int _spiritsToKill;

private:
	Common::SeekableReadStream *decryptFile(const Common::Path &filename);
	void loadRiddles(Common::SeekableReadStream *file, int offset, int number);
	void loadDOSFonts(Common::SeekableReadStream *file, int pos);
	void drawFullscreenRiddleAndWait(uint16 riddle);
	void drawRiddle(uint16 riddle, uint32 front, uint32 back, Graphics::Surface *surface);
	void tryToCollectKey();
	void addGhosts();
	Texture *_optionTexture;
};

}
