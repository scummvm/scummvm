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

	bool _resting;
	int _lastThirtySeconds;
	void updateTimeVariables() override;

	void initDOS();
	void initCPC();
	void initZX();
	void loadAssetsCPCDemo() override;
	void loadAssetsZXDemo() override;

	void initGameState() override;
	void executePrint(FCLInstruction &instruction) override;

	void drawBackground() override;
	void drawDOSUI(Graphics::Surface *surface) override;
	void drawCPCUI(Graphics::Surface *surface) override;
	void drawZXUI(Graphics::Surface *surface) override;
	void drawAnalogClock(Graphics::Surface *surface, int x, int y, uint32 colorHand1, uint32 colorHand2, uint32 colorBack);
	void drawAnalogClockHand(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color);

	soundFx *load1bPCM(Common::SeekableReadStream *file, int offset);

	bool checkIfGameEnded() override;
	void endGame() override;
	void loadSoundsFx(Common::SeekableReadStream *file, int offset, int number) override;
	void playSoundFx(int index, bool sync) override;

	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;
};

}
