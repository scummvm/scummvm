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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/core/menu.h"

#include "zvision/graphics/render_manager.h"

namespace ZVision {

enum {
	SLOT_START_SLOT = 151,
	SLOT_SPELL_1 = 191,
	SLOT_USER_CHOSE_THIS_SPELL = 205,
	SLOT_REVERSED_SPELLBOOK = 206
};

enum {
	menu_MAIN_SAVE = 0,
	menu_MAIN_REST = 1,
	menu_MAIN_PREF = 2,
	menu_MAIN_EXIT = 3
};

MenuHandler::MenuHandler(ZVision *engine) {
	_engine = engine;
	menuBarFlag = 0xFFFF;
}

MenuZGI::MenuZGI(ZVision *engine) :
	MenuHandler(engine) {
	menuMouseFocus = -1;
	inmenu = false;
	scrolled[0] = false;
	scrolled[1] = false;
	scrolled[2] = false;
	scrollPos[0] = 0.0;
	scrollPos[1] = 0.0;
	scrollPos[2] = 0.0;
	mouseOnItem = -1;
	redraw = false;
	clean = false;

	char buf[24];
	for (int i = 1; i < 4; i++) {
		sprintf(buf, "gmzau%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, menuback[i - 1][0], false);
		sprintf(buf, "gmzau%2.2x1.tga", i + 0x10);
		_engine->getRenderManager()->readImageToSurface(buf, menuback[i - 1][1], false);
	}
	for (int i = 0; i < 4; i++) {
		sprintf(buf, "gmzmu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, menubar[i][0], false);
		sprintf(buf, "gmznu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, menubar[i][1], false);
	}

	for (int i = 0; i < 50; i++) {
		items[i][0] = NULL;
		items[i][1] = NULL;
		itemId[i] = 0;
	}

	for (int i = 0; i < 12; i++) {
		magic[i][0] = NULL;
		magic[i][1] = NULL;
		magicId[i] = 0;
	}
}

MenuZGI::~MenuZGI() {
	for (int i = 0; i < 3; i++) {
		menuback[i][0].free();
		menuback[i][1].free();
	}
	for (int i = 0; i < 4; i++) {
		menubar[i][0].free();
		menubar[i][1].free();
	}
	for (int i = 0; i < 50; i++) {
		if (items[i][0]) {
			items[i][0]->free();
			delete items[i][0];
		}
		if (items[i][1]) {
			items[i][1]->free();
			delete items[i][1];
		}
	}
	for (int i = 0; i < 12; i++) {
		if (magic[i][0]) {
			magic[i][0]->free();
			delete magic[i][0];
		}
		if (magic[i][1]) {
			magic[i][1]->free();
			delete magic[i][1];
		}
	}
}

void MenuZGI::onMouseUp(const Common::Point &Pos) {
	if (Pos.y < 40) {
		switch (menuMouseFocus) {
		case menu_ITEM:
			if (menuBarFlag & menuBar_Items) {
				int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				if (itemCount == 0)
					itemCount = 20;

				for (int i = 0; i < itemCount; i++) {
					int itemspace = (600 - 28) / itemCount;

					if (Common::Rect(scrollPos[menu_ITEM] + itemspace * i, 0,
					                 scrollPos[menu_ITEM] + itemspace * i + 28, 32).contains(Pos)) {
						int32 mouseItem = _engine->getScriptManager()->getStateValue(StateKey_InventoryItem);
						if (mouseItem >= 0  && mouseItem < 0xE0) {
							_engine->getScriptManager()->inventoryDrop(mouseItem);
							_engine->getScriptManager()->inventoryAdd(_engine->getScriptManager()->getStateValue(SLOT_START_SLOT + i));
							_engine->getScriptManager()->setStateValue(SLOT_START_SLOT + i, mouseItem);

							redraw = true;
						}
					}
				}
			}
			break;

		case menu_MAGIC:
			if (menuBarFlag & menuBar_Magic) {
				for (int i = 0; i < 12; i++) {

					uint itemnum = _engine->getScriptManager()->getStateValue(SLOT_SPELL_1 + i);
					if (itemnum != 0) {
						if (_engine->getScriptManager()->getStateValue(SLOT_REVERSED_SPELLBOOK) == 1)
							itemnum = 0xEE + i;
						else
							itemnum = 0xE0 + i;
					}
					if (itemnum)
						if (_engine->getScriptManager()->getStateValue(StateKey_InventoryItem) == 0 || _engine->getScriptManager()->getStateValue(StateKey_InventoryItem) >= 0xE0)
							if (Common::Rect(668 + 47 * i - scrollPos[menu_MAGIC], 0,
							                 668 + 47 * i - scrollPos[menu_MAGIC] + 28, 32).contains(Pos))
								_engine->getScriptManager()->setStateValue(SLOT_USER_CHOSE_THIS_SPELL, itemnum);
				}

			}
			break;

		case menu_MAIN:

			// Exit
			if (menuBarFlag & menuBar_Exit)
				if (Common::Rect(320 + 135,
				                 scrollPos[menu_MAIN],
				                 320 + 135 + 135,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					_engine->ifQuit();
				}

			// Settings
			if (menuBarFlag & menuBar_Settings)
				if (Common::Rect(320 ,
				                 scrollPos[menu_MAIN],
				                 320 + 135,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					_engine->getScriptManager()->changeLocation('g', 'j', 'p', 'e', 0);
				}

			// Load
			if (menuBarFlag & menuBar_Restore)
				if (Common::Rect(320 - 135,
				                 scrollPos[menu_MAIN],
				                 320,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					_engine->getScriptManager()->changeLocation('g', 'j', 'r', 'e', 0);
				}

			// Save
			if (menuBarFlag & menuBar_Save)
				if (Common::Rect(320 - 135 * 2,
				                 scrollPos[menu_MAIN],
				                 320 - 135,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					_engine->getScriptManager()->changeLocation('g', 'j', 's', 'e', 0);
				}
			break;
		}
	}
}

void MenuZGI::onMouseMove(const Common::Point &Pos) {
	if (Pos.y < 40) {

		if (!inmenu)
			redraw = true;
		inmenu = true;
		switch (menuMouseFocus) {
		case menu_ITEM:
			if (menuBarFlag & menuBar_Items) {
				int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				if (itemCount == 0)
					itemCount = 20;
				else if (itemCount > 50)
					itemCount = 50;

				int lastItem = mouseOnItem;

				mouseOnItem = -1;

				for (int i = 0; i < itemCount; i++) {
					int itemspace = (600 - 28) / itemCount;

					if (Common::Rect(scrollPos[menu_ITEM] + itemspace * i, 0,
					                 scrollPos[menu_ITEM] + itemspace * i + 28, 32).contains(Pos)) {
						mouseOnItem = i;
						break;
					}
				}

				if (lastItem != mouseOnItem)
					if (_engine->getScriptManager()->getStateValue(SLOT_START_SLOT + mouseOnItem) ||
					        _engine->getScriptManager()->getStateValue(SLOT_START_SLOT + lastItem))
						redraw = true;
			}
			break;

		case menu_MAGIC:
			if (menuBarFlag & menuBar_Magic) {
				int lastItem = mouseOnItem;
				mouseOnItem = -1;
				for (int i = 0; i < 12; i++) {
					if (Common::Rect(668 + 47 * i - scrollPos[menu_MAGIC], 0,
					                 668 + 47 * i - scrollPos[menu_MAGIC] + 28, 32).contains(Pos)) {
						mouseOnItem = i;
						break;
					}
				}

				if (lastItem != mouseOnItem)
					if (_engine->getScriptManager()->getStateValue(SLOT_SPELL_1 + mouseOnItem) ||
					        _engine->getScriptManager()->getStateValue(SLOT_SPELL_1 + lastItem))
						redraw = true;

			}
			break;

		case menu_MAIN: {
			int lastItem = mouseOnItem;
			mouseOnItem = -1;

			// Exit
			if (menuBarFlag & menuBar_Exit)
				if (Common::Rect(320 + 135,
				                 scrollPos[menu_MAIN],
				                 320 + 135 + 135,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					mouseOnItem = menu_MAIN_EXIT;
				}

			// Settings
			if (menuBarFlag & menuBar_Settings)
				if (Common::Rect(320 ,
				                 scrollPos[menu_MAIN],
				                 320 + 135,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					mouseOnItem = menu_MAIN_PREF;
				}

			// Load
			if (menuBarFlag & menuBar_Restore)
				if (Common::Rect(320 - 135,
				                 scrollPos[menu_MAIN],
				                 320,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					mouseOnItem = menu_MAIN_REST;
				}

			// Save
			if (menuBarFlag & menuBar_Save)
				if (Common::Rect(320 - 135 * 2,
				                 scrollPos[menu_MAIN],
				                 320 - 135,
				                 scrollPos[menu_MAIN] + 32).contains(Pos)) {
					mouseOnItem = menu_MAIN_SAVE;
				}

			if (lastItem != mouseOnItem)
				redraw = true;
		}
		break;

		default:
			int cur_menu = menuMouseFocus;
			if (Common::Rect(64, 0, 64 + 512, 8).contains(Pos)) { // Main
				menuMouseFocus = menu_MAIN;
				scrolled[menu_MAIN]  = false;
				scrollPos[menu_MAIN] = menuback[menu_MAIN][1].h - menuback[menu_MAIN][0].h;
				_engine->getScriptManager()->setStateValue(StateKey_MenuState, 2);
			}

			if (menuBarFlag & menuBar_Magic)
				if (Common::Rect(640 - 28, 0, 640, 32).contains(Pos)) { // Magic
					menuMouseFocus = menu_MAGIC;
					scrolled[menu_MAGIC]  = false;
					scrollPos[menu_MAGIC] = 28;
					_engine->getScriptManager()->setStateValue(StateKey_MenuState, 3);
				}

			if (menuBarFlag & menuBar_Items)
				if (Common::Rect(0, 0, 28, 32).contains(Pos)) { // Items
					menuMouseFocus = menu_ITEM;
					scrolled[menu_ITEM]  = false;
					scrollPos[menu_ITEM] = 28 - 600;
					_engine->getScriptManager()->setStateValue(StateKey_MenuState, 1);
				}

			if (cur_menu != menuMouseFocus)
				clean = true;

			break;
		}
	} else {
		if (inmenu)
			clean = true;
		inmenu = false;
		if (_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 0)
			_engine->getScriptManager()->setStateValue(StateKey_MenuState, 0);
		menuMouseFocus = -1;
	}
}

void MenuZGI::process(uint32 deltatime) {
	if (clean) {
		_engine->getRenderManager()->clearMenuSurface();
		clean = false;
	}
	switch (menuMouseFocus) {
	case menu_ITEM:
		if (menuBarFlag & menuBar_Items)
			if (!scrolled[menu_ITEM]) {
				redraw = true;
				float scrl = 600.0 * (deltatime / 1000.0);

				if (scrl == 0)
					scrl = 1.0;

				scrollPos [menu_ITEM] += scrl;

				if (scrollPos[menu_ITEM] >= 0) {
					scrolled[menu_ITEM] = true;
					scrollPos [menu_ITEM] = 0;
				}
			}
		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menuback[menu_ITEM][0], scrollPos[menu_ITEM], 0);

			int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
			if (itemCount == 0)
				itemCount = 20;
			else if (itemCount > 50)
				itemCount = 50;

			for (int i = 0; i < itemCount; i++) {
				int itemspace = (600 - 28) / itemCount;

				bool inrect = false;

				if (mouseOnItem == i)
					inrect = true;

				uint curItemId = _engine->getScriptManager()->getStateValue(SLOT_START_SLOT + i);

				if (curItemId != 0) {
					if (itemId[i] != curItemId) {
						char buf[16];
						sprintf(buf, "gmzwu%2.2x1.tga", curItemId);
						items[i][0] = _engine->getRenderManager()->loadImage(buf, false);
						sprintf(buf, "gmzxu%2.2x1.tga", curItemId);
						items[i][1] = _engine->getRenderManager()->loadImage(buf, false);
						itemId[i] = curItemId;
					}

					if (inrect)
						_engine->getRenderManager()->blitSurfaceToMenu(*items[i][1], scrollPos[menu_ITEM] + itemspace * i, 0, 0);
					else
						_engine->getRenderManager()->blitSurfaceToMenu(*items[i][0], scrollPos[menu_ITEM] + itemspace * i, 0, 0);

				} else {
					if (items[i][0]) {
						items[i][0]->free();
						delete items[i][0];
						items[i][0] = NULL;
					}
					if (items[i][1]) {
						items[i][1]->free();
						delete items[i][1];
						items[i][1] = NULL;
					}
					itemId[i] = 0;
				}
			}

			redraw = false;
		}
		break;

	case menu_MAGIC:
		if (menuBarFlag & menuBar_Magic)
			if (!scrolled[menu_MAGIC]) {
				redraw = true;
				float scrl = 600.0 * (deltatime / 1000.0);

				if (scrl == 0)
					scrl = 1.0;

				scrollPos [menu_MAGIC] += scrl;

				if (scrollPos[menu_MAGIC] >= 600) {
					scrolled[menu_MAGIC] = true;
					scrollPos [menu_MAGIC] = 600;
				}
			}
		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menuback[menu_MAGIC][0], 640 - scrollPos[menu_MAGIC], 0);

			for (int i = 0; i < 12; i++) {
				bool inrect = false;

				if (mouseOnItem == i)
					inrect = true;

				uint curItemId = _engine->getScriptManager()->getStateValue(SLOT_SPELL_1 + i);
				if (curItemId) {
					if (_engine->getScriptManager()->getStateValue(SLOT_REVERSED_SPELLBOOK) == 1)
						curItemId = 0xEE + i;
					else
						curItemId = 0xE0 + i;
				}

				if (curItemId != 0) {
					if (itemId[i] != curItemId) {
						char buf[16];
						sprintf(buf, "gmzwu%2.2x1.tga", curItemId);
						magic[i][0] = _engine->getRenderManager()->loadImage(buf, false);
						sprintf(buf, "gmzxu%2.2x1.tga", curItemId);
						magic[i][1] = _engine->getRenderManager()->loadImage(buf, false);
						magicId[i] = curItemId;
					}

					if (inrect)
						_engine->getRenderManager()->blitSurfaceToMenu(*magic[i][1], 668 + 47 * i - scrollPos[menu_MAGIC], 0, 0);
					else
						_engine->getRenderManager()->blitSurfaceToMenu(*magic[i][0], 668 + 47 * i - scrollPos[menu_MAGIC], 0, 0);

				} else {
					if (magic[i][0]) {
						magic[i][0]->free();
						delete magic[i][0];
						magic[i][0] = NULL;
					}
					if (magic[i][1]) {
						magic[i][1]->free();
						delete magic[i][1];
						magic[i][1] = NULL;
					}
					magicId[i] = 0;
				}
			}
			redraw = false;
		}
		break;

	case menu_MAIN:
		if (!scrolled[menu_MAIN]) {
			redraw = true;
			float scrl = 32.0 * 2.0 * (deltatime / 1000.0);

			if (scrl == 0)
				scrl = 1.0;

			scrollPos [menu_MAIN] += scrl;

			if (scrollPos[menu_MAIN] >= 0) {
				scrolled[menu_MAIN] = true;
				scrollPos [menu_MAIN] = 0;
			}
		}
		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menuback[menu_MAIN][0], 30, scrollPos[menu_MAIN]);

			if (menuBarFlag & menuBar_Exit) {
				if (mouseOnItem == menu_MAIN_EXIT)
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_EXIT][1], 320 + 135, scrollPos[menu_MAIN]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_EXIT][0], 320 + 135, scrollPos[menu_MAIN]);
			}
			if (menuBarFlag & menuBar_Settings) {
				if (mouseOnItem == menu_MAIN_PREF)
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_PREF][1], 320, scrollPos[menu_MAIN]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_PREF][0], 320, scrollPos[menu_MAIN]);
			}
			if (menuBarFlag & menuBar_Restore) {
				if (mouseOnItem == menu_MAIN_REST)
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_REST][1], 320 - 135, scrollPos[menu_MAIN]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_REST][0], 320 - 135, scrollPos[menu_MAIN]);
			}
			if (menuBarFlag & menuBar_Save) {
				if (mouseOnItem == menu_MAIN_SAVE)
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_SAVE][1], 320 - 135 * 2, scrollPos[menu_MAIN]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menubar[menu_MAIN_SAVE][0], 320 - 135 * 2, scrollPos[menu_MAIN]);
			}
			redraw = false;
		}
		break;
	default:
		if (redraw) {
			if (inmenu) {
				_engine->getRenderManager()->blitSurfaceToMenu(menuback[menu_MAIN][1], 30, 0);

				if (menuBarFlag & menuBar_Items)
					_engine->getRenderManager()->blitSurfaceToMenu(menuback[menu_ITEM][1], 0, 0);

				if (menuBarFlag & menuBar_Magic)
					_engine->getRenderManager()->blitSurfaceToMenu(menuback[menu_MAGIC][1], 640 - 28, 0);
			}
			redraw = false;
		}
		break;
	}
}

