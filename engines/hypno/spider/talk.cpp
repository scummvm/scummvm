/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

void SpiderEngine::endConversation() {
	debugC(1, kHypnoDebugScene, "Ending and clearing conversation");
	for (Actions::iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		delete a;
	}
	_conversation.clear();
}

void SpiderEngine::showConversation() {
	debugC(1, kHypnoDebugScene, "Showing conversation");
	defaultCursor();
	uint32 x = 0;
	uint32 y = 0;
	Graphics::Surface *speaker = decodeFrame("dialog/speaker3.smk", 0);
	bool activeFound = false;
	bool skipRepeated = false;

	// First iteration on the talk commands
	Videos videos;
	for (Actions::iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;

		for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
			if (it->command == "P") {
				if (_intros[it->path]) {
					skipRepeated = true;
				}
			}
		}

		if (a->boxPos != Common::Point(0, 0)) {
			if (!(x == 0 && x == y))
				error("Multiple BOX positions found");

			x = a->boxPos.x;
			y = a->boxPos.y;
		}
		if (!a->intro.empty() && !_intros.contains(a->intro)) {
			videos.push_back(MVideo(a->intro, a->introPos, false, false, false));
			_intros[a->intro] = true;
		}

	}

	if (videos.size() > 0) {
		runIntros(videos);
		videos.clear();
	}

	if (x == 0 && x == y)
		error("BOX position not found");

	// Second iteration on the talk commands
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && !skipRepeated) {
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
				activeFound = true;
				Graphics::Surface *surf = decodeFrame("dialog/" + path, frame);

				drawImage(*speaker, x, y, false);
				drawImage(*surf, x + speaker->w, y, false);
				a->rect = Common::Rect(x + speaker->w, y, x + surf->w, y + surf->h);
				y = y + surf->h;

				surf->free();
				delete surf;
			}
		}
	}
	if (!activeFound) {
		debugC(1, kHypnoDebugScene, "No active item was found in the current conversation");
		// Final iteration on the talk commands
		bool shouldEscape = false;
		for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
			Talk *a = (Talk *)*itt;

			// Avoid this conversation next time
			for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
				if (it->command == "P") {
					if (!it->path.empty()) {
						_intros[it->path] = true;
					}
				}
			}

			if (!a->second.empty()) {
				debugC(1, kHypnoDebugScene, "Adding %s to play after the conversation ends", a->second.c_str());
				videos.push_back(MVideo(a->second, a->secondPos, false, false, false));
			}
			if (a->escape) {
				shouldEscape = true;
			}
		}

		if (videos.size() > 0) {
			runIntros(videos);
			videos.clear();
		}

		endConversation();
		_music.clear();

		if (shouldEscape) {
			runIntros(_escapeSequentialVideoToPlay);
			_escapeSequentialVideoToPlay.clear();

			// HACK
			Hotspots *hots = stack.back();
			if (hots->size() == 2) {
				debugC(1, kHypnoDebugScene, "Level should end here, since there is nothing else to do");
				_sceneState["GS_LEVELCOMPLETE"] = true;
				_sceneState["GS_LEVELWON"] = true;
			}

		}

		drawScreen();
	}
	speaker->free();
	delete speaker;
}

void SpiderEngine::leftClickedConversation(const Common::Point &mousePos) {
	defaultCursor();
	Talk *t;
	Videos videos;
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && a->rect.contains(mousePos)) {
			a->active = false;
			for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
				if (it->command == "A") {
					debugC(1, kHypnoDebugScene, "Adding option %d", it->num);
					t = (Talk *)_conversation[it->num];
					t->active = true;
					_refreshConversation = true;
				} else if (it->command == "D") {
					debugC(1, kHypnoDebugScene, "Disabling option %d", it->num);
					t = (Talk *)_conversation[it->num];
					t->active = false;
					_refreshConversation = true;
				} else if (it->command == "P") {
					debugC(1, kHypnoDebugScene, "Playing %s", it->path.c_str());
					videos.push_back(MVideo(it->path, it->position, false, false, false));
					_refreshConversation = true;
				} else if (it->command == "S") {
					debugC(1, kHypnoDebugScene, "Enabling variable %s", it->variable.c_str());
					_sceneState[it->variable] = 1;
					_refreshConversation = true;
				} else if (it->command == "L") {
					_sceneState["GS_LEVELCOMPLETE"] = true;
					_refreshConversation = true;
				}

			}
		}
		if (!a->background.empty()) {
			loadImage(a->background, a->backgroundPos.x, a->backgroundPos.y, false);
		}
	}

	if (_sceneState["GS_LEVELCOMPLETE"]) {
		debugC(1, kHypnoDebugScene, "Level is complete, clearing variables");
		resetSceneState();
		_sceneState["GS_LEVELCOMPLETE"] = true;
		_sceneState["GS_LEVELWON"] = true;
	}

	if (videos.size() > 0)
		runIntros(videos);
}

void SpiderEngine::rightClickedConversation(const Common::Point &mousePos) {
	Videos videos;
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && a->rect.contains(mousePos)) {
			for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
				if (it->command == "I") {
					debugC(1, kHypnoDebugScene, "Playing %s", it->path.c_str());
					// Not sure why position is 50, 50 since there is only one pixel
					videos.push_back(MVideo(it->path, Common::Point(0, 0), false, false, false));
				}
			}
		}
	}
	if (videos.size() > 0)
		runIntros(videos);
}

bool SpiderEngine::hoverConversation(const Common::Point &mousePos) {
	Mice mice(_defaultCursor, 1);

	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && a->rect.contains(mousePos)) {
			runMice(&mice);
			return true;
		}
	}
	return false;
}

} // End of namespace Hypno
