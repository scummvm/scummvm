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

#ifndef QDENGINE_MINIGAMES_ADV_M_KARAOKE_H
#define QDENGINE_MINIGAMES_ADV_M_KARAOKE_H

#include "qdengine/minigames/adv/MinigameInterface.h"

namespace QDEngine {

class Karaoke : public MinigameInterface {
public:
	Karaoke(MinigameManager *runtime);
	void quant(float dt);

	enum TagType {
		STRING,
		CLEAR
	};

private:
	const char *controlName_;
	const char *colorReaded_;

	struct Node {
		Node();
		TagType type;
		float time;
		string text;
	};

	typedef vector<Node> Nodes;
	Nodes nodes_;

	float startTime_;
	int startScreenTag_;
	int currentTag_;
	float startTagTime_;

	MinigameManager *_runtime;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_M_KARAOKE_H
