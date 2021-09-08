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

#include <typeinfo>
#include "common/events.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

extern int parse_mis(const char *);

const static char *sceneVariables[] = {
	"GS_NONE",
	"GS_SCTEXT",
	"GS_AMBIENT",
	"GS_MUSIC",
	"GS_VOLUME",
	"GS_MOUSESPEED",
	"GS_MOUSEON",
	"GS_LEVELCOMPLETE",
	"GS_LEVELWON",
	"GS_CHEATS",
	"GS_SWITCH0",
	"GS_SWITCH1",
	"GS_SWITCH2",
	"GS_SWITCH3",
	"GS_SWITCH4",
	"GS_SWITCH5",
	"GS_SWITCH6",
	"GS_SWITCH7",
	"GS_SWITCH8",
	"GS_SWITCH9",
	"GS_SWITCH10",
	"GS_SWITCH11",
	"GS_SWITCH12",
	"GS_COMBATJSON",
	"GS_COMBATLEVEL",
	"GS_PUZZLELEVEL",
	NULL};

void HypnoEngine::parseScene(Common::String prefix, Common::String filename) {
	debugC(1, kHypnoDebugParser, "Parsing %s", filename.c_str());
	filename = convertPath(filename);
	if (!prefix.empty())
		filename = prefix + "/" + filename;
	Common::File test;
	assert(test.open(filename.c_str()));

	const uint32 fileSize = test.size();
	char *buf = (char *)malloc(fileSize + 1);
	test.read(buf, fileSize);
	test.close();
	buf[fileSize] = '\0';
	parse_mis(buf);
	Level level;
	level.scene.prefix = prefix;
	level.scene.hots = *g_parsedHots;
	_levels[filename] = level;
	free(buf);
}

void HypnoEngine::resetSceneState() {
	uint32 i = 0;
	while (sceneVariables[i]) {
		_sceneState[sceneVariables[i]] = 0;
		i++;
	}
}

bool HypnoEngine::checkSceneCompleted() {
	return _sceneState["GS_LEVELCOMPLETE"];
}

// Hotspots

void HypnoEngine::clickedHotspot(Common::Point mousePos) {
	Hotspots *hots = stack.back();
	Hotspot selected;
	bool found = false;
	int rs = 100000000;
	int cs = 0;
	for (Hotspots::const_iterator it = hots->begin(); it != hots->end(); ++it) {
		const Hotspot h = *it;
		if (h.type != MakeHotspot)
			continue;

		cs = h.rect.width() * h.rect.height();
		if (h.rect.contains(mousePos)) {
			if (cs < rs) {
				selected = h;
				found = true;
				rs = cs;
			}
		}
	}
	if (found) {
		if (selected.smenu) {
			assert(!selected.smenu->empty());
			_nextHotsToAdd = selected.smenu;
		}

		for (Actions::const_iterator itt = selected.actions.begin(); itt != selected.actions.end(); ++itt) {
			Action *action = *itt;
			if (typeid(*action) == typeid(ChangeLevel))
				runChangeLevel((ChangeLevel *)action);
			if (typeid(*action) == typeid(Escape))
				runEscape((Escape *)action);
			else if (typeid(*action) == typeid(Cutscene))
				runCutscene((Cutscene *)action);
			else if (typeid(*action) == typeid(Play))
				runPlay((Play *)action);
			else if (typeid(*action) == typeid(WalN))
				runWalN((WalN *)action);
			else if (typeid(*action) == typeid(Global))
				runGlobal((Global *)action);
			else if (typeid(*action) == typeid(Talk))
				runTalk((Talk *)action);
			else if (typeid(*action) == typeid(Quit))
				runQuit((Quit *)action);
			else if (typeid(*action) == typeid(Palette))
				debugC(1, kHypnoDebugScene, "runPalette unimplemented");
		}
	}
}

bool HypnoEngine::hoverHotspot(Common::Point mousePos) {
	Hotspots *hots = stack.back();
	Hotspot selected;
	bool found = false;
	int rs = 100000000;
	int cs = 0;
	for (Hotspots::const_iterator it = hots->begin(); it != hots->end(); ++it) {
		const Hotspot h = *it;
		if (h.type != MakeHotspot)
			continue;

		cs = h.rect.width() * h.rect.height();
		if (h.rect.contains(mousePos)) {
			if (cs < rs) {
				selected = h;
				found = true;
				rs = cs;
			}
		}
	}
	if (found) {
		for (Actions::const_iterator itt = selected.actions.begin(); itt != selected.actions.end(); ++itt) {
			Action *action = *itt;
			if (typeid(*action) == typeid(Mice))
				runMice((Mice *)action);
		}
		return true;
	}
	return false;
}

