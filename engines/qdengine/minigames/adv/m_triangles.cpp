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
#include "common/memstream.h"

#include "qdengine/qdengine.h"
#include "qdengine/minigames/adv/m_triangles.h"
#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/EventManager.h"
#include "qdengine/minigames/adv/qdMath.h"

namespace QDEngine {

MinigameInterface *createMinigameTriangleGame() {
	return new MinigameTriangle;
}

enum {
	EVENT_TURN,
	EVENT_GET_RIGHT,
	EVENT_PUT_RIGHT
};

MinigameTriangle::Node::Node(int number, int rot) {
	number_ = number;
	rotation_ = rot;
	isBack_ = false;
	highlight_ = false;
	animated_ = false;
	flip = 0;
}

void MinigameTriangle::Node::release() {
	for (auto &it : face_)
		g_runtime->release(it);
}

bool MinigameTriangle::Node::hit(const mgVect2f& pos) const {
	return obj().hit(pos);
}

MinigameTriangle::MinigameTriangle() {
	int type = 0;
	if (!getParameter("game_type", type, true))
		return;

	switch (type) {
	case 1:
		gameType_ = RECTANGLE;
		break;
	case 2:
		gameType_ = HEXAGON;
		break;
	default:
		gameType_ = TRIANGLE;
	}

	fieldLines_ = fieldWidth_ = 0;

	if (!getParameter("size", fieldLines_, true))
		return;
	if (fieldLines_ < 2)
		return;

	if (gameType_ == RECTANGLE) {
		if (!getParameter("width", fieldWidth_, true))
			return;
		if (fieldWidth_ < 2)
			return;
	}

	switch (gameType_) {
	case TRIANGLE:
		fieldSize_ = sqr(fieldLines_);
		break;
	case RECTANGLE:
		fieldSize_ = fieldLines_ * fieldWidth_;
		break;
	case HEXAGON:
		assert(fieldLines_ % 2 == 0);
		if (fieldLines_ % 2 != 0)
			return;
		fieldSize_ = 3 * sqr(fieldLines_) / 2;
		break;
	}

	if (!getParameter("animation_time", animationTime_, true))
		return;

	const char *faceNameBegin = g_runtime->parameter("object_name_begin", "obj_");
	const char *backNameBegin = g_runtime->parameter("backg_name_begin", "element_back_");
	const char *selectNameBegin = g_runtime->parameter("select_name_begin", "element_select_");

	char name[64];
	name[63] = 0;
	for (int num = 0; num < fieldSize_; ++num) {
		nodes_.push_back(Node(num, 0));
		Node& node = nodes_.back();
		for (int angle = 1; angle <= 3; ++angle) {
			snprintf(name, 63, "%s%02d_%1d", faceNameBegin, num + 1, angle);
			QDObject obj = g_runtime->getObject(name);
			node.face_.push_back(obj);
			positions_.push_back(obj->R());
		}
	}

	Common::MemoryReadWriteStream gameData(DisposeAfterUse::YES);

	for (auto &it : positions_)
		it.write(gameData);

	if (!g_runtime->processGameData(gameData))
		return;

	for (auto &it : positions_)
		it.read(gameData);

	for (int num = 1; num <= 2; ++num) {
		for (int angle = 1; angle <= 3; ++angle) {
			snprintf(name, 63, "%s%1d_%1d", backNameBegin, num, angle);
			if (!backSides_[(num - 1) * 3 + angle - 1].load(name))
				return;
		}
		snprintf(name, 63, "%s%1d", selectNameBegin, num);
		if (!selectBorders_[num - 1].load(name))
			return;
	}

	selectDepth_ = nodes_[0].face_[0].depth() - 1000;

	selected_ = -1;
	hovered_ = -1;

	animationState_ = NO_ANIMATION;
	animatedNodes_[0] = animatedNodes_[1] = -1;
	animationTimer_ = 0.f;

	if (!g_runtime->debugMode())
		for (int i = 0; i < 150; ++i) {
			int pos1 = g_runtime->rnd(0, nodes_.size() - 1);
			for (int j = 0; j < 20; ++j) {
				int pos2 = g_runtime->rnd(pos1 - 10, pos1 + 10);
				if (compatible(pos1, pos2)) {
					swapNodes(pos1, pos2, true);
					break;
				}
			}
		}

	for (int idx = 0; idx < fieldSize_; ++idx)
		updateNode(nodes_[idx], idx);

	setState(RUNNING);
}

MinigameTriangle::~MinigameTriangle() {
	for (auto &it : nodes_)
		it.release();

	for (int idx = 0; idx < 2; ++idx)
		selectBorders_[idx].release();

	for (int idx = 0; idx < 6; ++idx)
		backSides_[idx].release();
}

void MinigameTriangle::Node::debugInfo() const {
	debugC(5, kDebugMinigames, "name:\"%s\" state:\"%s\" number:%d rotation:%d flip:%d isBack:%d highlight:%d animated:%d", obj().getName(), obj()->current_state_name(), number_, rotation_, flip, isBack_, highlight_, animated_);
}

const char *MinigameTriangle::Node::getFaceStateName(int angle, bool selected, bool animated, bool instantaneous) {
	assert(!selected || !animated); // анимированные выделенными быть не могут

	static const char *angleNames[3] = {"0", "120", "240"};
	assert(angle >= 0 && angle < ARRAYSIZE(angleNames));

	Common::String out;

	out = Common::String::format("%s%s%s", (animated ? "02_" : "01_"), angleNames[angle], (selected || instantaneous ? "_sel" : ""));
	return out.c_str();
}

const char *MinigameTriangle::Node::getBackStateName(bool selected, bool animated, bool instantaneous) {
	assert(!selected || !animated); // анимированные выделенными быть не могут

	if (animated)
		return selected || instantaneous ? "02_sel" : "02";
	else
		return selected || instantaneous ? "01_sel" : "01";
}

const char *MinigameTriangle::Node::getBorderStateName(bool selected) {
	return selected ? "01" : "02";
}

void MinigameTriangle::releaseNodeBack(Node& node) {
	if (node.back_) {
		node.back_.setState(Node::getBackStateName(false, false, false));
		for (int type = 0; type < 6; ++type)
			backSides_[type].releaseObject(node.back_);
	}
}

void MinigameTriangle::updateNode(Node& node, int position, int flip, bool quick) {
	for (auto &fit : node.face_)
		g_runtime->hide(fit);

	node.flip = flip;

	if (node.isBack_) {
		if (!node.back_)
			node.back_ = backSides_[orientation(position) * 3 + flip].getObject();
		node.back_->set_R(slotCoord(position, flip));
		node.back_->update_screen_R();
		node.back_.setState(Node::getBackStateName(node.highlight_, node.animated_, quick));
	} else {
		releaseNodeBack(node);

		QDObject& face = node.face_[flip];
		face->set_R(slotCoord(position, flip));
		face->update_screen_R();
		face.setState(Node::getFaceStateName(node.rotation_, node.highlight_, node.animated_, quick));
	}
}

void MinigameTriangle::highlight(int idx, bool hl) {
	if (idx >= 0) {
		assert(idx < (int)nodes_.size());
		nodes_[idx].highlight_ = hl;
		updateNode(nodes_[idx], idx);
	}
}

void MinigameTriangle::beginSwapNodes(int pos1, int pos2) {
	assert(compatible(pos1, pos2));

	if (pos1 > pos2)
		SWAP(pos1, pos2);

	animationState_ = FIRST_PHASE;
	animationTimer_ = animationTime_;

	animatedNodes_[0] = pos1;
	animatedNodes_[1] = pos2;

	Node& node1 = nodes_[pos1];
	Node& node2 = nodes_[pos2];

	node1.animated_ = true;
	node2.animated_ = true;

	releaseNodeBack(node1);
	releaseNodeBack(node2);

	updateNode(node1, pos1, destination(pos1, pos2));
	updateNode(node2, pos2, destination(pos1, pos2));

	debugC(5, kDebugMinigames, ">>>>>>>>>>>>>>>>>>>>>>>>>>> change %d <> %d, 1st phase <<<<<<<<<<<<<<<<<<<<<<<<<<<<", pos1, pos2);
	nodes_[pos1].debugInfo();
	nodes_[pos2].debugInfo();
}

void MinigameTriangle::endSwapNodes(int pos1, int pos2) {
	Node& node1 = nodes_[pos1];
	Node& node2 = nodes_[pos2];

	bool counted = false;
	if (node1.number_ == pos1) { // поставили на свое место
		assert(!node1.isBack_);
		counted = true;
		g_runtime->event(EVENT_PUT_RIGHT, node1.obj()->screen_R());
	}

	if (node2.number_ == pos1) { // сняли со своего места
		assert(node2.isBack_);
		counted = true;
		g_runtime->event(EVENT_GET_RIGHT, node1.obj()->screen_R());
	}

	if (node2.number_ == pos2) { // поставили на свое место
		assert(!node2.isBack_);
		counted = true;
		g_runtime->event(EVENT_PUT_RIGHT, node2.obj()->screen_R());
	}

	if (node1.number_ == pos2) { // сняли со своего места
		assert(node1.isBack_);
		counted = true;
		g_runtime->event(EVENT_GET_RIGHT, node2.obj()->screen_R());
	}

	if (!counted) { // просто сделали ход
		mgVect2i pos = node1.obj()->screen_R();
		pos += node2.obj()->screen_R();
		pos /= 2;
		g_runtime->event(EVENT_TURN, pos);
	}

	bool isWin = true;
	int position = 0;

	for (auto &it : nodes_) {
		if (it.number_ != position++) {
			isWin = false;
			break;
		}
	}

	if (isWin) {
		setState(GAME_WIN);
		return;
	}
}

bool MinigameTriangle::animate(float dt) {
	if (animationState_ == NO_ANIMATION)
		return false;

	animationTimer_ -= dt;
	if (animationTimer_ > 0)
		return true;

	Node& node1 = nodes_[animatedNodes_[0]];
	Node& node2 = nodes_[animatedNodes_[1]];

	switch (animationState_) {
	case FIRST_PHASE: {
		node1.rotation_ = getRotate(animatedNodes_[0], animatedNodes_[1]);
		node2.rotation_ = getRotate(animatedNodes_[1], animatedNodes_[0]);

		node1.isBack_ = !node1.isBack_;
		node2.isBack_ = !node2.isBack_;

		releaseNodeBack(node1);
		releaseNodeBack(node2);

		for (auto &it : node1.face_)
			it.setState(Node::getFaceStateName(0, false, false, false));

		for (auto &it : node2.face_)
			it.setState(Node::getFaceStateName(0, false, false, false));

		updateNode(node1, animatedNodes_[1], destination(animatedNodes_[0], animatedNodes_[1]), true);
		updateNode(node2, animatedNodes_[0], destination(animatedNodes_[1], animatedNodes_[0]), true);

		animationTimer_ = 0.f;
		animationState_ = SECOND_PHASE;

		debugC(5, kDebugMinigames, ">>>>>>>>>>>>>>>>>>>>>>>>>>> change %d <> %d, 2nd phase 1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<", animatedNodes_[0], animatedNodes_[1]);
		node1.debugInfo();
		node2.debugInfo();

		return true;
	}
	case SECOND_PHASE:
		node1.animated_ = false;
		node2.animated_ = false;

		updateNode(node1, animatedNodes_[1], destination(animatedNodes_[0], animatedNodes_[1]));
		updateNode(node2, animatedNodes_[0], destination(animatedNodes_[1], animatedNodes_[0]));

		SWAP(node1, node2);

		animationTimer_ = animationTime_;
		animationState_ = FIRD_PHASE;

		debugC(5, kDebugMinigames, ">>>>>>>>>>>>>>>>>>>>>>>>>>> change %d <> %d, 2nd phase 2 <<<<<<<<<<<<<<<<<<<<<<<<<<<<", animatedNodes_[0], animatedNodes_[1]);
		node2.debugInfo();
		node1.debugInfo();

		return true;

	case FIRD_PHASE:
		animationTimer_ = 0.f;
		animationState_ = NO_ANIMATION;

		releaseNodeBack(node1);
		releaseNodeBack(node2);

		updateNode(node1, animatedNodes_[0]);
		updateNode(node2, animatedNodes_[1]);

		endSwapNodes(animatedNodes_[0], animatedNodes_[1]);
		debugC(5, kDebugMinigames, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ change %d <> %d, finished ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^", animatedNodes_[0], animatedNodes_[1]);

		animatedNodes_[0] = -1;
		animatedNodes_[1] = -1;

		return true;

	default:
		break;
	}

	return false;
}

void MinigameTriangle::swapNodes(int pos1, int pos2, bool silentQuick) {
	if (silentQuick) {
		Node& node1 = nodes_[pos1];
		Node& node2 = nodes_[pos2];

		node1.rotation_ = getRotate(pos1, pos2);
		node2.rotation_ = getRotate(pos2, pos1);

		node1.isBack_ = !node1.isBack_;
		node2.isBack_ = !node2.isBack_;

		releaseNodeBack(node1);
		releaseNodeBack(node2);

		SWAP(node1, node2);

		updateNode(node1, pos1, 0, true);
		updateNode(node2, pos2, 0, true);
	} else
		beginSwapNodes(pos1, pos2);
}

void MinigameTriangle::quant(float dt) {
	if (selected_ >= 0)
		g_runtime->setGameHelpVariant(0);
	else
		g_runtime->setGameHelpVariant(1);

	if (animate(dt))
		return;

	int mousePos = -1;
	for (int idx = 0; idx < fieldSize_; ++idx)
		if (nodes_[idx].hit(g_runtime->mousePosition())) {
			mousePos = idx;
			break;
		}

	int startAnimation = -1;
	int lastSelected = selected_;

	if (g_runtime->mouseLeftPressed()) {
		if (mousePos < 0)                       // кликнули мимо - снимаем выделение
			selected_ = -1;
		else if (selected_ < 0)                 // ничего выделено небыло, просто выделяем
			selected_ = mousePos;
		else if (selected_ == mousePos)         // кликнули на выделенном - снимаем выделение
			selected_ = -1;
		else if (compatible(selected_, mousePos)) { // поменять фишки местами
			startAnimation = selected_;
			selected_ = -1;
		} else
			selected_ = -1;
	}

	if (selected_ != lastSelected) {
		for (int idx = 0; idx < fieldSize_; ++idx) {
			Node& node = nodes_[idx];
			if (idx == selected_ || compatible(selected_, idx)) { // с этой фишкой можно поменяться
				if (!node.border_)
					node.border_ = selectBorders_[orientation(idx)].getObject();
				node.border_.setState(Node::getBorderStateName(idx == selected_));
				node.border_->set_R(slotCoord(idx));
				node.border_->update_screen_R();
				g_runtime->setDepth(node.border_, selectDepth_);
			} else if (node.border_) {
				selectBorders_[0].releaseObject(node.border_);
				selectBorders_[1].releaseObject(node.border_);
			}
		}
	}

	if (hovered_ != mousePos || selected_ != lastSelected) {
		highlight(hovered_, false);
		highlight(selected_ >= 0 ? selected_ : lastSelected, false);

		hovered_ = mousePos;

		if (hovered_ >= 0 && startAnimation < 0) {
			if (selected_ >= 0) {
				if (compatible(selected_, hovered_)) {
					highlight(hovered_, true);
					highlight(selected_, true);
				}
			} else
				highlight(hovered_, true);
		}
	}

	if (startAnimation >= 0) {
		hovered_ = -1;
		swapNodes(startAnimation, mousePos, false);
	}

	if (g_runtime->mouseRightPressed() && mousePos >= 0) {
		debugC(2, kDebugMinigames, "----- DUBUG INFO FOR %d POSITION --------------------", mousePos);
		debugC(2, kDebugMinigames, "row = %d, begin = %d, orientation = %d", rowByNum(mousePos), rowBegin(rowByNum(mousePos)), orientation(mousePos));
		nodes_[mousePos].debugInfo();
	}
}

int MinigameTriangle::rowBegin(int row) const {
	if (row == fieldLines_)
		return fieldSize_;

	switch (gameType_) {
	case TRIANGLE:
		return sqr(row);
	case RECTANGLE:
		return row * fieldWidth_;
	default:
		break;
	}
	//case HEXAGON:
	assert(row >= 0 && row < fieldLines_);
	if (row >= fieldLines_ / 2) {
		row -= fieldLines_ / 2;
		return fieldSize_ / 2 + (2 * fieldLines_ - row) * row;
	}
	return (fieldLines_ + row) * row;

}

int MinigameTriangle::rowByNum(int num) const {
	if (num >= fieldSize_)
		return fieldLines_;

	switch (gameType_) {
	case TRIANGLE:
		return floor(sqrt((float)num));
	case RECTANGLE:
		return num / fieldWidth_;
	default:
		break;
	}
	//case HEXAGON:
	int row = num < fieldSize_ / 2 ? 0 : fieldLines_ / 2;
	while (row < fieldLines_ && num >= rowBegin(row))
		++row;
	return row > 0 ? row - 1 : 0;
}

int MinigameTriangle::orientation(int num) const {
	switch (gameType_) {
	case TRIANGLE:
		return (rowByNum(num) + num) % 2;
	case RECTANGLE:
		return num % 2;
	default:
		break;
	}
	//case HEXAGON:
	return (num + rowByNum(num) + (num >= fieldSize_ / 2 ? 1 : 0)) % 2;
}

bool MinigameTriangle::compatible(int num1, int num2) const {
	if (num1 > num2)
		SWAP(num1, num2);

	if (num1 < 0)
		return false;

	int row1 = rowByNum(num1);
	int row2 = rowByNum(num2);

	if (row2 >= fieldLines_)
		return false;

	if (row1 == row2) // в одном слое
		return num2 - num1 == 1; // должны быть рядом
	else if (row2 - row1 != 1) // или на соседних слоях
		return false;
	else if (orientation(num1) != 0) // широкими сторонами друг к другу
		return false;

	int center1 = (rowBegin(row1) + rowBegin(row1 + 1) - 1) / 2;
	int center2 = (rowBegin(row2) + rowBegin(row2 + 1) - 1) / 2;

	return center1 - num1 == center2 - num2; // и точно друг под другом
}

int MinigameTriangle::getRotate(int num1, int num2) const {
	static int solves[3][2][3] = {
		{{0, 2, 1}, {0, 2, 1}},
		{{2, 1, 0}, {1, 0, 2}},
		{{1, 0, 2}, {2, 1, 0}}
	};
	assert(compatible(num1, num2));
	return solves[rowByNum(num1) != rowByNum(num2) ? 0 : (num2 < num1 ? 1 : 2)]
	       [orientation(num1)][nodes_[num1].rotation_];
}

int MinigameTriangle::destination(int num1, int num2) const {
	if (orientation(num1) == 0)
		return rowByNum(num1) != rowByNum(num2) ? 0 : (num2 < num1 ? 1 : 2);
	else
		return rowByNum(num1) != rowByNum(num2) ? 0 : (num2 < num1 ? 2 : 1);
}

mgVect3f MinigameTriangle::slotCoord(int pos, int angle) const {
	assert(pos * 3 + angle < (int)positions_.size());
	return positions_[pos * 3 + angle];
}

} // namespace QDEngine
