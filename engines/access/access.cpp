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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "access/access.h"
#include "access/debugger.h"

namespace Access {

AccessEngine::AccessEngine(OSystem *syst, const AccessGameDescription *gameDesc)
	: _gameDescription(gameDesc), Engine(syst), _randomSource("Access"),
	  _useItem(_flags[99]), _startup(_flags[170]), _manScaleOff(_flags[172]) {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound and Music handling");

	_aboutBox = nullptr;
	_animation = nullptr;
	_bubbleBox = nullptr;
	_char = nullptr;
	_events = nullptr;
	_files = nullptr;
	_invBox = nullptr;
	_inventory = nullptr;
	_helpBox = nullptr;
	_midi = nullptr;
	_player = nullptr;
	_res = nullptr;
	_room = nullptr;
	_screen = nullptr;
	_scripts = nullptr;
	_sound = nullptr;
	_travelBox = nullptr;
	_video = nullptr;

	_destIn = nullptr;
	_current = nullptr;
	_mouseMode = 0;
	_playerDataCount = 0;
	_currentMan = 0;
	_currentManOld = -1;
	_converseMode = 0;
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
	_scrollCol = _scrollRow = 0;
	_scrollX = _scrollY = 0;
	_imgUnscaled = false;
	_canSaveLoad = false;
	_establish = nullptr;

	_conversation = 0;
	_currentMan = 0;
	_newTime = 0;
	_newDate = 0;
	Common::fill(&_objectsTable[0], &_objectsTable[100], (SpriteResource *)nullptr);
	Common::fill(&_establishTable[0], &_establishTable[100], false);
	Common::fill(&_flags[0], &_flags[256], 0);
	_establishFlag = false;
	_establishMode = 0;
	_establishGroup = 0;
	_establishCtrlTblOfs = 0;
	_lastTime = g_system->getMillis();
	_curTime = 0;
	_narateFile = 0;
	_txtPages = 0;
	_sndSubFile = 0;
	_loadSaveSlot = -1;
	_vidX = _vidY = 0;
	_cheatFl = false;
	_restartFl = false;
	_printEnd = 0;
	for (int i = 0; i < 100; i++)
		_objectsTable[i] = nullptr;
	_clearSummaryFlag = false;

	for (int i = 0; i < 60; i++)
		_travel[i] = 0;
	_startTravelItem = _startTravelBox = 0;
	for (int i = 0; i < 33; i++)
		_ask[i] = 0;
	_startAboutItem = _startAboutBox = 0;
	_byte26CB5 = 0;
	_bcnt = 0;
	_boxDataStart = 0;
	_boxDataEnd = false;
	_boxSelectY = 0;
	_boxSelectYOld = -1;
	_numLines = 0;
	_tempList = nullptr;
	_pictureTaken = 0;

	_vidEnd = false;
}

AccessEngine::~AccessEngine() {
	delete _animation;
	delete _bubbleBox;
	delete _helpBox;
	delete _travelBox;
	delete _invBox;
	delete _aboutBox;
	delete _char;
	delete _events;
	delete _files;
	delete _inventory;
	delete _midi;
	delete _player;
	delete _res;
	delete _room;
	delete _screen;
	delete _scripts;
	delete _sound;
	delete _video;

	freeCells();
	delete _establish;
}

void AccessEngine::setVGA() {
	initGraphics(320, 200);
}

void AccessEngine::initialize() {
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
	_animation = new AnimationManager(this);
	_bubbleBox = new BubbleBox(this, TYPE_2, 64, 32, 130, 122, 0, 0, 0, 0, "");
	if (getGameID() == GType_MartianMemorandum) {
		_helpBox = new BubbleBox(this, TYPE_1, 64, 24, 146, 122, 1, 32, 2, 76, "HELP");
		_travelBox = new BubbleBox(this, TYPE_1, 64, 32, 194, 122, 1, 24, 2, 74, "TRAVEL");
		_invBox = new BubbleBox(this, TYPE_1, 64, 32, 146, 122, 1, 32, 2, 76, "INVENTORY");
		_aboutBox = new BubbleBox(this, TYPE_1, 64, 32, 194, 122, 1, 32, 2, 76, "ASK ABOUT");
	} else {
		_helpBox = nullptr;
		_travelBox = nullptr;
		_invBox = nullptr;
		_aboutBox = nullptr;
	}
	_char = new CharManager(this);
	_events = new EventsManager(this);
	_files = new FileManager(this);
	_inventory = new InventoryManager(this);
	_player = Player::init(this);
	_screen = new Screen(this);
	_sound = new SoundManager(this, _mixer);
	_midi = new MusicManager(this);
	_video = new VideoPlayer(this);

	setDebugger(Debugger::init(this));
	_buffer1.create(g_system->getWidth() + TILE_WIDTH, g_system->getHeight());
	_buffer2.create(g_system->getWidth(), g_system->getHeight());
	_vidBuf.create(160, 101);

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadSaveSlot = saveSlot;
	}
}

