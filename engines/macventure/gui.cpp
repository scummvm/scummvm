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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "common/file.h"
#include "common/system.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "image/bmp.h"
#include "graphics/macgui/macfontmanager.h"

#include "macventure/gui.h"
#include "macventure/dialog.h"

namespace MacVenture {

enum {
	kCursorWidth = 2,
	kCursorHeight = 2
};

enum {
	kExitButtonWidth = 10,
	kExitButtonHeight = 10
};

enum {
	kMenuHighLevel = -1,
	kMenuAbout = 0,
	kMenuFile = 1,
	kMenuEdit = 2,
	kMenuSpecial = 3
};

enum {
	kCommandNum = 8
};

enum {
	kDragThreshold = 5
};

const bool kLoadStaticMenus = true;

static const Graphics::MacMenuData menuSubItems[] = {
	{ kMenuHighLevel,	"File",				0, 0, false },
	{ kMenuHighLevel,	"Edit",				0, 0, false },
	{ kMenuHighLevel,	"Special",			0, 0, false },
	{ kMenuHighLevel,	"Font",				0, 0, false },
	{ kMenuHighLevel,	"FontSize",			0, 0, false },

	//{ kMenuAbout,		"About",			kMenuActionAbout, 0, true},

	{ kMenuFile,		"New",				kMenuActionNew, 0, true },
	{ kMenuFile,		nullptr,				0, 0, false },
	{ kMenuFile,		"Open...",			kMenuActionOpen, 0, true },
	{ kMenuFile,		"Save",				kMenuActionSave, 0, true },
	{ kMenuFile,		"Save as...",		kMenuActionSaveAs, 0, true },
	{ kMenuFile,		nullptr,				0, 0, false },
	{ kMenuFile,		"Quit",				kMenuActionQuit, 0, true },

	{ kMenuEdit,		"Undo",				kMenuActionUndo, 'Z', false },
	{ kMenuEdit,		nullptr,				0, 0, false },
	{ kMenuEdit,		"Cut",				kMenuActionCut, 'K', false },
	{ kMenuEdit,		"Copy",				kMenuActionCopy, 'C', false },
	{ kMenuEdit,		"Paste",			kMenuActionPaste, 'V', false },
	{ kMenuEdit,		"Clear",			kMenuActionClear, 'B', false },

	{ kMenuSpecial,		"Clean Up",			kMenuActionCleanUp, 0, false },
	{ kMenuSpecial,		"Mess Up",			kMenuActionMessUp, 0, false },

	{ 0,				nullptr,				0, 0, false }
};


bool commandsWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool mainGameWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool outConsoleWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool selfWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool exitsWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool diplomaWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool inventoryWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);

void menuCommandsCallback(int action, Common::String &text, void *data);

Gui::Gui(MacVentureEngine *engine, Common::MacResManager *resman) {
	_engine = engine;
	_resourceManager = resman;
	_windowData = nullptr;
	_controlData = nullptr;
	_draggedObj.id = 0;
	_draggedObj.pos = Common::Point(0, 0);
	_dialog = nullptr;

	_cursor = new Cursor(this);

	_consoleText = new ConsoleText(this);
	_graphics = nullptr;

	initGUI();
}

Gui::~Gui() {

	if (_windowData)
		delete _windowData;

	if (_controlData)
		delete _controlData;

	if (_exitsData)
		delete _exitsData;

	if (_cursor)
		delete _cursor;

	if (_consoleText)
		delete _consoleText;

	if (_dialog)
		delete _dialog;

	clearAssets();

	if (_graphics)
		delete _graphics;
}

void Gui::initGUI() {
	_screen.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_wm.setScreen(&_screen);

	// Menu
	_menu = _wm.addMenu();
	if (!loadMenus())
		error("GUI: Could not load menus");
	_menu->setCommandsCallback(menuCommandsCallback, this);
	_menu->calcDimensions();

	loadGraphics();

	if (!loadWindows())
		error("GUI: Could not load windows");

	initWindows();

	assignObjReferences();

	if (!loadControls())
		error("GUI: Could not load controls");

	draw();

}

void Gui::reloadInternals() {
	clearAssets();
	loadGraphics();
}

void Gui::draw() {
	// Will be performance-improved after the milestone
	_wm.setFullRefresh(true);

	drawWindows();

	_wm.draw();

	drawDraggedObject();
	drawDialog();
	// TODO: When window titles with custom borders are in MacGui, this should be used.
	//drawWindowTitle(kMainGameWindow, _mainGameWindow->getWindowSurface());
}

void Gui::drawMenu() {
	_menu->draw(&_screen);
}

void Gui::drawTitle() {
	warning("drawTitle hasn't been tested yet");
}

void Gui::clearControls() {
	if (!_controlData)
		return;

	Common::Array<CommandButton>::iterator it = _controlData->begin();
	for (; it != _controlData->end(); ++it) {
		it->unselect();
	}
}

