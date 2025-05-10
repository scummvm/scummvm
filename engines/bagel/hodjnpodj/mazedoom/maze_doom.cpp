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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/mazedoom/maze_doom.h"
#include "bagel/hodjnpodj/mazedoom/defines.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/rules.h"
#include "bagel/hodjnpodj/views/message_box.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

MazeDoom::MazeDoom() : MinigameView("MazeDoom", "mazedoom/hnpmaze.dll"),
		_scrollButton("Scroll", this, Common::Rect(
			SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
			SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
			SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1)
		),
		_playerSprite(this),
		_timeRect(RectWH(TIME_LOCATION_X + 50, TIME_LOCATION_Y,
			TIME_WIDTH - 50, TIME_HEIGHT)) {
	addResource(SCROLLUP_BMP, SCROLLUP);
	addResource(SCROLLDOWN_BMP, SCROLLDOWN);
	addResource(IDB_LOCALE_BMP, Common::WinResourceID("idb_locale_bmp"));
	addResource(IDB_BLANK_BMP, Common::WinResourceID("idb_blank_bmp"));
	addResource(IDB_PARTS_BMP, IDB_PARTS);
	addResource(IDB_HODJ_UP_BMP, IDB_HODJ_UP);
	addResource(IDB_HODJ_DOWN_BMP, IDB_HODJ_DOWN);
	addResource(IDB_HODJ_LEFT_BMP, IDB_HODJ_LEFT);
	addResource(IDB_HODJ_RIGHT_BMP, IDB_HODJ_RIGHT);
	addResource(IDB_PODJ_UP_BMP, IDB_PODJ_UP);
	addResource(IDB_PODJ_DOWN_BMP, IDB_PODJ_DOWN);
	addResource(IDB_PODJ_LEFT_BMP, IDB_PODJ_LEFT);
	addResource(IDB_PODJ_RIGHT_BMP, IDB_PODJ_RIGHT);
}

bool MazeDoom::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	setupHodjPodj();
	loadBitmaps();
	loadIniSettings();
	initializeMaze();	// Set the surrounding wall and start/end squares 
	createMaze();		// Create a maze layout given the initialized maze
	setupMaze();
	paintMaze();		// Paint that sucker to the offscreen bitmap

	if (pGameParams->bMusicEnabled) {
		_gameSound = new CBofSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (_gameSound != nullptr) {
			(*_gameSound).midiLoopPlaySegment(3000, 32980, 0, FMT_MILLISEC);
		}
	}

	_playing = pGameParams->bplayingMetagame;
	_gameOver = false;
	_move.clear();

	return true;
}

bool MazeDoom::msgClose(const CloseMessage &msg) {
	MinigameView::msgClose(msg);

	_background.clear();
	_scrollButton.clear();
	_mazeBitmap.clear();
	_partsBitmap.clear();
	_blankBitmap.clear();
	_localeBitmap.clear();
	_playerSprite.clear();

	_wallBitmap.clear();
	_pathBitmap.clear();
	_startBitmap.clear();
	_leftEdgeBmp.clear();
	_rightEdgeBmp.clear();
	_topEdgeBmp.clear();
	_bottomEdgeBmp.clear();

	for (int i = 0; i < NUM_TRAP_MAPS; ++i)
		_trapBitmap[i].clear();

	delete _gameSound;
	_gameSound = nullptr;

	return true;
}

bool MazeDoom::msgFocus(const FocusMessage &msg) {
	_priorTime = g_system->getMillis();
	return MinigameView::msgFocus(msg);
}

bool MazeDoom::msgGame(const GameMessage &msg) {
	MinigameView::msgGame(msg);

	if (msg._name == "BUTTON") {
		// Show the main menu
		showMainMenu();
		return true;
	} else if (msg._name == "NEW_GAME") {
		assert(!pGameParams->bplayingMetagame);
		newGame();
		return true;
	} else if (msg._name == "GAME_OVER") {
		CBofSound::waitWaveSounds();
		if (pGameParams->bplayingMetagame) {
			pGameParams->lScore = 0;
			close();
		}

		return true;
	} else if (msg._name == "EXIT") {
		CBofSound::waitWaveSounds();

		if (pGameParams->bplayingMetagame) {
			pGameParams->lScore = 1;	// A victorious maze solving
			close();					// and close minigame
		}

		return true;
	} else if (msg._name == "OPTIONS") {
		_options.setTime(_minutes, _seconds);
		_options.addView();
		return true;
	} else if (msg._name == "TOGGLE_OVERLAYS") {
		_showOverlays = !_showOverlays;
		paintMaze();
		return true;
	}

	return false;
}

