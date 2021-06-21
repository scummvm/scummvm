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

GridItemWidget::GridItemWidget(GridWidget *boss, int x, int y, int w, int h) :
ContainerWidget(boss, x, y, w, h) {
	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_activeEntry = nullptr;
	_grid = boss;
	isHighlighted = false;
}
GridItemWidget::GridItemWidget(GridWidget *boss) :
			GridItemWidget(boss, 0, 0, 0, 0) {}

void GridItemWidget::attachEntry(Common::String key, Common::String description, Common::ConfigManager::Domain *domain) {
	Common::String gameid = domain->getVal("gameid");
	Common::String engineid = domain->getVal("engineid");
	Common::String language = "XX";
	Common::String platform = "UNK";
	domain->tryGetVal("language",language);
	domain->tryGetVal("platform", platform);
	_attachedEntries.push_back(GridItemInfo(key, gameid, engineid, description, language, platform));
}

void GridItemWidget::attachEntry(GridItemInfo &entry) {
	_attachedEntries.push_back(entry);
}

void GridItemWidget::attachEntries(Common::Array<GridItemInfo> entries) {
	_attachedEntries.push_back(entries);
}

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
	if ((!_activeEntry) && (!_attachedEntries.empty())) {
		_activeEntry = _attachedEntries.begin(); 
	}

	updateThumb();
	markAsDirty();
}

void GridItemWidget::drawWidget() {
	if (isHighlighted)
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x - 10, _y - 10, _x + _w + 10, _y + _h), 
										ThemeEngine::WidgetBackground::kGridItemHighlight);
	else
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x - 20, _y - 10, _x + _w + 20, _y + _h + 10), 
										ThemeEngine::WidgetBackground::kGridItemBackground);
	
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + kThumbnailWidth, _y + kThumbnailHeight), 
										ThemeEngine::WidgetBackground::kThumbnailBackground);
	
	g_gui.theme()->drawSurface(Common::Point(_x, _y), 
								_thumbGfx, true);

	if (_activeEntry->platform != kPlatformUnknown) {
		const Graphics::ManagedSurface *platGfx = _grid->platformToSurface(_activeEntry->platform);
		g_gui.theme()->drawSurface(Common::Point(_x + kThumbnailWidth - 32, _y + kThumbnailHeight - 32), 
									*platGfx, true);
	}

	const Graphics::ManagedSurface *flagGfx = _grid->languageToSurface(_activeEntry->language);
	if (flagGfx)
		g_gui.theme()->drawSurface(Common::Point(_x + kThumbnailWidth - flagGfx -> w - 5, _y + 5), 
									*flagGfx, true);

	g_gui.theme()->drawText(Common::Rect(_x, _y + kThumbnailHeight, _x + kThumbnailWidth, _y + kThumbnailHeight + 32),
							_activeEntry->title, GUI::ThemeEngine::kStateEnabled ,Graphics::kTextAlignLeft);
}

void GridItemWidget::handleMouseWheel(int x, int y, int direction) {
	_grid->handleMouseWheel(x, y, direction);
}

void GridItemWidget::handleMouseEntered(int button) {
	if (!isHighlighted) {
		_grid->selectedEntry = _activeEntry;
		isHighlighted = true;
		markAsDirty();
	}
}

void GridItemWidget::handleMouseLeft(int button) {
	if (isHighlighted) {
		_grid->selectedEntry = nullptr;
		isHighlighted = false;
		markAsDirty();
	}
}

void GridItemWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isHighlighted) {
		Dialog *tray = new GridItemTray(getAbsX() - 10, getAbsY() + _h, _w + 20, 60, _activeEntry->entryID, _grid);
		
		int buttonWidth = tray->getWidth() / 4;
		int buttonHeight = tray->getHeight() / 2;
		
		ButtonWidget *playButton = new ButtonWidget(tray, (buttonWidth / 4), buttonHeight / 2, buttonWidth, buttonHeight, Common::U32String("Play"), Common::U32String(), kStartCmd);
		ButtonWidget *loadButton = new ButtonWidget(tray, buttonWidth + 2*(buttonWidth / 4), buttonHeight / 2, buttonWidth, buttonHeight, Common::U32String("Saves"), Common::U32String(), kLoadGameCmd);
		ButtonWidget *editButton = new ButtonWidget(tray, 2*buttonWidth + 3*(buttonWidth / 4), buttonHeight / 2, buttonWidth, buttonHeight, Common::U32String("Edit"), Common::U32String(), kEditGameCmd);
		// playButton->markAsDirty();
		tray->runModal();
	}
}

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


