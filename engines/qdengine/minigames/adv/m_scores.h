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

#ifndef QDENGINE_MINIGAMES_ADV_M_SCORES_H
#define QDENGINE_MINIGAMES_ADV_M_SCORES_H

#include "qdengine/minigames/adv/MinigameInterface.h"
#include "qdengine/minigames/adv/RunTime.h"

namespace QDEngine {

class Scores : public MinigameInterface {
public:
	Scores(MinigameManager *runtime);
	~Scores();

	void quant(float dt);

private:
	struct GameData {
		GameData(int gameNum, const MinigameData& inf) : num(gameNum), info(inf) {}
		int num;
		MinigameData info;
		bool operator< (const GameData& rsh) const {
			return info._sequenceIndex < rsh.info._sequenceIndex;
		}
	};
	typedef Std::vector<GameData> GameDatas;
	struct Level {
		Level(int lvl = 0) : level(lvl) {}
		int level;
		Indexes games;
		GameDatas data;
		bool operator< (const Level& rsh) const {
			return level < rsh.level;
		}
	};
	typedef Std::vector<Level> Levels;
	Levels levels_;

	const char *_currentLevel;
	const char *_bestScore;
	const char *_bestTime;
	const char *_lastScore;
	const char *_lastTime;

	QDObject _prev;
	QDObject next_;
	QDObject outMaxLevel_;

	QDObjects games_;

	int preLevel_;
	int level_;

	Coords positions_;

	const char *getName(const char* begin, int idx) const;

	MinigameManager *_runtime;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_M_SCORES_H