void Gui::initWindows() {
	Common::Rect bounds;
	BorderBounds bbs(0 , 0, 0, 0, 0, 0);
	// Game Controls Window
	_controlsWindow = _wm.addWindow(false, false, false);


	bounds = getWindowData(kCommandsWindow).bounds;
	bbs = borderBounds(findWindowData(kCommandsWindow).type);
	loadBorders(_controlsWindow, findWindowData(kCommandsWindow).type);
	_controlsWindow->resize(bounds.width(), bounds.height(), true);
	_controlsWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_controlsWindow->setActive(false);
	_controlsWindow->setCallback(commandsWindowCallback, this);

	// Main Game Window
	_mainGameWindow = _wm.addWindow(false, false, false);
	bounds = getWindowData(kMainGameWindow).bounds;
	bbs = borderBounds(findWindowData(kMainGameWindow).type);

	loadBorders(_mainGameWindow, findWindowData(kMainGameWindow).type);
	_mainGameWindow->resize(bounds.width(), bounds.height(), true);
	_mainGameWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_mainGameWindow->setActive(false);
	_mainGameWindow->setCallback(mainGameWindowCallback, this);

	// In-game Output Console
	_outConsoleWindow = _wm.addWindow(true, true, false);

	bounds = getWindowData(kOutConsoleWindow).bounds;
	bbs = borderBounds(findWindowData(kOutConsoleWindow).type);
	loadBorders(_outConsoleWindow, findWindowData(kOutConsoleWindow).type);
	_outConsoleWindow->resize(bounds.width() - bbs.rightScrollbarWidth, bounds.height() - bbs.bottomScrollbarHeight, true);
	_outConsoleWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);
	_outConsoleWindow->setActive(false);
	_outConsoleWindow->setCallback(outConsoleWindowCallback, this);
	_outConsoleWindow->setTitle("Untitled");
	// Self Window
	_selfWindow = _wm.addWindow(false, true, false);

	bounds = getWindowData(kSelfWindow).bounds;
	bbs = borderBounds(findWindowData(kSelfWindow).type);
	loadBorders(_selfWindow, findWindowData(kSelfWindow).type);
	_selfWindow->resize(bounds.width(), bounds.height(), true);
	_selfWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_selfWindow->setActive(false);
	_selfWindow->setCallback(selfWindowCallback, this);

	// Exits Window
	_exitsWindow = _wm.addWindow(false, false, false);

	bounds = getWindowData(kExitsWindow).bounds;
	bbs = borderBounds(findWindowData(kExitsWindow).type);
	loadBorders(_exitsWindow, findWindowData(kExitsWindow).type);
	_exitsWindow->resize(bounds.width(), bounds.height(), true);
	_exitsWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_exitsWindow->setActive(false);
	_exitsWindow->setCallback(exitsWindowCallback, this);
	_exitsWindow->setTitle(findWindowData(kExitsWindow).title);
	// TODO: In the original, the background is actually a clickable
	// object that can be used to refer to the room itself. In that case,
	// the background should be kPatternDarkGray.
	_exitsWindow->setBackgroundPattern(kPatternLightGray);
}

const WindowData &Gui::getWindowData(WindowReference reference) {
	return findWindowData(reference);
}

const Graphics::Font &Gui::getCurrentFont() {
	return *_wm._fontMan->getFont(Graphics::MacFont(Graphics::kMacFontChicago, 12));
}

void Gui::bringToFront(WindowReference winID) {
	_wm.setActiveWindow(findWindow(winID)->getId());
}

void Gui::setWindowTitle(WindowReference winID, const Common::String &string) {
	findWindowData(winID).title = string;
	findWindowData(winID).titleLength = string.size();
	findWindow(winID)->setTitle(string);
}

void Gui::updateWindowInfo(WindowReference ref, ObjID objID, const Common::Array<ObjID> &children) {
	if (ref == kNoWindow) {
		return;
	}
	WindowData &data = findWindowData(ref);
	data.children.clear();
	data.objRef = objID;
	uint32 originx = 0x7fff;
	uint32 originy = 0x7fff;
	for (uint i = 0; i < children.size(); i++) {
		if (children[i] != 1) {
			ObjID child = children[i];
			if (ref != kMainGameWindow) {
				Common::Point childPos = _engine->getObjPosition(child);
				originx = originx > (uint)childPos.x ? (uint)childPos.x : originx;
				originy = originy > (uint)childPos.y ? (uint)childPos.y : originy;
			}
			data.children.push_back(DrawableObject(child, kBlitBIC));
		}
	}
	if (originx != 0x7fff) {
		data.bounds.left = originx;
	}
	if (originy != 0x7fff) {
		data.bounds.top = originy;
	}
	if (ref != kMainGameWindow) {
		data.updateScroll = true;
	}
}

void Gui::addChild(WindowReference target, ObjID child) {
	findWindowData(target).children.push_back(DrawableObject(child, kBlitBIC));
}

void Gui::removeChild(WindowReference target, ObjID child) {
	WindowData &data = findWindowData(target);
	uint index = 0;
	for (;index < data.children.size(); index++) {
		if (data.children[index].obj == child) {
			break;
		}
	}

	if (index < data.children.size())
		data.children.remove_at(index);
}

void Gui::assignObjReferences() {
	findWindowData(kSelfWindow).objRef = 0;
}

WindowReference Gui::createInventoryWindow(ObjID objRef) {
	Graphics::MacWindow *newWindow = _wm.addWindow(true, true, false);
	WindowData newData;
	GlobalSettings settings = _engine->getGlobalSettings();
	newData.refcon = (WindowReference)(_inventoryWindows.size() + kInventoryStart); // This is a HACK

	if (_windowData->back().refcon < 0x80) { // There is already another inventory window
		newData.bounds = _windowData->back().bounds; // Inventory windows are always last
		newData.bounds.translate(newData.bounds.left + settings._invOffsetX, newData.bounds.top + settings._invOffsetY);
	} else {
		newData.bounds = Common::Rect(
			settings._invLeft,
			settings._invTop,
			settings._invLeft + settings._invWidth,
			settings._invTop + settings._invHeight
		);
	}
	newData.type = kInvWindow;
	newData.hasCloseBox = true;
	newData.visible = true;
	newData.objRef = objRef;
	_windowData->push_back(newData);

	BorderBounds bbs = borderBounds(newData.type);
	//newWindow->setDimensions(newData.bounds);
	//newWindow->setActive(false);
	loadBorders(newWindow, newData.type);
	newWindow->resize(newData.bounds.width(), newData.bounds.height() - bbs.bottomScrollbarHeight, true);
	newWindow->move(newData.bounds.left - bbs.leftOffset, newData.bounds.top - bbs.topOffset);
	newWindow->setCallback(inventoryWindowCallback, this);
	//newWindow->setCloseable(true);

	_inventoryWindows.push_back(newWindow);

	debugC(1, kMVDebugGUI, "Create new inventory window. Reference: %d", newData.refcon);
	return newData.refcon;
}

