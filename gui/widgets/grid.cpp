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
#include "common/platform.h"

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
		setTooltip(_activeEntry->title);
		markAsDirty();
	}
}

void GridItemWidget::move(int x, int y) {
	Widget::setPos(getRelX() + x, getRelY() + y);
}

void GridItemWidget::drawWidget() {
	int thumbHeight = _grid->getThumbnailHeight();
	int thumbWidth = _grid->getThumbnailWidth();
	Array<U32String> titleLines;
	g_gui.getFont().wordWrapText(_activeEntry->title, thumbWidth, titleLines);

	if ((isHighlighted) || (_grid->getSelected() == _activeEntry->entryID)) {
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
		int linesInThumb = MIN(thumbHeight / kLineHeight, (int)titleLines.size());
		for (int i = 0; i < linesInThumb; ++i) {
			g_gui.theme()->drawText(Common::Rect(_x, _y + ((thumbHeight - (linesInThumb - 2 * i) * kLineHeight)) / 2,
							_x + thumbWidth, _y + ((thumbHeight - (linesInThumb - 2 * i) * kLineHeight)) / 2 + kLineHeight),
							titleLines[i], GUI::ThemeEngine::kStateEnabled, Graphics::kTextAlignCenter,
							ThemeEngine::kTextInversionNone, 0, true, ThemeEngine::kFontStyleNormal,
							ThemeEngine::kFontColorAlternate, false);
		}
	} else
		g_gui.theme()->drawSurface(Common::Point(_x, _y), _thumbGfx, true);

	// Draw Platform Icon
	const Graphics::ManagedSurface *platGfx = _grid->platformToSurface(_activeEntry->platform);
	if (platGfx) {
		g_gui.theme()->drawSurface(Common::Point(_x + thumbWidth - platGfx->w, _y + thumbHeight - platGfx->h), 
									*platGfx, true);
	}

	// Draw Flag
	const Graphics::ManagedSurface *flagGfx = _grid->languageToSurface(_activeEntry->language);
	if (flagGfx)
		g_gui.theme()->drawSurface(Common::Point(_x + thumbWidth - flagGfx->w - 5, _y + 5), 
									*flagGfx, true);

	// Draw Title
	if (_grid->_isTitlesVisible) {
		if (titleLines.size() < 2) {
			titleLines.push_back(U32String());
		} else if (titleLines.size() > 2) {
			titleLines[1].erase(titleLines[1].size() - 3);
			titleLines[1] += U32String("...");
		}
		g_gui.theme()->drawText(Common::Rect(_x, _y + thumbHeight, _x + thumbWidth, _y + thumbHeight + kLineHeight),
								titleLines[0], GUI::ThemeEngine::kStateEnabled, Graphics::kTextAlignCenter);
		g_gui.theme()->drawText(Common::Rect(_x, _y + thumbHeight + kLineHeight, _x + thumbWidth, _y + thumbHeight + 2 * kLineHeight),
								titleLines[1], GUI::ThemeEngine::kStateEnabled, Graphics::kTextAlignCenter);
	}
}

void GridItemWidget::handleMouseWheel(int x, int y, int direction) {
	_grid->handleMouseWheel(x, y, direction);
	isHighlighted = false;
}

void GridItemWidget::handleMouseEntered(int button) {
	if (!isHighlighted) {
		isHighlighted = true;
		markAsDirty();
	}
}

