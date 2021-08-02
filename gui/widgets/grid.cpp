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
	: ContainerWidget(boss, 0, 0, 0, 0), CommandSender(boss) {

	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_activeEntry = nullptr;
	_grid = boss;
	isHighlighted = false;
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
	if (_activeEntry->isHeader) {
		g_gui.theme()->drawFoldIndicator(Common::Rect(_x - _h, _y, _x, _y + _h), _grid->groupExpanded(_activeEntry->entryID));
		g_gui.theme()->drawText(Common::Rect(_x, _y, _x + _w, _y + _h), Common::U32String(_activeEntry->title),
								ThemeEngine::kStateEnabled, Graphics::kTextAlignLeft, ThemeEngine::kTextInversionFocus, _h, false);
		return;
	}
	int thumbHeight = _grid->getThumbnailHeight();
	int thumbWidth = _grid->getThumbnailWidth();
	Array<U32String> titleLines;
	g_gui.getFont().wordWrapText(_activeEntry->title, thumbWidth, titleLines);

	// FIXME/HACK: We reserve 1/3 of the space between two items to draw the
	//			selection border. This can break when the stroke width of
	//			the border is comparable to 1/3 of grid item spacing. Also,
	//			border shadow is not considered.
	const int kMarginX = _grid->_gridXSpacing / 3;
	const int kMarginY = _grid->_gridYSpacing / 3;

	if ((isHighlighted) || (_grid->getSelected() == _activeEntry->entryID)) {
		Common::Rect r(_x - kMarginX, _y - kMarginY,
					   _x + _w + kMarginX, _y + _h + kMarginY);
		// Draw a highlighted BG on hover
		g_gui.theme()->drawWidgetBackground(r, ThemeEngine::kGridItemHighlight);
	} else {
		Common::Rect r(_x - 2 * kMarginX, _y - 2 * kMarginY,
					   _x + _w + 2 * kMarginX, _y + _h + 2 * kMarginY);
		// Draw a BG of the same color as grid area
		// when it is not highlighted to cover up
		// the highlight shadow
		// FIXME: Find a way to redraw the area around the widget
		//		 instead of just drawing a cover-up
		g_gui.theme()->drawWidgetBackground(r, ThemeEngine::kGridItemBackground);
	}

	// Draw Thumbnail Background
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + thumbWidth, _y + thumbHeight),
										ThemeEngine::kThumbnailBackground);

	// Draw Thumbnail
	if (_thumbGfx.empty()) {
		// Draw Title when thumbnail is missing
		int linesInThumb = MIN(thumbHeight / kLineHeight, (int)titleLines.size());
		Common::Rect r(_x, _y + (thumbHeight - linesInThumb * kLineHeight) / 2,
					   _x + thumbWidth, _y + (thumbHeight - linesInThumb * kLineHeight) / 2 + kLineHeight);
		for (int i = 0; i < linesInThumb; ++i) {
			g_gui.theme()->drawText(r, titleLines[i], ThemeEngine::kStateEnabled,
									Graphics::kTextAlignCenter, ThemeEngine::kTextInversionNone,
									0, true, ThemeEngine::kFontStyleNormal,
									ThemeEngine::kFontColorAlternate, false);
			r.translate(0, kLineHeight);
		}
	} else {
		g_gui.theme()->drawSurface(Common::Point(_x, _y), _thumbGfx, true);
	}

	// Draw Platform Icon
	const Graphics::ManagedSurface *platGfx = _grid->platformToSurface(_activeEntry->platform);
	if (platGfx) {
		Common::Point p(_x + thumbWidth - platGfx->w, _y + thumbHeight - platGfx->h);
		g_gui.theme()->drawSurface(p, *platGfx, true);
	}

	// Draw Flag
	const Graphics::ManagedSurface *flagGfx = _grid->languageToSurface(_activeEntry->language);
	if (flagGfx) {
		Common::Point p(_x + thumbWidth - flagGfx->w - 5, _y + 5);
		g_gui.theme()->drawSurface(p, *flagGfx, true);
	}

	// Draw Title
	if (_grid->_isTitlesVisible) {
		// TODO: Currently title is fixed to two lines at all times, we may want
		//		flexibility to let the theme define the number of title lines.
		if (titleLines.size() > 2) {
			for (uint k = 0; (k < 3) && (titleLines[1].size() > 0); ++k)
				titleLines[1].deleteLastChar();
			titleLines[1] += U32String("...");
		}
		Common::Rect r(_x, _y + thumbHeight, _x + thumbWidth, _y + thumbHeight + kLineHeight);
		for (uint k = 0; k < MIN(2U, titleLines.size()); ++k) {
			g_gui.theme()->drawText(r, titleLines[k], GUI::ThemeEngine::kStateEnabled, Graphics::kTextAlignCenter);
			r.translate(0, kLineHeight);
		}
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

void GridWidget::toggleGroup(int groupID) {
	// Shrink group if it is expanded
	if (_groupExpanded[groupID]) {
		_groupExpanded[groupID] = false;
		sortGroups();
	} else {
	// Expand group if it is shrunk
		_groupExpanded[groupID] = true;
		sortGroups();
	}
	// TODO: Replace reflowLayout with only the necessary sequence of steps
	reflowLayout();
}

void GridItemWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (_activeEntry->isHeader) {
		_grid->_selectedEntry = nullptr;
		_grid->toggleGroup(_activeEntry->entryID);
	} else if (isHighlighted && isVisible()) {
		_grid->_selectedEntry = _activeEntry;
		sendCommand(kItemClicked, 0);
		// Since user expected to click on "entry" and not the "widget", we
		// must open the tray where the user expects it to be, which might
		// not be at the new widget location.
		// TODO: Make a scrollToSelection() function which does this
		if (_y > (_grid->getHeight() - _h - _grid->_trayHeight)) {
			int offsetY = _y - (_grid->getHeight() - _h - _grid->_trayHeight);
			sendCommand(kSetPositionCmd, _grid->getScrollPos() + offsetY);
			_grid->scrollBarRecalc();
			_grid->markAsDirty();
			_grid->draw();
		}
	}
}

void GridItemWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	_grid->openTrayAtSelected();
}

#pragma mark -

GridItemTray::GridItemTray(GuiObject *boss, int x, int y, int w, int h, int entryID, GridWidget *grid)
	: Dialog(x, y, w, h), CommandSender(boss) {

	_entryID = entryID;
	_boss = boss;
	_grid = grid;

	// TODO: Currently, the tray has a hardcoded layout. Theme file may
	//		provide a different layout of buttons.
	int buttonWidth = w / 3;
	int buttonHeight = h / 3;
	int buttonSpacingX = buttonWidth / 3;
	int buttonSpacingY = buttonHeight / 3;
	int trayPaddingX = buttonSpacingX;
	int trayPaddingY = buttonSpacingY;

	PicButtonWidget *playButton = new PicButtonWidget(this, trayPaddingX, trayPaddingY,
													  2 * buttonWidth + buttonSpacingX, buttonHeight,
													  U32String("Play"), kPlayButtonCmd);
	PicButtonWidget *loadButton = new PicButtonWidget(this, trayPaddingX, trayPaddingY + buttonHeight + buttonSpacingY,
													  buttonWidth, buttonHeight,
													  U32String("Saves"), kLoadButtonCmd);
	PicButtonWidget *editButton = new PicButtonWidget(this, trayPaddingX + buttonWidth + buttonSpacingX, trayPaddingY + buttonHeight + buttonSpacingY,
													  buttonWidth, buttonHeight,
													  U32String("Edit"), kEditButtonCmd);

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
	const String path = String::format("%s/%s", ConfMan.get("iconspath").c_str(), name.c_str());
	if (name.hasSuffix(".png")) {
#ifdef USE_PNG
		const Graphics::Surface *srcSurface = nullptr;
		Image::PNGDecoder decoder;
		Common::FSNode fileNode(path);
		Common::SeekableReadStream *stream = fileNode.createReadStream();
		if (stream) {
			if (!decoder.loadStream(*stream)) {
				warning("Error decoding PNG");
				return surf;
			}

			srcSurface = decoder.getSurface();
			delete stream;
			if (!srcSurface) {
				warning("Failed to load surface : %s", name.c_str());
			} else if (srcSurface->format.bytesPerPixel != 1) {
				surf = new Graphics::ManagedSurface(srcSurface->convertTo(g_system->getOverlayFormat()));
			}

		}
#else
		error("No PNG support compiled");
#endif
	} else if (name.hasSuffix(".svg")) {
		Common::FSNode fileNode(path);
		Common::SeekableReadStream *stream = fileNode.createReadStream();
		if (stream) {
			Graphics::SVGBitmap *image = nullptr;
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
	_iconDir = ConfMan.get("iconspath");

	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	_minGridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");

	loadPlatformIcons();
	loadFlagIcons();

	_scrollBar = new ScrollBarWidget(this, _w - _scrollBarWidth, _y, _scrollBarWidth, _y + _h);
	_scrollBar->setTarget(this);
	_scrollPos = 0;
	_firstVisibleItem = 0;
	_itemsOnScreen = 0;
	_itemsPerRow = 0;

	_innerHeight = 0;
	_innerWidth = 0;
	_scrollWindowHeight = 0;
	_scrollWindowWidth = 0;
	_gridYSpacing = 0;
	_gridXSpacing = 0;

	_selectedEntry = nullptr;
}

GridWidget::GridWidget(GuiObject *boss, const String &name)
	: ContainerWidget(boss, name), CommandSender(boss) {
	_iconDir = ConfMan.get("iconspath");

	_thumbnailHeight = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Height");
	_thumbnailWidth = g_gui.xmlEval()->getVar("Globals.GridItemThumbnail.Width");
	_minGridXSpacing = g_gui.xmlEval()->getVar("Globals.Grid.XSpacing");
	_gridYSpacing = g_gui.xmlEval()->getVar("Globals.Grid.YSpacing");

	loadPlatformIcons();
	loadFlagIcons();

	_scrollBar = new ScrollBarWidget(this, _w - _scrollBarWidth, _y, _scrollBarWidth, _y + _h);
	_scrollBar->setTarget(this);
	_scrollPos = 0;
	_firstVisibleItem = 0;
	_itemsOnScreen = 0;
	_itemsPerRow = 0;

	_innerHeight = 0;
	_innerWidth = 0;
	_scrollWindowHeight = 0;
	_scrollWindowWidth = 0;
	_gridYSpacing = 0;
	_gridXSpacing = 0;

	_selectedEntry = nullptr;
}

GridWidget::~GridWidget() {
	_platformIcons.clear();
	_languageIcons.clear();
	_loadedSurfaces.clear();
	_gridItems.clear();
	_dataEntryList.clear();
	_sortedEntryList.clear();
	_visibleEntryList.clear();
}

const Graphics::ManagedSurface *GridWidget::filenameToSurface(const String &name) {
	for (Common::Array<GridItemInfo *>::iterator l = _visibleEntryList.begin(); l != _visibleEntryList.end(); ++l) {
		if ((!(*l)->isHeader) && ((*l)->thumbPath == name)) {
			return _loadedSurfaces[name];
		}
	}
	return nullptr;
}

const Graphics::ManagedSurface *GridWidget::languageToSurface(Common::Language languageCode) {
	if (languageCode == Common::UNK_LANG)
		return nullptr;
	return _languageIcons[languageCode];
}

const Graphics::ManagedSurface *GridWidget::platformToSurface(Common::Platform platformCode) {
	if (platformCode == Common::kPlatformUnknown)
		return nullptr;
	return _platformIcons[platformCode];
}

void GridWidget::setEntryList(Common::Array<GridItemInfo> *list) {
	_dataEntryList.clear();
	_sortedEntryList.clear();
	for (Common::Array<GridItemInfo>::iterator entryIter = list->begin(); entryIter != list->end(); ++entryIter) {
		_dataEntryList.push_back(*entryIter);
	}
	groupEntries();
	// TODO: Remove this below, add drawWidget(), that should do the drawing
	if (!_gridItems.empty()) {
		reflowLayout();
	}
}

void GridWidget::setAttributeValues(const Common::Array<U32String> &attrs) {
	assert(attrs.size() == _dataEntryList.size());
	for (uint i = 0; i < _dataEntryList.size(); ++i) {
		_dataEntryList[i].attribute = attrs[i];
	}
}

void GridWidget::groupEntries() {
	_groupExpanded.clear();
	_groupHeaders.clear();
	_groupValueIndex.clear();
	_itemsInGroup.clear();

	for (uint i = 0; i < _dataEntryList.size(); ++i) {
		U32String attrVal = _dataEntryList[i].attribute;
		if (!_groupValueIndex.contains(attrVal)) {
			int newGroupID = _groupValueIndex.size();
			_groupValueIndex.setVal(attrVal, newGroupID);
			_groupHeaders.push_back(attrVal);
			_groupExpanded.push_back(true);
		}
		int groupID = _groupValueIndex.getVal(attrVal);

		_itemsInGroup[groupID].push_back(i);
	}

	sortGroups();
}

void GridWidget::sortGroups() {
	uint oldHeight = _innerHeight;
	_sortedEntryList.clear();

	Common::sort(_groupHeaders.begin(), _groupHeaders.end());

	for (uint i = 0; i != _groupHeaders.size(); ++i) {
		U32String header = _groupHeaders[i];
		U32String displayedHeader = header;
		uint groupID = _groupValueIndex[header];

		_sortedEntryList.push_back(GridItemInfo(_groupHeaderPrefix + displayedHeader + _groupHeaderSuffix, groupID));

		if (_groupExpanded[groupID]) {
			for (int *k = _itemsInGroup[groupID].begin(); k != _itemsInGroup[groupID].end(); ++k) {
				_sortedEntryList.push_back(_dataEntryList[*k]);
			}
		}
	}

	calcEntrySizes();
	calcInnerHeight();
	scrollBarRecalc();
	// FIXME: Temporary solution to clear/display the background ofthe scrollbar when list
	// grows too small or large during group toggle. We shouldn't have to redraw the top dialog,
	// but not doing so the background of scrollbar isn't cleared.
	if ((((uint)_scrollBar->_entriesPerPage < oldHeight) && (_scrollBar->_entriesPerPage > _innerHeight)) ||
		(((uint)_scrollBar->_entriesPerPage > oldHeight) && (_scrollBar->_entriesPerPage < _innerHeight))) {
		g_gui.scheduleTopDialogRedraw();
	} else {
		markAsDirty();
	}
}

bool GridWidget::calcVisibleEntries() {
	bool needsReload = false;

	int nFirstVisibleItem = 0, nItemsOnScreen = 0;

	// Binary search to find the last element whose y value is less
	// than _scrollPos, i.e., the last item of the topmost visible row.
	int start = 0;
	int end = (int)_sortedEntryList.size() - 1;
	int mid;
	int ans = -1;
	while (start <= end) {
		mid = start + (end - start) / 2;
		if (_sortedEntryList[mid].rect.top >= _scrollPos) {
			end = mid - 1;
		} else {
			ans = mid;
			start = mid + 1;
		}
	}
	nFirstVisibleItem = ans;
	// We want the leftmost item from the topmost visible row, so we traverse backwards
	while ((nFirstVisibleItem >= 0) && 
		   (_sortedEntryList[nFirstVisibleItem].rect.top == _sortedEntryList[ans].rect.top)) {
			nFirstVisibleItem--;
	}
	nFirstVisibleItem++;
	nFirstVisibleItem = (nFirstVisibleItem < 0) ? 0 : nFirstVisibleItem;

	nItemsOnScreen = (3 + (_scrollWindowHeight / (_gridItemHeight + _gridYSpacing))) * (_itemsPerRow);

	// TODO / FIXME: The previous if condition had to be removed as it can break on toggling groups.
	//				Find a proper if condition and put the below code under it.
	{
		needsReload = true;
		_itemsOnScreen = nItemsOnScreen;
		_firstVisibleItem = nFirstVisibleItem;

		int toRender = MIN(_firstVisibleItem + _itemsOnScreen, (int)_sortedEntryList.size());

		_visibleEntryList.clear();
		for (int ind = _firstVisibleItem; ind < toRender; ++ind) {
			GridItemInfo *iter = _sortedEntryList.begin() + ind;
			_visibleEntryList.push_back(iter);
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

	for (Common::Array<GridItemInfo *>::iterator iter = _visibleEntryList.begin(); iter != _visibleEntryList.end(); ++iter) {
		GridItemInfo *entry = *iter;
		if (!_loadedSurfaces.contains(entry->thumbPath)) {
			surf = loadSurfaceFromFile(entry->thumbPath);
			if (surf) {
				const Graphics::ManagedSurface *scSurf(scaleGfx(surf, _thumbnailWidth, 512));
				_loadedSurfaces[entry->thumbPath] = scSurf;
				surf->free();
				delete surf;
			} else {
				_loadedSurfaces[entry->thumbPath] = nullptr;
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
	Common::Array<GridItemInfo *>::iterator eit = _visibleEntryList.begin();
	Common::Array<GridItemWidget *>::iterator it = _gridItems.begin();

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *item = *it;
		GridItemInfo *entry = *eit;
		if (eit != _visibleEntryList.end()) {
			// Assign entry and update
			item->setActiveEntry(*entry);
			item->setPos(entry->rect.left, entry->rect.top - _scrollPos);
			item->setSize(entry->rect.width(), entry->rect.height());
			item->update();
			if (k >= _itemsOnScreen - _itemsPerRow)
				item->setVisible(false);
			else
				item->setVisible(true);
			++eit;
		} else {
			// If we run out of visible entries to display.
			// e.g., scrolled to the very bottom, we make items invisible,
			// and move them out of view to keep them from registering mouse events.
			item->setPos(_scrollWindowWidth, _scrollWindowHeight);
			item->setVisible(false);
		}

		if (++it == _gridItems.end())
			it = _gridItems.begin();
	}
}

void GridWidget::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
	_scrollPos = _scrollBar->_currentPos;
}

void GridWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	// Work in progress
	switch (cmd) {
	case kSetPositionCmd:
		if (_scrollPos != (int)data) {
			_scrollPos = data;

			if (calcVisibleEntries()) {
				reloadThumbnails();
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

void GridWidget::calcInnerHeight() {
	int row = 0;
	int col = 0;
	int lastRowHeight = 0;
	Common::Point p(_gridXSpacing, _gridYSpacing);

	for (int k = 0; k < (int)_sortedEntryList.size(); ++k) {
		if (_sortedEntryList[k].isHeader) {
			while (col != 0) {
				if (++col >= _itemsPerRow) {
					col = 0;
					p.x = _gridXSpacing;
					++row;
					p.y += lastRowHeight;
					lastRowHeight = 0;
				}
			}
			_sortedEntryList[k].rect.moveTo(p);
			++row;
			p.y += _sortedEntryList[k].rect.height() + _gridYSpacing;
			lastRowHeight = 0;
		} else {
			_sortedEntryList[k].rect.moveTo(p);
			lastRowHeight = MAX(lastRowHeight, _sortedEntryList[k].rect.height() + _gridYSpacing);
			if (++col >= _itemsPerRow) {
				++row;
				p.y += lastRowHeight;
				lastRowHeight = 0;
				col = 0;
				p.x = _gridXSpacing;
			} else {
				p.x += _sortedEntryList[k].rect.width() + _gridXSpacing;
			}
		}
	}

	_rows = row;

	_innerHeight = p.y + _gridItemHeight + _gridYSpacing + _trayHeight;
	_innerWidth = _gridXSpacing + (_itemsPerRow * (_gridItemWidth + _gridXSpacing));
}

void GridWidget::calcEntrySizes() {
	_gridHeaderHeight = kLineHeight;
	_gridHeaderWidth = _scrollWindowWidth - _scrollBarWidth - 2 * _gridXSpacing;

	for (uint i = 0; i != _sortedEntryList.size(); ++i) {
		GridItemInfo *entry = &_sortedEntryList[i];
		if (entry->isHeader) {
			entry->rect.setHeight(_gridHeaderHeight);
			entry->rect.setWidth(_gridHeaderWidth);
		} else {
			int titleRows;
			if (_isTitlesVisible) {
				Array<U32String> titleLines;
				g_gui.getFont().wordWrapText(entry->title, _gridItemWidth, titleLines);
				titleRows = MIN(2U, titleLines.size());
			} else {
				titleRows = 0;
			}
			entry->rect.setHeight(_thumbnailHeight + titleRows * kLineHeight);
			entry->rect.setWidth(_gridItemWidth);
		}
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

	calcEntrySizes();
	calcInnerHeight();

	_scrollBar->checkBounds(_scrollBar->_currentPos);
	_scrollPos = _scrollBar->_currentPos;

	_scrollBar->resize(_scrollWindowWidth - _scrollBarWidth, 0, _scrollBarWidth, _scrollWindowHeight, false);

	if (calcVisibleEntries()) {
		reloadThumbnails();
	}

	int row = 0;
	int col = 0;

	for (int k = 0; k < _itemsOnScreen; ++k) {
		GridItemWidget *newItem = new GridItemWidget(this);
		newItem->setVisible(false);

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
	_tray = new GridItemTray(this, x - _gridXSpacing / 3, y, _gridItemWidth + 2 * (_gridXSpacing / 3), _trayHeight, entryId, this);
	_tray->runModal();
}

void GridWidget::openTrayAtSelected() {
	if (_selectedEntry) {
		_tray = new GridItemTray(this, _x + _selectedEntry->rect.left - _gridXSpacing / 3, _y + _selectedEntry->rect.bottom - _scrollPos,
								_gridItemWidth + 2 * (_gridXSpacing / 3), _trayHeight, _selectedEntry->entryID, this);
		_tray->runModal();
	}
}

void GridWidget::scrollBarRecalc() {
	_scrollBar->_numEntries = _innerHeight;
	_scrollBar->_entriesPerPage = _scrollWindowHeight - _gridYSpacing;
	_scrollBar->_currentPos = _scrollPos;
	_scrollBar->_singleStep = kLineHeight;

	_scrollBar->checkBounds(_scrollBar->_currentPos);
	_scrollPos = _scrollBar->_currentPos;
	_scrollBar->recalc();
}

} // End of namespace GUI