void Gui::loadBorders(Graphics::MacWindow *target, MVWindowType type) {
	loadBorder(target, type, false);
	loadBorder(target, type, true);
}

void Gui::loadBorder(Graphics::MacWindow *target, MVWindowType type, bool active) {
	Common::SeekableReadStream *stream;

	uint32 activeFlag = (active ? Graphics::kWindowBorderActive : 0);
	bool canHaveTitle = false;
	bool canHaveScrollbar = false;

	if (type == MacVenture::kInvWindow || type == MacVenture::kZoomDoc) {
		canHaveScrollbar = true;
	}

	if (type == MacVenture::kInvWindow || type == MacVenture::kZoomDoc || type == MacVenture::kNoGrowDoc || type == MacVenture::kRDoc4) {
		canHaveTitle = true;
	}

	Graphics::BorderOffsets offsets = borderOffsets(type);
	stream = _engine->getBorderFile(type, active);

	if (stream) {
		target->loadBorder(*stream, activeFlag, offsets);
	}

	if (canHaveTitle) {
		if (stream) {
			stream->seek(0);
			target->loadBorder(*stream, activeFlag | Graphics::kWindowBorderTitle, offsets);
		}
	}

	if (canHaveScrollbar) {
		if (stream) {
			stream->seek(0);
			target->loadBorder(*stream, activeFlag | Graphics::kWindowBorderScrollbar, offsets);
		}
	}

	if (canHaveTitle && canHaveScrollbar) {
		if (stream) {
			stream->seek(0);
			target->loadBorder(*stream, activeFlag | Graphics::kWindowBorderTitle | Graphics::kWindowBorderScrollbar, offsets);
		}
	}
	delete stream;
}

void Gui::loadGraphics() {
	if (_graphics)
		delete _graphics;
	_graphics = new Container(_engine->getFilePath(kGraphicPathID));
}

void Gui::clearAssets() {
	Common::HashMap<ObjID, ImageAsset*>::const_iterator it = _assets.begin();
	for (; it != _assets.end(); it++) {
		delete it->_value;
	}
	_assets.clear();
}

bool Gui::loadMenus() {

	if (kLoadStaticMenus) {
		// We assume that, if there are static menus, we don't need dynamic ones
		_menu->addStaticMenus(menuSubItems);
		return true;
	}

	Common::MacResIDArray resArray;
	Common::MacResIDArray::const_iterator iter;

	if ((resArray = _resourceManager->getResIDArray(MKTAG('M', 'E', 'N', 'U'))).size() == 0)
		return false;

	_menu->addMenuItem(nullptr, "Abb", kMenuActionAbout, 0, 'A', true);

	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		_menu->loadMenuResource(_resourceManager, *iter);
	}

	return true;
}

bool Gui::loadWindows() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	_windowData = new Common::List<WindowData>();

	if ((resArray = _resourceManager->getResIDArray(MKTAG('W', 'I', 'N', 'D'))).size() == 0)
		return false;

	uint32 id = kCommandsWindow;
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('W', 'I', 'N', 'D'), *iter);
		WindowData data;
		uint16 top, left, bottom, right;
		top = res->readUint16BE();
		left = res->readUint16BE();
		bottom = res->readUint16BE();
		right = res->readUint16BE();
		data.type = (MVWindowType)res->readUint16BE();
		data.bounds = Common::Rect(
			left,
			top,
			right,
			bottom
		);
		data.visible = res->readUint16BE();
		data.hasCloseBox = res->readUint16BE();
		data.refcon = (WindowReference)id; id++;
		res->readUint32BE(); // Skip the true id. For some reason it's reading 0
		data.titleLength = res->readByte();
		if (data.titleLength) {
			char *newTitle = new char[data.titleLength + 1];
			res->read(newTitle, data.titleLength);
			newTitle[data.titleLength] = '\0';
			data.title = Common::String(newTitle);
			delete[] newTitle;
		}
		data.scrollPos = Common::Point(0, 0);

		debugC(1, kMVDebugGUI, "Window loaded: %s", data.title.c_str());

		_windowData->push_back(data);

		delete res;
	}

	return true;
}

bool Gui::loadControls() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	_controlData = new Common::Array<CommandButton>();
	_exitsData = new Common::Array<CommandButton>();

	if ((resArray = _resourceManager->getResIDArray(MKTAG('C', 'N', 'T', 'L'))).size() == 0)
		return false;

	uint32 id = kControlExitBox;
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('C', 'N', 'T', 'L'), *iter);
		ControlData data;
		uint16 top, left, bottom, right;
		top = res->readUint16BE();
		left = res->readUint16BE();
		bottom = res->readUint16BE();
		right = res->readUint16BE();
		data.scrollValue = res->readUint16BE();
		data.visible = res->readByte();
		res->readByte(); // Unused
		data.scrollMax = res->readUint16BE();
		data.scrollMin = res->readUint16BE();
		data.cdef = res->readUint16BE();
		data.refcon = (ControlAction)res->readUint32BE();
		data.type = (ControlType)id; id++;
		data.titleLength = res->readByte();
		if (data.titleLength) {
			char *title = new char[data.titleLength + 1];
			res->read(title, data.titleLength);
			title[data.titleLength] = '\0';
			data.title = Common::String(title);
			delete[] title;
		}
		data.bounds = Common::Rect(left, top, right, bottom);
		_controlData->push_back(CommandButton(data, this));
		delete res;
	}

	return true;
}

void Gui::drawWindows() {

	drawCommandsWindow();
	drawMainGameWindow();
	drawSelfWindow();
	drawInventories();
	drawExitsWindow();
	drawConsoleWindow();

}

