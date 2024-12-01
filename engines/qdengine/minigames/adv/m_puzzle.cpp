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

	assert(angle >= 0 && angle < angles_);
	angle = (angle + globalAngle_) % angles_;

	snprintf(buf, 31, "%s%02d%s", !singleSize_ && small ? small_pref : "", angle + 1, selected ? selected_suf : "");
	return buf;
}

Puzzle::Puzzle(MinigameManager *runtime) {
	_runtime = runtime;

	if (!_runtime->getParameter("game_size", gameSize_, true))
		return;
	assert(gameSize_ > 0 && gameSize_ < 100);

	field_.resize(gameSize_, -1);
	globalAngle_ = 0;

	singleSize_ = _runtime->getParameter("small_objects", false);

	angles_ = _runtime->getParameter("angles", 4);
	assert(angles_ > 0 &&  angles_ < 10);

	if (!(stackBottom_ = _runtime->getObject(_runtime->parameter("inventory_bottom"))))
		return;
	if (!_runtime->getParameter("inventory_size", stackSize_, true))
		return;

	if (_runtime->getParameter("rotate_period", rotateTimePeriod_, false)) {
		assert(sqr(sqrt((float)gameSize_)) == gameSize_);
		if (sqr(sqrt((float)gameSize_)) != gameSize_)
			return;
	} else
		rotateTimePeriod_ = 86400; // сутки
	nextRotateTime_ = _runtime->getTime() + rotateTimePeriod_;

	flySpeed_ = _runtime->getParameter("inventory_drop_speed", 240.f);
	assert(flySpeed_ > 0.f);
	returnSpeed_ = _runtime->getParameter("inventory_return_speed", -1.f);

	warning("STUB: Puzzle::Puzzle()");

#if 0
	const char *name_begin = _runtime->parameter("obj_name_begin", "obj_");

	char buf[128];
	buf[127] = 0;

	XBuffer gameData;
	for (int idx = 0; idx < gameSize_; ++idx) {
		snprintf(buf, 127, "%s%02d", name_begin, idx + 1);

		Node node;
		node.obj = _runtime->getObject(buf);

		if (_runtime->debugMode()) {
			node.pos = nodes_.size();
			node.angle = 0;
			field_[node.pos] = node.pos;
		} else
			node.angle = _runtime->rnd(0, angles_ - 1);
		node.obj.setState(getStateName(node.angle, false, true));

		gameData.write(node.obj->R());

		nodes_.push_back(node);
	}

	if (!_runtime->processGameData(gameData))
		return;

	for (int idx = 0; idx < gameSize_; ++idx) {
		mgVect3f crd;
		gameData.read(crd);
		nodes_[idx].obj->set_R(crd);
		positions_.push_back(crd);
	}
#endif
	if (_runtime->debugMode())
		nodes_[0].angle = angles_ - 1;

	size_ = _runtime->getSize(nodes_[0].obj);
	debugC(2, kDebugMinigames, "size = (%6.2f,%6.2f)", size_.x, size_.y);

	depth_ = nodes_[0].obj.depth(runtime);

	stackPlaceSize_ = _runtime->getParameter("inventory_place_size", size_ * 1.2f);
	assert(stackPlaceSize_.x > 0.f && stackPlaceSize_.x < 500.f && stackPlaceSize_.y > 0.f && stackPlaceSize_.y < 500.f);
	debugC(2, kDebugMinigames, "stackPlaceSize = (%5.1f, %5.1f)", stackPlaceSize_.x, stackPlaceSize_.y);

	prevPlace_ = -1;
	pickedItem_ = -1;
	mouseObjPose_ = stidx(stackSize_ + 1);

	inField_ = _runtime->debugMode() ? nodes_.size() : 0;
	nextObjTime_ = _runtime->getTime();

	setState(MinigameInterface::RUNNING);
}

Puzzle::~Puzzle() {
	for (auto &it : nodes_)
		_runtime->release(it.obj);

	_runtime->release(stackBottom_);
}

void Puzzle::rotate(int item) {
	assert(item >= 0 && item < (int)nodes_.size());
	nodes_[item].angle = (nodes_[item].angle + 1) % angles_;
}

int Puzzle::stidx(int idx) const {
	return -idx - 2;
}

bool Puzzle::testPlace(int item) const {
	assert(item >= 0 && item < (int)nodes_.size());
	return nodes_[item].pos == item && nodes_[item].angle == 0;
}

bool Puzzle::isFlying(int idx) const {
	for (auto &it : flyObjs_)
		if (it.data == idx)
			return true;
	return false;
}

bool Puzzle::isOnMouse(const Node& node) const {
	if (node.pos == mouseObjPose_) {
		return true;
	}
	return false;
}

