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

#include "common/system.h"
#include "gui/gui-manager.h"
#include "gui/widgets/grid.h"

#include "gui/ThemeEval.h"

namespace GUI {

GridItemWidget::GridItemWidget(GridWidget *boss, int x, int y, int w, int h)
	: ContainerWidget(boss, x, y, w, h) {

	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_activeEntry = nullptr;
	_grid = boss;
	isHighlighted = false;
}
GridItemWidget::GridItemWidget(GridWidget *boss) :
			GridItemWidget(boss, 0, 0, 0, 0) {}

void GridItemWidget::setActiveEntry(GridItemInfo &entry) {
	_activeEntry = &entry;
}

void GridItemWidget::updateThumb() {
	// TODO: Placeholder image for missing thumb
	const Graphics::ManagedSurface *gfx = _grid->filenameToSurface(_activeEntry->thumbPath);
	_thumbGfx.free();
	if (gfx)
		_thumbGfx.copyFrom(*gfx);
}

void GridItemWidget::update() {
	if (_activeEntry) {
		updateThumb();
		markAsDirty();
	}
}

void GridItemWidget::move(int x, int y) {
	Widget::setPos(getRelX() + x, getRelY() + y);
}

void GridItemWidget::drawWidget() {
	int thumbHeight = _grid->getThumbnailHeight();
	int thumbWidth = _grid->getThumbnailWidth();

	if (isHighlighted) {
		// Draw a highlighted BG on hover
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x - 10, _y - 10, _x + _w + 10, _y + _h), 
										ThemeEngine::WidgetBackground::kGridItemHighlight);
	} else {
		// Draw a BG of the same color as grid area
		// when it is not highlighted to cover up 
		// the highlight shadow
		// FIXME: Find a way to redraw the area around the widget
		//		 instead of just drawing a cover-up
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x - 20, _y - 10, _x + _w + 20, _y + _h + 10), 
										ThemeEngine::WidgetBackground::kGridItemBackground);
	}

	// Draw Thumbnail Background
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _grid->getThumbnailWidth(), _y + thumbHeight), 
										ThemeEngine::WidgetBackground::kThumbnailBackground);
	
	// Draw Thumbnail
	g_gui.theme()->drawSurface(Common::Point(_x, _y), _thumbGfx, true);

	// Draw Platform Icon
	if (_activeEntry->platform != kPlatformUnknown) {
		const Graphics::ManagedSurface *platGfx = _grid->platformToSurface(_activeEntry->platform);
		g_gui.theme()->drawSurface(Common::Point(_x + thumbWidth - 32, _y + thumbHeight - 32), 
									*platGfx, true);
	}

	// Draw Flag
	const Graphics::ManagedSurface *flagGfx = _grid->languageToSurface(_activeEntry->language);
	if (flagGfx)
		g_gui.theme()->drawSurface(Common::Point(_x + thumbWidth - flagGfx -> w - 5, _y + 5), 
									*flagGfx, true);

	// Draw Title
	g_gui.theme()->drawText(Common::Rect(_x, _y + thumbHeight, _x + thumbWidth, _y + thumbHeight + 32),
							_activeEntry->title, GUI::ThemeEngine::kStateEnabled ,Graphics::kTextAlignLeft);
}

void GridItemWidget::handleMouseWheel(int x, int y, int direction) {
	_grid->handleMouseWheel(x, y, direction);
}

void GridItemWidget::handleMouseEntered(int button) {
	if (!isHighlighted) {
		_grid->_selectedEntry = _activeEntry;
		isHighlighted = true;
		markAsDirty();
	}
}

void GridItemWidget::handleMouseLeft(int button) {
	if (isHighlighted) {
		_grid->_selectedEntry = nullptr;
		isHighlighted = false;
		markAsDirty();
	}
}

void GridItemWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isHighlighted) {
		// Work in progress
		Dialog *tray = new GridItemTray(getAbsX() - 10, getAbsY() + _h, _w + 20, 60, _activeEntry->entryID, _grid);
		
		int buttonWidth = tray->getWidth() / 4;
		int buttonHeight = tray->getHeight() / 2;
		
		PicButtonWidget *playButton = new PicButtonWidget(tray, (buttonWidth / 4), buttonHeight / 2, buttonWidth, buttonHeight, U32String("Play"), kStartCmd);
		PicButtonWidget *loadButton = new PicButtonWidget(tray, buttonWidth + 2*(buttonWidth / 4), buttonHeight / 2, buttonWidth, buttonHeight, U32String("Saves"), kLoadGameCmd);
		PicButtonWidget *editButton = new PicButtonWidget(tray, 2*buttonWidth + 3*(buttonWidth / 4), buttonHeight / 2, buttonWidth, buttonHeight, U32String("Edit"), kEditGameCmd);
	
		playButton->setGfxFromTheme("button_play.bmp");
		loadButton->setGfxFromTheme("button_load.bmp");
		editButton->setGfxFromTheme("button_options.bmp");
		// playButton->markAsDirty();
		tray->runModal();
	}
}

