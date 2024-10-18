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
		if (const char * descr = runtime->parameter(str_cache, false)) {
			sscanf(descr, "%255s", str_cache);
			Font digit;
			if (!digit.pool.load(str_cache))
				break;

			debugCN(2, kDebugMinigames, "TextManager(): %d character set \"%s\" loaded, ", idx, str_cache);

			snprintf(str_cache, 127, "font_size_%d", idx);
			if ((descr = runtime->parameter(str_cache, false))) {
				int read = sscanf(descr, "%f %f", &digit.size.x, &digit.size.y);
				if (read != 2)
					warning("TextManager(): incorrect font size definition in [%s]", str_cache);
			} else {
				QDObject obj = digit.pool.getObject();
				obj.setState("0");
				digit.size = runtime->getSize(obj);
				digit.pool.releaseObject(obj);
			}
			debugC(2, kDebugMinigames, "set size to (%5.1f, %5.1f)\n", digit.size.x, digit.size.y);
			fonts_.push_back(digit);
		} else
			break;
	}

	for (int idx = 0;; ++idx) {
		snprintf(str_cache, 127, "register_particle_escape_%d", idx);
		if (const char * descr = runtime->parameter(str_cache, false)) {
			Escape escape;
			int read = sscanf(descr, "%d (%f><%f, %f><%f) (%f><%f, %f><%f) %f '%15s",
			                  &escape.depth,
			                  &escape.vel_min.x, &escape.vel_max.x, &escape.vel_min.y, &escape.vel_max.y,
			                  &escape.accel_min.x, &escape.accel_max.x, &escape.accel_min.y, &escape.accel_max.y,
			                  &escape.aliveTime, escape.format);

			if (read != 11)
				warning("TextManager(): incorrect particle definition in [%s]", str_cache);

			if (read != 11)
				break;
			escapes_.push_back(escape);
		} else
			break;
	}
	debugCN(2, kDebugMinigames, "TextManager(): registered %d particle escapes", escapes_.size());

	if (getStaticPreset(show_scores_, "show_scores"))
		show_scores_.textID = createStaticText(show_scores_.pos, show_scores_.font, show_scores_.align);
	else
		show_scores_.textID = -1;

	if (getStaticPreset(show_time_, "show_time"))
		show_time_.textID = createStaticText(show_time_.pos, show_time_.font, show_time_.align);
	else
		show_time_.textID = -1;

	targetScore_ = 0;
	currentScore_ = 0;
	scoreUpdateTimer_ = 0.f;

	scoreUpdateTime_ = getParameter("score_update_time", 0.1f);
}

bool TextManager::getStaticPreset(StaticTextPreset& preset, const char* name) const {
	if (const char * descr = runtime->parameter(name, false)) {
		int align = 0;
		char str[64];
		str[63] = 0;
		int read = sscanf(descr, "%d %d |%63s", &align, &preset.font, str);

		if (read != 3)
				warning("TextManager::getStaticPreset(): Incorrect text format description in %s", transCyrillic(name));

		if (read != 3)
			return false;

		char *pos_obj = strchr(str, '|');

		if (!pos_obj)
				warning("TextManager::getStaticPreset(): Incorrect text format description (2) in %s", transCyrillic(name));

		if (!pos_obj)
			return false;
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

		if (QDObject obj = runtime->getObject(pos_obj)) {
			preset.pos = runtime->world2game(obj);
			runtime->release(obj);
		} else
			return false;
	} else
		return false;

	return true;
}

TextManager::~TextManager() {
	for (auto &mit : flowMsgs_)
		mit.release();

	for (auto &sit : staticMsgs_)
		sit.release();

	for (auto &dit : fonts_)
		dit.pool.release();
}

int TextManager::createStaticText(const mgVect3f& pos, int fontID, TextAlign align) {
	assert(fontID >= 0 && fontID < fonts_.size());

	StaticMessage msg(&fonts_[fontID]);

	msg.align_ = align;
	msg.depth_ = pos.z;
	msg.pos_ = mgVect2f(pos.x, pos.y);

	staticMsgs_.push_back(msg);
	return (int)staticMsgs_.size() - 1;
}

void TextManager::updateStaticText(int textID, const char* txt) {
	assert(textID >= 0 && textID < staticMsgs_.size());

	staticMsgs_[textID].setText(txt);
}

