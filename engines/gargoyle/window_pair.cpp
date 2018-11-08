/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gargoyle/window_pair.h"
#include "gargoyle/conf.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/screen.h"

namespace Gargoyle {

PairWindow::PairWindow(Windows *windows, glui32 method, Window *key, glui32 size) :
		Window(windows, 0),
		_dir(method & winmethod_DirMask),
		_division(method & winmethod_DivisionMask),
		_wBorder((method & winmethod_BorderMask) == winmethod_Border),
		_vertical(_dir == winmethod_Left || _dir == winmethod_Right),
		_backward(_dir == winmethod_Left || _dir == winmethod_Above),
		_key(key), _size(size), _keyDamage(0), _child1(nullptr), _child2(nullptr) {
	_type = wintype_Pair;
}

void PairWindow::rearrange(const Rect &box) {
	Rect box1, box2;
	int min, diff, split, splitwid, max;
	Window *ch1, *ch2;

	_bbox = box;

	if (_vertical) {
		min = _bbox.left;
		max = _bbox.right;
	} else {
		min = _bbox.top;
		max = _bbox.bottom;
	}
	diff = max - min;

	// We now figure split.
	if (_vertical)
		splitwid = g_conf->_wPaddingX; // want border?
	else
		splitwid = g_conf->_wPaddingY; // want border?

	switch (_division) {
	case winmethod_Proportional:
		split = (diff * _size) / 100;
		break;

	case winmethod_Fixed:
		split = !_key ? 0 : _key->getSplit(_size, _vertical);
		break;

	default:
		split = diff / 2;
		break;
	}

	if (!_backward)
		split = max - split - splitwid;
	else
		split = min + split;

	if (min >= max) {
		split = min;
	} else {
		if (split < min)
			split = min;
		else if (split > max - splitwid)
			split = max - splitwid;
	}

	if (_vertical) {
		box1.left = _bbox.left;
		box1.right = split;
		box2.left = split + splitwid;
		box2.right = _bbox.right;
		box1.top = _bbox.top;
		box1.bottom = _bbox.bottom;
		box2.top = _bbox.top;
		box2.bottom = _bbox.bottom;
	} else {
		box1.top = _bbox.top;
		box1.bottom = split;
		box2.top = split + splitwid;
		box2.bottom = _bbox.bottom;
		box1.left = _bbox.left;
		box1.right = _bbox.right;
		box2.left = _bbox.left;
		box2.right = _bbox.right;
	}

	if (!_backward) {
		ch1 = _child1;
		ch2 = _child2;
	} else {
		ch1 = _child2;
		ch2 = _child1;
	}

	ch1->rearrange(box1);
	ch2->rearrange(box2);
}

void PairWindow::redraw() {
	Window::redraw();

	_child1->redraw();
	_child2->redraw();

	Window *child = !_backward ? _child1 : _child2;
	Rect box(child->_bbox.left, child->_yAdj ? child->_bbox.top - child->_yAdj : child->_bbox.top,
		child->_bbox.right, child->_bbox.bottom);

	if (_vertical) {
		int xBord = _wBorder ? g_conf->_wBorderX : 0;
		int xPad = (g_conf->_wPaddingX - xBord) / 2;
		
		g_vm->_screen->fillRect(Rect(box.right + xPad, box.top, box.right + xPad + xBord, box.bottom),
			g_conf->_borderColor);
	} else {
		int yBord = _wBorder ? g_conf->_wBorderY : 0;
		int yPad = (g_conf->_wPaddingY - yBord) / 2;
		g_vm->_screen->fillRect(Rect(box.left, box.bottom + yPad, box.right, box.bottom + yPad + yBord),
			g_conf->_borderColor);
	}
}

void PairWindow::getArrangement(glui32 *method, glui32 *size, Window **keyWin) {
	glui32 val = _dir | _division;
	if (!_wBorder)
		val |= winmethod_NoBorder;

	if (size)
		*size = _size;
	if (keyWin) {
		if (_key)
			*keyWin = _key;
		else
			*keyWin = nullptr;
	}

	if (method)
		*method = val;
}

void PairWindow::setArrangement(glui32 method, glui32 size, Window *keyWin) {
	glui32 newDir;
	bool newVertical, newBackward;

	if (_key) {
		Window *wx;
		PairWindow *pairWin = dynamic_cast<PairWindow *>(_key);

		if (pairWin) {
			warning("setArrangement: keywin cannot be a Pair");
			return;
		}

		for (wx = _key; wx; wx = wx->_parent) {
			if (wx == this)
				break;
		}
		if (wx == nullptr) {
			warning("setArrangement: keywin must be a descendant");
			return;
		}
	}

	newDir = method & winmethod_DirMask;
	newVertical = (newDir == winmethod_Left || newDir == winmethod_Right);
	newBackward = (newDir == winmethod_Left || newDir == winmethod_Above);
	if (!keyWin)
		keyWin = _key;

	if ((newVertical && !_vertical) || (!newVertical && _vertical)) {
		if (!_vertical)
			warning("setArrangement: split must stay horizontal");
		else
			warning("setArrangement: split must stay vertical");
		return;
	}

	if (keyWin && dynamic_cast<BlankWindow *>(keyWin)
			&& (method & winmethod_DivisionMask) == winmethod_Fixed) {
		warning("setArrangement: a Blank window cannot have a fixed size");
		return;
	}

	if ((newBackward && !_backward) || (!newBackward && _backward)) {
		// switch the children
		Window *tmpWin = _child1;
		_child1 = _child2;
		_child2 = tmpWin;
	}

	// set up everything else
	_dir = newDir;
	_division = method & winmethod_DivisionMask;
	_key = keyWin;
	_size = size;
	_wBorder = ((method & winmethod_BorderMask) == winmethod_Border);

	_vertical = (_dir == winmethod_Left || _dir == winmethod_Right);
	_backward = (_dir == winmethod_Left || _dir == winmethod_Above);

	_windows->rearrange();
}

void PairWindow::click(const Point &newPos) {
	if (_child1->_bbox.contains(newPos))
		_child1->click(newPos);

	if (_child2->_bbox.contains(newPos))
		_child2->click(newPos);
}

} // End of namespace Gargoyle
