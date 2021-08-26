#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

void HypnoEngine::showConversation() {
	uint32 x = 18;
	uint32 y = 20;
	Graphics::Surface *speaker = decodeFrame("dialog/speaker3.smk", 0);
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active) {
			uint32 frame;
			Common::String path;
			for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
				if (it->command == "F") {
					frame = it->num;
				} else if (it->command == "G") {
					path = it->path;
				}
			}
			if (!path.empty()) {
				frame = frame;
				debug("decoding %s frame %d", path.c_str(), frame);
				Graphics::Surface *surf = decodeFrame("dialog/" + path, frame);

				drawImage(*speaker, x, y, false);
				drawImage(*surf, x + speaker->w, y, false);
				a->rect = Common::Rect(x + speaker->w, y, x + surf->w, y + surf->h);
				y = y + surf->h;
				//delete surf;
				//free(surf);
			}
		}
	}
}

bool HypnoEngine::leftClickedConversation(Common::Point mousePos) {
	Talk *t;
	bool activeFound = false;
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && a->rect.contains(mousePos)) {
			activeFound = true;
			a->active = false;
			for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
				if (it->command == "A") {
					debug("Adding %d", it->num);
					t = (Talk *)_conversation[it->num];
					t->active = true;
					_refreshConversation = true;
				} else if (it->command == "D") {
					debug("Disabling %d", it->num);
					t = (Talk *)_conversation[it->num];
					t->active = false;
					_refreshConversation = true;
				} else if (it->command == "P") {
					debug("Playing %s", it->path.c_str());
					_nextSequentialVideoToPlay.push_back(MVideo(it->path, it->position, false, false, false));
				}
			}
		}
		if (!a->background.empty()) {
			loadImage(a->background, a->position.x, a->position.y, false);
		}
	}
	if (!activeFound) {
		_conversation.clear();
		runMenu(*stack.back());
		drawScreen();
	}
}

bool HypnoEngine::rightClickedConversation(Common::Point mousePos) {
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && a->rect.contains(mousePos)) {
			for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
				if (it->command == "I") {
					debug("Playing %s", it->path.c_str());
					// Not sure why position is 50, 50 since there is only one pixel
					_nextSequentialVideoToPlay.push_back(MVideo(it->path, Common::Point(0, 0), false, false, false));
				}
			}
		}
	}
}

} // End of namespace Hypno