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
#include "common/file.h"
#include "common/language.h"

#include "gui/gui-manager.h"
#include "gui/widgets/grid.h"

#include "gui/ThemeEval.h"

namespace GUI {

GridItemWidget::GridItemWidget(GridWidget *boss, int x, int y, int w, int h)
	: ContainerWidget(boss, x, y, w, h), CommandSender(boss) {

	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_activeEntry = nullptr;
	_grid = boss;
	isHighlighted = false;
}
GridItemWidget::GridItemWidget(GridWidget *boss)
	: GridItemWidget(boss, 0, 0, 0, 0) {}

void GridItemWidget::setActiveEntry(GridItemInfo &entry) {
	_activeEntry = &entry;
}

void GridItemWidget::updateThumb() {
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
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x - (_grid->_gridXSpacing / 3), _y - (_grid->_gridYSpacing / 3),
											_x + _w + (_grid->_gridXSpacing / 3), _y + _h + (_grid->_gridYSpacing / 3)), 
										ThemeEngine::WidgetBackground::kGridItemHighlight);
	} else {
		// Draw a BG of the same color as grid area
		// when it is not highlighted to cover up 
		// the highlight shadow
		// FIXME: Find a way to redraw the area around the widget
		//		 instead of just drawing a cover-up
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x - 2 * (_grid->_gridXSpacing / 3), _y - 2 * (_grid->_gridYSpacing / 3),
											_x + _w + 2 * (_grid->_gridXSpacing / 3), _y + _h + 2 * (_grid->_gridYSpacing / 3)), 
										ThemeEngine::WidgetBackground::kGridItemBackground);
	}

	// Draw Thumbnail Background
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _grid->getThumbnailWidth(), _y + thumbHeight), 
										ThemeEngine::WidgetBackground::kThumbnailBackground);
	
	// Draw Thumbnail
	if (_thumbGfx.empty()) {
		// Draw Title when thumbnail is missing
		g_gui.theme()->drawText(Common::Rect(_x, _y, _x + thumbWidth, _y + thumbHeight),
							_activeEntry->title, GUI::ThemeEngine::kStateEnabled ,Graphics::kTextAlignCenter,
							ThemeEngine::kTextInversion, 0, true, ThemeEngine::kFontStyleBold,
							ThemeEngine::kFontColorAlternate, false);
	}
	else g_gui.theme()->drawSurface(Common::Point(_x, _y), _thumbGfx, true);

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
	if (_grid->_isTitlesVisible)
		g_gui.theme()->drawText(Common::Rect(_x, _y + thumbHeight, _x + thumbWidth, _y + thumbHeight + kLineHeight),
								_activeEntry->title, GUI::ThemeEngine::kStateEnabled ,Graphics::kTextAlignLeft);
}

void GridItemWidget::handleMouseWheel(int x, int y, int direction) {
	_grid->handleMouseWheel(x, y, direction);
	_grid->_selectedEntry = nullptr;
	isHighlighted = false;
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

void GridItemWidget::handleMouseMoved(int x, int y, int button) {
	if (!isHighlighted) {
		handleMouseEntered(button);
	}
}

void GridItemWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isHighlighted) {
		// Work in progress
		_grid->openTray(getAbsX() - _grid->_gridXSpacing / 3, getAbsY() + _h, _w + 2 * (_grid->_gridXSpacing / 3), _h / 3, _activeEntry->entryID);
		_grid->_tray->runModal();
	}
}

#pragma mark -

