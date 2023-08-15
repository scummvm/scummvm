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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/crab.h"
#include "crab/XMLDoc.h"
#include "crab/ui/OptionMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::music;

void OptionMenu::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("option");
		if (nodeValid(node)) {
			if (nodeValid("bg", node))
				_bg.load(node->first_node("bg"));

			if (nodeValid("state", node)) {
				_menu.load(node->first_node("state"));

				if (!_menu._element.empty())
					_menu._element[0].state(true);
			}

			if (nodeValid("keybind", node))
				_keybind.load(node->first_node("keybind"));

			if (nodeValid("controller", node))
				_conbind.load(node->first_node("controller"));

			if (nodeValid("graphics", node))
				_gfx.load(node->first_node("graphics"));

			if (nodeValid("general", node))
				_general.load(node->first_node("general"));

			if (nodeValid("change", node)) {
				rapidxml::xml_node<char> *chanode = node->first_node("change");

				if (nodeValid("accept", chanode))
					_accept.load(chanode->first_node("accept"));

				if (nodeValid("cancel", chanode))
					_cancel.load(chanode->first_node("cancel"));

				if (nodeValid("message", chanode))
					_noticeRes.load(chanode->first_node("message"));

				if (nodeValid("width", chanode))
					_promptW.load(chanode->first_node("width"));

				if (nodeValid("height", chanode))
					_promptH.load(chanode->first_node("height"));

				if (nodeValid("countdown", chanode)) {
					rapidxml::xml_node<char> *counode = chanode->first_node("countdown");
					_countdown.load(counode);
					_timer.load(counode, "time");
				}

				if (nodeValid("bg", chanode))
					_questionbox.load(chanode->first_node("bg"));
			}
		}
	}
}

void OptionMenu::reset() {
	_keybind.reset();
	_state = STATE_GENERAL;

	for (uint i = 0; i < _menu._element.size(); ++i)
		_menu._element[i].state(i == STATE_GENERAL);
}

void OptionMenu::draw(Button &back) {
	if (_state < STATE_ENTER_W) {
		_bg.draw();

		switch (_state) {
		case STATE_GENERAL:
			_general.draw();
			break;
		case STATE_GRAPHICS:
			_gfx.draw();
			break;
		case STATE_KEYBOARD:
			_keybind.draw();
			break;
		case STATE_CONTROLLER:
			_conbind.draw();
			break;
		default:
			break;
		}

		_menu.draw();
		back.draw();
	} else {
		_questionbox.draw();

		switch (_state) {
		case STATE_ENTER_W:
			_promptW.draw();
			break;
		case STATE_ENTER_H:
			_promptH.draw();
			break;
		case STATE_CONFIRM:
			_noticeRes.draw();
			_countdown.draw(numberToString(_timer.remainingTicks() / 1000));
			break;
		default:
			break;
		}

		_accept.draw();
		_cancel.draw();
	}
}

bool OptionMenu::handleEvents(Button &back, const Common::Event &event) {
	if (_state < STATE_ENTER_W) {
		switch (_state) {
		case STATE_GENERAL:
			_general.handleEvents(event);
			break;
		case STATE_KEYBOARD:
			_keybind.handleEvents(event);
			break;
		case STATE_GRAPHICS: {
			int result = _gfx.handleEvents(event);
			if (result == 1) {
				_state = STATE_CONFIRM;
				_timer.start();
				_gfx.SetInfo();
			} else if (result == 2)
				_state = STATE_ENTER_W;
		} break;
		default:
			break;
		}

		return handleTabs(back, event);
	} else {
		_questionbox.draw();

		switch (_state) {
		case STATE_ENTER_W:
			if (_promptW.handleEvents(event, true) || _accept.handleEvents(event) == BUAC_LCLICK) {
				g_engine->_screenSettings->_cur.w = stringToNumber<int>(_promptW._text);
				_state = STATE_ENTER_H;
			} else if (_cancel.handleEvents(event) == BUAC_LCLICK) {
				_gfx.SetInfo();
				_state = STATE_GRAPHICS;
			}
			break;
		case STATE_ENTER_H:
			if (_promptH.handleEvents(event, true) || _accept.handleEvents(event) == BUAC_LCLICK) {
				g_engine->_screenSettings->_cur.h = stringToNumber<int>(_promptH._text);
				_state = STATE_CONFIRM;
				_timer.start();
				_gfx.SetInfo();
			} else if (_cancel.handleEvents(event) == BUAC_LCLICK) {
				_gfx.SetInfo();
				_state = STATE_GRAPHICS;
			}

			break;
		case STATE_CONFIRM:
			if (_accept.handleEvents(event) != BUAC_IGNORE) {
				_state = STATE_GRAPHICS;
				_timer.stop();
			} else if (_cancel.handleEvents(event) != BUAC_IGNORE) {
				_gfx.SetInfo();
				_state = STATE_GRAPHICS;
			}
			break;
		default:
			break;
		}

		_accept.draw();
		_cancel.draw();
	}

	return false;
}

