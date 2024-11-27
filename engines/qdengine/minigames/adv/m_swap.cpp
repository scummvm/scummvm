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

MinigameInterface *createMinigameSwap() {
	return new Swap(g_runtime);
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

	assert(angle >= 0 && angle < angles_);

	snprintf(buf, 31, "%02d%s", angle + 1, selected ? selected_suf : "");
	return buf;
}

Swap::Swap(MinigameManager *runtime) {
	_runtime = runtime;

	if (!getParameter("game_size", gameSize_, true) || gameSize_ < 2)
		return;

	if ((angles_ = getParameter("angles", 4)) < 1)
		return;

	if ((rotateTimePeriod_ = getParameter("rotate_period", 86400.f)) < 10.f)
		return;
	nextRotateTime_ = _runtime->getTime() + rotateTimePeriod_;

	/*const char *name_begin = */_runtime->parameter("obj_name_begin", "obj_");

	warning("STUB: Swap::Swap()");
#if 0
	XBuffer gameData;

	for (int idx = 0; idx < gameSize_; ++idx) {
		Common::String buf = Common::String::format("%s%02d", name_begin, idx + 1);

		Node node(idx);
		node.obj = _runtime->getObject(buf.c_str());
		node.angle = 0;
		node.obj.setState(getStateName(node.angle, false));
		nodes_.push_back(node);

		gameData.write(node.obj->R());
	}

	if (!_runtime->processGameData(gameData))
		return;

	positions_.resize(gameSize_);
	for (int idx = 0; idx < gameSize_; ++idx)
		gameData.read(positions_[idx]);
#endif

	size_ = getParameter("element_size", _runtime->getSize(nodes_[0].obj));
	assert(size_.x > 0.f && size_.y > 0.f && size_.x < 500.f && size_.y < 500.f);
	debugC(2, kDebugMinigames, "element_size = (%6.2f,%6.2f)", size_.x, size_.y);

	pickedItem_ = -1;
	last1_ = last2_ = -1;

	if (_runtime->debugMode()) {
		last1_ = 0;
		last2_ = 1;
		rotate(last1_, last2_, false);
	} else
		for (int cnt = 0; cnt < 50; ++cnt) {
			rotate(_runtime->rnd(0, gameSize_ - 1), _runtime->rnd(0, gameSize_ - 1), true, true);
			swap(_runtime->rnd(0, gameSize_ - 1), _runtime->rnd(0, gameSize_ - 1), true);
		}


	setState(MinigameInterface::RUNNING);

}

Swap::~Swap() {
	for (auto &it : nodes_)
		_runtime->release(it.obj);

}

void Swap::quant(float dt) {
	if (pickedItem_ >= 0)
		_runtime->setGameHelpVariant(1);
	else if (last1_ >= 0)
		_runtime->setGameHelpVariant(2);
	else
		_runtime->setGameHelpVariant(0);

	if (_runtime->getTime() > nextRotateTime_) {
		int item1 = _runtime->rnd(0, gameSize_ - 1);
		int item2 = _runtime->rnd(0, gameSize_ - 1);
		if (item1 != last1_ && item1 != last2_ && item1 != pickedItem_ && item2 != last1_ && item2 != last2_ && item2 != pickedItem_) {
			nextRotateTime_ = _runtime->getTime() + rotateTimePeriod_;
			rotate(item1, item2, false, true);
			_runtime->event(EVENT_AUTO_ROTATE, mgVect2f(400, 300));
			return;
		}
	}

	mgVect2f mouse = _runtime->mousePosition();

	int hovPlace = -1;  // Номер места которое сейчас под мышкой
	if (pickedItem_ == -1) {
		for (auto &it : nodes_)
			if (it.obj.hit(mouse)) {
				hovPlace = Common::distance(nodes_.begin(), &it);
				break;
			}
	}
	if (hovPlace == -1)
		for (int idx = 0; idx < gameSize_; ++idx) {
			Rectf rect(size_ * 0.9f);
			rect.center(_runtime->world2game(position(idx)));
			if (rect.point_inside(mouse)) {
				hovPlace = idx;
				break;
			}
		}

	if (_runtime->mouseLeftPressed()) {
		if (hovPlace >= 0) { // клик по полю
			if (pickedItem_ == -1) { // мышь пустая, берем
				deactivate();
				_runtime->event(EVENT_GET, mouse);
				pickedItem_ = hovPlace;
			} else if (pickedItem_ == hovPlace) { // вернуть на место
				_runtime->event(EVENT_RETURN, mouse);
				put(pickedItem_, false);
				pickedItem_ = -1;
			} else { // поменять местами
				last1_ = pickedItem_;
				last2_ = hovPlace;
				swap(last1_, last2_, false);
				pickedItem_ = -1;
			}
		} else { // пустой клик мимо игрового поля
			deactivate();
			_runtime->event(EVENT_CLICK, mouse);
		}
	} else if (_runtime->mouseRightPressed()) {
		if (pickedItem_ >= 0) // если на мыши фрагмент ничего не делаем
			_runtime->event(EVENT_CLICK, mouse);
		else if (hovPlace == last1_ || hovPlace == last2_) // клик по выделенным
			rotate(last1_, last2_, false);
		else // пустой клик мимо активного места
			_runtime->event(EVENT_CLICK, mouse);
	}

	if (pickedItem_ >= 0)
		nodes_[pickedItem_].obj->set_R(_runtime->game2world(mouse, -5000));

	int idx = 0;
	for (; idx < gameSize_; ++idx)
		if (!testPlace(idx))
			break;

	if (idx == (int)nodes_.size()) {
		deactivate();
		setState(MinigameInterface::GAME_WIN);
	}
}

const mgVect3f &Swap::position(int num) const {
	assert(num >= 0 && num < (int)positions_.size());
	return positions_[num];
}

void Swap::put(int item, bool hl) {
	assert(item >= 0 && item < (int)nodes_.size());
	nodes_[item].obj->set_R(position(item));
	nodes_[item].obj.setState(getStateName(nodes_[item].angle, hl));

}

void Swap::deactivate() {
	if (last1_ >= 0) {
		assert(last2_ >= 0);
		put(last1_, false);
		put(last2_, false);
	}
	last1_ = -1;
	last2_ = -1;
}

bool Swap::testPlace(int item) const {
	assert(item >= 0 && item < (int)nodes_.size());
	return nodes_[item].home == item && nodes_[item].angle == 0;
}

void Swap::swap(int item1, int item2, bool silent) {
	assert(item1 >= 0 && item1 < (int)nodes_.size());
	assert(item2 >= 0 && item2 < (int)nodes_.size());

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

	SWAP(nodes_[item1], nodes_[item2]);
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
	assert(item1 >= 0 && item1 < (int)nodes_.size());
	assert(item2 >= 0 && item2 < (int)nodes_.size());

	if (!silent) {
		if (testPlace(item1)) // сняли со своего места
			_runtime->event(EVENT_GET_RIGHT, _runtime->world2game(position(item1)));
		if (testPlace(item2)) // сняли со своего места
			_runtime->event(EVENT_GET_RIGHT, _runtime->world2game(position(item2)));
	}

	nodes_[item1].angle = (nodes_[item1].angle + 1) % angles_;
	nodes_[item2].angle = (nodes_[item2].angle + 1) % angles_;
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