GridItemTray::GridItemTray(GuiObject *boss, int x, int y, int w, int h, int entryID)
	: Dialog(x, y, w, h), CommandSender(boss) {
		
	_entryID = entryID;
	_boss = boss;
	
	int buttonWidth = w / 3;
	int buttonHeight = h / 3;

	PicButtonWidget *playButton = new PicButtonWidget(this, (buttonWidth / 3), buttonHeight / 3, (buttonWidth / 3) + buttonWidth * 2, buttonHeight, U32String("Play"), kStartCmd);
	PicButtonWidget *loadButton = new PicButtonWidget(this, (buttonWidth / 3), (buttonHeight * 5) / 3, buttonWidth, buttonHeight, U32String("Saves"), kLoadGameCmd);
	PicButtonWidget *editButton = new PicButtonWidget(this, buttonWidth + 2 * (buttonWidth / 3), (buttonHeight * 5) / 3, buttonWidth, buttonHeight, U32String("Edit"), kEditGameCmd);

	playButton->setGfxFromTheme("button_play.bmp", 0, false);
	loadButton->setGfxFromTheme("button_load.bmp", 0, false);
	editButton->setGfxFromTheme("button_options.bmp", 0, false);
}

void GridItemTray::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd)
	{
	case kStartCmd:
		close();
		sendCommand(kStartCmd, _entryID);
		break;
	case kLoadGameCmd:
		close();
		sendCommand(kLoadGameCmd, _entryID);
		break;
	case kEditGameCmd:
		close();
		sendCommand(kEditGameCmd, _entryID);
		break;
	case kCloseCmd:
		close();
		break;
	default:
		break;
	}
}

void GridItemTray::handleMouseDown(int x, int y, int button, int clickCount) {
	Dialog::handleMouseDown(x, y, button, clickCount);
	// HACK: Tray height is determined relative to grid item height
	// hence we know the grid item height from within the tray.
	// We might want to make tray height independent of grid item height.
	if ((x < 0 || x > _w) || (y > _h || y < -(_h * 3))) {
		// Close on clicking outside
		close();
	} else if (y < 0 && clickCount >= 2) {
		// Run on double clicking thumbnail
		close();
		sendCommand(kStartCmd, _entryID);
	}
}

void GridItemTray::handleMouseWheel(int x, int y, int direction) {
	close();
}

#pragma mark -

Graphics::ManagedSurface *loadSurfaceFromFile(const Common::String &name) {
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

GridWidget::~GridWidget() {
	_platformIcons.clear();
	_loadedSurfaces.clear();
	_gridItems.clear();
	_allEntries.clear();
	_visibleEntries.clear();
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
	if ((platformCode == kPlatformUnknown) || (platformCode < 0 || platformCode >= (int)_platformIcons.size())) {
		warning("Unknown Platform");
		return nullptr;
	}
	return _platformIcons[platformCode];
}

void GridWidget::setEntryList(Common::Array<GridItemInfo> *list) {
	_allEntries.clear();
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

void GridWidget::setTitlesVisible(bool vis) {
	_isTitlesVisible = vis;
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
				surf->free();
				delete surf;
			}
			else {
				_loadedSurfaces[path] = nullptr;
			}
		}
	}
}