bool MazeDoom::msgMouseDown(const MouseDownMessage &msg) {
	if (_move.isWalking() || MinigameView::msgMouseDown(msg))
		return true;

	if (msg._button == MouseDownMessage::MB_LEFT) {
		const RectWH rectTitle(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
			NEWGAME_WIDTH, NEWGAME_HEIGHT);

		if (rectTitle.contains(msg._pos) && !pGameParams->bplayingMetagame)
			// Start new game
			newGame();

		if (_playing && inArtRegion(msg._pos))
			movePlayer(msg._pos);
	}

	return true;
}

bool MazeDoom::msgMouseMove(const MouseMoveMessage &msg) {
	if (_move.isWalking() || MinigameView::msgMouseMove(msg))
		return true;

	if (inArtRegion(msg._pos) && _playing) {
		// If the cursor is within the border
		// and we're playing, update the cursor
		getNewCursor(msg._pos);

		if (msg._button == MouseMoveMessage::MB_LEFT) {
			// If the Left mouse button is down,
			// have the player follow the mouse
			movePlayer(msg._pos);
		}
	} else {
		g_events->setCursor(1);
	}

	return true;
}

bool MazeDoom::msgKeypress(const KeypressMessage &msg) {
	if (_move.isWalking())
		return true;
	Common::Point newPosition = _playerSprite.getPosition();

	switch (msg.keycode) {
	case Common::KEYCODE_F1:
		// Bring up the Rules
		CBofSound::waitWaveSounds();
		Rules::show(RULES_TEXT,
			(pGameParams->bSoundEffectsEnabled ? RULES_WAV : nullptr)
		);
		break;

	case Common::KEYCODE_F2:
		// Options menu
		showMainMenu();
		break;

	case Common::KEYCODE_h:
		newPosition.x -= SQ_SIZE_X;
		movePlayer(newPosition);
		break;

	case Common::KEYCODE_k:
		newPosition.y -= SQ_SIZE_Y;
		movePlayer(newPosition);
		break;

	case Common::KEYCODE_l:
		newPosition.x += SQ_SIZE_X;
		movePlayer(newPosition);
		break;

	case Common::KEYCODE_j:
		newPosition.y += SQ_SIZE_Y;
		movePlayer(newPosition);
		break;

	default:
		return MinigameView::msgKeypress(msg);
	}

	return true;
}

bool MazeDoom::msgAction(const ActionMessage &msg) {
	if (_move.isWalking())
		return true;

	Common::Point newPosition = _playerSprite.getPosition();

	switch (msg._action) {
	case KEYBIND_LEFT:
		newPosition.x -= SQ_SIZE_X;
		movePlayer(newPosition);
		break;

	case KEYBIND_UP:
		newPosition.y -= SQ_SIZE_Y;
		movePlayer(newPosition);
		break;

	case KEYBIND_RIGHT:
		newPosition.x += SQ_SIZE_X;
		movePlayer(newPosition);
		break;

	case KEYBIND_DOWN:
		newPosition.y += SQ_SIZE_Y;
		movePlayer(newPosition);
		break;

	default:
		return MinigameView::msgAction(msg);
	}

	return true;
}

bool MazeDoom::tick() {
	MinigameView::tick();

	uint32 time = g_system->getMillis();
	if (time >= (_priorTime + 1000)) {
		_priorTime = time;

		if (_playing && !_gameOver)
			updateTimer();
	}

	// Handle automatic moving
	if (_move.isWalking()) {
		if (!_move._substepCtr)
			walking();
		else
			walkOneTile();
	}

	return true;
}

void MazeDoom::updateTimer() {
	if (_time == 0) {
		// No time limit, increment 
		_seconds++;
		if (_seconds == 60) {
			_minutes++;
			_seconds = 0;
		}
	} else {
		// Count down time left
		if (_seconds == 0 && _minutes == 0) {
			gameOver();
		} else if (_seconds == 0) {
			_minutes--;
			_seconds = 60;
		}

		_seconds--;
	}

	redraw();
}

