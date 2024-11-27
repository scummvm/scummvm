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

#include "common/debug.h"

#include "qdengine/qdengine.h"
#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/m_scores.h"

namespace QDEngine {

MinigameInterface *createGame() {
	return new Scores;
}

Scores::Scores() {
	const char *fileName = g_runtime->parameter("minigame_list");
	if (!fileName || !*fileName)
		return;

	if (!scumm_stricmp(fileName, g_runtime->gameListFileName())) {
		error("[minigame_list] must refer to \"%s\"", transCyrillic(g_runtime->gameListFileName()));
	}

	const char *gameButtonName = g_runtime->parameter("game_miniature_button");
	if (!gameButtonName || !*gameButtonName)
		return;

	warning("STUB: Scores::Scores()");
#if 0
	XBuffer gameData;
	char name[128];
	name[127] = 0;
	for (int num = 1; ; ++num) {
		snprintf(name, 127, "%s%02d", gameButtonName, num);
		if (g_runtime->testObject(name)) {
			QDObject obj = g_runtime->getObject(name);
			gameData.write(obj->R());
			games_.push_back(g_runtime->getObject(name));
		} else
			break;
	}

	if (games_.empty()) {
		error("Game images not found '%s'", transCyrillic(gameButtonName));
	}

	if (!g_runtime->processGameData(gameData))
		return;

	positions_.resize(games_.size());
	for (int idx = 0; idx < games_.size(); ++idx)
		gameData.read(positions_[idx]);

	XStream file(false);
	if (!file.open(fileName, XS_IN)) {
		error("Failed to open games list file '%s'", transCyrillic(fileName));
	}

	char read_buf[512];
	while (!file.eof()) {
		file.getline(read_buf, 512);
		XBuffer xbuf((void*)read_buf, strlen(read_buf));
		int level;
		xbuf >= level;
		unsigned char ch;
		xbuf > ch;
		if (ch != ':') {
			error("Wrong file format");
		}
		Level lvl(level);
		debugCN(2, kDebugMinigames, "%d: ", level);
		while (xbuf.tell() < xbuf.size()) {
			xbuf > ch;
			if (Common::isDigit(ch)) {
				--xbuf;
				int game;
				xbuf >= game;
				lvl.games.push_back(game);
				debugCN(2, kDebugMinigames, "%d, ", game);
				if (const MinigameData * data = g_runtime->getScore(level, game))
					lvl.data.push_back(GameData(game, *data));
			}
		}
		if (lvl.games.size() > games_.size()) {
			error("Not enough game images");
		}
		Common::sort(lvl.data.begin(), lvl.data.end());
		levels_.push_back(lvl);
		debugC(2, kDebugMinigames, "");
	}
#endif

	if (levels_.empty())
		return;
	Common::sort(levels_.begin(), levels_.end());
	level_ = 0;
	preLevel_ = -1;

	if (!(bestScore_ = g_runtime->parameter("best_score")))
		return;
	if (!(bestTime_ = g_runtime->parameter("best_time")))
		return;
	if (!(lastScore_ = g_runtime->parameter("last_score")))
		return;
	if (!(lastTime_ = g_runtime->parameter("last_time")))
		return;
	if (!(currentLevel_ = g_runtime->parameter("current_level")))
		return;

	if (!(prev_ = g_runtime->getObject(g_runtime->parameter("prev_button"))))
		return;
	if (!(next_ = g_runtime->getObject(g_runtime->parameter("next_button"))))
		return;

	outMaxLevel_ = g_runtime->getObject(g_runtime->parameter("for_game_level"));
	if (outMaxLevel_) {
		uint level = 0;
		for (; level < levels_.size(); ++level)
			if (levels_[level].data.size() < levels_[level].games.size())
				break;
		if (level < levels_.size())
			outMaxLevel_.setState(Common::String::format("%d", levels_[level].level).c_str());
		else
			outMaxLevel_.setState("all");
	}

	setState(MinigameInterface::RUNNING);

}

Scores::~Scores() {
	g_runtime->release(prev_);
	g_runtime->release(next_);

	for (auto &it : games_)
		g_runtime->release(it);

}

void Scores::quant(float dt) {
	assert(level_ >= 0 && level_ < (int)levels_.size());
	const Level& lvl = levels_[level_];

	if (level_ != preLevel_) {
		preLevel_ = level_;


		g_runtime->setText(currentLevel_, lvl.level);

		for (int idx = 0; idx < (int)games_.size(); ++idx)
			g_runtime->hide(games_[idx]);

		for (int idx = 0; idx < (int)games_.size(); ++idx) {
			if (idx < (int)lvl.data.size()) {
				const GameData& data = lvl.data[idx];
				int gameId = data.num;
				int gameNum;
				for (gameNum = 0; gameNum < (int)lvl.games.size(); ++gameNum)
					if (gameId == lvl.games[gameNum])
						break;
				assert(gameNum < (int)lvl.games.size());
				assert(gameNum < (int)games_.size());
				games_[gameNum].setState(Common::String::format("%d", level_).c_str());
				games_[gameNum]->set_R(positions_[idx]);
				g_runtime->setText(getName(bestScore_, idx), data.info._bestScore);
				g_runtime->setText(getName(bestTime_, idx), data.info._bestTime);
				g_runtime->setText(getName(lastScore_, idx), data.info._lastScore);
				g_runtime->setText(getName(lastTime_, idx), data.info._lastTime);
			} else {
				g_runtime->setText(getName(bestScore_, idx), "");
				g_runtime->setText(getName(bestTime_, idx), "");
				g_runtime->setText(getName(lastScore_, idx), "");
				g_runtime->setText(getName(lastTime_, idx), "");
			}
		}
	}

	if (g_runtime->mouseLeftPressed()) {
		if (level_ < (int)levels_.size() - 1 && lvl.data.size() == lvl.games.size() && next_.hit(g_runtime->mousePosition()))
			++level_;
		else if (level_ > 0 && prev_.hit(g_runtime->mousePosition()))
			--level_;
	}
}

const char *Scores::getName(const char* begin, int idx) const {
	static char buf[32];
	buf[31] = 0;
	snprintf(buf, 31, "%s%02d", begin, idx + 1);
	return buf;
}

} // namespace QDEngine
