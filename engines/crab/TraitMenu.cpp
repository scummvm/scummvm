#include "stdafx.h"
#include "TraitMenu.h"

using namespace pyrodactyl::ui;

void TraitMenu::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("dim", node))
	{
		rapidxml::xml_node<char> *dimnode = node->first_node("dim");
		LoadNum(rows, "rows", dimnode);
		LoadNum(cols, "cols", dimnode);
		size = rows * cols;
	}

	if (NodeValid("ref", node))
		ref.Load(node->first_node("ref"));

	if (NodeValid("inc", node))
		inc.Load(node->first_node("inc"));

	if (NodeValid("desc", node))
		desc.Load(node->first_node("desc"));

	for (auto i = 0; i < size; ++i)
	{
		TraitButton b;
		b.Init(ref, inc.x*(i%cols), inc.y*(i / cols));
		menu.element.push_back(b);
	}

	bool usekey = false;
	LoadBool(usekey, "keyboard", node);
	menu.UseKeyboard(usekey);

	menu.AssignPaths();
}

void TraitMenu::Draw(const pyrodactyl::people::Person *obj)
{
	if (obj != nullptr)
	{
		auto i = menu.element.begin();
		for (auto t = obj->trait.begin(); t != obj->trait.end() && i != menu.element.end(); ++t, ++i)
		{
			i->Draw();
			if (t->unread)
				pyrodactyl::image::gImageManager.NotifyDraw(i->x + i->w, i->y);
		}

		for (; i != menu.element.end(); ++i)
			i->Draw();

		if (select > -1 && select < obj->trait.size())
			desc.Draw(obj->trait.at(select).desc);
	}
	else
		for (auto &i : menu.element)
			i.Draw();
}

void TraitMenu::HandleEvents(pyrodactyl::people::Person *obj, const SDL_Event &Event)
{
	int choice = menu.HandleEvents(Event);
	if (choice >= 0)
	{
		for (auto i = menu.element.begin(); i != menu.element.end(); ++i)
			i->State(false);

		menu.element.at(choice).State(true);
		select = choice;

		if (obj != nullptr)
		{
			if (select > -1 && select < obj->trait.size())
				obj->trait.at(select).unread = false;
		}
	}
}

void TraitMenu::Cache(const pyrodactyl::people::Person &obj)
{
	auto e = menu.element.begin();

	for (auto i = obj.trait.begin(); i != obj.trait.end() && e != menu.element.end(); ++i, ++e)
		e->Cache(*i);

	for (; e != menu.element.end(); ++e)
		e->Empty();
}

void TraitMenu::Clear()
{
	for (auto e = menu.element.begin(); e != menu.element.end(); ++e)
		e->Empty();
}

void TraitMenu::SetUI()
{
	ref.SetUI();
	desc.SetUI();
	menu.SetUI();
}