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

#ifndef QDENGINE_MINIGAMES_ADV_M_PUZZLE_H
#define QDENGINE_MINIGAMES_ADV_M_PUZZLE_H

#include "qdengine/minigames/adv/MinigameInterface.h"
#include "qdengine/minigames/adv/FlyObject.h"

namespace QDEngine {

class Puzzle : public MinigameInterface {
	struct Node {
		QDObject obj;
		int angle;
		int pos;

		bool inStack() const {
			return pos < -1;
		}
		bool isFree() const {
			return pos == -1;
		}

		Node() : angle(1), pos(-1) {}
	};

	typedef Std::vector<Node> Nodes;

public:
	Puzzle(MinigameManager *runtime);
	~Puzzle();

	void quant(float dt);

private:
	int gameSize_;
	int angles_;

	int globalAngle_;
	float rotateTimePeriod_;
	float nextRotateTime_;

	bool singleSize_;
	mgVect2f size_;
	float depth_;

	Nodes nodes_;
	/// Номер места с которого взяли фрагмент
	int prevPlace_;
	/// Индекс фрагмента на мыши
	int pickedItem_;

	int inField_;

	float nextObjTime_;
	int mouseObjPose_;

	QDObject stackBottom_;
	int stackSize_;
	mgVect2f stackPlaceSize_;

	Indexes stack_;
	Indexes field_;

	FlyQDObjects flyObjs_;
	/// скорость падения новых в стек
	float flySpeed_;
	/// скорость возврата в стек
	float returnSpeed_;

	Coords positions_;

	const char *getStateName(int angle, bool selected, bool small) const;
	/// повернуть фишку
	void rotate(int hovItem);
	/// проверить нахождение фишки на своем месте
	bool testPlace(int idx) const;
	/// фишка на мыши?
	bool isOnMouse(const Node& node) const;
	/// проверить фишку на предмет самостоятельного управления позиционированием
	bool isFlying(int idx) const;
	/// конверсия между номером в стеке и индексом положения
	int stidx(int idx) const;
	/// положить what в ячейку where
	void put(int where, int what, float flowSpeed = -1.f);
	/// положить на вершину инвентори
	void putOnStack(int what, float speed);
	/// вернуть с мыши в инвентори
	void returnToStack();
	/// мировые координаты слота на поле
	const mgVect3f &position(int num) const;
	/// положение N-ой фишки в инвентори
	mgVect3f stackPosition(int N) const;

	MinigameManager *_runtime;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_M_PUZZLE_H