MenuNemesis::MenuNemesis(ZVision *engine) :
	MenuHandler(engine) {
	inmenu = false;
	scrolled = false;
	scrollPos = 0.0;
	mouseOnItem = -1;

	char buf[24];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 6; j++) {
			sprintf(buf, "butfrm%d%d.tga", i + 1, j);
			_engine->getRenderManager()->readImageToSurface(buf, but[i][j], false);
		}

	_engine->getRenderManager()->readImageToSurface("bar.tga", menubar, false);

	frm = 0;
}

MenuNemesis::~MenuNemesis() {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 6; j++)
			but[i][j].free();

	menubar.free();
}

static const int16 buts[4][2] = { {120 , 64}, {144, 184}, {128, 328}, {120, 456} };

void MenuNemesis::onMouseUp(const Common::Point &Pos) {
	if (Pos.y < 40) {
		// Exit
		if (menuBarFlag & menuBar_Exit)
			if (Common::Rect(buts[3][1],
			                 scrollPos,
			                 buts[3][0] + buts[3][1],
			                 scrollPos + 32).contains(Pos)) {
				_engine->ifQuit();
				frm = 5;
				redraw = true;
			}

		// Settings
		if (menuBarFlag & menuBar_Settings)
			if (Common::Rect(buts[2][1],
			                 scrollPos,
			                 buts[2][0] + buts[2][1],
			                 scrollPos + 32).contains(Pos)) {
				_engine->getScriptManager()->changeLocation('g', 'j', 'p', 'e', 0);
				frm = 5;
				redraw = true;
			}

		// Load
		if (menuBarFlag & menuBar_Restore)
			if (Common::Rect(buts[1][1],
			                 scrollPos,
			                 buts[1][0] + buts[1][1],
			                 scrollPos + 32).contains(Pos)) {
				_engine->getScriptManager()->changeLocation('g', 'j', 'r', 'e', 0);
				frm = 5;
				redraw = true;
			}

		// Save
		if (menuBarFlag & menuBar_Save)
			if (Common::Rect(buts[0][1],
			                 scrollPos,
			                 buts[0][0] + buts[0][1],
			                 scrollPos + 32).contains(Pos)) {
				_engine->getScriptManager()->changeLocation('g', 'j', 's', 'e', 0);
				frm = 5;
				redraw = true;
			}
	}
}

