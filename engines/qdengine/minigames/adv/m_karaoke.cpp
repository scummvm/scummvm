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
#include "common/file.h"
#include "common/system.h"

#include "qdengine/qdengine.h"
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

	_controlName = _runtime->parameter("control_name", true);
	if (!_controlName || !*_controlName)
		return;

	_colorReaded = _runtime->parameter("color_first", true);
	if (!_colorReaded || !*_colorReaded)
		return;

	struct Parse {
		Common::File &file;
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
				node.text = Common::String(begin, cur);
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

		Parse(Common::File&  _file, Nodes& _nodes) : file(_file), nodes(_nodes) {
			currentTime = 0.f;
			begin = cur = buf;
			bool prevNumber = false;
			while (!file.eos()) {
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

					file.seek(-1, SEEK_CUR);
					float tm = 0;

					char c;
					do {
						c = read();
					} while ((c >= '0' && c <= '9') || c == '.');

					cur = buf;
					tm = strtod(cur, nullptr);

					if (tm <= 0.f) {
						currentTime = 0.f;
						nodes.push_back(Node());
					} else
						currentTime = tm;
					file.seek(-1, SEEK_CUR);
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

	Common::File file;
	Common::Path path((const char *)transCyrillic(fileName), '\\');

	if (!file.open(path)) {
		// Try with punyencoded path
		if (!file.open(path.punycodeEncode())) {
			assert(!(Common::String() + "Не удалось открыть файл \"" + fileName + "\"").c_str());
			return;
		}
	}

	Parse(file, _nodes);
	debugC(2, kDebugMinigames, "read %d tags", _nodes.size());

	_startScreenTag = 0;
	_currentTag = 0;

	_startTime = 0.001f * g_system->getMillis();
	_startTagTime = 0.f;

	setState(MinigameInterface::RUNNING);

}

void Karaoke::quant(float dt) {
	float curTime = 0.001f * g_system->getMillis() - _startTime;
	if (curTime < 0.f)
		curTime = 0.f;

	Node& node = _nodes[_currentTag];
	if (node.type == CLEAR) {
		++_currentTag;
		if ((uint)_currentTag == _nodes.size())
			setState(MinigameInterface::GAME_WIN);
		_startScreenTag = _currentTag;
		return;
	}

	Common::String outText;
	outText += _colorReaded;
	int idx = _startScreenTag;
	while (idx < _currentTag) {
		assert((uint)idx < _nodes.size());
		assert(_nodes[idx].type == STRING);
		outText += _nodes[idx].text.c_str();
		++idx;
	}

	float phase = (curTime - _startTagTime) / node.time;
	assert(phase >= 0.f);
	if (phase >= 1.f) {
		outText += node.text + "&>";
		++_currentTag;
		_startTagTime += node.time;
		if ((uint)_currentTag == _nodes.size())
			setState(MinigameInterface::GAME_WIN);
	} else {
		int part = phase * node.text.size();
		outText += Common::String(node.text.begin(), node.text.begin() + part) + "&>";
		outText += Common::String(node.text.begin() + part, node.text.end()).c_str();
	}

	++idx;
	while ((uint)idx < _nodes.size()) {
		if (_nodes[idx].type == CLEAR)
			break;
		outText += _nodes[idx].text.c_str();
		++idx;
	}

	if (_runtime->mouseRightPressed())
		debugC(2, kDebugMinigames, "%s", outText.c_str());

	_runtime->setText(_controlName, outText.c_str());
}

} // namespace QDEngine
