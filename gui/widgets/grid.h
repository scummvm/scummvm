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

#ifndef GUI_WIDGETS_GRID_H
#define GUI_WIDGETS_GRID_H

#include "gui/dialog.h"
#include "gui/widget.h"
#include "common/str.h"
#include "common/array.h"
#include "image/bmp.h"
#include "image/png.h"

namespace GUI {

const Graphics::ManagedSurface *scaleGfx(Graphics::ManagedSurface *gfx, int w, int h);

enum Platform {
	kPlatformDOS,
	kPlatformAmiga,
	kPlatformApple2,
	kPlatformUnknown = -1
};

struct GridItemInfo
{
	Common::String engineid;
	Common::String gameid;
	Common::String language;
	Common::String title;
	Common::String platform;
	Common::String thumbPath;

	GridItemInfo(Common::String &eid, Common::String &gid, Common::String &t, Common::String &l, Common::String &p) : 
		gameid(gid), engineid(eid), title(t), language(l), platform(p) {
		thumbPath = Common::String::format("%s-%s.png", engineid.c_str(), gameid.c_str());
	}
};

class GridItemWidget;

/* GridWidget */
class GridWidget : public ContainerWidget {
private:
	Common::Array<const Graphics::ManagedSurface *> _platformIcons;
	// _gridItems should be reserved to hold few more than visible items
	Common::Array<GridItemWidget *> _gridItems;
	Common::Array<GridItemInfo> _allEntries;
	Common::Array<GridItemInfo> _visibleEntries;
	Common::HashMap<Common::String, const Graphics::ManagedSurface *> _loadedSurfaces;

	Common::Array<Common::Array<GridItemWidget *>> _grid;

	ScrollBarWidget *_scrollBar;

	uint16 _scrollWindowHeight, _scrollWindowWidth, _scrollSpeed;
	uint16 _innerHeight, _innerWidth;
	uint16 _thumbnailHeight, _thumbnailWidth;
	uint16 _gridItemHeight, _gridItemWidth;
	uint16 _gridXSpacing, _gridYSpacing;
	
	int _scrollPos;
	int _itemsPerRow;
	int _firstVisibleItem;
	int _itemsOnScreen;
	
	bool _titlesVisible;


public:

	GridWidget(GuiObject *boss, int x, int y, int w, int h);
	GridWidget(GuiObject *boss, const Common::String &name);

	const Graphics::ManagedSurface * filenameToSurface(Common::String &name);
	const Graphics::ManagedSurface * platformToSurface(Platform platformCode);


	bool calcVisibleEntries(void);
	void setEntryList(Common::Array<GridItemInfo> *list);
	void destroyItems();
	void loadPlatformIcons();
	void updateGrid(void);
	void gridFromGameList();
	int getLoadedNumber(void) {return _loadedSurfaces.size();}
	void reloadThumbnails();
	void handleMouseWheel(int x, int y, int direction) override;
	void reflowLayout() override;


	int selectedEntry;
};

enum {
	kThumbnailWidth = 192,
	kThumbnailHeight = 192
};

/* EntryContainerWidget */
class GridItemWidget : public ContainerWidget {
public:
	GraphicsWidget *_thumb;
	GraphicsWidget *_plat;
	StaticTextWidget *_lang;
	StaticTextWidget *_title;
	GridWidget *_grid;

	Common::Array<GridItemInfo> _attachedEntries;
	GridItemInfo *_activeEntry;

	bool isHighlighted;
	void setActiveEntry(GridItemInfo &entry);

public:
	GridItemWidget(GridWidget *boss, int x, int y, int w, int h);
	GridItemWidget(GridWidget *boss, GraphicsWidget *th, GraphicsWidget *p, StaticTextWidget *l, StaticTextWidget *t);
	
	void attachEntry(Common::String key, Common::String description, Common::ConfigManager::Domain *domain);
	void attachEntry(GridItemInfo &entry);
	void attachEntries(Common::Array<GridItemInfo> entry);
	void setActiveEntry(int i) {setActiveEntry(_attachedEntries[i]);};
	void update();
	void updateThumb();
	void drawWidget() override;

};

} // End of namespace GUI

#endif
