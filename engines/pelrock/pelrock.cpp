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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "image/pcx.h"
#include "image/png.h"

#include "pelrock.h"
#include "pelrock/console.h"
#include "pelrock/detection.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

PelrockEngine *g_engine;

PelrockEngine::PelrockEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																				 _gameDescription(gameDesc), _randomSource("Pelrock") {
	g_engine = this;
	_chronoManager = new ChronoManager();
	_room = new RoomManager();
	_res = new ResourceManager();
}

PelrockEngine::~PelrockEngine() {
	delete _room;
	delete[] _compositeBuffer;
	delete[] _currentBackground;
	delete _largeFont;
	delete _screen;
	delete _chronoManager;
	delete _videoManager;

	// if (_bgPopupBalloon)
	// 	delete[] _bgPopupBalloon;
	delete _smallFont;
}

uint32 PelrockEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PelrockEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Array<Common::Array<Common::String> > wordWrap(Common::String text);

Common::Error PelrockEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 400);
	_screen = new Graphics::Screen();
	_videoManager = new VideoManager(_screen);

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, 60);

	init();
	if (shouldPlayIntro == false) {
		stateGame = GAME;
	} else {
		stateGame = INTRO;
		_videoManager->playIntro();
	}

	while (!shouldQuit()) {
		_chronoManager->updateChrono();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				switch (e.kbd.keycode) {
				case Common::KEYCODE_w:
					alfredState = ALFRED_WALKING;
					break;
				case Common::KEYCODE_t:
					alfredState = ALFRED_TALKING;
					break;
				case Common::KEYCODE_s:
					alfredState = ALFRED_IDLE;
					break;
				case Common::KEYCODE_c:
					alfredState = ALFRED_COMB;
					break;
				case Common::KEYCODE_i:
					alfredState = ALFRED_INTERACTING;
					break;
				default:
					break;
				}
			} else if (e.type == Common::EVENT_MOUSEMOVE) {
				mouseX = e.mouse.x;
				mouseY = e.mouse.y;
				// debug(3, "Mouse moved to (%d,%d)", mouseX, mouseY);
			} else if (e.type == Common::EVENT_LBUTTONDOWN) {
				if (!_isMouseDown) {
					_mouseClickTime = g_system->getMillis();
					_isMouseDown = true;
				}
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				_isMouseDown = false;
				checkMouseClick(e.mouse.x, e.mouse.y);
				_displayPopup = false;
				_longClick = false;
			}
		}
		if (_isMouseDown) {
			if (g_system->getMillis() - _mouseClickTime >= kLongClickDuration) {
				debug("long click!");
				_longClick = true;
				_isMouseDown = false;
				checkLongMouseClick(e.mouse.x, e.mouse.y);
			}
		}
		checkMouseHover();
		frames();

		_screen->update();
		// limiter.delayBeforeSwap();
		// limiter.startFrame();
	}

	return Common::kNoError;
}

void PelrockEngine::init() {
	_res->loadCursors();
	_res->loadInteractionIcons();

	_compositeBuffer = new byte[640 * 400];
	_currentBackground = new byte[640 * 400];

	_smallFont = new SmallFont();
	_smallFont->load("ALFRED.4");
	_largeFont = new LargeFont();
	_largeFont->load("ALFRED.7");

	changeCursor(DEFAULT);
	CursorMan.showMouse(true);

	if (gameInitialized == false) {
		gameInitialized = true;
		loadAnims();
		// setScreen(5, 0); //museum entrance
		// setScreen(13, 1); // restaurants kitchen
		setScreen(2, 2); // hooker
	}
}

void PelrockEngine::loadAnims() {
	_res->loadAlfredAnims();
}

void PelrockEngine::talk(byte object) {
	debug("Talking to object %d", object);
	if (_room->_currentRoomConversations.size() == 0)
		return;

	AnimSet *animSet;
	for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
		if (_room->_currentRoomAnims[i].extra == object) {
			animSet = &_room->_currentRoomAnims[i];
		}
	}

	ConversationNode selectedNode = _room->_currentRoomConversations[0];

	bool isNPC = selectedNode.speakerId != 13;
	if (isNPC) {
		sayNPC(animSet, selectedNode.text, selectedNode.speakerId);
	}
	// for(int i= 0; i< _currentRoomConversations.size(); i++) {
	// _currentRoomConversations
	// }

	// showDescription(_currentRoomConversations[0].text, x, y, _currentRoomConversations[0].speakerId);
	// for(int i = 0; i < _currentRoomConversations[0].choices.size(); i++) {
	// 	int idx = _currentRoomConversations.size() - 1 - i;
	// 	_smallFont->drawString(_screen, _currentRoomConversations[0].choices[idx].text.c_str(), 0, 400 - ((i + 1) * 12), 640, 14);
	// }
}

byte *PelrockEngine::grabBackgroundSlice(int x, int y, int w, int h) {
	byte *bg = new byte[w * h];
	for (int j = 0; j < w; j++) {
		for (int i = 0; i < h; i++) {
			int idx = i * w + j;
			if (y + i < 400 && x + j < 640) {
				*(bg + idx) = _currentBackground[(y + i) * 640 + (x + j)];
			}
		}
	}
	return bg;
}

void PelrockEngine::putBackgroundSlice(int x, int y, int w, int h, byte *slice) {
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int index = (j * w + i);
			if (x + i < 640 && y + j < 400)
				*(byte *)g_engine->_screen->getBasePtr(x + i, y + j) = slice[index];
		}
	}
}

