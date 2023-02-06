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

#ifndef TETRAEDGE_GAME_DOCUMENTS_BROWSER_H
#define TETRAEDGE_GAME_DOCUMENTS_BROWSER_H

#include "common/str.h"
#include "tetraedge/game/document.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class DocumentsBrowser : public TeLayout {
public:
	DocumentsBrowser();

	int addDocument(Document *document);
	void addDocument(const Common::String &str);

	void currentPage(int page);
	int documentCount(const Common::String &str) { // never used?
		return 1;
	}

	Common::String documentDescription(const Common::String &name);
	Common::String documentName(const Common::String &name);

	void enter();
	void hideDocument();
	void leave();
	void load();
	// void loadFromBackup(TiXmlNode *node);
	void loadZoomed();
	// void saveToBackup(TiXmlNode *node);

	void onDocumentSelected(Document *doc);
	bool onNextPage();
	bool onPreviousPage();
	bool onQuitDocumentDoubleClickTimer();
	bool onZoomedButton();

	// Sorry, this is how the original does it...
	bool onShowedDocumentButton0();
	bool onShowedDocumentButton1();
	bool onShowedDocumentButton2();
	bool onShowedDocumentButton3();
	bool onShowedDocumentButton4();
	bool onShowedDocumentButton5();
	bool onShowedDocumentButton6();
	bool onShowedDocumentButton7();
	bool onShowedDocumentButton8();
	bool onShowedDocumentButton9();
	bool onShowedDocumentButton10();
	bool onShowedDocumentButton11();
	bool onShowedDocumentButton12();
	bool onShowedDocumentButton13();
	bool onShowedDocumentButton14();
	bool onShowedDocumentButton15();
	bool onShowedDocumentButton16();
	bool onShowedDocumentButton17();
	bool onShowedDocumentButton18();
	bool onShowedDocumentButton19();

	void showDocument(const Common::String &str, int startPage);
	void unload();

	TeLayout &zoomedLayout() { return _zoomedLayout; }

	TeLuaGUI &gui1() { return _gui1; }

private:
	TeTimer _timer;
	TeLayout _zoomedLayout;
	uint64 _curPage;
	uint64 _startPage;
	int _zoomCount;
	Common::String _curDocName;

	TeLuaGUI _gui1;
	TeLuaGUI _gui2;
	// TiXmlDocument _xmldoc;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_DOCUMENTS_BROWSER_H
