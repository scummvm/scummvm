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
#include "qdengine/minigames/adv/TextManager.h"
#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/qdMath.h"

namespace QDEngine {

TextManager::TextManager() {
	char str_cache[256];

	for (int idx = 0;; ++idx) {
		snprintf(str_cache, 127, "register_font_%d", idx);
		if (const char * descr = g_runtime->parameter(str_cache, false)) {
			sscanf(descr, "%255s", str_cache);
			Font digit;
			if (!digit.pool.load(str_cache))
				break;

			debugCN(2, kDebugMinigames, "TextManager(): %d character set \"%s\" loaded, ", idx, str_cache);

			snprintf(str_cache, 127, "font_size_%d", idx);
			if ((descr = g_runtime->parameter(str_cache, false))) {
				int read = sscanf(descr, "%f %f", &digit.size.x, &digit.size.y);
				if (read != 2)
					warning("TextManager(): incorrect font size definition in [%s]", str_cache);
			} else {
				QDObject obj = digit.pool.getObject();
				obj.setState("0");
				digit.size = g_runtime->getSize(obj);
				digit.pool.releaseObject(obj);
			}
			debugC(2, kDebugMinigames, "set size to (%5.1f, %5.1f)\n", digit.size.x, digit.size.y);
			_fonts.push_back(digit);
		} else
			break;
	}

	for (int idx = 0;; ++idx) {
		snprintf(str_cache, 127, "register_particle_escape_%d", idx);
		if (const char * descr = g_runtime->parameter(str_cache, false)) {
			Escape escape;
			int read = sscanf(descr, "%d (%f><%f, %f><%f) (%f><%f, %f><%f) %f '%15s",
			                  &escape.depth,
			                  &escape.vel_min.x, &escape.vel_max.x, &escape.vel_min.y, &escape.vel_max.y,
			                  &escape.accel_min.x, &escape.accel_max.x, &escape.accel_min.y, &escape.accel_max.y,
			                  &escape.aliveTime, escape.format);

			if (read != 11) {
				warning("TextManager(): incorrect particle definition in [%s]", str_cache);
				break;
			}
			_escapes.push_back(escape);
		} else
			break;
	}
	debugCN(2, kDebugMinigames, "TextManager(): registered %d particle escapes", _escapes.size());

	if (getStaticPreset(_show_scores, "show_scores"))
		_show_scores.textID = createStaticText(_show_scores.pos, _show_scores.font, _show_scores.align);
	else
		_show_scores.textID = -1;

	if (getStaticPreset(_show_time, "show_time"))
		_show_time.textID = createStaticText(_show_time.pos, _show_time.font, _show_time.align);
	else
		_show_time.textID = -1;

	_targetScore = 0;
	_currentScore = 0;
	_scoreUpdateTimer = 0.f;

	_scoreUpdateTime = getParameter("score_update_time", 0.1f);
}

bool TextManager::getStaticPreset(StaticTextPreset& preset, const char* name) const {
	if (const char * descr = g_runtime->parameter(name, false)) {
		int align = 0;
		char str[64];
		str[63] = 0;
		int read = sscanf(descr, "%d %d |%63s", &align, &preset.font, str);

		if (read != 3) {
			warning("TextManager::getStaticPreset(): Incorrect text format description in %s", transCyrillic(name));
			return false;
		}

		char *pos_obj = strchr(str, '|');

		if (!pos_obj) {
			warning("TextManager::getStaticPreset(): Incorrect text format description (2) in %s", transCyrillic(name));
			return false;
		}

		*pos_obj = 0;
		++pos_obj;

		strncpy(preset.format, str, 15);

		switch (align) {
		case 0:
			preset.align = ALIGN_RIGHT;
			break;
		case 1:
			preset.align = ALIGN_LEFT;
			break;
		default:
			preset.align = ALIGN_CENTER;
			break;
		}

		if (QDObject obj = g_runtime->getObject(pos_obj)) {
			preset.pos = g_runtime->world2game(obj);
			g_runtime->release(obj);
		} else
			return false;
	} else
		return false;

	return true;
}

TextManager::~TextManager() {
	for (auto &mit : _flowMsgs)
		mit.release();

	for (auto &sit : _staticMsgs)
		sit.release();

	for (auto &dit : _fonts)
		dit.pool.release();
}

int TextManager::createStaticText(const mgVect3f& pos, int fontID, TextAlign align) {
	assert(fontID >= 0 && fontID < _fonts.size());

	StaticMessage msg(&_fonts[fontID]);

	msg._align = align;
	msg._depth = pos.z;
	msg._pos = mgVect2f(pos.x, pos.y);

	_staticMsgs.push_back(msg);
	return (int)_staticMsgs.size() - 1;
}

void TextManager::updateStaticText(int textID, const char* txt) {
	assert(textID >= 0 && textID < _staticMsgs.size());

	_staticMsgs[textID].setText(txt);
}

void TextManager::showText(const char* txt, const mgVect2f& pos, int fontID, int escapeID) {
	assert(fontID >= 0 && fontID < _fonts.size());
	assert(escapeID >= 0 && escapeID < _escapes.size());

	Escape& es = _escapes[escapeID];

	Message msg(&_fonts[fontID]);

	msg.setText(txt);
	if (msg.empty())
		return;

	msg._time = es.aliveTime > 0 ? es.aliveTime : 1.e6f;

	msg._depth = es.depth;
	msg._pos = pos;

	msg._vel.x = g_runtime->rnd(es.vel_min.x, es.vel_max.x);
	msg._vel.y = g_runtime->rnd(es.vel_min.y, es.vel_max.y);
	msg._accel.x = g_runtime->rnd(es.accel_min.x, es.accel_max.x);
	msg._accel.y = g_runtime->rnd(es.accel_min.y, es.accel_max.y);

	_flowMsgs.push_back(msg);
}

void TextManager::showNumber(int num, const mgVect2f& pos, int fontID, int escapeID) {
	assert(fontID >= 0 && fontID < _fonts.size());
	assert(escapeID >= 0 && escapeID < _escapes.size());

	char buf[16];
	buf[15] = 0;
	snprintf(buf, 15, _escapes[escapeID].format, num);

	showText(buf, pos, fontID, escapeID);
}

TextManager::Escape::Escape() {
	depth = 0;
	aliveTime = -1;
	format[15] = 0;
}

TextManager::StaticTextPreset::StaticTextPreset() {
	font = -1;
	align = ALIGN_CENTER;
	format[15] = 0;
	textID = 0;
}

TextManager::StaticMessage::StaticMessage(Font* font, TextAlign align) {
	_font = font;
	_align = align;
	_depth = 0.f;
}

void TextManager::StaticMessage::release() {
	for (auto &it : _objects)
		_font->pool.releaseObject(it);

	_objects.clear();
}

void TextManager::StaticMessage::setText(const char* str) {
	assert(_font);

	if (!str) {
		release();
		return;
	}

	int len = (int)strlen(str);

	if (_objects.size() < len)
		_objects.resize(len);
	else
		while (_objects.size() > len) {
			if (_objects.back())
				_font->pool.releaseObject(_objects.back());
			_objects.pop_back();
		}

	for (int idx = 0; idx < len; ++idx) {
		if (validSymbol(str[idx])) {
			if (!_objects[idx])
				_objects[idx] = _font->pool.getObject();
		} else if (_objects[idx])
			_font->pool.releaseObject(_objects[idx]);
	}

	char name[2];
	name[1] = 0;
	for (int idx = 0; idx < len; ++idx) {
		if (_objects[idx]) {
			name[0] = str[idx];
			_objects[idx].setState(name);
		}
	}

	update();
}

void TextManager::StaticMessage::update() {
	if (_objects.empty())
		return;

	float width = _font->size.x * (_objects.size() - 1);
	float x = _pos.x;
	float y = _pos.y;
	switch (_align) {
	case ALIGN_RIGHT:
		x -= width;
		break;
	case ALIGN_CENTER:
		x -= width / 2.f;
		break;
	default:
		break;
	}
	if (y < -_font->size.y || y > g_runtime->screenSize().y + _font->size.y
	        || x < -2 * width || x > g_runtime->screenSize().x + 2 * width) {
		release();
		return;
	}

	for (auto &it : _objects) {
		if (it)
			it->set_R(g_runtime->game2world(mgVect2f(x, y), _depth));
		x += _font->size.x;
	}
}

TextManager::Message::Message(Font* font)
	: StaticMessage(font) {
	_time = 0.f;
}

void TextManager::Message::release() {
	StaticMessage::release();
	_time = 0.f;
}

void TextManager::Message::quant(float dt) {
	if (empty())
		return;

	_time -= dt;
	if (_time < 0.f) {
		release();
		return;
	}

	_vel += _accel * dt;
	_pos += _vel * dt;

	update();
}

void TextManager::quant(float dt) {
	Messages::iterator it = _flowMsgs.begin();
	while (it != _flowMsgs.end()) {
		it->quant(dt);
		if (it->empty())
			it = _flowMsgs.erase(it);
		else
			++it;
	}

	if (_show_scores.textID >= 0) {
		if (_scoreUpdateTimer >= 0.f && _scoreUpdateTimer <= g_runtime->getTime()) {
			int sgn = _targetScore - _currentScore < 0 ? -1 : 1;
			int mod = abs(_currentScore - _targetScore);
			_currentScore += sgn * (mod / 10 + 1);

			char buf[16];
			buf[15] = 0;
			snprintf(buf, 15, _show_scores.format, _currentScore);
			updateStaticText(_show_scores.textID, buf);

			_scoreUpdateTimer = _currentScore != _targetScore ? g_runtime->getTime() + _scoreUpdateTime : -1.f;
		}
	}
}

void TextManager::updateScore(int score) {
	_targetScore = score;
	if (_scoreUpdateTimer < 0.f)
		_scoreUpdateTimer = g_runtime->getTime();
}

void TextManager::updateTime(int seconds) {
	if (_show_time.textID >= 0) {
		char buf[16];
		buf[15] = 0;
		int h = seconds / 3600;
		seconds -= 3600 * h;
		int minutes = seconds / 60;
		seconds -= 60 * minutes;
		snprintf(buf, 15, _show_time.format, h, minutes, seconds);
		updateStaticText(_show_time.textID, buf);
	}
}

} // namespace QDEngine
