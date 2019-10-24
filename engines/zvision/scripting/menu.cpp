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

#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/menu.h"

namespace ZVision {

enum {
	kMainMenuSave = 0,
	kMainMenuLoad = 1,
	kMainMenuPrefs = 2,
	kMainMenuExit = 3
};

enum {
	kMenuItem = 0,
	kMenuMagic = 1,
	kMenuMain = 2
};

MenuHandler::MenuHandler(ZVision *engine) {
	_engine = engine;
	menuBarFlag = 0xFFFF;
}

MenuZGI::MenuZGI(ZVision *engine) :
	MenuHandler(engine) {
	menuMouseFocus = -1;
	inMenu = false;
	scrolled[0] = false;
	scrolled[1] = false;
	scrolled[2] = false;
	scrollPos[0] = 0;
	scrollPos[1] = 0;
	scrollPos[2] = 0;
	mouseOnItem = -1;
	redraw = false;
	clean = false;

	char buf[24];
	for (int i = 1; i < 4; i++) {
		sprintf(buf, "gmzau%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, menuBack[i - 1][0], false);
		sprintf(buf, "gmzau%2.2x1.tga", i + 0x10);
		_engine->getRenderManager()->readImageToSurface(buf, menuBack[i - 1][1], false);
	}
	for (int i = 0; i < 4; i++) {
		sprintf(buf, "gmzmu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, menuBar[i][0], false);
		sprintf(buf, "gmznu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, menuBar[i][1], false);
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
		menuBack[i][0].free();
		menuBack[i][1].free();
	}
	for (int i = 0; i < 4; i++) {
		menuBar[i][0].free();
		menuBar[i][1].free();
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
		case kMenuItem:
			if (menuBarFlag & kMenubarItems) {
				int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				if (itemCount == 0)
					itemCount = 20;

				for (int i = 0; i < itemCount; i++) {
					int itemspace = (600 - 28) / itemCount;

					if (Common::Rect(scrollPos[kMenuItem] + itemspace * i, 0,
					                 scrollPos[kMenuItem] + itemspace * i + 28, 32).contains(Pos)) {
						int32 mouseItem = _engine->getScriptManager()->getStateValue(StateKey_InventoryItem);
						if (mouseItem >= 0  && mouseItem < 0xE0) {
							_engine->getScriptManager()->inventoryDrop(mouseItem);
							_engine->getScriptManager()->inventoryAdd(_engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + i));
							_engine->getScriptManager()->setStateValue(StateKey_Inv_StartSlot + i, mouseItem);

							redraw = true;
						}
					}
				}
			}
			break;

		case kMenuMagic:
			if (menuBarFlag & kMenubarMagic) {
				for (int i = 0; i < 12; i++) {

					uint itemnum = _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + i);
					if (itemnum != 0) {
						if (_engine->getScriptManager()->getStateValue(StateKey_Reversed_Spellbooc) == 1)
							itemnum = 0xEE + i;
						else
							itemnum = 0xE0 + i;
					}
					if (itemnum)
						if (_engine->getScriptManager()->getStateValue(StateKey_InventoryItem) == 0 || _engine->getScriptManager()->getStateValue(StateKey_InventoryItem) >= 0xE0)
							if (Common::Rect(668 + 47 * i - scrollPos[kMenuMagic], 0,
							                 668 + 47 * i - scrollPos[kMenuMagic] + 28, 32).contains(Pos))
								_engine->getScriptManager()->setStateValue(StateKey_Active_Spell, itemnum);
				}

			}
			break;

		case kMenuMain:

			// Exit
			if (menuBarFlag & kMenubarExit)
				if (Common::Rect(320 + 135,
				                 scrollPos[kMenuMain],
				                 320 + 135 + 135,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					_engine->ifQuit();
				}

			// Settings
			if (menuBarFlag & kMenubarSettings)
				if (Common::Rect(320 ,
				                 scrollPos[kMenuMain],
				                 320 + 135,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					_engine->getScriptManager()->changeLocation('g', 'j', 'p', 'e', 0);
				}

			// Load
			if (menuBarFlag & kMenubarRestore)
				if (Common::Rect(320 - 135,
				                 scrollPos[kMenuMain],
				                 320,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					_engine->getScriptManager()->changeLocation('g', 'j', 'r', 'e', 0);
				}

			// Save
			if (menuBarFlag & kMenubarSave)
				if (Common::Rect(320 - 135 * 2,
				                 scrollPos[kMenuMain],
				                 320 - 135,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					_engine->getScriptManager()->changeLocation('g', 'j', 's', 'e', 0);
				}
			break;

		default:
			break;
		}
	}
}

void MenuZGI::onMouseMove(const Common::Point &Pos) {
	if (Pos.y < 40) {

		if (!inMenu)
			redraw = true;
		inMenu = true;
		switch (menuMouseFocus) {
		case kMenuItem:
			if (menuBarFlag & kMenubarItems) {
				int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				if (itemCount == 0)
					itemCount = 20;
				else if (itemCount > 50)
					itemCount = 50;

				int lastItem = mouseOnItem;

				mouseOnItem = -1;

				for (int i = 0; i < itemCount; i++) {
					int itemspace = (600 - 28) / itemCount;

					if (Common::Rect(scrollPos[kMenuItem] + itemspace * i, 0,
					                 scrollPos[kMenuItem] + itemspace * i + 28, 32).contains(Pos)) {
						mouseOnItem = i;
						break;
					}
				}

				if (lastItem != mouseOnItem)
					if (_engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + mouseOnItem) ||
					        _engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + lastItem))
						redraw = true;
			}
			break;

		case kMenuMagic:
			if (menuBarFlag & kMenubarMagic) {
				int lastItem = mouseOnItem;
				mouseOnItem = -1;
				for (int i = 0; i < 12; i++) {
					if (Common::Rect(668 + 47 * i - scrollPos[kMenuMagic], 0,
					                 668 + 47 * i - scrollPos[kMenuMagic] + 28, 32).contains(Pos)) {
						mouseOnItem = i;
						break;
					}
				}

				if (lastItem != mouseOnItem)
					if (_engine->getScriptManager()->getStateValue(StateKey_Spell_1 + mouseOnItem) ||
					        _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + lastItem))
						redraw = true;

			}
			break;

		case kMenuMain: {
			int lastItem = mouseOnItem;
			mouseOnItem = -1;

			// Exit
			if (menuBarFlag & kMenubarExit)
				if (Common::Rect(320 + 135,
				                 scrollPos[kMenuMain],
				                 320 + 135 + 135,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					mouseOnItem = kMainMenuExit;
				}

			// Settings
			if (menuBarFlag & kMenubarSettings)
				if (Common::Rect(320 ,
				                 scrollPos[kMenuMain],
				                 320 + 135,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					mouseOnItem = kMainMenuPrefs;
				}

			// Load
			if (menuBarFlag & kMenubarRestore)
				if (Common::Rect(320 - 135,
				                 scrollPos[kMenuMain],
				                 320,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					mouseOnItem = kMainMenuLoad;
				}

			// Save
			if (menuBarFlag & kMenubarSave)
				if (Common::Rect(320 - 135 * 2,
				                 scrollPos[kMenuMain],
				                 320 - 135,
				                 scrollPos[kMenuMain] + 32).contains(Pos)) {
					mouseOnItem = kMainMenuSave;
				}

			if (lastItem != mouseOnItem)
				redraw = true;
		}
		break;

		default:
			int cur_menu = menuMouseFocus;
			if (Common::Rect(64, 0, 64 + 512, 8).contains(Pos)) { // Main
				menuMouseFocus = kMenuMain;
				scrolled[kMenuMain]  = false;
				scrollPos[kMenuMain] = menuBack[kMenuMain][1].h - menuBack[kMenuMain][0].h;
				_engine->getScriptManager()->setStateValue(StateKey_MenuState, 2);
			}

			if (menuBarFlag & kMenubarMagic)
				if (Common::Rect(640 - 28, 0, 640, 32).contains(Pos)) { // Magic
					menuMouseFocus = kMenuMagic;
					scrolled[kMenuMagic]  = false;
					scrollPos[kMenuMagic] = 28;
					_engine->getScriptManager()->setStateValue(StateKey_MenuState, 3);
				}

			if (menuBarFlag & kMenubarItems)
				if (Common::Rect(0, 0, 28, 32).contains(Pos)) { // Items
					menuMouseFocus = kMenuItem;
					scrolled[kMenuItem]  = false;
					scrollPos[kMenuItem] = 28 - 600;
					_engine->getScriptManager()->setStateValue(StateKey_MenuState, 1);
				}

			if (cur_menu != menuMouseFocus)
				clean = true;

			break;
		}
	} else {
		if (inMenu)
			clean = true;
		inMenu = false;
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
	case kMenuItem:
		if (menuBarFlag & kMenubarItems)
			if (!scrolled[kMenuItem]) {
				redraw = true;
				float scrl = 600.0 * (deltatime / 1000.0);

				if (scrl == 0)
					scrl = 1.0;

				scrollPos[kMenuItem] += (int)scrl;

				if (scrollPos[kMenuItem] >= 0) {
					scrolled[kMenuItem] = true;
					scrollPos[kMenuItem] = 0;
				}
			}
		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menuBack[kMenuItem][0], scrollPos[kMenuItem], 0);

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

				uint curItemId = _engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + i);

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
						_engine->getRenderManager()->blitSurfaceToMenu(*items[i][1], scrollPos[kMenuItem] + itemspace * i, 0, 0);
					else
						_engine->getRenderManager()->blitSurfaceToMenu(*items[i][0], scrollPos[kMenuItem] + itemspace * i, 0, 0);

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

	case kMenuMagic:
		if (menuBarFlag & kMenubarMagic)
			if (!scrolled[kMenuMagic]) {
				redraw = true;
				float scrl = 600.0 * (deltatime / 1000.0);

				if (scrl == 0)
					scrl = 1.0;

				scrollPos[kMenuMagic] += (int)scrl;

				if (scrollPos[kMenuMagic] >= 600) {
					scrolled[kMenuMagic] = true;
					scrollPos[kMenuMagic] = 600;
				}
			}
		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menuBack[kMenuMagic][0], 640 - scrollPos[kMenuMagic], 0);

			for (int i = 0; i < 12; i++) {
				bool inrect = false;

				if (mouseOnItem == i)
					inrect = true;

				uint curItemId = _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + i);
				if (curItemId) {
					if (_engine->getScriptManager()->getStateValue(StateKey_Reversed_Spellbooc) == 1)
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
						_engine->getRenderManager()->blitSurfaceToMenu(*magic[i][1], 668 + 47 * i - scrollPos[kMenuMagic], 0, 0);
					else
						_engine->getRenderManager()->blitSurfaceToMenu(*magic[i][0], 668 + 47 * i - scrollPos[kMenuMagic], 0, 0);

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

	case kMenuMain:
		if (!scrolled[kMenuMain]) {
			redraw = true;
			float scrl = 32.0 * 2.0 * (deltatime / 1000.0);

			if (scrl == 0)
				scrl = 1.0;

			scrollPos[kMenuMain] += (int)scrl;

			if (scrollPos[kMenuMain] >= 0) {
				scrolled[kMenuMain] = true;
				scrollPos[kMenuMain] = 0;
			}
		}
		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menuBack[kMenuMain][0], 30, scrollPos[kMenuMain]);

			if (menuBarFlag & kMenubarExit) {
				if (mouseOnItem == kMainMenuExit)
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuExit][1], 320 + 135, scrollPos[kMenuMain]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuExit][0], 320 + 135, scrollPos[kMenuMain]);
			}
			if (menuBarFlag & kMenubarSettings) {
				if (mouseOnItem == kMainMenuPrefs)
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuPrefs][1], 320, scrollPos[kMenuMain]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuPrefs][0], 320, scrollPos[kMenuMain]);
			}
			if (menuBarFlag & kMenubarRestore) {
				if (mouseOnItem == kMainMenuLoad)
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuLoad][1], 320 - 135, scrollPos[kMenuMain]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuLoad][0], 320 - 135, scrollPos[kMenuMain]);
			}
			if (menuBarFlag & kMenubarSave) {
				if (mouseOnItem == kMainMenuSave)
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuSave][1], 320 - 135 * 2, scrollPos[kMenuMain]);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(menuBar[kMainMenuSave][0], 320 - 135 * 2, scrollPos[kMenuMain]);
			}
			redraw = false;
		}
		break;
	default:
		if (redraw) {
			if (inMenu) {
				_engine->getRenderManager()->blitSurfaceToMenu(menuBack[kMenuMain][1], 30, 0);

				if (menuBarFlag & kMenubarItems)
					_engine->getRenderManager()->blitSurfaceToMenu(menuBack[kMenuItem][1], 0, 0);

				if (menuBarFlag & kMenubarMagic)
					_engine->getRenderManager()->blitSurfaceToMenu(menuBack[kMenuMagic][1], 640 - 28, 0);
			}
			redraw = false;
		}
		break;
	}
}

