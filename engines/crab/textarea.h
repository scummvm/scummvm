//=============================================================================
// Author:   Arvind
// Purpose:  A box for entering text
//=============================================================================
#pragma once

#include "HoverInfo.h"
#include "MusicManager.h"
#include "TextManager.h"
#include "common_header.h"
#include "input.h"

namespace pyrodactyl {
namespace ui {
class TextArea : public TextData {
	// The maximum number of characters allowed
	unsigned int size;

	// The name of the text area is stored here
	HoverInfo title;

	// Sound effects
	pyrodactyl::music::ChunkKey se_entry, se_erase, se_accept;

public:
	std::string text;

	TextArea(void) : text("") {
		size = 20;
		se_entry = -1;
		se_erase = -1;
		se_accept = -1;
	}
	~TextArea(void) {}

	void Load(rapidxml::xml_node<char> *node);
	bool HandleEvents(const SDL_Event &Event, bool numbers_only = false);
	void Draw();

	void SetUI() {
		title.SetUI();
		TextData::SetUI();
	}
};
} // End of namespace ui
} // End of namespace pyrodactyl