Common::Array<VerbIcons> PelrockEngine::availableActions(HotSpot *hotspot) {
	Common::Array<VerbIcons> verbs;
	verbs.push_back(LOOK);

	if (hotspot->type & 1) {
		debug("Hotspot allows OPEN action");
		verbs.push_back(OPEN);
	}
	if (hotspot->type & 2) {
		debug("Hotspot allows CLOSE action");
		verbs.push_back(CLOSE);
	}
	if (hotspot->type & 4) {
		debug("Hotspot allows UNKNOWN action");
		verbs.push_back(UNKNOWN);
	}
	if (hotspot->type & 8) {
		debug("Hotspot allows PICKUP action");
		verbs.push_back(PICKUP);
	}
	if (hotspot->type & 16) {
		debug("Hotspot allows TALK action");
		verbs.push_back(TALK);
	}
	if (hotspot->type & 32) {
		debug("Hotspot allows WALK action");
		verbs.push_back(PUSH);
	}
	if (hotspot->type & 128) {
		debug("Hotspot allows PULL action");
		verbs.push_back(PULL);
	}
	return verbs;
}

void PelrockEngine::frames() {

	if (_chronoManager->_gameTick) {

		memcpy(_compositeBuffer, _currentBackground, 640 * 400);

		// debug("Game tick!");
		for (int i = 0; i < _room->_currentRoomAnims.size(); i++) {
			// debug("Processing animation set %d, numAnims %d", num, i->numAnims);
			AnimSet &animSet = _room->_currentRoomAnims[i];
			drawNextFrame(&animSet);
		}

		switch (alfredState) {
		case ALFRED_WALKING: {
			MovementStep step = _currentContext.movement_buffer[_current_step];

			if (step.distance_x > 0) {
				if (step.flags & MOVE_RIGHT) {
					dirAlfred = 0;
					xAlfred += MIN((uint16_t)6, step.distance_x);
				}
				if (step.flags & MOVE_LEFT) {
					dirAlfred = 1;
					xAlfred -= MIN((uint16_t)6, step.distance_x);
				}
			}
			if (step.distance_y > 0) {
				if (step.flags & MOVE_DOWN) {
					dirAlfred = 2;
					yAlfred += MIN((uint16_t)6, step.distance_y);
				}
				if (step.flags & MOVE_UP) {
					dirAlfred = 3;
					yAlfred -= MIN((uint16_t)6, step.distance_y);
				}
			}

			if (step.distance_x > 0)
				step.distance_x -= MIN((uint16_t)6, step.distance_x);

			if (step.distance_y > 0)
				step.distance_y -= MIN((uint16_t)6, step.distance_y);

			if (step.distance_x <= 0 && step.distance_y <= 0) {
				_current_step++;
				if (_current_step >= _currentContext.movement_count) {
					_current_step = 0;
					alfredState = ALFRED_IDLE;
				}
			} else {
				_currentContext.movement_buffer[_current_step] = step;
			}

			Exit *exit = isExitUnder(xAlfred, yAlfred);

			if (exit != nullptr) {
				xAlfred = exit->targetX;
				yAlfred = exit->targetY;
				setScreen(exit->targetRoom, exit->dir);
			}

			if (curAlfredFrame >= walkingAnimLengths[dirAlfred]) {
				curAlfredFrame = 0;
			}

			drawAlfred(_res->alfredWalkFrames[dirAlfred][curAlfredFrame]);
			curAlfredFrame++;
			break;
		}
		case ALFRED_TALKING:
			if (curAlfredFrame >= talkingAnimLengths[dirAlfred] - 1) {
				curAlfredFrame = 0;
			}
			drawAlfred(_res->alfredTalkFrames[dirAlfred][curAlfredFrame]);
			curAlfredFrame++;
			break;
		case ALFRED_COMB:
			if (curAlfredFrame >= 11) {
				curAlfredFrame = 0;
			}
			drawSpriteToBuffer(_compositeBuffer, 640, _res->alfredCombFrames[0][curAlfredFrame], xAlfred, yAlfred - kAlfredFrameHeight, 51, 102, 255);
			curAlfredFrame++;
			break;
		case ALFRED_INTERACTING:
			if (curAlfredFrame >= interactingAnimLength) {
				curAlfredFrame = 0;
			}
			drawAlfred(_res->alfredInteractFrames[dirAlfred][curAlfredFrame]);
			curAlfredFrame++;
			break;
		default:
			drawAlfred(_res->alfredIdle[dirAlfred]);
			break;
		}
		if (_displayPopup) {

			// byte *bgDialog = new byte[kBalloonWidth * kBalloonHeight];
			// for (int j = 0; j < kBalloonWidth; j++) {
			// 	for (int i = 0; i < kBalloonHeight; i++) {
			// 		int idx = i * kBalloonWidth + j;
			// 		if (_popupY + i < 400 && _popupX + j < 640) {
			// 			*(bgDialog + idx) = _currentBackground[(_popupY + i) * 640 + (_popupX + j)];
			// 		}
			// 	}
			// }
			// if (_bgPopupBalloon != nullptr) {
			// 	putBackgroundSlice(_popupX, _popupY, kBalloonWidth, kBalloonHeight, _bgPopupBalloon);
			// }
			showActionBalloon(_popupX, _popupY, _currentPopupFrame);
			if (_currentPopupFrame < 3) {
				_currentPopupFrame++;
			} else
				_currentPopupFrame = 0;
		}

		memcpy(_screen->getPixels(), _compositeBuffer, 640 * 400);

		if (alfredState != ALFRED_WALKING && !_currentTextPages.empty()) {
			if (_chronoManager->_textTtl > 0) {
				renderText(_currentTextPages[_currentTextPageIndex], _textColor, _textPos.x, _textPos.y);
			} else if (_currentTextPageIndex < _currentTextPages.size() - 1) {
				_currentTextPageIndex++;

				int totalChars = 0;
				for (int i = 0; i < _currentTextPages[_currentTextPageIndex].size(); i++) {
					totalChars += _currentTextPages[_currentTextPageIndex][i].size();
				}
				_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
			} else {
				_currentTextPages.clear();
				_currentTextPageIndex = 0;
				alfredState = ALFRED_IDLE;
				isNPCATalking = false;
				isNPCBTalking = false;
			}
		}

		// debug("Drawing walkboxes..., %d, _currentRoomWalkboxes.size()=%d",  _currentRoomWalkboxes.size(), _currentRoomWalkboxes.size());
		for (int i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
			// debug("Drawing walkbox %d", i);
			WalkBox box = _room->_currentRoomWalkboxes[i];
			drawRect(_screen, box.x, box.y, box.w, box.h, 150 + i);
		}
		if (_curWalkTarget.x < 640 && _curWalkTarget.y < 400 && _curWalkTarget.x >= 0 && _curWalkTarget.y >= 0) {
			_screen->setPixel(_curWalkTarget.x, _curWalkTarget.y, 100);
			if (_curWalkTarget.x - 1 > 0 && _curWalkTarget.y - 1 > 0)
				_screen->setPixel(_curWalkTarget.x - 1, _curWalkTarget.y - 1, 100);
			if (_curWalkTarget.x - 1 > 0 && _curWalkTarget.y + 1 < 400)
				_screen->setPixel(_curWalkTarget.x - 1, _curWalkTarget.y + 1, 100);
			if (_curWalkTarget.x + 1 < 640 && _curWalkTarget.y - 1 > 0)
				_screen->setPixel(_curWalkTarget.x + 1, _curWalkTarget.y - 1, 100);
			if (_curWalkTarget.x + 1 < 640 && _curWalkTarget.y + 1 < 400)
				_screen->setPixel(_curWalkTarget.x + 1, _curWalkTarget.y + 1, 100);
			if (_curWalkTarget.x - 2 > 0)
				_screen->setPixel(_curWalkTarget.x - 2, _curWalkTarget.y, 100);
			if (_curWalkTarget.x + 2 < 640)
				_screen->setPixel(_curWalkTarget.x + 2, _curWalkTarget.y, 100);
			if (_curWalkTarget.y - 2 > 0)
				_screen->setPixel(_curWalkTarget.x, _curWalkTarget.y - 2, 100);
			if (_curWalkTarget.y + 2 < 400)
				_screen->setPixel(_curWalkTarget.x, _curWalkTarget.y + 2, 100);
		}
		_screen->markAllDirty();
		// _screen->update();
	}
}

