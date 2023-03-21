#include "stdafx.h"
#include "DevConsole.h"

using namespace pyrodactyl::ui;

void DebugConsole::Load(const std::string &filename)
{
	XMLDoc conf(filename);
	if (conf.ready())
	{
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("debug");
		if (NodeValid(node))
		{
			if (NodeValid("menu", node))
				menu.Load(node->first_node("menu"));

			if (NodeValid("variable", node))
			{
				rapidxml::xml_node<char> *varnode = node->first_node("variable");

				if (NodeValid("bg", varnode))
					bg.Load(varnode->first_node("bg"));

				if (NodeValid("check", varnode))
					check.Load(varnode->first_node("check"));

				if (NodeValid("back", varnode))
					back.Load(varnode->first_node("back"));

				if (NodeValid("value", varnode))
					value.Load(varnode->first_node("value"));

				if (NodeValid("text", varnode))
					text_field.Load(varnode->first_node("text"));
			}
		}
	}
}

void DebugConsole::Draw(pyrodactyl::event::Info &info)
{
	switch (state)
	{
	case STATE_NORMAL:
		menu.Draw();
		break;
	case STATE_VAR:
		bg.Draw();
		check.Draw();
		back.Draw();
		text_field.Draw();

		{
			int temp = 0;
			if (info.VarGet(var_name, temp))
				value.Draw(NumberToString(temp));
			else
				value.Draw("Does not exist.");
		}
		break;

	default:break;
	}
}

void DebugConsole::HandleEvents(const SDL_Event &Event)
{
	switch (state)
	{
	case STATE_NORMAL:
	{
		int choice = menu.HandleEvents(Event);
		if (choice == 0)
			state = STATE_VAR;
	}
		break;
	case STATE_VAR:
		text_field.HandleEvents(Event);

		if (check.HandleEvents(Event))
			var_name = text_field.text;

		//Control+V pastes clipboard text into text field
		if (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_V && Event.key.keysym.mod & KMOD_CTRL)
		{
			if (SDL_HasClipboardText() == SDL_TRUE)
			{
				char* temp = SDL_GetClipboardText();
				text_field.text = temp;
				SDL_free(temp);
			}
		}

		if (back.HandleEvents(Event))
			state = STATE_NORMAL;
		break;

	default:break;
	}
}

void DebugConsole::InternalEvents()
{
}