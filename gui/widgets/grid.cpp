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
	_plat = new GraphicsWidget(this, kThumbnailWidth - 32, kThumbnailHeight - 32, 32, 32);
	_lang = new StaticTextWidget(this, kThumbnailWidth - 32, 0, 32, 32, Common::U32String("XX"), Graphics::TextAlign::kTextAlignRight);
	_title = new StaticTextWidget(this, 0, kThumbnailHeight, w , kLineHeight*2, Common::U32String("Title"), Graphics::TextAlign::kTextAlignLeft);
	_thumb = new GraphicsWidget(this, 0, 0 , kThumbnailWidth, kThumbnailHeight);
	_activeEntry = nullptr;
	_grid = boss;
}
GridItemWidget::GridItemWidget(GridWidget *boss, GraphicsWidget *th, GraphicsWidget *p, StaticTextWidget *l, StaticTextWidget *t) :
			ContainerWidget(boss, 0, 0, 0, 0), _thumb(th), _plat(p), _lang(l), _title(t) {
	_activeEntry = nullptr;
	_grid = boss;
			}

void GridItemWidget::attachEntry(Common::String key, Common::String description, Common::ConfigManager::Domain *domain) {
	Common::String gameid = domain->getVal("gameid");
	Common::String engineid = domain->getVal("engineid");
	Common::String language = "XX";
	Common::String platform = "UNK";
	domain->tryGetVal("language",language);
	domain->tryGetVal("platform", platform);
	_attachedEntries.push_back(GridItemInfo(gameid, engineid, description, language, platform));
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
	_thumb->setGfx(gfx);
}

void GridItemWidget::update() {
	if ((!_activeEntry) && (!_attachedEntries.empty())) {
		_activeEntry = _attachedEntries.begin(); 
	}

	updateThumb();

	_lang->setLabel(_activeEntry->language);
	_title->setLabel(_activeEntry->title);
	
	const Graphics::ManagedSurface *gfx;

	if (_activeEntry->platform == "pc")
		gfx = _grid->platformToSurface(kPlatformDOS);
	else if (_activeEntry->platform == "amiga")
		gfx = _grid->platformToSurface(kPlatformAmiga);
	else if (_activeEntry->platform == "apple2")
		gfx = _grid->platformToSurface(kPlatformApple2);
	else
		gfx = _grid->platformToSurface(kPlatformUnknown);

	_plat->setGfx(gfx);

	markAsDirty();
}

void GridItemWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x,_y,_x+kThumbnailWidth,_y+kThumbnailHeight), ThemeEngine::WidgetBackground::kThumbnailBackground);
}

#pragma mark -

Graphics::ManagedSurface *loadSurfaceFromFile(Common::String &name) {
	Graphics::ManagedSurface *surf = nullptr;
	const Graphics::Surface *srcSurface = nullptr;
	if (name.hasSuffix(".png")) {
#ifdef USE_PNG
		Image::PNGDecoder decoder;
		Common::FSNode fileNode(name);
		Common::SeekableReadStream * stream = fileNode.createReadStream();
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
	else {

	}
	return surf;
}

#pragma mark -

GridWidget::GridWidget(GuiObject *boss, int x, int y, int w, int h) : 
		ContainerWidget(boss, x, y, w, h) {
	loadPlatformIcons();
}

GridWidget::GridWidget(GuiObject *boss, const Common::String &name) : 
		ContainerWidget(boss, name) {
	loadPlatformIcons();
	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	_gridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");
	
	_gridItemHeight = _thumbnailHeight + (2*kLineHeight);
	_gridItemWidth = _thumbnailWidth;
	
	_scrollPos = 0;
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
			gfx->free();
			delete gfx;
		}
	}
}

