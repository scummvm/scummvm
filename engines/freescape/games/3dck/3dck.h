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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FREESCAPE_GAMES_3DCK_H
#define FREESCAPE_GAMES_3DCK_H

#include "freescape/freescape.h"

namespace Freescape {

class KitEngine : public FreescapeEngine {
public:
	KitEngine(OSystem *syst, const ADGameDescription *gd);

	void loadAssets() override;
	void initGameState() override;
	void gotoArea(uint16 areaID, int entranceID) override;
	void checkIfStillInArea() override;
	bool checkIfGameEnded() override;
	void borderScreen() override {}
	void drawUI() override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override { return false; }
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override { return false; }

private:
	void loadWorld(Common::SeekableReadStream &file);
	Area *loadArea(Common::SeekableReadStream &file);
	Object *loadObject(Common::SeekableReadStream &file);

	byte _palette[256 * 3];
	uint16 _initialPlayerHeight;
};

} // namespace Freescape

#endif
