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

MinigameInterface *createMinigameScores(MinigameManager *runtime) {
	return new Scores(runtime);
}

Scores::Scores(MinigameManager *runtime) {
	_runtime = runtime;

	const char *fileName = _runtime->parameter("minigame_list");
	if (!fileName || !*fileName)
		return;

	if (!scumm_stricmp(fileName, _runtime->gameListFileName())) {
		error("[minigame_list] must refer to \"%s\"", transCyrillic(_runtime->gameListFileName()));
	}

	const char *gameButtonName = _runtime->parameter("game_miniature_button");
	if (!gameButtonName || !*gameButtonName)
		return;

	warning("STUB: Scores::Scores()");
#if 0
	XBuffer gameData;
	char name[128];
	name[127] = 0;
	for (int num = 1; ; ++num) {
		snprintf(name, 127, "%s%02d", gameButtonName, num);
		if (_runtime->testObject(name)) {
			QDObject obj = _runtime->getObject(name);
			gameData.write(obj->R());
			_games.push_back(_runtime->getObject(name));
		} else
			break;
	}

	if (_games.empty()) {
		error("Game images not found '%s'", transCyrillic(gameButtonName));
	}

	if (!_runtime->processGameData(gameData))
		return;

	_positions.resize(_games.size());
	for (int idx = 0; idx < _games.size(); ++idx)
		gameData.read(_positions[idx]);

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
				if (const MinigameData *data = _runtime->getScore(level, game))
					lvl.data.push_back(GameData(game, *data));
			}
		}
		if (lvl.games.size() > _games.size()) {
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
	_level = 0;
	_preLevel = -1;

	if (!(_bestScore = _runtime->parameter("best_score")))
		return;
	if (!(_bestTime = _runtime->parameter("best_time")))
		return;
	if (!(_lastScore = _runtime->parameter("last_score")))
		return;
	if (!(_lastTime = _runtime->parameter("last_time")))
		return;
	if (!(_currentLevel = _runtime->parameter("current_level")))
		return;

	if (!(_prev = _runtime->getObject(_runtime->parameter("_prevbutton"))))
		return;
	if (!(_next = _runtime->getObject(_runtime->parameter("_nextbutton"))))
		return;

	_outMaxLevel = _runtime->getObject(_runtime->parameter("for_game_level"));
	if (_outMaxLevel) {
		uint level = 0;
		for (; level < levels_.size(); ++level)
			if (levels_[level].data.size() < levels_[level].games.size())
				break;
		if (level < levels_.size())
			_outMaxLevel.setState(Common::String::format("%d", levels_[level].level).c_str());
		else
			_outMaxLevel.setState("all");
	}

	setState(MinigameInterface::RUNNING);

}

Scores::~Scores() {
	_runtime->release(_prev);
	_runtime->release(_next);

	for (auto &it : _games)
		_runtime->release(it);

}

void Scores::quant(float dt) {
	assert(_level >= 0 && _level < (int)levels_.size());
	const Level& lvl = levels_[_level];

	if (_level != _preLevel) {
		_preLevel = _level;


		_runtime->setText(_currentLevel, lvl.level);

		for (int idx = 0; idx < (int)_games.size(); ++idx)
			_runtime->hide(_games[idx]);

		for (int idx = 0; idx < (int)_games.size(); ++idx) {
			if (idx < (int)lvl.data.size()) {
				const GameData& data = lvl.data[idx];
				int gameId = data.num;
				int gameNum;
				for (gameNum = 0; gameNum < (int)lvl.games.size(); ++gameNum)
					if (gameId == lvl.games[gameNum])
						break;
				assert(gameNum < (int)lvl.games.size());
				assert(gameNum < (int)_games.size());
				_games[gameNum].setState(Common::String::format("%d", _level).c_str());
				_games[gameNum]->set_R(_positions[idx]);
				_runtime->setText(getName(_bestScore, idx), data.info._bestScore);
				_runtime->setText(getName(_bestTime, idx), data.info._bestTime);
				_runtime->setText(getName(_lastScore, idx), data.info._lastScore);
				_runtime->setText(getName(_lastTime, idx), data.info._lastTime);
			} else {
				_runtime->setText(getName(_bestScore, idx), "");
				_runtime->setText(getName(_bestTime, idx), "");
				_runtime->setText(getName(_lastScore, idx), "");
				_runtime->setText(getName(_lastTime, idx), "");
			}
		}
	}

	if (_runtime->mouseLeftPressed()) {
		if (_level < (int)levels_.size() - 1 && lvl.data.size() == lvl.games.size() && _next.hit(_runtime->mousePosition()))
			++_level;
		else if (_level > 0 && _prev.hit(_runtime->mousePosition()))
			--_level;
	}
}

const char *Scores::getName(const char *begin, int idx) const {
	static char buf[32];
	buf[31] = 0;
	snprintf(buf, 31, "%s%02d", begin, idx + 1);
	return buf;
}

} // namespace QDEngine