Common::Error AccessEngine::run() {
	_res = Resources::init(this);
	Common::U32String errorMessage;
	if (!_res->load(errorMessage)) {
		GUIErrorMessage(errorMessage);
		return Common::kNoError;
	}

	setVGA();
	initialize();

	playGame();

	return Common::kNoError;
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

void AccessEngine::speakText(BaseSurface *s, const Common::String &msg) {
	Common::String lines = msg;
	Common::String line;
	int curPage = 0;
	int soundsLeft = 0;

	while (!shouldQuit()) {
		soundsLeft = _countTbl[curPage];
		_events->zeroKeys();

		int width = 0;
		bool lastLine = _fonts._font2->getLine(lines, s->_maxChars * 6, line, width);

		// Set font colors
		Font::_fontColors[0] = 0;
		Font::_fontColors[1] = 28;
		Font::_fontColors[2] = 29;
		Font::_fontColors[3] = 30;

		_fonts._font2->drawString(s, line, s->_printOrg);
		s->_printOrg = Common::Point(s->_printStart.x, s->_printOrg.y + 9);

		if ((s->_printOrg.y > _printEnd) && (!lastLine)) {
			_events->clearEvents();
			while (!shouldQuit()) {
				_sound->freeSounds();
				_sound->loadSoundTable(0, _narateFile + 99, _sndSubFile);
				_sound->playSound(0);

				while(_sound->isSFXPlaying() && !shouldQuit())
					_events->pollEvents();

				_scripts->cmdFreeSound();

				if (_events->isKeyMousePressed()) {
					_sndSubFile += soundsLeft;
					break;
				} else {
					++_sndSubFile;
					--soundsLeft;
					if (soundsLeft == 0)
						break;
					_events->clearEvents();
				}
			}

			s->copyBuffer(&_buffer2);
			s->_printOrg.y = s->_printStart.y;
			++curPage;
			soundsLeft = _countTbl[curPage];
		}

		if (lastLine)
			break;
	}

	while (soundsLeft) {
		_sound->freeSounds();
		Resource *res = _sound->loadSound(_narateFile + 99, _sndSubFile);
		_sound->_soundTable.push_back(SoundEntry(res, 1));
		_sound->playSound(0);

		while(_sound->isSFXPlaying() && !shouldQuit())
			_events->pollEvents();

		_scripts->cmdFreeSound();

		if (_events->_leftButton) {
			_events->debounceLeft();
			_sndSubFile += soundsLeft;
			break;
		} else if (_events->isKeyPending()) {
			_sndSubFile += soundsLeft;
			break;
		} else {
			++_sndSubFile;
			--soundsLeft;
		}
	}
}

void AccessEngine::printText(BaseSurface *s, const Common::String &msg) {
	Common::String lines = msg;
	Common::String line;
	int width = 0;

	for (;;) {
		bool lastLine = _fonts._font2->getLine(lines, s->_maxChars * 6, line, width);

		// Set font colors
		_fonts._font2->_fontColors[0] = 0;
		_fonts._font2->_fontColors[1] = 28;
		_fonts._font2->_fontColors[2] = 29;
		_fonts._font2->_fontColors[3] = 30;
		_fonts._font2->drawString(s, line, s->_printOrg);

		s->_printOrg = Common::Point(s->_printStart.x, s->_printOrg.y + 9);

		if (s->_printOrg.y >_printEnd && !lastLine) {
			_events->waitKeyMouse();
			s->copyBuffer(&_buffer2);
			s->_printOrg.y = s->_printStart.y;
		}

		if (lastLine)
			break;
	}
	_events->waitKeyMouse();
}


void AccessEngine::plotList() {
	_player->calcPlayer();
	plotList1();
}

void AccessEngine::plotList1() {
	for (uint idx = 0; idx < _images.size(); ++idx) {
		ImageEntry &ie = _images[idx];

		_imgUnscaled = (ie._flags & IMGFLAG_UNSCALED) != 0;
		Common::Point pt = ie._position - _screen->_bufferStart;
		SpriteResource *sprites = ie._spritesPtr;
		SpriteFrame *frame = sprites->getFrame(ie._frameNumber);

		Common::Rect bounds(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h);
		if (!_imgUnscaled) {
			bounds.setWidth(_screen->_scaleTable1[frame->w]);
			bounds.setHeight(_screen->_scaleTable1[frame->h]);
		}

		// Make a copy - some of the drawing methods I've adapted need the full
		// scaled dimensions on-screen, and handle clipping themselves
		Common::Rect destBounds = bounds;

		if (_buffer2.clip(bounds)) {
			ie._flags |= IMGFLAG_CROPPED;
		} else {
			ie._flags &= ~IMGFLAG_CROPPED;
			if (_buffer2._leftSkip != 0 ||  _buffer2._rightSkip != 0
				|| _buffer2._topSkip != 0 || _buffer2._bottomSkip != 0)
				ie._flags |= IMGFLAG_CROPPED;

			_newRects.push_back(bounds);

			if (!_imgUnscaled) {
				_buffer2._rightSkip /= _scale;
				bounds.setWidth(bounds.width() / _scale);

				if (ie._flags & IMGFLAG_BACKWARDS) {
					_buffer2.sPlotB(frame, destBounds);
				} else {
					_buffer2.sPlotF(frame, destBounds);
				}
			} else {
				if (ie._flags & IMGFLAG_BACKWARDS) {
					_buffer2.plotB(frame, Common::Point(destBounds.left, destBounds.top));
				} else {
					_buffer2.plotF(frame, Common::Point(destBounds.left, destBounds.top));
				}
			}
		}

		ie._flags |= IMGFLAG_DRAWN;
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
		Common::Rect(_scrollX, _scrollY,
		_scrollX + _screen->_vWindowBytesWide,
		_scrollY + _screen->_vWindowLinesTall));
}

void AccessEngine::copyBF2Vid() {
	_screen->blitFrom(_buffer2,
		Common::Rect(0, 0, _screen->_vWindowBytesWide, _screen->_vWindowLinesTall),
		Common::Point(_screen->_windowXAdd, _screen->_windowYAdd));
}

void AccessEngine::playVideo(int videoNum, const Common::Point &pt) {
	_video->setVideo(_screen, pt, FileIdent(96, videoNum), 10);

	while (!shouldQuit() && !_video->_videoEnd) {
		_video->playVideo();
		_events->pollEventsAndWait();
	}
}

void AccessEngine::freeChar() {
	_scripts->freeScriptData();
	_animation->clearTimers();
	_animation->freeAnimationData();
}

Common::Error AccessEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(
		getSaveStateName(slot));
	if (!out)
		return Common::kCreatingFileFailed;

	AccessSavegameHeader header;
	header._saveName = desc;
	writeSavegameHeader(out, header);

	Common::Serializer s(nullptr, out);
	synchronize(s);

	out->finalize();
	delete out;

	return Common::kNoError;
}