void Gui::drawCommandsWindow() {
	if (_engine->needsClickToContinue()) {
		Graphics::ManagedSurface *srf = _controlsWindow->getWindowSurface();
		WindowData data = getWindowData(kCommandsWindow);
		srf->fillRect(Common::Rect(0, 0, srf->w, srf->h), kColorWhite);
		getCurrentFont().drawString(
			srf,
			_engine->getCommandsPausedString(),
			0,
			(srf->h - getCurrentFont().getFontHeight()) / 2 - 1,
			data.bounds.right - data.bounds.left,
			kColorBlack,
			Graphics::kTextAlignCenter);
	} else {
		Common::Array<CommandButton>::const_iterator it = _controlData->begin();
		for (; it != _controlData->end(); ++it) {
			CommandButton button = *it;
			if (button.getData().type != kControlExitBox)
				button.draw(*_controlsWindow->getWindowSurface());
		}
	}
}

void Gui::drawMainGameWindow() {
	const WindowData &data = getWindowData(kMainGameWindow);
	ObjID objRef = data.objRef;

	_mainGameWindow->setDirty(true);

	if (data.objRef > 0 && data.objRef < 2000) {
		ensureAssetLoaded(objRef);

		_assets[objRef]->blitInto(
			_mainGameWindow->getWindowSurface(),
			0,
			0,
			kBlitDirect);
	}

	drawObjectsInWindow(data, _mainGameWindow->getWindowSurface());

	if (DebugMan.isDebugChannelEnabled(kMVDebugGUI)) {
		Graphics::MacWindow *win = findWindow(data.refcon);
		Common::Rect innerDims = win->getInnerDimensions();
		innerDims.translate(-innerDims.left, -innerDims.top);
		win->getWindowSurface()->frameRect(innerDims, kColorGreen);
	}

	findWindow(kMainGameWindow)->setDirty(true);
}

void Gui::drawSelfWindow() {
	drawObjectsInWindow(getWindowData(kSelfWindow), _selfWindow->getWindowSurface());
	if (_engine->isObjSelected(1)) {
		invertWindowColors(kSelfWindow);
	}
	findWindow(kSelfWindow)->setDirty(true);
}

void Gui::drawInventories() {

	Graphics::ManagedSurface *srf;
	for (uint i = 0; i < _inventoryWindows.size(); i++) {
		const WindowData &data = getWindowData((WindowReference)(kInventoryStart + i));
		Graphics::MacWindow *win = findWindow(data.refcon);
		srf = win->getWindowSurface();
		srf->clear(kColorGreen);
		srf->fillRect(srf->getBounds(), kColorWhite);
		drawObjectsInWindow(data, srf);

		if (DebugMan.isDebugChannelEnabled(kMVDebugGUI)) {
			Common::Rect innerDims = win->getInnerDimensions();
			innerDims.translate(-innerDims.left, -innerDims.top);
			srf->frameRect(innerDims, kColorGreen);
		}

		findWindow(data.refcon)->setDirty(true);
	}

}

void Gui::drawExitsWindow() {
	_exitsWindow->setBackgroundPattern(kPatternLightGray);

	Graphics::ManagedSurface *srf = _exitsWindow->getWindowSurface();

	Common::Array<CommandButton>::const_iterator it = _exitsData->begin();
	for (; it != _exitsData->end(); ++it) {
		CommandButton button = *it;
		button.draw(*srf);
	}

	Graphics::BorderOffsets offsets = borderOffsets(MacVenture::kRDoc4);
	offsets.dark = _exitsWindow->_active;
	_exitsWindow->setBorderOffsets(offsets);

	findWindow(kExitsWindow)->setDirty(true);
}

void Gui::drawConsoleWindow() {

	Graphics::ManagedSurface *srf = _outConsoleWindow->getWindowSurface();
	BorderBounds bounds = borderBounds(getWindowData(kOutConsoleWindow).type);
	_consoleText->renderInto(srf, bounds, kConsoleLeftOffset);
}

void Gui::drawObjectsInWindow(const WindowData &targetData, Graphics::ManagedSurface *surface) {
	BorderBounds border = borderBounds(targetData.type);
	Common::Point pos;
	ObjID child;
	BlitMode mode;

	if (targetData.children.size() == 0) {
		return;
	}

	Graphics::ManagedSurface composeSurface;
	createInnerSurface(&composeSurface, surface, border);
	assert(composeSurface.w <= surface->w &&
			composeSurface.h <= surface->h);
	composeSurface.clear(kColorGreen);

	for (uint i = 0; i < targetData.children.size(); i++) {
		child = targetData.children[i].obj;
		mode = (BlitMode)targetData.children[i].mode;
		pos = _engine->getObjPosition(child);
		pos -= targetData.scrollPos;
		ensureAssetLoaded(child);

		_assets[child]->blitInto(
			&composeSurface,
			pos.x,
			pos.y,
			mode);

		if (_engine->isObjVisible(child)) {
			if (_engine->isObjSelected(child) ||
				child == _draggedObj.id) {

				_assets[child]->blitInto(
					&composeSurface, pos.x, pos.y, kBlitOR);
			}
		}

		if (DebugMan.isDebugChannelEnabled(kMVDebugGUI)) {
			Common::Rect testBounds = _engine->getObjBounds(child);
			testBounds.translate(-targetData.scrollPos.x, -targetData.scrollPos.y);
			surface->frameRect(testBounds, kColorGreen);
		}
	}
	Common::Point composePosition = Common::Point(0, 0);
	surface->transBlitFrom(composeSurface, composePosition, kColorGreen);
}

void Gui::drawWindowTitle(WindowReference target, Graphics::ManagedSurface *surface) {
	// TODO: Implement when MacGui supports titles in windows with custom borders.
}