void PelrockEngine::doAction(byte action, byte object) {
	if (action == TALK) {
		talk(object);
	}
}

void PelrockEngine::renderText(Common::Array<Common::String> lines, int color, int baseX, int baseY) {

	int maxW = 0;
	for (size_t i = 0; i < lines.size(); i++) {
		Common::Rect r = _largeFont->getBoundingBox(lines[i]);
		if (r.width() > maxW) {
			maxW = r.width();
		}
	}
	int lineSize = lines.size();
	for (size_t i = 0; i < lines.size(); i++) {
		int textX = baseX - (maxW / 2);
		int textY = baseY - (lineSize * 25) + (i * 25);
		drawText(lines[i], textX, textY, maxW, color);
	}
}

void PelrockEngine::drawAlfred(byte *buf) {
	drawSpriteToBuffer(_compositeBuffer, 640, buf, xAlfred, yAlfred - kAlfredFrameHeight, kAlfredFrameWidth, kAlfredFrameHeight, 255);
}

void PelrockEngine::drawNextFrame(AnimSet *animSet) {
	Anim &animData = animSet->animData[animSet->curAnimIndex];
	int x = animData.x;
	int y = animData.y;
	int w = animData.w;
	int h = animData.h;
	int extra = animSet->extra;

	if (whichNPCTalking == extra) {
		drawTalkNPC(animSet);
		return;
	}

	int frameSize = animData.w * animData.h;
	int curFrame = animData.curFrame;
	byte *frame = new byte[frameSize];
	extractSingleFrame(animData.animData, frame, curFrame, animData.w, animData.h);

	drawSpriteToBuffer(_compositeBuffer, 640, frame, animSet->x, animSet->y, animSet->w, animSet->h, 255);

	if (animData.elpapsedFrames == animData.speed) {
		animData.elpapsedFrames = 0;
		if (animData.curFrame < animData.nframes - 1) {
			animData.curFrame++;
		} else {
			if (animData.curLoop < animData.loopCount - 1) {
				animData.curFrame = 0;
				animData.curLoop++;
			} else {
				animData.curFrame = 0;
				animData.curLoop = 0;
				if (animSet->curAnimIndex < animSet->numAnims - 1) {
					animSet->curAnimIndex++;
				} else {
					animSet->curAnimIndex = 0;
				}
			}
		}
	} else {
		animData.elpapsedFrames++;
	}
}

