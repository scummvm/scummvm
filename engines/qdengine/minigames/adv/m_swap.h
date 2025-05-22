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

#ifndef QDENGINE_MINIGAMES_ADV_M_SWAP_H
#define QDENGINE_MINIGAMES_ADV_M_SWAP_H

#include "qdengine/minigames/adv/MinigameInterface.h"

namespace QDEngine {

MinigameInterface *createMinigameSwap(MinigameManager *runtime);

class Swap : public MinigameInterface {
public:
	Swap(MinigameManager *runtime);
	~Swap();

	void quant(float dt);
private:
	int _gameSize;
	int _angles;

	float _rotateTimePeriod;
	float _nextRotateTime;

	mgVect2f _size;

	struct Node {
		Node(int idx = -1) : home(idx), angle(0) {}
		QDObject obj;
		int angle;
		int home;
	};
	typedef Std::vector<Node> Nodes;
	Nodes _nodes;

	// Индекс фрагмента на мыши
	int _pickedItem;
	// активные фрагменты после обмена
	int _last1, _last2;

	Coords _positions;

	const char *getStateName(int angle, bool selected) const;
	// поменять местами, если было снятие или укладка на/с правильного места, то true
	void swap(int item1, int item2, bool silent);
	// повернуть фишку
	void rotate(int item1, int item2, bool silent, bool avto = false);
	// погасить выделенные
	void deactivate();
	// проверить нахождение фишки на своем месте
	bool testPlace(int idx) const;
	// поставить объект на свое место и включить нужное состояние
	void put(int idx, bool hl);
	// мировые координаты слота на поле
	const mgVect3f &position(int num) const;

	MinigameManager *_runtime;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_M_SWAP_H
