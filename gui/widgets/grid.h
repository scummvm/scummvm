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
#include "gui/widgets/scrollbar.h"
#include "common/str.h"

#include "image/bmp.h"
#include "image/png.h"
#include "graphics/svg.h"

using Common::String;
using Common::U32String;
using Common::Array;

typedef Array<String> StringArray;

namespace GUI {

class ScrollBarWidget;
class GridItemWidget;
class GridWidget;

const Graphics::ManagedSurface *scaleGfx(const Graphics::ManagedSurface *gfx, int w, int h);
Graphics::ManagedSurface *loadSurfaceFromFile(const Common::String &name);

enum {
	kPlayButtonCmd = 'PLAY',
	kEditButtonCmd = 'EDIT',
	kLoadButtonCmd = 'LOAD',
	kOpenTrayCmd = 'OPTR',
	kItemClicked = 'LBX1',
	kItemDoubleClickedCmd = 'LBX2'
};

/* GridItemInfo */
struct GridItemInfo {
	int 		entryID;
	String 		engineid;
	String 		gameid;
	String 		title;
	String 		thumbPath;
	Common::Language	language;
	Common::Platform 	platform;

	GridItemInfo(int id, const String &eid, const String &gid
		,const String &t, Common::Language l, Common::Platform p)
		: entryID(id), gameid(gid), engineid(eid), title(t), language(l), platform(p) {
		thumbPath = String::format("%s-%s.png", engineid.c_str(), gameid.c_str());
	}
};

/* GridItemTray */
class GridItemTray: public Dialog, public CommandSender {
	int			 	_entryID;
	GridWidget 		*_grid;
	GuiObject 		*_boss;
public:
	GridItemTray(GuiObject *boss, int x, int y, int w, int h, int entryID, GridWidget *grid);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseWheel(int x, int y, int direction) override;
	void handleMouseMoved(int x, int y, int button) override;
};


/* GridWidget */
class GridWidget : public ContainerWidget, public CommandSender {
protected:
	Common::HashMap<int, const Graphics::ManagedSurface *> _platformIcons;
	Common::HashMap<int, const Graphics::ManagedSurface *> _languageIcons;

	// Images are mapped by filename -> surface.
	Common::HashMap<String, const Graphics::ManagedSurface *> _loadedSurfaces;

	Common::Array<GridItemWidget *> 	_gridItems;
	Common::Array<GridItemInfo> 		_allEntries;
	Common::Array<GridItemInfo> 		_visibleEntries;

	ScrollBarWidget *_scrollBar;

	int 			_scrollBarWidth;
	int				_scrollWindowHeight;
	int				_scrollWindowWidth;
	int				_scrollSpeed;
	int				_scrollPos;

public:
	int				_innerHeight;
	int				_innerWidth;
	int				_thumbnailHeight;
	int				_thumbnailWidth;
	int				_gridItemHeight;
	int				_gridItemWidth;
	int				_minGridXSpacing;
	int				_minGridYSpacing;
	int				_gridXSpacing;
	int				_gridYSpacing;
	int				_trayHeight;

	int				_rows;
	int				_itemsPerRow;
	int				_firstVisibleItem;
	int				_itemsOnScreen;

	bool 			_isTitlesVisible;

	int 			_selectedEntry;
	GridItemTray 	*_tray;
	String 			_iconDir;

	GridWidget(GuiObject *boss, int x, int y, int w, int h);
	GridWidget(GuiObject *boss, const String &name);
	~GridWidget();

	const Graphics::ManagedSurface *filenameToSurface(const String &name);
	const Graphics::ManagedSurface *languageToSurface(Common::Language languageCode);
	const Graphics::ManagedSurface *platformToSurface(Common::Platform platformCode);

	/// Update _visibleEntries from _allEntries and returns true if reload is required.
	bool calcVisibleEntries();
	void setEntryList(Common::Array<GridItemInfo> *list);
	void setTitlesVisible(bool vis);

	void reloadThumbnails();
	void loadFlagIcons();
	void loadPlatformIcons();

	void destroyItems();
	void updateGrid();
	void move(int x, int y);
	void assignEntriesToItems();

	int getScrollPos() const { return -_scrollPos; }
	int getSelected() const { return _selectedEntry; }
	int getThumbnailHeight() const { return _thumbnailHeight; }
	int getThumbnailWidth() const { return _thumbnailWidth; }

	void handleMouseWheel(int x, int y, int direction) override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	void reflowLayout() override;

	void openTray(int x, int y, int entryID);
	void scrollBarRecalc();
};

/* GridItemWidget */
class GridItemWidget : public ContainerWidget, public CommandSender {
protected:
	Graphics::ManagedSurface _thumbGfx;

	GridItemInfo 	*_activeEntry;
	GridWidget 		*_grid;
	bool 			isHighlighted;

public:
	GridItemWidget(GridWidget *boss, int x, int y, int w, int h);
	GridItemWidget(GridWidget *boss);

	void move(int x, int y);
	void update();
	void updateThumb();
	void setActiveEntry(GridItemInfo &entry);

	void drawWidget() override;

	void handleMouseWheel(int x, int y, int direction) override;
	void handleMouseEntered(int button) override;
	void handleMouseLeft(int button) override;
	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseMoved(int x, int y, int button) override;
};

} // End of namespace GUI

#endif
