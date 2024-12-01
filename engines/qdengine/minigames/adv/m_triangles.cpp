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

MinigameInterface *createMinigameTriangle(MinigameManager *runtime) {
	return new MinigameTriangle(runtime);
}

enum {
	EVENT_TURN,
	EVENT_GET_RIGHT,
	EVENT_PUT_RIGHT
};

MinigameTriangle::Node::Node(int number, int rot) {
	_number = number;
	_rotation = rot;
	_isBack = false;
	_highlight = false;
	_animated = false;
	_flip = 0;
}

void MinigameTriangle::Node::release(MinigameManager *runtime) {
	for (auto &it : _face)
		runtime->release(it);
}

bool MinigameTriangle::Node::hit(const mgVect2f &pos) const {
	return obj().hit(pos);
}

MinigameTriangle::MinigameTriangle(MinigameManager *runtime) {
	_runtime = runtime;

	int type = 0;
	if (!_runtime->getParameter("game_type", type, true))
		return;

	switch (type) {
	case 1:
		_gameType = RECTANGLE;
		break;
	case 2:
		_gameType = HEXAGON;
		break;
	default:
		_gameType = TRIANGLE;
	}

	_fieldLines = _fieldWidth = 0;

	if (!_runtime->getParameter("size", _fieldLines, true))
		return;
	if (_fieldLines < 2)
		return;

	if (_gameType == RECTANGLE) {
		if (!_runtime->getParameter("width", _fieldWidth, true))
			return;
		if (_fieldWidth < 2)
			return;
	}

	switch (_gameType) {
	case TRIANGLE:
		fieldSize_ = sqr(_fieldLines);
		break;
	case RECTANGLE:
		fieldSize_ = _fieldLines * _fieldWidth;
		break;
	case HEXAGON:
		assert(_fieldLines % 2 == 0);
		if (_fieldLines % 2 != 0)
			return;
		fieldSize_ = 3 * sqr(_fieldLines) / 2;
		break;
	}

	if (!_runtime->getParameter("animation_time", _animationTime, true))
		return;

	const char *faceNameBegin = _runtime->parameter("object_name_begin", "obj_");
	const char *backNameBegin = _runtime->parameter("backg_name_begin", "element__back");
	const char *selectNameBegin = _runtime->parameter("select_name_begin", "element_select_");

	char name[64];
	name[63] = 0;
	for (int num = 0; num < fieldSize_; ++num) {
		_nodes.push_back(Node(num, 0));
		Node &node = _nodes.back();
		for (int angle = 1; angle <= 3; ++angle) {
			snprintf(name, 63, "%s%02d_%1d", faceNameBegin, num + 1, angle);
			QDObject obj = _runtime->getObject(name);
			node._face.push_back(obj);
			_positions.push_back(obj->R());
		}
	}

	Common::MemoryReadWriteStream gameData(DisposeAfterUse::YES);

	for (auto &it : _positions)
		it.write(gameData);

	if (!_runtime->processGameData(gameData))
		return;

	for (auto &it : _positions)
		it.read(gameData);

	for (int num = 1; num <= 2; ++num) {
		for (int angle = 1; angle <= 3; ++angle) {
			snprintf(name, 63, "%s%1d_%1d", backNameBegin, num, angle);
			if (!_backSides[(num - 1) * 3 + angle - 1].load(name, _runtime))
				return;
		}
		snprintf(name, 63, "%s%1d", selectNameBegin, num);
		if (!_selectBorders[num - 1].load(name, _runtime))
			return;
	}

	_selectDepth = _nodes[0]._face[0].depth(_runtime) - 1000;

	_selected = -1;
	_hovered = -1;

	_animationState = NO_ANIMATION;
	_animatedNodes[0] = _animatedNodes[1] = -1;
	_animationTimer = 0.f;

	if (!_runtime->debugMode())
		for (int i = 0; i < 150; ++i) {
			int pos1 = _runtime->rnd(0, _nodes.size() - 1);
			for (int j = 0; j < 20; ++j) {
				int pos2 = _runtime->rnd(pos1 - 10, pos1 + 10);
				if (compatible(pos1, pos2)) {
					swapNodes(pos1, pos2, true);
					break;
				}
			}
		}

	for (int idx = 0; idx < fieldSize_; ++idx)
		updateNode(_nodes[idx], idx);

	setState(RUNNING);
}