void MenuNemesis::onMouseMove(const Common::Point &Pos) {
	if (Pos.y < 40) {

		inmenu = true;

		if (_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 2)
			_engine->getScriptManager()->setStateValue(StateKey_MenuState, 2);

		int lastItem = mouseOnItem;
		mouseOnItem = -1;

		// Exit
		if (menuBarFlag & menuBar_Exit)
			if (Common::Rect(buts[3][1],
			                 scrollPos,
			                 buts[3][0] + buts[3][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = menu_MAIN_EXIT;
			}

		// Settings
		if (menuBarFlag & menuBar_Settings)
			if (Common::Rect(buts[2][1],
			                 scrollPos,
			                 buts[2][0] + buts[2][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = menu_MAIN_PREF;
			}

		// Load
		if (menuBarFlag & menuBar_Restore)
			if (Common::Rect(buts[1][1],
			                 scrollPos,
			                 buts[1][0] + buts[1][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = menu_MAIN_REST;
			}

		// Save
		if (menuBarFlag & menuBar_Save)
			if (Common::Rect(buts[0][1],
			                 scrollPos,
			                 buts[0][0] + buts[0][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = menu_MAIN_SAVE;
			}

		if (lastItem != mouseOnItem) {
			redraw = true;
			frm = 0;
			delay = 200;
		}
	} else {
		inmenu = false;
		if (_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 0)
			_engine->getScriptManager()->setStateValue(StateKey_MenuState, 0);
		mouseOnItem = -1;
	}
}

void MenuNemesis::process(uint32 deltatime) {
	if (inmenu) {
		if (!scrolled) {
			float scrl = 32.0 * 2.0 * (deltatime / 1000.0);

			if (scrl == 0)
				scrl = 1.0;

			scrollPos += scrl;
			redraw = true;
		}

		if (scrollPos >= 0) {
			scrolled = true;
			scrollPos = 0;
		}

		if (mouseOnItem != -1) {
			delay -= deltatime;
			if (delay <= 0 && frm < 4) {
				delay = 200;
				frm++;
				redraw = true;
			}
		}

		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menubar, 64, scrollPos);

			if (menuBarFlag & menuBar_Exit)
				if (mouseOnItem == menu_MAIN_EXIT)
					_engine->getRenderManager()->blitSurfaceToMenu(but[3][frm], buts[3][1], scrollPos);

			if (menuBarFlag & menuBar_Settings)
				if (mouseOnItem == menu_MAIN_PREF)
					_engine->getRenderManager()->blitSurfaceToMenu(but[2][frm], buts[2][1], scrollPos);

			if (menuBarFlag & menuBar_Restore)
				if (mouseOnItem == menu_MAIN_REST)
					_engine->getRenderManager()->blitSurfaceToMenu(but[1][frm], buts[1][1], scrollPos);

			if (menuBarFlag & menuBar_Save)
				if (mouseOnItem == menu_MAIN_SAVE)
					_engine->getRenderManager()->blitSurfaceToMenu(but[0][frm], buts[0][1], scrollPos);

			redraw = false;
		}
	} else {
		scrolled = false;
		if (scrollPos > -32) {
			float scrl = 32.0 * 2.0 * (deltatime / 1000.0);

			if (scrl == 0)
				scrl = 1.0;

			Common::Rect cl(64, 32 + scrollPos - scrl, 64 + 512, 32 + scrollPos + 1);
			_engine->getRenderManager()->clearMenuSurface(cl);

			scrollPos -= scrl;
			redraw = true;
		} else
			scrollPos = -32;

		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menubar, 64, scrollPos);
			redraw = false;
		}
	}
}

} // End of namespace ZVision
