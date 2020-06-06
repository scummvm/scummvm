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

#include "common/util.h"
#include "common/system.h"

#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/ThemeEval.h"
#include "gui/ThemeLayout.h"

#include "graphics/font.h"

#ifdef LAYOUT_DEBUG_DIALOG
#include "graphics/surface.h"
#endif

namespace GUI {

void ThemeLayout::importLayout(ThemeLayout *layout) {
	assert(layout->getLayoutType() == kLayoutMain);

	if (layout->_children.size() == 0)
		return;

	layout = layout->_children[0];

	if (getLayoutType() == layout->getLayoutType()) {
		for (uint i = 0; i < layout->_children.size(); ++i)
			_children.push_back(layout->_children[i]->makeClone(this));
	} else {
		ThemeLayout *clone = layout->makeClone(this);

		// When importing a layout into a layout of the same type, the children
		// of the imported layout are copied over, ignoring the padding of the
		// imported layout. Here when importing a layout of a different type
		// into a layout we explicitly ignore the padding so the appearance
		// is the same in both cases.
		clone->setPadding(0, 0, 0, 0);

		_children.push_back(clone);
	}
}

void ThemeLayout::resetLayout() {
	_x = 0;
	_y = 0;
	_w = _defaultW;
	_h = _defaultH;

	for (uint i = 0; i < _children.size(); ++i)
		_children[i]->resetLayout();
}

bool ThemeLayout::getWidgetData(const Common::String &name, int16 &x, int16 &y, int16 &w, int16 &h, bool &useRTL) {
	if (name.empty()) {
		assert(getLayoutType() == kLayoutMain);
		x = _x; y = _y;
		w = _w; h = _h;
		useRTL = _useRTL;

		return true;
	}

	for (uint i = 0; i < _children.size(); ++i) {
		if (_children[i]->getWidgetData(name, x, y, w, h, useRTL))
			return true;
	}

	return false;
}

Graphics::TextAlign ThemeLayout::getWidgetTextHAlign(const Common::String &name) {
	if (name.empty()) {
		assert(getLayoutType() == kLayoutMain);
		return _textHAlign;
	}

	Graphics::TextAlign res;

	for (uint i = 0; i < _children.size(); ++i) {
		if ((res = _children[i]->getWidgetTextHAlign(name)) != Graphics::kTextAlignInvalid)
			return res;
	}

	return Graphics::kTextAlignInvalid;
}

int16 ThemeLayoutStacked::getParentWidth() {
	ThemeLayout *p = _parent;
	int width = 0;

	while (p && p->getLayoutType() != kLayoutMain) {
		width += p->_padding.right + p->_padding.left;
		if (p->getLayoutType() == kLayoutHorizontal) {
			const int spacing = ((ThemeLayoutStacked *)p)->_spacing;
			for (uint i = 0; i < p->_children.size(); ++i)
				width += p->_children[i]->getWidth() + spacing;
		}
		// FIXME: Do we really want to assume that any layout type different
		// from kLayoutHorizontal corresponds to width 0 ?
		p = p->_parent;
	}

	assert(p && p->getLayoutType() == kLayoutMain);
	return p->getWidth() - width;
}

int16 ThemeLayoutStacked::getParentHeight() {
	ThemeLayout *p = _parent;
	int height = 0;

	while (p && p->getLayoutType() != kLayoutMain) {
		height += p->_padding.bottom + p->_padding.top;
		if (p->getLayoutType() == kLayoutVertical) {
			const int spacing = ((ThemeLayoutStacked *)p)->_spacing;
			for (uint i = 0; i < p->_children.size(); ++i)
				height += p->_children[i]->getHeight() + spacing;
		}
		// FIXME: Do we really want to assume that any layout type different
		// from kLayoutVertical corresponds to height 0 ?
		p = p->_parent;
	}

	assert(p && p->getLayoutType() == kLayoutMain);
	return p->getHeight() - height;
}

#ifdef LAYOUT_DEBUG_DIALOG
void ThemeLayout::debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
	uint32 color = 0xFFFFFFFF;
	font->drawString(screen, getName(), _x, _y, _w, color, Graphics::kTextAlignRight, 0, true);
	screen->hLine(_x, _y, _x + _w, color);
	screen->hLine(_x, _y + _h, _x + _w , color);
	screen->vLine(_x, _y, _y + _h, color);
	screen->vLine(_x + _w, _y, _y + _h, color);

	for (uint i = 0; i < _children.size(); ++i)
		_children[i]->debugDraw(screen, font);
}
#endif


bool ThemeLayoutWidget::getWidgetData(const Common::String &name, int16 &x, int16 &y, int16 &w, int16 &h, bool &useRTL) {
	if (name == _name) {
		x = _x; y = _y;
		w = _w; h = _h;
		useRTL = _useRTL;

		return true;
	}

	return false;
}

Graphics::TextAlign ThemeLayoutWidget::getWidgetTextHAlign(const Common::String &name) {
	if (name == _name) {
		return _textHAlign;
	}

	return Graphics::kTextAlignInvalid;
}

