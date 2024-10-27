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
	void updateStaticText(int textID, const char *txt);

	void showText(const char *txt, const mgVect2f& pos, int fontID, int escapeID);
	void showNumber(int num, const mgVect2f& pos, int fontID, int escapeID);

	void quant(float dt);
	void updateScore(int score);
	void updateTime(int seconds);

private:
	struct Font {
		mgVect2f size;
		ObjectContainer pool;
	};
	typedef Std::vector<Font> Fonts;

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
	typedef Std::vector<Escape> Escapes;

	struct StaticTextPreset {
		StaticTextPreset();
		mgVect3f pos;
		int font;
		TextAlign align;
		char format[16];
		int textID;
	};
	bool getStaticPreset(StaticTextPreset& preset, const char *name) const;

	struct StaticMessage {
		StaticMessage(Font *font = 0, TextAlign _align = ALIGN_CENTER);
		void release();

		bool empty() const {
			return _objects.empty();
		}

		void setText(const char *str);

		int _depth;
		mgVect2f _pos;
		TextAlign _align;

	protected:
		void update();
		bool validSymbol(unsigned char ch) const {
			return ch > ' ' && ch != '_';
		}

	private:
		Font *_font;

		QDObjects _objects;
	};
	typedef Std::vector<StaticMessage> StaticMessages;

	struct Message : public StaticMessage {
		Message(Font *font = 0);
		void release();

		void quant(float dt);

		float _time;
		mgVect2f _vel;
		mgVect2f _accel;

	};
	typedef Std::vector<Message> Messages;

	Fonts _fonts;
	Escapes _escapes;
	StaticTextPreset _show_scores;
	StaticTextPreset _show_time;
	StaticMessages _staticMsgs;
	Messages _flowMsgs;

	int _targetScore;
	int _currentScore;
	float _scoreUpdateTime;
	float _scoreUpdateTimer;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_TEXT_MANAGER_H
