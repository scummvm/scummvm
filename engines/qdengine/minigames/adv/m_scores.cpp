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

#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/m_scores.h"

namespace QDEngine {

MinigameInterface *createGame() {
	return new Scores;
}

Scores::Scores() {
	const char *fileName = runtime->parameter("minigame_list");
	if (!fileName || !*fileName)
		return;

	if (!_stricmp(fileName, runtime->gameListFileName())) {
		xxassert(0, (XBuffer() < "[minigame_list] должен ссылаться на \"" < runtime->gameListFileName() < "\"").c_str());
		return;
	}

	const char *gameButtonName = runtime->parameter("game_miniature_button");
	if (!gameButtonName || !*gameButtonName)
		return;

	XBuffer gameData;
	char name[128];
	name[127] = 0;
	for (int num = 1; ; ++num) {
		_snprintf(name, 127, "%s%02d", gameButtonName, num);
		if (runtime->testObject(name)) {
			QDObject obj = runtime->getObject(name);
			gameData.write(obj->R());
			games_.push_back(runtime->getObject(name));
		} else
			break;
	}

	if (games_.empty()) {
		xxassert(false, (XBuffer() < "Не найдены образы игр \"" < gameButtonName < "\"").c_str());
		return;
	}

	if (!runtime->processGameData(gameData))
		return;

	positions_.resize(games_.size());
	for (int idx = 0; idx < games_.size(); ++idx)
		gameData.read(positions_[idx]);

	XStream file(false);
	if (!file.open(fileName, XS_IN)) {
		xxassert(false, (XBuffer() < "Не удалось открыть файл со списком игр \"" < fileName < "\"").c_str());
		return;
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
			xxassert(ch != ':', "Неправильный формат файла.");
			return;
		}
		Level lvl(level);
		debugCN(2, kDebugMinigames, "%d: ", level);
		while (xbuf.tell() < xbuf.size()) {
			xbuf > ch;
			if (isdigit(ch)) {
				--xbuf;
				int game;
				xbuf >= game;
				lvl.games.push_back(game);
				debugCN(2, kDebugMinigames, "%d, ", game);
				if (const MinigameData * data = runtime->getScore(level, game))
					lvl.data.push_back(GameData(game, *data));
			}
		}
		if (lvl.games.size() > games_.size()) {
			xxassert(lvl.games.size() <= games_.size(), "Мало образов игр");
			return;
		}
		sort(lvl.data.begin(), lvl.data.end());
		levels_.push_back(lvl);
		debugC(2, kDebugMinigames, "");
	}
	if (levels_.empty())
		return;
	sort(levels_.begin(), levels_.end());
	level_ = 0;
	preLevel_ = -1;

	if (!(bestScore_ = runtime->parameter("best_score")))
		return;
	if (!(bestTime_ = runtime->parameter("best_time")))
		return;
	if (!(lastScore_ = runtime->parameter("last_score")))
		return;
	if (!(lastTime_ = runtime->parameter("last_time")))
		return;
	if (!(currentLevel_ = runtime->parameter("current_level")))
		return;

	if (!(prev_ = runtime->getObject(runtime->parameter("prev_button"))))
		return;
	if (!(next_ = runtime->getObject(runtime->parameter("next_button"))))
		return;

	outMaxLevel_ = runtime->getObject(runtime->parameter("for_game_level"));
	if (outMaxLevel_) {
		int level = 0;
		for (; level < levels_.size(); ++level)
			if (levels_[level].data.size() < levels_[level].games.size())
				break;
		if (level < levels_.size())
			outMaxLevel_.setState((XBuffer() <= levels_[level].level).c_str());
		else
			outMaxLevel_.setState("all");
	}

	setState(MinigameInterface::RUNNING);

}

Scores::~Scores() {
	runtime->release(prev_);
	runtime->release(next_);

	QDObjects::iterator it;
	FOR_EACH(games_, it)
	runtime->release(*it);

}

void Scores::quant(float dt) {
	xassert(level_ >= 0 && level_ < levels_.size());
	const Level& lvl = levels_[level_];

	if (level_ != preLevel_) {
		preLevel_ = level_;


		runtime->setText(currentLevel_, lvl.level);

		for (int idx = 0; idx < games_.size(); ++idx)
			runtime->hide(games_[idx]);

		for (int idx = 0; idx < games_.size(); ++idx) {
			if (idx < lvl.data.size()) {
				const GameData& data = lvl.data[idx];
				int gameId = data.num;
				int gameNum;
				for (gameNum = 0; gameNum < lvl.games.size(); ++gameNum)
					if (gameId == lvl.games[gameNum])
						break;
				xassert(gameNum < lvl.games.size());
				xassert(gameNum < games_.size());
				games_[gameNum].setState((XBuffer() < level_).c_str());
				games_[gameNum]->set_R(positions_[idx]);
				runtime->setText(getName(bestScore_, idx), data.info.bestScore_);
				runtime->setText(getName(bestTime_, idx), data.info.bestTime_);
				runtime->setText(getName(lastScore_, idx), data.info.lastScore_);
				runtime->setText(getName(lastTime_, idx), data.info.lastTime_);
			} else {
				runtime->setText(getName(bestScore_, idx), "");
				runtime->setText(getName(bestTime_, idx), "");
				runtime->setText(getName(lastScore_, idx), "");
				runtime->setText(getName(lastTime_, idx), "");
			}
		}
	}

	if (runtime->mouseLeftPressed()) {
		if (level_ < levels_.size() - 1 && lvl.data.size() == lvl.games.size() && next_.hit(runtime->mousePosition()))
			++level_;
		else if (level_ > 0 && prev_.hit(runtime->mousePosition()))
			--level_;
	}
}

const char *Scores::getName(const char* begin, int idx) const {
	static char buf[32];
	buf[31] = 0;
	_snprintf(buf, 31, "%s%02d", begin, idx + 1);
	return buf;
}

} // namespace QDEngine
