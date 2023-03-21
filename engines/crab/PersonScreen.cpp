#include "stdafx.h"
#include "PersonScreen.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::event;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;

void PersonScreen::Load(const std::string &filename)
{
	XMLDoc conf(filename);
	if (conf.ready())
	{
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("character");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("name", node))
				name.Load(node->first_node("name"));

			if (NodeValid("img", node))
				img.Load(node->first_node("img"));

			if (NodeValid("menu", node))
				menu.Load(node->first_node("menu"));
		}
	}
}

void PersonScreen::HandleEvents(pyrodactyl::event::Info &info, const std::string &id, const SDL_Event &Event)
{
	if (info.PersonValid(id))
		menu.HandleEvents(&info.PersonGet(id), Event);
	else
		menu.HandleEvents(nullptr, Event);
}

void PersonScreen::InternalEvents()
{
	if (cur_sp != nullptr)
		cur_sp->DialogUpdateClip(PST_NORMAL);
}

void PersonScreen::Draw(pyrodactyl::event::Info &info, const std::string &id)
{
	bg.Draw();

	if (info.PersonValid(id))
	{
		name.Draw(info.PersonGet(id).name);
		menu.Draw(&info.PersonGet(id));
	}
	else
		menu.Draw(nullptr);

	if (cur_sp != nullptr)
	{
		Rect clip = cur_sp->DialogClip(PST_NORMAL);
		gImageManager.Draw(img.x, img.y, cur_sp->Img(), &clip);
	}
}

void PersonScreen::Cache(Info &info, const std::string &id, pyrodactyl::level::Level &level)
{
	cur_sp = level.GetSprite(id);

	if (info.PersonValid(id))
		menu.Cache(info.PersonGet(id));
	else
		menu.Clear();
}

void PersonScreen::SetUI()
{
	bg.SetUI();
	name.SetUI();
	img.SetUI();
	menu.SetUI();
}