void GridWidget::loadFlagIcons() {
	const Common::LanguageDescription *l =Common::g_languages;
	for (; l->code; ++l) {
		String path = String::format("./icons/%s.svg", l->code);
		Graphics::ManagedSurface *gfx = loadSurfaceFromFile(path);
		if (gfx) {
			const Graphics::ManagedSurface *scGfx = scaleGfx(gfx, 32, 32);
			_loadedSurfaces[path] = scGfx;
			gfx->free();
			delete gfx;
		}
		else {
			_loadedSurfaces[path] = nullptr;
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

void GridWidget::move(int x, int y) {
	for (auto i = _gridItems.begin(); i != _gridItems.end(); ++i) {
		(*i)->move(x, y);
	}
}

void GridWidget::updateGrid() {
	for (auto i = _gridItems.begin(); i != _gridItems.end(); ++i) {
		(*i)->update();
	}
}

void GridWidget::assignEntriesToItems() {
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
			// e.g., scrolled to the very bottom, we make items invisible.
			item->setActiveEntry(_visibleEntries.front());
			item->update();
			item->setVisible(false);
		}

		if (++it == _gridItems.end())
			it = _gridItems.begin();
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
		move(0, _gridItemHeight + _gridYSpacing);
	
	} else if (_gridItems.front()->getRelY() > -_gridItemHeight) {
		needsReload = true;
		_firstVisibleItem -= _itemsPerRow;
		
		// Move the entire grid one row up
		move(0, -(_gridItemHeight + _gridYSpacing));
	}

	if (needsReload) {
		calcVisibleEntries();
		reloadThumbnails();
		assignEntriesToItems();
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
			_scrollPos = -data * (_gridItemHeight + _gridYSpacing);
			_firstVisibleItem = data * _itemsPerRow;
			if (calcVisibleEntries()) {
				reloadThumbnails();
			}
			
			int row = 0;
			int col = 0;

			for (auto it = _gridItems.begin(); it != _gridItems.end(); ++it) {
				(*it)->setPos(_minGridXSpacing + col * (_gridItemWidth + _gridXSpacing), 
							(_scrollPos % (_gridItemHeight + _gridYSpacing)) + _gridYSpacing + (row - 1) * (_gridItemHeight + _gridYSpacing));
				if (++col >= _itemsPerRow) {
					++row;
					col = 0;
				}
			}

			assignEntriesToItems();
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
	
	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	_minGridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");

	_isTitlesVisible = g_gui.xmlEval()->getVar("Globals.Grid.ShowTitles");

	_scrollBarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	
	_gridItemHeight = _thumbnailHeight + (2 * kLineHeight * _isTitlesVisible);
	_gridItemWidth = _thumbnailWidth;

	_itemsPerRow = MAX(((_scrollWindowWidth - (2 * _minGridXSpacing) - _scrollBarWidth) / (_gridItemWidth + _minGridXSpacing)), 1);
	_gridXSpacing = MAX(((_scrollWindowWidth - (2 * _minGridXSpacing) - _scrollBarWidth) - (_itemsPerRow * _gridItemWidth)) / _itemsPerRow, _minGridXSpacing);
	
	_rows = ceil(_allEntries.size() / (float)_itemsPerRow);
	
	_innerHeight = _gridYSpacing + (_rows * (_gridItemHeight + _gridYSpacing));
	_innerWidth = (2 * _minGridXSpacing) + (_itemsPerRow * (_gridItemWidth + _gridXSpacing));

	if (_scrollPos < -(_innerHeight - _scrollWindowHeight))
		_scrollPos = -(_innerHeight - _scrollWindowHeight);

	int row = 0;
	int col = 0;

	_firstVisibleItem = _itemsPerRow * ((int)ceil(-_scrollPos / (float)(_gridItemHeight + _gridYSpacing)) - 1);
	_firstVisibleItem = (_firstVisibleItem < 0) ? 0 : _firstVisibleItem;

	_scrollBar->resize(_w - _scrollBarWidth, 0, _scrollBarWidth, _h, false);

	if (calcVisibleEntries()) {
		reloadThumbnails();
	}

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *newItem = new GridItemWidget(this, 
									_minGridXSpacing + col * (_gridItemWidth + _gridXSpacing), 
									(_scrollPos % (_gridItemHeight + _gridYSpacing)) + _gridYSpacing + (row - 1) * (_gridItemHeight + _gridYSpacing), 
									_gridItemWidth, 
									_gridItemHeight);
		
		_gridItems.push_back(newItem);

		if (++col >= _itemsPerRow) {
			++row;
			col = 0;
		}
	}

	assignEntriesToItems();
	scrollBarRecalc();
	markAsDirty();
}

void GridWidget::openTray(int x, int y, int w, int h, int entryId) {
	_tray = new GridItemTray(_boss, x, y, w, h, entryId);
}

void GridWidget::scrollBarRecalc() {
	int old_pos = _scrollBar->_currentPos;
	
	_scrollBar->_numEntries = _rows;
	_scrollBar->_entriesPerPage = _scrollWindowHeight / (_gridItemHeight + _gridYSpacing);
	_scrollBar->_currentPos = _firstVisibleItem / _itemsPerRow;
	
	_scrollBar->checkBounds(old_pos);
	_scrollBar->recalc();
}

} // End of namespace GUI