void PelrockEngine::checkLongMouseClick(int x, int y) {
	int hotspotIndex = isHotspotUnder(mouseX, mouseY);
	if (hotspotIndex != -1) {

		_popupX = x - kBalloonWidth / 2;
		if (_popupX < 0)
			_popupX = 0;
		if (_popupX + kBalloonWidth > 640) {
			_popupX -= 640 - (_popupX + kBalloonWidth);
		}

		_popupY = y - kBalloonHeight;
		if (_popupY < 0) {
			_popupY = 0;
		}
		_displayPopup = true;
		_currentPopupFrame = 0;
		_currentHotspot = &_room->_currentRoomHotspots[hotspotIndex];
		debug("Current hotspot type: %d", _currentHotspot->type);
	}
}

int PelrockEngine::isHotspotUnder(int x, int y) {

	for (size_t i = 0; i < _room->_currentRoomHotspots.size(); i++) {
		HotSpot hotspot = _room->_currentRoomHotspots[i];
		if (hotspot.isEnabled &&
			mouseX >= hotspot.x && mouseX <= (hotspot.x + hotspot.w) &&
			mouseY >= hotspot.y && mouseY <= (hotspot.y + hotspot.h)) {
			return i;
		}
	}
	return -1;
}

Exit *PelrockEngine::isExitUnder(int x, int y) {
	for (int i = 0; i < _room->_currentRoomExits.size(); i++) {
		Exit exit = _room->_currentRoomExits[i];
		if (x >= exit.x && x <= (exit.x + exit.w) &&
			y >= exit.y && y <= (exit.y + exit.h)) {
			return &(_room->_currentRoomExits[i]);
		}
	}
	return nullptr;
}

void PelrockEngine::showActionBalloon(int posx, int posy, int curFrame) {

	drawSpriteToBuffer(_compositeBuffer, 640, _res->_popUpBalloon + (curFrame * kBalloonHeight * kBalloonWidth), posx, posy, kBalloonWidth, kBalloonHeight, 255);
	Common::Array<VerbIcons> actions = availableActions(_currentHotspot);

	drawSpriteToBuffer(_compositeBuffer, 640, _res->_verbIcons[LOOK], posx + 20, posy + 20, kVerbIconWidth, kVerbIconHeight, 1);
	// Graphics::Surface rects;
	// rects.create(kVerbIconWidth, kVerbIconHeight, Graphics::PixelFormat::createFormatCLUT8());
	// drawRect(&rects, 0, 0, kVerbIconWidth, kVerbIconHeight, 255);

	// blitSurfaceToBuffer(&rects, _compositeBuffer, 640, 480, posx + ver, posy + 20);

	for (int i = 0; i < actions.size(); i++) {
		drawSpriteToBuffer(_compositeBuffer, 640, _res->_verbIcons[actions[i]], posx + 20 + (i * (kVerbIconWidth + 2)), posy + 20, kVerbIconWidth, kVerbIconHeight, 1);
	}
}

void PelrockEngine::drawTalkNPC(AnimSet *animSet) {
	// Change with the right index

	int index = animSet->index;
	TalkingAnimHeader *animHeader = &_room->_talkingAnimHeader;

	int x = animSet->x + (index ? animHeader->offsetXAnimB : animHeader->offsetXAnimA);
	int y = animSet->y + (index ? animHeader->offsetYAnimB : animHeader->offsetYAnimA);

	int w = index ? animHeader->wAnimB : animHeader->wAnimA;
	int h = index ? animHeader->hAnimB : animHeader->hAnimA;
	int numFrames = index ? animHeader->numFramesAnimB : animHeader->numFramesAnimA;
	int curFrame = index ? animHeader->currentFrameAnimB++ : animHeader->currentFrameAnimA++;
	if (curFrame >= numFrames) {
		if (index) {
			animHeader->currentFrameAnimB = 0;
		} else {
			animHeader->currentFrameAnimA = 0;
		}
		curFrame = 0;
	}
	byte *frame = index ? animHeader->animB[curFrame] : animHeader->animA[curFrame];
	debug("Talking NPC frame %d/%d, x=%d, y=%d, w=%d, h=%d", curFrame, numFrames, x, y, w, h);

	drawSpriteToBuffer(_compositeBuffer, 640, frame, x, y, w, h, 255);
}

void PelrockEngine::walkTo(int x, int y) {
	alfredState = ALFRED_WALKING;
	curAlfredFrame = 0;

	PathContext context = {NULL, NULL, NULL, 0, 0, 0};

	pathFind(x, y, &context);
	// debug("\nPath Information:\n");
	// debug("================\n");

	// debug("Walkbox path (%d boxes): ", context.path_length);
	for (int i = 0; i < context.path_length && context.path_buffer[i] != PATH_END; i++) {
		debug("%d ", context.path_buffer[i]);
	}

	// debug("Movement steps (%d steps):\n", context.movement_count);
	for (int i = 0; i < context.movement_count; i++) {
		MovementStep *step = &context.movement_buffer[i];
		// debug("  Step %d: ", i);

		// if (step->flags & MOVE_RIGHT)
		// 	debug("RIGHT ");
		// if (step->flags & MOVE_LEFT)
		// 	debug("LEFT ");
		// if (step->flags & MOVE_DOWN)
		// 	debug("DOWN ");
		// if (step->flags & MOVE_UP)
		// 	debug("UP ");

		// debug("(dx=%d, dy=%d)\n", step->distance_x, step->distance_y);
	}

	// debug("\nCompressed path (%d bytes): ", context.compressed_length);
	for (int i = 0; i < context.compressed_length; i++) {
		debug("%02X ", context.compressed_path[i]);
	}

	// if (x > xAlfred) {
	// 	dirAlfred = RIGHT;
	// } else if (x < xAlfred) {
	// 	dirAlfred = LEFT;
	// } else if (y < yAlfred) {
	// 	dirAlfred = UP;
	// } else if (y > yAlfred) {
	// 	dirAlfred = DOWN;
	// }
	// debug("Setting Alfred to walk towards (%d, %d) from (%d, %d) in direction %d", x, y, xAlfred, yAlfred, dirAlfred);
	_currentContext = context;
	// debug("Path find complete, movement count: %d", _currentContext.movement_count);
}

