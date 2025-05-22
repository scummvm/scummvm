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

#ifndef QDENGINE_MINIGAMES_ADV_M_TRIANGLES_H
#define QDENGINE_MINIGAMES_ADV_M_TRIANGLES_H

#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/MinigameInterface.h"
#include "qdengine/minigames/adv/ObjectContainer.h"

namespace QDEngine {

MinigameInterface *createMinigameTriangle(MinigameManager *runtime);

class MinigameTriangle : public MinigameInterface {
	enum GameType {
		TRIANGLE,
		RECTANGLE,
		HEXAGON
	};

	enum AnimationState {
		NO_ANIMATION,
		FIRST_PHASE,
		SECOND_PHASE,
		FIRD_PHASE
	};

	struct Node {
		Node(int number = -1, int rot = -1);

		void release(MinigameManager *runtime);
		void debugInfo() const;

		const QDObject &obj() const {
			return _isBack ? _back : _face[_flip];
		}

		bool hit(const mgVect2f &pos) const;

		int _number; // правильная позиция (номер слота)
		int _rotation; // текущий угол поворота (правильный угол = 0)
		int _flip;
		QDObjects _face; // набор возможных углов переворота для лицевой стороны
		QDObject _back; // обратная сторона
		QDObject _border; // рамка
		bool _isBack; // повернут лицом (true) или рубашкой (false)
		bool _highlight;
		bool _animated;

		static const char *getFaceStateName(int angle, bool selected, bool animated, bool instantaneous);
		static const char *getBackStateName(bool selected, bool animated, bool instantaneous);
		static const char *getBorderStateName(bool selected);
	};
	typedef Std::vector<Node> Nodes;

public:
	MinigameTriangle(MinigameManager *runtime);
	~MinigameTriangle();
	void quant(float dt);

private:
	GameType _gameType = TRIANGLE;
	Coords _positions;
	int _selectDepth = 0;

	int _fieldLines = 0;
	int _fieldWidth = 0;
	int _fieldSize = 0;
	Nodes _nodes;
	ObjectContainer _selectBorders[2];
	ObjectContainer _backSides[6];
	int _selected = 0;
	int _hovered = 0;

	bool _quickReselect;

	AnimationState _animationState = NO_ANIMATION;
	int _animatedNodes[2] =  { 0 };
	float _animationTime = 0.0;
	float _animationTimer = 0.0;

	/// очистить рубашку фишки
	void releaseNodeBack(Node &node);
	/// выставить графические состояния соответствующие текущему логическому
	void updateNode(Node &node, int position, int flip = 0, bool quick = false);
	/// подсветить/потушить фрагмент
	void highlight(int idx, bool hl);

	/// поменять местами фишки
	void swapNodes(int pos1, int pos2, bool quick);
	/// начать анимацию обмена
	void beginSwapNodes(int pos1, int pos2);
	/// отработка анимации переворота фишек
	bool animate(float dt);
	/// вызывается после окончания переворота
	void endSwapNodes(int pos1, int pos2);

	/// по номеру фишки вычисляет слой
	int rowByNum(int num) const;
	/// возвращает с какой фишки начинается слой
	int rowBegin(int row) const;
	/// 0 - угол вверх
	int orientation(int num) const;
	/// можно поменять местами
	bool compatible(int num1, int num2) const;
	/// определить какой будет угол поворота у num1 при переходе в num2
	int getRotate(int num1, int num2) const;
	/// направление переворота
	int destination(int num1, int num2) const;
	/// по номеру слота и углу переворота (с учетом типа игры) возвращает экранные координаты
	mgVect3f slotCoord(int pos, int angle = 0) const;

	MinigameManager *_runtime;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_M_TRIANGLES_H
