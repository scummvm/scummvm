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
#include "graphics/macgui/mactextwindow.h"

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

	{ kMenuSpecial,		"Clean Up",			kMenuActionCleanUp, 0, true },
	{ kMenuSpecial,		"Mess Up",			kMenuActionMessUp, 0, true },

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
	_dialog = nullptr;

	_activeWinRef = kNoWindow;

	_cursor = new Cursor(this);

	_consoleText = new ConsoleText(this);
	_graphics = nullptr;
	_diplomaImage = nullptr;
	_diplomaWindow = nullptr;

	_lassoStart = Common::Point(0, 0);
	_lassoEnd = Common::Point(0, 0);
	_lassoBeingDrawn = false;
	_lassoWinRef = WindowReference(0);

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

	drawDraggedObjects();
	drawDialog();
	// TODO: When window titles with custom borders are in MacGui, this should be used.
	//drawWindowTitle(kMainGameWindow, _mainGameWindow->getWindowSurface());
}

void Gui::drawMenu() {
	_menu->draw(&_screen);
}

bool Gui::decodeStartupScreen() {
	Common::SeekableReadStream *stream = Common::MacResManager::openFileOrDataFork("StartupScreen");

	if (!stream)
		return false;

	for (int y = 0; y < kScreenHeight; y++) {
		for (int x = 0; x < kScreenWidth / 8; x++) {
			byte b = stream->readByte();

			for (int z = 0; z < 8; z++) {
				_screen.setPixel(8 * x + z, y, (b & (0x80 >> z)) ? kColorBlack : kColorWhite);
			}
		}
	}

	return true;
}

bool Gui::decodeTitleScreen() {
	Common::MacResManager resMan;
	Common::Path titlePath = _engine->getFilePath(kTitlePathID);

	if (resMan.open(titlePath)) {
		Common::SeekableReadStream *stream = resMan.getResource(MKTAG('P', 'P', 'I', 'C'), 0);

		if (stream) {
			// New PPICT title screen
			ImageAsset *title = new ImageAsset(stream);

			_screen.fillRect(Common::Rect(kScreenWidth, kScreenHeight), kColorBlack);
			title->blitInto(&_screen, 0, (kScreenHeight - title->getHeight()) / 2, kBlitDirect);

			delete title;
		} else {
			// Old PACK title screen
			stream = Common::MacResManager::openFileOrDataFork(titlePath);
			if (!stream)
				return false;

			stream->seek(0x200);

			for (int y = 0; y < 302; y++) {
				byte line[72];
				int count = 0;

				while (count < 72) {
					byte n = stream->readByte();

					if (n == 0x80) {
						// Do nothing
					} else if (n & 0x80) {
						n = (n ^ 0xFF) + 2;
						byte v = stream->readByte();

						while (n--)
							line[count++] = v;
					} else {
						n++;
						while (n--)
							line[count++] = stream->readByte();
					}
				}

				for (int x = 0; x < 512; x++)
					_screen.setPixel(x, y, (line[x / 8] & (0x80 >> (x % 8))) ? kColorBlack : kColorWhite);
			}

			delete stream;
		}
	}

	return true;
}

