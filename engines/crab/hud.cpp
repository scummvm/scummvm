//=============================================================================
// Author:   Arvind
// Purpose:  The tray where you have inventory, map and GameObjectives icons
//=============================================================================
#include "stdafx.h"
#include "hud.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void HUD::Load(const std::string &filename, pyrodactyl::level::TalkNotify &tn, pyrodactyl::level::PlayerDestMarker &pdm)
{
	XMLDoc conf(filename);
	if (conf.ready())
	{
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("hud");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("tray", node))
				menu.Load(node->first_node("tray"));

			pause.Load(node->first_node("pause"));
			gom.Load(node->first_node("game_over"));
			back.Load(node->first_node("back"));
			//health.Load(node->first_node("health"));

			if (NodeValid("notify", node))
			{
				rapidxml::xml_node<char> *notifynode = node->first_node("notify");

				LoadImgKey(gImageManager.notify, "img", notifynode);
				tn.Load(notifynode);
				pdm.Load(notifynode);

				if (NodeValid("anim", notifynode))
				{
					rapidxml::xml_node<char> *animnode = notifynode->first_node("anim");
					LoadImgKey(notify_anim, "img", animnode);
					clip.Load(animnode);

					timer.Load(animnode, "delay");
					timer.Start();
				}
			}
		}

		//Create a copy of all the tooltips
		for (auto i = menu.element.begin(); i != menu.element.end(); ++i)
			tooltip.push_back(i->tooltip.text);

		SetTooltip();
	}
}

void HUD::Draw(pyrodactyl::event::Info &info, const std::string &id)
{
	bg.Draw();
	menu.Draw();

	if (info.unread.journal)
	{
		gImageManager.Draw(menu.element.at(HS_JOURNAL).x + menu.element.at(HS_JOURNAL).w - clip.w / 2,
			menu.element.at(HS_JOURNAL).y - clip.h / 2, notify_anim, &clip);
	}

	if (info.unread.inventory)
	{
		gImageManager.Draw(menu.element.at(HS_INV).x + menu.element.at(HS_INV).w - clip.w / 2,
			menu.element.at(HS_INV).y - clip.h / 2, notify_anim, &clip);
	}

	if (info.unread.trait)
	{
		gImageManager.Draw(menu.element.at(HS_CHAR).x + menu.element.at(HS_CHAR).w - clip.w / 2,
			menu.element.at(HS_CHAR).y - clip.h / 2, notify_anim, &clip);
	}

	if (info.unread.map)
	{
		gImageManager.Draw(menu.element.at(HS_MAP).x + menu.element.at(HS_MAP).w - clip.w / 2,
			menu.element.at(HS_MAP).y - clip.h / 2, notify_anim, &clip);
	}
}

void HUD::InternalEvents(bool ShowMap)
{
	menu.element.at(HS_MAP).visible = ShowMap;

	if (timer.TargetReached())
	{
		clip.x += clip.w;

		if (clip.x >= gImageManager.GetTexture(notify_anim).W())
			clip.x = 0;

		timer.Start();
	}
}

HUDSignal HUD::HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event)
{
	gMouse.inside_hud = bg.Contains(gMouse.motion.x, gMouse.motion.y);

	int choice = menu.HandleEvents(Event);

	if (choice == HS_JOURNAL) info.unread.journal = false;
	else if (choice == HS_INV) info.unread.inventory = false;
	else if (choice == HS_CHAR) info.unread.trait = false;
	else if (choice == HS_MAP) info.unread.map = false;

	return static_cast<HUDSignal>(choice);
}

void HUD::State(const int &val)
{
	int count = 0;
	for (auto i = menu.element.begin(); i != menu.element.end(); ++i, ++count)
		i->State(val == count);
}

void HUD::SetTooltip()
{
	int count = 0;
	for (auto i = menu.element.begin(); i != menu.element.end() && count < tooltip.size(); ++i, ++count)
		i->tooltip.text = tooltip.at(count) + " (" + i->hotkey.Name() + ")";

	menu.element.at(HS_PAUSE).tooltip.text = tooltip.at(HS_PAUSE) + " (" + SDL_GetScancodeName(gInput.iv[IG_PAUSE].key) + ")";
}

void HUD::SetUI()
{
	bg.SetUI();
	menu.SetUI();

	gom.SetUI();
	pause.SetUI();
	back.SetUI();
}