/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

void SpiderEngine::showConversation() {
	debugC(1, kHypnoDebugScene, "Showing conversation");
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
	speaker->free();
	delete speaker;
}

void SpiderEngine::leftClickedConversation(const Common::Point &mousePos) {
	Talk *t;
	bool activeFound = false;
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && a->rect.contains(mousePos)) {
			activeFound = true;
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

void SpiderEngine::rightClickedConversation(const Common::Point &mousePos) {
	for (Actions::const_iterator itt = _conversation.begin(); itt != _conversation.end(); ++itt) {
		Talk *a = (Talk *)*itt;
		if (a->active && a->rect.contains(mousePos)) {
			for (TalkCommands::const_iterator it = a->commands.begin(); it != a->commands.end(); ++it) {
				if (it->command == "I") {
					debugC(1, kHypnoDebugScene, "Playing %s", it->path.c_str());
					// Not sure why position is 50, 50 since there is only one pixel
					_nextSequentialVideoToPlay.push_back(MVideo(it->path, Common::Point(0, 0), false, false, false));
				}
			}
		}
	}
}

} // End of namespace Hypno