void Gui::drawDraggedObject() {
	if (_draggedObj.id != 0 &&
		_engine->isObjVisible(_draggedObj.id)) {
		ensureAssetLoaded(_draggedObj.id);
		ImageAsset *asset = _assets[_draggedObj.id];

		// In case of overflow from the right/top
		uint w = asset->getWidth() + MIN((int16)0, _draggedObj.pos.x);
		uint h = asset->getHeight() + MIN((int16)0, _draggedObj.pos.y);

		// In case of overflow from the bottom/left
		if (_draggedObj.pos.x > 0 && _draggedObj.pos.x + w > kScreenWidth) {
			w = kScreenWidth - _draggedObj.pos.x;
		}
		if (_draggedObj.pos.y > 0 && _draggedObj.pos.y + h > kScreenHeight) {
			h = kScreenHeight - _draggedObj.pos.y;
		}

		Common::Point target = _draggedObj.pos;
		if (target.x < 0) {
			target.x = 0;
		}
		if (target.y < 0) {
			target.y = 0;
		}

		_draggedSurface.create(w, h, _screen.format);
		_draggedSurface.blitFrom(
			_screen,
			Common::Rect(
				target.x,
				target.y,
				target.x + _draggedSurface.w,
				target.y + _draggedSurface.h),
			Common::Point(0, 0));
		asset->blitInto(&_draggedSurface, MIN((int16)0, _draggedObj.pos.x), MIN((int16)0, _draggedObj.pos.y), kBlitBIC);

		g_system->copyRectToScreen(
			_draggedSurface.getBasePtr(0, 0),
			_draggedSurface.pitch,
			target.x,
			target.y,
			_draggedSurface.w,
			_draggedSurface.h
		);
	}
}

void Gui::drawDialog() {
	if (_dialog) {
		_dialog->draw();
	}
}

void Gui::updateWindow(WindowReference winID, bool containerOpen) {
	if (winID == kNoWindow) {
		return;
	}
	if (winID == kSelfWindow || containerOpen) {
		WindowData &data = findWindowData(winID);
		if (winID == kCommandsWindow) {
			Common::Array<CommandButton>::iterator it = _controlData->begin();
			for (; it != _controlData->end(); ++it) {
				it->unselect();
			}
		}
		Common::Array<DrawableObject> &children = data.children;
		for (uint i = 0; i < children.size(); i++) {
			uint flag = 0;
			ObjID child = children[i].obj;
			BlitMode mode = kBlitDirect;
			bool off = !_engine->isObjVisible(child);
			// CHECKME: Since flag = 0, this always evaluates to false
			if (flag || !off || !_engine->isObjClickable(child)) {
				mode = kBlitBIC;
				if (off || flag) {
					mode = kBlitXOR;
				} else if (!off && _engine->isObjSelected(child)) {
					mode = kBlitOR;
				}
				children[i] = DrawableObject(child, mode);
			} else {
				children[i] = DrawableObject(child, kBlitXOR);
			}
		}
		if (winID == kMainGameWindow) {
			drawMainGameWindow();
		} else {
			Graphics::MacWindow *winRef = findWindow(winID);
			winRef->getWindowSurface()->fillRect(data.bounds, kColorGray80);
		}
		if (data.type == kZoomDoc && data.updateScroll) {
			warning("Unimplemented: update scroll");
		}
	}
}

void Gui::clearExits() {
	_exitsData->clear();
}

void Gui::unselectExits() {
	Common::Array<CommandButton>::const_iterator it = _exitsData->begin();
	for (; it != _exitsData->end(); ++it) {
		CommandButton button = *it;
		button.unselect();
	}
}

void Gui::updateExit(ObjID obj) {
	if (!_engine->isObjExit(obj)) {
		return;
	}

	int ctl = -1;
	int i = 0;
	Common::Array<CommandButton>::const_iterator it = _exitsData->begin();
	for (;it != _exitsData->end(); it++) {
		if ((ObjID)it->getData().refcon == obj)
			ctl = i;
		else
			i++;
	}

	if (ctl != -1)
		_exitsData->remove_at(ctl);

	if (!_engine->isHiddenExit(obj) &&
		_engine->getParent(obj) == _engine->getParent(1)) {
		ControlData data;
		data.titleLength = 0;
		data.refcon = (ControlAction)obj; // Objects can be exits (actions)
		Common::Point pos = _engine->getObjExitPosition(obj);
		//pos.x += border.leftOffset;
		//pos.y += border.topOffset;
		data.bounds = Common::Rect(pos.x, pos.y, pos.x + kExitButtonWidth, pos.y + kExitButtonHeight);
		data.visible = true;

		_exitsData->push_back(CommandButton(data, this));
	}
}

void Gui::printText(const Common::String &text) {
	debugC(1, kMVDebugGUI, "Print Text: %s", text.c_str());
	_consoleText->printLine(text, _outConsoleWindow->getInnerDimensions().width());
}

void Gui::showPrebuiltDialog(PrebuiltDialogs type) {
	closeDialog();
	_dialog = new Dialog(this, type);
}

bool Gui::isDialogOpen() {
	return _dialog != nullptr;
}

void Gui::setTextInput(const Common::String &str) {
	_engine->setTextInput(str);
}

void Gui::closeDialog() {
	delete _dialog;
	_dialog = nullptr;
}

void Gui::getTextFromUser() {
	if (_dialog) {
		delete _dialog;
	}
	showPrebuiltDialog(kSpeakDialog);
}

void Gui::loadGame() {
	_engine->scummVMSaveLoadDialog(false);
}

void Gui::saveGame() {
	_engine->scummVMSaveLoadDialog(true);
}

void Gui::newGame() {
	_engine->newGame();
}

void Gui::quitGame() {
	_engine->requestQuit();
}

