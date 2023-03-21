#include "stdafx.h"
#include "ModMenu.h"

using namespace pyrodactyl::ui;

void ModMenu::Load(const std::string &filename)
{
	XMLDoc conf(filename);
	if (conf.ready())
	{
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("mod_menu");

		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("menu", node))
			menu.Load(node->first_node("menu"));

		if (NodeValid("preview", node))
		{
			auto prnode = node->first_node("preview");
			img.pos.Load(prnode);
			LoadStr(img.no_preview_path, "path", prnode);
		}

		if (NodeValid("offset", node))
		{
			rapidxml::xml_node<char> *offnode = node->first_node("offset");

			//Stuff displayed on the slot button
			td_b[DATA_SAVENAME].Load(offnode->first_node("mod_name"));
			td_b[DATA_LASTMODIFIED].Load(offnode->first_node("last_modified"));

			//Stuff displayed when you hover over a slot button
			td_h[DATA_AUTHOR].Load(offnode->first_node("author"));
			td_h[DATA_VERSION].Load(offnode->first_node("version"));
			td_h[DATA_INFO].Load(offnode->first_node("info"));
			td_h[DATA_WEBSITE].Load(offnode->first_node("website"));

			//Titles for the stuff displayed when you hover over a slot button
			hov[DATA_AUTHOR].Load(offnode->first_node("author_title"));
			hov[DATA_VERSION].Load(offnode->first_node("info_title"));
			hov[DATA_INFO].Load(offnode->first_node("version_title"));
			hov[DATA_WEBSITE].Load(offnode->first_node("website_title"));
		}

		extension = gFilePath.mod_ext;
		directory = gFilePath.mod_path;
		ScanDir();
	}
}

bool ModMenu::HandleEvents(const SDL_Event &Event)
{
	int choice = menu.HandleEvents(Event);
	if (choice >= 0)
	{
		gFilePath.mod_cur = slot_info[menu.Index() + choice].path;
		return true;
	}

	return false;
}

void ModMenu::Draw()
{
	bg.Draw();
	menu.Draw();
	for (auto i = menu.Index(), count = 0u; i < menu.IndexPlusOne() && i < slot_info.size(); i++, count++)
	{
		auto base_x = menu.BaseX(count), base_y = menu.BaseY(count);
		td_b[DATA_SAVENAME].Draw(slot_info[i].name, base_x, base_y);
		td_b[DATA_LASTMODIFIED].Draw(slot_info[i].last_modified, base_x, base_y);
	}

	if (menu.HoverIndex() >= 0)
	{
		int i = menu.HoverIndex();

		if (!img.loaded || prev_hover != i)
		{
			img.loaded = true;
			prev_hover = i;
			if (!img.preview.Load(slot_info[i].preview))
				img.preview.Load(img.no_preview_path);
		}

		hover = true;
		img.preview.Draw(img.pos.x, img.pos.y);

		td_h[DATA_AUTHOR].Draw(slot_info[i].author);
		td_h[DATA_VERSION].Draw(slot_info[i].version);
		td_h[DATA_INFO].Draw(slot_info[i].info);
		td_h[DATA_WEBSITE].Draw(slot_info[i].website);

		for (int num = 0; num < DATA_HOVER_TOTAL; ++num)
			hov[num].Draw();
	}
	else if (hover)
		Reset();
}