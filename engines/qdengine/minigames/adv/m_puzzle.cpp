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

#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/m_puzzle.h"
#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/Rect.h"
#include "qdengine/minigames/adv/qdMath.h"
#include "qdengine/system/input/keyboard_input.h"

namespace QDEngine {

typedef Rect<float, mgVect2f> Rectf;

MinigameInterface *createGame() {
	return new Puzzle;
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

	xassert(angle >= 0 && angle < angles_);
	angle = (angle + globalAngle_) % angles_;

	_snprintf(buf, 31, "%s%02d%s", !singleSize_ && small ? small_pref : "", angle + 1, selected ? selected_suf : "");
	return buf;
}

Puzzle::Puzzle() {
	if (!getParameter("game_size", gameSize_, true))
		return;
	xassert(gameSize_ > 0 && gameSize_ < 100);

	field_.resize(gameSize_, -1);
	globalAngle_ = 0;

	singleSize_ = getParameter("small_objects", false);

	angles_ = getParameter("angles", 4);
	xassert(angles_ > 0 &&  angles_ < 10);

	if (!(stackBottom_ = g_runtime->getObject(g_runtime->parameter("inventory_bottom"))))
		return;
	if (!getParameter("inventory_size", stackSize_, true))
		return;

	if (getParameter("rotate_period", rotateTimePeriod_, false)) {
		xassert(sqr(sqrt((float)gameSize_)) == gameSize_);
		if (sqr(sqrt((float)gameSize_)) != gameSize_)
			return;
	} else
		rotateTimePeriod_ = 86400; // сутки
	nextRotateTime_ = g_runtime->getTime() + rotateTimePeriod_;

	flySpeed_ = getParameter("inventory_drop_speed", 240.f);
	xassert(flySpeed_ > 0.f);
	returnSpeed_ = getParameter("inventory_return_speed", -1.f);

	const char *name_begin = g_runtime->parameter("obj_name_begin", "obj_");

	char buf[128];
	buf[127] = 0;

	XBuffer gameData;
	for (int idx = 0; idx < gameSize_; ++idx) {
		_snprintf(buf, 127, "%s%02d", name_begin, idx + 1);

		Node node;
		node.obj = g_runtime->getObject(buf);

		if (g_runtime->debugMode()) {
			node.pos = nodes_.size();
			node.angle = 0;
			field_[node.pos] = node.pos;
		} else
			node.angle = g_runtime->rnd(0, angles_ - 1);
		node.obj.setState(getStateName(node.angle, false, true));

		gameData.write(node.obj->R());

		nodes_.push_back(node);
	}

	if (!g_runtime->processGameData(gameData))
		return;

	for (int idx = 0; idx < gameSize_; ++idx) {
		mgVect3f crd;
		gameData.read(crd);
		nodes_[idx].obj->set_R(crd);
		positions_.push_back(crd);
	}

	if (g_runtime->debugMode())
		nodes_[0].angle = angles_ - 1;

	size_ = g_runtime->getSize(nodes_[0].obj);
	debugC(2, kDebugMinigames, "size = (%6.2f,%6.2f)", size_.x, size_.y);

	depth_ = nodes_[0].obj.depth();

	stackPlaceSize_ = getParameter("inventory_place_size", size_ * 1.2f);
	xassert(stackPlaceSize_.x > 0.f && stackPlaceSize_.x < 500.f && stackPlaceSize_.y > 0.f && stackPlaceSize_.y < 500.f);
	debugC(2, kDebugMinigames, "stackPlaceSize = (%5.1f, %5.1f)", stackPlaceSize_.x, stackPlaceSize_.y);

	prevPlace_ = -1;
	pickedItem_ = -1;
	mouseObjPose_ = stidx(stackSize_ + 1);

	inField_ = g_runtime->debugMode() ? nodes_.size() : 0;
	nextObjTime_ = g_runtime->getTime();

	setState(MinigameInterface::RUNNING);
}

Puzzle::~Puzzle() {
	Nodes::iterator it;
	FOR_EACH(nodes_, it)
	g_runtime->release(it->obj);

	g_runtime->release(stackBottom_);
}

void Puzzle::rotate(int item) {
	xassert(item >= 0 && item < nodes_.size());
	nodes_[item].angle = (nodes_[item].angle + 1) % angles_;
}

int Puzzle::stidx(int idx) const {
	return -idx - 2;
}

bool Puzzle::testPlace(int item) const {
	xassert(item >= 0 && item < nodes_.size());
	return nodes_[item].pos == item && nodes_[item].angle == 0;
}

bool Puzzle::isFlying(int idx) const {
	FlyQDObjects::const_iterator it;
	FOR_EACH(flyObjs_, it)
	if (it->data == idx)
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
	xassert(where < (int)field_.size());
	xassert(what >= 0 && what < nodes_.size());

	Node& node = nodes_[what];
	int start = node.pos;

	if (flowSpeed > 0.f || isFlying(what)) {
		FlyQDObject* flyObj = 0;

		FlyQDObjects::iterator fit;
		FOR_EACH(flyObjs_, fit)
		if (fit->data == what)
			break;
		if (fit != flyObjs_.end()) // Этот фрагмент уже летит, просто поменять точку назначения
			flyObj = &*fit;
		else { // Добавляем новый летящий фрагмент
			flyObjs_.push_back(FlyQDObject());
			flyObj = &flyObjs_.back();

			flyObj->data = what;

			mgVect3f from = isOnMouse(node) ? node.obj->R() : start < -1 ? stackPosition(stidx(start)) : position(start);
			flyObj->current = g_runtime->world2game(from);
			node.obj->set_R(from);

			flyObj->speed = flowSpeed;
		}

		mgVect3f to = where < -1 ? stackPosition(stidx(where)) : position(where);
		flyObj->target = g_runtime->world2game(to);
		flyObj->depth = g_runtime->getDepth(to);
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
	xassert(pickedItem_ != -1);
	g_runtime->event(EVENT_RETURN, g_runtime->mousePosition());
	if (prevPlace_ >= 0)
		put(prevPlace_, pickedItem_);
	else
		putOnStack(pickedItem_, returnSpeed_);
	prevPlace_ = -1;
	pickedItem_ = -1;
	g_runtime->event(EVENT_CLICK, g_runtime->mousePosition());
}

void Puzzle::quant(float dt) {
	if (pickedItem_ == -1)
		g_runtime->setGameHelpVariant(0);
	else
		g_runtime->setGameHelpVariant(1);

	if (g_runtime->getTime() > nextRotateTime_) {
		g_runtime->event(EVENT_FIELD_ROTATE, mgVect2f(400, 300));
		nextRotateTime_ = g_runtime->getTime() + rotateTimePeriod_;
		globalAngle_ = (globalAngle_ + 1) % angles_;
		g_runtime->setCompleteHelpVariant(globalAngle_);
	}

	FlyQDObjects::iterator fit = flyObjs_.begin();
	while (fit != flyObjs_.end())
		if (!isOnMouse(nodes_[fit->data]) && fit->quant(dt, nodes_[fit->data].obj))
			++fit;
		else
			fit = flyObjs_.erase(fit);

	if (inField_ < nodes_.size() && g_runtime->getTime() > nextObjTime_ &&
	(stack_.size() < stackSize_ - 1 || stack_.size() < stackSize_ && pickedItem_ == -1)) { // нужно добавить в инвентори фишку
		// ищем случайный не выставленный фрагмент
		int freeIdx = round(g_runtime->rnd(0.f, nodes_.size() - 1));
		Nodes::iterator it = nodes_.begin();
		for (;;) {
			if (++it == nodes_.end())
				it = nodes_.begin();
			if (it->isFree())
				if (!freeIdx--)
					break;
		}
		int idx = distance(nodes_.begin(), it);

		++inField_;
		nextObjTime_ = g_runtime->getTime() + stackPlaceSize_.y / flySpeed_;

		it->pos = stidx(stackSize_);
		it->obj.setState(getStateName(it->angle, false, true));

		putOnStack(idx, flySpeed_);
	}

	mgVect2f mouse = g_runtime->mousePosition();

	int hovPlace = -1;  // Номер места которое сейчас под мышкой
	for (int idx = 0; idx < stack_.size(); ++idx)
		if (nodes_[stack_[idx]].obj.hit(mouse)) {
			hovPlace = stidx(idx);
			break;
		}
	if (hovPlace == -1) {
		float radius = 0.5f * size_.x;
		for (int idx = 0; idx < gameSize_; ++idx)
			if (dist(g_runtime->world2game(position(idx)), mouse) < radius) {
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

	if (g_runtime->mouseLeftPressed()) {
		if (hovPlace >= 0) { // клик по полю
			Indexes::value_type& hovItem = field_[hovPlace];
			if (hovItem == -1) // клик по пустой ячейке
				if (pickedItem_ == -1) // на мыши ничего нет
					g_runtime->event(EVENT_CLICK, mouse);
				else { // кладем фрагмент с мыши
					put(hovPlace, pickedItem_);
					if (testPlace(pickedItem_)) // положили на свое свое место
						g_runtime->event(EVENT_PUT_RIGHT, mouse);
					else // просто положили
						g_runtime->event(EVENT_PUT, mouse);
					pickedItem_ = -1;
					prevPlace_ = -1;
				} else { // клик по непустой ячейке
				if (testPlace(hovPlace)) // клик по правильно уложенной фишке
					g_runtime->event(EVENT_CLICK_RIGHT, mouse);
				else if (pickedItem_ != -1) { // поменять с тем что на мыше
					bool swap = true;
					if (prevPlace_ >= 0)
						put(prevPlace_, hovItem);
					else
						putOnStack(hovItem, returnSpeed_);
					if (testPlace(hovItem)) { // оказалась при обмене на своем месте
						g_runtime->event(EVENT_PUT_RIGHT, g_runtime->world2game(position(prevPlace_)));
						swap = false;
					}
					put(hovPlace, pickedItem_);
					if (testPlace(pickedItem_)) { // положили на свое свое место
						g_runtime->event(EVENT_PUT_RIGHT, mouse);
						swap = false;
					}
					if (swap) // просто обменяли
						g_runtime->event(EVENT_SWAP, mouse);
					pickedItem_ = -1;
					prevPlace_ = -1;
				} else { // взять фрагмент на мышь
					g_runtime->event(EVENT_GET, mouse);
					prevPlace_ = hovPlace;
					pickedItem_ = hovItem;
					nodes_[pickedItem_].pos = mouseObjPose_;
					hovItem = -1;
				}
			}
		} else if (hovPlace < -1) { // клик по стеку
			int hovStack = stidx(hovPlace);
			if (pickedItem_ == -1) // на мыши ничего нет
				if (hovStack < stack_.size()) { // взять фрагмент из стека на мышь
					g_runtime->event(EVENT_GET, mouse);
					Indexes::iterator it = stack_.begin() + hovStack;
					xassert(*it >= 0);
					prevPlace_ = -1;
					pickedItem_ = *it;
					nodes_[pickedItem_].pos = mouseObjPose_;
					stack_.erase(it);
					for (int idx = hovStack; idx < stack_.size(); ++idx)
						put(stidx(idx), stack_[idx], flySpeed_);
				} else // пустой клик в области стека
					g_runtime->event(EVENT_CLICK, mouse);
			else // вернуть фишку на место
				returnToStack();
		} else // пустой клик мимо игрового поля
			g_runtime->event(EVENT_CLICK, mouse);
	} else if (g_runtime->mouseRightPressed()) {
		if (pickedItem_ == -1) {
			if (hovPlace >= 0) { // клик по полю
				if (testPlace(hovPlace)) // клик по правильно уложенной фишке
					g_runtime->event(EVENT_CLICK_RIGHT, mouse);
				else {
					Indexes::value_type& hovItem = field_[hovPlace];
					if (hovItem >= 0) {
						rotate(hovItem);
						if (testPlace(hovItem)) // повернули на правильный угол
							g_runtime->event(EVENT_PUT_RIGHT, mouse);
						else // просто положили
							g_runtime->event(EVENT_ROTATE_IN_FIELD, mouse);
					} else // попытка прокрутить пустое место
						g_runtime->event(EVENT_CLICK, mouse);
				}
			} else  if (hovPlace < -1) { // клик по стеку
				int hovStack = stidx(hovPlace);
				if (hovStack < stack_.size()) { // покрутить внутри стека
					g_runtime->event(EVENT_ROTATE_IN_STACK, mouse);
					rotate(stack_[hovStack]);
				} else // попытка прокрутить пустое место
					g_runtime->event(EVENT_CLICK, mouse);
			} else // пустой клик мимо игрового поля
				g_runtime->event(EVENT_CLICK, mouse);
		} else // вернуть фишку на место
			returnToStack();
	}

	bool iWin = true;
	for (int idx = 0; idx < nodes_.size(); ++idx) {
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
				node.obj->set_R(g_runtime->game2world(mouse, stackBottom_.depth() - 200));
			} else {
				node.obj.setState(getStateName(node.angle, node.pos == hovPlace && pickedItem_ == -1, true));
				if (!isFlying(idx))
					node.obj->set_R(stackPosition(stidx(node.pos)));
			}
			iWin = iWin && testPlace(idx);
		} else {
			g_runtime->hide(node.obj);
			iWin = false;
		}
	}

	if (iWin)
		setState(GAME_WIN);

}

const mgVect3f &Puzzle::position(int num) const {
	xassert(num >= 0 && num < positions_.size());
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
	xassert(num >= 0 && num < positions_.size());
	return positions_[num];
}

mgVect3f Puzzle::stackPosition(int num) const {
	mgVect3f bottom = g_runtime->world2game(stackBottom_);
	bottom.y -= stackPlaceSize_.y * num;
	return g_runtime->game2world(bottom);
}

} // namespace QDEngine