#pragma mark -

void GridItemTray::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd)
	{
	case kStartCmd:
		ConfMan.setActiveDomain(entryID);
		close();
		break;
	default:
		break;
	}
}

#pragma mark -

Graphics::ManagedSurface *loadSurfaceFromFile(Common::String &name) {
	Graphics::ManagedSurface *surf = nullptr;
	const Graphics::Surface *srcSurface = nullptr;
	if (name.hasSuffix(".png")) {
#ifdef USE_PNG
		Image::PNGDecoder decoder;
		Common::FSNode fileNode(name);
		Common::SeekableReadStream *stream = fileNode.createReadStream();
		if (stream) {
			if (!decoder.loadStream(*stream))
				warning("Error decoding PNG");
			
			srcSurface = decoder.getSurface();
			delete stream;
			if (!srcSurface) {
				warning("Failed to load surface : %s", name.c_str());
			}
			if (srcSurface && srcSurface->format.bytesPerPixel != 1) {
				surf = new Graphics::ManagedSurface(srcSurface->convertTo(g_system->getOverlayFormat()));
			}
				
		} else {
			warning("No such file : %s", name.c_str());
		}
#else
		error("No PNG support compiled");
#endif
	} else if (name.hasSuffix(".svg")) {
		Graphics::SVGBitmap *image = nullptr;
		Common::FSNode fileNode(name);
		Common::SeekableReadStream *stream = fileNode.createReadStream();
		if (stream) {
			image = new Graphics::SVGBitmap(stream);
			surf = new Graphics::ManagedSurface(60, 30, *image->getPixelFormat());
			image->render(*surf, 60, 30);
			delete image;
		}
	}
	return surf;
}

#pragma mark -

GridWidget::GridWidget(GuiObject *boss, int x, int y, int w, int h)
	: ContainerWidget(boss, x, y, w, h) {
	
	loadPlatformIcons();
	loadFlagIcons();

	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	_minGridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");
	
	_gridItemHeight = _thumbnailHeight + (2 * kLineHeight);
	_gridItemWidth = _thumbnailWidth;
	
	_scrollBar = new ScrollBarWidget(this, 0, 0, 20, 200);
	_scrollBar->setTarget(this);
	_scrollPos = 0;
	_firstVisibleItem = 0;
}

GridWidget::GridWidget(GuiObject *boss, const String &name)
	: ContainerWidget(boss, name) {
	
	loadPlatformIcons();
	loadFlagIcons();

	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	_minGridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");
	
	_gridItemHeight = _thumbnailHeight + (2 * kLineHeight);
	_gridItemWidth = _thumbnailWidth;
	
	_scrollBar = new ScrollBarWidget(this, 0, 0, 20, 200);
	_scrollBar->setTarget(this);
	_scrollPos = 0;
	_firstVisibleItem = 0;
}

const Graphics::ManagedSurface *GridWidget::filenameToSurface(const String &name) {
	String path = String("./icons/")+name;
	
	for (auto l = _visibleEntries.begin(); l!=_visibleEntries.end(); ++l) {
		if (l->thumbPath == name) {
			return _loadedSurfaces[path];
		}
	}
	return nullptr;
}

const Graphics::ManagedSurface *GridWidget::languageToSurface(const String &lang) {
	String path = String::format("./icons/%s.svg", lang.c_str());
	return _loadedSurfaces[path];
}

const Graphics::ManagedSurface *GridWidget::platformToSurface(Platform platformCode) {
	if ((platformCode == kPlatformUnknown) || (platformCode < 0 || platformCode >= _platformIcons.size())) {
		warning("Unknown Platform");
		return nullptr;
	}
	return _platformIcons[platformCode];
}

void GridWidget::setEntryList(Common::Array<GridItemInfo> *list) {
	for (auto entryIter = list->begin(); entryIter != list->end(); ++entryIter) {
		_allEntries.push_back(*entryIter);
	}
}

bool GridWidget::calcVisibleEntries() {
	bool needsReload = false;

	int nItemsOnScreen = 0;

	nItemsOnScreen = (3 + (_scrollWindowHeight / (_gridItemHeight + _gridYSpacing))) * (_itemsPerRow);

	needsReload = true;
	_itemsOnScreen = nItemsOnScreen;

	int toRender = MIN(_firstVisibleItem + _itemsOnScreen, (int)_allEntries.size()-1);

	_visibleEntries.clear();
	for (int ind = _firstVisibleItem; ind < toRender; ++ind) {
		GridItemInfo *iter = _allEntries.begin() + ind;
		_visibleEntries.push_back(*iter);
	}
	return needsReload;
}

