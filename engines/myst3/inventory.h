/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef INVENTORY_H_
#define INVENTORY_H_

#include "common/list.h"
#include "common/memstream.h"
#include "common/rect.h"

#include "engines/myst3/gfx.h"

#include "video/bink_decoder.h"

namespace Myst3 {

class Myst3Engine;
class Texture;

class Inventory {
public:
	Inventory(Myst3Engine *vm);
	virtual ~Inventory();

	void loadFromState();
	void updateState();

	void addItem(uint16 var, bool atEnd);
	void addSaavedroChapter(uint16 var);
	void addAll();
	void removeItem(uint16 var);
	void reset();

	uint16 hoveredItem();
	void useItem(uint16 var);

	void draw();
private:
	struct InventoryItem {
		uint16 var;
		Common::Rect rect;
	};

	typedef Common::List<InventoryItem> ItemList;

	struct ItemData {
		uint16 textureX;
		uint16 textureWidth;
		uint16 textureHeight;
		uint16 var;
	};

	static const ItemData _availableItems[8];
	const ItemData &getData(uint16 var);

	Myst3Engine *_vm;

	Texture *_texture;
	ItemList _inventory;

	void initializeTexture();

	bool hasItem(uint16 var);
	void reflow();

	void openBook(uint16 age, uint16 room, uint16 node);
	void closeAllBooks();
};

class DragItem : public Drawable {
public:
	DragItem(Myst3Engine *vm, uint id);
	~DragItem();
	void drawOverlay();
	void setFrame(uint16 frame);

private:
	Myst3Engine *_vm;

	Common::MemoryReadStream *_movieStream;
	Video::BinkDecoder _bink;

	uint16 _frame;
	Texture *_texture;

	Common::Rect getPosition();
};

} /* namespace Myst3 */
#endif /* INVENTORY_H_ */
