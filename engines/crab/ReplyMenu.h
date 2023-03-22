#pragma once

#include "GameEventInfo.h"
#include "ImageData.h"
#include "PersonHandler.h"
#include "ReplyButton.h"
#include "common_header.h"
#include "conversationdata.h"
#include "emotion.h"
#include "eventstore.h"
#include "menu.h"

namespace pyrodactyl {
namespace ui {
class ReplyMenu : public Menu<ReplyButton> {
	// Data about the background image
	ImageData bg;

	// The minimum spacing between two reply choices
	int spacing;

	// The emotion indicator used to indicate the type of reply selected
	EmotionIndicator tone;

public:
	ReplyMenu() { spacing = 20; }
	~ReplyMenu() {}

	void Load(const std::string &filename);
	int HandleEvents(pyrodactyl::event::Info &info, pyrodactyl::event::ConversationData &dat,
					 const std::string &cur_id, PersonHandler &oh, const SDL_Event &Event);

	void Draw();
	void Cache(pyrodactyl::event::Info &info, pyrodactyl::event::ConversationData &dat);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl