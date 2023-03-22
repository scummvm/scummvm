//=============================================================================
// Author:   Arvind
// Purpose:  TextArea functions
//=============================================================================
#include "textarea.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;

void TextArea::Load(rapidxml::xml_node<char> *node) {
	if (TextData::Load(node)) {
		LoadStr(text, "text", node);
		LoadNum(size, "size", node);

		LoadNum(se_entry, "entry", node);
		LoadNum(se_erase, "erase", node);
		LoadNum(se_accept, "accept", node);

		if (NodeValid("caption", node))
			title.Load(node->first_node("caption"), this);
	}
}

bool TextArea::HandleEvents(const SDL_Event &Event, bool numbers_only) {
	// If a key was pressed
	if (Event.type == SDL_TEXTINPUT) {
		// If the string less than maximum size and does not contain invalid characters \ / : * ? " < > |
		if (text.length() < size && Event.text.text[0] != '\\' && Event.text.text[0] != '/' && Event.text.text[0] != ':' && Event.text.text[0] != '*' && Event.text.text[0] != '?' && Event.text.text[0] != '\"' && Event.text.text[0] != '<' && Event.text.text[0] != '>' && Event.text.text[0] != '|') {
			// Should we only accept numbers?
			if (numbers_only && !isdigit(Event.text.text[0]))
				return false;

			// Now play the text input sound
			pyrodactyl::music::gMusicManager.PlayEffect(se_entry, 0);

			// Append the character to string
			text += Event.text.text;
		}
	} else if (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE && text.length() != 0) {
		// Now play the text erase sound
		pyrodactyl::music::gMusicManager.PlayEffect(se_erase, 0);

		// If backspace was pressed and the string isn't blank, remove a character from the end
		text.erase(text.length() - 1);
	} else if (gInput.Equals(IU_ACCEPT, Event) == SDL_RELEASED && text.length() != 0) {
		// Now play the accept sound
		pyrodactyl::music::gMusicManager.PlayEffect(se_accept, 0);

		return true;
	}

	return false;
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void TextArea::Draw() {
	title.Draw();
	TextData::Draw(text + "_");
}