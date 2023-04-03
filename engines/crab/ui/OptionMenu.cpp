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

#include "crab/ui/OptionMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::music;

namespace pyrodactyl {
namespace ui {
OptionMenu gOptionMenu;
}
} // End of namespace pyrodactyl

void OptionMenu::Load(const std::string &filename) {
	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("option");
		if (NodeValid(node)) {
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("state", node)) {
				menu.Load(node->first_node("state"));

				if (!menu.element.empty())
					menu.element.at(0).State(true);
			}

			if (NodeValid("keybind", node))
				keybind.Load(node->first_node("keybind"));

			if (NodeValid("controller", node))
				conbind.Load(node->first_node("controller"));

			if (NodeValid("graphics", node))
				gfx.Load(node->first_node("graphics"));

			if (NodeValid("general", node))
				general.Load(node->first_node("general"));

			if (NodeValid("change", node)) {
				rapidxml::xml_node<char> *chanode = node->first_node("change");

				if (NodeValid("accept", chanode))
					accept.Load(chanode->first_node("accept"));

				if (NodeValid("cancel", chanode))
					cancel.Load(chanode->first_node("cancel"));

				if (NodeValid("message", chanode))
					notice_res.Load(chanode->first_node("message"));

				if (NodeValid("width", chanode))
					prompt_w.Load(chanode->first_node("width"));

				if (NodeValid("height", chanode))
					prompt_h.Load(chanode->first_node("height"));

				if (NodeValid("countdown", chanode)) {
					rapidxml::xml_node<char> *counode = chanode->first_node("countdown");
					countdown.Load(counode);
					timer.Load(counode, "time");
				}

				if (NodeValid("bg", chanode))
					questionbox.Load(chanode->first_node("bg"));
			}
		}
	}
}

void OptionMenu::Reset() {
	keybind.Reset();
	state = STATE_GENERAL;

	for (unsigned i = 0; i < menu.element.size(); ++i)
		menu.element.at(i).State(i == STATE_GENERAL);
}

void OptionMenu::Draw(Button &back) {
	if (state < STATE_ENTER_W) {
		bg.Draw();

		switch (state) {
		case STATE_GENERAL:
			general.Draw();
			break;
		case STATE_GRAPHICS:
			gfx.Draw();
			break;
		case STATE_KEYBOARD:
			keybind.Draw();
			break;
		case STATE_CONTROLLER:
			conbind.Draw();
			break;
		default:
			break;
		}

		menu.Draw();
		back.Draw();
	} else {
		questionbox.Draw();

		switch (state) {
		case STATE_ENTER_W:
			prompt_w.Draw();
			break;
		case STATE_ENTER_H:
			prompt_h.Draw();
			break;
		case STATE_CONFIRM:
			notice_res.Draw();
			countdown.Draw(NumberToString(timer.RemainingTicks() / 1000).c_str());
			break;
		default:
			break;
		}

		accept.Draw();
		cancel.Draw();
	}
}

bool OptionMenu::HandleEvents(Button &back, const Common::Event &Event) {
	if (state < STATE_ENTER_W) {
		bg.Draw();

		switch (state) {
		case STATE_GENERAL:
			general.HandleEvents(Event);
			break;
		case STATE_KEYBOARD:
			keybind.HandleEvents(Event);
			break;
		case STATE_GRAPHICS: {
			int result = gfx.HandleEvents(Event);
			if (result == 1) {
				state = STATE_CONFIRM;
				timer.Start();
				gScreenSettings.SetResolution();
				gfx.SetInfo();
			} else if (result == 2)
				state = STATE_ENTER_W;
		} break;
		default:
			break;
		}

		return HandleTabs(back, Event);
	} else {
		questionbox.Draw();

		switch (state) {
		case STATE_ENTER_W:
			if (prompt_w.HandleEvents(Event, true) || accept.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.cur.w = StringToNumber<int>(prompt_w.text);
				state = STATE_ENTER_H;
			} else if (cancel.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.RestoreBackup();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}
			break;
		case STATE_ENTER_H:
			if (prompt_h.HandleEvents(Event, true) || accept.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.cur.h = StringToNumber<int>(prompt_h.text);
				state = STATE_CONFIRM;
				timer.Start();
				gScreenSettings.SetResolution();
				gfx.SetInfo();
			} else if (cancel.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.RestoreBackup();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}

			break;
		case STATE_CONFIRM:
			if (accept.HandleEvents(Event)) {
				state = STATE_GRAPHICS;
				timer.Stop();
			} else if (cancel.HandleEvents(Event)) {
				gScreenSettings.RestoreBackup();
				gScreenSettings.SetResolution();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}
			break;
		default:
			break;
		}

		accept.Draw();
		cancel.Draw();
	}

	return false;
}