bool GridWidget::calcVisibleEntries() {
	bool needsReload = false;

	int nFirstVisibleItem = 0;
	int nItemsOnScreen = 0;

	nFirstVisibleItem = _itemsPerRow * (-_scrollPos / (_gridItemHeight + _gridYSpacing));
	nItemsOnScreen = (3 + (_scrollWindowHeight / (_gridItemHeight + _gridYSpacing))) * (_itemsPerRow);

	if ((nFirstVisibleItem != _firstVisibleItem) || (nItemsOnScreen != _itemsOnScreen)) {
		needsReload = true;
		_firstVisibleItem = nFirstVisibleItem;
		_itemsOnScreen = nItemsOnScreen;

		int toRender = MIN(_firstVisibleItem + _itemsOnScreen, (int)_allEntries.size()-1);

		_visibleEntries.clear();
		for (int ind = _firstVisibleItem; ind < toRender; ++ind) {
			GridItemInfo *iter = _allEntries.begin() + ind;
			_visibleEntries.push_back(*iter);
		}
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

void GridWidget::handleMouseWheel(int x, int y, int direction) {
	int scrollSpeed = -direction*40;
	
	_scrollPos += scrollSpeed;
	
	if (_scrollPos > 0) {
		_scrollPos = 0;
		scrollSpeed = 0;
	}
	if (_scrollPos < -(_innerHeight - _scrollWindowHeight)) {
		_scrollPos = -(_innerHeight - _scrollWindowHeight);
		scrollSpeed = 0;
	}

	bool needsReload = calcVisibleEntries();
	
	if (needsReload) {
		reloadThumbnails();
	}

	// warning("%d %d", _visibleEntries.size(), _gridItems.size());
	Common::Array<GridItemInfo>::iterator eIter = _visibleEntries.begin();
	Common::Array<GridItemWidget *>::iterator iter = _gridItems.begin() + (_firstVisibleItem % _gridItems.size());

	for (int k = 0; k < _gridItems.size(); ++k) {
		GridItemWidget *it = *iter;
		it->setPos(it->getRelX(), scrollSpeed + it->getRelY());

		if (it->getRelY() <= -_gridItemHeight + 50) {
			it->setVisible(false);
			if (it->getRelY() <= -((_gridItemHeight) * 2)) {
				it->setPos(it->getRelX(), it->getRelY() + ((_itemsOnScreen / _itemsPerRow) * (_gridItemHeight + _gridYSpacing)));
			}
		}
		else if (it->getRelY() >= _h) {
			it->setVisible(false);
			if (it->getRelY() >= -_gridItemHeight + ((_itemsOnScreen / _itemsPerRow) * (_gridItemHeight + _gridYSpacing))) {
				it->setPos(it->getRelX(), it->getRelY() - ((_itemsOnScreen / _itemsPerRow) * (_gridItemHeight + _gridYSpacing)));
			}
		}
		else {
			it->setVisible(true);
		}

		if (eIter != _visibleEntries.end()) {
			it->setActiveEntry(*eIter);
			eIter++;
		}
		else {
			it->setActiveEntry(*_visibleEntries.begin());
			it->setVisible(false);
		}
		iter++;
		if (iter == _gridItems.end())
			iter = _gridItems.begin();
	}

	if (needsReload) {
		updateGrid();
	}

	markAsDirty();
}

void GridWidget::reflowLayout() {
	Widget::reflowLayout();
	destroyItems();
	_scrollWindowHeight = _h;
	_scrollWindowWidth = _w;
	_itemsPerRow = MAX(((_scrollWindowWidth - 50) / (_gridItemWidth + _gridXSpacing)), 1);
	int rows = _allEntries.size() / _itemsPerRow; // change this to be calced using eindow sizes
	
	_innerHeight = 100 + ((rows) * (_gridItemHeight + _gridYSpacing));
	_innerWidth = 100 + (_itemsPerRow * (_gridItemWidth + _gridXSpacing));

	if (_scrollPos < -(_innerHeight - _scrollWindowHeight))
		_scrollPos = -(_innerHeight - _scrollWindowHeight);

	int row = 0;
	int col = 0;

	if (calcVisibleEntries()) {
		reloadThumbnails();
	}

	Common::Array<GridItemInfo>::iterator eIter = _visibleEntries.begin();

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *it = new GridItemWidget(this, 
												50 + col * (_gridItemWidth + _gridXSpacing), 
												(_scrollPos % _scrollWindowHeight) + 50 + row * (_gridItemHeight + _gridYSpacing), 
												_gridItemWidth, 
												_gridItemHeight);
		_gridItems.push_back(it);
		
		if (it->getRelY() <= -_gridItemHeight + 50) {
			warning("upside");
			it->setVisible(false);
		}
		else if (it->getRelY() >= _h) {
			warning("downside");
			it->setVisible(false);
		}
		else {
			it->setVisible(true);
		}

		if (eIter != _visibleEntries.end()) {
			it->setActiveEntry(*eIter);
			eIter++;
		}
		else {
			it->setActiveEntry(*_visibleEntries.begin());
			it->setVisible(false);
		}
		
		if (++col >= _itemsPerRow) {
			++row;
			col = 0;
		}
		it->update();
	}

	markAsDirty();
}

void GridWidget::updateGrid() {
	for (auto i = _gridItems.begin(); i != _gridItems.end(); ++i) {
		(*i)->update();
	}
}

} // End of namespace GUI