void TextManager::showText(const char* txt, const mgVect2f& pos, int fontID, int escapeID) {
	assert(fontID >= 0 && fontID < fonts_.size());
	assert(escapeID >= 0 && escapeID < escapes_.size());

	Escape& es = escapes_[escapeID];

	Message msg(&fonts_[fontID]);

	msg.setText(txt);
	if (msg.empty())
		return;

	msg.time_ = es.aliveTime > 0 ? es.aliveTime : 1.e6f;

	msg.depth_ = es.depth;
	msg.pos_ = pos;

	msg.vel_.x = runtime->rnd(es.vel_min.x, es.vel_max.x);
	msg.vel_.y = runtime->rnd(es.vel_min.y, es.vel_max.y);
	msg.accel_.x = runtime->rnd(es.accel_min.x, es.accel_max.x);
	msg.accel_.y = runtime->rnd(es.accel_min.y, es.accel_max.y);

	flowMsgs_.push_back(msg);
}

void TextManager::showNumber(int num, const mgVect2f& pos, int fontID, int escapeID) {
	assert(fontID >= 0 && fontID < fonts_.size());
	assert(escapeID >= 0 && escapeID < escapes_.size());

	char buf[16];
	buf[15] = 0;
	snprintf(buf, 15, escapes_[escapeID].format, num);

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
}

TextManager::StaticMessage::StaticMessage(Font* font, TextAlign align) {
	font_ = font;
	align_ = align;
	depth_ = 0.f;
}

void TextManager::StaticMessage::release() {
	for (auto &it : objects_)
		font_->pool.releaseObject(it);

	objects_.clear();
}

void TextManager::StaticMessage::setText(const char* str) {
	assert(font_);

	if (!str) {
		release();
		return;
	}

	int len = (int)strlen(str);

	if (objects_.size() < len)
		objects_.resize(len);
	else
		while (objects_.size() > len) {
			if (objects_.back())
				font_->pool.releaseObject(objects_.back());
			objects_.pop_back();
		}

	for (int idx = 0; idx < len; ++idx) {
		if (validSymbol(str[idx])) {
			if (!objects_[idx])
				objects_[idx] = font_->pool.getObject();
		} else if (objects_[idx])
			font_->pool.releaseObject(objects_[idx]);
	}

	char name[2];
	name[1] = 0;
	for (int idx = 0; idx < len; ++idx) {
		if (objects_[idx]) {
			name[0] = str[idx];
			objects_[idx].setState(name);
		}
	}

	update();
}

void TextManager::StaticMessage::update() {
	if (objects_.empty())
		return;

	float width = font_->size.x * (objects_.size() - 1);
	float x = pos_.x;
	float y = pos_.y;
	switch (align_) {
	case ALIGN_RIGHT:
		x -= width;
		break;
	case ALIGN_CENTER:
		x -= width / 2.f;
		break;
	default:
		break;
	}
	if (y < -font_->size.y || y > runtime->screenSize().y + font_->size.y
	        || x < -2 * width || x > runtime->screenSize().x + 2 * width) {
		release();
		return;
	}

	for (auto &it : objects_) {
		if (it)
			it->set_R(runtime->game2world(mgVect2f(x, y), depth_));
		x += font_->size.x;
	}
}

TextManager::Message::Message(Font* font)
	: StaticMessage(font) {
	time_ = 0.f;
}

void TextManager::Message::release() {
	StaticMessage::release();
	time_ = 0.f;
}

void TextManager::Message::quant(float dt) {
	if (empty())
		return;

	time_ -= dt;
	if (time_ < 0.f) {
		release();
		return;
	}

	vel_ += accel_ * dt;
	pos_ += vel_ * dt;

	update();
}

void TextManager::quant(float dt) {
	Messages::iterator it = flowMsgs_.begin();
	while (it != flowMsgs_.end()) {
		it->quant(dt);
		if (it->empty())
			it = flowMsgs_.erase(it);
		else
			++it;
	}

	if (show_scores_.textID >= 0) {
		if (scoreUpdateTimer_ >= 0.f && scoreUpdateTimer_ <= runtime->getTime()) {
			int sgn = targetScore_ - currentScore_ < 0 ? -1 : 1;
			int mod = abs(currentScore_ - targetScore_);
			currentScore_ += sgn * (mod / 10 + 1);

			char buf[16];
			buf[15] = 0;
			snprintf(buf, 15, show_scores_.format, currentScore_);
			updateStaticText(show_scores_.textID, buf);

			scoreUpdateTimer_ = currentScore_ != targetScore_ ? runtime->getTime() + scoreUpdateTime_ : -1.f;
		}
	}
}

void TextManager::updateScore(int score) {
	targetScore_ = score;
	if (scoreUpdateTimer_ < 0.f)
		scoreUpdateTimer_ = runtime->getTime();
}

void TextManager::updateTime(int seconds) {
	if (show_time_.textID >= 0) {
		char buf[16];
		buf[15] = 0;
		int h = seconds / 3600;
		seconds -= 3600 * h;
		int minutes = seconds / 60;
		seconds -= 60 * minutes;
		snprintf(buf, 15, show_time_.format, h, minutes, seconds);
		updateStaticText(show_time_.textID, buf);
	}
}

} // namespace QDEngine