bool PelrockEngine::pathFind(int x, int y, PathContext *context) {

	if (context->path_buffer == NULL) {
		context->path_buffer = (uint8_t *)malloc(MAX_PATH_LENGTH);
	}
	if (context->movement_buffer == NULL) {
		context->movement_buffer = (MovementStep *)malloc(MAX_MOVEMENT_STEPS * sizeof(MovementStep));
	}
	// if (context->compressed_path == NULL) {
	//     context->compressed_path = (uint8_t*)malloc(MAX_COMPRESSED_PATH);
	// }

	int startX = xAlfred;
	int startY = yAlfred;
	Common::Point target = calculateWalkTarget(x, y);
	x = target.x;
	y = target.y;
	debug("Startx= %d, starty= %d, destx= %d, desty= %d", startX, startY, x, y);

	uint8_t start_box = find_walkbox_for_point(startX, startY);
	uint8_t dest_box = find_walkbox_for_point(x, y);

	debug("Pathfinding from (%d, %d) in box %d to (%d, %d) in box %d\n",
		  startX, startY, start_box,
		  x, y, dest_box);
	// Check if both points are in valid walkboxes
	if (start_box == 0xFF || dest_box == 0xFF) {
		debug("Error: Start or destination not in any walkbox\n");
		return false;
	}
	// Special case: same walkbox
	if (start_box == dest_box) {
		// Generate direct movement
		MovementStep direct_step;
		direct_step.flags = 0;
		if (startX < x) {
			direct_step.distance_x = x - startX;
			direct_step.flags |= MOVE_RIGHT;
		} else {
			direct_step.distance_x = startX - x;
			direct_step.flags |= MOVE_LEFT;
		}

		if (startY < y) {
			direct_step.distance_y = y - startY;
			direct_step.flags |= MOVE_DOWN;
		} else {
			direct_step.distance_y = startY - y;
			direct_step.flags |= MOVE_UP;
		}

		context->movement_buffer[0] = direct_step;
		context->movement_count = 1;
	} else {
		// Build walkbox path
		context->path_length = build_walkbox_path(start_box, dest_box,
												  context->path_buffer);

		if (context->path_length == 0) {
			debug("Error: No path found\n");
			return false;
		}

		// Generate movement steps
		context->movement_count = generate_movement_steps(
			context->path_buffer,
			context->path_length,
			startX, startY,
			x, y,
			context->movement_buffer);
	}
	return true;
}

/**
 * Calculate movement needed to reach a target within a walkbox
 */
void calculate_movement_to_target(uint16_t current_x, uint16_t current_y,
								  uint16_t target_x, uint16_t target_y,
								  WalkBox *box,
								  MovementStep *step) {
	step->flags = 0;
	step->distance_x = 0;
	step->distance_y = 0;

	// Calculate horizontal movement
	if (current_x < box->x) {
		// Need to move right to enter walkbox
		step->distance_x = box->x - current_x;
		step->flags |= MOVE_RIGHT;
	} else if (current_x > box->x + box->w) {
		// Need to move left to enter walkbox
		step->distance_x = current_x - (box->x + box->w);
		step->flags |= MOVE_LEFT;
	}

	// Calculate vertical movement
	if (current_y < box->y) {
		// Need to move down to enter walkbox
		step->distance_y = box->y - current_y;
		step->flags |= MOVE_DOWN;
	} else if (current_y > box->y + box->h) {
		// Need to move up to enter walkbox
		step->distance_y = current_y - (box->y + box->h);
		step->flags |= MOVE_UP;
	}
}

/**
 * Generate movement steps from walkbox path
 * Returns: number of movement steps generated
 */
uint16_t PelrockEngine::generate_movement_steps(uint8_t *path_buffer,
												uint16_t path_length,
												uint16_t start_x, uint16_t start_y,
												uint16_t dest_x, uint16_t dest_y,
												MovementStep *movement_buffer) {
	uint16_t current_x = start_x;
	uint16_t current_y = start_y;
	uint16_t movement_index = 0;

	// Generate movements for each walkbox in path
	for (uint16_t i = 0; i < path_length && path_buffer[i] != PATH_END; i++) {
		uint8_t box_index = path_buffer[i];
		WalkBox *box = &_room->_currentRoomWalkboxes[box_index];

		MovementStep step;
		calculate_movement_to_target(current_x, current_y,
									 dest_x, dest_y,
									 box, &step);

		if (step.distance_x > 0 || step.distance_y > 0) {
			movement_buffer[movement_index++] = step;

			// Update current position
			if (step.flags & MOVE_RIGHT) {
				current_x = box->x;
			} else if (step.flags & MOVE_LEFT) {
				current_x = box->x + box->w;
			}

			if (step.flags & MOVE_DOWN) {
				current_y = box->y;
			} else if (step.flags & MOVE_UP) {
				current_y = box->y + box->h;
			}
		}
	}

	// Final movement to exact destination
	MovementStep final_step;
	final_step.flags = 0;

	if (current_x < dest_x) {
		final_step.distance_x = dest_x - current_x;
		final_step.flags |= MOVE_RIGHT;
	} else if (current_x > dest_x) {
		final_step.distance_x = current_x - dest_x;
		final_step.flags |= MOVE_LEFT;
	} else {
		final_step.distance_x = 0;
	}

	if (current_y < dest_y) {
		final_step.distance_y = dest_y - current_y;
		final_step.flags |= MOVE_DOWN;
	} else if (current_y > dest_y) {
		final_step.distance_y = current_y - dest_y;
		final_step.flags |= MOVE_UP;
	} else {
		final_step.distance_y = 0;
	}

	if (final_step.distance_x > 0 || final_step.distance_y > 0) {
		movement_buffer[movement_index++] = final_step;
	}

	return movement_index;
}

