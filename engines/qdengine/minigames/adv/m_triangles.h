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

MinigameInterface *createMinigameTriangleGame();

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

		void release();
		void debugInfo() const;

		const QDObject &obj() const {
			return isBack_ ? back_ : face_[flip];
		}

		bool hit(const mgVect2f& pos) const;

		int number_; // правильная позиция (номер слота)
		int rotation_; // текущий угол поворота (правильный угол = 0)
		int flip;
		QDObjects face_; // набор возможных углов переворота для лицевой стороны
		QDObject back_; // обратная сторона
		QDObject border_; // рамка
		bool isBack_; // повернут лицом (true) или рубашкой (false)
		bool highlight_;
		bool animated_;

		static const char *getFaceStateName(int angle, bool selected, bool animated, bool instantaneous);
		static const char *getBackStateName(bool selected, bool animated, bool instantaneous);
		static const char *getBorderStateName(bool selected);
	};
	typedef Std::vector<Node> Nodes;

public:
	MinigameTriangle();
	~MinigameTriangle();
	void quant(float dt);

private:
	GameType gameType_;
	Coords positions_;
	int selectDepth_ = 0;

	int fieldLines_ = 0;
	int fieldWidth_ = 0;
	int fieldSize_ = 0;
	Nodes nodes_;
	ObjectContainer selectBorders_[2];
	ObjectContainer backSides_[6];
	int selected_ = 0;
	int hovered_ = 0;

	AnimationState animationState_ = NO_ANIMATION;
	int animatedNodes_[2] =  { 0 };
	float animationTime_ = 0.0;
	float animationTimer_ = 0.0;

	/// очистить рубашку фишки
	void releaseNodeBack(Node& node);
	/// выставить графические состояния соответствующие текущему логическому
	void updateNode(Node& node, int position, int flip = 0, bool quick = false);
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
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_M_TRIANGLES_H
