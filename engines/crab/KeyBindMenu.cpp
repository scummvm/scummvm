#include "KeyBindMenu.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;

void KeyBindMenu::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		if (NodeValid("menu", node)) {
			rapidxml::xml_node<char> *menode = node->first_node("menu");

			if (NodeValid("primary", menode))
				prim.Load(menode->first_node("primary"));

			if (NodeValid("alt", menode))
				alt.Load(menode->first_node("alt"));

			if (NodeValid("prompt", menode))
				prompt.Load(menode->first_node("prompt"));

			if (NodeValid("inc", menode))
				inc.Load(menode->first_node("inc"));

			if (NodeValid("dim", menode))
				dim.Load(menode->first_node("dim"));

			if (NodeValid("divide", menode))
				divide.Load(menode->first_node("divide"));

			if (NodeValid("desc", menode))
				desc.Load(menode->first_node("desc"));
		}

		// Initialize the menus
		InitMenu(CON_GAME);
		InitMenu(CON_UI);

		if (NodeValid("controls", node))
			sel_controls.Load(node->first_node("controls"));
	}
}

void KeyBindMenu::StartAndSize(const int &type, int &start, int &size) {
	switch (type) {
	case CON_GAME:
		start = IG_START;
		size = IG_SIZE;
		break;
	case CON_UI:
		start = IU_START;
		size = IU_SIZE;
		break;
	default:
		break;
	}
}

void KeyBindMenu::InitMenu(const int &type) {
	int start = 0, size = 0;
	StartAndSize(type, start, size);

	// Initialize the menu
	menu[type].element.resize(size * 2);
	for (int i = 0; i < size * 2; i += 2) {
		int xoffset = inc.x * (i / 2) + divide.x * ((i / 2) / dim.x);
		int yoffset = inc.y * ((i / 2) % dim.x) + divide.y * ((i / 2) / dim.x);

		menu[type].element.at(i).Init(prim, xoffset, yoffset);
		menu[type].element.at(i).caption.text = SDL_GetScancodeName(gInput.iv[start + (i / 2)].key);

		menu[type].element.at(i + 1).Init(alt, xoffset, yoffset);
		menu[type].element.at(i + 1).caption.text = SDL_GetScancodeName(gInput.iv[start + (i / 2)].alt);
	}
}

void KeyBindMenu::DrawDesc(const int &type) {
	int start = 0, size = 0;
	StartAndSize(type, start, size);

	for (int i = 0; i < size; i++) {
		int xoffset = inc.x * i + divide.x * (i / dim.x);
		int yoffset = inc.y * (i % dim.x) + divide.y * (i / dim.x);

		desc.Draw(gInput.iv[i + start].name, xoffset, yoffset);
	}
}

void KeyBindMenu::Draw() {
	sel_controls.Draw();

	menu[sel_controls.cur].Draw();
	DrawDesc(sel_controls.cur);
}

void KeyBindMenu::SetCaption() {
	int start = 0, size = 0;
	StartAndSize(sel_controls.cur, start, size);

	for (int i = 0; i < size * 2; i += 2) {
		menu[sel_controls.cur].element.at(i).caption.text = SDL_GetScancodeName(gInput.iv[start + (i / 2)].key);
		menu[sel_controls.cur].element.at(i + 1).caption.text = SDL_GetScancodeName(gInput.iv[start + (i / 2)].alt);
	}
}

void KeyBindMenu::HandleEvents(const SDL_Event &Event) {
	if (sel_controls.HandleEvents(Event))
		SetCaption();

	switch (state) {
	case STATE_NORMAL:
		choice = menu[sel_controls.cur].HandleEvents(Event);
		if (choice >= 0) {
			prompt.Swap(menu[sel_controls.cur].element.at(choice).caption);
			state = STATE_KEY;
			break;
		}

		break;
	case STATE_KEY:
		if (Event.type == SDL_KEYDOWN) {
			SwapKey(Event.key.keysym.scancode);
			SetCaption();
			menu[sel_controls.cur].element.at(choice).caption.col = prompt.col_prev;
			state = STATE_NORMAL;
		}
		break;
	default:
		break;
	}
}

void KeyBindMenu::SwapKey(const SDL_Scancode &find) {
	int start = 0, size = 0;
	StartAndSize(sel_controls.cur, start, size);
	int pos = start + (choice / 2);

	for (int i = start; i < start + size; ++i) {
		if (gInput.iv[i].key == find) {
			gInput.iv[i].key = gInput.iv[pos].key;
			break;
		} else if (gInput.iv[i].alt == find) {
			gInput.iv[i].alt = gInput.iv[pos].key;
			break;
		}
	}

	if (choice % 2 == 0)
		gInput.iv[pos].key = find;
	else
		gInput.iv[pos].alt = find;
}

void KeyBindMenu::SetUI() {
	menu[CON_GAME].Clear();
	menu[CON_UI].Clear();

	// Initialize the menus
	prim.SetUI();
	alt.SetUI();
	InitMenu(CON_GAME);
	InitMenu(CON_UI);

	desc.SetUI();
	sel_controls.SetUI();
}