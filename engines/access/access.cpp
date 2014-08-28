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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "access/access.h"

namespace Access {

AccessEngine::AccessEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
		_gameDescription(gameDesc), Engine(syst), _randomSource("Access"),
		_useItem(_flags[99]), _startup(_flags[170]), _manScaleOff(_flags[172]) {
	_animation = nullptr;
	_bubbleBox = nullptr;
	_char = nullptr;
	_debugger = nullptr;
	_events = nullptr;
	_files = nullptr;
	_inventory = nullptr;
	_player = nullptr;
	_room = nullptr;
	_screen = nullptr;
	_scripts = nullptr;
	_sound = nullptr;
	_video = nullptr;

	_destIn = nullptr;
	_current = nullptr;
	_pCount = 0;
	_normalMouse = true;
	_mouseMode = 0;
	_currentMan = 0;
	_currentManOld = -1;
	_inactive = nullptr;
	_music = nullptr;
	_title = nullptr;
	_converseMode = 0;
	_startAboutBox = 0;
	_startTravelBox = 0;
	_numAnimTimers = 0;
	_startup = 0;
	_currentCharFlag = false;
	_boxSelect = false;
	_scale = 0;
	_scaleH1 = _scaleH2 = 0;
	_scaleN1 = 0;
	_scaleT1 = 0;
	_scaleMaxY = 0;
	_scaleI = 0;
	_scaleFlag = false;
	_eseg = nullptr;

	_conversation = 0;
	_currentMan = 0;
	_newTime = 0;
	_newDate = 0;
	_intTim[3] = 0;
	_timer[3] = 0;
	Common::fill(&_objectsTable[0], &_objectsTable[100], (SpriteResource *)nullptr);
	Common::fill(&_establishTable[0], &_establishTable[100], false);
	Common::fill(&_flags[0], &_flags[256], 0);
	_establishFlag = false;
	_establishMode = 0;
	_establishGroup = 0;
	_establishCtrlTblOfs = 0;
	Common::fill(&_help1[0], &_help1[366], 0);
	Common::fill(&_help2[0], &_help2[366], 0);
	Common::fill(&_help1[0], &_help3[366], 0);
	_helpTbl[0] = _help1;
	_helpTbl[1] = _help2;
	_helpTbl[2] = _help3;
	_travel = 0;
	_ask = 0;
	_rScrollRow = 0;
	_rScrollCol = 0;
	_rScrollX = 0;
	_rScrollY = 0;
	_rOldRectCount = 0;
	_rNewRectCount = 0;
	_rKeyFlag = 0;
	_mapOffset = 0;
	_screenVirtX = 0;
	_lastTime = g_system->getMillis();
	_curTime = 0;
	_narateFile = 0;
	_txtPages = 0;
	_sndSubFile = 0;
}

AccessEngine::~AccessEngine() {
	delete _animation;
	delete _bubbleBox;
	delete _char;
	delete _debugger;
	delete _events;
	delete _files;
	delete _inventory;
	delete _player;
	delete _room;
	delete _screen;
	delete _scripts;
	delete _sound;
	delete _video;

	freeCells();
	delete _inactive;
	delete _music;
	delete _title;
}

void AccessEngine::setVGA() {
	initGraphics(320, 200, false);
}

void AccessEngine::initialize() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");

	if (isCD()) {
		const Common::FSNode gameDataDir(ConfMan.get("path"));
		// The CD version contains two versions of the game.
		// - The MCGA version, in the CDROM folder
		// - The VESA version, in the TDROM folder
		// We use the hires version.
		const Common::FSNode cdromDir = gameDataDir.getChild("tdrom");

		for (int idx = 0; idx < 15; ++idx) {
			Common::String folder = (idx == 0) ? "game" :
				Common::String::format("chap%.2d", idx);
			SearchMan.addSubDirectoryMatching(cdromDir, folder);
		}
	}

