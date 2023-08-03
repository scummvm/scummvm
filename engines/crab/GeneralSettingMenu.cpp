#include "stdafx.h"
#include "GeneralSettingMenu.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::music;

//------------------------------------------------------------------------
// Purpose: Load components from file
//------------------------------------------------------------------------
void GeneralSettingMenu::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("volume", node))
	{
		rapidxml::xml_node<char> *musnode = node->first_node("volume");

		if (NodeValid("desc", musnode))
			notice_volume.Load(musnode->first_node("desc"));

		if (NodeValid("music", musnode))
			vol_music.Load(musnode->first_node("music"), 0, MIX_MAX_VOLUME, gMusicManager.VolMusic());

		if (NodeValid("effects", musnode))
			vol_effects.Load(musnode->first_node("effects"), 0, MIX_MAX_VOLUME, gMusicManager.VolEffects());
	}

	if (NodeValid("mouse_trap", node))
		mouse_trap.Load(node->first_node("mouse_trap"));

	if (NodeValid("save_on_exit", node))
		save_on_exit.Load(node->first_node("save_on_exit"));

	if (NodeValid("text_speed", node))
		text_speed.Load(node->first_node("text_speed"));

	//Sync popup text value with actual value
	for (auto &i : text_speed.element)
		i.state = (i.val == gScreenSettings.text_speed);
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
void GeneralSettingMenu::HandleEvents(const SDL_Event &Event)
{
	if (vol_music.HandleEvents(Event))
		gMusicManager.VolMusic(vol_music.Value());

	if (vol_effects.HandleEvents(Event))
		gMusicManager.VolEffects(vol_effects.Value());

	//No need to change screen here
	if (save_on_exit.HandleEvents(Event))
		gScreenSettings.save_on_exit = !gScreenSettings.save_on_exit;

	if (mouse_trap.HandleEvents(Event))
	{
		gScreenSettings.mouse_trap = !gScreenSettings.mouse_trap;
		gScreenSettings.SetMouseTrap();
	}

	int result = text_speed.HandleEvents(Event);
	if (result >= 0)
		gScreenSettings.text_speed = text_speed.element.at(result).val;
}

//------------------------------------------------------------------------
// Purpose: Sync our buttons with screen settings
//------------------------------------------------------------------------
void GeneralSettingMenu::InternalEvents()
{
	save_on_exit.state = gScreenSettings.save_on_exit;
	mouse_trap.state = gScreenSettings.mouse_trap;
}

//------------------------------------------------------------------------
// Purpose: Draw stuff
//------------------------------------------------------------------------
void GeneralSettingMenu::Draw()
{
	//Draw volume sliders
	notice_volume.Draw();
	vol_music.Draw();
	vol_effects.Draw();

	//Draw the auto-save on exit option
	save_on_exit.Draw();
	mouse_trap.Draw();

	//Text speed radio button menu
	text_speed.Draw();
}

//------------------------------------------------------------------------
// Purpose: Revert to previously backed up settings
//------------------------------------------------------------------------
void GeneralSettingMenu::RestoreBackup()
{
	vol_music.RestoreBackup();
	gMusicManager.VolMusic(vol_music.Value());

	vol_effects.RestoreBackup();
	gMusicManager.VolEffects(vol_effects.Value());
}

//------------------------------------------------------------------------
// Purpose: Set UI positioned on screen size change
//------------------------------------------------------------------------
void GeneralSettingMenu::SetUI()
{
	save_on_exit.SetUI();
	mouse_trap.SetUI();

	vol_music.SetUI();
	vol_effects.SetUI();

	text_speed.SetUI();
}