void Puzzle::put(int where, int what, float flowSpeed) {
	assert(where < (int)field_.size());
	assert(what >= 0 && what < (int)nodes_.size());

	Node& node = nodes_[what];
	int start = node.pos;

	if (flowSpeed > 0.f || isFlying(what)) {
		FlyQDObject* flyObj = 0;

		for (auto &fit : flyObjs_) {
			if (fit.data == what)
				break;
			if (&fit != flyObjs_.end()) // Этот фрагмент уже летит, просто поменять точку назначения
				flyObj = &fit;
			else { // Добавляем новый летящий фрагмент
				flyObjs_.push_back(FlyQDObject());
				flyObj = &flyObjs_.back();

				flyObj->data = what;

				mgVect3f from = isOnMouse(node) ? node.obj->R() : start < -1 ? stackPosition(stidx(start)) : position(start);
				flyObj->current = _runtime->world2game(from);
				node.obj->set_R(from);

				flyObj->speed = flowSpeed;
			}
		}

		mgVect3f to = where < -1 ? stackPosition(stidx(where)) : position(where);
		flyObj->target = _runtime->world2game(to);
		flyObj->depth = _runtime->getDepth(to);
	}

	if (where >= 0)
		field_[where] = what;

	node.pos = where;
}

void Puzzle::putOnStack(int what, float speed) {
	put(stidx((int)stack_.size()), what, speed);
	stack_.push_back(what);
}

void Puzzle::returnToStack() {
	assert(pickedItem_ != -1);
	_runtime->event(EVENT_RETURN, _runtime->mousePosition());
	if (prevPlace_ >= 0)
		put(prevPlace_, pickedItem_);
	else
		putOnStack(pickedItem_, returnSpeed_);
	prevPlace_ = -1;
	pickedItem_ = -1;
	_runtime->event(EVENT_CLICK, _runtime->mousePosition());
}