MinigameTriangle::~MinigameTriangle() {
	for (auto &it : _nodes)
		it.release(_runtime);

	for (int idx = 0; idx < 2; ++idx)
		_selectBorders[idx].release(_runtime);

	for (int idx = 0; idx < 6; ++idx)
		_backSides[idx].release(_runtime);
}

void MinigameTriangle::Node::debugInfo() const {
	debugC(5, kDebugMinigames, "name:\"%s\" state:\"%s\" number:%d rotation:%d flip:%d isBack:%d highlight:%d animated:%d", obj().getName(), obj()->current_state_name(), _number, _rotation, _flip, _isBack, _highlight, _animated);
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

void MinigameTriangle::releaseNodeBack(Node &node) {
	if (node._back) {
		node._back.setState(Node::getBackStateName(false, false, false));
		for (int type = 0; type < 6; ++type)
			_backSides[type].releaseObject(node._back, _runtime);
	}
}

void MinigameTriangle::updateNode(Node &node, int position, int flip, bool quick) {
	for (auto &fit : node._face)
		_runtime->hide(fit);

	node._flip = flip;

	if (node._isBack) {
		if (!node._back)
			node._back = _backSides[orientation(position) * 3 + flip].getObject();
		node._back->set_R(slotCoord(position, flip));
		node._back->update_screen_R();
		node._back.setState(Node::getBackStateName(node._highlight, node._animated, quick));
	} else {
		releaseNodeBack(node);

		QDObject &face = node._face[flip];
		face->set_R(slotCoord(position, flip));
		face->update_screen_R();
		face.setState(Node::getFaceStateName(node._rotation, node._highlight, node._animated, quick));
	}
}

void MinigameTriangle::highlight(int idx, bool hl) {
	if (idx >= 0) {
		assert(idx < (int)_nodes.size());
		_nodes[idx]._highlight = hl;
		updateNode(_nodes[idx], idx);
	}
}

void MinigameTriangle::beginSwapNodes(int pos1, int pos2) {
	assert(compatible(pos1, pos2));

	if (pos1 > pos2)
		SWAP(pos1, pos2);

	_animationState = FIRST_PHASE;
	_animationTimer = _animationTime;

	_animatedNodes[0] = pos1;
	_animatedNodes[1] = pos2;

	Node &node1 = _nodes[pos1];
	Node &node2 = _nodes[pos2];

	node1._animated = true;
	node2._animated = true;

	releaseNodeBack(node1);
	releaseNodeBack(node2);

	updateNode(node1, pos1, destination(pos1, pos2));
	updateNode(node2, pos2, destination(pos1, pos2));

	debugC(5, kDebugMinigames, ">>>>>>>>>>>>>>>>>>>>>>>>>>> change %d <> %d, 1st phase <<<<<<<<<<<<<<<<<<<<<<<<<<<<", pos1, pos2);
	_nodes[pos1].debugInfo();
	_nodes[pos2].debugInfo();
}

void MinigameTriangle::endSwapNodes(int pos1, int pos2) {
	Node &node1 = _nodes[pos1];
	Node &node2 = _nodes[pos2];

	bool counted = false;
	if (node1._number == pos1) { // поставили на свое место
		assert(!node1._isBack);
		counted = true;
		_runtime->event(EVENT_PUT_RIGHT, node1.obj()->screen_R());
	}

	if (node2._number == pos1) { // сняли со своего места
		assert(node2._isBack);
		counted = true;
		_runtime->event(EVENT_GET_RIGHT, node1.obj()->screen_R());
	}

	if (node2._number == pos2) { // поставили на свое место
		assert(!node2._isBack);
		counted = true;
		_runtime->event(EVENT_PUT_RIGHT, node2.obj()->screen_R());
	}

	if (node1._number == pos2) { // сняли со своего места
		assert(node1._isBack);
		counted = true;
		_runtime->event(EVENT_GET_RIGHT, node2.obj()->screen_R());
	}

	if (!counted) { // просто сделали ход
		mgVect2i pos = node1.obj()->screen_R();
		pos += node2.obj()->screen_R();
		pos /= 2;
		_runtime->event(EVENT_TURN, pos);
	}

	bool isWin = true;
	int position = 0;

	for (auto &it : _nodes) {
		if (it._number != position++) {
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
	if (_animationState == NO_ANIMATION)
		return false;

	_animationTimer -= dt;
	if (_animationTimer > 0)
		return true;

	Node &node1 = _nodes[_animatedNodes[0]];
	Node &node2 = _nodes[_animatedNodes[1]];

	switch (_animationState) {
	case FIRST_PHASE: {
		node1._rotation = getRotate(_animatedNodes[0], _animatedNodes[1]);
		node2._rotation = getRotate(_animatedNodes[1], _animatedNodes[0]);

		node1._isBack = !node1._isBack;
		node2._isBack = !node2._isBack;

		releaseNodeBack(node1);
		releaseNodeBack(node2);

		for (auto &it : node1._face)
			it.setState(Node::getFaceStateName(0, false, false, false));

		for (auto &it : node2._face)
			it.setState(Node::getFaceStateName(0, false, false, false));

		updateNode(node1, _animatedNodes[1], destination(_animatedNodes[0], _animatedNodes[1]), true);
		updateNode(node2, _animatedNodes[0], destination(_animatedNodes[1], _animatedNodes[0]), true);

		_animationTimer = 0.f;
		_animationState = SECOND_PHASE;

		debugC(5, kDebugMinigames, ">>>>>>>>>>>>>>>>>>>>>>>>>>> change %d <> %d, 2nd phase 1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<", _animatedNodes[0], _animatedNodes[1]);
		node1.debugInfo();
		node2.debugInfo();

		return true;
	}
	case SECOND_PHASE:
		node1._animated = false;
		node2._animated = false;

		updateNode(node1, _animatedNodes[1], destination(_animatedNodes[0], _animatedNodes[1]));
		updateNode(node2, _animatedNodes[0], destination(_animatedNodes[1], _animatedNodes[0]));

		SWAP(node1, node2);

		_animationTimer = _animationTime;
		_animationState = FIRD_PHASE;

		debugC(5, kDebugMinigames, ">>>>>>>>>>>>>>>>>>>>>>>>>>> change %d <> %d, 2nd phase 2 <<<<<<<<<<<<<<<<<<<<<<<<<<<<", _animatedNodes[0], _animatedNodes[1]);
		node2.debugInfo();
		node1.debugInfo();

		return true;

	case FIRD_PHASE:
		_animationTimer = 0.f;
		_animationState = NO_ANIMATION;

		releaseNodeBack(node1);
		releaseNodeBack(node2);

		updateNode(node1, _animatedNodes[0]);
		updateNode(node2, _animatedNodes[1]);

		endSwapNodes(_animatedNodes[0], _animatedNodes[1]);
		debugC(5, kDebugMinigames, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ change %d <> %d, finished ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^", _animatedNodes[0], _animatedNodes[1]);

		_animatedNodes[0] = -1;
		_animatedNodes[1] = -1;

		return true;

	default:
		break;
	}

	return false;
}

void MinigameTriangle::swapNodes(int pos1, int pos2, bool silentQuick) {
	if (silentQuick) {
		Node &node1 = _nodes[pos1];
		Node &node2 = _nodes[pos2];

		node1._rotation = getRotate(pos1, pos2);
		node2._rotation = getRotate(pos2, pos1);

		node1._isBack = !node1._isBack;
		node2._isBack = !node2._isBack;

		releaseNodeBack(node1);
		releaseNodeBack(node2);

		SWAP(node1, node2);

		updateNode(node1, pos1, 0, true);
		updateNode(node2, pos2, 0, true);
	} else
		beginSwapNodes(pos1, pos2);
}

void MinigameTriangle::quant(float dt) {
	if (_selected >= 0)
		_runtime->setGameHelpVariant(0);
	else
		_runtime->setGameHelpVariant(1);

	if (animate(dt))
		return;

	int mousePos = -1;
	for (int idx = 0; idx < fieldSize_; ++idx)
		if (_nodes[idx].hit(_runtime->mousePosition())) {
			mousePos = idx;
			break;
		}

	int startAnimation = -1;
	int lastSelected = _selected;

	if (_runtime->mouseLeftPressed()) {
		if (mousePos < 0)                       // кликнули мимо - снимаем выделение
			_selected = -1;
		else if (_selected < 0)                 // ничего выделено небыло, просто выделяем
			_selected = mousePos;
		else if (_selected == mousePos)         // кликнули на выделенном - снимаем выделение
			_selected = -1;
		else if (compatible(_selected, mousePos)) { // поменять фишки местами
			startAnimation = _selected;
			_selected = -1;
		} else
			_selected = -1;
	}

	if (_selected != lastSelected) {
		for (int idx = 0; idx < fieldSize_; ++idx) {
			Node &node = _nodes[idx];
			if (idx == _selected || compatible(_selected, idx)) { // с этой фишкой можно поменяться
				if (!node._border)
					node._border = _selectBorders[orientation(idx)].getObject();
				node._border.setState(Node::getBorderStateName(idx == _selected));
				node._border->set_R(slotCoord(idx));
				node._border->update_screen_R();
				_runtime->setDepth(node._border, _selectDepth);
			} else if (node._border) {
				_selectBorders[0].releaseObject(node._border, _runtime);
				_selectBorders[1].releaseObject(node._border, _runtime);
			}
		}
	}

	if (_hovered != mousePos || _selected != lastSelected) {
		highlight(_hovered, false);
		highlight(_selected >= 0 ? _selected : lastSelected, false);

		_hovered = mousePos;

		if (_hovered >= 0 && startAnimation < 0) {
			if (_selected >= 0) {
				if (compatible(_selected, _hovered)) {
					highlight(_hovered, true);
					highlight(_selected, true);
				}
			} else
				highlight(_hovered, true);
		}
	}

	if (startAnimation >= 0) {
		_hovered = -1;
		swapNodes(startAnimation, mousePos, false);
	}

	if (_runtime->mouseRightPressed() && mousePos >= 0) {
		debugC(2, kDebugMinigames, "----- DUBUG INFO FOR %d POSITION --------------------", mousePos);
		debugC(2, kDebugMinigames, "row = %d, begin = %d, orientation = %d", rowByNum(mousePos), rowBegin(rowByNum(mousePos)), orientation(mousePos));
		_nodes[mousePos].debugInfo();
	}
}

int MinigameTriangle::rowBegin(int row) const {
	if (row == _fieldLines)
		return fieldSize_;

	switch (_gameType) {
	case TRIANGLE:
		return sqr(row);
	case RECTANGLE:
		return row * _fieldWidth;
	default:
		break;
	}
	//case HEXAGON:
	assert(row >= 0 && row < _fieldLines);
	if (row >= _fieldLines / 2) {
		row -= _fieldLines / 2;
		return fieldSize_ / 2 + (2 * _fieldLines - row) * row;
	}
	return (_fieldLines + row) * row;

}

int MinigameTriangle::rowByNum(int num) const {
	if (num >= fieldSize_)
		return _fieldLines;

	switch (_gameType) {
	case TRIANGLE:
		return floor(sqrt((float)num));
	case RECTANGLE:
		return num / _fieldWidth;
	default:
		break;
	}
	//case HEXAGON:
	int row = num < fieldSize_ / 2 ? 0 : _fieldLines / 2;
	while (row < _fieldLines && num >= rowBegin(row))
		++row;
	return row > 0 ? row - 1 : 0;
}

int MinigameTriangle::orientation(int num) const {
	switch (_gameType) {
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

	if (row2 >= _fieldLines)
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
	       [orientation(num1)][_nodes[num1]._rotation];
}

int MinigameTriangle::destination(int num1, int num2) const {
	if (orientation(num1) == 0)
		return rowByNum(num1) != rowByNum(num2) ? 0 : (num2 < num1 ? 1 : 2);
	else
		return rowByNum(num1) != rowByNum(num2) ? 0 : (num2 < num1 ? 2 : 1);
}

mgVect3f MinigameTriangle::slotCoord(int pos, int angle) const {
	assert(pos * 3 + angle < (int)_positions.size());
	return _positions[pos * 3 + angle];
}

} // namespace QDEngine