void GridWidget::reloadThumbnails() {
	Graphics::ManagedSurface *surf = nullptr;
	String gameid;
	String engineid;
	String path;
	
	for (Common::Array<GridItemInfo>::iterator iter = _visibleEntries.begin(); iter != _visibleEntries.end(); ++iter) {
		path = String("./icons/")+iter->thumbPath;
		if (_loadedSurfaces.contains(path)) {
			// warning("Thumbnail already loaded, skipping...");
		} else {
			surf = loadSurfaceFromFile(path);
			if (surf) {
				const Graphics::ManagedSurface *scSurf(scaleGfx(surf, _thumbnailWidth, 512));
				_loadedSurfaces[path] = scSurf;
			}
		}
	}
}

void GridWidget::loadFlagIcons() {
	String pathPrefix("./icons/");
	StringArray iconFilenames;
	
	// TODO: Can we read the language list from language.cpp?
	iconFilenames.push_back(String("en.svg"));
	iconFilenames.push_back(String("en-gb.svg"));
	iconFilenames.push_back(String("en-us.svg"));
	iconFilenames.push_back(String("it.svg"));
	iconFilenames.push_back(String("fr.svg"));

	for (auto i = iconFilenames.begin(); i != iconFilenames.end(); ++i) {
		String fullPath = pathPrefix + (*i);
		Graphics::ManagedSurface *gfx = loadSurfaceFromFile(fullPath);
		if (gfx) {
			const Graphics::ManagedSurface *scGfx = scaleGfx(gfx, 32, 32);
			_loadedSurfaces[fullPath] = scGfx;
			gfx->free();
			delete gfx;
		}
	}
}

void GridWidget::loadPlatformIcons() {
	for (auto iter = _platformIcons.begin(); iter != _platformIcons.end(); ++iter) {
		delete *iter;
	}
	_platformIcons.clear();
	String pathPrefix("./icons/");
	StringArray iconFilenames;
	
	// TODO: Can we make a list of all platforms?
	iconFilenames.push_back(String("dos.png"));
	iconFilenames.push_back(String("amiga.png"));
	iconFilenames.push_back(String("apple2.png"));

	for (auto i = iconFilenames.begin(); i != iconFilenames.end(); ++i) {
		String fullPath = pathPrefix + (*i);
		Graphics::ManagedSurface *gfx = loadSurfaceFromFile(fullPath);
		if (gfx) {
			const Graphics::ManagedSurface *scGfx = scaleGfx(gfx, 32, 32);
			_platformIcons.push_back(scGfx);
			_loadedSurfaces[fullPath] = scGfx;
			gfx->free();
			delete gfx;
		}
	}
}

void GridWidget::destroyItems() {
	for (Common::Array<GridItemWidget *>::iterator i = _gridItems.begin(), end = _gridItems.end(); i != end; ++i) {
		removeWidget((*i));
		delete (*i);
	}

	_gridItems.clear();
}

void GridWidget::updateGrid() {
	for (auto i = _gridItems.begin(); i != _gridItems.end(); ++i) {
		(*i)->update();
	}
}

void GridWidget::handleMouseWheel(int x, int y, int direction) {
	int scrollSpeed = -direction*40;
	bool needsReload = false;

	_scrollPos += scrollSpeed;
	
	if (_scrollPos > 0) {
		_scrollPos = 0;
		scrollSpeed = 0;
	}
	// This lets user scroll a bit more after the last item
	// comes into the view, which messes with the scrollbar.
	// TODO: Make it so that scrolling doesn't go past last item.
	if (_scrollPos < -(_innerHeight - _scrollWindowHeight)) {
		_scrollPos = -(_innerHeight - _scrollWindowHeight);
		scrollSpeed = 0;
	}

	for (auto it = _gridItems.begin(); it != _gridItems.end(); ++it) {
		GridItemWidget *item = *it;
		item->move(0, scrollSpeed);
	}

	if (_gridItems.front()->getRelY() < -2 * (_gridItemHeight + _gridYSpacing)) {
		needsReload = true;
		_firstVisibleItem += _itemsPerRow;
		
		// Move the entire grid one row down
		for (auto it = _gridItems.begin(); it != _gridItems.end(); ++it) {
			GridItemWidget *item = *it;
			item->move(0, _gridItemHeight + _gridYSpacing);
		}
	
	} else if (_gridItems.front()->getRelY() > 50 - (_gridItemHeight + _gridYSpacing)) {
		needsReload = true;
		_firstVisibleItem -= _itemsPerRow;
		
		// Move the entire grid one row up
		for (auto it = _gridItems.begin(); it != _gridItems.end(); ++it) {
			GridItemWidget *item = *it;
			item->move(0, -(_gridItemHeight + _gridYSpacing));
		}
	}

	if (needsReload) {
		calcVisibleEntries();
		reloadThumbnails();
		
		// TODO: Refactor this into a function
		// Assign entries from _visibleEntries to each GridItem in _gridItems
		auto entry = _visibleEntries.begin();
		// Start assigning from the second row as the first row is supposed
		// to be offscreen.
		auto it = _gridItems.begin() + _itemsPerRow;
		
		for (int k = 0; k < _itemsOnScreen; ++k) {
			GridItemWidget *item = *it;
			if (entry != _visibleEntries.end()) {
				// Assign entry and update
				item->setActiveEntry(*entry);
				item->update();
				item->setVisible(true);
				++entry;
			} else {
				// If we run out of visible entries to display.
				// i.e., scrolled to the very bottom, we make items invisible.
				item->setActiveEntry(_visibleEntries.front());
				item->update();
				item->setVisible(false);
			}

			if (++it == _gridItems.end())
				it = _gridItems.begin();
		}
	}

	_scrollBar->_currentPos = _firstVisibleItem / _itemsPerRow;
	_scrollBar->recalc();
	
	markAsDirty();
}

void GridWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	// Work in progress
	switch (cmd) {
	case kSetPositionCmd:
		if ((_firstVisibleItem / _itemsPerRow) != (int)data) {
			markAsDirty();

			((GUI::Dialog *)_boss)->setFocusWidget(this);
		}
		break;
	default:
		break;
	}
}

void GridWidget::reflowLayout() {
	Widget::reflowLayout();
	destroyItems();
	
	_scrollWindowHeight = _h;
	_scrollWindowWidth = _w;
	
	// In all calculations with 50 or 100, we are assuming 50 to be
	// the Hor / Vert padding of contents inside the grid.
	// TODO: Expose this padding via themes.
	_itemsPerRow = MAX(((_scrollWindowWidth - 100) / (_gridItemWidth + _minGridXSpacing)), 1);
	_gridXSpacing = MAX(((_scrollWindowWidth - 50) - (_itemsPerRow * _gridItemWidth)) / _itemsPerRow, _minGridXSpacing);
	
	_rows = ceil(_allEntries.size() / (float)_itemsPerRow);
	
	_innerHeight = 100 + (_rows * (_gridItemHeight + _gridYSpacing));
	_innerWidth = 100 + (_itemsPerRow * (_gridItemWidth + _gridXSpacing));

	if (_scrollPos < -(_innerHeight - _scrollWindowHeight))
		_scrollPos = -(_innerHeight - _scrollWindowHeight);

	int row = 0;
	int col = 0;

	_firstVisibleItem = _itemsPerRow * ((int)ceil(-_scrollPos / (float)(_gridItemHeight + _gridYSpacing)) - 1);
	_firstVisibleItem = (_firstVisibleItem < 0) ? 0 : _firstVisibleItem;

	_scrollBar->setSize(20, _h);
	_scrollBar->setPos(_w - _scrollBar->getWidth(), 0);

	if (calcVisibleEntries()) {
		reloadThumbnails();
	}

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *newItem = new GridItemWidget(this, 
									50 + col * (_gridItemWidth + _gridXSpacing), 
									(_scrollPos % (_gridItemHeight + _gridYSpacing)) + 50 + (row - 1) * (_gridItemHeight + _gridYSpacing), 
									_gridItemWidth, 
									_gridItemHeight);
		
		_gridItems.push_back(newItem);

		if (++col >= _itemsPerRow) {
			++row;
			col = 0;
		}
	}

	// This assignment method is same while scrolling or resizing
	// TODO: Refactor this into a function

	auto entry = _visibleEntries.begin();
	auto it = _gridItems.begin() + _itemsPerRow;

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *item = *it;
		if (entry != _visibleEntries.end()) {
			item->setActiveEntry(*entry);
			item->update();
			item->setVisible(true);
			++entry;
		} else {
			item->setActiveEntry(_visibleEntries.front());
			item->update();
			item->setVisible(false);
		}
		if (++it == _gridItems.end())
			it = _gridItems.begin();
	}

	scrollBarRecalc();
	markAsDirty();
}

void GridWidget::scrollBarRecalc() {
	// HACK: Our grid scrolls some distance past the last element.
	// 	2 is an experimental value and might not work on all window sizes.
	_scrollBar->_numEntries = _rows + 2;
	_scrollBar->_entriesPerPage = _itemsOnScreen / _itemsPerRow;
	_scrollBar->_currentPos = _firstVisibleItem / _itemsPerRow;
	_scrollBar->recalc();
}

} // End of namespace GUI
