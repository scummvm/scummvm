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

namespace GUI {

class ScrollBarWidget;
class GridItemWidget;
class GridWidget;

const Graphics::ManagedSurface *scaleGfx(Graphics::ManagedSurface *gfx, int w, int h);

// TODO: Add more platforms
enum Platform {
	kPlatformDOS,
	kPlatformAmiga,
	kPlatformApple2,
	kPlatformUnknown = -1
};

// Cut-pasted commands from Launcher to here to make things work
// TODO: Find a way to put them back in Launcher.cpp
enum {
	kStartCmd = 'STRT',
	kEditGameCmd = 'EDTG',
	kLoadGameCmd = 'LOAD',
	kOpenTrayCmd = 'OPTR',
};

/* GridItemInfo */
struct GridItemInfo
{
	typedef Common::String String;

	String 		engineid;
	String 		gameid;
	String 		language;
	String 		title;
	Platform 	platform;
	String 		thumbPath;
	int 		entryID;

	GridItemInfo(int id, const String &eid, const String &gid
		,const String &t, const String &l, const String &p)
		: entryID(id), gameid(gid), engineid(eid), title(t), language(l) {
		
		if (p == "pc")
			platform = kPlatformDOS;
		else if (p == "amiga")
			platform = kPlatformAmiga;
		else if (p == "apple2")
			platform = kPlatformApple2;
		else
			platform = kPlatformUnknown;

		thumbPath = String::format("%s-%s.png", engineid.c_str(), gameid.c_str());
	}
};

/* GridItemTray */
class GridItemTray: public Dialog, public CommandSender {
	int			 	_entryID;
	GuiObject 		*_boss;
public:
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;

	typedef Common::U32String U32String;
	typedef Common::Array<Common::U32String> U32StringArray;

	GridItemTray(GuiObject *boss, int x, int y, int w, int h, int entryID);
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
};


/* GridWidget */
class GridWidget : public ContainerWidget {
public:
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;

	typedef Common::U32String U32String;
	typedef Common::Array<Common::U32String> U32StringArray;

protected:
	Common::Array<const Graphics::ManagedSurface *> _platformIcons;
	
	// Images are mapped by filename -> surface.
	Common::HashMap<String, const Graphics::ManagedSurface *> _loadedSurfaces;

	Common::Array<GridItemWidget *> 	_gridItems;
	Common::Array<GridItemInfo> 		_allEntries;
	Common::Array<GridItemInfo> 		_visibleEntries;

	ScrollBarWidget *_scrollBar;

	int				_scrollWindowHeight;
	int				_scrollWindowWidth;
	int				_scrollSpeed;
	int				_scrollPos;

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
	
	int				_rows;
	int				_itemsPerRow;
	int				_firstVisibleItem;
	int				_itemsOnScreen;
	
	bool 			_isTitlesVisible;

public:
	GridItemInfo 	*_selectedEntry;
	GridItemTray 	*_tray;
	
	GridWidget(GuiObject *boss, int x, int y, int w, int h);
	GridWidget(GuiObject *boss, const String &name);

	const Graphics::ManagedSurface *filenameToSurface(const String &name);
	const Graphics::ManagedSurface *languageToSurface(const String &lang);
	const Graphics::ManagedSurface *platformToSurface(Platform platformCode);

	/// Update _visibleEntries from _allEntries and returns true if reload is required.
	bool calcVisibleEntries();
	void setEntryList(Common::Array<GridItemInfo> *list);
	
	void reloadThumbnails();
	void loadFlagIcons();
	void loadPlatformIcons();
	
	void destroyItems();
	void updateGrid();

	int getThumbnailHeight() const { return _thumbnailHeight; }
	int getThumbnailWidth() const { return _thumbnailWidth; }

	void handleMouseWheel(int x, int y, int direction) override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	
	void reflowLayout() override;
	
	void openTray(int x, int y, int w, int h, int entryID);
	void scrollBarRecalc();
};

/* GridItemWidget */
class GridItemWidget : public ContainerWidget, public CommandSender {
public:
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;

	typedef Common::U32String U32String;
	typedef Common::Array<Common::U32String> U32StringArray;

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
};

} // End of namespace GUI

#endif