void MazeDoom::gameOver() {
	CBofSound *pEffect = nullptr;

	_playing = false;
	_gameOver = true;
	_move.clear();

	if (pGameParams->bSoundEffectsEnabled) {
		pEffect = new CBofSound(this, LOSE_SOUND,
			SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	}

	g_events->clearEvents();

	MessageBox::show("Game over.", "Time ran out!",
		"GAME_OVER");
}

void MazeDoom::draw() {
	GfxSurface s = getSurface();

	// Draw background
	s.blitFrom(_background);

	// Draw the maze. Note that we only draw the bottom
	// half of the top row, and the top half of the bottom row
	s.blitFrom(_mazeBitmap,
		Common::Rect(0, SQ_SIZE_Y / 2, _mazeBitmap.w,
			_mazeBitmap.h - SQ_SIZE_Y / 2),
		Common::Point(SIDE_BORDER, TOP_BORDER));

	// Draw the player sprite
	if (!_playerSprite.empty() && _playing)
		s.blitFrom(_playerSprite, _playerSprite);

	if (_playing) {
		// only false when the options are displayed
		char msg[64];
		s.blitFrom(_blankBitmap, Common::Point(TIME_LOCATION_X, TIME_LOCATION_Y));

		if (_time == 0)
			Common::sprintf_s(msg, "Time Used: %02d:%02d", _minutes, _seconds);
		else {
			Common::sprintf_s(msg, "Time Left: %02d:%02d", _minutes, _seconds);
		}

		s.setFontSize(8);
		s.writeString(msg, _timeRect, BLACK);
	} else if (!_localeBitmap.empty()) {
		s.blitFrom(_localeBitmap,
			Common::Point(TIME_LOCATION_X, TIME_LOCATION_Y));
	}
}

void MazeDoom::setupHodjPodj() {
	if (pGameParams->bPlayingHodj) {
		_upBitmap.loadCels(IDB_HODJ_UP_BMP, NUM_CELS);
		_downBitmap.loadCels(IDB_HODJ_DOWN_BMP, NUM_CELS);
		_leftBitmap.loadCels(IDB_HODJ_LEFT_BMP, NUM_CELS);
		_rightBitmap.loadCels(IDB_HODJ_RIGHT_BMP, NUM_CELS);
	} else {
		_upBitmap.loadCels(IDB_PODJ_UP_BMP, NUM_CELS);
		_downBitmap.loadCels(IDB_PODJ_DOWN_BMP, NUM_CELS);
		_leftBitmap.loadCels(IDB_PODJ_LEFT_BMP, NUM_CELS);
		_rightBitmap.loadCels(IDB_PODJ_RIGHT_BMP, NUM_CELS);
	}

	_upBitmap.setTransparentColor(WHITE);
	_downBitmap.setTransparentColor(WHITE);
	_leftBitmap.setTransparentColor(WHITE);
	_rightBitmap.setTransparentColor(WHITE);
}

void MazeDoom::setupMaze() {
	MazeGen::setupMaze();

	// Set player sprite position
	_playerSprite.x = (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER;
	_playerSprite.y = (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2;
}

void MazeDoom::loadBitmaps() {
	Image::BitmapDecoder decoder;
	Common::File f;

	if (!f.open(MAIN_SCREEN) || !decoder.loadStream(f))
		error("Could not load - %s", MAIN_SCREEN);
	loadPalette(decoder.getPalette());

	_background.copyFrom(*decoder.getSurface());

	_scrollButton.loadBitmaps(SCROLLUP_BMP, SCROLLDOWN_BMP,
		nullptr, nullptr);
	_mazeBitmap.create(NUM_COLUMNS * SQ_SIZE_X,
		NUM_ROWS * SQ_SIZE_Y);

	// Load up the various bitmaps for wall, edge, booby traps, etc.
	_partsBitmap.loadBitmap(IDB_PARTS_BMP);

	_wallBitmap = GfxSurface(_partsBitmap,
		RectWH(WALL_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);
	_pathBitmap = GfxSurface(_partsBitmap,
		RectWH(PATH_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);
	_startBitmap = GfxSurface(_partsBitmap,
		RectWH(START_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);

	_leftEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(0, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT), this);
	_rightEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(EDGE_WIDTH, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT), this);
	_topEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(EDGE_WIDTH * 2, EDGE_Y, EDGE_HEIGHT, EDGE_WIDTH), this);
	_bottomEdgeBmp = GfxSurface(_partsBitmap,
		RectWH((EDGE_WIDTH * 2) + EDGE_HEIGHT, EDGE_Y,
			EDGE_HEIGHT, EDGE_WIDTH), this);

	for (int i = 0; i < NUM_TRAP_MAPS; i++) {
		_trapBitmap[i] = GfxSurface(_partsBitmap,
			RectWH(TRAP_WIDTH * i, 0, TRAP_WIDTH, TRAP_HEIGHT), this);
	}

	_playerSprite = _leftBitmap;
	_playerSprite.setTransparentColor(WHITE);

	_localeBitmap.loadBitmap(IDB_LOCALE_BMP);
	_blankBitmap.loadBitmap(IDB_BLANK_BMP);
}

void MazeDoom::loadIniSettings() {
	if (pGameParams->bplayingMetagame) {
		if (pGameParams->nSkillLevel == SkillLevel::SKILLLEVEL_LOW) {
			// Total Wussy
			_difficulty = MIN_DIFFICULTY;
			_time = 60;
		} else if (pGameParams->nSkillLevel == SkillLevel::SKILLLEVEL_MEDIUM) {
			// Big Sissy
			_difficulty = 2;
			_time = 60;
		} else {
			// Minor Whimp
			_difficulty = 4;
			_time = 60;
		}
	} else {
		_difficulty = !_settings.hasKey("difficulty") ? DEFAULT_DIFFICULTY :
			CLIP(_settings.getInt("difficulty"), MIN_DIFFICULTY, MAX_DIFFICULTY);
		int time = !_settings.hasKey("time_limit") ? TIMER_DEFAULT :
			CLIP(_settings.getInt("time_limit"), TIMER_MIN, TIMER_MAX);
		_time = (time == 0) ? 0 : TIME_SCALES[time - 1];
	}

	_seconds = _time % 60;
	_minutes = _time / 60;
	_priorTime = g_system->getMillis();
}

void MazeDoom::showMainMenu() {
	// Flag is not initially set so we can draw the maze
	// in standalone mode withouth the time remaining showing
	_playing = true;

	MainMenu::show(
		pGameParams->bplayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		RULES_TEXT,
		pGameParams->bSoundEffectsEnabled ? RULES_WAV : NULL);
}

void MazeDoom::newGame() {
	loadIniSettings();
	_move.clear();

	initializeMaze();		// Set the surrounding wall and start/end squares
	createMaze();			// Create a maze layout given the intiialized maze
	setupMaze();			// Translate maze data to grid layout for display
	paintMaze();			// Paint that sucker to the offscreen bitmap

	// Flag the new game to start
	_playing = true;
	_gameOver = false;
}

void MazeDoom::movePlayer(const Common::Point &point) {
	Common::Point tileLocation;
	Common::Point delta;
	const GfxSurface *bitmap = &_rightBitmap;

	_move.clear();
	_move._hit = screenToTile(point);
	_move._newPosition.x = _playerPos.x;
	_move._newPosition.y = _playerPos.y;

	delta.x = _playerPos.x - _move._hit.x;	// Get x distance from mouse click to player in Tile spaces 
	delta.y = _playerPos.y - _move._hit.y;	// Get y distance from mouse click to player in Tile spaces 

	if (ABS(delta.x) > ABS(delta.y)) {
		// Moving horizontally
		if (delta.x < 0) {
			// To the RIGHT
			_move._step.x = 1;			// move one tile at a time
			bitmap = &_rightBitmap;
		} else if (delta.x > 0) {
			// To the LEFT
			_move._step.x = -1;		// move one tile at a time
			bitmap = &_leftBitmap;
		}
	} else if (ABS(delta.y) > ABS(delta.x)) {
		if (delta.y > 0) {
			// Going Upward
			_move._step.y = -1;		// move one tile at a time                                         
			bitmap = &_upBitmap;	// use Bitmap of player moving Up
		} else if (delta.y < 0) {
			// Going Downward
			_move._step.y = 1;		// move one tile at a time
			bitmap = &_downBitmap;	// use Bitmap of player moving Down
		}
	}

	if ((_move._step.x != 0) || (_move._step.y != 0)) {
		// If the click is not in the player's tile, preparing for moving
		_playerSprite = *bitmap;
		_playerSprite.x = (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER;
		_playerSprite.y = (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2;

		_move.walk();
	}
}

void MazeDoom::walking() {
	assert(_move.isWalking());
	_move._newPosition += _move._step;

	if (_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == PATH ||      // Either a pathway 
		((_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == TRAP &&     //...or a 
			_mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden == false) ||  //...revealed trap
			_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == EXIT)) {   //...or exit is a GO
		_playerSprite.x = (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER;                  // Get player's position
		_playerSprite.y = (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2;     //...in screen coords

		// Start movement to new tile
		_move._substepCtr = 4;
		return;
	}

	// Carry on with movement check
	checkWalkFinished();
}

void MazeDoom::walkOneTile() {
	_playerSprite.x += _move._step.x * (SQ_SIZE_X / 4);
	_playerSprite.y += _move._step.y * (SQ_SIZE_Y / 4);
	redraw();

	if (--_move._substepCtr == 0) {
		// Set new player position
		_playerPos = _move._newPosition;
		_playerSprite.x = (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER;
		_playerSprite.y = (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2;

		checkWalkFinished();
	}
}

void MazeDoom::checkWalkFinished() {
	bool bCollision = false;
	CBofSound *pEffect = nullptr;

	if ((_mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden) &&
		(_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == WALL)) {

		if (pGameParams->bSoundEffectsEnabled) {
			// SOUND_ASYNCH ...Wave file, to delete itself
			pEffect = new CBofSound(this, HIT_SOUND,
				SOUND_WAVE | SOUND_AUTODELETE);
			(*pEffect).play();                                                      //...play the narration
		}
		_mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden = false;
		bCollision = true;
	}

	if (_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == TRAP &&
		(_mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden)) {
		// Traps are only good once 
		_mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden = false;

		// Set new position
		_playerPos.x = _mazeTile[_move._newPosition.x][_move._newPosition.y].m_nDest.x;
		_playerPos.y = _mazeTile[_move._newPosition.x][_move._newPosition.y].m_nDest.y;
		_playerSprite.x = (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER;
		_playerSprite.y = (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2;

		if (pGameParams->bSoundEffectsEnabled) {
			// SOUND_ASYNCH ...Wave file, to delete itself
			pEffect = new CBofSound(this, TRAP_SOUND,
				SOUND_WAVE | SOUND_AUTODELETE);
			(*pEffect).play();                                                      //...play the narration
		}

		// Paint trap in Maze bitmap for display
		_mazeBitmap.blitFrom(_trapBitmap[_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nTrap],
			Common::Point(
				_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nStart.x,
				_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nStart.y
			));
		bCollision = true;
	}

	if ((_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == WALL) ||
		(_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == START)) {
		bCollision = true;
	}

	if ((_move._newPosition.x == _move._hit.x) && (_move._newPosition.y == _move._hit.y))
		bCollision = true;

	exitCheck();

	if (bCollision) {
		paintMaze();
		redraw();
		_move.clear();
	}
}

void MazeDoom::exitCheck() {
	CBofSound *pEffect = nullptr;

	if (_mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall != EXIT)
		return;

	_playing = false;
	_gameOver = true;

	if (pGameParams->bSoundEffectsEnabled) {
		//...Wave file, to delete itself
		pEffect = new CBofSound(this, WIN_SOUND,
			SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);
		(*pEffect).play();                                                      //...play the narration
	}

	g_events->clearEvents();

	MessageBox::show("Game over.", "He's free!", "EXIT");
}

Common::Point MazeDoom::screenToTile(const Common::Point &pointScreen) const {
	Common::Point point;

	point.x = (pointScreen.x - SIDE_BORDER) / SQ_SIZE_X;
	point.y = (pointScreen.y - TOP_BORDER + SQ_SIZE_Y / 2) / SQ_SIZE_Y;

	return point;
}

bool MazeDoom::inArtRegion(const Common::Point &point) const {
	return ((point.x > SIDE_BORDER && point.x < GAME_WIDTH - SIDE_BORDER) &&
		(point.y > TOP_BORDER && point.y < GAME_HEIGHT - BOTTOM_BORDER));
}

void MazeDoom::getNewCursor(const Common::Point &mousePos) {
	Common::Point hit, delta;
	int hNewCursor = -1;
	hit = screenToTile(mousePos);

	delta.x = _playerPos.x - hit.x;
	delta.y = _playerPos.y - hit.y;

	if ((_playerPos.x == hit.x) && (_playerPos.y == hit.y)) {
		// Directly over player
		hNewCursor = IDC_MOD_NOARROW;
	}

	else if (ABS(delta.x) >= ABS(delta.y)) {
		// Moving horizontally:
		if (delta.x <= 0)
			// To the Right
			hNewCursor = IDC_MOD_RTARROW;
		else if (delta.x > 0)
			// To the Left
			hNewCursor = IDC_MOD_LFARROW;
	} else if (ABS(delta.y) > ABS(delta.x)) {
		if (delta.y >= 0)
			// Going Upward
			hNewCursor = IDC_MOD_UPARROW;
		else if (delta.y < 0)
			// Going Downward
			hNewCursor = IDC_MOD_DNARROW;
	}

	if (hNewCursor != -1)
		g_events->setCursor(hNewCursor);
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
