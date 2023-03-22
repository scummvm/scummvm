//=============================================================================
// Author:   Arvind
// Purpose:  Dialog box!
//=============================================================================
#include "dialogbox.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Load stuff
//------------------------------------------------------------------------
void GameDialogBox::Load(rapidxml::xml_node<char> *node) {
	LoadImgKey(bg, "bg", node);
	LoadImgKey(bg_p, "bg_p", node);
	pos.Load(node, bg);

	if (NodeValid("text", node))
		text.Load(node->first_node("text"), &pos);

	if (NodeValid("button", node))
		button.Load(node->first_node("button"));
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box background
//------------------------------------------------------------------------
void GameDialogBox::Draw(const bool &player) {
	if (player)
		gImageManager.Draw(pos.x, pos.y, bg_p);
	else
		gImageManager.Draw(pos.x, pos.y, bg);
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box text
//------------------------------------------------------------------------
void GameDialogBox::Draw(pyrodactyl::event::Info &info, std::string &message) {
	// Create a copy of the string
	std::string msg = message;
	info.InsertName(msg);

	text.Draw(message);
	button.Draw();
}
//------------------------------------------------------------------------
// Purpose: Handle input
//------------------------------------------------------------------------
bool GameDialogBox::HandleEvents(const SDL_Event &Event) {
	return (button.HandleEvents(Event) == BUAC_LCLICK);
}

void GameDialogBox::SetUI() {
	pos.SetUI();
	text.SetUI(&pos);
	button.SetUI();
}