uint16_t PelrockEngine::build_walkbox_path(
	uint8_t start_box,
	uint8_t dest_box,
	uint8_t *path_buffer) {
	uint16_t path_index = 0;
	uint8_t current_box = start_box;

	// Initialize path with start walkbox
	path_buffer[path_index++] = start_box;

	// Clear visited flags
	clear_visited_flags();

	// Breadth-first search through walkboxes
	while (current_box != dest_box && path_index < MAX_PATH_LENGTH - 1) {
		uint8_t next_box = get_adjacent_walkbox(current_box);

		if (next_box == 0xFF) {
			// Dead end - backtrack
			if (path_index > 1) {
				path_index--;
				current_box = path_buffer[path_index - 1];
			} else {
				// No path exists
				return 0;
			}
		} else if (next_box == dest_box) {
			// Found destination
			path_buffer[path_index++] = dest_box;
			break;
		} else {
			// Continue searching
			path_buffer[path_index++] = next_box;
			current_box = next_box;
		}
	}

	// Terminate path
	path_buffer[path_index] = PATH_END;

	return path_index;
}

void PelrockEngine::clear_visited_flags() {
	for (int i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
		_room->_currentRoomWalkboxes[i].flags = 0;
	}
}

/**
 * Check if two walkboxes overlap or touch (are adjacent)
 */
bool walkboxes_adjacent(WalkBox *box1, WalkBox *box2) {
	uint16_t box1_x_max = box1->x + box1->w;
	uint16_t box1_y_max = box1->y + box1->h;
	uint16_t box2_x_max = box2->x + box2->w;
	uint16_t box2_y_max = box2->y + box2->h;

	// Check if X ranges overlap
	bool x_overlap = (box1->x <= box2_x_max) && (box2->x <= box1_x_max);

	// Check if Y ranges overlap
	bool y_overlap = (box1->y <= box2_y_max) && (box2->y <= box1_y_max);

	return x_overlap && y_overlap;
}

uint8_t PelrockEngine::get_adjacent_walkbox(uint8_t current_box_index) {
	WalkBox *current_box = &_room->_currentRoomWalkboxes[current_box_index];

	// Mark current walkbox as visited
	current_box->flags = 0x01;

	// Search for adjacent unvisited walkbox
	for (uint8_t i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
		// Skip current walkbox
		if (i == current_box_index) {
			continue;
		}

		// Skip already visited walkboxes
		if (_room->_currentRoomWalkboxes[i].flags == 0x01) {
			continue;
		}

		// Check if walkboxes are adjacent
		if (walkboxes_adjacent(current_box, &_room->_currentRoomWalkboxes[i])) {
			return i;
		}
	}

	return 0xFF; // No adjacent walkbox found
}

bool PelrockEngine::point_in_walkbox(WalkBox *box, uint16_t x, uint16_t y) {
	return (x >= box->x &&
			x <= box->x + box->w &&
			y >= box->y &&
			y <= box->y + box->h);
}

uint8_t PelrockEngine::find_walkbox_for_point(uint16_t x, uint16_t y) {
	for (uint8_t i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {
		if (point_in_walkbox(&_room->_currentRoomWalkboxes[i], x, y)) {
			return i;
		}
	}
	return 0xFF; // Not found
}

void PelrockEngine::checkMouseClick(int x, int y) {

	if (whichNPCTalking)
		whichNPCTalking = false;

	if (_displayPopup) {
		Common::Array<VerbIcons> actions = availableActions(_currentHotspot);
		for (int i = 0; i < actions.size(); i++) {
			debug("Available action %d at index %d", actions[i], i);
		}
		Common::Rect lookRect = Common::Rect(_popupX + 20, _popupY + 20, _popupX + 20 + kVerbIconWidth, _popupY + 20 + kVerbIconHeight);
		// debug("Look rect: x=%d, y=%d, w=%d, h=%d", lookRect.left, lookRect.top, lookRect, lookRect.h);
		if (lookRect.contains(x, y)) {
			debug("Look action clicked");
			walkTo(_currentHotspot->x, _currentHotspot->y);
			sayAlfred(_room->_currentRoomDescriptions[_currentHotspot->index].text);
			_displayPopup = false;
			return;
		}
		for (int i = 1; i < actions.size(); i++) {

			// debug("Checking action %d at index %d for mouse click = %d, %d", actions[i], i, x, y);
			int x = _popupX + 20 + (i * (kVerbIconWidth + 2));
			int y = _popupY + 20;
			Common::Rect actionRect = Common::Rect(x, y, x + kVerbIconWidth, y + kVerbIconHeight);

			if (actionRect.contains(x, y)) {
				debug("Action %d clicked", actions[i]);
				doAction(actions[i], _currentHotspot->extra);
				_displayPopup = false;
				return;
			}
		}
	}

	_displayPopup = false;
	_currentHotspot = nullptr;

	Common::Point walkTarget = calculateWalkTarget(mouseX, mouseY);
	_curWalkTarget = walkTarget;

	{ // For quick room navigation
		Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);

		if (exit != nullptr) {
			xAlfred = exit->targetX;
			yAlfred = exit->targetY;

			debug("Placing character at %d, %d", exit->targetX, exit->targetY);
			setScreen(exit->targetRoom, exit->dir);
		} else {
			walkTo(walkTarget.x, walkTarget.y);
		}
	}
}