	// Create sub-objects of the engine
	ASurface::init();
	_animation = new AnimationManager(this);
	_bubbleBox = new BubbleBox(this);
	_char = new CharManager(this);
	_debugger = new Debugger(this);
	_events = new EventsManager(this);
	_files = new FileManager(this);
	_inventory = new InventoryManager(this);
	_player = new Player(this);
	_screen = new Screen(this);
	_sound = new SoundManager(this, _mixer);
	_video = new VideoPlayer(this);

	_buffer1.create(g_system->getWidth() + TILE_WIDTH, g_system->getHeight());
	_buffer2.create(g_system->getWidth(), g_system->getHeight());
}

Common::Error AccessEngine::run() {
	setVGA();
	initialize();

	playGame();

	dummyLoop();
	return Common::kNoError;
}

void AccessEngine::dummyLoop() {
	// Dummy game loop
	while (!shouldQuit()) {
		_events->pollEvents();

		_curTime = g_system->getMillis();
		// Process machine once every tick
		while (_curTime - _lastTime < 20) {
			g_system->delayMillis(5);
			_curTime = g_system->getMillis();
		}

		_lastTime = _curTime;

		g_system->updateScreen();

		if (_events->_leftButton) {
			CursorType cursorId = _events->getCursor();
			_events->setCursor((cursorId == CURSOR_HELP) ? CURSOR_ARROW : (CursorType)(cursorId + 1));
		}
	}
}

int AccessEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void AccessEngine::loadCells(Common::Array<CellIdent> &cells) {
	for (uint i = 0; i < cells.size(); ++i) {
		Resource *spriteData = _files->loadFile(cells[i]);
		_objectsTable[cells[i]._cell] = new SpriteResource(this, spriteData);
		delete spriteData;
	}
}

void AccessEngine::freeCells() {
	for (int i = 0; i < 100; ++i) {
		delete _objectsTable[i];
		_objectsTable[i] = nullptr;
	}
}

void AccessEngine::freeInactiveData() {
	delete[] _inactive;
	_inactive = nullptr;
}

void AccessEngine::speakText(ASurface *s, Common::Array<Common::String> msgArr) {
	int curPage = 0;
	int soundsLeft = 0;

	while(true) {
		soundsLeft = _countTbl[curPage];
		_events->zeroKeys();

		Common::String line;
		int width = 0;
		bool lastLine = _fonts._font2.getLine(msgArr[curPage], s->_maxChars * 6, line, width);
		// Set font colors
		_fonts._font2._fontColors[0] = 0;
		_fonts._font2._fontColors[1] = 28;
		_fonts._font2._fontColors[2] = 29;
		_fonts._font2._fontColors[3] = 30;

		_fonts._font2.drawString(s, line, s->_printOrg);
		s->_printOrg = Common::Point(s->_printStart.x, s->_printOrg.y + 9);

		if ((s->_printOrg.y > _printEnd) && (!lastLine)) {
			while (true) {
				_sound->_soundTable[0] = _sound->loadSound(_narateFile + 99, _sndSubFile);
				_sound->_soundPriority[0] = 1;
				_sound->playSound(1);
				_scripts->CMDFREESOUND();

				_events->pollEvents();

				if (_events->_leftButton) {
					_events->debounceLeft();
					_sndSubFile += soundsLeft;
					break;
				} else if (_events->_keypresses.size() != 0) {
					_sndSubFile += soundsLeft;
					break;
				} else {
					++_sndSubFile;
					--soundsLeft;
					if (soundsLeft == 0)
						break;
				}
			}
			_buffer2.copyBuffer(s);
			s->_printOrg.y = s->_printStart.y;
			++curPage;
			soundsLeft = _countTbl[curPage];
		}

		if (lastLine)
			break;
	}

	if (soundsLeft == 0)
		return;

	while(true) {
		_sound->_soundTable[0] = _sound->loadSound(_narateFile + 99, _sndSubFile);
		_sound->_soundPriority[0] = 1;
		_sound->playSound(1);
		_scripts->CMDFREESOUND();

		_events->pollEvents();

		if (_events->_leftButton) {
			_events->debounceLeft();
			_sndSubFile += soundsLeft;
			break;
		} else if (_events->_keypresses.size() != 0) {
			_sndSubFile += soundsLeft;
			break;
		} else {
			++_sndSubFile;
			--soundsLeft;
			if (soundsLeft == 0)
				break;
		}
	}
}

