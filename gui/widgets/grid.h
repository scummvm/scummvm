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

#ifndef GUI_WIDGETS_GRID_H
#define GUI_WIDGETS_GRID_H

#include "gui/dialog.h"
#include "gui/widgets/scrollbar.h"
#include "common/str.h"

#include "image/bmp.h"
#include "image/png.h"
#include "graphics/svg.h"

namespace GUI {

class ScrollBarWidget;
class GridItemWidget;
class GridWidget;

enum {
	kPlayButtonCmd = 'PLAY',
	kEditButtonCmd = 'EDIT',
	kLoadButtonCmd = 'LOAD',
	kOpenTrayCmd = 'OPTR',
	kItemClicked = 'LBX1',
	kItemDoubleClickedCmd = 'LBX2',
	kItemSizeCmd = 'SIZE'
};

/* GridItemInfo */
struct GridItemInfo {
	bool		isHeader;
	int 		entryID;
	Common::String 		engineid;
	Common::String 		gameid;
	Common::String 		title;
	Common::String		description;
	Common::String		extra;
	Common::String 		thumbPath;
	// Generic attribute value, may be any piece of metadata
	Common::String		attribute;
	Common::Language	language;
	Common::Platform 	platform;

	int32				x, y, w, h;

	GridItemInfo(int id, const Common::String &eid, const Common::String &gid, const Common::String &t,
		const Common::String &d, const Common::String &e, Common::Language l, Common::Platform p)
		: entryID(id), gameid(gid), engineid(eid), title(t), description(d), extra(e), language(l), platform(p), isHeader(false) {
		thumbPath = Common::String::format("icons/%s-%s.png", engineid.c_str(), gameid.c_str());
	}

	GridItemInfo(const Common::String &groupHeader, int groupID) : title(groupHeader), description(groupHeader),
		isHeader(true), entryID(groupID), language(Common::UNK_LANG), platform(Common::kPlatformUnknown) {
		thumbPath = Common::String("");
	}
};

/* GridItemTray */
class GridItemTray: public Dialog, public CommandSender {
	int				_entryID;
	GridWidget		*_grid;
	GuiObject		*_boss;
	PicButtonWidget	*_playButton;
	PicButtonWidget	*_loadButton;
	PicButtonWidget	*_editButton;
public:
	GridItemTray(GuiObject *boss, int x, int y, int w, int h, int entryID, GridWidget *grid);

	void reflowLayout() override;

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
	Common::HashMap<int, const Graphics::ManagedSurface *> _extraIcons;

	// Images are mapped by filename -> surface.
	Common::HashMap<Common::String, const Graphics::ManagedSurface *> _loadedSurfaces;

	Common::Array<GridItemInfo>			_dataEntryList;
	Common::Array<GridItemInfo>			_headerEntryList;
	Common::Array<GridItemInfo *>		_sortedEntryList;
	Common::Array<GridItemInfo *>		_visibleEntryList;

	Common::String							_groupingAttribute;
	Common::HashMap<Common::U32String, int>	_groupValueIndex;
	Common::Array<bool>						_groupExpanded;
	Common::U32String						_groupHeaderPrefix;
	Common::U32String						_groupHeaderSuffix;
	Common::Array<Common::U32String>		_groupHeaders;
	Common::StringMap							_metadataNames;
	Common::HashMap<int, Common::Array<int> >	_itemsInGroup;

	Common::Array<GridItemWidget *>		_gridItems;

	ScrollBarWidget *_scrollBar;

	int				_scrollBarWidth;
	int				_scrollWindowHeight;
	int				_scrollWindowWidth;
	int				_scrollSpeed;
	int				_scrollPos;
	int				_innerHeight;
	int				_innerWidth;
	int				_thumbnailHeight;
	int				_thumbnailWidth;
	int				_flagIconHeight;
	int				_flagIconWidth;
	int				_platformIconHeight;
	int				_platformIconWidth;
	int				_extraIconHeight;
	int				_extraIconWidth;
	int				_minGridXSpacing;
	int				_minGridYSpacing;
	int				_rows;
	int				_itemsPerRow;
	int				_firstVisibleItem;
	int				_lastVisibleItem;
	bool			_isGridInvalid;

	int				_scrollWindowPaddingX;
	int				_scrollWindowPaddingY;
	int				_gridHeaderHeight;
	int				_gridHeaderWidth;
	int				_trayHeight;

public:
	int				_gridItemHeight;
	int				_gridItemWidth;
	int				_gridXSpacing;
	int				_gridYSpacing;

	bool			_isTitlesVisible;

	GridItemInfo	*_selectedEntry;

	Common::U32String	_filter;

	GridWidget(GuiObject *boss, const Common::String &name);
	~GridWidget();

	template<typename T>
	void unloadSurfaces(Common::HashMap<T, const Graphics::ManagedSurface *> &surfaces);

	const Graphics::ManagedSurface *filenameToSurface(const Common::String &name);
	const Graphics::ManagedSurface *languageToSurface(Common::Language languageCode);
	const Graphics::ManagedSurface *platformToSurface(Common::Platform platformCode);
	const Graphics::ManagedSurface *demoToSurface(const Common::String extraString);

	/// Update _visibleEntries from _allEntries and returns true if reload is required.
	bool calcVisibleEntries();
	void setEntryList(Common::Array<GridItemInfo> *list);
	void setAttributeValues(const Common::Array<Common::U32String> &attrs);
	void setMetadataNames(const Common::StringMap &metadata);
	void setTitlesVisible(bool vis);
	void markGridAsInvalid() { _isGridInvalid = true; }
	void setGroupHeaderFormat(const Common::U32String &prefix, const Common::U32String &suffix);

	void groupEntries();
	void sortGroups();
	bool groupExpanded(int groupID) { return _groupExpanded[groupID]; }
	void toggleGroup(int groupID);

	void reloadThumbnails();
	void loadFlagIcons();
	void loadPlatformIcons();
	void loadExtraIcons();

	void destroyItems();
	void calcInnerHeight();
	void calcEntrySizes();
	void updateGrid();
	void move(int x, int y);
	void scrollToEntry(int id, bool forceToTop);
	void assignEntriesToItems();

	int getScrollPos() const { return _scrollPos; }
	int getSelected() const { return ((_selectedEntry == nullptr) ? -1 : _selectedEntry->entryID); }
	int getThumbnailHeight() const { return _thumbnailHeight; }
	int getThumbnailWidth() const { return _thumbnailWidth; }

	void handleMouseWheel(int x, int y, int direction) override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	void reflowLayout() override;

	bool wantsFocus() override { return true; }

	void openTray(int x, int y, int entryID);
	void openTrayAtSelected();
	void scrollBarRecalc();

	void setSelected(int id);
	void setFilter(const Common::U32String &filter);
};

/* GridItemWidget */
class GridItemWidget : public ContainerWidget, public CommandSender {
protected:
	Graphics::ManagedSurface _thumbGfx;

	GridItemInfo	*_activeEntry;
	GridWidget		*_grid;
	bool			_isHighlighted;

public:
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