void PelrockEngine::changeCursor(Cursor cursor) {
	CursorMan.replaceCursor(_res->_cursorMasks[cursor], kCursorWidth, kCursorHeight, 0, 0, 255);
}

void PelrockEngine::checkMouseHover() {
	bool isSomethingUnder = false;

	// Calculate walk target first (before checking anything else)
	Common::Point walkTarget = calculateWalkTarget(mouseX, mouseY);

	// Check if walk target hits any exit
	bool exitDetected = false;
	Exit *exit = isExitUnder(walkTarget.x, walkTarget.y);
	if (exit != nullptr) {
		exitDetected = true;
	}

	int hotspotIndex = isHotspotUnder(mouseX, mouseY);

	if (hotspotIndex != -1) {
		isSomethingUnder = true;
	}

	if (isSomethingUnder && exitDetected) {
		changeCursor(COMBINATION);
	} else if (isSomethingUnder) {
		changeCursor(HOTSPOT);
	} else if (exitDetected) {
		changeCursor(EXIT);
	} else {
		changeCursor(DEFAULT);
	}
}

Common::Point PelrockEngine::calculateWalkTarget(int mouseX, int mouseY) {
	// Starting point for pathfinding
	int sourceX = mouseX;
	int sourceY = mouseY;

	// TODO: If hovering over a sprite/hotspot, adjust source point to sprite center
	// For now, just use mouse position

	// Find nearest walkable point in walkboxes
	uint32 minDistance = 0xFFFFFFFF;
	Common::Point bestTarget(sourceX, sourceY);

	// for (Common::List<WalkBox>::iterator it = _currentRoomWalkboxes.begin();
	//  it != _currentRoomWalkboxes.end(); ++it) {
	for (size_t i = 0; i < _room->_currentRoomWalkboxes.size(); i++) {

		// Calculate distance from source point to this walkbox (Manhattan distance)
		int dx = 0;
		int dy = 0;

		// Calculate horizontal distance
		if (sourceX < _room->_currentRoomWalkboxes[i].x) {
			dx = _room->_currentRoomWalkboxes[i].x - sourceX;
		} else if (sourceX > _room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w) {
			dx = sourceX - (_room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w);
		}
		// else: sourceX is inside walkbox horizontally, dx = 0

		// Calculate vertical distance
		if (sourceY < _room->_currentRoomWalkboxes[i].y) {
			dy = _room->_currentRoomWalkboxes[i].y - sourceY;
		} else if (sourceY > _room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h) {
			dy = sourceY - (_room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h);
		}
		// else: sourceY is inside walkbox vertically, dy = 0

		uint32 distance = dx + dy;

		if (distance < minDistance) {
			minDistance = distance;

			// Calculate target point (nearest point on walkbox to source)
			int targetX = sourceX;
			int targetY = sourceY;

			if (sourceX < _room->_currentRoomWalkboxes[i].x) {
				targetX = _room->_currentRoomWalkboxes[i].x;
			} else if (sourceX > _room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w) {
				targetX = _room->_currentRoomWalkboxes[i].x + _room->_currentRoomWalkboxes[i].w;
			}

			if (sourceY < _room->_currentRoomWalkboxes[i].y) {
				targetY = _room->_currentRoomWalkboxes[i].y;
			} else if (sourceY > _room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h) {
				targetY = _room->_currentRoomWalkboxes[i].y + _room->_currentRoomWalkboxes[i].h;
			}

			bestTarget.x = targetX;
			bestTarget.y = targetY;
		}
	}

	return bestTarget;
}

void PelrockEngine::drawText(Common::String text, int x, int y, int w, byte color) {
	Common::Rect rect = _largeFont->getBoundingBox(text.c_str());
	if (x + rect.width() > 640) {
		x = 640 - rect.width() - 2;
	}
	if (y + rect.height() > 400) {
		y = 400 - rect.height();
	}
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	// Draw main text on top
	_largeFont->drawString(_screen, text.c_str(), x, y, w, color, Graphics::kTextAlignCenter);
}

void PelrockEngine::sayNPC(AnimSet *anim, Common::String text, byte color) {
	isNPCATalking = true;
	whichNPCTalking = anim->extra;
	debug("NPC says %s, color = %d", text.c_str(), color);
	_currentTextPages = wordWrap(text);
	_textColor = color;
	int totalChars = 0;
	for (int i = 0; i < _currentTextPages[0].size(); i++) {
		totalChars += _currentTextPages[0][i].size();
	}
	_textPos = Common::Point(anim->x, anim->y - anim->h - 10);
	_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
}

void PelrockEngine::sayAlfred(Common::String text) {
	alfredState = ALFRED_TALKING;
	curAlfredFrame = 0;
	debug("Alfred says: %s", text.c_str());
	_currentTextPages = wordWrap(text);
	_textColor = 13;
	int totalChars = 0;
	for (int i = 0; i < _currentTextPages[0].size(); i++) {
		totalChars += _currentTextPages[0][i].size();
	}
	_textPos = Common::Point(xAlfred, yAlfred - kAlfredFrameHeight - 10);
	_chronoManager->_textTtl = totalChars * kTextCharDisplayTime;
}