void AccessEngine::plotList() {
	_player->calcPlayer();
	plotList1();
}

void AccessEngine::plotList1() {
	for (uint idx = 0; idx < _images.size(); ++idx) {
		ImageEntry &ie = _images[idx];

		_scaleFlag = (ie._flags & 8) != 0;
		Common::Point pt = ie._position - _screen->_bufferStart;
		SpriteResource *sprites = ie._spritesPtr;
		SpriteFrame *frame = sprites->getFrame(ie._frameNumber);

		Common::Rect bounds(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h);
		if (!_scaleFlag) {
			bounds.setWidth(_screen->_scaleTable1[frame->w]);
			bounds.setHeight(_screen->_scaleTable1[frame->h]);
		}

		// Make a copy - some of the drawing methods I've adapted need the full
		// scaled dimensions on-screen, and handle clipping themselves
		Common::Rect destBounds = bounds;

		if (_buffer2.clip(bounds)) {
			ie._flags |= 1;
		} else {
			ie._flags &= ~1;
			if (_buffer2._leftSkip != 0 ||  _buffer2._rightSkip != 0
				|| _buffer2._topSkip != 0 || _buffer2._bottomSkip != 0)
				ie._flags |= 1;

			_newRects.push_back(bounds);

			if (!_scaleFlag) {
				_buffer2._rightSkip /= _scale;
				bounds.setWidth(bounds.width() / _scale);

				if (ie._flags & 2) {
					_buffer2.sPlotB(frame, destBounds);
				} else {
					_buffer2.sPlotF(frame, destBounds);
				}
			} else {
				if (ie._flags & 2) {
					_buffer2.plotB(frame, Common::Point(bounds.left, bounds.top));
				} else {
					_buffer2.plotF(frame, Common::Point(bounds.left, bounds.top));
				}
			}
		}
	}
}

void AccessEngine::copyBlocks() {
	// Copy the block list from the previous frame
	for (uint i = 0; i < _oldRects.size(); ++i) {
		_screen->copyBlock(&_buffer2, _oldRects[i]);
	}

	copyRects();
}

void AccessEngine::copyRects() {
	_oldRects.clear();
	for (uint i = 0; i < _newRects.size(); ++i) {
		_screen->copyBlock(&_buffer2, _newRects[i]);
		_oldRects.push_back(_newRects[i]);
	}
}

void AccessEngine::copyBF1BF2() {
	_buffer2.copyRectToSurface(_buffer1, 0, 0,
		Common::Rect(_buffer1._scrollX, _buffer1._scrollY,
		_buffer1._scrollX + _screen->_vWindowBytesWide,
		_buffer1._scrollY + _screen->_vWindowLinesTall));
}

void AccessEngine::copyBF2Vid() {
	const byte *srcP = (const byte *)_buffer2.getPixels();
	byte *destP = (byte *)_screen->getBasePtr(_screen->_windowXAdd, 
		_screen->_windowYAdd + _screen->_screenYOff);

	for (int yp = 0; yp < _screen->_vWindowLinesTall; ++yp) {
		Common::copy(srcP, srcP + _screen->_vWindowBytesWide, destP);
		srcP += _buffer2.pitch;
		destP += _screen->pitch;
	}
}

void AccessEngine::doLoadSave() {
	error("TODO: doLoadSave");
}

void AccessEngine::freeChar() {
	_scripts->freeScriptData();
	_animation->clearTimers();
	_animation->freeAnimationData();
}

} // End of namespace Access
