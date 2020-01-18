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

#include "ultima/shared/gfx/visual_item.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

BEGIN_MESSAGE_MAP(VisualItem, NamedItem)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(HideMsg)
END_MESSAGE_MAP()

void VisualItem::init(TreeItem *parent) {
	_isDirty = true;
	if (parent != nullptr)
		addUnder(parent);
}

bool VisualItem::ShowMsg(CShowMsg &msg) {
	// When a view is shown, mark it to be redrawn
	_isDirty = true;
	//	Font *font = Font::getActiveFont();
	//	_fontDetails._fontNumber = font ? font->_fontNumber : 1;

	return false;
}

bool VisualItem::HideMsg(CHideMsg &msg) {
	// When view is hidden, mark it as not dirty
	_isDirty = false;
	return false;
}

VisualSurface VisualItem::getSurface() {
	Graphics::ManagedSurface src(*g_vm->_screen, _bounds);
	return VisualSurface(src, _bounds);
}

void VisualItem::setBounds(const Common::Rect &r) {
	_bounds = r;
	setDirty();
}

void VisualItem::setPosition(const Common::Point &pt) {
	_bounds.moveTo(pt);
	setDirty();
}

void VisualItem::setDirty() {
	// Flag the item as dirty
	_isDirty = true;

	// Flag any child items also as dirty, since rendering the parent
	// would cover up any areas that the sub-items render within
	for (TreeItem *treeItem = scan(this); treeItem; treeItem = treeItem->scan(this)) {
		VisualItem *item = dynamic_cast<VisualItem *>(treeItem);
		if (item)
			item->_isDirty = true;
	}
}

void VisualItem::changeView(const Common::String &name) {
	// TODO
}

} // End of namespace Gfx
} // End of namespace Shaerd
} // End of namespace Ultima