void Puzzle::quant(float dt) {
	if (pickedItem_ == -1)
		_runtime->setGameHelpVariant(0);
	else
		_runtime->setGameHelpVariant(1);

	if (_runtime->getTime() > nextRotateTime_) {
		_runtime->event(EVENT_FIELD_ROTATE, mgVect2f(400, 300));
		nextRotateTime_ = _runtime->getTime() + rotateTimePeriod_;
		globalAngle_ = (globalAngle_ + 1) % angles_;
		_runtime->setCompleteHelpVariant(globalAngle_);
	}

	FlyQDObjects::iterator fit = flyObjs_.begin();
	while (fit != flyObjs_.end())
		if (!isOnMouse(nodes_[fit->data]) && fit->quant(dt, nodes_[fit->data].obj, _runtime))
			++fit;
		else
			fit = flyObjs_.erase(fit);

	if (inField_ < (int)nodes_.size() && _runtime->getTime() > nextObjTime_ &&
			((int)stack_.size() < stackSize_ - 1 || ((int)stack_.size() < stackSize_ && pickedItem_ == -1))) { // нужно добавить в инвентори фишку
		// ищем случайный не выставленный фрагмент
		int freeIdx = round(_runtime->rnd(0.f, nodes_.size() - 1));
		Nodes::iterator it = nodes_.begin();
		for (;;) {
			if (++it == nodes_.end())
				it = nodes_.begin();
			if (it->isFree())
				if (!freeIdx--)
					break;
		}
		int idx = Common::distance(nodes_.begin(), it);

		++inField_;
		nextObjTime_ = _runtime->getTime() + stackPlaceSize_.y / flySpeed_;

		it->pos = stidx(stackSize_);
		it->obj.setState(getStateName(it->angle, false, true));

		putOnStack(idx, flySpeed_);
	}

	mgVect2f mouse = _runtime->mousePosition();

	int hovPlace = -1;  // Номер места которое сейчас под мышкой
	for (int idx = 0; idx < (int)stack_.size(); ++idx)
		if (nodes_[stack_[idx]].obj.hit(mouse)) {
			hovPlace = stidx(idx);
			break;
		}
	if (hovPlace == -1) {
		float radius = 0.5f * size_.x;
		for (int idx = 0; idx < gameSize_; ++idx)
			if (dist(_runtime->world2game(position(idx)), mouse) < radius) {
				hovPlace = idx;
				break;
			}
	}
	if (hovPlace == -1) {
		mgVect2i st = stackBottom_->screen_R();
		st.y -= stackPlaceSize_.y * stackSize_ - 0.5f * stackPlaceSize_.x;
		Rectf stackPos(st.x - 0.5f * stackPlaceSize_.x, st.y, stackPlaceSize_.x, stackPlaceSize_.y * stackSize_);
		if (stackPos.point_inside(mouse))
			hovPlace = stidx(stackSize_);
	}

	if (_runtime->mouseLeftPressed()) {
		if (hovPlace >= 0) { // клик по полю
			Indexes::value_type& hovItem = field_[hovPlace];
			if (hovItem == -1) // клик по пустой ячейке
				if (pickedItem_ == -1) // на мыши ничего нет
					_runtime->event(EVENT_CLICK, mouse);
				else { // кладем фрагмент с мыши
					put(hovPlace, pickedItem_);
					if (testPlace(pickedItem_)) // положили на свое свое место
						_runtime->event(EVENT_PUT_RIGHT, mouse);
					else // просто положили
						_runtime->event(EVENT_PUT, mouse);
					pickedItem_ = -1;
					prevPlace_ = -1;
				} else { // клик по непустой ячейке
				if (testPlace(hovPlace)) // клик по правильно уложенной фишке
					_runtime->event(EVENT_CLICK_RIGHT, mouse);
				else if (pickedItem_ != -1) { // поменять с тем что на мыше
					bool swap = true;
					if (prevPlace_ >= 0)
						put(prevPlace_, hovItem);
					else
						putOnStack(hovItem, returnSpeed_);
					if (testPlace(hovItem)) { // оказалась при обмене на своем месте
						_runtime->event(EVENT_PUT_RIGHT, _runtime->world2game(position(prevPlace_)));
						swap = false;
					}
					put(hovPlace, pickedItem_);
					if (testPlace(pickedItem_)) { // положили на свое свое место
						_runtime->event(EVENT_PUT_RIGHT, mouse);
						swap = false;
					}
					if (swap) // просто обменяли
						_runtime->event(EVENT_SWAP, mouse);
					pickedItem_ = -1;
					prevPlace_ = -1;
				} else { // взять фрагмент на мышь
					_runtime->event(EVENT_GET, mouse);
					prevPlace_ = hovPlace;
					pickedItem_ = hovItem;
					nodes_[pickedItem_].pos = mouseObjPose_;
					hovItem = -1;
				}
			}
		} else if (hovPlace < -1) { // клик по стеку
			int hovStack = stidx(hovPlace);
			if (pickedItem_ == -1) // на мыши ничего нет
				if (hovStack < (int)stack_.size()) { // взять фрагмент из стека на мышь
					_runtime->event(EVENT_GET, mouse);
					Indexes::iterator it = stack_.begin() + hovStack;
					assert(*it >= 0);
					prevPlace_ = -1;
					pickedItem_ = *it;
					nodes_[pickedItem_].pos = mouseObjPose_;
					stack_.erase(it);
					for (int idx = hovStack; idx < (int)stack_.size(); ++idx)
						put(stidx(idx), stack_[idx], flySpeed_);
				} else // пустой клик в области стека
					_runtime->event(EVENT_CLICK, mouse);
			else // вернуть фишку на место
				returnToStack();
		} else // пустой клик мимо игрового поля
			_runtime->event(EVENT_CLICK, mouse);
	} else if (_runtime->mouseRightPressed()) {
		if (pickedItem_ == -1) {
			if (hovPlace >= 0) { // клик по полю
				if (testPlace(hovPlace)) // клик по правильно уложенной фишке
					_runtime->event(EVENT_CLICK_RIGHT, mouse);
				else {
					Indexes::value_type& hovItem = field_[hovPlace];
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
				if (hovStack < (int)stack_.size()) { // покрутить внутри стека
					_runtime->event(EVENT_ROTATE_IN_STACK, mouse);
					rotate(stack_[hovStack]);
				} else // попытка прокрутить пустое место
					_runtime->event(EVENT_CLICK, mouse);
			} else // пустой клик мимо игрового поля
				_runtime->event(EVENT_CLICK, mouse);
		} else // вернуть фишку на место
			returnToStack();
	}

	bool iWin = true;
	for (int idx = 0; idx < (int)nodes_.size(); ++idx) {
		Node& node = nodes_[idx];
		if (node.pos != -1) {
			if (node.pos >= 0) {
				if (isFlying(idx))
					node.obj.setState(getStateName(node.angle, false, false));
				else {
					node.obj.setState(getStateName(node.angle, node.pos == hovPlace && !testPlace(idx), false));
					node.obj->set_R(position(node.pos));
				}
			} else if (idx == pickedItem_) {
				node.obj.setState(getStateName(node.angle, hovPlace >= 0 && !testPlace(hovPlace), false));
				node.obj->set_R(_runtime->game2world(mouse, stackBottom_.depth(_runtime) - 200));
			} else {
				node.obj.setState(getStateName(node.angle, node.pos == hovPlace && pickedItem_ == -1, true));
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
	assert(num >= 0 && num < (int)positions_.size());
	// Если глобальный поворот ненулевой, пересчитываем индекс
	if (globalAngle_ > 0) {
		int size = sqrt((float)gameSize_);
		int y = num / size;
		int x = num - y * size;
		--size;
		for (int angle = 0; angle < globalAngle_; ++angle) {
			int tmp = x;
			x = size - y;
			y = tmp;
		}
		num = y * (size + 1) + x;
	}
	assert(num >= 0 && num < (int)positions_.size());
	return positions_[num];
}

mgVect3f Puzzle::stackPosition(int num) const {
	mgVect3f bottom = _runtime->world2game(stackBottom_);
	bottom.y -= stackPlaceSize_.y * num;
	return _runtime->game2world(bottom);
}

} // namespace QDEngine