MenuNemesis::MenuNemesis(ZVision *engine) :
	MenuHandler(engine) {
	inMenu = false;
	scrolled = false;
	scrollPos = 0;
	mouseOnItem = -1;
	redraw = false;
	delay = 0;

	char buf[24];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 6; j++) {
			sprintf(buf, "butfrm%d%d.tga", i + 1, j);
			_engine->getRenderManager()->readImageToSurface(buf, but[i][j], false);
		}

	_engine->getRenderManager()->readImageToSurface("bar.tga", menuBar, false);

	frm = 0;
}

MenuNemesis::~MenuNemesis() {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 6; j++)
			but[i][j].free();

	menuBar.free();
}

static const int16 buts[4][2] = { {120 , 64}, {144, 184}, {128, 328}, {120, 456} };

void MenuNemesis::onMouseUp(const Common::Point &Pos) {
	if (Pos.y < 40) {
		// Exit
		if (menuBarFlag & kMenubarExit)
			if (Common::Rect(buts[3][1],
			                 scrollPos,
			                 buts[3][0] + buts[3][1],
			                 scrollPos + 32).contains(Pos)) {
				_engine->ifQuit();
				frm = 5;
				redraw = true;
			}

		// Settings
		if (menuBarFlag & kMenubarSettings)
			if (Common::Rect(buts[2][1],
			                 scrollPos,
			                 buts[2][0] + buts[2][1],
			                 scrollPos + 32).contains(Pos)) {
				_engine->getScriptManager()->changeLocation('g', 'j', 'p', 'e', 0);
				frm = 5;
				redraw = true;
			}

		// Load
		if (menuBarFlag & kMenubarRestore)
			if (Common::Rect(buts[1][1],
			                 scrollPos,
			                 buts[1][0] + buts[1][1],
			                 scrollPos + 32).contains(Pos)) {
				_engine->getScriptManager()->changeLocation('g', 'j', 'r', 'e', 0);
				frm = 5;
				redraw = true;
			}

		// Save
		if (menuBarFlag & kMenubarSave)
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

		inMenu = true;

		if (_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 2)
			_engine->getScriptManager()->setStateValue(StateKey_MenuState, 2);

		int lastItem = mouseOnItem;
		mouseOnItem = -1;

		// Exit
		if (menuBarFlag & kMenubarExit)
			if (Common::Rect(buts[3][1],
			                 scrollPos,
			                 buts[3][0] + buts[3][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = kMainMenuExit;
			}

		// Settings
		if (menuBarFlag & kMenubarSettings)
			if (Common::Rect(buts[2][1],
			                 scrollPos,
			                 buts[2][0] + buts[2][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = kMainMenuPrefs;
			}

		// Load
		if (menuBarFlag & kMenubarRestore)
			if (Common::Rect(buts[1][1],
			                 scrollPos,
			                 buts[1][0] + buts[1][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = kMainMenuLoad;
			}

		// Save
		if (menuBarFlag & kMenubarSave)
			if (Common::Rect(buts[0][1],
			                 scrollPos,
			                 buts[0][0] + buts[0][1],
			                 scrollPos + 32).contains(Pos)) {
				mouseOnItem = kMainMenuSave;
			}

		if (lastItem != mouseOnItem) {
			redraw = true;
			frm = 0;
			delay = 200;
		}
	} else {
		inMenu = false;
		if (_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 0)
			_engine->getScriptManager()->setStateValue(StateKey_MenuState, 0);
		mouseOnItem = -1;
	}
}

void MenuNemesis::process(uint32 deltatime) {
	if (inMenu) {
		if (!scrolled) {
			float scrl = 32.0 * 2.0 * (deltatime / 1000.0);

			if (scrl == 0)
				scrl = 1.0;

			scrollPos += (int)scrl;
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
			_engine->getRenderManager()->blitSurfaceToMenu(menuBar, 64, scrollPos);

			if (menuBarFlag & kMenubarExit)
				if (mouseOnItem == kMainMenuExit)
					_engine->getRenderManager()->blitSurfaceToMenu(but[3][frm], buts[3][1], scrollPos);

			if (menuBarFlag & kMenubarSettings)
				if (mouseOnItem == kMainMenuPrefs)
					_engine->getRenderManager()->blitSurfaceToMenu(but[2][frm], buts[2][1], scrollPos);

			if (menuBarFlag & kMenubarRestore)
				if (mouseOnItem == kMainMenuLoad)
					_engine->getRenderManager()->blitSurfaceToMenu(but[1][frm], buts[1][1], scrollPos);

			if (menuBarFlag & kMenubarSave)
				if (mouseOnItem == kMainMenuSave)
					_engine->getRenderManager()->blitSurfaceToMenu(but[0][frm], buts[0][1], scrollPos);

			redraw = false;
		}
	} else {
		scrolled = false;
		if (scrollPos > -32) {
			float scrl = 32.0 * 2.0 * (deltatime / 1000.0);

			if (scrl == 0)
				scrl = 1.0;

			Common::Rect cl(64, (int16)(32 + scrollPos - scrl), 64 + 512, 32 + scrollPos + 1);
			_engine->getRenderManager()->clearMenuSurface(cl);

			scrollPos -= (int)scrl;
			redraw = true;
		} else
			scrollPos = -32;

		if (redraw) {
			_engine->getRenderManager()->blitSurfaceToMenu(menuBar, 64, scrollPos);
			redraw = false;
		}
	}
}

} // End of namespace ZVision