void Gui::createInnerSurface(Graphics::ManagedSurface *innerSurface, Graphics::ManagedSurface *outerSurface, const BorderBounds &borders) {
	innerSurface->create(
		outerSurface->w,// - borders.leftOffset - borders.rightOffset,
		outerSurface->h,// - borders.topOffset - borders.bottomOffset,
		outerSurface->format);
}

void Gui::moveDraggedObject(Common::Point target) {
	ensureAssetLoaded(_draggedObj.id);
	_draggedObj.pos = target + _draggedObj.mouseOffset;

	// TODO FInd more elegant way of making pow2
	_draggedObj.hasMoved = (_draggedObj.startPos.sqrDist(_draggedObj.pos) >= (kDragThreshold * kDragThreshold));

	debugC(4, kMVDebugGUI, "Dragged obj position: (%d, %d), mouse offset: (%d, %d), hasMoved: %d, dist: %d, threshold: %d",
		_draggedObj.pos.x, _draggedObj.pos.y,
		_draggedObj.mouseOffset.x, _draggedObj.mouseOffset.y,
		_draggedObj.hasMoved,
		_draggedObj.startPos.sqrDist(_draggedObj.pos),
		kDragThreshold * kDragThreshold
	);

}

WindowReference Gui::findWindowAtPoint(Common::Point point) {
	Common::List<WindowData>::iterator it;
	Graphics::MacWindow *win = _wm.findWindowAtPoint(point);
	if (win != nullptr) {
		for (it = _windowData->begin(); it != _windowData->end(); it++) {
			if (win == findWindow(it->refcon) && it->refcon != kDiplomaWindow) { //HACK, diploma should be considered
				if (win->getDimensions().contains(point)) {
					return it->refcon;
				}
			}
		}
	}
	return kNoWindow;
}

Common::Point Gui::getGlobalScrolledSurfacePosition(WindowReference reference) {
	const WindowData &data = getWindowData(reference);
	//BorderBounds border = borderBounds(data.type);
	Graphics::MacWindow *win = findWindow(reference);
	if (!win) {
		return Common::Point(0, 0);
	}
	return Common::Point(
		win->getInnerDimensions().left - data.scrollPos.x,
		win->getInnerDimensions().top - data.scrollPos.y);
}

WindowData &Gui::findWindowData(WindowReference reference) {
	assert(_windowData);

	Common::List<WindowData>::iterator iter = _windowData->begin();
	while (iter->refcon != reference && iter != _windowData->end()) {
		iter++;
	}

	if (iter->refcon == reference)
		return *iter;

	error("GUI: Could not locate the desired window data");
}

Graphics::MacWindow *Gui::findWindow(WindowReference reference) {
	if (reference < 0x80 && reference >= kInventoryStart) { // It's an inventory window
		return _inventoryWindows[reference - kInventoryStart];
	}
	switch (reference) {
	case MacVenture::kNoWindow:
		return nullptr;
	case MacVenture::kCommandsWindow:
		return _controlsWindow;
	case MacVenture::kMainGameWindow:
		return _mainGameWindow;
	case MacVenture::kOutConsoleWindow:
		return _outConsoleWindow;
	case MacVenture::kSelfWindow:
		return _selfWindow;
	case MacVenture::kExitsWindow:
		return _exitsWindow;
	case MacVenture::kDiplomaWindow:
		return _diplomaWindow;
	default:
		return nullptr;
	}
	return nullptr;
}

void Gui::ensureInventoryOpen(WindowReference reference, ObjID id) {
	assert(reference < 0x80 && reference >= kInventoryStart);
	if (reference - kInventoryStart == (int)_inventoryWindows.size()) {
		createInventoryWindow(id);
	}
}

WindowReference Gui::getObjWindow(ObjID objID) {
	switch (objID) {
	case 0xfffc: return kExitsWindow;
	case 0xfffd: return kSelfWindow;
	case 0xfffe: return kOutConsoleWindow;
	case 0xffff: return kCommandsWindow;
	default: return findObjWindow(objID);
	}
}

WindowReference Gui::findObjWindow(ObjID objID) {
	// This is a bit of a HACK, we take advantage of the consecutive nature of references
	for (uint i = kCommandsWindow; i <= kDiplomaWindow; i++) {
		const WindowData &data = getWindowData((WindowReference)i);
		if (data.objRef == objID) {
			return data.refcon;
		}
	}

	for (uint i = kInventoryStart; i < _inventoryWindows.size() + kInventoryStart; i++) {
		const WindowData &data = getWindowData((WindowReference)i);
		if (data.objRef == objID) {
			return data.refcon;
		}
	}

	return kNoWindow;
}

void Gui::checkSelect(const WindowData &data, Common::Point pos, const Common::Rect &clickRect, WindowReference ref) {
	ObjID child = 0;
	for (Common::Array<DrawableObject>::const_iterator it = data.children.begin(); it != data.children.end(); it++) {
		if (canBeSelected((*it).obj, clickRect, ref)) {
			child = (*it).obj;
		}
	}
	if (child != 0) {
		selectDraggable(child, ref, pos);
		bringToFront(ref);
	}
}

bool Gui::canBeSelected(ObjID obj, const Common::Rect &clickRect, WindowReference ref) {
	return (_engine->isObjClickable(obj) &&
			isRectInsideObject(clickRect, obj));
}

bool Gui::isRectInsideObject(Common::Rect target, ObjID obj) {
	ensureAssetLoaded(obj);
	Common::Rect bounds = _engine->getObjBounds(obj);
	Common::Rect intersection = bounds.findIntersectingRect(target);
	// We translate it to the image's coord system
	intersection = Common::Rect(
		intersection.left - bounds.left,
		intersection.top - bounds.top,
		intersection.left - bounds.left + intersection.width(),
		intersection.top - bounds.top + intersection.height());

	return _assets[obj]->isRectInside(intersection);
}

