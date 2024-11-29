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
#include "qdengine/minigames/adv/m_karaoke.h"
#include "qdengine/minigames/adv/RunTime.h"

namespace QDEngine {

MinigameInterface *createMinigameKaraoke(MinigameManager *runtime) {
	return new Karaoke(runtime);
}

Karaoke::Node::Node() {
	type = Karaoke::CLEAR;
	time = 0.f;

}


Karaoke::Karaoke(MinigameManager *runtime) {
	_runtime = runtime;

	controlName_ = _runtime->parameter("control_name", true);
	if (!controlName_ || !*controlName_)
		return;

	colorReaded_ = _runtime->parameter("color_first", true);
	if (!colorReaded_ || !*colorReaded_)
		return;

	struct Parse {
		XStream &file;
		Nodes &nodes;

		float currentTime;

		char buf[1024];
		const char *begin;
		const char *cur;

		void putLine(bool putEmpty = false) {
			if (cur > begin && *begin) {
				Node node;
				node.type = STRING;
				node.time = currentTime;
				node.text = string(begin, cur);
				nodes.push_back(node);
			} else if (putEmpty) {
				Node node;
				node.type = STRING;
				node.time = currentTime;
				nodes.push_back(node);
			}
			begin = cur = buf;
		}

		char read() {
			if (!file.read((void *)cur, 1)) {
				putLine();
				return 0;
			}
			return *cur++;
		}

		Parse(XStream&  _file, Nodes& _nodes) : file(_file), nodes(_nodes) {
			currentTime = 0.f;
			begin = cur = buf;
			bool prevNumber = false;
			while (!file.eof()) {
				switch (read()) {
				case 0:
					return;
				case '/': {
					if (read() == '/') {
						--cur;
						break;
					}
					cur -= 2;
					putLine(prevNumber);
					prevNumber = true;

					file.seek(-1, XS_CUR);
					float tm = 0;
					file >= tm;

					if (tm <= 0.f) {
						currentTime = 0.f;
						nodes.push_back(Node());
					} else
						currentTime = tm;
					file.seek(-1, XS_CUR);
					continue;
				}
				case '>':
					if (prevNumber)
						--cur;
				}
				prevNumber = false;
			}
			putLine();

		}
	};

	const char *fileName = _runtime->parameter("text_file", true);
	if (!fileName)
		return;

	XStream file(false);
	if (!file.open(fileName, XS_IN)) {
		xxassert(false, (XBuffer() < "Не удалось открыть файл \"" < fileName < "\"").c_str());
		return;
	}

	Parse(file, nodes_);
	debugC(2, kDebugMinigames, "read %d tags", nodes_.size());

	startScreenTag_ = 0;
	currentTag_ = 0;

	startTime_ = 0.001f * GetTickCount();
	startTagTime_ = 0.f;

	setState(MinigameInterface::RUNNING);

}

void Karaoke::quant(float dt) {
	float curTime = 0.001f * GetTickCount() - startTime_;
	if (curTime < 0.f)
		curTime = 0.f;

	Node& node = nodes_[currentTag_];
	if (node.type == CLEAR) {
		++currentTag_;
		if (currentTag_ == nodes_.size())
			setState(MinigameInterface::GAME_WIN);
		startScreenTag_ = currentTag_;
		return;
	}

	XBuffer outText;
	outText < colorReaded_;
	int idx = startScreenTag_;
	while (idx < currentTag_) {
		xassert(idx < nodes_.size());
		xassert(nodes_[idx].type == STRING);
		outText < nodes_[idx].text.c_str();
		++idx;
	}

	float phase = (curTime - startTagTime_) / node.time;
	xassert(phase >= 0.f);
	if (phase >= 1.f) {
		outText < node.text.c_str() < "&>";
		++currentTag_;
		startTagTime_ += node.time;
		if (currentTag_ == nodes_.size())
			setState(MinigameInterface::GAME_WIN);
	} else {
		int part = phase * node.text.size();
		outText < string(node.text.begin(), node.text.begin() + part).c_str() < "&>";
		outText < string(node.text.begin() + part, node.text.end()).c_str();
	}

	++idx;
	while (idx < nodes_.size()) {
		if (nodes_[idx].type == CLEAR)
			break;
		outText < nodes_[idx].text.c_str();
		++idx;
	}

	if (_runtime->mouseRightPressed())
		debugC(2, kDebugMinigames, "%s", outText.c_str());

	_runtime->setText(controlName_, outText.c_str());
}

} // namespace QDEngine