bool Gui::displayTitleScreenAndWait(uint32 ms) {
	g_system->copyRectToScreen(_screen.getPixels(), kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);

	uint32 now = g_system->getMillis();
	while (g_system->getMillis() < now + 3000) {
		if (_engine->shouldQuit())
			return false;

		Common::Event event;

		while (_engine->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				return false;
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return true;
}

void Gui::drawTitle() {
	bool success = true;

	_wm.pushCursor(Graphics::kMacCursorOff);

	if (decodeStartupScreen())
		success = displayTitleScreenAndWait(4000);

	if (success) {
		if (decodeTitleScreen())
			displayTitleScreenAndWait(4000);
	}

	_wm.popCursor();
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
	_controlsWindow->resizeInner(bounds.width(), bounds.height());
	_controlsWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_controlsWindow->setActive(false);
	_controlsWindow->setCallback(commandsWindowCallback, this);

	// Main Game Window
	_mainGameWindow = _wm.addWindow(false, false, false);
	bounds = getWindowData(kMainGameWindow).bounds;
	bbs = borderBounds(findWindowData(kMainGameWindow).type);

	loadBorders(_mainGameWindow, findWindowData(kMainGameWindow).type);
	_mainGameWindow->resizeInner(bounds.width(), bounds.height());
	_mainGameWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_mainGameWindow->setActive(false);
	_mainGameWindow->setCallback(mainGameWindowCallback, this);

	// In-game Output Console
	bounds = getWindowData(kOutConsoleWindow).bounds;
	bbs = borderBounds(findWindowData(kOutConsoleWindow).type);
	_outConsoleWindow = _wm.addTextWindow(&getCurrentFont(), kColorBlack, kColorWhite,
										  bounds.width() - bbs.rightScrollbarWidth, Graphics::kTextAlignLeft, _menu, false);
	_outConsoleWindow->enableScrollbar(true);
	_outConsoleWindow->setEditable(false);
	loadBorders(_outConsoleWindow, findWindowData(kOutConsoleWindow).type);
	_outConsoleWindow->setDimensions(bounds);
	_outConsoleWindow->resizeInner(bounds.width() - bbs.rightScrollbarWidth, bounds.height() - bbs.bottomScrollbarHeight);
	_outConsoleWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);
	_outConsoleWindow->setActive(false);
	_outConsoleWindow->setCallback(outConsoleWindowCallback, this);
	_outConsoleWindow->setTitle("Untitled");
	// Self Window
	_selfWindow = _wm.addWindow(false, true, false);

	bounds = getWindowData(kSelfWindow).bounds;
	bbs = borderBounds(findWindowData(kSelfWindow).type);
	loadBorders(_selfWindow, findWindowData(kSelfWindow).type);
	_selfWindow->resizeInner(bounds.width(), bounds.height());
	_selfWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_selfWindow->setActive(false);
	_selfWindow->setCallback(selfWindowCallback, this);

	// Exits Window
	_exitsWindow = _wm.addWindow(false, false, false);

	bounds = getWindowData(kExitsWindow).bounds;
	bbs = borderBounds(findWindowData(kExitsWindow).type);
	loadBorders(_exitsWindow, findWindowData(kExitsWindow).type);
	_exitsWindow->resizeInner(bounds.width(), bounds.height());
	_exitsWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);

	_exitsWindow->setActive(false);
	_exitsWindow->setCallback(exitsWindowCallback, this);
	_exitsWindow->setTitle(findWindowData(kExitsWindow).title);
	_exitsWindow->setBackgroundPattern(kPatternLightGray);
}

const WindowData &Gui::getWindowData(WindowReference reference) {
	return findWindowData(reference);
}

const Graphics::Font &Gui::getCurrentFont() {
	return *_wm._fontMan->getFont(Graphics::MacFont(Graphics::kMacFontSystem, 12));
}

