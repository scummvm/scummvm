#include "stdafx.h"
#include "PauseMenu.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void PauseMenu::Load(rapidxml::xml_node<char> *node)
{
	menu.Load(node->first_node("menu"));
	save.Load(node->first_node("save"));

	if (NodeValid("bg", node))
		bg.Load(node->first_node("bg"));
}

bool PauseMenu::Draw(Button &back)
{
	switch (state)
	{
	case STATE_NORMAL:
		bg.Draw();
		menu.Draw();
		break;
	case STATE_SAVE:
		save.Draw();
		back.Draw();
		break;
	case STATE_LOAD:
		gLoadMenu.Draw();
		back.Draw();
		break;
	case STATE_OPTION:
		gOptionMenu.Draw(back);
		return true;
	default:break;
	}

	return false;
}

PauseSignal PauseMenu::HandleEvents(const SDL_Event &Event, Button &back)
{
	switch (state)
	{
	case STATE_NORMAL:
		choice = menu.HandleEvents(Event);
		if (choice == -1)
		{
			if (back.hotkey.HandleEvents(Event))
				return PS_RESUME;
		}
		else
		{
			switch (choice)
			{
			case 0: state = STATE_NORMAL; return PS_RESUME;
			case 1: state = STATE_SAVE; save.ScanDir(); break;
			case 2: state = STATE_LOAD; gLoadMenu.ScanDir(); break;
			case 3: state = STATE_OPTION; break;
			case 4: return PS_HELP;
			case 5: return PS_QUIT_MENU;
			case 6: return PS_QUIT_GAME;
			default: break;
			}
		}
		break;
	case STATE_SAVE:
		if (save.HandleEvents(Event))
		{
			state = STATE_NORMAL;
			return PS_SAVE;
		}
		else if (back.HandleEvents(Event) == BUAC_LCLICK && !save.DisableHotkeys())
			state = STATE_NORMAL;
		break;
	case STATE_OPTION:
		if (gOptionMenu.HandleEvents(back, Event))
		{
			gOptionMenu.Reset();
			state = STATE_NORMAL;
		}
		break;
	case STATE_LOAD:
		if (gLoadMenu.HandleEvents(Event))
			return PS_LOAD;
		else if (back.HandleEvents(Event) == BUAC_LCLICK)
			state = STATE_NORMAL;
		break;
	default:break;
	}

	return PS_NONE;
}

bool PauseMenu::DisableHotkeys()
{
	return (state == STATE_SAVE && save.DisableHotkeys()) || (state == STATE_OPTION && gOptionMenu.DisableHotkeys());
}

void PauseMenu::SetUI()
{
	bg.SetUI();
	menu.SetUI();
	save.SetUI();
}