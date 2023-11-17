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

enum EclipseReleaseFlags {
	GF_ZX_DEMO_CRASH = (1 << 0),
	GF_ZX_DEMO_MICROHOBBY = (1 << 1),
};

class EclipseEngine : public FreescapeEngine {
public:
	EclipseEngine(OSystem *syst, const ADGameDescription *gd);

	void gotoArea(uint16 areaID, int entranceID) override;

	void borderScreen() override;

	void loadAssetsDOSFullGame() override;

	void initDOS();
	void initCPC();
	void initZX();
	void loadAssetsCPCDemo() override;
	void loadAssetsZXDemo() override;
	void executePrint(FCLInstruction &instruction) override;

	void drawDOSUI(Graphics::Surface *surface) override;
	void drawCPCUI(Graphics::Surface *surface) override;
	void drawZXUI(Graphics::Surface *surface) override;


	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;
};

}