void Gui::bringToFront(WindowReference winID) {
	if (winID != kNoWindow) {
		_wm.setActiveWindow(findWindow(winID)->getId());
	}
	_activeWinRef = winID;

	Graphics::MacMenuItem *specialItem = _menu->getMenuItem("Special");

	if (winID >= kInventoryStart && winID < 0x80) { // if inventory window
		_wm.setMenuItemEnabled(specialItem, true);
	} else {
		_wm.setMenuItemEnabled(specialItem, false);
	}
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
	WindowData &winData = findWindowData(target);
	winData.children.push_back(DrawableObject(child, kBlitBIC));
	winData.updateScroll = true;
	_engine->updateWindow(winData.refcon);
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
	if (!_objToInvRef.contains(objRef)) {
		_objToInvRef[objRef] = (WindowReference)(_inventoryWindows.size() + kInventoryStart); // This is a HACK
		newData.refcon = _objToInvRef[objRef];
	} else {
		newData.refcon = _objToInvRef[objRef];
	}

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
	newWindow->resizeInner(newData.bounds.width(), newData.bounds.height() - bbs.bottomScrollbarHeight);
	newWindow->move(newData.bounds.left - bbs.leftOffset, newData.bounds.top - bbs.topOffset);
	newWindow->setCallback(inventoryWindowCallback, new InventoryCallbackStruct{this, newData.refcon});
	//newWindow->setCloseable(true);

	_inventoryWindows.push_back(InventoryWindowData{newWindow, newData.refcon});

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

void Gui::loadDiploma() {
	// Dialog
	closeDialog();
	_dialog = new Dialog(this, _resourceManager, kDialogBoxDiplomaID);
	DialogElement *quitButton = _dialog->getElement("Quit");
	quitButton->setAction(kDAQuit);

	// Image
	if (!_diplomaImage) {
		Common::Path diplomaFilePath = _engine->getDiplomaFileName();
		_resourceManager->open(diplomaFilePath);
		Common::SeekableReadStream *stream = _resourceManager->getResource(MKTAG('P', 'P', 'I', 'C'), 0);
		_diplomaImage = new ImageAsset(stream);
	}

	// Diploma Window
	closeAllWindows();
	_diplomaWindow = _wm.addWindow(false, false, false);
	Common::Rect bounds = getWindowData(kDiplomaWindow).bounds;
	BorderBounds bbs = borderBounds(findWindowData(kDiplomaWindow).type);
	loadBorders(_diplomaWindow, findWindowData(kDiplomaWindow).type);
	_diplomaWindow->resizeInner(bounds.width(), bounds.height());
	_diplomaWindow->move(bounds.left - bbs.leftOffset, bounds.top - bbs.topOffset);
	_diplomaWindow->setActive(true);
	_diplomaWindow->setCallback(diplomaWindowCallback, this);
}

void Gui::clearAssets() {
	Common::HashMap<ObjID, ImageAsset*>::const_iterator it = _assets.begin();
	for (; it != _assets.end(); it++) {
		delete it->_value;
	}
	_assets.clear();

	if (_diplomaImage) {
		delete _diplomaImage;
		_diplomaImage = nullptr;
	}
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
		data.objRef = 0;
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
	if (_engine->getGameState() == kGameStateWinning) {
		drawDiplomaWindow();
	} else {
		drawCommandsWindow();
		drawMainGameWindow();
		drawSelfWindow();
		drawInventories();
		drawExitsWindow();
		drawConsoleWindow();
	}

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

void Gui::drawDiplomaWindow() {
	_diplomaWindow->setDirty(true);

	_diplomaImage->blitInto(
		_diplomaWindow->getWindowSurface(),
		0,
		0,
		kBlitDirect);

	findWindow(kDiplomaWindow)->setDirty(true);
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
	Graphics::ManagedSurface *srf = _selfWindow->getWindowSurface();
	srf->fillRect(Common::Rect(0, 0, srf->w, srf->h), kColorWhite);
	drawObjectsInWindow(getWindowData(kSelfWindow), srf);
	findWindow(kSelfWindow)->setDirty(true);
}

void Gui::drawInventories() {

	Graphics::ManagedSurface *srf;
	for (auto &invWinData: _inventoryWindows) {
		Graphics::MacWindow *win = invWinData.win;
		WindowData data = getWindowData(invWinData.ref);

		srf = win->getWindowSurface();
		srf->clear(kColorGreen);
		srf->fillRect(srf->getBounds(), kColorWhite);
		drawObjectsInWindow(data, srf);

		if (data.refcon == _lassoWinRef && _lassoBeingDrawn) {
			Common::Point topLeft(_lassoStart);
			topLeft.y -= 12;
			Common::Point bottomRight(_lassoEnd);
			bottomRight.y -= 12;
			if (topLeft.x > bottomRight.x)
				SWAP(topLeft.x, bottomRight.x);
			if (topLeft.y > bottomRight.y)
				SWAP(topLeft.y, bottomRight.y);
			Common::Rect lassoRect(topLeft, bottomRight);

			Graphics::MacPlotData plotData(srf, nullptr, &_wm.getBuiltinPatterns(), kPatternCheckers2, 0, 0, 1, kColorWhite, false);
			Graphics::Primitives &primitives = _wm.getDrawPrimitives();
			primitives.drawRect(lassoRect, kColorBlack, &plotData);
		}

		if (DebugMan.isDebugChannelEnabled(kMVDebugGUI)) {
			Common::Rect innerDims = win->getInnerDimensions();
			innerDims.translate(-innerDims.left, -innerDims.top);
			srf->frameRect(innerDims, kColorGreen);
		}

		invWinData.win->setDirty(true);
	}

}

void Gui::drawExitsWindow() {
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

		uint flag = 0;
		bool off = !_engine->isObjVisible(child);
		if (flag || !off || !_engine->isObjClickable(child)) {
			mode = kBlitBIC;
			if (off || flag) {
				mode = kBlitXOR;
			} else if (!off && _engine->isObjSelected(child)) {
				mode = kBlitOR;
			}
		}

		_assets[child]->blitInto(
			&composeSurface,
			pos.x,
			pos.y,
			mode);

		if (_engine->isObjVisible(child)) {
			if (_engine->isObjSelected(child) ||
				(_draggedObjects.size() && child == _draggedObjects[0].id)) {

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

void Gui::drawDraggedObjects() {
	for (uint i = 0; i < _draggedObjects.size(); i++) {
		if (_draggedObjects[i].id != 0 &&
			_engine->isObjVisible(_draggedObjects[i].id) && _draggedObjects[i].hasMoved) {
			ensureAssetLoaded(_draggedObjects[i].id);
			ImageAsset *asset = _assets[_draggedObjects[i].id];

			// In case of overflow from the right/top
			uint w = asset->getWidth() + MIN((int16)0, _draggedObjects[i].pos.x);
			uint h = asset->getHeight() + MIN((int16)0, _draggedObjects[i].pos.y);

			// In case of overflow from the bottom/left
			if (_draggedObjects[i].pos.x > 0 && _draggedObjects[i].pos.x + w > kScreenWidth) {
				w = kScreenWidth - _draggedObjects[i].pos.x;
			}
			if (_draggedObjects[i].pos.y > 0 && _draggedObjects[i].pos.y + h > kScreenHeight) {
				h = kScreenHeight - _draggedObjects[i].pos.y;
			}

			Common::Point target = _draggedObjects[i].pos;
			if (target.x < 0) {
				target.x = 0;
			}
			if (target.y < 0) {
				target.y = 0;
			}

			_draggedSurfaces[i].create(w, h, _screen.format);
			_draggedSurfaces[i].blitFrom(
				_screen,
				Common::Rect(
					target.x,
					target.y,
					target.x + _draggedSurfaces[i].w,
					target.y + _draggedSurfaces[i].h),
				Common::Point(0, 0));
			asset->blitInto(&_draggedSurfaces[i], MIN((int16)0, _draggedObjects[i].pos.x), MIN((int16)0, _draggedObjects[i].pos.y), kBlitBIC);

			g_system->copyRectToScreen(
				_draggedSurfaces[i].getBasePtr(0, 0),
				_draggedSurfaces[i].pitch,
				target.x,
				target.y,
				_draggedSurfaces[i].w,
				_draggedSurfaces[i].h
			);
		}
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

void Gui::clearDraggedObjects() {
	_draggedObjects.clear();
	_draggedSurfaces.clear();
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

void Gui::resetExitBackgroundPattern() {
	if (!_exitsWindow)
		return;
	_exitsWindow->setBackgroundPattern(kPatternLightGray);
}

Common::String Gui::getConsoleText() const {
	return Common::String(_outConsoleWindow->getTextChunk(0, 0, -1, -1));
}

void Gui::setConsoleText(const Common::String &text) {
	_outConsoleWindow->clearText();

	_outConsoleWindow->setEditable(true);
	_outConsoleWindow->appendText(text);
	_outConsoleWindow->setEditable(false);
}

void Gui::printText(const Common::String &text) {
	debugC(1, kMVDebugGUI, "Print Text: %s", text.c_str());
	// WORKAROUND: Make sure newly added line is visible by
	// making the window editable for the moment
	_outConsoleWindow->setEditable(true);
	_outConsoleWindow->appendText(text + '\n');
	_outConsoleWindow->setEditable(false);
}

void Gui::showPrebuiltDialog(PrebuiltDialogs type, const Common::String &title) {
	closeDialog();
	_dialog = new Dialog(this, type, title);
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

void Gui::getTextFromUser(Common::String &title) {
	if (_dialog) {
		delete _dialog;
	}
	showPrebuiltDialog(kSpeakDialog, title);
}

void Gui::loadGame() {
	if (g_engine->canLoadGameStateCurrently())
		_engine->scummVMSaveLoadDialog(false);
}

void Gui::saveGame() {
	if (g_engine->canSaveAutosaveCurrently())
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

void Gui::moveDraggedObjects(Common::Point target) {
	for (auto &obj: _draggedObjects) {
		ensureAssetLoaded(obj.id);
		obj.pos = target + obj.mouseOffset;

		// TODO FInd more elegant way of making pow2
		obj.hasMoved = (obj.startPos.sqrDist(obj.pos) >= (kDragThreshold * kDragThreshold));

		debugC(4, kMVDebugGUI, "Dragged obj position: (%d, %d), mouse offset: (%d, %d), hasMoved: %d, dist: %d, threshold: %d",
			obj.pos.x, obj.pos.y,
			obj.mouseOffset.x, obj.mouseOffset.y,
			obj.hasMoved,
			obj.startPos.sqrDist(obj.pos),
			kDragThreshold * kDragThreshold
		);
	}

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
		for (auto &invWindowData: _inventoryWindows) {
			if (reference == invWindowData.ref)
				return invWindowData.win;
		}
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

void Gui::checkSelect(const WindowData &data, Common::Point pos, const Common::Rect &clickRect, WindowReference ref, bool shiftPressed, bool isDoubleClick) {
	ObjID child = 0;
	for (Common::Array<DrawableObject>::const_iterator it = data.children.begin(); it != data.children.end(); it++) {
		if (canBeSelected((*it).obj, clickRect, ref)) {
			child = (*it).obj;
		}
	}
	if (child != 0 || data.refcon == kMainGameWindow) {
		if (!isDoubleClick)
			selectDraggable(child, ref, pos);
		_engine->handleObjectSelect(child, ref, shiftPressed, isDoubleClick);
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
	if (_engine->isObjClickable(child) && !_draggedObjects.size()) {
		if (!_engine->getSelectedObjects().size()) {
			_engine->getSelectedObjects().push_back(child);
		}

		for (auto &selObj: _engine->getSelectedObjects()) {
			if (_engine->isObjDraggable(selObj)) {
				DraggedObj obj;
				obj.hasMoved = false;
				obj.id = selObj;
				obj.startWin = origin;
				Common::Point localizedClick = click - getGlobalScrolledSurfacePosition(origin);
				obj.mouseOffset = _engine->getObjPosition(selObj) - localizedClick;
				obj.pos = click + obj.mouseOffset;
				obj.startPos = obj.pos;

				_draggedObjects.push_back(obj);
				_draggedSurfaces.push_back(Graphics::ManagedSurface());
			}
		}
		_engine->getSelectedObjects().clear();
	}
}

void Gui::handleDragRelease(bool shiftPressed, bool isDoubleClick) {
	if (_draggedObjects.size()) {
		for (auto &obj : _draggedObjects) {
			if (obj.id != 0) {
				WindowReference destinationWindow = findWindowAtPoint(obj.pos - obj.mouseOffset);
				if (destinationWindow == kNoWindow) {
					return;
				}
				if (obj.hasMoved) {
					const WindowData &destinationWindowData = getWindowData(destinationWindow);
					ObjID destObject = destinationWindowData.objRef;
					Common::Point dropPosition = obj.pos - obj.startPos;
					dropPosition = localizeTravelledDistance(dropPosition, obj.startWin, destinationWindow);
					debugC(3, kMVDebugGUI, "Drop the object %d at obj %d, pos (%d, %d)", obj.id, destObject, dropPosition.x, dropPosition.y);

					_engine->handleObjectDrop(obj.id, dropPosition, destObject);
				}
				if (isDoubleClick)
					_engine->handleObjectSelect(obj.id, destinationWindow, shiftPressed, isDoubleClick);
				obj.id = 0;
				obj.hasMoved = false;
			}
		}
		clearDraggedObjects();
	} else {
		WindowReference destinationWindow = findWindowAtPoint(_cursor->getPos());
		if (destinationWindow == kNoWindow) {
			return;
		}
		if (isDoubleClick) {
			WindowData &data = findWindowData((WindowReference)destinationWindow);
			Graphics::MacWindow *win = findWindow(destinationWindow);

			ObjID child = 0;
			Common::Rect clickRect = calculateClickRect(_cursor->getPos() + data.scrollPos, win->getInnerDimensions());

			for (Common::Array<DrawableObject>::const_iterator it = data.children.begin(); it != data.children.end(); it++) {
				if (canBeSelected((*it).obj, clickRect, destinationWindow)) {
					child = (*it).obj;
				}
			}

			_engine->handleObjectSelect(child, destinationWindow, shiftPressed, isDoubleClick);
		}
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
	for (auto &invWinData: _inventoryWindows) {
		if (invWinData.ref == ref) {
			_inventoryWindows.erase(&invWinData);
			break;
		}
	}

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
		_engine->cleanUp(_activeWinRef);
		break;
	case MacVenture::kMenuActionMessUp:
		_engine->messUp(_activeWinRef);
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

bool inventoryWindowCallback(Graphics::WindowClick click, Common::Event &event, void *data) {
	InventoryCallbackStruct *g = (InventoryCallbackStruct *)data;
	bool res = g->gui->processInventoryEvents(g->ref, click, event);

	if (event.type == Common::EVENT_LBUTTONUP && click == Graphics::kBorderCloseButton)
		delete g;

	return res;
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
	if (win) {
		_wm.removeWindow(win);
		if (winID < 0x80) {
			removeInventoryWindow(winID);
		}
	}
	return true;
}

bool Gui::tryCloseWindowRec(WindowReference ref, bool runControl) {
	WindowData data = findWindowData(ref);
	if (data.children.size()) {
		for (auto &child: data.children) {
			if (_objToInvRef.contains(child.obj)) { // child is inv window
				if (findWindow(_objToInvRef[child.obj]) == nullptr)
					continue;
				tryCloseWindowRec(_objToInvRef[child.obj], runControl);
			}
		}
	}

	if (runControl) {
		// HACK: Run script with "close" action
		ScriptEngine *scriptEngine = _engine->getScriptEngine();
		scriptEngine->runControl(kClose, data.objRef, 0, {0, 0});
	}

	return tryCloseWindow(ref);
}

void Gui::resetWindows() {
	for (auto &invWin: _inventoryWindows) {
		tryCloseWindow(invWin.ref);
	}
	// WORKAROUND: Set main game window data's objRef
	// to zero, instead of destroying the window and its data.
	WindowData &data = findWindowData(kMainGameWindow);
	data.objRef = 0;
}

void Gui::closeAllWindows() {
	resetWindows();
	_mainGameWindow->setVisible(false);
	_exitsWindow->setVisible(false);
	_outConsoleWindow->setVisible(false);
	_selfWindow->setVisible(false);
	_controlsWindow->setVisible(false);
	if (_diplomaWindow)
		_diplomaWindow->setVisible(false);
}

void Gui::highlightExitButton(ObjID objID) {
	if (!_exitsData)
		return;

	Common::Array<CommandButton>::iterator it = _exitsData->begin();
	for (; it != _exitsData->end(); ++it) {
		if (it->getData().refcon == (int)objID)
			it->select();
		else
			it->unselect();
	}
	if (objID == _engine->getParent(1)) {
		_exitsWindow->setBackgroundPattern(kPatternDarkGray);
	} else {
		_exitsWindow->setBackgroundPattern(kPatternLightGray);
	}
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
		if (_draggedObjects.size() && _draggedObjects[0].id != 0) {
			moveDraggedObjects(event.mouse);
		}
		processed = true;
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		clearDraggedObjects();
	}

	processed |= _wm.processEvent(event);
	return (processed);
}

bool Gui::processCommandEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		bringToFront(kOutConsoleWindow);

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
	if (event.type == Common::EVENT_LBUTTONUP) {
		bringToFront(kMainGameWindow);
	}

	if (_engine->needsClickToContinue())
		return true;

	return false;
}

bool MacVenture::Gui::processOutConsoleEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		bringToFront(kOutConsoleWindow);
		if (click == kBorderResizeButton) {
			_outConsoleWindow->setDimensions(_outConsoleWindow->getDimensions());
		}
	}

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
		bringToFront(kSelfWindow);

		_engine->handleObjectSelect(1, kSelfWindow, false, false);
	}
	return true;
}

bool MacVenture::Gui::processExitsEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		bringToFront(kExitsWindow);

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
	if (event.type == Common::EVENT_LBUTTONUP) {
		bringToFront(kDiplomaWindow);
	}

	if (_engine->needsClickToContinue())
		return true;

	return getWindowData(kDiplomaWindow).visible;
}

bool Gui::processInventoryEvents(WindowReference ref, WindowClick click, Common::Event &event) {
	if (click == kBorderCloseButton) {
		if (event.type == Common::EVENT_LBUTTONUP) {
			tryCloseWindowRec(ref, true);
			return true;
		}
		return true;
	}

	if (_engine->needsClickToContinue())
		return true;

	if (click == kBorderResizeButton)
		return true;

	if (event.type == Common::EVENT_LBUTTONDOWN) {
		WindowData &data = findWindowData((WindowReference)ref);

		if (click == kBorderInner && !_draggedObjects.size()) {
			_engine->unselectAll();
			_engine->getSelectedObjects().clear();

			_lassoStart = event.mouse;
			_lassoEnd = _lassoStart;
			_lassoBeingDrawn = !_lassoBeingDrawn;
			_lassoWinRef = ref;
		} else {
			_lassoBeingDrawn = false;
			_lassoStart = _lassoEnd = {0, 0};
			_lassoWinRef = WindowReference(0);
		}

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
	} else if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_lassoBeingDrawn)
			_lassoEnd = event.mouse;
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		bringToFront(ref);

		if (_lassoBeingDrawn && !_draggedObjects.size()) {
			WindowData &data = findWindowData((WindowReference)ref);

			Common::Point topLeft(_lassoStart);
			Common::Point bottomRight(_lassoEnd);
			if (topLeft.x > bottomRight.x)
				SWAP(topLeft.x, bottomRight.x);
			if (topLeft.y > bottomRight.y)
				SWAP(topLeft.y, bottomRight.y);
			Common::Rect lassoArea(topLeft, bottomRight);

			Common::Array<ObjID> &selectedObjects = _engine->getSelectedObjects();
			bool selectSelfWindow = true;

			for (auto &obj : data.children) {
				ObjID id = obj.obj;
				Common::Rect bounds = _engine->getObjBounds(id);
				if (lassoArea.intersects(bounds) || lassoArea.contains(bounds)) {
					_engine->selectObject(id);
					selectedObjects.push_back(id);
					selectSelfWindow = false;
				}
			}

			if (selectSelfWindow) {
				_engine->handleObjectSelect(1, kSelfWindow, false, false);
			}
		}

		_lassoBeingDrawn = false;
		_lassoStart = _lassoEnd = {0, 0};
	}
	return true;
}

void Gui::select(Common::Point cursorPosition, bool shiftPressed, bool isDoubleClick) {
	WindowReference ref = findWindowAtPoint(cursorPosition);
	if (ref == kNoWindow) {
		return;
	}
	if (_engine->needsClickToContinue())
		return;

	Graphics::MacWindow *win = findWindow(ref);

	Common::Rect innerDims = win->getInnerDimensions();
	if (!innerDims.contains(cursorPosition))
		return;

	WindowData &data = findWindowData((WindowReference)ref);

	Common::Rect clickRect = calculateClickRect(cursorPosition + data.scrollPos, win->getInnerDimensions());
	checkSelect(data, cursorPosition, clickRect, (WindowReference)ref, isDoubleClick, shiftPressed);
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