Common::Error AccessEngine::loadGameState(int slot) {
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		getSaveStateName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	Common::Serializer s(saveFile, nullptr);

	// Load the savaegame header
	AccessSavegameHeader header;
	if (!readSavegameHeader(saveFile, header))
		error("Invalid savegame");

	// Load most of the savegame data
	synchronize(s);
	delete saveFile;

	// Set extra post-load state
	_room->_function = FN_CLEAR1;
	_timers._timersSavedFlag = false;
	_events->clearEvents();

	return Common::kNoError;
}

bool AccessEngine::canLoadGameStateCurrently() {
	return _canSaveLoad;
}

bool AccessEngine::canSaveGameStateCurrently() {
	return _canSaveLoad;
}

void AccessEngine::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_conversation);
	s.syncAsUint16LE(_currentMan);
	s.syncAsUint32LE(_newTime);
	s.syncAsUint32LE(_newDate);

	for (int i = 0; i < 256; ++i)
		s.syncAsUint16LE(_flags[i]);
	for (int i = 0; i < 100; ++i)
		s.syncAsByte(_establishTable[i]);

	// Synchronize sub-objects
	_timers.synchronize(s);
	_inventory->synchronize(s);
	_player->synchronize(s);
}

const char *const SAVEGAME_STR = "ACCESS";
#define SAVEGAME_STR_SIZE 6

WARN_UNUSED_RESULT bool AccessEngine::readSavegameHeader(Common::InSaveFile *in, AccessSavegameHeader &header, bool skipThumbnail) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > ACCESS_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header._saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header._thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();
	header._totalFrames = in->readUint32LE();

	return true;
}

void AccessEngine::writeSavegameHeader(Common::OutSaveFile *out, AccessSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(ACCESS_SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(header._saveName);
	out->writeByte('\0');

	// Write a thumbnail of the screen
	uint8 thumbPalette[PALETTE_SIZE];
	_screen->getPalette(thumbPalette);
	Graphics::Surface saveThumb;
	::createThumbnail(&saveThumb, (const byte *)_screen->getPixels(),
		_screen->w, _screen->h, thumbPalette);
	Graphics::saveThumbnail(*out, saveThumb);
	saveThumb.free();

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(_events->getFrameCounter());
}

void AccessEngine::SPRINTCHR(char c, int fontNum) {
	warning("TODO: SPRINTCHR");
	_fonts._font1->drawChar(_screen, c, _screen->_printOrg);
}

void AccessEngine::PRINTCHR(Common::String msg, int fontNum) {
	_events->hideCursor();
	warning("TODO: PRINTCHR - Handle fontNum");

	for (int i = 0; msg[i]; i++) {
		if (!(_fonts._charSet._hi & 8)) {
			_fonts._font1->drawChar(_screen, msg[i], _screen->_printOrg);
			continue;
		} else if (_fonts._charSet._hi & 2) {
			Common::Point oldPos = _screen->_printOrg;
			int oldFontLo = _fonts._charFor._lo;

			_fonts._charFor._lo = 0;
			_screen->_printOrg.x++;
			_screen->_printOrg.y++;
			SPRINTCHR(msg[i], fontNum);

			_screen->_printOrg = oldPos;
			_fonts._charFor._lo = oldFontLo;
		}
		SPRINTCHR(msg[i], fontNum);
	}
	_events->showCursor();
}

bool AccessEngine::shouldQuitOrRestart() {
	return shouldQuit() || _restartFl;
}
} // End of namespace Access
