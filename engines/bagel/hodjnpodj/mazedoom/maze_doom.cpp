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
		pPlayerSprite(this),
		_timeRect(RectWH(TIME_LOCATION_X + 50, TIME_LOCATION_Y,
			TIME_WIDTH - 50, TIME_HEIGHT)) {
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
		pGameSound = new CBofSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (pGameSound != nullptr) {
			(*pGameSound).midiLoopPlaySegment(3000, 32980, 0, FMT_MILLISEC);
		}
	}

	bPlaying = pGameParams->bPlayingMetagame;
	m_bGameOver = false;
	_move.clear();

	return true;
}

bool MazeDoom::msgClose(const CloseMessage &msg) {
	MinigameView::msgClose(msg);

	_background.clear();
	_scrollButton.clear();
	_mazeBitmap.clear();
	_partsBitmap.clear();
	pBlankBitmap.clear();
	pLocaleBitmap.clear();
	pPlayerSprite.clear();

	pWallBitmap.clear();
	pPathBitmap.clear();
	pStartBitmap.clear();
	pLeftEdgeBmp.clear();
	pRightEdgeBmp.clear();
	pTopEdgeBmp.clear();
	pBottomEdgeBmp.clear();

	for (int i = 0; i < NUM_TRAP_MAPS; ++i)
		TrapBitmap[i].clear();

	delete pGameSound;
	pGameSound = nullptr;

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
		assert(!pGameParams->bPlayingMetagame);
		newGame();
		return true;
	} else if (msg._name == "GAME_OVER") {
		CBofSound::waitWaveSounds();
		if (pGameParams->bPlayingMetagame) {
			pGameParams->lScore = 0;
			close();
		}

		return true;
	} else if (msg._name == "EXIT") {
		CBofSound::waitWaveSounds();

		if (pGameParams->bPlayingMetagame) {
			pGameParams->lScore = 1;	// A victorious maze solving
			close();					// and close minigame
		}

		return true;
	} else if (msg._name == "OPTIONS") {
		_options.setTime(nMinutes, nSeconds);
		_options.addView();
	}

	return false;
}

bool MazeDoom::msgMouseDown(const MouseDownMessage &msg) {
	if (MinigameView::msgMouseDown(msg))
		return true;

	if (msg._button == MouseDownMessage::MB_LEFT) {
		const RectWH rectTitle(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
			NEWGAME_WIDTH, NEWGAME_HEIGHT);

		if (rectTitle.contains(msg._pos) && !pGameParams->bPlayingMetagame)
			// Start new game
			newGame();

		if (bPlaying && inArtRegion(msg._pos))
			movePlayer(msg._pos);
	}

	return true;
}

bool MazeDoom::msgMouseMove(const MouseMoveMessage &msg) {
	if (MinigameView::msgMouseMove(msg))
		return true;

	if (inArtRegion(msg._pos) && bPlaying) {
		// If the cursor is within the border
		// and we're playing, update the cursor
		getNewCursor(msg._pos);

		if (msg._button == MouseMoveMessage::MB_LEFT) {
			// If the Left mouse button is down,
			// have the player follow the mouse
			movePlayer(msg._pos);
		}
	} else {
		g_events->setCursor(IDC_ARROW);
	}

	return true;
}

bool MazeDoom::msgKeypress(const KeypressMessage &msg) {
	Common::Point newPosition = pPlayerSprite.getPosition();

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
	Common::Point newPosition = pPlayerSprite.getPosition();

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

		if (bPlaying && !m_bGameOver)
			updateTimer();
	}

	// Handle automatic moving
	if (_move.isMoving())
		playerMoving();

	exitCheck();

	return true;
}

void MazeDoom::updateTimer() {
	if (m_nTime == 0) {
		// No time limit, increment 
		nSeconds++;
		if (nSeconds == 60) {
			nMinutes++;
			nSeconds = 0;
		}
	} else {
		// Count down time left
		if (nSeconds == 0 && nMinutes == 0) {
			gameOver();
		} else if (nSeconds == 0) {
			nMinutes--;
			nSeconds = 60;
		}

		nSeconds--;
	}

	redraw();
}