void Gui::selectDraggable(ObjID child, WindowReference origin, Common::Point click) {
	if (_engine->isObjClickable(child) && _draggedObj.id == 0) {
		_draggedObj.hasMoved = false;
		_draggedObj.id = child;
		_draggedObj.startWin = origin;
		Common::Point localizedClick = click - getGlobalScrolledSurfacePosition(origin);
		_draggedObj.mouseOffset = _engine->getObjPosition(child) - localizedClick;
		_draggedObj.pos = click + _draggedObj.mouseOffset;
		_draggedObj.startPos = _draggedObj.pos;
	}
}

void Gui::handleDragRelease(bool shiftPressed, bool isDoubleClick) {
	if (_draggedObj.id != 0) {
		WindowReference destinationWindow = findWindowAtPoint(_draggedObj.pos - _draggedObj.mouseOffset);
		if (destinationWindow == kNoWindow) {
			return;
		}
		if (_draggedObj.hasMoved) {
			const WindowData &destinationWindowData = getWindowData(destinationWindow);
			ObjID destObject = destinationWindowData.objRef;
			Common::Point dropPosition = _draggedObj.pos - _draggedObj.startPos;
			dropPosition = localizeTravelledDistance(dropPosition, _draggedObj.startWin, destinationWindow);
			debugC(3, kMVDebugGUI, "Drop the object %d at obj %d, pos (%d, %d)", _draggedObj.id, destObject, dropPosition.x, dropPosition.y);

			_engine->handleObjectDrop(_draggedObj.id, dropPosition, destObject);
		}
		_engine->handleObjectSelect(_draggedObj.id, destinationWindow, shiftPressed, isDoubleClick);
		_draggedObj.id = 0;
		_draggedObj.hasMoved = false;
	}
}

Common::Rect Gui::calculateClickRect(Common::Point clickPos, Common::Rect windowBounds) {
	int left = clickPos.x - windowBounds.left;
	int top = clickPos.y - windowBounds.top;
	return Common::Rect(left - kCursorWidth, top - kCursorHeight, left + kCursorWidth, top + kCursorHeight);
}

Common::Point Gui::localizeTravelledDistance(Common::Point point, WindowReference origin, WindowReference target) {
	if (origin != target) {
		// ori.local to global
		point += getGlobalScrolledSurfacePosition(origin);
		if (findWindow(target)) {
			// dest.globalToLocal
			point -= getGlobalScrolledSurfacePosition(target);
		}
	}
	return point;
}

void Gui::removeInventoryWindow(WindowReference ref) {
	_inventoryWindows.remove_at(ref - kInventoryStart);
	Common::List<WindowData>::iterator it;
	for (it = _windowData->begin(); it != _windowData->end(); it++) {
		if (it->refcon == ref) {
			_windowData->erase(it);
			break;
		}
	}
}


/* HANDLERS */
void Gui::handleMenuAction(MenuAction action) {
	switch (action)	{
	case MacVenture::kMenuActionAbout:
		warning("Unimplemented MacVenture Menu Action: About");
		break;
	case MacVenture::kMenuActionNew:
		_engine->newGame();
		break;
	case MacVenture::kMenuActionOpen:
		loadGame();
		break;
	case MacVenture::kMenuActionSave:
		saveGame();
		break;
	case MacVenture::kMenuActionSaveAs:
		saveGame();
		break;
	case MacVenture::kMenuActionQuit:
		_engine->requestQuit();
		break;
	case MacVenture::kMenuActionUndo:
		warning("Unimplemented MacVenture Menu Action: Undo");
		break;
	case MacVenture::kMenuActionCut:
		warning("Unimplemented MacVenture Menu Action: Cut");
		break;
	case MacVenture::kMenuActionCopy:
		warning("Unimplemented MacVenture Menu Action: Copy");
		break;
	case MacVenture::kMenuActionPaste:
		warning("Unimplemented MacVenture Menu Action: Paste");
		break;
	case MacVenture::kMenuActionClear:
		warning("Unimplemented MacVenture Menu Action: Clear");
		break;
	case MacVenture::kMenuActionCleanUp:
		warning("Unimplemented MacVenture Menu Action: Clean Up");
		break;
	case MacVenture::kMenuActionMessUp:
		warning("Unimplemented MacVenture Menu Action: Mess Up");
		break;
	case MacVenture::kMenuActionCommand:
		warning("Unimplemented MacVenture Menu Action: GENERIC");
		break;
	default:
		break;
	}
}

/* CALLBACKS */

bool commandsWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;
	return g->processCommandEvents(click, event);
}

bool mainGameWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;
	return g->processMainGameEvents(click, event);
}

bool outConsoleWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;
	return g->processOutConsoleEvents(click, event);
}

bool selfWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processSelfEvents(click, event);
}

bool exitsWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processExitsEvents(click, event);
}

bool diplomaWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processDiplomaEvents(click, event);
}

bool inventoryWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processInventoryEvents(click, event);
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	Gui *g = (Gui *)data;

	g->handleMenuAction((MenuAction)action);
}


void Gui::invertWindowColors(WindowReference winID) {
	Graphics::ManagedSurface *srf = findWindow(winID)->getWindowSurface();
	for (int y = 0; y < srf->h; y++) {
		for (int x = 0; x < srf->w; x++) {
			byte p = *(byte *)srf->getBasePtr(x, y);
			*(byte *)srf->getBasePtr(x, y) =
				(p == kColorWhite) ? kColorBlack : kColorGray80;
		}
	}
}

bool Gui::tryCloseWindow(WindowReference winID) {
	//WindowData data = findWindowData(winID);
	Graphics::MacWindow *win = findWindow(winID);
	_wm.removeWindow(win);
	if (winID < 0x80) {
		removeInventoryWindow(winID);
	}
	return true;
}

