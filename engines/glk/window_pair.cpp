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

#include "glk/window_pair.h"
#include "glk/conf.h"
#include "glk/glk.h"
#include "glk/screen.h"

namespace Glk {

PairWindow::PairWindow(Windows *windows, uint method, Window *key, uint size) :
	Window(windows, 0),
	_dir(method & winmethod_DirMask),
	_division(method & winmethod_DivisionMask),
	_wBorder((method & winmethod_BorderMask) == winmethod_Border),
	_vertical(_dir == winmethod_Left || _dir == winmethod_Right),
	_backward(_dir == winmethod_Left || _dir == winmethod_Above),
	_key(key), _size(size), _keyDamage(0) {
	_type = wintype_Pair;
}

PairWindow::~PairWindow() {
	for (uint idx = 0; idx < _children.size(); ++idx) {
		_children[idx]->_parent = nullptr;
		delete _children[idx];
	}
}

void PairWindow::rearrange(const Rect &box) {
	Rect box1, box2;
	int min, diff, split, splitwid, max;
	Window *ch1, *ch2;

	_bbox = box;

	if (_dir == winmethod_Arbitrary) {
		// When a pair window is in "arbitrary" mode, each child window has it's own independant positioning,
		// so thre's no need to be readjusting it
		return;
	}

	if (!_backward) {
		ch1 = _children[0];
		ch2 = _children[1];
	} else {
		ch1 = _children[1];
		ch2 = _children[0];
	}

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

	ch1->rearrange(box1);
	ch2->rearrange(box2);
}

void PairWindow::redraw() {
	// When the windows can be in arbitrary positions, some of them may be transparent, so we always
	// need to force a full screen redraw in such cases
	if (_dir == winmethod_Arbitrary)
		Windows::_forceRedraw = true;

	Window::redraw();

	for (int ctr = 0, idx = (_backward ? (int)_children.size() - 1 : 0); ctr < (int)_children.size();
		++ctr, idx += (_backward ? -1 : 1)) {
		_children[idx]->redraw();
	}

	Window *child = !_backward ? _children.front() : _children.back();
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

void PairWindow::getArrangement(uint *method, uint *size, Window **keyWin) {
	uint val = _dir | _division;
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

void PairWindow::setArrangement(uint method, uint size, Window *keyWin) {
	uint newDir;
	bool newVertical, newBackward;
	assert((method & winmethod_DirMask) != winmethod_Arbitrary && _dir != winmethod_Arbitrary);

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
		SWAP(_children[0], _children[1]);
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
	// Note in case windows are partially overlapping, we want the top-most window to get the click.
	// WHich is why we recurse in the opposite of the rendering direction (as the _backward flag) indicates
	for (int ctr = 0, idx = (!_backward ? (int)_children.size() - 1 : 0); ctr < (int)_children.size();
		++ctr, idx += (!_backward ? -1 : 1)) {
		Window *w = _children[idx];
		if (w->_bbox.contains(newPos))
			w->click(newPos);
	}
}

} // End of namespace Glk