void MazeDoom::gameOver() {
	CBofSound *pEffect = nullptr;

	bPlaying = false;
	m_bGameOver = true;
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

	// Draw the maze
	s.blitFrom(_mazeBitmap, Common::Point(
		SIDE_BORDER, TOP_BORDER));

	if (!pPlayerSprite.empty() && bPlaying)
		s.blitFrom(pPlayerSprite, Common::Point(
			(m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER,
			(m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2
		));

	if (bPlaying) {
		// only false when the options are displayed
		char msg[64];
		s.blitFrom(pBlankBitmap, Common::Point(TIME_LOCATION_X, TIME_LOCATION_Y));

		if (m_nTime == 0)
			Common::sprintf_s(msg, "Time Used: %02d:%02d", nMinutes, nSeconds);
		else {
			Common::sprintf_s(msg, "Time Left: %02d:%02d", nMinutes, nSeconds);
		}

		s.setFontSize(8);
		s.writeString(msg, _timeRect, BLACK);
	} else if (!pLocaleBitmap.empty()) {
		s.blitFrom(pLocaleBitmap,
			Common::Point(TIME_LOCATION_X, TIME_LOCATION_Y));
	}
}

void MazeDoom::setupHodjPodj() {
	if (pGameParams->bPlayingHodj) {
		_upBmp = IDB_HODJ_UP_BMP;
		_downBmp = IDB_HODJ_DOWN_BMP;
		_leftBmp = IDB_HODJ_LEFT_BMP;
		_rightBmp = IDB_HODJ_RIGHT_BMP;
	} else {
		_upBmp = IDB_PODJ_UP_BMP;
		_downBmp = IDB_PODJ_DOWN_BMP;
		_leftBmp = IDB_PODJ_LEFT_BMP;
		_rightBmp = IDB_PODJ_RIGHT_BMP;
	}
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

	pWallBitmap = GfxSurface(_partsBitmap,
		RectWH(WALL_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);
	pPathBitmap = GfxSurface(_partsBitmap,
		RectWH(PATH_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);
	pStartBitmap = GfxSurface(_partsBitmap,
		RectWH(START_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT), this);

	pLeftEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(0, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT), this);
	pRightEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(EDGE_WIDTH, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT), this);
	pTopEdgeBmp = GfxSurface(_partsBitmap,
		RectWH(EDGE_WIDTH * 2, EDGE_Y, EDGE_HEIGHT, EDGE_WIDTH), this);
	pBottomEdgeBmp = GfxSurface(_partsBitmap,
		RectWH((EDGE_WIDTH * 2) + EDGE_HEIGHT, EDGE_Y,
			EDGE_HEIGHT, EDGE_WIDTH), this);

	for (int i = 0; i < NUM_TRAP_MAPS; i++) {
		TrapBitmap[i] = GfxSurface(_partsBitmap,
			RectWH(TRAP_WIDTH * i, 0, TRAP_WIDTH, TRAP_HEIGHT), this);
	}

	pPlayerSprite.loadCels(_leftBmp, NUM_CELS);
	pPlayerSprite.setTransparentColor(WHITE);

	pLocaleBitmap.loadBitmap(IDB_LOCALE_BMP);
	pBlankBitmap.loadBitmap(IDB_BLANK_BMP);
}

void MazeDoom::loadIniSettings() {
	if (pGameParams->bPlayingMetagame) {
		if (pGameParams->nSkillLevel == Metagame::SKILLLEVEL_LOW) {
			// Total Wussy
			m_nDifficulty = MIN_DIFFICULTY;
			m_nTime = 60;
		} else if (pGameParams->nSkillLevel == Metagame::SKILLLEVEL_MEDIUM) {
			// Big Sissy
			m_nDifficulty = 2;
			m_nTime = 60;
		} else {
			// Minor Whimp
			m_nDifficulty = 4;
			m_nTime = 60;
		}
	} else {
		Common::String domain = ConfMan.getActiveDomainName();
		ConfMan.setActiveDomain("MazeDoom");

		m_nDifficulty = !ConfMan.hasKey("Difficulty") ? DEFAULT_DIFFICULTY :
			CLIP(ConfMan.getInt("Difficulty"), MIN_DIFFICULTY, MAX_DIFFICULTY);
		int time = !ConfMan.hasKey("Time") ? TIMER_DEFAULT :
			CLIP(ConfMan.getInt("Time"), TIMER_MIN, TIMER_MAX);
		m_nTime = (time == 0) ? 0 : TIME_SCALES[time - 1];

		ConfMan.setActiveDomain(domain);
	}

	nSeconds = m_nTime % 60;
	nMinutes = m_nTime / 60;
	_priorTime = g_system->getMillis();
}

void MazeDoom::showMainMenu() {
	// Flag is not initially set so we can draw the maze
	// in standalone mode withouth the time remaining showing
	bPlaying = true;

	MainMenu::show(
		pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		RULES_TEXT,
		pGameParams->bSoundEffectsEnabled ? RULES_WAV : NULL);
}

void MazeDoom::newGame() {
	loadIniSettings();
	_move.clear();

	if (m_nTime != 0) {				// If we've got a time limit
		nMinutes = m_nTime / 60;	//...get the minutes and seconds
		nSeconds = m_nTime % 60;
	} else {
		nMinutes = 0;
		nSeconds = 0;
	}

	initializeMaze();		// Set the surrounding wall and start/end squares
	createMaze();			// Create a maze layout given the intiialized maze
	setupMaze();			// Translate maze data to grid layout for display
	paintMaze();			// Paint that sucker to the offscreen bitmap

	// Flag the new game to start
	bPlaying = true;
	m_bGameOver = false;
}

void MazeDoom::movePlayer(const Common::Point &point) {
	Common::Point tileLocation;
	Common::Point delta;
	const char *nBmpID = _rightBmp;

	_move.clear();
	_move._hit = screenToTile(point);
	_move._newPosition.x = m_PlayerPos.x;
	_move._newPosition.y = m_PlayerPos.y;

	delta.x = m_PlayerPos.x - _move._hit.x;	// Get x distance from mouse click to player in Tile spaces 
	delta.y = m_PlayerPos.y - _move._hit.y;	// Get y distance from mouse click to player in Tile spaces 

	if (ABS(delta.x) > ABS(delta.y)) {
		// Moving horizontally
		if (delta.x < 0) {
			// To the RIGHT
			_move._step.x = 1;			// move one tile at a time
			nBmpID = _rightBmp;
		} else if (delta.x > 0) {
			// To the LEFT
			_move._step.x = -1;		// move one tile at a time
			nBmpID = _leftBmp;
		}
	} else if (ABS(delta.y) > ABS(delta.x)) {
		if (delta.y > 0) {
			// Going Upward
			_move._step.y = -1;		// move one tile at a time                                         
			nBmpID = _upBmp;	// use Bitmap of player moving Up
		} else if (delta.y < 0) {
			// Going Downward
			_move._step.y = 1;			// move one tile at a time
			nBmpID = _downBmp;	// use Bitmap of player moving Down
		}
	}

	if ((_move._step.x != 0) || (_move._step.y != 0)) {
		// If the click is not in the player's tile, preparing for moving
		pPlayerSprite.loadCels(nBmpID, NUM_CELS);
		pPlayerSprite.setTransparentColor(WHITE);
	}
}

void MazeDoom::playerMoving() {
	bool bCollision = false;
	CBofSound *pEffect = nullptr;

	assert(_move.isMoving());
	_move._newPosition += _move._step;

	if ((mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden) &&
		(mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == WALL)) {

		if (pGameParams->bSoundEffectsEnabled) {
			// SOUND_ASYNCH ...Wave file, to delete itself
			pEffect = new CBofSound(this, HIT_SOUND,
				SOUND_WAVE | SOUND_AUTODELETE);
			(*pEffect).play();                                                      //...play the narration
		}
		mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden = false;
		bCollision = true;
	}

	if (mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == TRAP &&
		(mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden)) {
		// Traps are only good once 
		mazeTile[_move._newPosition.x][_move._newPosition.y].m_bHidden = false;

		m_PlayerPos.x = mazeTile[_move._newPosition.x][_move._newPosition.y].m_nDest.x;
		m_PlayerPos.y = mazeTile[_move._newPosition.x][_move._newPosition.y].m_nDest.y;

		if (pGameParams->bSoundEffectsEnabled) {
			// SOUND_ASYNCH ...Wave file, to delete itself
			pEffect = new CBofSound(this, TRAP_SOUND,
				SOUND_WAVE | SOUND_AUTODELETE);
			(*pEffect).play();                                                      //...play the narration
		}

		// Paint trap in Maze bitmap for display
		_mazeBitmap.blitFrom(TrapBitmap[mazeTile[_move._newPosition.x][_move._newPosition.y].m_nTrap],
			Common::Point(
				mazeTile[_move._newPosition.x][_move._newPosition.y].m_nStart.x,
				mazeTile[_move._newPosition.x][_move._newPosition.y].m_nStart.y
			));
		bCollision = true;
	}

	if ((mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == WALL) ||
		(mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall == START)) {
		bCollision = true;
	}

	if ((_move._newPosition.x == _move._hit.x) && (_move._newPosition.y == _move._hit.y))
		bCollision = true;

	if (bCollision)
		_move.clear();
}

void MazeDoom::exitCheck() {
	CBofSound *pEffect = nullptr;

	if (mazeTile[_move._newPosition.x][_move._newPosition.y].m_nWall != EXIT)
		return;

	bPlaying = false;
	m_bGameOver = true;

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

	delta.x = m_PlayerPos.x - hit.x;
	delta.y = m_PlayerPos.y - hit.y;

	if ((m_PlayerPos.x == hit.x) && (m_PlayerPos.y == hit.y)) {
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
