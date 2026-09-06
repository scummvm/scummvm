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
	struct ConditionData {
		Common::String name;
		Common::Array<byte> code;
	};

	struct SensorData {
		byte colors[2] = {};
		uint16 interval = 0;
		uint16 range = 0;
		uint16 unknown = 0;
		uint16 directions = 0;
	};

	struct ObjectData {
		uint16 id = 0;
		byte type = 0;
		byte flags = 0;
		uint16 state = 0;
		Math::Vector3d origin, size, initialOrigin;
		Common::Array<uint16> members;
		Common::Array<uint16> extra;
		Common::Array<byte> code;
		SensorData sensor;
	};

	struct AreaData {
		Common::HashMap<uint16, ObjectData> objects;
		Common::Array<ConditionData> conditions;
	};

	void loadWorld(Common::SeekableReadStream &file);
	Area *loadArea(Common::SeekableReadStream &file);
	Object *loadObject(Common::SeekableReadStream &file, ObjectData &data);
	Common::Array<ConditionData> loadConditions(Common::SeekableReadStream &file);

	Common::HashMap<uint16, AreaData> _areaData;
	Common::Array<ConditionData> _globalConditions;
	Common::Array<uint16> _indicatorData;

	byte _palette[256 * 3];
	uint16 _initialPlayerHeight;
};

} // namespace Freescape

#endif