void GridItemWidget::move(int x, int y) {
	Widget::setPos(getRelX() + x, getRelY() + y);
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
	} 
	else if (name.hasSuffix(".svg")) {
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

void GridWidget::scrollBarRecalc() {
	_scrollBar->_numEntries = _rows + 2;
	_scrollBar->_entriesPerPage = _itemsOnScreen / _itemsPerRow;
	_scrollBar->_currentPos = _firstVisibleItem / _itemsPerRow;
	_scrollBar->recalc();
}

GridWidget::GridWidget(GuiObject *boss, int x, int y, int w, int h) : 
		ContainerWidget(boss, x, y, w, h) {
	loadPlatformIcons();
	loadFlagIcons();
}

GridWidget::GridWidget(GuiObject *boss, const Common::String &name) : 
		ContainerWidget(boss, name) {
	loadPlatformIcons();
	loadFlagIcons();
	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	_minGridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");
	
	_gridItemHeight = _thumbnailHeight + (2*kLineHeight);
	_gridItemWidth = _thumbnailWidth;
	
	_scrollBar = new ScrollBarWidget(this, 0, 0, 20, 200);
	_scrollBar->setTarget(this);

	_scrollPos = 0 * (_gridItemHeight + _gridYSpacing);
	_firstVisibleItem = 0;
}

void GridWidget::setEntryList(Common::Array<GridItemInfo> *list) {
	for (auto entryIter = list->begin(); entryIter != list->end(); ++entryIter) {
		_allEntries.push_back(*entryIter);
	}
}

void GridWidget::destroyItems() {
	for (Common::Array<GridItemWidget *>::iterator i = _gridItems.begin(), end = _gridItems.end(); i != end; ++i) {
		removeWidget((*i));
		delete (*i);
	}

	_gridItems.clear();
}

void GridWidget::loadPlatformIcons() {
	for (auto iter = _platformIcons.begin(); iter != _platformIcons.end(); ++iter) {
		delete *iter;
	}
	_platformIcons.clear();
	Common::String pathPrefix("./icons/");
	Common::Array<Common::String> iconFilenames;
	iconFilenames.push_back(Common::String("dos.png"));
	iconFilenames.push_back(Common::String("amiga.png"));
	iconFilenames.push_back(Common::String("apple2.png"));

	for (auto i = iconFilenames.begin(); i != iconFilenames.end(); ++i) {
		Common::String fullPath = pathPrefix + (*i);
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

void GridWidget::loadFlagIcons() {
	for (auto iter = _flagIcons.begin(); iter != _flagIcons.end(); ++iter) {
		delete *iter;
	}
	_flagIcons.clear();
	Common::String pathPrefix("./icons/");
	Common::Array<Common::String> iconFilenames;
	iconFilenames.push_back(Common::String("en.svg"));
	iconFilenames.push_back(Common::String("en-gb.svg"));
	iconFilenames.push_back(Common::String("en-us.svg"));
	iconFilenames.push_back(Common::String("it.svg"));
	iconFilenames.push_back(Common::String("fr.svg"));

	for (auto i = iconFilenames.begin(); i != iconFilenames.end(); ++i) {
		Common::String fullPath = pathPrefix + (*i);
		Graphics::ManagedSurface *gfx = loadSurfaceFromFile(fullPath);
		if (gfx) {
			const Graphics::ManagedSurface *scGfx = scaleGfx(gfx, 32, 32);
			_flagIcons.push_back(scGfx);
			_loadedSurfaces[fullPath] = scGfx;
			gfx->free();
			delete gfx;
		}
	}
}

bool GridWidget::calcVisibleEntries() {
	bool needsReload = false;

	int nFirstVisibleItem = 0;
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
	Common::String gameid;
	Common::String engineid;
	Common::String path;
	for (Common::Array<GridItemInfo>::iterator iter = _visibleEntries.begin(); iter != _visibleEntries.end(); ++iter) {
		path = Common::String("./icons/")+iter->thumbPath;
		if (_loadedSurfaces.contains(path)) {
			// warning("Thumbnail already loaded, skipping...");
		}
		else {
			surf = loadSurfaceFromFile(path);
			if (surf) {
				const Graphics::ManagedSurface *scSurf(scaleGfx(surf, kThumbnailWidth, 512));
				_loadedSurfaces[path] = scSurf;
			}
		}
	}
}

const Graphics::ManagedSurface *GridWidget::filenameToSurface(Common::String &name) {
	Common::String path = Common::String("./icons/")+name;
	
	for (auto l = _visibleEntries.begin(); l!=_visibleEntries.end(); ++l) {
		if (l->thumbPath == name) {
			return _loadedSurfaces[path];
		}
	}

	return nullptr;
}

const Graphics::ManagedSurface *GridWidget::platformToSurface(Platform platformCode) {
	if ((platformCode == kPlatformUnknown) || (platformCode < 0 || platformCode >= _platformIcons.size())) {
		warning("Unknown Platform");
		return nullptr;
	}
	return _platformIcons[platformCode];
}

const Graphics::ManagedSurface *GridWidget::languageToSurface(Common::String &lang) {
	Common::String path = Common::String::format("./icons/%s.svg", lang.c_str());
	return _loadedSurfaces[path];
}

void GridWidget::handleMouseWheel(int x, int y, int direction) {
	int scrollSpeed = -direction*40;
	bool needsReload = false;

	_scrollPos += scrollSpeed;
	
	if (_scrollPos > 0) {
		_scrollPos = 0;
		scrollSpeed = 0;
	}
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
		for (auto it = _gridItems.begin(); it != _gridItems.end(); ++it) {
			GridItemWidget *item = *it;
			item->move(0, _gridItemHeight + _gridYSpacing);
		}
	}

	else if (_gridItems.front()->getRelY() > 50 - (_gridItemHeight + _gridYSpacing)) {
		needsReload = true;
		_firstVisibleItem -= _itemsPerRow;
		for (auto it = _gridItems.begin(); it != _gridItems.end(); ++it) {
			GridItemWidget *item = *it;
			item->move(0, -(_gridItemHeight + _gridYSpacing));
		}
	}

	if (needsReload) {
		calcVisibleEntries();
		reloadThumbnails();
	}

	if (needsReload) {
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
	}

	_scrollBar->_currentPos = _firstVisibleItem / _itemsPerRow;
	_scrollBar->recalc();
	markAsDirty();
}

void GridWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
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
	_itemsPerRow = MAX(((_scrollWindowWidth - 100) / (_gridItemWidth + _minGridXSpacing)), 1);
	
	_gridXSpacing = MAX(((_scrollWindowWidth - 50) - (_itemsPerRow * _gridItemWidth)) / _itemsPerRow, (int)_minGridXSpacing);
	
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

void GridWidget::updateGrid() {
	for (auto i = _gridItems.begin(); i != _gridItems.end(); ++i) {
		(*i)->update();
	}
}

} // End of namespace GUI
