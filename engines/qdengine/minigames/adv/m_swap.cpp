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

#include "qdengine/qdengine.h"
#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/m_swap.h"
#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/Rect.h"

namespace QDEngine {

typedef Rect<float, mgVect2f> Rectf;

MinigameInterface *createMinigameSwap(MinigameManager *runtime) {
	return new Swap(runtime);
}

enum {
	EVENT_GET,
	EVENT_SWAP,
	EVENT_ROTATE,
	EVENT_RETURN,
	EVENT_PUT_RIGHT,
	EVENT_GET_RIGHT,
	EVENT_CLICK,
	EVENT_AUTO_ROTATE
};

const char *Swap::getStateName(int angle, bool selected) const {
	static const char *selected_suf = "_sel";

	static char buf[32];
	buf[31] = 0;

	assert(angle >= 0 && angle < _angles);

	snprintf(buf, 31, "%02d%s", angle + 1, selected ? selected_suf : "");
	return buf;
}

Swap::Swap(MinigameManager *runtime) {
	_runtime = runtime;

	if (!_runtime->getParameter("game_size", _gameSize, true) || _gameSize < 2)
		return;

	if ((_angles = _runtime->getParameter("angles", 4)) < 1)
		return;

	if ((_rotateTimePeriod = _runtime->getParameter("rotate_period", 86400.f)) < 10.f)
		return;
	_nextRotateTime = _runtime->getTime() + _rotateTimePeriod;

	/*const char *name_begin = */_runtime->parameter("obj_name_begin", "obj_");

	warning("STUB: Swap::Swap()");
#if 0
	XBuffer gameData;

	for (int idx = 0; idx < _gameSize; ++idx) {
		Common::String buf = Common::String::format("%s%02d", name_begin, idx + 1);

		Node node(idx);
		node.obj = _runtime->getObject(buf.c_str());
		node.angle = 0;
		node.obj.setState(getStateName(node.angle, false));
		_nodes.push_back(node);

		gameData.write(node.obj->R());
	}

	if (!_runtime->processGameData(gameData))
		return;

	_positions.resize(_gameSize);
	for (int idx = 0; idx < _gameSize; ++idx)
		gameData.read(_positions[idx]);
#endif

	_size = _runtime->getParameter("element_size", _runtime->getSize(_nodes[0].obj));
	assert(_size.x > 0.f && _size.y > 0.f && _size.x < 500.f && _size.y < 500.f);
	debugC(2, kDebugMinigames, "element_size = (%6.2f,%6.2f)", _size.x, _size.y);

	_pickedItem = -1;
	_last1 = _last2 = -1;

	if (_runtime->debugMode()) {
		_last1 = 0;
		_last2 = 1;
		rotate(_last1, _last2, false);
	} else
		for (int cnt = 0; cnt < 50; ++cnt) {
			rotate(_runtime->rnd(0, _gameSize - 1), _runtime->rnd(0, _gameSize - 1), true, true);
			swap(_runtime->rnd(0, _gameSize - 1), _runtime->rnd(0, _gameSize - 1), true);
		}


	setState(MinigameInterface::RUNNING);

}

Swap::~Swap() {
	for (auto &it : _nodes)
		_runtime->release(it.obj);

}

void Swap::quant(float dt) {
	if (_pickedItem >= 0)
		_runtime->setGameHelpVariant(1);
	else if (_last1 >= 0)
		_runtime->setGameHelpVariant(2);
	else
		_runtime->setGameHelpVariant(0);

	if (_runtime->getTime() > _nextRotateTime) {
		int item1 = _runtime->rnd(0, _gameSize - 1);
		int item2 = _runtime->rnd(0, _gameSize - 1);
		if (item1 != _last1 && item1 != _last2 && item1 != _pickedItem && item2 != _last1 && item2 != _last2 && item2 != _pickedItem) {
			_nextRotateTime = _runtime->getTime() + _rotateTimePeriod;
			rotate(item1, item2, false, true);
			_runtime->event(EVENT_AUTO_ROTATE, mgVect2f(400, 300));
			return;
		}
	}

	mgVect2f mouse = _runtime->mousePosition();

	int hovPlace = -1;  // Номер места которое сейчас под мышкой
	if (_pickedItem == -1) {
		for (auto &it : _nodes)
			if (it.obj.hit(mouse)) {
				hovPlace = Common::distance(_nodes.begin(), &it);
				break;
			}
	}
	if (hovPlace == -1)
		for (int idx = 0; idx < _gameSize; ++idx) {
			Rectf rect(_size * 0.9f);
			rect.center(_runtime->world2game(position(idx)));
			if (rect.point_inside(mouse)) {
				hovPlace = idx;
				break;
			}
		}

	if (_runtime->mouseLeftPressed()) {
		if (hovPlace >= 0) { // клик по полю
			if (_pickedItem == -1) { // мышь пустая, берем
				deactivate();
				_runtime->event(EVENT_GET, mouse);
				_pickedItem = hovPlace;
			} else if (_pickedItem == hovPlace) { // вернуть на место
				_runtime->event(EVENT_RETURN, mouse);
				put(_pickedItem, false);
				_pickedItem = -1;
			} else { // поменять местами
				_last1 = _pickedItem;
				_last2 = hovPlace;
				swap(_last1, _last2, false);
				_pickedItem = -1;
			}
		} else { // пустой клик мимо игрового поля
			deactivate();
			_runtime->event(EVENT_CLICK, mouse);
		}
	} else if (_runtime->mouseRightPressed()) {
		if (_pickedItem >= 0) // если на мыши фрагмент ничего не делаем
			_runtime->event(EVENT_CLICK, mouse);
		else if (hovPlace == _last1 || hovPlace == _last2) // клик по выделенным
			rotate(_last1, _last2, false);
		else // пустой клик мимо активного места
			_runtime->event(EVENT_CLICK, mouse);
	}

	if (_pickedItem >= 0)
		_nodes[_pickedItem].obj->set_R(_runtime->game2world(mouse, -5000));

	int idx = 0;
	for (; idx < _gameSize; ++idx)
		if (!testPlace(idx))
			break;

	if (idx == (int)_nodes.size()) {
		deactivate();
		setState(MinigameInterface::GAME_WIN);
	}
}

const mgVect3f &Swap::position(int num) const {
	assert(num >= 0 && num < (int)_positions.size());
	return _positions[num];
}

void Swap::put(int item, bool hl) {
	assert(item >= 0 && item < (int)_nodes.size());
	_nodes[item].obj->set_R(position(item));
	_nodes[item].obj.setState(getStateName(_nodes[item].angle, hl));

}

void Swap::deactivate() {
	if (_last1 >= 0) {
		assert(_last2 >= 0);
		put(_last1, false);
		put(_last2, false);
	}
	_last1 = -1;
	_last2 = -1;
}

bool Swap::testPlace(int item) const {
	assert(item >= 0 && item < (int)_nodes.size());
	return _nodes[item].home == item && _nodes[item].angle == 0;
}

void Swap::swap(int item1, int item2, bool silent) {
	assert(item1 >= 0 && item1 < (int)_nodes.size());
	assert(item2 >= 0 && item2 < (int)_nodes.size());

	bool res = false;
	if (!silent) {
		if (testPlace(item1)) { // сняли со своего места
			_runtime->event(EVENT_GET_RIGHT, _runtime->world2game(position(item1)));
			res = true;
		}
		if (testPlace(item2)) { // сняли со своего места
			_runtime->event(EVENT_GET_RIGHT, _runtime->world2game(position(item2)));
			res = true;
		}
	}

	SWAP(_nodes[item1], _nodes[item2]);
	put(item1, !silent);
	put(item2, !silent);

	if (!silent) {
		if (testPlace(item1)) { // оказалась при обмене на своем месте
			_runtime->event(EVENT_PUT_RIGHT, _runtime->world2game(position(item1)));
			res = true;
		}
		if (testPlace(item2)) { // положили на свое свое место
			_runtime->event(EVENT_PUT_RIGHT, _runtime->world2game(position(item2)));
			res = true;
		}
		if (!res) // просто обменяли
			_runtime->event(EVENT_SWAP, _runtime->mousePosition());
	}
}

void Swap::rotate(int item1, int item2, bool silent, bool avto) {
	assert(item1 >= 0 && item1 < (int)_nodes.size());
	assert(item2 >= 0 && item2 < (int)_nodes.size());

	if (!silent) {
		if (testPlace(item1)) // сняли со своего места
			_runtime->event(EVENT_GET_RIGHT, _runtime->world2game(position(item1)));
		if (testPlace(item2)) // сняли со своего места
			_runtime->event(EVENT_GET_RIGHT, _runtime->world2game(position(item2)));
	}

	_nodes[item1].angle = (_nodes[item1].angle + 1) % _angles;
	_nodes[item2].angle = (_nodes[item2].angle + 1) % _angles;
	put(item1, !avto);
	put(item2, !avto);

	if (!silent) {
		if (testPlace(item1)) // оказалась при обмене на своем месте
			_runtime->event(EVENT_PUT_RIGHT, _runtime->world2game(position(item1)));
		if (testPlace(item2)) // положили на свое свое место
			_runtime->event(EVENT_PUT_RIGHT, _runtime->world2game(position(item2)));
		_runtime->event(EVENT_ROTATE, _runtime->mousePosition());
	}
}

} // namespace QDEngine
