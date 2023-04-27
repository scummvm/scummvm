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

#include "tetraedge/game/documents_browser.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/documents_browser_xml_parser.h"
#include "tetraedge/game/syberia_game.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_lua_thread.h"
#include "tetraedge/te/te_scrolling_layout.h"

namespace Tetraedge {

DocumentsBrowser::DocumentsBrowser() : _startPage(0), _curPage(0), _zoomCount(0) {
	_timer.alarmSignal().add(this, &DocumentsBrowser::onQuitDocumentDoubleClickTimer);
}

void DocumentsBrowser::enter() {
	setVisible(true);
	currentPage(_curPage);
}

void DocumentsBrowser::hideDocument() {
	Common::String docName = _curDocName;
	_curDocName.clear();
	TeSpriteLayout *zoomedSprite = _gui.spriteLayout("zoomedSprite");
	if (!zoomedSprite)
		return;
	Application *app = g_engine->getApplication();
	app->captureFade();
	zoomedSprite->unload();
	_gui.buttonLayoutChecked("zoomed")->setVisible(false);
	_zoomedDocGui.unload();
	Game *game = g_engine->getGame();

	bool callFn = true;
	SyberiaGame *sybgame = dynamic_cast<SyberiaGame *>(game);
	if (sybgame) {
		Common::Array<SyberiaGame::YieldedCallback> &yieldedcallbacks = sybgame->yieldedCallbacks();
		for (uint i = 0; i < yieldedcallbacks.size(); i++) {
			if (yieldedcallbacks[i]._luaFnName == "OnDocumentClosed" &&
				yieldedcallbacks[i]._luaParam == docName) {
				yieldedcallbacks.remove_at(i);
				if (yieldedcallbacks[i]._luaThread) {
					yieldedcallbacks[i]._luaThread->resume();
					callFn = false;
				}
				break;
			}
		}
		if (callFn)
			game->luaScript().execute("OnDocumentClosed", docName);
	}

	app->fade();
}

void DocumentsBrowser::leave() {
	_timer.stop();
	setVisible(false);
}

void DocumentsBrowser::load() {
	setVisible(false);
	setName("_documentsBrowser");
	setSizeType(RELATIVE_TO_PARENT);
	setSize(TeVector3f32(1.0f, 1.0f, userSize().z()));

	_gui.load("DocumentsBrowser/DocumentsBrowser.lua");

	TeLayout *docBrowser = _gui.layout("documentBrowser");
	if (docBrowser)
		addChild(docBrowser);

	TeButtonLayout *button = _gui.buttonLayoutChecked("previousPage");
	button->onMouseClickValidated().add(this, &DocumentsBrowser::onPreviousPage);
	button = _gui.buttonLayoutChecked("nextPage");
	button->onMouseClickValidated().add(this, &DocumentsBrowser::onNextPage);
	button = _gui.buttonLayoutChecked("zoomed");
	button->onMouseClickValidated().add(this, &DocumentsBrowser::onZoomedButton);
	button->setVisible(false);

	if (g_engine->gameIsAmerzone()) {
		TeLayout *bglayout = _gui.layoutChecked("background");
		bglayout->setRatioMode(RATIO_MODE_NONE);

		loadXMLFile("DocumentsBrowser/Documents/Documents.xml");
	}
	_timer.start();
}

void DocumentsBrowser::loadZoomed() {
	_zoomedLayout.setSizeType(RELATIVE_TO_PARENT);
	TeVector3f32 usersz = userSize();
	_zoomedLayout.setSize(TeVector3f32(1.0f, 1.0f, usersz.z()));
	TeLayout *zoomedChild = _gui.layout("zoomed");
	_zoomedLayout.addChild(zoomedChild);
}

void DocumentsBrowser::loadXMLFile(const Common::String &path) {
	Common::FSNode node = g_engine->getCore()->findFile(path);
	Common::File xmlfile;
	xmlfile.open(node);
	int64 fileLen = xmlfile.size();
	char *buf = new char[fileLen + 1];
	buf[fileLen] = '\0';
	xmlfile.read(buf, fileLen);
	const Common::String xmlContents = Common::String::format("<?xml version=\"1.0\" encoding=\"UTF-8\"?><document>%s</document>", buf);
	delete [] buf;
	xmlfile.close();

	DocumentsBrowserXmlParser parser;
	if (!parser.loadBuffer((const byte *)xmlContents.c_str(), xmlContents.size()))
		error("Couldn't load inventory xml.");
	if (!parser.parse())
		error("Couldn't parse inventory xml.");
	_documentData = parser._objects;
}

void DocumentsBrowser::currentPage(int setPage) {
	const Common::String setPageName = Common::String::format("page%d", setPage);
	TeLayout *pageLayout = _gui.layout(setPageName);
	if (!pageLayout)
		return;

	_curPage = setPage;

	int pageNo = 0;
	while (true) {
		const Common::String pageName = Common::String::format("page%d", pageNo);
		pageLayout = _gui.layout(pageName);
		if (!pageLayout)
			break;
		pageLayout->setVisible(pageNo == setPage);
		const Common::String diodeName = Common::String::format("diode%d", pageNo);
		_gui.buttonLayoutChecked(diodeName)->setEnable(pageNo == setPage);
		pageNo++;
	}
}

bool DocumentsBrowser::onQuitDocumentDoubleClickTimer() {
	uint64 time = _timer.getTimeFromStart();
	_timer.stop();
	if (time >= 200000) {
		showDocument(_curDocName, _startPage + 1);
	} else {
		hideDocument();
	}
	return false;
}

bool DocumentsBrowser::onNextPage() {
	currentPage(_curPage + 1);
	return false;
}

bool DocumentsBrowser::onPreviousPage() {
	currentPage(_curPage - 1);
	return false;
}

bool DocumentsBrowser::onZoomedButton() {
	int count = _zoomCount;
	_zoomCount++;
	if (count == 0) {
		_timer.start();
		_timer.setAlarmIn(200000);
	} else {
		onQuitDocumentDoubleClickTimer();
	}
	return false;
}

bool DocumentsBrowser::addDocument(Document *document) {
	int pageno = 0;
	while (true) {
		Common::String pageName = Common::String::format("page%d", pageno);
		TeLayout *page = _gui.layout(pageName);
		if (!page)
			break;
		int slotno = 0;
		while (true) {
			Common::String pageSlotName = Common::String::format("page%dSlot%d", pageno, slotno);
			TeLayout *slot = _gui.layout(pageSlotName);
			if (!slot)
				break;
			if (slot->childCount() == 0) {
				slot->addChild(document);
				return true;
			}
			slotno++;
		}
		pageno++;
	}
	return false;
}

void DocumentsBrowser::addDocument(const Common::String &str) {
	Document *doc = new Document(this);
	doc->load(str);
	if (!addDocument(doc))
		delete doc;
}

Common::String DocumentsBrowser::documentDescription(const Common::String &key) const {
	if (_documentData.contains(key))
		return _documentData.getVal(key)._description;
	return "";
}

Common::String DocumentsBrowser::documentName(const Common::String &key) const {
	if (_documentData.contains(key))
		return _documentData.getVal(key)._name;
	return "";
}

void DocumentsBrowser::showDocument(const Common::String &docName, int startPage) {
	_curPage = startPage;
	_startPage = startPage;
	_curDocName = docName;
	_zoomedDocGui.unload();
	TeCore *core = g_engine->getCore();
	const Common::Path docPathBase(Common::String::format("DocumentsBrowser/Documents/Documents/%s_zoomed_%d", docName.c_str(), (int)startPage));
	Common::Path docPath = docPathBase.append(".png");
	Common::FSNode docNode = core->findFile(docPath);
	if (!docNode.exists()) {
		docPath = docPathBase.append(".jpg");
		docNode = core->findFile(docPath);
		if (!docNode.exists()) {
			// Probably the end of the doc
			if (startPage == 0)
				warning("Can't find first page of doc named %s", docName.c_str());
			hideDocument();
			return;
		}
	}
	Application *app = g_engine->getApplication();
	app->captureFade();
	TeSpriteLayout *sprite = _gui.spriteLayoutChecked("zoomedSprite");
	//sprite->setSizeType(ABSOLUTE);
	sprite->load(docNode);
	TeVector2s32 spriteSize = sprite->_tiledSurfacePtr->tiledTexture()->totalSize();
	sprite->setSizeType(RELATIVE_TO_PARENT);
	TeVector3f32 winSize = app->getMainWindow().size();
	sprite->setSize(TeVector3f32(1.0f, (4.0f / (winSize.y() / winSize.x() * 4.0f)) *
							((float)spriteSize._y / (float)spriteSize._x), 0.0f));
	TeScrollingLayout *scroll = _gui.scrollingLayout("scroll");
	if (!scroll)
		error("DocumentsBrowser::showDocument Couldn't fetch scroll object");
	scroll->resetScrollPosition();
	scroll->playAutoScroll();
	Common::FSNode luaNode = core->findFile(docPathBase.append(".lua"));
	if (luaNode.exists()) {
		_zoomedDocGui.load(luaNode);
		error("Finish DocumentsBrowser::showDocument");
	}
	_gui.layoutChecked("zoomed")->setVisible(true);
	_zoomCount = 0;
	app->fade();
}

void DocumentsBrowser::unload() {
	hideDocument();
	int pageno = 0;
	while (true) {
		Common::String pageName = Common::String::format("page%d", pageno);
		TeLayout *page = _gui.layout(pageName);
		if (!page)
			break;
		int slotno = 0;
		while (true) {
			Common::String pageSlotName = Common::String::format("page%dSlot%d", pageno, slotno);
			TeLayout *slot = _gui.layout(pageSlotName);
			if (!slot)
				break;
			for (int i = 0; i < slot->childCount(); i++) {
				Document *doc = dynamic_cast<Document *>(slot->child(i));
				if (doc)
					delete doc;
			}
			slotno++;
		}
		pageno++;
	}
	_gui.unload();
}

} // end namespace Tetraedge
