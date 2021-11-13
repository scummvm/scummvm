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
	nullptr
};

void HypnoEngine::loadSceneLevel(const Common::String &current, const Common::String &next, const Common::String &prefix) {
	debugC(1, kHypnoDebugParser, "Parsing %s", current.c_str());
	Common::String name = convertPath(current);
	// if (!prefix.empty())
	// 	name = prefix + "/" + name;

	Common::File test;
	if (!test.open(name.c_str()))
		error("Failed to open %s", name.c_str());

	const uint32 fileSize = test.size();
	char *buf = (char *)malloc(fileSize + 1);
	test.read(buf, fileSize);
	test.close();
	buf[fileSize] = '\0';
	debugC(1, kHypnoDebugParser, "%s", buf);
	parse_mis(buf);
	Scene *level = new Scene();
	level->prefix = prefix;
	level->levelIfWin = next;
	level->hots = *g_parsedHots;
	_levels[name] = level;
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
	Hotspot selected(MakeHotspot, "");
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
	if (!found)
		return;

	if (selected.smenu) {
		if (selected.smenu->empty())
			error("Invalid menu selected");
		_nextHotsToAdd = selected.smenu;
	}

	_videosPlaying.clear();
	_nextParallelVideoToPlay.clear();
	_nextSequentialVideoToPlay.clear();

	bool cont = true;
	for (Actions::const_iterator itt = selected.actions.begin(); itt != selected.actions.end() && cont; ++itt) {
		Action *action = *itt;
		switch (action->type) {
			case ChangeLevelAction:
				runChangeLevel((ChangeLevel *)action);
			break;

			case EscapeAction:
				runEscape();
			break;

			case CutsceneAction:
				runCutscene((Cutscene *)action);
			break;

			case PlayAction:
				runPlay((Play *)action);
			break;

			case WalNAction:
				runWalN((WalN *)action);
			break;
		
			case GlobalAction:
				cont = runGlobal((Global *)action);
			break;

			case TalkAction:
				runTalk((Talk *)action);
			break;

			case QuitAction:
				runQuit((Quit *)action);
			break;
			case AmbientAction: 
				runAmbient((Ambient *)action);
			break;
			case PaletteAction:
				debugC(1, kHypnoDebugScene, "runPalette unimplemented");
			break;

			default:
			break;
		}
	}
}

bool HypnoEngine::hoverHotspot(Common::Point mousePos) {
	Hotspots *hots = stack.back();
	Hotspot selected(MakeHotspot, "");
	bool found = false;
	int rs = 100000000;
	for (Hotspots::const_iterator it = hots->begin(); it != hots->end(); ++it) {
		const Hotspot h = *it;
		if (h.type != MakeHotspot)
			continue;

		int cs = h.rect.width() * h.rect.height();
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
			switch (action->type) {
				case MiceAction:
					runMice((Mice *)action);
				break;
				default:
				break;
			}
		}
		return true;
	}
	return false;
}

void HypnoEngine::runTransition(Transition *trans) {
	if (!trans->frameImage.empty()) {
		debugC(1, kHypnoDebugScene, "Rendering %s frame in transaction", trans->frameImage.c_str());
		Graphics::Surface *frame = decodeFrame(trans->frameImage, trans->frameNumber);
		Graphics::Surface *sframe = frame->scale(_screenW, _screenH);
		drawImage(*sframe, 0, 0, false);
		drawScreen();
		frame->free();
		delete frame;
		sframe->free();
		delete sframe;
		Common::String *ptr = new Common::String(trans->level);
		if (!installTimer(2 * 1000000, ptr)) // 2 seconds
			error("Failed to install timer");
	} else
		_nextLevel = trans->level;
}


void HypnoEngine::runScene(Scene *scene) {
	_nextLoopingVideoToPlay.clear();
	_nextParallelVideoToPlay.clear();
	_nextSequentialVideoToPlay.clear();
	_intros.clear();

	_refreshConversation = false;
	_conversation.clear();
	Common::Event event;
	Common::Point mousePos;
	Common::List<uint32> videosToRemove;
	bool enableLoopingVideos = true;

	stack.clear();
	_nextHotsToAdd = &scene->hots;
	defaultCursor();

	while (!shouldQuit() && _nextLevel.empty()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {
						if (it->decoder) {
							skipVideo(*it);
							if (it->scaled) {
								runMenu(*stack.back());
								drawScreen();
							}
						}

					}
					_videosPlaying.clear();

					if (!_conversation.empty())
						_refreshConversation = true;
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
				if (!_nextHotsToAdd && !_nextHotsToRemove /*&& _videosPlaying.empty()*/) {
					clickedHotspot(mousePos);
					drawScreen();
				}
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

		if (_refreshConversation && !_conversation.empty() && 
			_nextSequentialVideoToPlay.empty() && 
			_nextParallelVideoToPlay.empty() &&
			_videosPlaying.empty()) {
			showConversation();
			drawScreen();
			_refreshConversation = false;
		}

		// Movies
		if (!_nextParallelVideoToPlay.empty()) {
			for (Videos::iterator it = _nextParallelVideoToPlay.begin(); it != _nextParallelVideoToPlay.end(); ++it) {
				playVideo(*it);
				_videosPlaying.push_back(*it);
			}
			_nextParallelVideoToPlay.clear();
		}

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
					if (it->loop && enableLoopingVideos) {
						it->decoder->rewind();
						it->decoder->start();
					} else {
						it->decoder->close();
						delete it->decoder;
						it->decoder = nullptr;
						videosToRemove.push_back(i);
						if (it->scaled) {
							runMenu(*stack.back());
							drawScreen();
						}

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
			debug("Something to play: %d", _videosPlaying.size());
			// Nothing else to play
			if (_videosPlaying.empty() && _nextSequentialVideoToPlay.empty() && !checkSceneCompleted()) {
				if (!_conversation.empty())
					_refreshConversation = true;
			}
		}

		if (checkSceneCompleted()) {
			// Make sure all the videos are played before we finish
			enableLoopingVideos = false;
			if (_conversation.empty() && 
				_videosPlaying.empty() && 
				_nextSequentialVideoToPlay.empty() && 
				_nextParallelVideoToPlay.empty()) {
				debugC(1, kHypnoDebugScene, "Wining level and jumping to %s", scene->levelIfWin.c_str());
				if (_nextLevel.empty()) {
					assert(!scene->levelIfWin.empty());
					_nextLevel = scene->levelIfWin;
				}
				continue;
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
			_nextHotsToRemove = nullptr;
			drawScreen();
		} else if (_nextHotsToAdd) {
			debugC(1, kHypnoDebugScene, "Adding a hotspot list!");
			stack.push_back(_nextHotsToAdd);
			runMenu(*stack.back());
			_nextHotsToAdd = nullptr;
			drawScreen();
		}

		if (_music.empty() && !scene->music.empty()) {
			_music = scene->music;
			playSound(_music, 0);
		}

		g_system->updateScreen();
		g_system->delayMillis(30);
	}

	// Deallocate videos
	for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {
		if (it->decoder)
			skipVideo(*it);
	}
}

void HypnoEngine::showConversation() { error("Not implemented"); }
void HypnoEngine::rightClickedConversation(const Common::Point &mousePos) { error("Not implemented"); }
void HypnoEngine::leftClickedConversation(const Common::Point &mousePos) { error("Not implemented"); }

} // End of namespace Hypno