bool isEndMarker(char char_byte) {
	return char_byte == CHAR_END_MARKER_1 || char_byte == CHAR_END_MARKER_2 || char_byte == CHAR_END_MARKER_3 || char_byte == CHAR_END_MARKER_4;
}

int calculateWordLength(Common::String text, int startPos, bool &isEnd) {
	// return word_length, is_end
	int wordLength = 0;
	int pos = startPos;
	while (pos < text.size()) {
		char char_byte = text[pos];
		if (char_byte == CHAR_SPACE || isEndMarker(char_byte)) {
			break;
		}
		wordLength++;
		pos++;
	}
	// Check if we hit an end marker
	if (pos < text.size() && isEndMarker(text[pos])) {
		isEnd = true;
	}
	// Count ALL trailing spaces as part of this word
	if (pos < text.size() && !isEnd) {
		if (text[pos] == CHAR_END_MARKER_3) { // 0xF8 (-8) special case
			wordLength += 3;
		} else {
			// Count all consecutive spaces
			while (pos < text.size() && text[pos] == CHAR_SPACE) {
				wordLength++;
				pos++;
			}
		}
	}
	return wordLength;
}

Common::Array<Common::Array<Common::String> > wordWrap(Common::String text) {

	Common::Array<Common::Array<Common::String> > pages;
	Common::Array<Common::String> currentPage;
	Common::Array<Common::String> currentLine;
	int charsRemaining = MAX_CHARS_PER_LINE;
	int position = 0;
	int currentLineNum = 0;
	while (position < text.size()) {
		bool isEnd = false;
		int wordLength = calculateWordLength(text, position, isEnd);
		// # Extract the word (including trailing spaces)
		// word = text[position:position + word_length].decode('latin-1', errors='replace')
		Common::String word = text.substr(position, wordLength).decode(Common::kLatin1);
		// # Key decision: if word_length > chars_remaining, wrap to next line
		if (wordLength > charsRemaining) {
			// Word is longer than the entire line - need to split
			currentPage.push_back(joinStrings(currentLine, ""));
			currentLine.clear();
			charsRemaining = MAX_CHARS_PER_LINE;
			currentLineNum++;

			if (currentLineNum >= MAX_LINES) {
				pages.push_back(currentPage);
				currentPage.clear();
				currentLineNum = 0;
			}
		}
		// Add word to current line
		currentLine.push_back(word);
		charsRemaining -= wordLength;

		if (charsRemaining == 0 && isEnd) {
			Common::String lineText = joinStrings(currentLine, "");
			while (lineText.lastChar() == CHAR_SPACE) {
				lineText = lineText.substr(0, lineText.size() - 1);
			}
			int trailingSpaces = currentLine.size() - lineText.size();
			if (trailingSpaces > 0) {
				currentPage.push_back(lineText);
				//  current_line = [' ' * trailing_spaces]
				Common::String currentLine(trailingSpaces, ' ');
				charsRemaining = MAX_CHARS_PER_LINE - trailingSpaces;
				currentLineNum += 1;

				if (currentLineNum >= MAX_LINES) {
					pages.push_back(currentPage);
					currentPage.clear();
					currentLineNum = 0;
				}
			}
		}

		position += wordLength;
		if (isEnd) {
			// End of sentence/paragraph/page
			break;
		}
	}
	if (currentLine.empty() == false) {
		Common::String lineText = joinStrings(currentLine, "");
		while (lineText.lastChar() == CHAR_SPACE) {
			lineText = lineText.substr(0, lineText.size() - 1);
		}
		currentPage.push_back(lineText);
	}
	if (currentPage.empty() == false) {
		pages.push_back(currentPage);
	}
	debug("Word wrap produced %d pages", pages.size());
	for (int i = 0; i < pages.size(); i++) {
		debug(" Page %d:", i);
		for (int j = 0; j < pages[i].size(); j++) {
			debug("   Line %d: %s", j, pages[i][j].c_str());
		}
	}
	return pages;
}

void PelrockEngine::setScreen(int number, int dir) {

	Common::File roomFile;
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		error("Could not open ALFRED.1");
		return;
	}
	dirAlfred = dir;
	alfredState = ALFRED_IDLE;
	_current_step = 0;
	int roomOffset = number * kRoomStructSize;
	curAlfredFrame = 0;
	byte *palette = new byte[256 * 3];
	_room->getPalette(&roomFile, roomOffset, palette);

	int paletteOffset = roomOffset + (11 * 8);
	roomFile.seek(paletteOffset, SEEK_SET);
	uint32 offset = roomFile.readUint32LE();

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	byte *background = new byte[640 * 400];
	_room->getBackground(&roomFile, roomOffset, background);
	if (_currentBackground != nullptr)
		delete[] _currentBackground;
	_currentBackground = new byte[640 * 400];
	Common::copy(background, background + 640 * 400, _currentBackground);
	for (int i = 0; i < 640; i++) {
		for (int j = 0; j < 400; j++) {
			_screen->setPixel(i, j, background[j * 640 + i]);
		}
	}

	_room->loadRoomMetadata(&roomFile, roomOffset);
	_room->loadRoomTalkingAnimations(number);

	_screen->markAllDirty();
	roomFile.close();
	delete[] background;
	delete[] palette;
}

Common::Error PelrockEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Pelrock
