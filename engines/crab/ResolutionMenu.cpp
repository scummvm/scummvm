#include "stdafx.h"

#include "ResolutionMenu.h"

using namespace pyrodactyl::image;
using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void ResolutionMenu::Load(rapidxml::xml_node<char> *node)
{
	cancel.Load(node->first_node("cancel"));
	change.Load(node->first_node("change"));
	custom.Load(node->first_node("custom"));

	info.Load(node->first_node("info"));
	def_info = info.text;

	if (NodeValid("reference", node))
		ref.Load(node->first_node("reference"));

	if (NodeValid("inc", node))
	{
		inc.Load(node->first_node("inc"));
		LoadNum(columns, "columns", node->first_node("inc"));
	}

	if (NodeValid("options", node))
	{
		int count_slot = 0;
		rapidxml::xml_node<char> *resnode = node->first_node("options");
		for (auto n = resnode->first_node("res"); n != NULL; n = n->next_sibling("res"), count_slot++)
		{
			Dimension d;
			LoadNum(d.w, "x", n);
			LoadNum(d.h, "y", n);

			if (gScreenSettings.ValidDimension(d))
			{
				dim.push_back(d);
				Button b;
				b.Init(ref, inc.x * (count_slot % columns), inc.y * (count_slot / columns));
				b.caption.text = NumberToString(d.w) + " x " + NumberToString(d.h);
				element.push_back(b);
			}
		}
	}

	SetInfo();

	LoadBool(use_keyboard, "keyboard", node, false);
	AssignPaths();
}

void ResolutionMenu::Draw()
{
	info.Draw();

	switch (state)
	{
	case STATE_NORMAL:
		change.Draw();
		break;
	case STATE_CHANGE:
		Menu::Draw();
		cancel.Draw();
		custom.Draw();
		break;
	default:break;
	}
}

int ResolutionMenu::HandleEvents(const SDL_Event &Event)
{
	switch (state)
	{
	case STATE_NORMAL:
		if (change.HandleEvents(Event) == BUAC_LCLICK)
			state = STATE_CHANGE;
		break;
	case STATE_CHANGE:
	{
		int choice = Menu::HandleEvents(Event);
		if (choice >= 0)
		{
			gScreenSettings.cur = dim[choice];
			state = STATE_NORMAL;
			return 1;
		}

		if (custom.HandleEvents(Event) == BUAC_LCLICK)
		{
			state = STATE_NORMAL;
			return 2;
		}

		if (cancel.HandleEvents(Event) == BUAC_LCLICK)
			state = STATE_NORMAL;
	}
		break;
	default:break;
	}

	return 0;
}

void ResolutionMenu::SetUI()
{
	cancel.SetUI();
	change.SetUI();
	custom.SetUI();

	info.SetUI();
	ref.SetUI();
	ButtonMenu::SetUI();
}