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
#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/m_puzzle.h"
#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/Rect.h"
#include "qdengine/minigames/adv/qdMath.h"
#include "qdengine/system/input/keyboard_input.h"

namespace QDEngine {

typedef Rect<float, mgVect2f> Rectf;

MinigameInterface *createMinigamePuzzle(MinigameManager *runtime) {
	return new Puzzle(runtime);
}

enum {
	EVENT_GET,
	EVENT_PUT,
	EVENT_SWAP,
	EVENT_ROTATE_IN_FIELD,
	EVENT_RETURN,
	EVENT_PUT_RIGHT,
	EVENT_CLICK_RIGHT,
	EVENT_CLICK,
	EVENT_ROTATE_IN_STACK,
	EVENT_FIELD_ROTATE
};

const char *Puzzle::getStateName(int angle, bool selected, bool small) const {
	static const char *small_pref = "inv_";
	static const char *selected_suf = "_sel";

	static char buf[32];
	buf[31] = 0;

	assert(angle >= 0 && angle < _angles);
	angle = (angle + _globalAngle) % _angles;

	snprintf(buf, 31, "%s%02d%s", !_singleSize && small ? small_pref : "", angle + 1, selected ? selected_suf : "");
	return buf;
}

Puzzle::Puzzle(MinigameManager *runtime) {
	_runtime = runtime;

	if (!_runtime->getParameter("game_size", _gameSize, true))
		return;
	assert(_gameSize > 0 && _gameSize < 100);

	_field.resize(_gameSize, -1);
	_globalAngle = 0;

	_singleSize = _runtime->getParameter("small_objects", false);

	_angles = _runtime->getParameter("angles", 4);
	assert(_angles > 0 &&  _angles < 10);

	if (!(_stackBottom = _runtime->getObject(_runtime->parameter("inventory_bottom"))))
		return;
	if (!_runtime->getParameter("inventory_size", _stackSize, true))
		return;

	if (_runtime->getParameter("rotate_period", _rotateTimePeriod, false)) {
		assert(sqr(sqrt((float)_gameSize)) == _gameSize);
		if (sqr(sqrt((float)_gameSize)) != _gameSize)
			return;
	} else
		_rotateTimePeriod = 86400; // сутки
	_nextRotateTime = _runtime->getTime() + _rotateTimePeriod;

	_flySpeed = _runtime->getParameter("inventory_drop_speed", 240.f);
	assert(_flySpeed > 0.f);
	_returnSpeed = _runtime->getParameter("inventory_return_speed", -1.f);

	const char *name_begin = _runtime->parameter("obj_name_begin", "obj_");

	char buf[128];
	buf[127] = 0;

	Common::MemoryReadWriteStream gameData(DisposeAfterUse::YES);
	for (int idx = 0; idx < _gameSize; ++idx) {
		snprintf(buf, 127, "%s%02d", name_begin, idx + 1);

		Node node;
		node.obj = _runtime->getObject(buf);

		if (_runtime->debugMode()) {
			node.pos = _nodes.size();
			node.angle = 0;
			_field[node.pos] = node.pos;
		} else
			node.angle = _runtime->rnd(0, _angles - 1);
		node.obj.setState(getStateName(node.angle, false, true));

		node.obj->R().write(gameData);

		_nodes.push_back(node);
	}

	if (!_runtime->processGameData(gameData))
		return;

	GameInfo *gameInfo = _runtime->getCurrentGameInfo();
	if (gameInfo) {
		Common::MemoryReadStream data((byte *)gameInfo->_gameData, gameInfo->_dataSize);
		for (int idx = 0; idx < _gameSize; ++idx) {
			mgVect3f crd;
			crd.read(data);
			_nodes[idx].obj->set_R(crd);
			_positions.push_back(crd);
		}
	} else {
		for (int idx = 0; idx < _gameSize; ++idx) {
			mgVect3f crd;
			crd.read(gameData);
			_nodes[idx].obj->set_R(crd);
			_positions.push_back(crd);
		}
	}

	if (_runtime->debugMode())
		_nodes[0].angle = _angles - 1;

	_size = _runtime->getSize(_nodes[0].obj);
	debugC(2, kDebugMinigames, "size = (%6.2f,%6.2f)", _size.x, _size.y);

	_depth = _nodes[0].obj.depth(runtime);

	_stackPlaceSize = _runtime->getParameter("inventory_place_size", _size * 1.2f);
	assert(_stackPlaceSize.x > 0.f && _stackPlaceSize.x < 500.f && _stackPlaceSize.y > 0.f && _stackPlaceSize.y < 500.f);
	debugC(2, kDebugMinigames, "stackPlaceSize = (%5.1f, %5.1f)", _stackPlaceSize.x, _stackPlaceSize.y);

	_prevPlace = -1;
	_pickedItem = -1;
	_mouseObjPose = stidx(_stackSize + 1);

	_inField = _runtime->debugMode() ? _nodes.size() : 0;
	_nextObjTime = _runtime->getTime();

	setState(MinigameInterface::RUNNING);
}

Puzzle::~Puzzle() {
	for (auto &it : _nodes)
		_runtime->release(it.obj);

	_runtime->release(_stackBottom);
}

void Puzzle::rotate(int item) {
	assert(item >= 0 && item < (int)_nodes.size());
	_nodes[item].angle = (_nodes[item].angle + 1) % _angles;
}

int Puzzle::stidx(int idx) const {
	return -idx - 2;
}

bool Puzzle::testPlace(int item) const {
	assert(item >= 0 && item < (int)_nodes.size());
	return _nodes[item].pos == item && _nodes[item].angle == 0;
}

bool Puzzle::isFlying(int idx) const {
	for (auto &it : _flyObjs)
		if (it.data == idx)
			return true;
	return false;
}

bool Puzzle::isOnMouse(const Node& node) const {
	if (node.pos == _mouseObjPose) {
		return true;
	}
	return false;
}

void Puzzle::put(int where, int what, float flowSpeed) {
	assert(where < (int)_field.size());
	assert(what >= 0 && what < (int)_nodes.size());

	Node& node = _nodes[what];
	int start = node.pos;

	if (flowSpeed > 0.f || isFlying(what)) {
		FlyQDObject* flyObj = 0;

		FlyQDObjects::iterator fit;
		for (fit = _flyObjs.begin(); fit != _flyObjs.end(); fit++) {
			if (fit->data == what)
				break;
		}
		if (fit != _flyObjs.end()) // Этот фрагмент уже летит, просто поменять точку назначения
			flyObj = fit;
		else { // Добавляем новый летящий фрагмент
			_flyObjs.push_back(FlyQDObject());
			flyObj = &_flyObjs.back();

			flyObj->data = what;

			mgVect3f from = isOnMouse(node) ? node.obj->R() : start < -1 ? stackPosition(stidx(start)) : position(start);
			flyObj->current = _runtime->world2game(from);
			node.obj->set_R(from);

			flyObj->speed = flowSpeed;
		}

		mgVect3f to = where < -1 ? stackPosition(stidx(where)) : position(where);
		flyObj->target = _runtime->world2game(to);
		flyObj->depth = _runtime->getDepth(to);
	}

	if (where >= 0)
		_field[where] = what;

	node.pos = where;
}

void Puzzle::putOnStack(int what, float speed) {
	put(stidx((int)_stack.size()), what, speed);
	_stack.push_back(what);
}

void Puzzle::returnToStack() {
	assert(_pickedItem != -1);
	_runtime->event(EVENT_RETURN, _runtime->mousePosition());
	if (_prevPlace >= 0)
		put(_prevPlace, _pickedItem);
	else
		putOnStack(_pickedItem, _returnSpeed);
	_prevPlace = -1;
	_pickedItem = -1;
	_runtime->event(EVENT_CLICK, _runtime->mousePosition());
}

void Puzzle::quant(float dt) {
	if (_pickedItem == -1)
		_runtime->setGameHelpVariant(0);
	else
		_runtime->setGameHelpVariant(1);

	if (_runtime->getTime() > _nextRotateTime) {
		_runtime->event(EVENT_FIELD_ROTATE, mgVect2f(400, 300));
		_nextRotateTime = _runtime->getTime() + _rotateTimePeriod;
		_globalAngle = (_globalAngle + 1) % _angles;
		_runtime->setCompleteHelpVariant(_globalAngle);
	}

	FlyQDObjects::iterator fit = _flyObjs.begin();
	while (fit != _flyObjs.end())
		if (!isOnMouse(_nodes[fit->data]) && fit->quant(dt, _nodes[fit->data].obj, _runtime))
			++fit;
		else
			fit = _flyObjs.erase(fit);

	if (_inField < (int)_nodes.size() && _runtime->getTime() > _nextObjTime &&
			((int)_stack.size() < _stackSize - 1 || ((int)_stack.size() < _stackSize && _pickedItem == -1))) { // нужно добавить в инвентори фишку
		// ищем случайный не выставленный фрагмент
		int freeIdx = round(_runtime->rnd(0.f, _nodes.size() - 1));
		Nodes::iterator it = _nodes.begin();
		for (;;) {
			if (++it == _nodes.end())
				it = _nodes.begin();
			if (it->isFree())
				if (!freeIdx--)
					break;
		}
		int idx = Common::distance(_nodes.begin(), it);

		++_inField;
		_nextObjTime = _runtime->getTime() + _stackPlaceSize.y / _flySpeed;

		it->pos = stidx(_stackSize);
		it->obj.setState(getStateName(it->angle, false, true));

		putOnStack(idx, _flySpeed);
	}

	mgVect2f mouse = _runtime->mousePosition();

	int hovPlace = -1;  // Номер места которое сейчас под мышкой
	for (int idx = 0; idx < (int)_stack.size(); ++idx)
		if (_nodes[_stack[idx]].obj.hit(mouse)) {
			hovPlace = stidx(idx);
			break;
		}
	if (hovPlace == -1) {
		float radius = 0.5f * _size.x;
		for (int idx = 0; idx < _gameSize; ++idx)
			if (dist(_runtime->world2game(position(idx)), mouse) < radius) {
				hovPlace = idx;
				break;
			}
	}
	if (hovPlace == -1) {
		mgVect2i st = _stackBottom->screen_R();
		st.y -= _stackPlaceSize.y * _stackSize - 0.5f * _stackPlaceSize.x;
		Rectf stackPos(st.x - 0.5f * _stackPlaceSize.x, st.y, _stackPlaceSize.x, _stackPlaceSize.y * _stackSize);
		if (stackPos.point_inside(mouse))
			hovPlace = stidx(_stackSize);
	}

	if (_runtime->mouseLeftPressed()) {
		if (hovPlace >= 0) { // клик по полю
			Indexes::value_type& hovItem = _field[hovPlace];
			if (hovItem == -1) // клик по пустой ячейке
				if (_pickedItem == -1) // на мыши ничего нет
					_runtime->event(EVENT_CLICK, mouse);
				else { // кладем фрагмент с мыши
					put(hovPlace, _pickedItem);
					if (testPlace(_pickedItem)) // положили на свое свое место
						_runtime->event(EVENT_PUT_RIGHT, mouse);
					else // просто положили
						_runtime->event(EVENT_PUT, mouse);
					_pickedItem = -1;
					_prevPlace = -1;
				} else { // клик по непустой ячейке
				if (testPlace(hovPlace)) // клик по правильно уложенной фишке
					_runtime->event(EVENT_CLICK_RIGHT, mouse);
				else if (_pickedItem != -1) { // поменять с тем что на мыше
					bool swap = true;
					if (_prevPlace >= 0)
						put(_prevPlace, hovItem);
					else
						putOnStack(hovItem, _returnSpeed);
					if (testPlace(hovItem)) { // оказалась при обмене на своем месте
						_runtime->event(EVENT_PUT_RIGHT, _runtime->world2game(position(_prevPlace)));
						swap = false;
					}
					put(hovPlace, _pickedItem);
					if (testPlace(_pickedItem)) { // положили на свое свое место
						_runtime->event(EVENT_PUT_RIGHT, mouse);
						swap = false;
					}
					if (swap) // просто обменяли
						_runtime->event(EVENT_SWAP, mouse);
					_pickedItem = -1;
					_prevPlace = -1;
				} else { // взять фрагмент на мышь
					_runtime->event(EVENT_GET, mouse);
					_prevPlace = hovPlace;
					_pickedItem = hovItem;
					_nodes[_pickedItem].pos = _mouseObjPose;
					hovItem = -1;
				}
			}
		} else if (hovPlace < -1) { // клик по стеку
			int hovStack = stidx(hovPlace);
			if (_pickedItem == -1) // на мыши ничего нет
				if (hovStack < (int)_stack.size()) { // взять фрагмент из стека на мышь
					_runtime->event(EVENT_GET, mouse);
					Indexes::iterator it = _stack.begin() + hovStack;
					assert(*it >= 0);
					_prevPlace = -1;
					_pickedItem = *it;
					_nodes[_pickedItem].pos = _mouseObjPose;
					_stack.erase(it);
					for (int idx = hovStack; idx < (int)_stack.size(); ++idx)
						put(stidx(idx), _stack[idx], _flySpeed);
				} else // пустой клик в области стека
					_runtime->event(EVENT_CLICK, mouse);
			else // вернуть фишку на место
				returnToStack();
		} else // пустой клик мимо игрового поля
			_runtime->event(EVENT_CLICK, mouse);
	} else if (_runtime->mouseRightPressed()) {
		if (_pickedItem == -1) {
			if (hovPlace >= 0) { // клик по полю
				if (testPlace(hovPlace)) // клик по правильно уложенной фишке
					_runtime->event(EVENT_CLICK_RIGHT, mouse);
				else {
					Indexes::value_type& hovItem = _field[hovPlace];
					if (hovItem >= 0) {
						rotate(hovItem);
						if (testPlace(hovItem)) // повернули на правильный угол
							_runtime->event(EVENT_PUT_RIGHT, mouse);
						else // просто положили
							_runtime->event(EVENT_ROTATE_IN_FIELD, mouse);
					} else // попытка прокрутить пустое место
						_runtime->event(EVENT_CLICK, mouse);
				}
			} else  if (hovPlace < -1) { // клик по стеку
				int hovStack = stidx(hovPlace);
				if (hovStack < (int)_stack.size()) { // покрутить внутри стека
					_runtime->event(EVENT_ROTATE_IN_STACK, mouse);
					rotate(_stack[hovStack]);
				} else // попытка прокрутить пустое место
					_runtime->event(EVENT_CLICK, mouse);
			} else // пустой клик мимо игрового поля
				_runtime->event(EVENT_CLICK, mouse);
		} else // вернуть фишку на место
			returnToStack();
	}

	bool iWin = true;
	for (int idx = 0; idx < (int)_nodes.size(); ++idx) {
		Node& node = _nodes[idx];
		if (node.pos != -1) {
			if (node.pos >= 0) {
				if (isFlying(idx))
					node.obj.setState(getStateName(node.angle, false, false));
				else {
					node.obj.setState(getStateName(node.angle, node.pos == hovPlace && !testPlace(idx), false));
					node.obj->set_R(position(node.pos));
				}
			} else if (idx == _pickedItem) {
				node.obj.setState(getStateName(node.angle, hovPlace >= 0 && !testPlace(hovPlace), false));
				node.obj->set_R(_runtime->game2world(mouse, _stackBottom.depth(_runtime) - 200));
			} else {
				node.obj.setState(getStateName(node.angle, node.pos == hovPlace && _pickedItem == -1, true));
				if (!isFlying(idx))
					node.obj->set_R(stackPosition(stidx(node.pos)));
			}
			iWin = iWin && testPlace(idx);
		} else {
			_runtime->hide(node.obj);
			iWin = false;
		}
	}

	if (iWin)
		setState(GAME_WIN);

}

const mgVect3f &Puzzle::position(int num) const {
	assert(num >= 0 && num < (int)_positions.size());
	// Если глобальный поворот ненулевой, пересчитываем индекс
	if (_globalAngle > 0) {
		int size = sqrt((float)_gameSize);
		int y = num / size;
		int x = num - y * size;
		--size;
		for (int angle = 0; angle < _globalAngle; ++angle) {
			int tmp = x;
			x = size - y;
			y = tmp;
		}
		num = y * (size + 1) + x;
	}
	assert(num >= 0 && num < (int)_positions.size());
	return _positions[num];
}

mgVect3f Puzzle::stackPosition(int num) const {
	mgVect3f bottom = _runtime->world2game(_stackBottom);
	bottom.y -= _stackPlaceSize.y * num;
	return _runtime->game2world(bottom);
}

} // namespace QDEngine
