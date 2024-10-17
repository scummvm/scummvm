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

#ifndef QDENGINE_MINIGAMES_ADV_TEXT_MANAGER_H
#define QDENGINE_MINIGAMES_ADV_TEXT_MANAGER_H

#include "qdengine/minigames/adv/ObjectContainer.h"

namespace QDEngine {

enum TextAlign {
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER
};

class TextManager {
public:
	TextManager();
	~TextManager();

	int createStaticText(const mgVect3f& screen_pos, int fontID, TextAlign align);
	void updateStaticText(int textID, const char* txt);

	void showText(const char* txt, const mgVect2f& pos, int fontID, int escapeID);
	void showNumber(int num, const mgVect2f& pos, int fontID, int escapeID);

	void quant(float dt);
	void updateScore(int score);
	void updateTime(int seconds);

private:
	struct Font {
		mgVect2f size;
		ObjectContainer pool;
	};
	typedef vector<Font> Fonts;

	struct Escape {
		Escape();
		int depth;
		float aliveTime;
		mgVect2f vel_min;
		mgVect2f vel_max;
		mgVect2f accel_min;
		mgVect2f accel_max;
		char format[16];
	};
	typedef vector<Escape> Escapes;

	struct StaticTextPreset {
		StaticTextPreset();
		mgVect3f pos;
		int font;
		TextAlign align;
		char format[16];
		int textID;
	};
	bool getStaticPreset(StaticTextPreset& preset, const char* name) const;

	struct StaticMessage {
		StaticMessage(Font* font = 0, TextAlign align_ = ALIGN_CENTER);
		void release();

		bool empty() const {
			return objects_.empty();
		}

		void setText(const char* str);

		int depth_;
		mgVect2f pos_;
		TextAlign align_;

	protected:
		void update();
		bool validSymbol(unsigned char ch) const {
			return ch > ' ' && ch != '_';
		}

	private:
		Font *font_;

		QDObjects objects_;
	};
	typedef vector<StaticMessage> StaticMessages;

	struct Message : public StaticMessage {
		Message(Font* font = 0);
		void release();

		void quant(float dt);

		float time_;
		mgVect2f vel_;
		mgVect2f accel_;

	};
	typedef vector<Message> Messages;

	Fonts fonts_;
	Escapes escapes_;
	StaticTextPreset show_scores_;
	StaticTextPreset show_time_;
	StaticMessages staticMsgs_;
	Messages flowMsgs_;

	int targetScore_;
	int currentScore_;
	float scoreUpdateTime_;
	float scoreUpdateTimer_;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_TEXT_MANAGER_H
