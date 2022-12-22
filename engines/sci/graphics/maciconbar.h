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

#ifndef SCI_GRAPHICS_MACICONBAR_H
#define SCI_GRAPHICS_MACICONBAR_H

#include "common/array.h"

#include "sci/engine/vm.h"
#include "sci/event.h"

namespace Graphics {
struct Surface;
}

namespace Sci {

class GfxMacIconBar {
public:
	GfxMacIconBar();
	~GfxMacIconBar();

	void initIcons(uint16 count, reg_t *objs);
	void drawIcons();
	void setIconEnabled(int16 index, bool enabled);
	void setInventoryIcon(int16 icon);
	bool handleEvents(SciEvent evt, reg_t &iconObj);

private:
	struct IconBarItem {
		reg_t object;
		Graphics::Surface *nonSelectedImage;
		Graphics::Surface *selectedImage;
		Common::Rect rect;
		bool enabled;
	};

	Common::Array<IconBarItem> _iconBarItems;
	uint16 _inventoryIndex;
	Graphics::Surface *_inventoryIcon;
	bool _allDisabled;

	bool _isUpscaled;
	Common::SpanOwner<SciSpan<byte> > _upscaleBuffer;

	Graphics::Surface *loadPict(ResourceId id);
	Graphics::Surface *createImage(uint32 iconIndex, bool isSelected);
	void remapColors(Graphics::Surface *surf, const byte *palette);

	void freeIcons();
	void addIcon(reg_t obj);
	void drawIcon(uint16 index, bool selected);
	void drawSelectedImage(uint16 index);
	bool isIconEnabled(uint16 index) const;
	void drawDisabledPattern(Graphics::Surface &surface, const Common::Rect &rect);
	void drawImage(Graphics::Surface *surface, const Common::Rect &rect, bool enabled);
	bool pointOnIcon(uint32 iconIndex, Common::Point point);
};

} // End of namespace Sci

#endif
