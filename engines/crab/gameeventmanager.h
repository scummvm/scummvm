#pragma once

#include "ChapterIntro.h"
#include "EventSeqGroup.h"
#include "PersonHandler.h"
#include "PersonScreen.h"
#include "ReplyMenu.h"
#include "common_header.h"
#include "hud.h"
#include "journal.h"
#include "level.h"
#include "textarea.h"

namespace pyrodactyl {
namespace event {
class Manager {
protected:
	// All the events in the game
	std::unordered_map<std::string, EventSeqGroup> event_map;

	// The currently happening or active sequence
	unsigned int active_seq;

	// THIS IS NOT THE DEFINITIVE LIST OF ENDED SEQUENCES
	// JUST A TEMPORARY LIST OF EVENT SEQUENCES TO PASS AROUND
	std::vector<EventSeqInfo> end_seq;

	// The objects used to draw the dialog box and opinion bars
	pyrodactyl::ui::PersonHandler oh;

	// The reply menu and the colors and font of the text
	pyrodactyl::ui::ReplyMenu reply;

	// The field for text input
	pyrodactyl::ui::TextArea textin;

	// The info for intro events
	pyrodactyl::ui::ChapterIntro intro;

	// Store the current event data temporarily
	GameEvent *cur_event;
	bool player;
	pyrodactyl::anim::Sprite *cur_sp;

	void UpdateDialogBox(Info &info, pyrodactyl::level::Level &level);

public:
	// The object used to draw the character screen
	pyrodactyl::ui::PersonScreen per;

	// A flag used to stop drawing the game for a better fade in/fade out experience
	bool draw_game;

	Manager() { Init(); }
	~Manager() {}

	void Init();
	void Load(rapidxml::xml_node<char> *node, pyrodactyl::ui::ParagraphData &popup);

	void Draw(Info &info, pyrodactyl::ui::HUD &hud, pyrodactyl::level::Level &level);

	// cur_per is also updated here
	void InternalEvents(Info &info, pyrodactyl::level::Level &level, std::vector<EventResult> &result);

	void HandleEvents(Info &info, const std::string &player_id, SDL_Event &Event,
					  pyrodactyl::ui::HUD &hud, pyrodactyl::level::Level &level, std::vector<EventResult> &result);

	void CalcActiveSeq(Info &info, pyrodactyl::level::Level &level, const Rect &camera);

	void EndSequence(const std::string &curloc);
	bool EventInProgress();

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace event
} // End of namespace pyrodactyl