void HypnoEngine::runTransition(Transition trans) {
	for (Filenames::iterator it = trans.intros.begin(); it != trans.intros.end(); ++it) {
		MVideo v(*it, Common::Point(0, 0), false, true, false);
		runIntro(v);
	}

	if (!trans.frameImage.empty()) {
		debugC(1, kHypnoDebugScene, "Rendering %s frame in transaction", trans.frameImage.c_str());
		Graphics::Surface *frame = decodeFrame(trans.frameImage, trans.frameNumber);
		Graphics::Surface *sframe = frame->scale(_screenW, _screenH);
		drawImage(*sframe, 0, 0, false);
		drawScreen();
		frame->free();
		delete frame;
		sframe->free();
		delete sframe;
		assert(installTimer(2 * 1000000, &trans.level));
	} else
		_nextLevel = trans.level;
}


void HypnoEngine::runScene(Scene scene) {
	_refreshConversation = false;
	_conversation.clear();
	Common::Event event;
	Common::Point mousePos;
	Common::List<uint32> videosToRemove;

	stack.clear();
	_nextHotsToAdd = &scene.hots;
	defaultCursor();

	while (!shouldQuit() && _nextLevel.empty()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {
						if (it->decoder)
							skipVideo(*it);
					}
					_videosPlaying.clear();

					if (!stack.empty()) {
						runMenu(*stack.back());
						drawScreen();
					}
				}

				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_RBUTTONDOWN:
				if (stack.empty())
					break;
				if (!_conversation.empty()) {
					rightClickedConversation(mousePos);
					break;
				}
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (stack.empty())
					break;
				if (!_conversation.empty()) {
					leftClickedConversation(mousePos);
					break;
				}
				if (!_nextHotsToAdd && !_nextHotsToRemove /*&& _videosPlaying.empty()*/)
					clickedHotspot(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				// Reset cursor to default
				//changeCursor("default");
				// The following functions will return true
				// if the cursor is changed
				if (stack.empty() || !_conversation.empty() || !_videosPlaying.empty())
					break;

				if (!hoverHotspot(mousePos))
					defaultCursor();
				break;

			default:
				break;
			}
		}

		if (_refreshConversation && !_conversation.empty() && _nextSequentialVideoToPlay.empty()) {
			showConversation();
			drawScreen();
			_refreshConversation = false;
			_videosPlaying.clear();
		}

		// Movies
		if (!_nextSequentialVideoToPlay.empty() && _videosPlaying.empty()) {
			playVideo(*_nextSequentialVideoToPlay.begin());
			_videosPlaying.push_back(*_nextSequentialVideoToPlay.begin());
			_nextSequentialVideoToPlay.remove_at(0);
		}
		uint32 i = 0;
		videosToRemove.clear();
		for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {

			if (it->decoder) {
				if (it->decoder->endOfVideo()) {
					if (it->loop) {
						it->decoder->rewind();
						it->decoder->start();
					} else {
						it->decoder->close();
						delete it->decoder;
						it->decoder = nullptr;
						videosToRemove.push_back(i);
					}

				} else if (it->decoder->needsUpdate()) {
					updateScreen(*it);
				}
			}
			i++;
		}
		if (!videosToRemove.empty()) {

			for (Common::List<uint32>::iterator it = videosToRemove.begin(); it != videosToRemove.end(); ++it) {
				debugC(1, kHypnoDebugScene, "removing %d from %d size", *it, _videosPlaying.size());
				_videosPlaying.remove_at(*it);
			}

			// Nothing else to play
			if (_videosPlaying.empty() && _nextSequentialVideoToPlay.empty()) {
				if (!_conversation.empty())
					_refreshConversation = true;
				else if (!stack.empty()) {
					runMenu(*stack.back());
					drawScreen();
				}
			}
		}

		if (!_videosPlaying.empty() || !_nextSequentialVideoToPlay.empty()) {
			drawScreen();
			continue;
		}

		if (_nextHotsToRemove) {
			debugC(1, kHypnoDebugScene, "Removing a hotspot list!");
			stack.pop_back();
			runMenu(*stack.back());
			_nextHotsToRemove = NULL;
			drawScreen();
		} else if (_nextHotsToAdd) {
			debugC(1, kHypnoDebugScene, "Adding a hotspot list!");
			stack.push_back(_nextHotsToAdd);
			runMenu(*stack.back());
			_nextHotsToAdd = NULL;
			drawScreen();
		}

		if (_music.empty() && !scene.sound.empty()) {
			_music = scene.sound;
			playSound(_music, 1);
		}

		if (checkSceneCompleted())
			_nextLevel = scene.levelIfWin;

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	// Deallocate videos
	for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {
		if (it->decoder)
			skipVideo(*it);
	}
}

void HypnoEngine::showConversation() { error("Not implemented"); }
void HypnoEngine::rightClickedConversation(Common::Point mousePos) { error("Not implemented"); }
void HypnoEngine::leftClickedConversation(Common::Point mousePos) { error("Not implemented"); }

} // End of namespace Hypno