bool OptionMenu::handleTabs(Button &back, const Common::Event &event) {
	if (back.handleEvents(event) == BUAC_LCLICK) {
		reset();
		return true;
	}

	int choice = _menu.handleEvents(event);
	if (choice >= 0) {
		if (choice < 4)
			for (int i = 0; i < (int)_menu._element.size(); ++i)
				_menu._element[i].state(i == choice);

		switch (choice) {
		case 0:
			_state = STATE_GENERAL;
			break;
		case 1:
			_state = STATE_GRAPHICS;
			break;
		case 2:
			_state = STATE_KEYBOARD;
			break;
		case 3:
			_state = STATE_CONTROLLER;
			break;

		case 4:
			// Save settings to file
			//g_engine->_inputManager->Save();
			g_engine->_screenSettings->saveState();
			g_engine->_musicManager->saveState();
			saveState();
			//general.CreateBackup();
			//g_engine->_screenSettings->CreateBackup();
			return true;

		case 5:
			// Revert all changes made to settings and exit
			//g_engine->_inputManager->RestoreBackup();
			//keybind.SetCaption();
			//g_engine->_screenSettings->RestoreBackup();
			_general.restoreBackup();
			_general.setUI();
			return true;
		default:
			break;
		}
	}

	return false;
}

#if 0
bool OptionMenu::handleEvents(Button &back, const SDL_Event &Event) {
	if (state < STATE_ENTER_W) {
		bg.draw();

		switch (state) {
		case STATE_GENERAL:
			general.handleEvents(Event);
			break;
		case STATE_KEYBOARD:
			keybind.handleEvents(Event);
			break;
		case STATE_GRAPHICS: {
			int result = gfx.handleEvents(Event);
			if (result == 1) {
				state = STATE_CONFIRM;
				timer.Start();
				g_engine->_screenSettings->SetResolution();
				gfx.SetInfo();
			} else if (result == 2)
				state = STATE_ENTER_W;
		} break;
		default:
			break;
		}

		return HandleTabs(back, Event);
	} else {
		questionbox.draw();

		switch (state) {
		case STATE_ENTER_W:
			if (prompt_w.handleEvents(Event, true) || accept.handleEvents(Event) == BUAC_LCLICK) {
				g_engine->_screenSettings->cur.w = StringToNumber<int>(prompt_w.text);
				state = STATE_ENTER_H;
			} else if (cancel.handleEvents(Event) == BUAC_LCLICK) {
				g_engine->_screenSettings->RestoreBackup();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}
			break;
		case STATE_ENTER_H:
			if (prompt_h.handleEvents(Event, true) || accept.handleEvents(Event) == BUAC_LCLICK) {
				g_engine->_screenSettings->cur.h = StringToNumber<int>(prompt_h.text);
				state = STATE_CONFIRM;
				timer.Start();
				g_engine->_screenSettings->SetResolution();
				gfx.SetInfo();
			} else if (cancel.handleEvents(Event) == BUAC_LCLICK) {
				g_engine->_screenSettings->RestoreBackup();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}

			break;
		case STATE_CONFIRM:
			if (accept.handleEvents(Event)) {
				state = STATE_GRAPHICS;
				timer.Stop();
			} else if (cancel.handleEvents(Event)) {
				g_engine->_screenSettings->RestoreBackup();
				g_engine->_screenSettings->SetResolution();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}
			break;
		default:
			break;
		}

		accept.draw();
		cancel.draw();
	}

	return false;
}


bool OptionMenu::HandleTabs(Button &back, const SDL_Event &Event) {
	if (back.handleEvents(Event) == BUAC_LCLICK) {
		reset();
		return true;
	}

	int choice = menu.handleEvents(Event);
	if (choice >= 0) {
		if (choice < 4)
			for (uint i = 0; i < menu.element.size(); ++i)
				menu.element[i].State(i == choice);

		switch (choice) {
		case 0:
			state = STATE_GENERAL;
			break;
		case 1:
			state = STATE_GRAPHICS;
			break;
		case 2:
			state = STATE_KEYBOARD;
			break;
		case 3:
			state = STATE_CONTROLLER;
			break;

		case 4:
			// Save settings to file
			g_engine->_inputManager->Save();
			saveState();
			general.CreateBackup();
			g_engine->_screenSettings->CreateBackup();
			return true;

		case 5:
			// Revert all changes made to settings and exit
			g_engine->_inputManager->RestoreBackup();
			keybind.SetCaption();
			g_engine->_screenSettings->RestoreBackup();
			general.RestoreBackup();

			SDL_DisplayMode current;
			if (SDL_GetCurrentDisplayMode(0, &current) == 0) {
				if (g_engine->_screenSettings->cur.w != current.w || g_engine->_screenSettings->cur.h != current.h)
					gfx.SetInfo();
			}

			g_engine->_screenSettings->SetResolution();
			return true;
		default:
			break;
		}
	}

	return false;
}
#endif

void OptionMenu::internalEvents() {
	// Since these states can be changed at any time, we just update it regularly
	_gfx.internalEvents();
	_general.internalEvents();

	if (_state == STATE_CONFIRM && _timer.targetReached()) {
		_gfx.SetInfo();
		_state = STATE_GRAPHICS;
	}
}

void OptionMenu::saveState() {
	warning("STUB: OptionMenu::saveState()");

#if 0
	rapidxml::xml_document<char> doc;

	// xml declaration
	rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	// root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "settings");
	g_engine->_screenSettings->saveState(doc, root);
	g_engine->_musicManager->saveState(doc, root);

	doc.append_node(root);
	Common::String xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	Common::String settingpath = g_engine->_filePath->appdata;
	settingpath += "settings.xml";

	std::ofstream save(settingpath, std::ios::out);
	if (save.is_open()) {
		save << xml_as_string;
		save.close();
	}

	doc.clear();
#endif
}

void OptionMenu::setUI() {
	_bg.setUI();
	_menu.setUI();

	_keybind.setUI();
	_conbind.setUI();

	_gfx.setUI();
	_general.setUI();

	_noticeRes.setUI();

	_countdown.setUI();
	_questionbox.setUI();

	_promptW.setUI();
	_promptH.setUI();

	_accept.setUI();
	_cancel.setUI();
}

} // End of namespace Crab
