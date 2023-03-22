#include "LoadingScreen.h"
#include "XMLDoc.h"
#include "stdafx.h"

void LoadingScreen::Load() {
	const std::string &filename = "res/layout/loading.xml";
	XMLDoc doc(filename);
	if (doc.ready()) {
		rapidxml::xml_node<char> *node = doc.Doc()->first_node("loading");
		if (NodeValid(node)) {
			if (NodeValid("screens", node)) {
				rapidxml::xml_node<char> *scrnode = node->first_node("screens");
				for (auto n = scrnode->first_node("screen"); n != NULL; n = n->next_sibling("screen"))
					screen.push_back(n);
			}

			if (NodeValid("text", node))
				text.Load(node->first_node("text"), "img");
		}
	}
}

void LoadingScreen::Draw() {
	// Change to a random screen
	Change();

	// Draw the current screen
	if (cur < screen.size())
		screen.at(cur).Draw();

	// Draw the loading text
	text.Draw((gScreenSettings.cur.w - text.W()) / 2, (gScreenSettings.cur.h - text.H()) / 2);

	// Update the screen
	SDL_RenderPresent(gRenderer);
}

void LoadingScreen::Dim() {
	// This is used when starting or loading a game from the main menu in order to dim the screen
	// until an actual loading screen is drawn
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
	SDL_RenderFillRect(gRenderer, NULL);

	// Update the screen
	SDL_RenderPresent(gRenderer);
}

void LoadingScreen::Quit() {
	text.Delete();

	for (auto i = screen.begin(); i != screen.end(); ++i)
		i->Delete();
}

LoadingScreen gLoadScreen;