void ThemeLayoutWidget::reflowLayout(Widget *widgetChain) {
	Widget *guiWidget = getWidget(widgetChain);
	if (!guiWidget) {
		return;
	}

	int minWidth  = -1;
	int minHeight = -1;
	guiWidget->getMinSize(minWidth, minHeight);

	if (_w != -1 && minWidth != -1 && minWidth > _w) {
		_w = minWidth;
	}

	if (_h != -1 && minHeight != -1 && minHeight > _h) {
		_h = minHeight;
	}
}

bool ThemeLayoutWidget::isBound(Widget *widgetChain) const {
	Widget *guiWidget = getWidget(widgetChain);
	return guiWidget != nullptr;
}

Widget *ThemeLayoutWidget::getWidget(Widget *widgetChain) const {
	const ThemeLayout *topLevelLayout = this;
	while (topLevelLayout->_parent) {
		topLevelLayout = topLevelLayout->_parent;
	}

	assert(topLevelLayout && topLevelLayout->getLayoutType() == kLayoutMain);
	const ThemeLayoutMain *dialogLayout = static_cast<const ThemeLayoutMain *>(topLevelLayout);

	Common::String widgetName = Common::String::format("%s.%s", dialogLayout->getName(), _name.c_str());
	return Widget::findWidgetInChain(widgetChain, widgetName.c_str());
}

void ThemeLayoutMain::reflowLayout(Widget *widgetChain) {
	assert(_children.size() <= 1);

	resetLayout();

	if (_overlays == "screen") {
		_x = 0;
		_y = 0;
		_w = g_system->getOverlayWidth();
		_h = g_system->getOverlayHeight();
	} else if (_overlays == "screen_center") {
		_x = -1;
		_y = -1;
		_w = _defaultW > 0 ? MIN(_defaultW, g_system->getOverlayWidth()) : -1;
		_h = _defaultH > 0 ? MIN(_defaultH, g_system->getOverlayHeight()) : -1;
	} else {
		if (!g_gui.xmlEval()->getWidgetData(_overlays, _x, _y, _w, _h)) {
			warning("Unable to retrieve overlayed dialog position %s", _overlays.c_str());
		}

		if (_w == -1 || _h == -1) {
			warning("The overlayed dialog %s has not been sized, using a default size for %s", _overlays.c_str(), _name.c_str());
			_x = g_system->getOverlayWidth()      / 10;
			_y = g_system->getOverlayHeight()     / 10;
			_w = g_system->getOverlayWidth()  * 8 / 10;
			_h = g_system->getOverlayHeight() * 8 / 10;
		}
	}

	if (g_gui.useRTL()) {
		if (this->_name == "GameOptions" || this->_name == "GlobalOptions" || this->_name == "Browser") {
			/** The dialogs named above are the stacked dialogs for which the left+right paddings need to be adjusted for RTL.
				Whenever a stacked dialog is opened, the below code sets the left and right paddings and enables widgets to be
				shifted by that amount. If any new stacked and padded dialogs are added in the future,
				add them here and in Widget::draw() to enable RTL support for that particular dialog
			*/
			int oldX = _x;
			_x = g_system->getOverlayWidth() - _w - _x;
			g_gui.setDialogPaddings(oldX, _x);
		}
	}

	if (_x >= 0) _x += _inset;
	if (_y >= 0) _y += _inset;
	if (_w >= 0) _w -= 2 * _inset;
	if (_h >= 0) _h -= 2 * _inset;

	if (_children.size()) {
		_children[0]->setWidth(_w);
		_children[0]->setHeight(_h);
		_children[0]->reflowLayout(widgetChain);

		if (_w == -1)
			_w = _children[0]->getWidth();

		if (_h == -1)
			_h = _children[0]->getHeight();

		if (_y == -1)
			_y = (g_system->getOverlayHeight() >> 1) - (_h >> 1);

		if (_x == -1)
			_x = (g_system->getOverlayWidth() >> 1) - (_w >> 1);
	}
}

void ThemeLayoutStacked::reflowLayoutVertical(Widget *widgetChain) {
	int curY;
	int resize[8];
	int rescount = 0;
	bool fixedWidth = _w != -1;

	curY = _padding.top;
	_h = _padding.top + _padding.bottom;

	for (uint i = 0; i < _children.size(); ++i) {
		if (!_children[i]->isBound(widgetChain)) continue;

		_children[i]->reflowLayout(widgetChain);

		if (_children[i]->getWidth() == -1) {
			int16 width = (_w == -1 ? getParentWidth() : _w) - _padding.left - _padding.right;
			_children[i]->setWidth(MAX<int16>(width, 0));
		}

		if (_children[i]->getHeight() == -1) {
			assert(rescount < ARRAYSIZE(resize));
			resize[rescount++] = i;
			_children[i]->setHeight(0);
		}

		_children[i]->offsetY(curY);

		// Advance the vertical offset by the height of the newest item, plus
		// the item spacing value.
		curY += _children[i]->getHeight() + _spacing;

		// Update width and height of this stack layout
		if (!fixedWidth) {
			_w = MAX(_w, (int16)(_children[i]->getWidth() + _padding.left + _padding.right));
		}
		_h += _children[i]->getHeight() + _spacing;
	}

	// If there are any children at all, then we added the spacing value once
	// too often. Correct that.
	if (!_children.empty())
		_h -= _spacing;

	// If the width is not set at this point, then we have no bound widgets.
	if (!fixedWidth && _w == -1) {
		_w = 0;
	}

	for (uint i = 0; i < _children.size(); ++i) {
		switch (_itemAlign) {
		case kItemAlignStart:
		default:
			_children[i]->offsetX(_padding.left);
			break;
		case kItemAlignCenter:
			// Center child if it this has been requested *and* the space permits it.
			if (_children[i]->getWidth() < (_w - _padding.left - _padding.right)) {
				_children[i]->offsetX((_w >> 1) - (_children[i]->getWidth() >> 1));
			} else {
				_children[i]->offsetX(_padding.left);
			}
			break;
		case kItemAlignEnd:
			_children[i]->offsetX(_w - _children[i]->getWidth() - _padding.right);
			break;
		case kItemAlignStretch:
			_children[i]->offsetX(_padding.left);
			_children[i]->setWidth(_w - _padding.left - _padding.right);
			break;
		}
	}

	// If there were any items with undetermined height, then compute and set
	// their height now. We do so by determining how much space is left, and
	// then distributing this equally over all items which need auto-resizing.
	if (rescount) {
		int newh = (getParentHeight() - _h - _padding.bottom) / rescount;
		if (newh < 0) newh = 0; // In case there is no room left, avoid giving a negative height to widgets

		for (int i = 0; i < rescount; ++i) {
			// Set the height of the item.
			_children[resize[i]]->setHeight(newh);
			// Increase the height of this ThemeLayoutStacked accordingly, and
			// then shift all subsequence children.
			_h += newh;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->offsetY(newh);
		}
	}
}

void ThemeLayoutStacked::reflowLayoutHorizontal(Widget *widgetChain) {
	int curX;
	int resize[8];
	int rescount = 0;
	bool fixedHeight = _h != -1;

	curX = _padding.left;
	_w = _padding.left + _padding.right;

	for (uint i = 0; i < _children.size(); ++i) {
		if (!_children[i]->isBound(widgetChain)) continue;

		_children[i]->reflowLayout(widgetChain);

		if (_children[i]->getHeight() == -1) {
			int16 height = (_h == -1 ? getParentHeight() : _h) - _padding.top - _padding.bottom;
			_children[i]->setHeight(MAX<int16>(height, 0));
		}

		if (_children[i]->getWidth() == -1) {
			assert(rescount < ARRAYSIZE(resize));
			resize[rescount++] = i;
			_children[i]->setWidth(0);
		}

		_children[i]->offsetX(curX);

		// Advance the horizontal offset by the width of the newest item, plus
		// the item spacing value.
		curX += (_children[i]->getWidth() + _spacing);

		// Update width and height of this stack layout
		_w += _children[i]->getWidth() + _spacing;
		if (!fixedHeight) {
			_h = MAX(_h, (int16)(_children[i]->getHeight() + _padding.top + _padding.bottom));
		}
	}

	// If there are any children at all, then we added the spacing value once
	// too often. Correct that.
	if (!_children.empty())
		_w -= _spacing;

	// If the height is not set at this point, then we have no bound widgets.
	if (!fixedHeight && _h == -1) {
		_h = 0;
	}

	for (uint i = 0; i < _children.size(); ++i) {
		switch (_itemAlign) {
		case kItemAlignStart:
		default:
			_children[i]->offsetY(_padding.top);
			break;
		case kItemAlignCenter:
			// Center child if it this has been requested *and* the space permits it.
			if (_children[i]->getHeight() < (_h - _padding.top - _padding.bottom)) {
				_children[i]->offsetY((_h >> 1) - (_children[i]->getHeight() >> 1));
			} else {
				_children[i]->offsetY(_padding.top);
			}
			break;
		case kItemAlignEnd:
			_children[i]->offsetY(_h - _children[i]->getHeight() - _padding.bottom);
			break;
		case kItemAlignStretch:
			_children[i]->offsetY(_padding.top);
			_children[i]->setHeight(_w - _padding.top - _padding.bottom);
			break;
		}
	}

	// If there were any items with undetermined width, then compute and set
	// their width now. We do so by determining how much space is left, and
	// then distributing this equally over all items which need auto-resizing.
	if (rescount) {
		int neww = (getParentWidth() - _w - _padding.right) / rescount;
		if (neww < 0) neww = 0; // In case there is no room left, avoid giving a negative width to widgets

		for (int i = 0; i < rescount; ++i) {
			// Set the width of the item.
			_children[resize[i]]->setWidth(neww);
			// Increase the width of this ThemeLayoutStacked accordingly, and
			// then shift all subsequence children.
			_w += neww;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->offsetX(neww);
		}
	}
}

} // End of namespace GUI
