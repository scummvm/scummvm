#include "stdafx.h"
#include "ChapterIntro.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ChapterIntro::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("dialog", node))
		dialog.Load(node->first_node("dialog"));

	if (NodeValid("image", node))
		pos.Load(node->first_node("image"));

	if (NodeValid("trait", node))
		traits.Load(node->first_node("trait"));
}

bool ChapterIntro::HandleEvents(SDL_Event &Event)
{
	if (traits.HandleEvents(Event))
		show_traits = true;

	return dialog.HandleEvents(Event);
}

void ChapterIntro::Draw(pyrodactyl::event::Info &info, std::string &text,
	pyrodactyl::anim::Sprite *cur_sp, const pyrodactyl::people::PersonState &state)
{
	dialog.Draw(false);
	dialog.Draw(info, text);

	traits.Draw();

	if (cur_sp != nullptr)
	{
		Rect clip = cur_sp->DialogClip(state);
		gImageManager.Draw(pos.x, pos.y, cur_sp->Img(), &clip);
	}
}