void GridItemWidget::handleMouseLeft(int button) {
	if (isHighlighted) {
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
	if (isHighlighted && isVisible()) {
		_grid->_selectedEntry = _activeEntry->entryID;
		sendCommand(kItemClicked, 0);
		// Work in progress
		// Since user expected to click on "entry" and not the "widget", we
		// must open the tray where the user expects it to be, which might
		// not be at the new widget location.
		int oldX = getAbsX(), oldY = getAbsY();
		int offsetY = 0;
		if (_y > (_grid->getHeight() - _h - _grid->_trayHeight)) {
			offsetY = _y - (_grid->getHeight() - _h - _grid->_trayHeight);
			sendCommand(kSetPositionCmd, _grid->getScrollPos() + offsetY);
			_grid->markAsDirty();
			_grid->draw();
		}
		_grid->openTray(oldX,  oldY - offsetY + _h, _activeEntry->entryID);
		_grid->_tray->runModal();
	}
}

#pragma mark -

GridItemTray::GridItemTray(GuiObject *boss, int x, int y, int w, int h, int entryID, GridWidget *grid)
	: Dialog(x, y, w, h), CommandSender(boss) {

	_entryID = entryID;
	_boss = boss;
	_grid = grid;

	int buttonWidth = w / 3;
	int buttonHeight = h / 3;

	PicButtonWidget *playButton = new PicButtonWidget(this, (buttonWidth / 3), buttonHeight / 3, (buttonWidth / 3) + buttonWidth * 2, buttonHeight, U32String("Play"), kPlayButtonCmd);
	PicButtonWidget *loadButton = new PicButtonWidget(this, (buttonWidth / 3), (buttonHeight * 5) / 3, buttonWidth, buttonHeight, U32String("Saves"), kLoadButtonCmd);
	PicButtonWidget *editButton = new PicButtonWidget(this, buttonWidth + 2 * (buttonWidth / 3), (buttonHeight * 5) / 3, buttonWidth, buttonHeight, U32String("Edit"), kEditButtonCmd);

	playButton->useThemeTransparency(true);
	loadButton->useThemeTransparency(true);
	editButton->useThemeTransparency(true);

	playButton->setGfxFromTheme("button_play.bmp", 0, false);
	loadButton->setGfxFromTheme("button_load.bmp", 0, false);
	editButton->setGfxFromTheme("button_options.bmp", 0, false);
}

void GridItemTray::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPlayButtonCmd:
		close();
		sendCommand(kPlayButtonCmd, _entryID);
		break;
	case kLoadButtonCmd:
		close();
		sendCommand(kLoadButtonCmd, _entryID);
		break;
	case kEditButtonCmd:
		close();
		sendCommand(kEditButtonCmd, _entryID);
		break;
	case kCloseCmd:
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, 0);
		break;
	}
}

void GridItemTray::handleMouseDown(int x, int y, int button, int clickCount) {
	Dialog::handleMouseDown(x, y, button, clickCount);
	if ((x < 0 || x > _w) || (y > _h || y < -(_grid->_gridItemHeight))) {
		// Close on clicking outside
		close();
	} else if (y < 0 && clickCount >= 2) {
		// Run on double clicking thumbnail
		close();
		sendCommand(kItemDoubleClickedCmd, _entryID);
	}
}

void GridItemTray::handleMouseWheel(int x, int y, int direction) {
	Dialog::handleMouseWheel(x, y, direction);
	close();
}

void GridItemTray::handleMouseMoved(int x, int y, int button) {
	Dialog::handleMouseMoved(x, y, button);
	if ((x < 0 || x > _w) || (y > _h || y < -(_grid->_gridItemHeight))) {
		// Close on going outside
		close();
	}
}

#pragma mark -