Common::Point Gui::getObjMeasures(ObjID obj) {
	ensureAssetLoaded(obj);
 	int w = _assets[obj]->getWidth();
	int h = _assets[obj]->getHeight();
	return Common::Point(w, h);
}

bool Gui::processEvent(Common::Event &event) {
	bool processed = false;

	processed |= _cursor->processEvent(event);

	if (_dialog && _dialog->processEvent(event)) {
		return true;
	}

	if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_draggedObj.id != 0) {
			moveDraggedObject(event.mouse);
		}
		processed = true;
	}

	processed |= _wm.processEvent(event);
	return (processed);
}

bool Gui::processCommandEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		if (_engine->needsClickToContinue()) {
			_engine->selectControl(kClickToContinue);
			return true;
		}
		BorderBounds bbs = borderBounds(findWindowData(kCommandsWindow).type);
		Common::Point position(
			event.mouse.x - bbs.leftOffset,
			event.mouse.y - bbs.topOffset
		);

		CommandButton data;
		if (!_controlData)
			return false;
		bool flag = false;
		Common::Array<CommandButton>::iterator it = _controlData->begin();
		for (; it != _controlData->end(); ++it) {
			if (it->isInsideBounds(position)) {
				it->select();
				data = *it;
				flag = true;
			} else {
				it->unselect();
			}
		}
		if (flag)
			_engine->selectControl(data.getData().refcon);
		_engine->refreshReady();
		_engine->preparedToRun();
	}
	return false;
}

bool MacVenture::Gui::processMainGameEvents(WindowClick click, Common::Event &event) {
	if (_engine->needsClickToContinue())
		return true;

	return false;
}

bool MacVenture::Gui::processOutConsoleEvents(WindowClick click, Common::Event &event) {
	if (_engine->needsClickToContinue())
		return true;

	if (click == kBorderScrollUp && event.type == Common::EVENT_LBUTTONDOWN) {
		_consoleText->scrollUp();
		return true;
	}
	if (click == kBorderScrollDown && event.type == Common::EVENT_LBUTTONDOWN) {
		_consoleText->scrollDown();
		return true;
	}

	return getWindowData(kOutConsoleWindow).visible;
}

bool MacVenture::Gui::processSelfEvents(WindowClick click, Common::Event &event) {
	if (_engine->needsClickToContinue())
		return true;

	if (event.type == Common::EVENT_LBUTTONUP) {
		_engine->handleObjectSelect(1, kSelfWindow, false, false);
	}
	return true;
}

bool MacVenture::Gui::processExitsEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		if (_engine->needsClickToContinue()) {
			return true;
		}

		BorderBounds bbs = borderBounds(findWindowData(kExitsWindow).type);
		Common::Point position(
			event.mouse.x - bbs.leftOffset,
			event.mouse.y - bbs.topOffset);

		CommandButton button;
		if (!_exitsData)
			return false;

		Common::Array<CommandButton>::iterator it = _exitsData->begin();
		for (; it != _exitsData->end(); ++it) {
			if (it->isInsideBounds(position)) {
				it->select();
				button = *it;
				_engine->handleObjectSelect(button.getData().refcon, kExitsWindow, false, false);
				return true;
			} else {
				it->unselect();
			}
		}

	}
	return getWindowData(kExitsWindow).visible;
}

bool MacVenture::Gui::processDiplomaEvents(WindowClick click, Common::Event &event) {
	if (_engine->needsClickToContinue())
		return true;

	return getWindowData(kDiplomaWindow).visible;
}

bool Gui::processInventoryEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONDOWN && click == kBorderCloseButton) {
		WindowReference ref = findWindowAtPoint(event.mouse);
		if (ref == kNoWindow) {
			return false;
		}

		if (click == kBorderCloseButton) {
			removeInventoryWindow(ref);
			return true;
		}
	}

	if (_engine->needsClickToContinue())
		return true;

	if (event.type == Common::EVENT_LBUTTONDOWN) {
		// Find the appropriate window
		WindowReference ref = findWindowAtPoint(event.mouse);
		if (ref == kNoWindow) {
			return false;
		}

		WindowData &data = findWindowData((WindowReference) ref);

		if (click == kBorderScrollUp) {
			data.scrollPos.y = MAX(0, data.scrollPos.y - kScrollAmount);
		}
		if (click == kBorderScrollDown) {
			data.scrollPos.y += kScrollAmount;
		}
		if (click == kBorderScrollLeft) {
			data.scrollPos.x = MAX(0, data.scrollPos.x - kScrollAmount);
		}
		if (click == kBorderScrollRight) {
			data.scrollPos.x += kScrollAmount;
		}
	}
	return true;
}

void Gui::selectForDrag(Common::Point cursorPosition) {
	WindowReference ref = findWindowAtPoint(cursorPosition);
	if (ref == kNoWindow) {
		return;
	}
	if (_engine->needsClickToContinue())
		return;

	Graphics::MacWindow *win = findWindow(ref);
	WindowData &data = findWindowData((WindowReference)ref);

	Common::Rect clickRect = calculateClickRect(cursorPosition + data.scrollPos, win->getInnerDimensions());
	checkSelect(data, cursorPosition, clickRect, (WindowReference)ref);
}

void Gui::handleSingleClick() {
	debugC(2, kMVDebugGUI, "Registered Single Click");
	// HACK THERE HAS TO BE A MORE ELEGANT WAY
	if (_dialog) {
		return;
	}
	handleDragRelease(false, false);
}

void Gui::handleDoubleClick() {
	debugC(2, kMVDebugGUI, "Registered Double Click");
	if (_dialog) {
		return;
	}
	handleDragRelease(false, true);
}

void Gui::ensureAssetLoaded(ObjID obj) {
	if (!_assets.contains(obj)) {
		_assets[obj] = new ImageAsset(obj, _graphics);
	}
}


} // End of namespace MacVenture