bool OptionMenu::HandleTabs(Button &back, const Common::Event &Event) {
	warning("STUB: OptionMenu::HandleTabs()");

	return false;
}

#if 0
bool OptionMenu::HandleEvents(Button &back, const SDL_Event &Event) {
	if (state < STATE_ENTER_W) {
		bg.Draw();

		switch (state) {
		case STATE_GENERAL:
			general.HandleEvents(Event);
			break;
		case STATE_KEYBOARD:
			keybind.HandleEvents(Event);
			break;
		case STATE_GRAPHICS: {
			int result = gfx.HandleEvents(Event);
			if (result == 1) {
				state = STATE_CONFIRM;
				timer.Start();
				gScreenSettings.SetResolution();
				gfx.SetInfo();
			} else if (result == 2)
				state = STATE_ENTER_W;
		} break;
		default:
			break;
		}

		return HandleTabs(back, Event);
	} else {
		questionbox.Draw();

		switch (state) {
		case STATE_ENTER_W:
			if (prompt_w.HandleEvents(Event, true) || accept.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.cur.w = StringToNumber<int>(prompt_w.text);
				state = STATE_ENTER_H;
			} else if (cancel.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.RestoreBackup();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}
			break;
		case STATE_ENTER_H:
			if (prompt_h.HandleEvents(Event, true) || accept.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.cur.h = StringToNumber<int>(prompt_h.text);
				state = STATE_CONFIRM;
				timer.Start();
				gScreenSettings.SetResolution();
				gfx.SetInfo();
			} else if (cancel.HandleEvents(Event) == BUAC_LCLICK) {
				gScreenSettings.RestoreBackup();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}

			break;
		case STATE_CONFIRM:
			if (accept.HandleEvents(Event)) {
				state = STATE_GRAPHICS;
				timer.Stop();
			} else if (cancel.HandleEvents(Event)) {
				gScreenSettings.RestoreBackup();
				gScreenSettings.SetResolution();
				gfx.SetInfo();
				state = STATE_GRAPHICS;
			}
			break;
		default:
			break;
		}

		accept.Draw();
		cancel.Draw();
	}

	return false;
}


bool OptionMenu::HandleTabs(Button &back, const SDL_Event &Event) {
	if (back.HandleEvents(Event) == BUAC_LCLICK) {
		Reset();
		return true;
	}

	int choice = menu.HandleEvents(Event);
	if (choice >= 0) {
		if (choice < 4)
			for (unsigned i = 0; i < menu.element.size(); ++i)
				menu.element.at(i).State(i == choice);

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
			pyrodactyl::input::gInput.Save();
			SaveState();
			general.CreateBackup();
			gScreenSettings.CreateBackup();
			return true;

		case 5:
			// Revert all changes made to settings and exit
			pyrodactyl::input::gInput.RestoreBackup();
			keybind.SetCaption();
			gScreenSettings.RestoreBackup();
			general.RestoreBackup();

			SDL_DisplayMode current;
			if (SDL_GetCurrentDisplayMode(0, &current) == 0) {
				if (gScreenSettings.cur.w != current.w || gScreenSettings.cur.h != current.h)
					gfx.SetInfo();
			}

			gScreenSettings.SetResolution();
			return true;
		default:
			break;
		}
	}

	return false;
}
#endif

void OptionMenu::InternalEvents() {
	// Since these states can be changed at any time, we just update it regularly
	gfx.InternalEvents();
	general.InternalEvents();

	if (state == STATE_CONFIRM && timer.TargetReached()) {
		gScreenSettings.RestoreBackup();
		gScreenSettings.SetResolution();
		gfx.SetInfo();
		state = STATE_GRAPHICS;
	}
}

void OptionMenu::SaveState() {
	warning("STUB: OptionMenu::SaveState()");

#if 0
	rapidxml::xml_document<char> doc;

	// xml declaration
	rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	// root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "settings");
	gScreenSettings.SaveState(doc, root);
	gMusicManager.SaveState(doc, root);

	doc.append_node(root);
	std::string xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	std::string settingpath = gFilePath.appdata;
	settingpath += "settings.xml";

	std::ofstream save(settingpath, std::ios::out);
	if (save.is_open()) {
		save << xml_as_string;
		save.close();
	}

	doc.clear();
#endif
}

void OptionMenu::SetUI() {
	bg.SetUI();
	menu.SetUI();

	keybind.SetUI();
	conbind.SetUI();

	gfx.SetUI();
	general.SetUI();

	notice_res.SetUI();

	countdown.SetUI();
	questionbox.SetUI();

	prompt_w.SetUI();
	prompt_h.SetUI();

	accept.SetUI();
	cancel.SetUI();
}

} // End of namespace Crab