Graphics::ManagedSurface *loadSurfaceFromFile(const Common::String &name) {
	Graphics::ManagedSurface *surf = nullptr;
	const Graphics::Surface *srcSurface = nullptr;
	const String path = String::format("%s/%s", ConfMan.get("iconpath").c_str(), name.c_str());
	if (name.hasSuffix(".png")) {
#ifdef USE_PNG
		Image::PNGDecoder decoder;
		Common::FSNode fileNode(path);
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

		}
#else
		error("No PNG support compiled");
#endif
	} else if (name.hasSuffix(".svg")) {
		Graphics::SVGBitmap *image = nullptr;
		Common::FSNode fileNode(path);
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
	: ContainerWidget(boss, x, y, w, h), CommandSender(boss) {
	_iconDir = ConfMan.get("iconpath");
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
	_itemsOnScreen = 0;

	_selectedEntry = -1;
}

GridWidget::GridWidget(GuiObject *boss, const String &name)
	: ContainerWidget(boss, name), CommandSender(boss) {
	_iconDir = ConfMan.get("iconpath");
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
	_itemsOnScreen = 0;

	_selectedEntry = -1;
}

GridWidget::~GridWidget() {
	_platformIcons.clear();
	_languageIcons.clear();
	_loadedSurfaces.clear();
	_gridItems.clear();
	_allEntries.clear();
	_visibleEntries.clear();
}

const Graphics::ManagedSurface *GridWidget::filenameToSurface(const String &name) {
	for (Common::Array<GridItemInfo>::iterator l = _visibleEntries.begin(); l!=_visibleEntries.end(); ++l) {
		if (l->thumbPath == name) {
			return _loadedSurfaces[name];
		}
	}
	return nullptr;
}

const Graphics::ManagedSurface *GridWidget::languageToSurface(Common::Language languageCode) {
	if (languageCode == Common::Language::UNK_LANG)
		return nullptr;
	return _languageIcons[languageCode];
}

const Graphics::ManagedSurface *GridWidget::platformToSurface(Common::Platform platformCode) {
	if (platformCode == Common::Platform::kPlatformUnknown)
		return nullptr;
	return _platformIcons[platformCode];
}

void GridWidget::setEntryList(Common::Array<GridItemInfo> *list) {
	_allEntries.clear();
	for (Common::Array<GridItemInfo>::iterator entryIter = list->begin(); entryIter != list->end(); ++entryIter) {
		_allEntries.push_back(*entryIter);
	}
	if (!_gridItems.empty()) {
		reflowLayout();
	}
}

bool GridWidget::calcVisibleEntries() {
	bool needsReload = false;

	int nFirstVisibleItem = 0, nItemsOnScreen = 0;

	nFirstVisibleItem = _itemsPerRow * (-_scrollPos / (_gridItemHeight + _gridYSpacing));
	nFirstVisibleItem = (nFirstVisibleItem < 0) ? 0 : nFirstVisibleItem;

	nItemsOnScreen = (3 + (_scrollWindowHeight / (_gridItemHeight + _gridYSpacing))) * (_itemsPerRow);

	if (nFirstVisibleItem != _firstVisibleItem || nItemsOnScreen != _itemsOnScreen) {
		needsReload = true;
		_itemsOnScreen = nItemsOnScreen;
		_firstVisibleItem = nFirstVisibleItem;

		int toRender = MIN(_firstVisibleItem + _itemsOnScreen, (int)_allEntries.size());

		_visibleEntries.clear();
		for (int ind = _firstVisibleItem; ind < toRender; ++ind) {
			GridItemInfo *iter = _allEntries.begin() + ind;
			_visibleEntries.push_back(*iter);
		}
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

	for (Common::Array<GridItemInfo>::iterator iter = _visibleEntries.begin(); iter != _visibleEntries.end(); ++iter) {
		if (!_loadedSurfaces.contains(iter->thumbPath)) {
			surf = loadSurfaceFromFile(iter->thumbPath);
			if (surf) {
				const Graphics::ManagedSurface *scSurf(scaleGfx(surf, _thumbnailWidth, 512));
				_loadedSurfaces[iter->thumbPath] = scSurf;
				surf->free();
				delete surf;
			} else {
				_loadedSurfaces[iter->thumbPath] = nullptr;
			}
		}
	}
}

void GridWidget::loadFlagIcons() {
	const Common::LanguageDescription *l = Common::g_languages;
	for (; l->code; ++l) {
		String path = String::format("flags/%s.svg", l->code);
		Graphics::ManagedSurface *gfx = loadSurfaceFromFile(path);
		if (gfx) {
			const Graphics::ManagedSurface *scGfx = scaleGfx(gfx, _thumbnailWidth, 32);
			_languageIcons[l->id] = scGfx;
			gfx->free();
			delete gfx;
		} else {
			_languageIcons[l->id] = nullptr;
		}
	}
}

void GridWidget::loadPlatformIcons() {
	const Common::PlatformDescription *l = Common::g_platforms;
	for (; l->code; ++l) {
		String path = String::format("platforms/%s.png", l->code);
		Graphics::ManagedSurface *gfx = loadSurfaceFromFile(path);
		if (gfx) {
			const Graphics::ManagedSurface *scGfx = scaleGfx(gfx, _thumbnailWidth, 32);
			_platformIcons[l->id] = scGfx;
			gfx->free();
			delete gfx;
		} else {
			_platformIcons[l->id] = nullptr;
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
	for (Common::Array<GridItemWidget *>::iterator i = _gridItems.begin(); i != _gridItems.end(); ++i) {
		(*i)->move(x, y);
	}
}

void GridWidget::updateGrid() {
	for (Common::Array<GridItemWidget *>::iterator i = _gridItems.begin(); i != _gridItems.end(); ++i) {
		(*i)->update();
	}
}

void GridWidget::assignEntriesToItems() {
	// Assign entries from _visibleEntries to each GridItem in _gridItems
	Common::Array<GridItemInfo>::iterator entry = _visibleEntries.begin();
	// Start assigning from the second row as the first row is supposed
	// to be offscreen.
	Common::Array<GridItemWidget *>::iterator it = _gridItems.begin() + _itemsPerRow;

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *item = *it;
		if (entry != _visibleEntries.end()) {
			// Assign entry and update
			item->setActiveEntry(*entry);
			item->update();
			if (k >= _itemsOnScreen - _itemsPerRow)
				item->setVisible(false);
			else
				item->setVisible(true);
			++entry;
		} else {
			// If we run out of visible entries to display.
			// e.g., scrolled to the very bottom, we make items invisible.
			item->setVisible(false);
		}

		if (++it == _gridItems.end())
			it = _gridItems.begin();
	}
}

void GridWidget::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
	_scrollPos = -_scrollBar->_currentPos;
}

void GridWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	// Work in progress
	switch (cmd) {
	case kSetPositionCmd:
		if (-_scrollPos != (int)data) {
			_scrollPos = -data;

			if (calcVisibleEntries()) {
				reloadThumbnails();
			}

			int row = 0;
			int col = 0;

			for (Common::Array<GridItemWidget *>::iterator it = _gridItems.begin(); it != _gridItems.end(); ++it) {
				(*it)->setPos(2 * _minGridXSpacing + col * (_gridItemWidth + _gridXSpacing), 
							  _gridYSpacing + (row - 1) * (_gridItemHeight + _gridYSpacing) - (-_scrollPos % (_gridItemHeight + _gridYSpacing)));
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
		sendCommand(cmd, 0);
		break;
	}
}

void GridWidget::reflowLayout() {
	Widget::reflowLayout();
	destroyItems();

	_scrollWindowHeight = _h;
	_scrollWindowWidth = _w;

	int oldThumbnailHeight = _thumbnailHeight;
	int oldThumbnailWidth = _thumbnailWidth;
	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	if ((oldThumbnailHeight != _thumbnailHeight) || (oldThumbnailWidth != _thumbnailWidth)) {
		_loadedSurfaces.clear();
		reloadThumbnails();
		loadFlagIcons();
	}
	_minGridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");

	_isTitlesVisible = g_gui.xmlEval()->getVar("Globals.Grid.ShowTitles");

	_scrollBarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	_trayHeight = kLineHeight * 3;
	_gridItemHeight = _thumbnailHeight + (2 * kLineHeight * _isTitlesVisible);
	_gridItemWidth = _thumbnailWidth;

	_itemsPerRow = MAX(((_scrollWindowWidth - (2 * _minGridXSpacing) - _scrollBarWidth) / (_gridItemWidth + _minGridXSpacing)), 1);
	_gridXSpacing = MAX(((_scrollWindowWidth - (2 * _minGridXSpacing) - _scrollBarWidth) - (_itemsPerRow * _gridItemWidth)) / _itemsPerRow, _minGridXSpacing);

	_rows = ceil(_allEntries.size() / (float)_itemsPerRow);

	_innerHeight = _trayHeight + _gridYSpacing + _rows * (_gridItemHeight + _gridYSpacing);
	_innerWidth = (2 * _minGridXSpacing) + (_itemsPerRow * (_gridItemWidth + _gridXSpacing));

	_scrollBar->checkBounds(_scrollBar->_currentPos);
	_scrollPos = _scrollBar->_currentPos;

	int row = 0;
	int col = 0;


	_scrollBar->resize(_w - _scrollBarWidth, 0, _scrollBarWidth, _h, false);

	if (calcVisibleEntries()) {
		reloadThumbnails();
	}

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *newItem = new GridItemWidget(this, 
									2 * _minGridXSpacing + col * (_gridItemWidth + _gridXSpacing), 
							  		_gridYSpacing + (row - 1) * (_gridItemHeight + _gridYSpacing) - ((-_scrollPos) % (_gridItemHeight + _gridYSpacing)),
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

void GridWidget::openTray(int x, int y, int entryId) {
	_tray = new GridItemTray(this, x  - _gridXSpacing / 3, y, _gridItemWidth + 2 * (_gridXSpacing / 3), _trayHeight, entryId, this);
}

void GridWidget::scrollBarRecalc() {
	_scrollBar->_numEntries = _innerHeight;
	_scrollBar->_entriesPerPage = _scrollWindowHeight - _gridYSpacing;
	_scrollBar->_currentPos = -_scrollPos;
	_scrollBar->_singleStep = kLineHeight;

	_scrollBar->checkBounds(_scrollBar->_currentPos);
	_scrollPos = _scrollBar->_currentPos;
	_scrollBar->recalc();
}

} // End of namespace GUI
