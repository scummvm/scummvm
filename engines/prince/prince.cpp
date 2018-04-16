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
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/fs.h"
#include "common/keyboard.h"
#include "common/substream.h"
#include "common/str.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"

#include "engines/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "prince/prince.h"
#include "prince/font.h"
#include "prince/graphics.h"
#include "prince/script.h"
#include "prince/debugger.h"
#include "prince/object.h"
#include "prince/mob.h"
#include "prince/sound.h"
#include "prince/variatxt.h"
#include "prince/flags.h"
#include "prince/font.h"
#include "prince/mhwanh.h"
#include "prince/cursor.h"
#include "prince/archive.h"
#include "prince/hero.h"
#include "prince/resource.h"
#include "prince/animation.h"
#include "prince/option_text.h"
#include "prince/curve_values.h"

namespace Prince {

void PrinceEngine::debugEngine(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debug("Prince::Engine %s", buf);
}

PrinceEngine::PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _graph(nullptr), _script(nullptr), _interpreter(nullptr), _flags(nullptr),
	_locationNr(0), _debugger(nullptr), _midiPlayer(nullptr), _room(nullptr),
	_cursor1(nullptr), _cursor2(nullptr), _cursor3(nullptr), _font(nullptr),
	_suitcaseBmp(nullptr), _roomBmp(nullptr), _cursorNr(0), _picWindowX(0), _picWindowY(0), _randomSource("prince"),
	_invLineX(134), _invLineY(176), _invLine(5), _invLines(3), _invLineW(70), _invLineH(76), _maxInvW(72), _maxInvH(76),
	_invLineSkipX(2), _invLineSkipY(3), _showInventoryFlag(false), _inventoryBackgroundRemember(false),
	_mst_shadow(0), _mst_shadow2(0), _candleCounter(0), _invX1(53), _invY1(18), _invWidth(536), _invHeight(438),
	_invCurInside(false), _optionsFlag(false), _optionEnabled(0), _invExamY(120), _invMaxCount(2), _invCounter(0),
	_optionsMob(-1), _currentPointerNumber(1), _selectedMob(-1), _selectedItem(0), _selectedMode(0),
	_optionsWidth(210), _optionsHeight(170), _invOptionsWidth(210), _invOptionsHeight(130), _optionsStep(20),
	_invOptionsStep(20), _optionsNumber(7), _invOptionsNumber(5), _optionsColor1(236), _optionsColor2(252),
	_dialogWidth(600), _dialogHeight(0), _dialogLineSpace(10), _dialogColor1(220), _dialogColor2(223),
	_dialogFlag(false), _dialogLines(0), _dialogText(nullptr), _mouseFlag(1),
	_roomPathBitmap(nullptr), _roomPathBitmapTemp(nullptr), _coordsBufEnd(nullptr), _coordsBuf(nullptr), _coords(nullptr),
	_traceLineLen(0), _rembBitmapTemp(nullptr), _rembBitmap(nullptr), _rembMask(0), _rembX(0), _rembY(0), _fpX(0), _fpY(0),
	_checkBitmapTemp(nullptr), _checkBitmap(nullptr), _checkMask(0), _checkX(0), _checkY(0), _traceLineFirstPointFlag(false),
	_tracePointFirstPointFlag(false), _coordsBuf2(nullptr), _coords2(nullptr), _coordsBuf3(nullptr), _coords3(nullptr),
	_shanLen(0), _directionTable(nullptr), _currentMidi(0), _lightX(0), _lightY(0), _curveData(nullptr), _curvPos(0),
	_creditsData(nullptr), _creditsDataSize(0), _currentTime(0), _zoomBitmap(nullptr), _shadowBitmap(nullptr), _transTable(nullptr),
	_flcFrameSurface(nullptr), _shadScaleValue(0), _shadLineLen(0), _scaleValue(0), _dialogImage(nullptr), _mobTranslationData(nullptr),
	_mobTranslationSize(0) {

	// Debug/console setup
	DebugMan.addDebugChannel(DebugChannel::kScript, "script", "Prince Script debug channel");
	DebugMan.addDebugChannel(DebugChannel::kEngine, "engine", "Prince Engine debug channel");

	DebugMan.enableDebugChannel("script");

	memset(_audioStream, 0, sizeof(_audioStream));
}

PrinceEngine::~PrinceEngine() {
	DebugMan.clearAllDebugChannels();

	delete _rnd;
	delete _debugger;
	delete _cursor1;
	delete _cursor3;
	delete _midiPlayer;
	delete _script;
	delete _flags;
	delete _interpreter;
	delete _font;
	delete _roomBmp;
	delete _suitcaseBmp;
	delete _variaTxt;
	free(_talkTxt);
	free(_invTxt);
	free(_dialogDat);
	delete _graph;
	delete _room;

	if (_cursor2 != nullptr) {
		_cursor2->free();
		delete _cursor2;
	}

	for (uint i = 0; i < _objList.size(); i++) {
		delete _objList[i];
	}
	_objList.clear();

	free(_objSlot);

	for (uint32 i = 0; i < _pscrList.size(); i++) {
		delete _pscrList[i];
	}
	_pscrList.clear();

	for (uint i = 0; i < _maskList.size(); i++) {
		free(_maskList[i]._data);
	}
	_maskList.clear();

	_drawNodeList.clear();

	clearBackAnimList();
	_backAnimList.clear();

	freeAllNormAnims();
	_normAnimList.clear();

	for (uint i = 0; i < _allInvList.size(); i++) {
		_allInvList[i]._surface->free();
		delete _allInvList[i]._surface;
	}
	_allInvList.clear();

	_optionsPic->free();
	delete _optionsPic;

	_optionsPicInInventory->free();
	delete _optionsPicInInventory;

	for (uint i = 0; i < _mainHero->_moveSet.size(); i++) {
		delete _mainHero->_moveSet[i];
	}

	for (uint i = 0; i < _secondHero->_moveSet.size(); i++) {
		delete _secondHero->_moveSet[i];
	}

	delete _mainHero;
	delete _secondHero;

	free(_roomPathBitmap);
	free(_roomPathBitmapTemp);
	free(_coordsBuf);

	_mobPriorityList.clear();

	freeAllSamples();

	free(_zoomBitmap);
	free(_shadowBitmap);
	free(_transTable);

	free(_curveData);

	free(_shadowLine);

	free(_creditsData);

	if (_dialogImage != nullptr) {
		_dialogImage->free();
		delete _dialogImage;
	}

	free(_mobTranslationData);
}

GUI::Debugger *PrinceEngine::getDebugger() {
	return _debugger;
}

void PrinceEngine::init() {

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	debugEngine("Adding all path: %s", gameDataDir.getPath().c_str());

	if (getLanguage() != Common::RU_RUS) {
		PtcArchive *all = new PtcArchive();
		if (!all->open("all/databank.ptc"))
			error("Can't open all/databank.ptc");

		PtcArchive *voices = new PtcArchive();
		if (!voices->open("voices/databank.ptc"))
			error("Can't open voices/databank.ptc");

		PtcArchive *sound = new PtcArchive();
		if (!sound->open("sound/databank.ptc"))
			error("Can't open sound/databank.ptc");

		SearchMan.addSubDirectoryMatching(gameDataDir, "all");

		// Prefix the archive names, so that "all" doesn't conflict with the
		// "all" directory, if that happens to be named in all lower case.
		// It isn't on the CD, but we should try to stay case-insensitive.
		SearchMan.add("_all", all);
		SearchMan.add("_voices", voices);
		SearchMan.add("_sound", sound);
	} else {
		SearchMan.addSubDirectoryMatching(gameDataDir, "all");
		SearchMan.addSubDirectoryMatching(gameDataDir, "voices");
		SearchMan.addSubDirectoryMatching(gameDataDir, "sound");
	}

	if (getFeatures() & GF_TRANSLATED) {
		PtcArchive *translation = new PtcArchive();
		if (getFeatures() & GF_TRANSLATED) {
			if (!translation->openTranslation("all/prince_translation.dat"))
				error("Can't open prince_translation.dat");
		}

		SearchMan.add("translation", translation);
	}

	_graph = new GraphicsMan(this);

	_rnd = new Common::RandomSource("prince");

	_midiPlayer = new MusicPlayer(this);

	if (getLanguage() == Common::DE_DEU) {
		_font = new Font();
		Resource::loadResource(_font, "font3.raw", true);
	} else {
		_font = new Font();
		Resource::loadResource(_font, "font1.raw", true);
	}

	_suitcaseBmp = new MhwanhDecoder();
	Resource::loadResource(_suitcaseBmp, "walizka", true);

	_script = new Script(this);
	Resource::loadResource(_script, "skrypt.dat", true);

	_flags = new InterpreterFlags();
	_interpreter = new Interpreter(this, _script, _flags);

	_debugger = new Debugger(this, _flags);

	_variaTxt = new VariaTxt();
	if (getFeatures() & GF_TRANSLATED) {
		Resource::loadResource(_variaTxt, "variatxt_translate.dat", true);
	} else {
		Resource::loadResource(_variaTxt, "variatxt.dat", true);
	}

	_cursor1 = new Cursor();
	Resource::loadResource(_cursor1, "mouse1.cur", true);

	_cursor3 = new Cursor();
	Resource::loadResource(_cursor3, "mouse2.cur", true);

	Common::SeekableReadStream *talkTxtStream;
	if (getFeatures() & GF_TRANSLATED) {
		talkTxtStream = SearchMan.createReadStreamForMember("talktxt_translate.dat");
	} else {
		talkTxtStream = SearchMan.createReadStreamForMember("talktxt.dat");
	}
	if (!talkTxtStream) {
		error("Can't load talkTxtStream");
		return;
	}
	_talkTxtSize = talkTxtStream->size();
	_talkTxt = (byte *)malloc(_talkTxtSize);
	talkTxtStream->read(_talkTxt, _talkTxtSize);

	delete talkTxtStream;

	Common::SeekableReadStream *invTxtStream;
	if (getFeatures() & GF_TRANSLATED) {
		invTxtStream = SearchMan.createReadStreamForMember("invtxt_translate.dat");
	} else {
		invTxtStream = SearchMan.createReadStreamForMember("invtxt.dat");
	}
	if (!invTxtStream) {
		error("Can't load invTxtStream");
		return;
	}
	_invTxtSize = invTxtStream->size();
	_invTxt = (byte *)malloc(_invTxtSize);
	invTxtStream->read(_invTxt, _invTxtSize);

	delete invTxtStream;

	loadAllInv();

	Common::SeekableReadStream *dialogDatStream = SearchMan.createReadStreamForMember("dialog.dat");
	if (!dialogDatStream) {
		error("Can't load dialogDatStream");
		return;
	}
	_dialogDatSize = dialogDatStream->size();
	_dialogDat = (byte *)malloc(_dialogDatSize);
	dialogDatStream->read(_dialogDat, _dialogDatSize);

	delete dialogDatStream;

	_optionsPic = new Graphics::Surface();
	_optionsPic->create(_optionsWidth, _optionsHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect picRect(0, 0, _optionsWidth, _optionsHeight);
	_optionsPic->fillRect(picRect, _graph->kShadowColor);

	_optionsPicInInventory = new Graphics::Surface();
	_optionsPicInInventory->create(_invOptionsWidth, _invOptionsHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect invPicRect(0, 0, _invOptionsWidth, _invOptionsHeight);
	_optionsPicInInventory->fillRect(invPicRect, _graph->kShadowColor);

	_roomBmp = new Image::BitmapDecoder();

	_room = new Room();

	_mainHero = new Hero(this, _graph);
	_secondHero = new Hero(this, _graph);
	_secondHero->_maxBoredom = 140;
	_secondHero->loadAnimSet(3);

	_roomPathBitmap = (byte *)malloc(kPathBitmapLen);
	_roomPathBitmapTemp = (byte *)malloc(kPathBitmapLen);
	_coordsBuf = (byte *)malloc(kTracePts * 4);
	_coords = _coordsBuf;
	_coordsBufEnd = _coordsBuf + kTracePts * 4 - 4;

	BackgroundAnim tempBackAnim;
	tempBackAnim._seq._currRelative = 0;
	for (int i = 0; i < kMaxBackAnims; i++) {
		_backAnimList.push_back(tempBackAnim);
	}

	Anim tempAnim;
	tempAnim._animData = nullptr;
	tempAnim._shadowData = nullptr;
	for (int i = 0; i < kMaxNormAnims; i++) {
		_normAnimList.push_back(tempAnim);
	}

	_objSlot = (uint16 *)malloc(kMaxObjects * sizeof(uint16));
	for (int i = 0; i < kMaxObjects; i++) {
		_objSlot[i] = 0xFF;
	}

	_zoomBitmap = (byte *)malloc(kZoomBitmapLen);
	_shadowBitmap = (byte *)malloc(2 * kShadowBitmapSize);
	_transTable = (byte *)malloc(kTransTableSize);

	_curveData = (int16 *)malloc(2 * kCurveLen * sizeof(int16));

	_shadowLine = (byte *)malloc(kShadowLineArraySize);

	Common::SeekableReadStream *creditsDataStream;
	if (getFeatures() & GF_TRANSLATED) {
		creditsDataStream = SearchMan.createReadStreamForMember("credits_translate.dat");
	} else {
		creditsDataStream = SearchMan.createReadStreamForMember("credits.dat");
	}
	if (!creditsDataStream) {
		error("Can't load creditsDataStream");
		return;
	}
	_creditsDataSize = creditsDataStream->size();
	_creditsData = (byte *)malloc(_creditsDataSize);
	creditsDataStream->read(_creditsData, _creditsDataSize);
	delete creditsDataStream;

	if (getFeatures() & GF_TRANSLATED) {
		loadMobTranslationTexts();
	}
}

void PrinceEngine::showLogo() {
	MhwanhDecoder logo;
	if (Resource::loadResource(&logo, "logo.raw", true)) {
		loadSample(0, "LOGO.WAV");
		playSample(0, 0);
		_graph->draw(_graph->_frontScreen, logo.getSurface());
		_graph->change();
		_graph->update(_graph->_frontScreen);
		setPalette(logo.getPalette());

		uint32 logoStart = _system->getMillis();
		while (_system->getMillis() < logoStart + 5000) {
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			while (eventMan->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						stopSample(0);
						return;
					}
					break;
				case Common::EVENT_LBUTTONDOWN:
					stopSample(0);
					return;
				default:
					break;
				}
			}

			if (shouldQuit()) {
				return;
			}
		}
	}
}

Common::Error PrinceEngine::run() {
	syncSoundSettings();
	int startGameSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	init();
	if (startGameSlot == -1) {
		playVideo("topware.avi");
		showLogo();
	} else {
		loadLocation(59); // load intro location - easiest way to set everything up
		loadGame(startGameSlot);
	}
	mainLoop();
	return Common::kNoError;
}

void PrinceEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause) {
		_midiPlayer->pause();
	}
	else {
		_midiPlayer->resume();
	}
}

bool AnimListItem::loadFromStream(Common::SeekableReadStream &stream) {
	int32 pos = stream.pos();

	uint16 type = stream.readUint16LE();
	if (type == 0xFFFF) {
		return false;
	}
	_type = type;
	_fileNumber = stream.readUint16LE();
	_startPhase = stream.readUint16LE();
	_endPhase = stream.readUint16LE();
	_loopPhase = stream.readUint16LE();
	_x = stream.readSint16LE();
	_y = stream.readSint16LE();
	_loopType = stream.readUint16LE();
	_nextAnim = stream.readUint16LE();
	_flags = stream.readUint16LE();

	//debug("AnimListItem type %d, fileNumber %d, x %d, y %d, flags %d", _type, _fileNumber, _x, _y, _flags);
	//debug("startPhase %d, endPhase %d, loopPhase %d", _startPhase, _endPhase, _loopPhase);

	// 32 byte aligment
	stream.seek(pos + 32);

	return true;
}

bool PrinceEngine::loadLocation(uint16 locationNr) {

	blackPalette();

	_flicPlayer.close();

	memset(_textSlots, 0, sizeof(_textSlots));
	freeAllSamples();

	debugEngine("PrinceEngine::loadLocation %d", locationNr);
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.remove(Common::String::format("%02d", _locationNr));

	_locationNr = locationNr;
	_debugger->_locationNr = locationNr;

	_flags->setFlagValue(Flags::CURRROOM, _locationNr);
	_interpreter->stopBg();

	changeCursor(0);

	const Common::String locationNrStr = Common::String::format("%02d", _locationNr);
	debugEngine("loadLocation %s", locationNrStr.c_str());

	PtcArchive *locationArchive = new PtcArchive();
	if (!locationArchive->open(locationNrStr + "/databank.ptc"))
		error("Can't open location %s", locationNrStr.c_str());

	SearchMan.add(locationNrStr, locationArchive);

	loadMusic(_locationNr);

	// load location background, replace old one
	Resource::loadResource(_roomBmp, "room", true);
	if (_roomBmp->getSurface()) {
		_sceneWidth = _roomBmp->getSurface()->w;
	}

	loadZoom(_zoomBitmap, kZoomBitmapLen, "zoom");
	loadShadow(_shadowBitmap, kShadowBitmapSize, "shadow", "shadow2");
	loadTrans(_transTable, "trans");
	loadPath("path");

	for (uint32 i = 0; i < _pscrList.size(); i++) {
		delete _pscrList[i];
	}
	_pscrList.clear();
	Resource::loadResource(_pscrList, "pscr.lst", false);

	loadMobPriority("mobpri");

	_mobList.clear();
	if (getGameType() == kPrinceDataDE) {
		const Common::String mobLstName = Common::String::format("mob%02d.lst", _locationNr);
		debug("name: %s", mobLstName.c_str());
		Resource::loadResource(_mobList, mobLstName.c_str(), false);
	} else if (getGameType() == kPrinceDataPL) {
		Resource::loadResource(_mobList, "mob.lst", false);
	}
	if (getFeatures() & GF_TRANSLATED) {
		// update Mob texts for translated version
		setMobTranslationTexts();
	}

	_animList.clear();
	Resource::loadResource(_animList, "anim.lst", false);

	for (uint32 i = 0; i < _objList.size(); i++) {
		delete _objList[i];
	}
	_objList.clear();
	Resource::loadResource(_objList, "obj.lst", false);

	_room->loadRoom(_script->getRoomOffset(_locationNr));

	for (uint i = 0; i < _maskList.size(); i++) {
		free(_maskList[i]._data);
	}
	_maskList.clear();
	_script->loadAllMasks(_maskList, _room->_nak);

	_picWindowX = 0;

	_lightX = _script->getLightX(_locationNr);
	_lightY = _script->getLightY(_locationNr);
	setShadowScale(_script->getShadowScale(_locationNr));

	for (uint i = 0; i < _mobList.size(); i++) {
		_mobList[i]._visible = _script->getMobVisible(_room->_mobs, i);
	}

	_script->installObjects(_room->_obj);

	freeAllNormAnims();

	clearBackAnimList();
	_script->installBackAnims(_backAnimList, _room->_backAnim);

	_graph->makeShadowTable(70, _graph->_shadowTable70);
	_graph->makeShadowTable(50, _graph->_shadowTable50);

	_mainHero->freeOldMove();
	_secondHero->freeOldMove();

	_mainHero->scrollHero();

	return true;
}

void PrinceEngine::setShadowScale(int32 shadowScale) {
	shadowScale = 100 - shadowScale;
	if (!shadowScale) {
		_shadScaleValue = 10000;
	} else {
		_shadScaleValue = 10000 / shadowScale;
	}
}

void PrinceEngine::plotShadowLinePoint(int x, int y, int color, void *data) {
	PrinceEngine *vm = (PrinceEngine *)data;
	WRITE_LE_UINT16(&vm->_shadowLine[vm->_shadLineLen * 4], x);
	WRITE_LE_UINT16(&vm->_shadowLine[vm->_shadLineLen * 4 + 2], y);
	vm->_shadLineLen++;
}

void PrinceEngine::changeCursor(uint16 curId) {
	_debugger->_cursorNr = curId;
	_mouseFlag = curId;
	_flags->setFlagValue(Flags::MOUSEENABLED, curId);

	const Graphics::Surface *curSurface = nullptr;

	switch (curId) {
	default:
		error("Unknown cursor Id: %d", curId);
	case 0:
		CursorMan.showMouse(false);
		_optionsFlag = 0;
		_selectedMob = -1;
		return;
	case 1:
		curSurface = _cursor1->getSurface();
		break;
	case 2:
		curSurface = _cursor2;
		break;
	case 3:
		curSurface = _cursor3->getSurface();
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		mousePos.x = CLIP(mousePos.x, (int16) 315, (int16) 639);
		mousePos.y = CLIP(mousePos.y, (int16) 0, (int16) 170);
		_system->warpMouse(mousePos.x, mousePos.y);
		break;
	}

	CursorMan.replaceCursorPalette(_roomBmp->getPalette(), 0, 255);
	CursorMan.replaceCursor(
		curSurface->getBasePtr(0, 0),
		curSurface->w, curSurface->h,
		0, 0,
		255, false,
		&curSurface->format
	);
	CursorMan.showMouse(true);
}

void PrinceEngine::makeInvCursor(int itemNr) {
	const Graphics::Surface *cur1Surface = _cursor1->getSurface();
	int cur1W = cur1Surface->w;
	int cur1H = cur1Surface->h;
	const Common::Rect cur1Rect(0, 0, cur1W, cur1H);

	const Graphics::Surface *itemSurface = _allInvList[itemNr].getSurface();
	int itemW = itemSurface->w;
	int itemH = itemSurface->h;

	int cur2W = cur1W + itemW / 2;
	int cur2H = cur1H + itemH / 2;

	if (_cursor2 != nullptr) {
		_cursor2->free();
		delete _cursor2;
	}
	_cursor2 = new Graphics::Surface();
	_cursor2->create(cur2W, cur2H, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect cur2Rect(0, 0, cur2W, cur2H);
	_cursor2->fillRect(cur2Rect, 255);
	_cursor2->copyRectToSurface(*cur1Surface, 0, 0, cur1Rect);

	const byte *src1 = (const byte *)itemSurface->getBasePtr(0, 0);
	byte *dst1 = (byte *)_cursor2->getBasePtr(cur1W, cur1H);

	if (itemH % 2) {
		itemH--;
	}
	if (itemW % 2) {
		itemW--;
	}

	for (int y = 0; y < itemH; y++) {
		const byte *src2 = src1;
		byte *dst2 = dst1;
		if (y % 2 == 0) {
			for (int x = 0; x < itemW; x++, src2++) {
				if (x % 2 == 0) {
					if (*src2) {
						*dst2 = *src2;
					} else {
						*dst2 = 255;
					}
					dst2++;
				}
			}
			dst1 += _cursor2->pitch;
		}
		src1 += itemSurface->pitch;
	}
}

bool PrinceEngine::loadMusic(int musNumber) {
	uint8 midiNumber = MusicPlayer::_musRoomTable[musNumber];
	if (midiNumber) {
		if (midiNumber != 100) {
			 if (_currentMidi != midiNumber) {
				_currentMidi = midiNumber;
				const char *musName = MusicPlayer::_musTable[_currentMidi];
				_midiPlayer->loadMidi(musName);
			 }
		}
	} else {
		stopMusic();
	}
	return true;
}

void PrinceEngine::stopMusic() {
	if (_midiPlayer->isPlaying()) {
		_midiPlayer->stop();
	}
}

bool PrinceEngine::playNextFLCFrame() {
	if (!_flicPlayer.isVideoLoaded())
		return false;

	const Graphics::Surface *s = _flicPlayer.decodeNextFrame();
	if (s) {
		_graph->drawTransparentSurface(_graph->_frontScreen, 0, 0, s, 255);
		_graph->change();
		_flcFrameSurface = s;
	} else if (_flicLooped) {
		_flicPlayer.rewind();
		playNextFLCFrame();
	} else if (_flcFrameSurface) {
		_graph->drawTransparentSurface(_graph->_frontScreen, 0, 0, _flcFrameSurface, 255);
		_graph->change();
	}

	return true;
}

void PrinceEngine::playSample(uint16 sampleId, uint16 loopType) {
	if (_audioStream[sampleId]) {
		if (_mixer->isSoundIDActive(sampleId)) {
			return;
		}
		_audioStream[sampleId]->rewind();
		if (sampleId < 28) {
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle[sampleId], _audioStream[sampleId], sampleId, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		} else {
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle[sampleId], _audioStream[sampleId], sampleId, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		}
	}
}

void PrinceEngine::stopSample(uint16 sampleId) {
	_mixer->stopID(sampleId);
}

void PrinceEngine::stopAllSamples() {
	_mixer->stopAll();
}

void PrinceEngine::freeSample(uint16 sampleId) {
	stopSample(sampleId);
	if (_audioStream[sampleId] != nullptr) {
		delete _audioStream[sampleId];
		_audioStream[sampleId] = nullptr;
	}
}

void PrinceEngine::freeAllSamples() {
	for (int sampleId = 0; sampleId < kMaxSamples; sampleId++) {
		freeSample(sampleId);
	}
}

bool PrinceEngine::loadSample(uint32 sampleSlot, const Common::String &streamName) {
	// FIXME: This is just a workaround streamName is a path
	// SOUND\\SCIERKA1.WAV for now only last path component is used
	Common::String normalizedPath = lastPathComponent(streamName, '\\');

	// WALKAROUND: Wrong name in script, not existing sound in data files
	if (!normalizedPath.compareTo("9997BEKA.WAV")) {
		return 0;
	}

	debugEngine("loadSample slot %d, name %s", sampleSlot, normalizedPath.c_str());

	freeSample(sampleSlot);
	Common::SeekableReadStream *sampleStream = SearchMan.createReadStreamForMember(normalizedPath);
	if (sampleStream == nullptr) {
		delete sampleStream;
		error("Can't load sample %s to slot %d", normalizedPath.c_str(), sampleSlot);
	}
	_audioStream[sampleSlot] = Audio::makeWAVStream(sampleStream, DisposeAfterUse::NO);
	delete sampleStream;
	return true;
}

bool PrinceEngine::loadVoice(uint32 slot, uint32 sampleSlot, const Common::String &streamName) {
	debugEngine("Loading wav %s slot %d", streamName.c_str(), slot);

	if (slot >= kMaxTexts) {
		error("Text slot bigger than MAXTEXTS %d", kMaxTexts - 1);
		return false;
	}

	freeSample(sampleSlot);
	Common::SeekableReadStream *sampleStream = SearchMan.createReadStreamForMember(streamName);
	if (sampleStream == nullptr) {
		debug("Can't open %s", streamName.c_str());
		return false;
	}

	uint32 id = sampleStream->readUint32LE();
	if (id != MKTAG('F', 'F', 'I', 'R')) {
		error("It's not RIFF file %s", streamName.c_str());
		return false;
	}

	sampleStream->skip(0x20);
	id = sampleStream->readUint32LE();
	if (id != MKTAG('a', 't', 'a', 'd')) {
		error("No data section in %s id %04x", streamName.c_str(), id);
		return false;
	}

	id = sampleStream->readUint32LE();
	debugEngine("SetVoice slot %d time %04x", slot, id);
	id <<= 3;
	id /= 22050;
	id += 2;

	_textSlots[slot]._time = id;
	if (!slot) {
		_mainHero->_talkTime = id;
	} else if (slot == 1) {
		_secondHero->_talkTime = id;
	}

	debugEngine("SetVoice slot %d time %04x", slot, id);
	sampleStream->seek(SEEK_SET);
	_audioStream[sampleSlot] = Audio::makeWAVStream(sampleStream, DisposeAfterUse::NO);
	delete sampleStream;
	return true;
}

void PrinceEngine::setVoice(uint16 slot, uint32 sampleSlot, uint16 flag) {
	Common::String sampleName;
	uint32 currentString = _interpreter->getCurrentString();

	if (currentString >= 80000) {
		uint32 nr = currentString - 80000;
		sampleName = Common::String::format("%02d0%02d-%02d.WAV", nr / 100, nr % 100, flag);
	} else if (currentString >= 70000) {
		sampleName = Common::String::format("inv%02d-01.WAV", currentString - 70000);
	} else if (currentString >= 60000) {
		sampleName = Common::String::format("M%04d-%02d.WAV", currentString - 60000, flag);
	} else if (currentString >= 2000) {
		return;
	} else if (flag >= 100) {
		sampleName = Common::String::format("%03d-%03d.WAV", currentString, flag);
	} else {
		sampleName = Common::String::format("%03d-%02d.WAV", currentString, flag);
	}

	loadVoice(slot, sampleSlot, sampleName);
}

bool PrinceEngine::loadAnim(uint16 animNr, bool loop) {
	Common::String streamName = Common::String::format("AN%02d", animNr);
	Common::SeekableReadStream *flicStream = SearchMan.createReadStreamForMember(streamName);

	if (!flicStream) {
		error("Can't open %s", streamName.c_str());
		return false;
	}

	if (!_flicPlayer.loadStream(flicStream)) {
		error("Can't load flic stream %s", streamName.c_str());
	}

	debugEngine("%s loaded", streamName.c_str());
	_flicLooped = loop;
	_flicPlayer.start();
	playNextFLCFrame();
	return true;
}

bool PrinceEngine::loadZoom(byte *zoomBitmap, uint32 dataSize, const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		return false;
	}
	if (stream->read(zoomBitmap, dataSize) != dataSize) {
		free(zoomBitmap);
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool PrinceEngine::loadShadow(byte *shadowBitmap, uint32 dataSize, const char *resourceName1, const char *resourceName2) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName1);
	if (!stream) {
		delete stream;
		return false;
	}

	if (stream->read(shadowBitmap, dataSize) != dataSize) {
		free(shadowBitmap);
		delete stream;
		return false;
	}

	Common::SeekableReadStream *stream2 = SearchMan.createReadStreamForMember(resourceName2);
	if (!stream2) {
		delete stream;
		delete stream2;
		return false;
	}

	byte *shadowBitmap2 = shadowBitmap + dataSize;
	if (stream2->read(shadowBitmap2, dataSize) != dataSize) {
		free(shadowBitmap);
		delete stream;
		delete stream2;
		return false;
	}

	delete stream;
	delete stream2;
	return true;
}

bool PrinceEngine::loadTrans(byte *transTable, const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		for (int i = 0; i < 256; i++) {
			for (int j = 0; j < 256; j++) {
				transTable[i * 256 + j] = j;
			}
		}
		return true;
	}
	if (stream->read(transTable, kTransTableSize) != kTransTableSize) {
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool PrinceEngine::loadPath(const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		return false;
	}
	if (stream->read(_roomPathBitmap, kPathBitmapLen) != kPathBitmapLen) {
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool PrinceEngine::loadAllInv() {
	for (int i = 0; i < kMaxInv; i++) {
		InvItem tempInvItem;

		const Common::String invStreamName = Common::String::format("INV%02d", i);
		Common::SeekableReadStream *invStream = SearchMan.createReadStreamForMember(invStreamName);
		if (!invStream) {
			delete invStream;
			return true;
		}

		tempInvItem._x = invStream->readUint16LE();
		tempInvItem._y = invStream->readUint16LE();
		int width = invStream->readUint16LE();
		int height = invStream->readUint16LE();
		tempInvItem._surface = new Graphics::Surface();
		tempInvItem._surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

		for (int h = 0; h < tempInvItem._surface->h; h++) {
			invStream->read(tempInvItem._surface->getBasePtr(0, h), tempInvItem._surface->w);
		}

		_allInvList.push_back(tempInvItem);
		delete invStream;
	}

	return true;
}

bool PrinceEngine::loadMobPriority(const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		return false;
	}

	_mobPriorityList.clear();
	uint mobId;
	while (1) {
		mobId = stream->readUint32LE();
		if (mobId == 0xFFFFFFFF) {
			break;
		}
		_mobPriorityList.push_back(mobId);
	}
	delete stream;
	return true;
}

void PrinceEngine::loadMobTranslationTexts() {
	Common::SeekableReadStream *mobTranslationStream = SearchMan.createReadStreamForMember("mob_translate.dat");
	if (!mobTranslationStream) {
		error("Can't load mob_translate.dat");
	}
	_mobTranslationSize = mobTranslationStream->size();
	_mobTranslationData = (byte *)malloc(_mobTranslationSize);
	mobTranslationStream->read(_mobTranslationData, _mobTranslationSize);
	delete mobTranslationStream;
}

void PrinceEngine::setMobTranslationTexts() {
	int locationOffset = READ_UINT16(_mobTranslationData + (_locationNr - 1) * 2);
	if (locationOffset) {
		byte *locationText = _mobTranslationData + locationOffset;
		for (uint i = 0; i < _mobList.size(); i++) {
			byte c;
			locationText++;
			_mobList[i]._name.clear();
			while ((c = *locationText)) {
				_mobList[i]._name += c;
				locationText++;
			}
			locationText++;
			_mobList[i]._examText.clear();
			c = *locationText;
			locationText++;
			if (c) {
				_mobList[i]._examText += c;
				do {
					c = *locationText;
					_mobList[i]._examText += c;
					locationText++;
				} while (c != 255);
			}
		}
	}
}

void PrinceEngine::keyHandler(Common::Event event) {
	uint16 nChar = event.kbd.keycode;
	switch (nChar) {
	case Common::KEYCODE_d:
		if (event.kbd.hasFlags(Common::KBD_CTRL)) {
			getDebugger()->attach();
		}
		break;
	case Common::KEYCODE_z:
		if (_flags->getFlagValue(Flags::POWERENABLED)) {
			_flags->setFlagValue(Flags::MBFLAG, 1);
		}
		break;
	case Common::KEYCODE_x:
		if (_flags->getFlagValue(Flags::POWERENABLED)) {
			_flags->setFlagValue(Flags::MBFLAG, 2);
		}
		break;
	case Common::KEYCODE_ESCAPE:
		_flags->setFlagValue(Flags::ESCAPED2, 1);
		break;
	}
}

int PrinceEngine::getMob(Common::Array<Mob> &mobList, bool usePriorityList, int posX, int posY) {

	Common::Point pointPos(posX, posY);

	int mobListSize;
	if (usePriorityList) {
		mobListSize = _mobPriorityList.size();
	} else {
		mobListSize = mobList.size();
	}

	for (int mobNumber = 0; mobNumber < mobListSize; mobNumber++) {
		Mob *mob = nullptr;
		if (usePriorityList) {
			mob = &mobList[_mobPriorityList[mobNumber]];
		} else {
			mob = &mobList[mobNumber];
		}

		if (mob->_visible) {
			continue;
		}

		int type = mob->_type & 7;
		switch (type) {
		case 0:
		case 1:
			//normal_mob
			if (!mob->_rect.contains(pointPos)) {
				continue;
			}
			break;
		case 3:
			//mob_obj
			if (mob->_mask < kMaxObjects) {
				int nr = _objSlot[mob->_mask];
				if (nr != 0xFF) {
					Object &obj = *_objList[nr];
					Common::Rect objectRect(obj._x, obj._y, obj._x + obj._width, obj._y + obj._height);
					if (objectRect.contains(pointPos)) {
						Graphics::Surface *objSurface = obj.getSurface();
						byte *pixel = (byte *)objSurface->getBasePtr(posX - obj._x, posY - obj._y);
						if (*pixel != 255) {
							break;
						}
					}
				}
			}
			continue;
			break;
		case 2:
		case 5:
			//check_ba_mob
			if (!_backAnimList[mob->_mask].backAnims.empty()) {
				int currentAnim = _backAnimList[mob->_mask]._seq._currRelative;
				Anim &backAnim = _backAnimList[mob->_mask].backAnims[currentAnim];
				if (backAnim._animData != nullptr) {
					if (!backAnim._state) {
						Common::Rect backAnimRect(backAnim._currX, backAnim._currY, backAnim._currX + backAnim._currW, backAnim._currY + backAnim._currH);
						if (backAnimRect.contains(pointPos)) {
							int phase = backAnim._showFrame;
							int phaseFrameIndex = backAnim._animData->getPhaseFrameIndex(phase);
							Graphics::Surface *backAnimSurface = backAnim._animData->getFrame(phaseFrameIndex);
							byte pixel = *(byte *)backAnimSurface->getBasePtr(posX - backAnim._currX, posY - backAnim._currY);
							if (pixel != 255) {
								if (type == 5) {
									if (mob->_rect.contains(pointPos)) {
										break;
									}
								} else {
									break;
								}
							}
						}
					}
				}
			}
			continue;
			break;
		default:
			//not_part_ba
			continue;
			break;
		}

		if (usePriorityList) {
			return _mobPriorityList[mobNumber];
		} else {
			return mobNumber;
		}
	}
	return -1;
}

int PrinceEngine::checkMob(Graphics::Surface *screen, Common::Array<Mob> &mobList, bool usePriorityList) {
	if (_mouseFlag == 0 || _mouseFlag == 3) {
		return -1;
	}
	Common::Point mousePos = _system->getEventManager()->getMousePos();
	int mobNumber = getMob(mobList, usePriorityList, mousePos.x + _picWindowX, mousePos.y);

	if (mobNumber != -1) {
		Common::String mobName = mobList[mobNumber]._name;

		if (getLanguage() == Common::DE_DEU) {
			for (uint i = 0; i < mobName.size(); i++) {
				switch (mobName[i]) {
				case '\xc4':
					mobName.setChar('\x83', i);
					break;
				case '\xd6':
					mobName.setChar('\x84', i);
					break;
				case '\xdc':
					mobName.setChar('\x85', i);
					break;
				case '\xdf':
					mobName.setChar('\x7f', i);
					break;
				case '\xe4':
					mobName.setChar('\x80', i);
					break;
				case '\xf6':
					mobName.setChar('\x81', i);
					break;
				case '\xfc':
					mobName.setChar('\x82', i);
					break;
				}
			}
		}

		uint16 textW = getTextWidth(mobName.c_str());

		uint16 x = mousePos.x - textW / 2;
		if (x > screen->w) {
			x = 0;
		}

		if (x + textW > screen->w) {
			x = screen->w - textW;
		}

		uint16 y = mousePos.y - _font->getFontHeight();
		if (y > screen->h) {
			y = _font->getFontHeight() - 2;
		}

		_font->drawString(screen, mobName, x, y, screen->w, 216);
	}

	return mobNumber;
}

void PrinceEngine::printAt(uint32 slot, uint8 color, char *s, uint16 x, uint16 y) {
	debugC(1, DebugChannel::kEngine, "PrinceEngine::printAt slot %d, color %d, x %02d, y %02d, str %s", slot, color, x, y, s);

	if (getLanguage() == Common::DE_DEU)
		correctStringDEU(s);

	Text &text = _textSlots[slot];
	text._str = s;
	text._x = x;
	text._y = y;
	text._color = color;
	int lines = calcTextLines(s);
	text._time = calcTextTime(lines);
}

int PrinceEngine::calcTextLines(const char *s) {
	int lines = 1;
	while (*s) {
		if (*s == '\n') {
			lines++;
		}
		s++;
	}
	return lines;
}

int PrinceEngine::calcTextTime(int numberOfLines) {
	return numberOfLines * 30;
}

void PrinceEngine::correctStringDEU(char *s) {
	while (*s) {
		switch (*s) {
		case '\xc4':
			*s = '\x83';
			break;
		case '\xd6':
			*s = '\x84';
			break;
		case '\xdc':
			*s = '\x85';
			break;
		case '\xdf':
			*s = '\x7f';
			break;
		case '\xe4':
			*s = '\x80';
			break;
		case '\xf6':
			*s = '\x81';
			break;
		case '\xfc':
			*s = '\x82';
			break;
		}
		s++;
	}
}

uint32 PrinceEngine::getTextWidth(const char *s) {
	uint16 textW = 0;
	while (*s) {
		textW += _font->getCharWidth(*s) + _font->getKerningOffset(0, 0);
		s++;
	}
	return textW;
}

void PrinceEngine::showTexts(Graphics::Surface *screen) {
	for (uint32 slot = 0; slot < kMaxTexts; slot++) {

		if (_showInventoryFlag && slot) {
			// only slot 0 for inventory
			break;
		}

		Text& text = _textSlots[slot];
		if (!text._str && !text._time) {
			continue;
		}

		int x = text._x;
		int y = text._y;

		if (!_showInventoryFlag) {
			x -= _picWindowX;
			y -= _picWindowY;
		}

		Common::Array<Common::String> lines;
		_font->wordWrapText(text._str, _graph->_frontScreen->w, lines);

		int wideLine = 0;
		for (uint i = 0; i < lines.size(); i++) {
			int textLen = getTextWidth(lines[i].c_str());
			if (textLen > wideLine) {
				wideLine = textLen;
			}
		}

		int leftBorderText = 6;
		if (x + wideLine / 2 >  kNormalWidth - leftBorderText) {
			x = kNormalWidth - leftBorderText - wideLine / 2;
		}

		if (x - wideLine / 2 < leftBorderText) {
			x = leftBorderText + wideLine / 2;
		}

		int textSkip = 2;
		for (uint i = 0; i < lines.size(); i++) {
			int drawX = x - getTextWidth(lines[i].c_str()) / 2;
			int drawY = y - 10 - (lines.size() - i) * (_font->getFontHeight() - textSkip);
			if (drawX < 0) {
				drawX = 0;
			}
			if (drawY < 0) {
				drawY = 0;
			}
			_font->drawString(screen, lines[i], drawX, drawY, screen->w, text._color);
		}

		text._time--;
		if (!text._time) {
			text._str = nullptr;
		}
	}
}

bool PrinceEngine::spriteCheck(int sprWidth, int sprHeight, int destX, int destY) {
	destX -= _picWindowX;
	destY -= _picWindowY;

	 // if x1 is on visible part of screen
	if (destX < 0) {
		if (destX + sprWidth < 1) {
			//x2 is negative - out of window
			return false;
		}
	}
	 // if x1 is outside of screen on right side
	if (destX >= kNormalWidth) {
		return false;
	}

	if (destY < 0) {
		if (destY + sprHeight < 1) {
			//y2 is negative - out of window
			return false;
		}
	}
	if (destY >= kNormalHeight) {
		return false;
	}

	return true;
}

// CheckNak
void PrinceEngine::checkMasks(int x1, int y1, int sprWidth, int sprHeight, int z) {
	int x2 = x1 + sprWidth - 1;
	int y2 = y1 + sprHeight - 1;
	if (x1 < 0) {
		x1 = 0;
	}
	for (uint i = 0; i < _maskList.size(); i++) {
		if (!_maskList[i]._state && !_maskList[i]._flags) {
			if (_maskList[i]._z > z) {
				if (_maskList[i]._x1 <= x2 && _maskList[i]._x2 >= x1) {
					if (_maskList[i]._y1 <= y2 && _maskList[i]._y2 >= y1) {
						_maskList[i]._state = 1;
					}
				}
			}
		}
	}
}

// ClsNak
void PrinceEngine::clsMasks() {
	for (uint i = 0; i < _maskList.size(); i++) {
		if (_maskList[i]._state) {
			_maskList[i]._state = 0;
		}
	}
}

// InsertNakladki
void PrinceEngine::insertMasks(Graphics::Surface *originalRoomSurface) {
	for (uint i = 0; i < _maskList.size(); i++) {
		if (_maskList[i]._state) {
			if (_maskList[i]._data != nullptr) {
				showMask(i, originalRoomSurface);
			} else {
				error("insertMasks() - Wrong mask data- nr %d", i);
			}
		}
	}
}

// ShowNak
void PrinceEngine::showMask(int maskNr, Graphics::Surface *originalRoomSurface) {
	if (!_maskList[maskNr]._flags) {
		if (spriteCheck(_maskList[maskNr]._width, _maskList[maskNr]._height, _maskList[maskNr]._x1, _maskList[maskNr]._y1)) {
			int destX = _maskList[maskNr]._x1 - _picWindowX;
			int destY = _maskList[maskNr]._y1 - _picWindowY;
			DrawNode newDrawNode;
			newDrawNode.posX = destX;
			newDrawNode.posY = destY;
			newDrawNode.posZ = _maskList[maskNr]._z;
			newDrawNode.width = _maskList[maskNr]._width;
			newDrawNode.height = _maskList[maskNr]._height;
			newDrawNode.s = nullptr;
			newDrawNode.originalRoomSurface = originalRoomSurface;
			newDrawNode.data = _maskList[maskNr].getMask();
			newDrawNode.drawFunction = &_graph->drawMaskDrawNode;
			_drawNodeList.push_back(newDrawNode);
		}
	}
}

void PrinceEngine::showSprite(Graphics::Surface *spriteSurface, int destX, int destY, int destZ) {
	if (spriteCheck(spriteSurface->w, spriteSurface->h, destX, destY)) {
		destX -= _picWindowX;
		destY -= _picWindowY;
		DrawNode newDrawNode;
		newDrawNode.posX = destX;
		newDrawNode.posY = destY;
		newDrawNode.posZ = destZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.s = spriteSurface;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = _transTable;
		newDrawNode.drawFunction = &_graph->drawTransparentWithTransDrawNode;
		_drawNodeList.push_back(newDrawNode);
	}
}

void PrinceEngine::showSpriteShadow(Graphics::Surface *shadowSurface, int destX, int destY, int destZ) {
	if (spriteCheck(shadowSurface->w, shadowSurface->h, destX, destY)) {
		destX -= _picWindowX;
		destY -= _picWindowY;
		DrawNode newDrawNode;
		newDrawNode.posX = destX;
		newDrawNode.posY = destY;
		newDrawNode.posZ = destZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.s = shadowSurface;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = _graph->_shadowTable70;
		newDrawNode.drawFunction = &_graph->drawAsShadowDrawNode;
		_drawNodeList.push_back(newDrawNode);
	}
}

void PrinceEngine::showAnim(Anim &anim) {
	//ShowFrameCode
	//ShowAnimFrame
	int phase = anim._showFrame;
	int phaseFrameIndex = anim._animData->getPhaseFrameIndex(phase);
	int x = anim._x + anim._animData->getPhaseOffsetX(phase);
	int y = anim._y + anim._animData->getPhaseOffsetY(phase);
	int animFlag = anim._flags;
	int checkMaskFlag = (animFlag & 1);
	int maxFrontFlag = (animFlag & 2);
	int specialZFlag = anim._nextAnim;
	int z = anim._nextAnim;
	Graphics::Surface *animSurface = anim._animData->getFrame(phaseFrameIndex);
	int frameWidth = animSurface->w;
	int frameHeight = animSurface->h;
	int shadowZ = 0;

	if (checkMaskFlag) {
		if (!anim._nextAnim) {
			z = y + frameHeight - 1;
		}
		checkMasks(x, y, frameWidth, frameHeight, z);
	}

	if (specialZFlag) {
		z = specialZFlag;
	} else if (maxFrontFlag) {
		z = kMaxPicHeight + 1;
	} else {
		z = y + frameHeight - 1;
	}
	shadowZ = z;

	anim._currX = x;
	anim._currY = y;
	anim._currW = frameWidth;
	anim._currH = frameHeight;
	showSprite(animSurface, x, y, z);

	// make_special_shadow
	if ((anim._flags & 0x80)) {
		DrawNode newDrawNode;
		newDrawNode.posX = x;
		newDrawNode.posY = y + animSurface->h - anim._shadowBack;
		newDrawNode.posZ = Hero::kHeroShadowZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.scaleValue = _scaleValue;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = this;
		newDrawNode.drawFunction = &Hero::showHeroShadow;
		newDrawNode.s = animSurface;
		_drawNodeList.push_back(newDrawNode);
	}

	//ShowFrameCodeShadow
	//ShowAnimFrameShadow
	if (anim._shadowData != nullptr) {
		int shadowPhaseFrameIndex = anim._shadowData->getPhaseFrameIndex(phase);
		int shadowX = anim._shadowData->getBaseX() + anim._shadowData->getPhaseOffsetX(phase);
		int shadowY = anim._shadowData->getBaseY() + anim._shadowData->getPhaseOffsetY(phase);
		Graphics::Surface *shadowSurface = anim._shadowData->getFrame(shadowPhaseFrameIndex);
		int shadowFrameWidth = shadowSurface->w;
		int shadowFrameHeight = shadowSurface->h;

		if (checkMaskFlag) {
			checkMasks(shadowX, shadowY, shadowFrameWidth, shadowFrameHeight, shadowY + shadowFrameWidth - 1);
		}

		if (!shadowZ) {
			if (maxFrontFlag) {
				shadowZ = kMaxPicHeight + 1;
			} else {
				shadowZ = shadowY + shadowFrameWidth - 1;
			}
		}
		showSpriteShadow(shadowSurface, shadowX, shadowY, shadowZ);
	}
}

void PrinceEngine::showNormAnims() {
	for (int i = 0; i < kMaxNormAnims; i++) {
		Anim &anim = _normAnimList[i];
		if (anim._animData != nullptr) {
			int phaseCount = anim._animData->getPhaseCount();
			if (!anim._state) {
				if (anim._frame == anim._lastFrame - 1) {
					if (anim._loopType) {
						if (anim._loopType == 1) {
							anim._frame = anim._loopFrame;
						} else {
							continue;
						}
					}
				} else {
					anim._frame++;
				}
				anim._showFrame = anim._frame;
				if (anim._showFrame >= phaseCount) {
					anim._showFrame = phaseCount - 1;
				}
				showAnim(anim);
			}
		}
	}
}

void PrinceEngine::setBackAnim(Anim &backAnim) {
	int start = backAnim._basaData._start;
	if (start != -1) {
		backAnim._frame = start;
		backAnim._showFrame = start;
		backAnim._loopFrame = start;
	}
	int end = backAnim._basaData._end;
	if (end != -1) {
		backAnim._lastFrame = end;
	}
	backAnim._state = 0;
}

void PrinceEngine::showBackAnims() {
	for (int i = 0; i < kMaxBackAnims; i++) {
		BAS &seq = _backAnimList[i]._seq;
		int activeSubAnim = seq._currRelative;
		if (!_backAnimList[i].backAnims.empty()) {
			if (_backAnimList[i].backAnims[activeSubAnim]._animData != nullptr) {
				if (!_backAnimList[i].backAnims[activeSubAnim]._state) {
					seq._counter++;
					if (seq._type == 2) {
						if (!seq._currRelative) {
							if (seq._counter >= seq._data) {
								if (seq._anims > 2) {
									seq._currRelative = _randomSource.getRandomNumber(seq._anims - 2) + 1;
									activeSubAnim = seq._currRelative;
									seq._current = _backAnimList[i].backAnims[activeSubAnim]._basaData._num;
								}
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
								seq._counter = 0;
							}
						}
					}

					if (seq._type == 3) {
						if (!seq._currRelative) {
							if (seq._counter < seq._data2) {
								continue;
							} else {
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
							}
						}
					}

					if (_backAnimList[i].backAnims[activeSubAnim]._frame == _backAnimList[i].backAnims[activeSubAnim]._lastFrame - 1) {
						_backAnimList[i].backAnims[activeSubAnim]._frame = _backAnimList[i].backAnims[activeSubAnim]._loopFrame;
						switch (seq._type) {
						case 1:
							if (seq._anims > 1) {
								int rnd;
								do {
									rnd = _randomSource.getRandomNumber(seq._anims - 1);
								} while (rnd == seq._currRelative);
								seq._currRelative = rnd;
								seq._current = _backAnimList[i].backAnims[rnd]._basaData._num;
								activeSubAnim = rnd;
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
								seq._counter = 0;
							}
							break;
						case 2:
							if (seq._currRelative) {
								seq._currRelative = 0;
								seq._current = _backAnimList[i].backAnims[0]._basaData._num;
								activeSubAnim = 0;
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
								seq._counter = 0;
							}
							break;
						case 3:
							seq._currRelative = 0;
							seq._current = _backAnimList[i].backAnims[0]._basaData._num;
							seq._counter = 0;
							seq._data2 = _randomSource.getRandomNumber(seq._data - 1);
							continue; // for bug in original game
							break;
						}
					} else {
						_backAnimList[i].backAnims[activeSubAnim]._frame++;
					}
					_backAnimList[i].backAnims[activeSubAnim]._showFrame = _backAnimList[i].backAnims[activeSubAnim]._frame;
					showAnim(_backAnimList[i].backAnims[activeSubAnim]);
				}
			}
		}
	}
}

void PrinceEngine::removeSingleBackAnim(int slot) {
	if (!_backAnimList[slot].backAnims.empty()) {
		for (uint j = 0; j < _backAnimList[slot].backAnims.size(); j++) {
			if (_backAnimList[slot].backAnims[j]._animData != nullptr) {
				delete _backAnimList[slot].backAnims[j]._animData;
				_backAnimList[slot].backAnims[j]._animData = nullptr;
			}
			if (_backAnimList[slot].backAnims[j]._shadowData != nullptr) {
				delete _backAnimList[slot].backAnims[j]._shadowData;
				_backAnimList[slot].backAnims[j]._shadowData = nullptr;
			}
		}
		_backAnimList[slot].backAnims.clear();
		_backAnimList[slot]._seq._currRelative = 0;
	}
}

void PrinceEngine::clearBackAnimList() {
	for (int i = 0; i < kMaxBackAnims; i++) {
		removeSingleBackAnim(i);
	}
}

void PrinceEngine::grabMap() {
	_graph->_frontScreen->copyFrom(*_roomBmp->getSurface());
	showObjects();
	runDrawNodes();
	_graph->_mapScreen->copyFrom(*_graph->_frontScreen);
}

void PrinceEngine::initZoomIn(int slot) {
	freeZoomObject(slot);
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *zoomSource = object->getSurface();
		if (zoomSource != nullptr) {
			object->_flags |= 0x8000;
			object->_zoomSurface = new Graphics::Surface();
			object->_zoomSurface->create(zoomSource->w, zoomSource->h, Graphics::PixelFormat::createFormatCLUT8());
			object->_zoomSurface->fillRect(Common::Rect(zoomSource->w, zoomSource->h), 0xFF);
			object->_zoomTime = 20;
		}
	}
}

void PrinceEngine::initZoomOut(int slot) {
	freeZoomObject(slot);
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *zoomSource = object->getSurface();
		if (zoomSource != nullptr) {
			object->_flags |= 0x4000;
			object->_zoomSurface = new Graphics::Surface();
			object->_zoomSurface->copyFrom(*zoomSource);
			object->_zoomTime = 10;
		}
	}
}

void PrinceEngine::doZoomIn(int slot) {
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *orgSurface = object->getSurface();
		if (orgSurface != nullptr) {
			byte *src1 = (byte *)orgSurface->getBasePtr(0, 0);
			byte *dst1 = (byte *)object->_zoomSurface->getBasePtr(0, 0);
			int x = 0;
			int surfaceHeight = orgSurface->h;
			for (int y = 0; y < surfaceHeight; y++) {
				byte *src2 = src1;
				byte *dst2 = dst1;
				int w = orgSurface->w - x;
				src2 += x;
				dst2 += x;
				while (w > 0) {
					int randVal = _randomSource.getRandomNumber(zoomInStep - 1);
					if (randVal < w) {
						*(dst2 + randVal) = *(src2 + randVal);
						src2 += zoomInStep;
						dst2 += zoomInStep;
					} else if (y + 1 != surfaceHeight) {
						*(dst1 + orgSurface->pitch + randVal - w) = *(src1 + orgSurface->pitch + randVal - w);
					}
					w -= zoomInStep;
				}
				x = -1 * w;
				src1 += orgSurface->pitch;
				dst1 += orgSurface->pitch;
			}
		}
	}
}

void PrinceEngine::doZoomOut(int slot) {
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *orgSurface = object->getSurface();
		if (orgSurface != nullptr) {
			byte *dst1 = (byte *)object->_zoomSurface->getBasePtr(0, 0);
			int x = 0;
			int surfaceHeight = orgSurface->h;
			for (int y = 0; y < surfaceHeight; y++) {
				byte *dst2 = dst1;
				int w = orgSurface->w - x;
				dst2 += x;
				while (w > 0) {
					int randVal = _randomSource.getRandomNumber(zoomInStep - 1);
					if (randVal < w) {
						*(dst2 + randVal) = 255;
						dst2 += zoomInStep;
					} else if (y + 1 != surfaceHeight) {
						*(dst1 + orgSurface->pitch + randVal - w) = 255;
					}
					w -= zoomInStep;
				}
				x = -1 * w;
				dst1 += orgSurface->pitch;
			}
		}
	}
}

void PrinceEngine::freeZoomObject(int slot) {
	Object *object = _objList[slot];
	if (object != nullptr) {
		if (object->_zoomSurface != nullptr) {
			object->_zoomSurface->free();
			delete object->_zoomSurface;
			object->_zoomSurface = nullptr;
		}
	}
}

void PrinceEngine::showObjects() {
	for (int i = 0; i < kMaxObjects; i++) {
		int nr = _objSlot[i];
		if (nr != 0xFF) {
			Graphics::Surface *objSurface = nullptr;
			if ((_objList[nr]->_flags & 0x8000)) {
				_objList[nr]->_zoomTime--;
				if (!_objList[nr]->_zoomTime) {
					freeZoomObject(nr);
					_objList[nr]->_flags &= 0x7FFF;
					objSurface = _objList[nr]->getSurface();
				} else {
					doZoomIn(nr);
					objSurface = _objList[nr]->_zoomSurface;
				}
			} else if ((_objList[nr]->_flags & 0x4000)) {
				_objList[nr]->_zoomTime--;
				if (!_objList[nr]->_zoomTime) {
					freeZoomObject(nr);
					_objList[nr]->_flags &= 0xBFFF;
					objSurface = _objList[nr]->getSurface();
				} else {
					doZoomOut(nr);
					objSurface = _objList[nr]->_zoomSurface;
				}
			} else {
				objSurface = _objList[nr]->getSurface();
			}

			if (objSurface != nullptr) {
				if (spriteCheck(objSurface->w, objSurface->h, _objList[nr]->_x, _objList[nr]->_y)) {
					int destX = _objList[nr]->_x - _picWindowX;
					int destY = _objList[nr]->_y - _picWindowY;
					DrawNode newDrawNode;
					newDrawNode.posX = destX;
					newDrawNode.posY = destY;
					newDrawNode.posZ = _objList[nr]->_z;
					newDrawNode.width = 0;
					newDrawNode.height = 0;
					newDrawNode.s = objSurface;
					newDrawNode.originalRoomSurface = nullptr;
					if ((_objList[nr]->_flags & 0x2000)) {
						newDrawNode.data = nullptr;
						newDrawNode.drawFunction = &_graph->drawBackSpriteDrawNode;
					} else {
						newDrawNode.data = _transTable;
						if (_flags->getFlagValue(Flags::NOANTIALIAS)) {
							newDrawNode.drawFunction = &_graph->drawTransparentDrawNode;
						} else {
							newDrawNode.drawFunction = &_graph->drawTransparentWithTransDrawNode;
						}
					}
					_drawNodeList.push_back(newDrawNode);
				}

				if ((_objList[nr]->_flags & 1)) {
					checkMasks(_objList[nr]->_x, _objList[nr]->_y, objSurface->w, objSurface->h, _objList[nr]->_z);
				}
			}
		}
	}
}

void PrinceEngine::showParallax() {
	if (!_pscrList.empty()) {
		for (uint i = 0; i < _pscrList.size(); i++) {
			Graphics::Surface *pscrSurface = _pscrList[i]->getSurface();
			if (pscrSurface != nullptr) {
				int x = _pscrList[i]->_x - (_pscrList[i]->_step * _picWindowX / 4);
				int y = _pscrList[i]->_y;
				int z = PScr::kPScrZ;
				if (spriteCheck(pscrSurface->w, pscrSurface->h, x, y)) {
					showSprite(pscrSurface, x, y, z);
				}
			}
		}
	}
}

bool PrinceEngine::compareDrawNodes(DrawNode d1, DrawNode d2) {
	if (d1.posZ < d2.posZ) {
		return true;
	}
	return false;
}

void PrinceEngine::runDrawNodes() {
	Common::sort(_drawNodeList.begin(), _drawNodeList.end(), compareDrawNodes);

	for (uint i = 0; i < _drawNodeList.size(); i++) {
		(*_drawNodeList[i].drawFunction)(_graph->_frontScreen, &_drawNodeList[i]);
	}
	_graph->change();
}

void PrinceEngine::drawScreen() {
	if (!_showInventoryFlag || _inventoryBackgroundRemember) {
		clsMasks();

		_mainHero->showHero();
		_mainHero->scrollHero();
		_mainHero->drawHero();

		_secondHero->showHero();
		_secondHero->_drawX -= _picWindowX;
		_secondHero->drawHero();

		const Graphics::Surface *roomSurface;
		if (_locationNr != 50) {
			roomSurface = _roomBmp->getSurface();
		} else {
			roomSurface = _graph->_mapScreen;
		}
		Graphics::Surface visiblePart;
		if (roomSurface) {
			visiblePart = roomSurface->getSubArea(Common::Rect(_picWindowX, 0, roomSurface->w, roomSurface->h));
			_graph->draw(_graph->_frontScreen, &visiblePart);
		}

		showBackAnims();

		showNormAnims();

		playNextFLCFrame();

		showObjects();

		if (roomSurface) {
			insertMasks(&visiblePart);
		}

		showParallax();

		runDrawNodes();

		_drawNodeList.clear();

		if (!_inventoryBackgroundRemember && !_dialogFlag) {
			if (!_optionsFlag) {
				_selectedMob = checkMob(_graph->_frontScreen, _mobList, true);
			}
			showTexts(_graph->_frontScreen);
			checkOptions();
		} else {
			_inventoryBackgroundRemember = false;
		}

		showPower();

		getDebugger()->onFrame();

	} else {
		displayInventory();
	}
}

void PrinceEngine::blackPalette() {
	byte *paletteBackup = (byte *)malloc(256 * 3);
	byte *blackPalette1 = (byte *)malloc(256 * 3);

	int fadeStep = kFadeStep - 1;
	for (int i = 0; i < kFadeStep; i++) {
		_system->getPaletteManager()->grabPalette(paletteBackup, 0, 256);
		for (int j = 0; j < 256; j++) {
			blackPalette1[3 * j] = paletteBackup[3 * j] * fadeStep / 4;
			blackPalette1[3 * j + 1] = paletteBackup[3 * j + 1] * fadeStep / 4;
			blackPalette1[3 * j + 2] = paletteBackup[3 * j + 2] * fadeStep / 4;
		}
		fadeStep--;
		_graph->setPalette(blackPalette1);
		_system->updateScreen();
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		eventMan->pollEvent(event);
		if (shouldQuit()) {
			free(paletteBackup);
			free(blackPalette1);
			return;
		}
		pausePrinceEngine();
	}
	free(paletteBackup);
	free(blackPalette1);
}

void PrinceEngine::setPalette(const byte *palette) {
	if (palette != nullptr) {
		byte *blackPalette_ = (byte *)malloc(256 * 3);
		int fadeStep = 0;
		for (int i = 0; i <= kFadeStep; i++) {
			for (int j = 0; j < 256; j++) {
				blackPalette_[3 * j] = palette[3 * j] * fadeStep / 4;
				blackPalette_[3 * j + 1] = palette[3 * j + 1] * fadeStep / 4;
				blackPalette_[3 * j + 2] = palette[3 * j + 2] * fadeStep / 4;
			}
			fadeStep++;
			_graph->setPalette(blackPalette_);
			_system->updateScreen();
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				_graph->setPalette(palette);
				free(blackPalette_);
				return;
			}
			pausePrinceEngine();
		}
		_graph->setPalette(palette);
		free(blackPalette_);
	}
}

void PrinceEngine::pausePrinceEngine(int fps) {
	int delay = 1000 / fps - int32(_system->getMillis() - _currentTime);
	delay = delay < 0 ? 0 : delay;
	_system->delayMillis(delay);
	_currentTime = _system->getMillis();
}

void PrinceEngine::addInv(int heroId, int item, bool addItemQuiet) {
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}
	if (hero != nullptr) {
		if (hero->_inventory.size() < kMaxItems) {
			if (item != 0x7FFF) {
				hero->_inventory.push_back(item);
			}
			if (!addItemQuiet) {
				addInvObj();
			}
			_interpreter->setResult(0);
		} else {
			_interpreter->setResult(1);
		}
	}
}

void PrinceEngine::remInv(int heroId, int item) {
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}
	if (hero != nullptr) {
		for (uint i = 0; i < hero->_inventory.size(); i++) {
			if (hero->_inventory[i] == item) {
				hero->_inventory.remove_at(i);
				_interpreter->setResult(0);
				return;
			}
		}
	}
	_interpreter->setResult(1);
}

void PrinceEngine::clearInv(int heroId) {
	switch (heroId) {
	case 0:
		_mainHero->_inventory.clear();
		break;
	case 1:
		_secondHero->_inventory.clear();
		break;
	default:
		error("clearInv() - wrong hero slot");
		break;
	}
}

void PrinceEngine::swapInv(int heroId) {
	Common::Array<int> tempInv;
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}
	if (hero != nullptr) {
		for (uint i = 0; i < hero->_inventory.size(); i++) {
			tempInv.push_back(hero->_inventory[i]);
		}
		hero->_inventory.clear();
		for (uint i = 0; i < hero->_inventory2.size(); i++) {
			hero->_inventory.push_back(hero->_inventory2[i]);
		}
		hero->_inventory2.clear();
		for (uint i = 0; i < tempInv.size(); i++) {
			hero->_inventory2.push_back(tempInv[i]);
		}
		tempInv.clear();
	}
}

void PrinceEngine::addInvObj() {
	changeCursor(0);
	prepareInventoryToView();

	_inventoryBackgroundRemember = true;
	drawScreen();

	Graphics::Surface *suitcase = _suitcaseBmp->getSurface();

	if (!_flags->getFlagValue(Flags::CURSEBLINK)) {

		loadSample(27, "PRZEDMIO.WAV");
		playSample(27, 0);

		_mst_shadow2 = 1;

		while (_mst_shadow2 < 512) {
			rememberScreenInv();
			_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
			drawInvItems();
			_graph->update(_graph->_screenForInventory);
			_mst_shadow2 += 50;
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				return;
			}
			pausePrinceEngine();
		}
		while (_mst_shadow2 > 256) {
			rememberScreenInv();
			_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
			drawInvItems();
			_graph->update(_graph->_screenForInventory);
			_mst_shadow2 -= 42;
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				return;
			}
			pausePrinceEngine();
		}
	} else {
		//CURSEBLINK:
		for (int i = 0; i < 3; i++) {
			_mst_shadow2 = 256;
			while (_mst_shadow2 < 512) {
				rememberScreenInv();
				_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
				drawInvItems();
				_graph->update(_graph->_screenForInventory);
				_mst_shadow2 += 50;
				Common::Event event;
				Common::EventManager *eventMan = _system->getEventManager();
				eventMan->pollEvent(event);
				if (shouldQuit()) {
					return;
				}
				pausePrinceEngine();
			}
			while (_mst_shadow2 > 256) {
				rememberScreenInv();
				_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
				drawInvItems();
				_graph->update(_graph->_screenForInventory);
				_mst_shadow2 -= 50;
				Common::Event event;
				Common::EventManager *eventMan = _system->getEventManager();
				eventMan->pollEvent(event);
				if (shouldQuit()) {
					return;
				}
				pausePrinceEngine();
			}
		}
	}
	_mst_shadow2 = 0;
	for (int i = 0; i < 20; i++) {
		rememberScreenInv();
		_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
		drawInvItems();
		_graph->update(_graph->_screenForInventory);
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		eventMan->pollEvent(event);
		if (shouldQuit()) {
			return;
		}
		pausePrinceEngine();
	}
}

void PrinceEngine::rememberScreenInv() {
	_graph->_screenForInventory->copyFrom(*_graph->_frontScreen);
}

void PrinceEngine::inventoryFlagChange(bool inventoryState) {
	if (inventoryState) {
		_showInventoryFlag = true;
		_inventoryBackgroundRemember = true;
	} else {
		_showInventoryFlag = false;
	}
}

void PrinceEngine::prepareInventoryToView() {
	_invMobList.clear();
	int invItem = _mainHero->_inventory.size();
	_invLine =  invItem / 3;
	if (invItem % 3) {
		_invLine++;
	}
	if (_invLine < 4) {
		_invLine = 4;
	}
	_maxInvW = (374 - 2 * _invLine) / _invLine;
	_invLineW = _maxInvW - 2;

	int currInvX = _invLineX;
	int currInvY = _invLineY;

	Common::MemoryReadStream stream(_invTxt, _invTxtSize);
	byte c;

	uint item = 0;
	for (int i = 0; i < _invLines; i++) {
		for (int j = 0; j < _invLine; j++) {
			Mob tempMobItem;
			if (item < _mainHero->_inventory.size()) {
				int itemNr = _mainHero->_inventory[item];
				tempMobItem._visible = 0;
				tempMobItem._mask =  itemNr;
				tempMobItem._rect = Common::Rect(currInvX + _picWindowX, currInvY, currInvX + _picWindowX + _invLineW  - 1, currInvY + _invLineH - 1);
				tempMobItem._type = 0; // to work with checkMob()

				tempMobItem._name = "";
				tempMobItem._examText = "";
				int txtOffset = READ_LE_UINT32(&_invTxt[itemNr * 8]);
				int examTxtOffset = READ_LE_UINT32(&_invTxt[itemNr * 8 + 4]);

				stream.seek(txtOffset);
				while ((c = stream.readByte())) {
					tempMobItem._name += c;
				}

				stream.seek(examTxtOffset);
				while ((c = stream.readByte())) {
					tempMobItem._examText += c;
				}
				_invMobList.push_back(tempMobItem);
			}
			currInvX += _invLineW + _invLineSkipX;
			item++;
		}
		currInvX = _invLineX;
		currInvY += _invLineSkipY + _invLineH;
	}
}

void PrinceEngine::drawInvItems() {
	int currInvX = _invLineX;
	int currInvY = _invLineY;
	uint item = 0;
	for (int i = 0; i < _invLines; i++) {
		for (int j = 0; j < _invLine; j++) {
			if (item < _mainHero->_inventory.size()) {
				int itemNr = _mainHero->_inventory[item];
				_mst_shadow = 0;
				if (_mst_shadow2) {
					if (!_flags->getFlagValue(Flags::CURSEBLINK)) {
						if (item + 1 == _mainHero->_inventory.size()) { // last item in inventory
							_mst_shadow = 1;
						}
					} else if (itemNr == 1 || itemNr == 3 || itemNr == 4 || itemNr == 7) {
						_mst_shadow = 1;
					}
				}

				int drawX = currInvX;
				int drawY = currInvY;
				Graphics::Surface *itemSurface = nullptr;
				if (itemNr != 68) {
					itemSurface = _allInvList[itemNr].getSurface();
					if (itemSurface->h < _maxInvH) {
						drawY += (_maxInvH - itemSurface->h) / 2;
					}
				} else {
					// candle item:
					if (_candleCounter == 8) {
						_candleCounter = 0;
					}
					itemNr = _candleCounter;
					_candleCounter++;
					itemNr &= 7;
					itemNr += 71;
					itemSurface = _allInvList[itemNr].getSurface();
					drawY += _allInvList[itemNr]._y + (_maxInvH - 76) / 2 - 200;
				}
				if (itemSurface->w < _maxInvW) {
					drawX += (_maxInvW - itemSurface->w) / 2;
				}
				if (!_mst_shadow) {
					_graph->drawTransparentSurface(_graph->_screenForInventory, drawX, drawY, itemSurface);
				} else {
					_mst_shadow = _mst_shadow2;
					_graph->drawTransparentWithBlendSurface(_graph->_screenForInventory, drawX, drawY, itemSurface);
				}
			}
			currInvX += _invLineW + _invLineSkipX;
			item++;
		}
		currInvX = _invLineX;
		currInvY += _invLineSkipY + _invLineH;
	}
}

void PrinceEngine::walkTo() {
	if (_mainHero->_visible) {
		_mainHero->freeHeroAnim();
		_mainHero->freeOldMove();
		_interpreter->storeNewPC(_script->_scriptInfo.usdCode);
		int destX, destY;
		if (_optionsMob != -1) {
			destX = _mobList[_optionsMob]._examPosition.x;
			destY = _mobList[_optionsMob]._examPosition.y;
			_mainHero->_destDirection = _mobList[_optionsMob]._examDirection;
		} else {
			Common::Point mousePos = _system->getEventManager()->getMousePos();
			destX = mousePos.x + _picWindowX;
			destY = mousePos.y + _picWindowY;
			_mainHero->_destDirection = 0;
		}
		_mainHero->_coords = makePath(kMainHero, _mainHero->_middleX, _mainHero->_middleY, destX, destY);
		if (_mainHero->_coords != nullptr) {
			_mainHero->_currCoords = _mainHero->_coords;
			_mainHero->_dirTab = _directionTable;
			_mainHero->_currDirTab = _directionTable;
			_directionTable = nullptr;
			_mainHero->_state = Hero::kHeroStateMove;
			moveShandria();
		}
	}
}

void PrinceEngine::moveRunHero(int heroId, int x, int y, int dir, bool runHeroFlag) {
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}

	if (hero != nullptr) {
		if (dir) {
			hero->_destDirection = dir;
		}
		if (x || y) {
			hero->freeOldMove();
			hero->_coords = makePath(heroId, hero->_middleX, hero->_middleY, x, y);
			if (hero->_coords != nullptr) {
				hero->_currCoords = hero->_coords;
				hero->_dirTab = _directionTable;
				hero->_currDirTab = _directionTable;
				_directionTable = nullptr;
				if (runHeroFlag) {
					hero->_state = Hero::kHeroStateRun;
				} else {
					hero->_state = Hero::kHeroStateMove;
				}
				if (heroId == kMainHero && _mouseFlag) {
					moveShandria();
				}
			}
		} else {
			hero->freeOldMove();
			hero->_state = Hero::kHeroStateTurn;
		}
		hero->freeHeroAnim();
		hero->_visible = 1;
	}
}

void PrinceEngine::leftMouseButton() {
	_flags->setFlagValue(Flags::ESCAPED2, 1); // skip intro animation
	_flags->setFlagValue(Flags::LMOUSE, 1);
	if (_flags->getFlagValue(Flags::POWERENABLED)) {
		_flags->setFlagValue(Flags::MBFLAG, 1);
	}
	if (_mouseFlag) {
		int option = 0;
		int optionEvent = -1;

		if (_optionsFlag) {
			if (_optionEnabled < _optionsNumber && _optionEnabled != -1) {
				option = _optionEnabled;
				_optionsFlag = 0;
			} else {
				return;
			}
		} else {
			_optionsMob = _selectedMob;
			if (_optionsMob == -1) {
				walkTo();
				return;
			}
			option = 0;
		}
		//do_option
		if (_currentPointerNumber != 2) {
			//skip_use_code
			int optionScriptOffset = _room->getOptionOffset(option);
			if (optionScriptOffset != 0) {
				optionEvent = _script->scanMobEvents(_optionsMob, optionScriptOffset);
			}
			if (optionEvent == -1) {
				if (!option) {
					walkTo();
					return;
				} else {
					optionEvent = _script->getOptionStandardOffset(option);
				}
			}
		} else if (_selectedMode) {
			//give_item
			if (_room->_itemGive) {
				optionEvent = _script->scanMobEventsWithItem(_optionsMob, _room->_itemGive, _selectedItem);
			}
			if (optionEvent == -1) {
				//standard_giveitem
				optionEvent = _script->_scriptInfo.stdGiveItem;
			}
		} else {
			if (_room->_itemUse) {
				optionEvent = _script->scanMobEventsWithItem(_optionsMob, _room->_itemUse, _selectedItem);
				_flags->setFlagValue(Flags::SELITEM, _selectedItem);
			}
			if (optionEvent == -1) {
				//standard_useitem
				optionEvent = _script->_scriptInfo.stdUseItem;
			}
		}
		_interpreter->storeNewPC(optionEvent);
		_flags->setFlagValue(Flags::CURRMOB, _selectedMob);
		_selectedMob = -1;
		_optionsMob = -1;
	} else {
		if (!_flags->getFlagValue(Flags::POWERENABLED)) {
			if (!_flags->getFlagValue(Flags::NOCLSTEXT)) {
				for (int slot = 0; slot < kMaxTexts; slot++) {
					if (slot != 9) {
						Text& text = _textSlots[slot];
						if (!text._str) {
							continue;
						}
						text._str = 0;
						text._time = 0;
					}
				}
				_mainHero->_talkTime = 0;
				_secondHero->_talkTime = 0;
			}
		}
	}
}

void PrinceEngine::rightMouseButton() {
	if (_flags->getFlagValue(Flags::POWERENABLED)) {
		_flags->setFlagValue(Flags::MBFLAG, 2);
	}
	if (_mouseFlag && _mouseFlag != 3) {
		_mainHero->freeOldMove();
		_secondHero->freeOldMove();
		_interpreter->storeNewPC(0);
		if (_currentPointerNumber < 2) {
			enableOptions(true);
		} else {
			_currentPointerNumber = 1;
			changeCursor(1);
		}
	}
}

void PrinceEngine::inventoryLeftMouseButton() {
	if (!_mouseFlag) {
		_textSlots[0]._time = 0;
		_textSlots[0]._str = nullptr;
		stopSample(28);
	}

	if (_optionsFlag == 1) {
		if (_selectedMob != -1)  {
			if (_optionEnabled < _invOptionsNumber) {
				_optionsFlag = 0;
			} else {
				return;
			}
		} else {
			error("PrinceEngine::inventoryLeftMouseButton() - optionsFlag = 1, selectedMob = 0");
			if (_currentPointerNumber == 2) {
				changeCursor(1);
				_currentPointerNumber = 1;
				_selectedMob = -1;
				_optionsMob = -1;
				return;
			} else {
				return;
			}
		}
	} else {
		if (_selectedMob != -1) {
			if (_currentPointerNumber != 2) {
				if (_invMobList[_selectedMob]._mask != 29) {
					_optionEnabled = 0;
				} else {
					// map item
					_optionEnabled = 1;
				}
			} else {
				//use_item_on_item
				int invObjUU = _script->scanMobEventsWithItem(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUU, _selectedItem);
				if (invObjUU == -1) {
					int textNr = 80011; // "I can't do it."
					if (_selectedItem == 31 || _invMobList[_selectedMob]._mask == 31) {
						textNr = 80020; // "Nothing is happening."
					}
					_interpreter->setCurrentString(textNr);
					printAt(0, 216, (char *)_variaTxt->getString(textNr - 80000), kNormalWidth / 2, 100);
					setVoice(0, 28, 1);
					playSample(28, 0);
					_selectedMob = -1;
					_optionsMob = -1;
					return;
				} else {
					_interpreter->storeNewPC(invObjUU);
					_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
					_showInventoryFlag = false;
				}
			}
		} else {
			return;
		}
	}
	//do_option
	if (_optionEnabled == 0) {
		int invObjExamEvent = _script->scanMobEvents(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjExam);
		if (invObjExamEvent == -1) {
			// do_standard
			printAt(0, 216, (char *)_invMobList[_selectedMob]._examText.c_str(), kNormalWidth / 2, _invExamY);
			_interpreter->setCurrentString(_invMobList[_selectedMob]._mask + 70000);
			setVoice(0, 28, 1);
			playSample(28, 0);
			// disableuseuse
			changeCursor(0);
			_currentPointerNumber = 1;
		} else {
			_interpreter->storeNewPC(invObjExamEvent);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			_showInventoryFlag = false;
		}
	} else if (_optionEnabled == 1) {
		// not_examine
		int invObjUse = _script->scanMobEvents(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUse);
		if (invObjUse == -1) {
			// do_standard_use
			_selectedMode = 0;
			_selectedItem = _invMobList[_selectedMob]._mask;
			makeInvCursor(_invMobList[_selectedMob]._mask);
			_currentPointerNumber = 2;
			changeCursor(2);
		} else {
			_interpreter->storeNewPC(invObjUse);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			_showInventoryFlag = false;
		}
	} else if (_optionEnabled == 4) {
		// do_standard_give
		_selectedMode = 1;
		_selectedItem = _invMobList[_selectedMob]._mask;
		makeInvCursor(_invMobList[_selectedMob]._mask);
		_currentPointerNumber = 2;
		changeCursor(2);
	} else {
		// use_item_on_item
		int invObjUU = _script->scanMobEventsWithItem(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUU, _selectedItem);
		if (invObjUU == -1) {
			int textNr = 80011; // "I can't do it."
			if (_selectedItem == 31 || _invMobList[_selectedMob]._mask == 31) {
				textNr = 80020; // "Nothing is happening."
			}
			_interpreter->setCurrentString(textNr);
			printAt(0, 216, (char *)_variaTxt->getString(textNr - 80000), kNormalWidth / 2, 100);
			setVoice(0, 28, 1);
			playSample(28, 0);
		} else {
			_interpreter->storeNewPC(invObjUU);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			_showInventoryFlag = false;
		}
	}
	_selectedMob = -1;
	_optionsMob = -1;
}

void PrinceEngine::inventoryRightMouseButton() {
	if (_textSlots[0]._str == nullptr) {
		enableOptions(false);
	}
}

void PrinceEngine::enableOptions(bool checkType) {
	if (_optionsFlag != 1) {
		changeCursor(1);
		_currentPointerNumber = 1;
		if (_selectedMob != -1) {
			if (checkType) {
				if (_mobList[_selectedMob]._type & 0x100) {
					return;
				}
			}
			Common::Point mousePos = _system->getEventManager()->getMousePos();
			int x1 = mousePos.x - _optionsWidth / 2;
			int x2 = mousePos.x + _optionsWidth / 2;
			if (x1 < 0) {
				x1 = 0;
				x2 = _optionsWidth;
			} else if (x2 >= kNormalWidth) {
				x1 = kNormalWidth - _optionsWidth;
				x2 = kNormalWidth;
			}
			int y1 = mousePos.y - 10;
			if (y1 < 0) {
				y1 = 0;
			}
			if (y1 + _optionsHeight >= kNormalHeight) {
				y1 = kNormalHeight - _optionsHeight;
			}
			_optionsMob = _selectedMob;
			_optionsX = x1;
			_optionsY = y1;
			_optionsFlag = 1;
		}
	}
}

void PrinceEngine::checkOptions() {
	if (_optionsFlag) {
		Common::Rect optionsRect(_optionsX, _optionsY, _optionsX + _optionsWidth, _optionsY + _optionsHeight);
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		if (!optionsRect.contains(mousePos)) {
			_optionsFlag = 0;
			_selectedMob = -1;
			return;
		}
		_graph->drawAsShadowSurface(_graph->_frontScreen, _optionsX, _optionsY, _optionsPic, _graph->_shadowTable50);

		_optionEnabled = -1;
		int optionsYCord = mousePos.y - (_optionsY + 16);
		if (optionsYCord >= 0) {
			int selectedOptionNr = optionsYCord / _optionsStep;
			if (selectedOptionNr < _optionsNumber) {
				_optionEnabled = selectedOptionNr;
			}
		}
		int optionsColor;
		int textY = _optionsY + 16;
		for (int i = 0; i < _optionsNumber; i++) {
			if (i != _optionEnabled) {
				optionsColor = _optionsColor1;
			} else {
				optionsColor = _optionsColor2;
			}
			Common::String optText;
			switch(getLanguage()) {
			case Common::PL_POL:
				optText = optionsTextPL[i];
				break;
			case Common::DE_DEU:
				optText = optionsTextDE[i];
				break;
			case Common::EN_ANY:
				optText = optionsTextEN[i];
				break;
			default:
				break;
			};
			uint16 textW = getTextWidth(optText.c_str());
			uint16 textX = _optionsX + _optionsWidth / 2 - textW / 2;
			_font->drawString(_graph->_frontScreen, optText, textX, textY, textW, optionsColor);
			textY += _optionsStep;
		}
	}
}

void PrinceEngine::checkInvOptions() {
	if (_optionsFlag) {
		Common::Rect optionsRect(_optionsX, _optionsY, _optionsX + _invOptionsWidth, _optionsY + _invOptionsHeight);
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		if (!optionsRect.contains(mousePos)) {
			_optionsFlag = 0;
			_selectedMob = -1;
			return;
		}
		_graph->drawAsShadowSurface(_graph->_screenForInventory, _optionsX, _optionsY, _optionsPicInInventory, _graph->_shadowTable50);

		_optionEnabled = -1;
		int optionsYCord = mousePos.y - (_optionsY + 16);
		if (optionsYCord >= 0) {
			int selectedOptionNr = optionsYCord / _invOptionsStep;
			if (selectedOptionNr < _invOptionsNumber) {
				_optionEnabled = selectedOptionNr;
			}
		}
		int optionsColor;
		int textY = _optionsY + 16;
		for (int i = 0; i < _invOptionsNumber; i++) {
			if (i != _optionEnabled) {
				optionsColor = _optionsColor1;
			} else {
				optionsColor = _optionsColor2;
			}
			Common::String invText;
			switch(getLanguage()) {
			case Common::PL_POL:
				invText = invOptionsTextPL[i];
				break;
			case Common::DE_DEU:
				invText = invOptionsTextDE[i];
				break;
			case Common::EN_ANY:
				invText = invOptionsTextEN[i];
				break;
			default:
				error("Unknown game language %d", getLanguage());
				break;
			};
			uint16 textW = getTextWidth(invText.c_str());
			uint16 textX = _optionsX + _invOptionsWidth / 2 - textW / 2;
			_font->drawString(_graph->_screenForInventory, invText, textX, textY, _graph->_screenForInventory->w, optionsColor);
			textY += _invOptionsStep;
		}
	}
}

void PrinceEngine::displayInventory() {

	_mainHero->freeOldMove();
	_secondHero->freeOldMove();

	_interpreter->setFgOpcodePC(0);

	stopAllSamples();

	prepareInventoryToView();

	while (!shouldQuit()) {

		if (_textSlots[0]._str != nullptr) {
			changeCursor(0);
		} else {
			changeCursor(_currentPointerNumber);

			Common::Rect inventoryRect(_invX1, _invY1, _invX1 + _invWidth, _invY1 + _invHeight);
			Common::Point mousePos = _system->getEventManager()->getMousePos();

			if (!_invCurInside && inventoryRect.contains(mousePos)) {
				_invCurInside = true;
			}

			if (_invCurInside && !inventoryRect.contains(mousePos)) {
				inventoryFlagChange(false);
				_invCurInside = false;
				break;
			}
		}

		rememberScreenInv();

		Graphics::Surface *suitcase = _suitcaseBmp->getSurface();
		_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);

		drawInvItems();

		showTexts(_graph->_screenForInventory);

		if (!_optionsFlag && _textSlots[0]._str == nullptr) {
			_selectedMob = checkMob(_graph->_screenForInventory, _invMobList, false);
		}

		checkInvOptions();

		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				keyHandler(event);
				break;
			case Common::EVENT_LBUTTONDOWN:
				inventoryLeftMouseButton();
				break;
			case Common::EVENT_RBUTTONDOWN:
				inventoryRightMouseButton();
				break;
			default:
				break;
			}
		}

		if (!_showInventoryFlag) {
			break;
		}

		if (shouldQuit())
			return;

		getDebugger()->onFrame();
		_graph->update(_graph->_screenForInventory);
		pausePrinceEngine();
	}

	if (_currentPointerNumber == 2) {
		_flags->setFlagValue(Flags::SELITEM, _selectedItem);
	} else {
		_flags->setFlagValue(Flags::SELITEM, 0);
	}
}

void PrinceEngine::createDialogBox(int dialogBoxNr) {
	_dialogLines = 0;
	int amountOfDialogOptions = 0;
	int dialogDataValue = (int)READ_LE_UINT32(_dialogData);

	byte c;
	int sentenceNumber;
	_dialogText = _dialogBoxAddr[dialogBoxNr];
	byte *dialogText = _dialogText;

	while ((sentenceNumber = *dialogText) != 0xFF) {
		dialogText++;
		if (!(dialogDataValue & (1 << sentenceNumber))) {
			_dialogLines += calcTextLines((const char *)dialogText);
			amountOfDialogOptions++;
		}
		do {
			c = *dialogText;
			dialogText++;
		} while (c);
	}

	_dialogHeight = _font->getFontHeight() * _dialogLines + _dialogLineSpace * (amountOfDialogOptions + 1);
	_dialogImage = new Graphics::Surface();
	_dialogImage->create(_dialogWidth, _dialogHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect dBoxRect(0, 0, _dialogWidth, _dialogHeight);
	_dialogImage->fillRect(dBoxRect, _graph->kShadowColor);
}

void PrinceEngine::dialogRun() {

	_dialogFlag = true;

	while (!shouldQuit()) {

		_interpreter->stepBg();
		drawScreen();

		int dialogX = (640 - _dialogWidth) / 2;
		int dialogY = 460 - _dialogHeight;
		_graph->drawAsShadowSurface(_graph->_frontScreen, dialogX, dialogY, _dialogImage, _graph->_shadowTable50);

		int dialogSkipLeft = 14;
		int dialogSkipUp = 10;

		int dialogTextX = dialogX + dialogSkipLeft;
		int dialogTextY = dialogY + dialogSkipUp;

		Common::Point mousePos = _system->getEventManager()->getMousePos();

		byte c;
		int sentenceNumber;
		byte *dialogText = _dialogText;
		byte *dialogCurrentText = nullptr;
		int dialogSelected = -1;
		int dialogDataValue = (int)READ_LE_UINT32(_dialogData);

		while ((sentenceNumber = *dialogText) != 0xFF) {
			dialogText++;
			int actualColor = _dialogColor1;

			if (!(dialogDataValue & (1 << sentenceNumber))) {
				if (getLanguage() == Common::DE_DEU) {
					correctStringDEU((char *)dialogText);
				}
				Common::Array<Common::String> lines;
				_font->wordWrapText((const char *)dialogText, _graph->_frontScreen->w, lines);

				Common::Rect dialogOption(dialogTextX, dialogTextY - dialogSkipUp / 2, dialogX + _dialogWidth - dialogSkipLeft, dialogTextY + lines.size() * _font->getFontHeight() + dialogSkipUp / 2 - 1);
				if (dialogOption.contains(mousePos)) {
					actualColor = _dialogColor2;
					dialogSelected = sentenceNumber;
					dialogCurrentText = dialogText;
				}

				for (uint j = 0; j < lines.size(); j++) {
					_font->drawString(_graph->_frontScreen, lines[j], dialogTextX, dialogTextY, _graph->_frontScreen->w, actualColor);
					dialogTextY += _font->getFontHeight();
				}
				dialogTextY += _dialogLineSpace;
			}
			do {
				c = *dialogText;
				dialogText++;
			} while (c);
		}

		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				keyHandler(event);
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (dialogSelected != -1) {
					dialogLeftMouseButton(dialogCurrentText, dialogSelected);
					_dialogFlag = false;
				}
				break;
			default:
				break;
			}
		}

		if (shouldQuit()) {
			return;
		}

		if (!_dialogFlag) {
			break;
		}

		getDebugger()->onFrame();
		_graph->update(_graph->_frontScreen);
		pausePrinceEngine();
	}
	_dialogImage->free();
	delete _dialogImage;
	_dialogImage = nullptr;
	_dialogFlag = false;
}

void PrinceEngine::dialogLeftMouseButton(byte *string, int dialogSelected) {
	_interpreter->setString(string);
	talkHero(0);

	int dialogDataValue = (int)READ_LE_UINT32(_dialogData);
	dialogDataValue |= (1u << dialogSelected);
	WRITE_LE_UINT32(_dialogData, dialogDataValue);

	_flags->setFlagValue(Flags::BOXSEL, dialogSelected + 1);
	setVoice(0, 28, dialogSelected + 1);

	_flags->setFlagValue(Flags::VOICE_H_LINE, _dialogOptLines[dialogSelected * 4]);
	_flags->setFlagValue(Flags::VOICE_A_LINE, _dialogOptLines[dialogSelected * 4 + 1]);
	_flags->setFlagValue(Flags::VOICE_B_LINE, _dialogOptLines[dialogSelected * 4 + 2]);

	_interpreter->setString(_dialogOptAddr[dialogSelected]);
}

void PrinceEngine::talkHero(int slot) {
	// heroSlot = textSlot (slot 0 or 1)
	Text &text = _textSlots[slot];
	int lines = calcTextLines((const char *)_interpreter->getString());
	int time = lines * 30;

	if (slot == 0) {
		text._color = 220; // TODO - test this
		_mainHero->_state = Hero::kHeroStateTalk;
		_mainHero->_talkTime = time;
		text._x = _mainHero->_middleX;
		text._y = _mainHero->_middleY - _mainHero->_scaledFrameYSize;
	} else {
		text._color = _flags->getFlagValue(Flags::KOLOR); // TODO - test this
		_secondHero->_state = Hero::kHeroStateTalk;
		_secondHero->_talkTime = time;
		text._x = _secondHero->_middleX;
		text._y = _secondHero->_middleY - _secondHero->_scaledFrameYSize;
	}
	text._time = time;
	if (getLanguage() == Common::DE_DEU) {
		correctStringDEU((char *)_interpreter->getString());
	}
	text._str = (const char *)_interpreter->getString();
	_interpreter->increaseString();
}

void PrinceEngine::doTalkAnim(int animNumber, int slot, AnimType animType) {
	Text &text = _textSlots[slot];
	int lines = calcTextLines((const char *)_interpreter->getString());
	int time = lines * 30;
	if (animType == kNormalAnimation) {
		Anim &normAnim = _normAnimList[animNumber];
		if (normAnim._animData != nullptr) {
			if (!normAnim._state) {
				if (normAnim._currW && normAnim._currH) {
					text._color = _flags->getFlagValue(Flags::KOLOR);
					text._x = normAnim._currX + normAnim._currW / 2;
					text._y = normAnim._currY - 10;
				}
			}
		}
	} else if (animType == kBackgroundAnimation) {
		if (!_backAnimList[animNumber].backAnims.empty()) {
			int currAnim = _backAnimList[animNumber]._seq._currRelative;
			Anim &backAnim = _backAnimList[animNumber].backAnims[currAnim];
			if (backAnim._animData != nullptr) {
				if (!backAnim._state) {
					if (backAnim._currW && backAnim._currH) {
						text._color = _flags->getFlagValue(Flags::KOLOR);
						text._x = backAnim._currX + backAnim._currW / 2;
						text._y = backAnim._currY - 10;
					}
				}
			}
		}
	} else {
		error("doTalkAnim() - wrong animType: %d", animType);
	}
	text._time = time;
	if (getLanguage() == Common::DE_DEU) {
		correctStringDEU((char *)_interpreter->getString());
	}
	text._str = (const char *)_interpreter->getString();
	_interpreter->increaseString();
}

void PrinceEngine::freeNormAnim(int slot) {
	if (!_normAnimList.empty()) {
		_normAnimList[slot]._state = 1;
		if (_normAnimList[slot]._animData != nullptr) {
			delete _normAnimList[slot]._animData;
			_normAnimList[slot]._animData = nullptr;
		}
		if (_normAnimList[slot]._shadowData != nullptr) {
			delete _normAnimList[slot]._shadowData;
			_normAnimList[slot]._shadowData = nullptr;
		}
	}
}

void PrinceEngine::freeAllNormAnims() {
	for (int i = 0; i < kMaxNormAnims; i++) {
		freeNormAnim(i);
	}
}

void PrinceEngine::getCurve() {
	_flags->setFlagValue(Flags::TORX1, _curveData[_curvPos]);
	_flags->setFlagValue(Flags::TORY1, _curveData[_curvPos + 1]);
	_curvPos += 2;
}

void PrinceEngine::makeCurve() {
	_curvPos = 0;
	int x1 = _flags->getFlagValue(Flags::TORX1);
	int y1 = _flags->getFlagValue(Flags::TORY1);
	int x2 = _flags->getFlagValue(Flags::TORX2);
	int y2 = _flags->getFlagValue(Flags::TORY2);

	for (int i = 0; i < kCurveLen; i++) {
		int sum1 = x1 * curveValues[i][0];
		sum1 += (x2 + (x1 - x2) / 2) * curveValues[i][1];
		sum1 += x2 * curveValues[i][2];
		sum1 += x2 * curveValues[i][3];

		int sum2 = y1 * curveValues[i][0];
		sum2 += (y2 - 20) * curveValues[i][1];
		sum2 += (y2 - 10) * curveValues[i][2];
		sum2 += y2 * curveValues[i][3];

		_curveData[i * 2] = (sum1 >> 15);
		_curveData[i * 2 + 1] = (sum2 >> 15);
	}
}

void PrinceEngine::mouseWeirdo() {
	if (_mouseFlag == 3) {
		int weirdDir = _randomSource.getRandomNumber(3);
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		switch (weirdDir) {
		case 0:
			mousePos.x += kCelStep;
			break;
		case 1:
			mousePos.x -= kCelStep;
			break;
		case 2:
			mousePos.y += kCelStep;
			break;
		case 3:
			mousePos.y -= kCelStep;
			break;
		}
		mousePos.x = CLIP(mousePos.x, (int16) 315, (int16) 639);
		_flags->setFlagValue(Flags::MXFLAG, mousePos.x);
		mousePos.y = CLIP(mousePos.y, (int16) 0, (int16) 170);
		_flags->setFlagValue(Flags::MYFLAG, mousePos.y);
		_system->warpMouse(mousePos.x, mousePos.y);
	}
}

void PrinceEngine::showPower() {
	if (_flags->getFlagValue(Flags::POWERENABLED)) {
		int power = _flags->getFlagValue(Flags::POWER);

		byte *dst = (byte *)_graph->_frontScreen->getBasePtr(kPowerBarPosX, kPowerBarPosY);
		for (int y = 0; y < kPowerBarHeight; y++) {
			byte *dst2 = dst;
			for (int x = 0; x < kPowerBarWidth; x++, dst2++) {
				*dst2 = kPowerBarBackgroundColor;
			}
			dst += _graph->_frontScreen->pitch;
		}

		if (power) {
			dst = (byte *)_graph->_frontScreen->getBasePtr(kPowerBarPosX, kPowerBarGreenPosY);
			for (int y = 0; y < kPowerBarGreenHeight; y++) {
				byte *dst2 = dst;
				for (int x = 0; x < power + 1; x++, dst2++) {
					if (x < 58) {
						*dst2 = kPowerBarGreenColor1;
					} else {
						*dst2 = kPowerBarGreenColor2;
					}
				}
				dst += _graph->_frontScreen->pitch;
			}
		}

		_graph->change();
	}
}

void PrinceEngine::scrollCredits() {
	byte *scrollAdress = _creditsData;
	while (!shouldQuit()) {
		for (int scrollPos = 0; scrollPos > -23; scrollPos--) {
			const Graphics::Surface *roomSurface = _roomBmp->getSurface();
			if (roomSurface) {
				_graph->draw(_graph->_frontScreen, roomSurface);
			}
			char *s = (char *)scrollAdress;
			int drawY = scrollPos;
			for (int i = 0; i < 22; i++) {
				Common::String line;
				char *linePos = s;
				while ((*linePos != 13)) {
					line += *linePos;
					linePos++;
				}
				if (!line.empty()) {
					int drawX = (kNormalWidth - getTextWidth(line.c_str())) / 2;
					_font->drawString(_graph->_frontScreen, line, drawX, drawY, _graph->_frontScreen->w, 217);
				}

				char letter1;
				bool gotIt1 = false;
				do {
					letter1 = *s;
					s++;
					if (letter1 == 13) {
						if (*s == 10) {
							s++;
						}
						if (*s != 35) {
							gotIt1 = true;
						}
						break;
					}
				} while (letter1 != 35);

				if (gotIt1) {
					drawY += 23;
				} else {
					break;
				}
			}
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			while (eventMan->pollEvent(event)) {
				if (event.type == Common::EVENT_KEYDOWN) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						blackPalette();
						return;
					}
				}
			}
			if (shouldQuit()) {
				return;
			}
			_graph->change();
			_graph->update(_graph->_frontScreen);
			pausePrinceEngine(kFPS * 2);
		}
		char letter2;
		byte *scan2 = scrollAdress;
		bool gotIt2 = false;
		do {
			letter2 = *scan2;
			scan2++;
			if (letter2 == 13) {
				if (*scan2 == 10) {
					scan2++;
				}
				if (*scan2 != 35) {
					gotIt2 = true;
				}
				break;
			}
		} while (letter2 != 35);
		if (gotIt2) {
			scrollAdress = scan2;
		} else {
			break;
		}
	}
	blackPalette();
}

// Modified version of Graphics::drawLine() to allow breaking the loop and return value
int PrinceEngine::drawLine(int x0, int y0, int x1, int y1, int (*plotProc)(int, int, void *), void *data) {
	// Bresenham's line algorithm, as described by Wikipedia
	const bool steep = ABS(y1 - y0) > ABS(x1 - x0);

	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}

	const int delta_x = ABS(x1 - x0);
	const int delta_y = ABS(y1 - y0);
	const int delta_err = delta_y;
	int x = x0;
	int y = y0;
	int err = 0;

	const int x_step = (x0 < x1) ? 1 : -1;
	const int y_step = (y0 < y1) ? 1 : -1;

	int stopFlag = 0;
	if (steep)
		stopFlag = (*plotProc)(y, x, data);
	else
		stopFlag = (*plotProc)(x, y, data);

	while (x != x1 && !stopFlag) {
		x += x_step;
		err += delta_err;
		if (2 * err > delta_x) {
			y += y_step;
			err -= delta_x;
		}
		if (steep)
			stopFlag = (*plotProc)(y, x, data);
		else
			stopFlag = (*plotProc)(x, y, data);
	}
	return stopFlag;
}

int PrinceEngine::getPixelAddr(byte *pathBitmap, int x, int y) {
	int mask = 128 >> (x & 7);
	byte value = pathBitmap[x / 8 + y * 80];
	return (mask & value);
}

void PrinceEngine::findPoint(int x, int y) {
	_fpX = x;
	_fpY = y;

	if (getPixelAddr(_roomPathBitmap, x, y)) {
		return;
	}

	int fpL = x;
	int fpU = y;
	int fpR = x;
	int fpD = y;

	while (1) {
		if (fpD != kMaxPicHeight) {
			if (getPixelAddr(_roomPathBitmap, x, fpD)) {
				_fpX = x;
				_fpY = fpD;
				break;
			}
			fpD++;
		}
		if (fpU) {
			if (getPixelAddr(_roomPathBitmap, x, fpU)) {
				_fpX = x;
				_fpY = fpU;
				break;
			}
			fpU--;
		}
		if (fpL) {
			if (getPixelAddr(_roomPathBitmap, fpL, y)) {
				_fpX = fpL;
				_fpY = y;
				break;
			}
			fpL--;
		}
		if (fpR != _sceneWidth) {
			if (getPixelAddr(_roomPathBitmap, fpR, y)) {
				_fpX = fpR;
				_fpY = y;
				break;
			}
			fpR++;
		}
		if (!fpU && (fpD == kMaxPicHeight)) {
			if (!fpL && (fpR == _sceneWidth)) {
				break;
			}
		}
	}
}

Direction PrinceEngine::makeDirection(int x1, int y1, int x2, int y2) {
	if (x1 != x2) {
		if (y1 != y2) {
			if (x1 > x2) {
				if (y1 > y2) {
					if (x1 - x2 >= y1 - y2) {
						return kDirLU;
					} else {
						return kDirUL;
					}
				} else {
					if (x1 - x2 >= y2 - y1) {
						return kDirLD;
					} else {
						return kDirDL;
					}
				}
			} else {
				if (y1 > y2) {
					if (x2 - x1 >= y1 - y2) {
						return kDirRU;
					} else {
						return kDirUR;
					}
				} else {
					if (x2 - x1 >= y2 - y1) {
						return kDirRD;
					} else {
						return kDirDR;
					}
				}
			}
		} else {
			if (x1 >= x2) {
				return kDirL;
			} else {
				return kDirR;
			}
		}
	} else {
		if (y1 >= y2) {
			return kDirU;
		} else {
			return kDirD;
		}
	}
}

void PrinceEngine::specialPlot(int x, int y) {
	if (_coords < _coordsBufEnd) {
		WRITE_LE_UINT16(_coords, x);
		_coords += 2;
		WRITE_LE_UINT16(_coords, y);
		_coords += 2;
		specialPlot2(x, y);
	}
}

void PrinceEngine::specialPlot2(int x, int y) {
	int mask = 128 >> (x & 7);
	_roomPathBitmapTemp[x / 8 + y * 80] |= mask;
}

void PrinceEngine::specialPlotInside(int x, int y) {
	if (_coords < _coordsBufEnd) {
		WRITE_LE_UINT16(_coords, x);
		_coords += 2;
		WRITE_LE_UINT16(_coords, y);
		_coords += 2;
	}
}

int PrinceEngine::plotTraceLine(int x, int y, void *data) {
	PrinceEngine *traceLine = (PrinceEngine *)data;
	if (!traceLine->_traceLineFirstPointFlag) {
		if (!traceLine->getPixelAddr(traceLine->_roomPathBitmapTemp, x, y)) {
			if (traceLine->getPixelAddr(traceLine->_roomPathBitmap, x, y)) {
				traceLine->specialPlotInside(x, y);
				traceLine->_traceLineLen++;
				return 0;
			} else {
				return -1;
			}
		} else {
			return 1;
		}
	} else {
		traceLine->_traceLineFirstPointFlag = false;
		return 0;
	}
}

int PrinceEngine::leftDownDir() {
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::leftDir() {
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::leftUpDir() {
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::rightDownDir() {
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::rightDir() {
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::rightUpDir() {
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::upLeftDir() {
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::upDir() {
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::upRightDir() {
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::downLeftDir() {
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::downDir() {
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::downRightDir() {
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::cpe() {
	if ((*(_checkBitmap - kPBW) & _checkMask)) {
		if ((*(_checkBitmap + kPBW) & _checkMask)) {
			int value;
			switch (_checkMask) {
			case 128:
				value = READ_LE_UINT16(_checkBitmap - 1);
				value &= 0x4001;
				if (value != 0x4001) {
					return 0;
				}
				break;
			case 64:
				value = *_checkBitmap;
				value &= 0xA0;
				if (value != 0xA0) {
					return 0;
				}
				break;
			case 32:
				value = *_checkBitmap;
				value &= 0x50;
				if (value != 0x50) {
					return 0;
				}
				break;
			case 16:
				value = *_checkBitmap;
				value &= 0x28;
				if (value != 0x28) {
					return 0;
				}
				break;
			case 8:
				value = *_checkBitmap;
				value &= 0x14;
				if (value != 0x14) {
					return 0;
				}
				break;
			case 4:
				value = *_checkBitmap;
				value &= 0xA;
				if (value != 0xA) {
					return 0;
				}
				break;
			case 2:
				value = *_checkBitmap;
				value &= 0x5;
				if (value != 0x5) {
					return 0;
				}
				break;
			case 1:
				value = READ_LE_UINT16(_checkBitmap);
				value &= 0x8002;
				if (value != 0x8002) {
					return 0;
				}
				break;
			default:
				error("Wrong _checkMask value - cpe()");
				break;
			}
			_checkX = _rembX;
			_checkY = _rembY;
			_checkBitmapTemp = _rembBitmapTemp;
			_checkBitmap = _rembBitmap;
			_checkMask = _rembMask;
			return -1;
		}
		return 0;
	}
	return 0;
}

int PrinceEngine::checkLeftDownDir() {
	if (_checkX && _checkY != (kMaxPicHeight / 2 - 1)) {
		int tempMask = _checkMask;
		if (tempMask != 128) {
			tempMask <<= 1;
			if ((*(_checkBitmap + kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp + kPBW) & tempMask)) {
					_checkBitmap += kPBW;
					_checkBitmapTemp += kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap + kPBW - 1) & 1)) {
				if (!(*(_checkBitmapTemp + kPBW - 1) & 1)) {
					_checkBitmap += (kPBW - 1);
					_checkBitmapTemp += (kPBW - 1);
					_checkMask = 1;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX--;
		_checkY++;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkLeftDir() {
	if (_checkX) {
		int tempMask = _checkMask;
		if (tempMask != 128) {
			tempMask <<= 1;
			if ((*(_checkBitmap) & tempMask)) {
				if (!(*(_checkBitmapTemp) & tempMask)) {
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap - 1) & 1)) {
				if (!(*(_checkBitmapTemp - 1) & 1)) {
					_checkBitmap--;
					_checkBitmapTemp--;
					_checkMask = 1;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX--;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkDownDir() {
	if (_checkY != (kMaxPicHeight / 2 - 1)) {
		if ((*(_checkBitmap + kPBW) & _checkMask)) {
			if (!(*(_checkBitmapTemp + kPBW) & _checkMask)) {
				_checkBitmap += kPBW;
				_checkBitmapTemp += kPBW;
				_checkY++;
				return cpe();
			} else {
				return 1;
			}
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

int PrinceEngine::checkUpDir() {
	if (_checkY) {
		if ((*(_checkBitmap - kPBW) & _checkMask)) {
			if (!(*(_checkBitmapTemp - kPBW) & _checkMask)) {
				_checkBitmap -= kPBW;
				_checkBitmapTemp -= kPBW;
				_checkY--;
				return cpe();
			} else {
				return 1;
			}
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

int PrinceEngine::checkRightDir() {
	if (_checkX != (kMaxPicWidth / 2 - 1)) {
		int tempMask = _checkMask;
		if (tempMask != 1) {
			tempMask >>= 1;
			if ((*(_checkBitmap) & tempMask)) {
				if (!(*(_checkBitmapTemp) & tempMask)) {
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap + 1) & 128)) {
				if (!(*(_checkBitmapTemp + 1) & 128)) {
					_checkBitmap++;
					_checkBitmapTemp++;
					_checkMask = 128;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX++;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkLeftUpDir() {
	if (_checkX && _checkY) {
		int tempMask = _checkMask;
		if (tempMask != 128) {
			tempMask <<= 1;
			if ((*(_checkBitmap - kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp - kPBW) & tempMask)) {
					_checkBitmap -= kPBW;
					_checkBitmapTemp -= kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap - (kPBW + 1)) & 1)) {
				if (!(*(_checkBitmapTemp - (kPBW + 1)) & 1)) {
					_checkBitmap -= (kPBW + 1);
					_checkBitmapTemp -= (kPBW + 1);
					_checkMask = 1;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX--;
		_checkY--;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkRightDownDir() {
	if (_checkX != (kMaxPicWidth / 2 - 1) && _checkY != (kMaxPicHeight / 2 - 1)) {
		int tempMask = _checkMask;
		if (tempMask != 1) {
			tempMask >>= 1;
			if ((*(_checkBitmap + kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp + kPBW) & tempMask)) {
					_checkBitmap += kPBW;
					_checkBitmapTemp += kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap + kPBW + 1) & 128)) {
				if (!(*(_checkBitmapTemp + kPBW + 1) & 128)) {
					_checkBitmap += kPBW + 1;
					_checkBitmapTemp += kPBW + 1;
					_checkMask = 128;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX++;
		_checkY++;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkRightUpDir() {
	if (_checkX != (kMaxPicWidth / 2 - 1) && _checkY) {
		int tempMask = _checkMask;
		if (tempMask != 1) {
			tempMask >>= 1;
			if ((*(_checkBitmap - kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp - kPBW) & tempMask)) {
					_checkBitmap -= kPBW;
					_checkBitmapTemp -= kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap - kPBW + 1) & 128)) {
				if (!(*(_checkBitmapTemp - kPBW + 1) & 128)) {
					_checkBitmap -= (kPBW - 1);
					_checkBitmapTemp -= (kPBW - 1);
					_checkMask = 128;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX++;
		_checkY--;
		return cpe();
	} else {
		return -1;
	}
}

bool PrinceEngine::tracePath(int x1, int y1, int x2, int y2) {
	for (uint i = 0; i < kPathBitmapLen; i++) {
		_roomPathBitmapTemp[i] = 0;
	}
	if (x1 != x2 || y1 != y2) {
		if (getPixelAddr(_roomPathBitmap, x1, y1)) {
			if (getPixelAddr(_roomPathBitmap, x2, y2)) {
				_coords = _coordsBuf;
				specialPlot(x1, y1);

				int x = x1;
				int y = y1;

				while (1) {
					int btx = x;
					int bty = y;
					byte *bcad = _coords;

					_traceLineLen = 0;
					_traceLineFirstPointFlag = true;
					int drawLineFlag = drawLine(x, y, x2, y2, &this->plotTraceLine, this);

					if (!drawLineFlag) {
						return true;
					} else if (drawLineFlag == -1 && _traceLineLen >= 2) {
						byte *tempCorrds = bcad;
						while (tempCorrds != _coords) {
							x = READ_LE_UINT16(tempCorrds);
							y = READ_LE_UINT16(tempCorrds + 2);
							tempCorrds += 4;
							specialPlot2(x, y);
						}
					} else {
						_coords = bcad;
						x = btx;
						y = bty;
					}

					Direction dir = makeDirection(x, y, x2, y2);

					_rembBitmapTemp = &_roomPathBitmapTemp[x / 8 + y * 80];
					_rembBitmap = &_roomPathBitmap[x / 8 + y * 80];
					_rembMask = 128 >> (x & 7);
					_rembX = x;
					_rembY = y;

					_checkBitmapTemp = _rembBitmapTemp;
					_checkBitmap = _rembBitmap;
					_checkMask = _rembMask;
					_checkX = _rembX;
					_checkY = _rembY;

					int result;
					switch (dir) {
					case kDirLD:
						result = leftDownDir();
						break;
					case kDirL:
						result = leftDir();
						break;
					case kDirLU:
						result = leftUpDir();
						break;
					case kDirRD:
						result = rightDownDir();
						break;
					case kDirR:
						result = rightDir();
						break;
					case kDirRU:
						result = rightUpDir();
						break;
					case kDirUL:
						result = upLeftDir();
						break;
					case kDirU:
						result = upDir();
						break;
					case kDirUR:
						result = upRightDir();
						break;
					case kDirDL:
						result = downLeftDir();
						break;
					case kDirD:
						result = downDir();
						break;
					case kDirDR:
						result = downRightDir();
						break;
					default:
						result = -1;
						error("tracePath: wrong direction %d", dir);
						break;
					}

					if (result) {
						byte *tempCoords = _coords;
						tempCoords -= 4;
						if (tempCoords > _coordsBuf) {
							int tempX = READ_LE_UINT16(tempCoords);
							int tempY = READ_LE_UINT16(tempCoords + 2);
							if (_checkX == tempX && _checkY == tempY) {
								_coords = tempCoords;
							}
							x = READ_LE_UINT16(tempCoords);
							y = READ_LE_UINT16(tempCoords + 2);
						} else {
							return false;
						}
					} else {
						x = _checkX;
						y = _checkY;
					}
				}
				return true;
			} else {
				error("tracePath: wrong destination point");
			}
		} else {
			error("tracePath: wrong start point");
		}
	} else {
		error("tracePath: same point");
	}
}

void PrinceEngine::specialPlotInside2(int x, int y) {
	WRITE_LE_UINT16(_coords2, x);
	_coords2 += 2;
	WRITE_LE_UINT16(_coords2, y);
	_coords2 += 2;
}

int PrinceEngine::plotTracePoint(int x, int y, void *data) {
	PrinceEngine *tracePoint = (PrinceEngine *)data;
	if (!tracePoint->_tracePointFirstPointFlag) {
		if (tracePoint->getPixelAddr(tracePoint->_roomPathBitmap, x, y)) {
			tracePoint->specialPlotInside2(x, y);
			return 0;
		} else {
			return -1;
		}
	} else {
		tracePoint->_tracePointFirstPointFlag = false;
		return 0;
	}
}

void PrinceEngine::approxPath() {
	byte *oldCoords;
	_coords2 = _coordsBuf2;
	byte *tempCoordsBuf = _coordsBuf; // first point on path
	byte *tempCoords = _coords;
	if (tempCoordsBuf != tempCoords) {
		tempCoords -= 4; // last point on path
		while (tempCoordsBuf != tempCoords) {
			int x1 = READ_LE_UINT16(tempCoords);
			int y1 = READ_LE_UINT16(tempCoords + 2);
			int x2 = READ_LE_UINT16(tempCoordsBuf);
			int y2 = READ_LE_UINT16(tempCoordsBuf + 2);
			tempCoordsBuf += 4;
			//TracePoint
			oldCoords = _coords2;
			if (_coords2 == _coordsBuf2) {
				WRITE_LE_UINT16(_coords2, x1);
				WRITE_LE_UINT16(_coords2 + 2, y1);
				_coords2 += 4;
			} else {
				int testX = READ_LE_UINT16(_coords2 - 4);
				int testY = READ_LE_UINT16(_coords2 - 2);
				if (testX != x1 || testY != y1) {
					WRITE_LE_UINT16(_coords2, x1);
					WRITE_LE_UINT16(_coords2 + 2, y1);
					_coords2 += 4;
				}
			}
			_tracePointFirstPointFlag = true;
			bool drawLineFlag = drawLine(x1, y1, x2, y2, &this->plotTracePoint, this);
			if (!drawLineFlag) {
				tempCoords = tempCoordsBuf - 4;
				tempCoordsBuf = _coordsBuf;
			} else {
				_coords2 = oldCoords;
			}
		}
	}
}

void PrinceEngine::freeDirectionTable() {
	if (_directionTable != nullptr) {
		free(_directionTable);
		_directionTable = nullptr;
	}
}

int PrinceEngine::scanDirectionsFindNext(byte *tempCoordsBuf, int xDiff, int yDiff) {

	int tempX, tempY, direction;

	tempX = Hero::kHeroDirLeft;
	if (xDiff < 0) {
		tempX = Hero::kHeroDirRight;
	}

	tempY = Hero::kHeroDirUp;
	if (yDiff < 0) {
		tempY = Hero::kHeroDirDown;
	}

	while (1) {
		int againPointX1 = READ_LE_UINT16(tempCoordsBuf);
		int againPointY1 = READ_LE_UINT16(tempCoordsBuf + 2);
		tempCoordsBuf += 4;

		if (tempCoordsBuf == _coords) {
			direction = tempX;
			break;
		}

		int dX = againPointX1 - READ_LE_UINT16(tempCoordsBuf);
		int dY = againPointY1 - READ_LE_UINT16(tempCoordsBuf + 2);

		if (dX != xDiff) {
			direction = tempY;
			break;
		}

		if (dY != yDiff) {
			direction = tempX;
			break;
		}
	}
	return direction;
}

void PrinceEngine::scanDirections() {
	freeDirectionTable();
	byte *tempCoordsBuf = _coordsBuf;
	if (tempCoordsBuf != _coords) {
		int size = (_coords - tempCoordsBuf) / 4 + 1; // number of coord points plus one for end marker
		_directionTable = (byte *)malloc(size);
		byte *tempDirTab = _directionTable;
		int direction = -1;
		int lastDirection = -1;

		while (1) {
			int x1 = READ_LE_UINT16(tempCoordsBuf);
			int y1 = READ_LE_UINT16(tempCoordsBuf + 2);
			tempCoordsBuf += 4;
			if (tempCoordsBuf == _coords) {
				break;
			}
			int x2 = READ_LE_UINT16(tempCoordsBuf);
			int y2 = READ_LE_UINT16(tempCoordsBuf + 2);

			int xDiff = x1 - x2;
			int yDiff = y1 - y2;

			if (xDiff) {
				if (yDiff) {
					if (lastDirection != -1) {
						direction = lastDirection;
						if (direction == Hero::kHeroDirLeft) {
							if (xDiff < 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						} else if (direction == Hero::kHeroDirRight) {
							if (xDiff >= 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						} else if (direction == Hero::kHeroDirUp) {
							if (yDiff < 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						} else {
							if (yDiff >= 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						}
					} else {
						direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
					}
				} else {
					direction = Hero::kHeroDirLeft;
					if (xDiff < 0) {
						direction = Hero::kHeroDirRight;
					}
				}
			} else {
				if (yDiff) {
					direction = Hero::kHeroDirUp;
					if (yDiff < 0) {
						direction = Hero::kHeroDirDown;
					}
				} else {
					direction = lastDirection;
				}
			}
			lastDirection = direction;
			*tempDirTab = direction;
			tempDirTab++;
		}
		*tempDirTab = *(tempDirTab - 1);
		tempDirTab++;
		*tempDirTab = 0;
	}
}

void PrinceEngine::moveShandria() {
	int shanLen1 = _shanLen;
	if (_flags->getFlagValue(Flags::SHANDOG)) {
		_secondHero->freeHeroAnim();
		_secondHero->freeOldMove();
		byte *shanCoords = _mainHero->_currCoords + shanLen1 * 4 - 4;
		int shanX = READ_LE_UINT16(shanCoords - 4);
		int shanY = READ_LE_UINT16(shanCoords - 2);
		int xDiff = shanX - _secondHero->_middleX;
		if (xDiff < 0) {
			xDiff *= -1;
		}
		int yDiff = shanY - _secondHero->_middleY;
		if (yDiff < 0) {
			yDiff *= -1;
		}
		shanCoords -= 4;
		if (shanCoords != _mainHero->_currCoords) {
			yDiff *= 1.5;
			int shanDis =  xDiff * xDiff + yDiff * yDiff;
			if (shanDis >= kMinDistance) {
				while (1) {
					shanCoords -= 4;
					if (shanCoords == _mainHero->_currCoords) {
						break;
					}
					int x = READ_LE_UINT16(shanCoords);
					int y = READ_LE_UINT16(shanCoords + 2);
					int pointDiffX = x - shanX;
					if (pointDiffX < 0) {
						pointDiffX *= -1;
					}
					int pointDiffY = y - shanY;
					if (pointDiffY < 0) {
						pointDiffY *= -1;
					}
					pointDiffY *= 1.5;
					int distance = pointDiffX * pointDiffX + pointDiffY * pointDiffY;
					if (distance >= kMinDistance) {
						break;
					}
				}
				int pathSizeDiff = (shanCoords - _mainHero->_currCoords) / 4;
				int destDir = *(_mainHero->_currDirTab + pathSizeDiff);
				_secondHero->_destDirection = destDir;
				int destX = READ_LE_UINT16(shanCoords);
				int destY = READ_LE_UINT16(shanCoords + 2);
				_secondHero->_coords = makePath(kSecondHero, _secondHero->_middleX, _secondHero->_middleY, destX, destY);
				if (_secondHero->_coords != nullptr) {
					_secondHero->_currCoords = _secondHero->_coords;
					int delay = shanLen1 - _shanLen;
					if (delay < 6) {
						delay = 6;
					}
					_secondHero->_moveDelay = delay / 2;
					_secondHero->_state = Hero::kHeroStateDelayMove;
					_secondHero->_dirTab = _directionTable;
					_secondHero->_currDirTab = _directionTable;
					_directionTable = nullptr;
				}
			}
		}
	}
}

byte *PrinceEngine::makePath(int heroId, int currX, int currY, int destX, int destY) {
	int realDestX = destX;
	int realDestY = destY;
	_flags->setFlagValue(Flags::MOVEDESTX, destX);
	_flags->setFlagValue(Flags::MOVEDESTY, destY);

	int x1 = currX / 2;
	int y1 = currY / 2;
	int x2 = destX / 2;
	int y2 = destY / 2;

	if ((x1 != x2) || (y1 != y2)) {
		findPoint(x1, y1);
		if (!getPixelAddr(_roomPathBitmap, _fpX, _fpY)) {
			return nullptr;
		}
		if ((x1 != _fpX) || (y1 != _fpY)) {
			x1 = _fpX;
			y1 = _fpY;
		}
		findPoint(x2, y2);
		if (!getPixelAddr(_roomPathBitmap, _fpX, _fpY)) {
			return nullptr;
		}
		if ((x2 != _fpX) || (y2 != _fpY)) {
			x2 = _fpX;
			y2 = _fpY;
			if (!_flags->getFlagValue(Flags::EXACTMOVE)) {
				realDestX = x2 * 2;
				realDestY = y2 * 2;
				_flags->setFlagValue(Flags::MOVEDESTX, realDestX);
				_flags->setFlagValue(Flags::MOVEDESTY, realDestY);
			} else {
				return nullptr;
			}
		}

		if ((x1 == x2) && (y1 == y2)) {
			if (!heroId) {
				_mainHero->freeOldMove();
				_mainHero->_state = Hero::kHeroStateTurn;
			} else if (heroId == 1) {
				_secondHero->freeOldMove();
				_secondHero->_state = Hero::kHeroStateTurn;
			}
			return nullptr;
		}

		int pathLen1 = 0;
		int pathLen2 = 0;
		int stX = x1;
		int stY = y1;
		int sizeCoords2 = 0;

		if (tracePath(x1, y1, x2, y2)) {
			allocCoords2();
			approxPath();
			sizeCoords2 = _coords2 - _coordsBuf2;
			for (int i = 0; i < sizeCoords2; i++) {
				_coordsBuf[i] = _coordsBuf2[i];
			}
			_coords = _coordsBuf + sizeCoords2;
			approxPath();
			_coordsBuf3 = _coordsBuf2;
			_coordsBuf2 = nullptr;
			_coords3 = _coords2;
			_coords2 = nullptr;
			pathLen1 = _coords3 - _coordsBuf3;
		}
		if (tracePath(x2, y2, x1, y1)) {
			allocCoords2();
			approxPath();
			sizeCoords2 = _coords2 - _coordsBuf2;
			for (int i = 0; i < sizeCoords2; i++) {
				_coordsBuf[i] = _coordsBuf2[i];
			}
			_coords = _coordsBuf + sizeCoords2;
			approxPath();
			pathLen2 = _coords2 - _coordsBuf2;
		}

		byte *chosenCoordsBuf = _coordsBuf2;
		byte *choosenCoords = _coords2;
		int choosenLength = pathLen1;
		if (pathLen1 < pathLen2) {
			chosenCoordsBuf = _coordsBuf3;
			choosenCoords = _coords3;
			choosenLength = pathLen2;
		}

		if (choosenLength) {
			if (chosenCoordsBuf != nullptr) {
				int tempXBegin = READ_LE_UINT16(chosenCoordsBuf);
				int tempYBegin = READ_LE_UINT16(chosenCoordsBuf + 2);
				if (stX != tempXBegin || stY != tempYBegin) {
					SWAP(chosenCoordsBuf, choosenCoords);
					chosenCoordsBuf -= 4;
					byte *tempCoordsBuf = _coordsBuf;
					while (1) {
						int cord = READ_LE_UINT32(chosenCoordsBuf);
						WRITE_LE_UINT32(tempCoordsBuf, cord);
						tempCoordsBuf += 4;
						if (chosenCoordsBuf == choosenCoords) {
							break;
						}
						chosenCoordsBuf -= 4;
					}
					_coords = tempCoordsBuf;
				} else {
					int sizeChoosen = choosenCoords - chosenCoordsBuf;
					for (int i = 0; i < sizeChoosen; i++) {
						_coordsBuf[i] = chosenCoordsBuf[i];
					}
					_coords = _coordsBuf + sizeChoosen;
				}
				WRITE_LE_UINT32(_coords, 0xFFFFFFFF);
				freeCoords2();
				freeCoords3();
				scanDirections();

				byte *tempCoordsBuf = _coordsBuf;
				byte *tempCoords = _coords;
				byte *newCoords;
				if (tempCoordsBuf != tempCoords) {
					int normCoordsSize = _coords - _coordsBuf + 4;
					newCoords = (byte *)malloc(normCoordsSize);
					byte *newCoordsBegin = newCoords;
					while (tempCoordsBuf != tempCoords) {
						int newValueX = READ_LE_UINT16(tempCoordsBuf);
						WRITE_LE_UINT16(newCoords, newValueX * 2);
						newCoords += 2;
						int newValueY = READ_LE_UINT16(tempCoordsBuf + 2);
						WRITE_LE_UINT16(newCoords, newValueY * 2);
						newCoords += 2;
						tempCoordsBuf += 4;
					}
					WRITE_LE_UINT16(newCoords - 4, realDestX);
					WRITE_LE_UINT16(newCoords - 2, realDestY);
					WRITE_LE_UINT32(newCoords, 0xFFFFFFFF);
					newCoords += 4;
					_shanLen = (newCoords - newCoordsBegin);
					_shanLen /= 4;
					return newCoordsBegin;
				}
			}
		}
		_coords = _coordsBuf;
		freeCoords2();
		freeCoords3();
		return nullptr;
	} else {
		if (!heroId) {
			_mainHero->freeOldMove();
			_mainHero->_state = Hero::kHeroStateTurn;
		} else if (heroId == 1) {
			_secondHero->freeOldMove();
			_secondHero->_state = Hero::kHeroStateTurn;
		}
		return nullptr;
	}
}

void PrinceEngine::allocCoords2() {
	if (_coordsBuf2 == nullptr) {
		_coordsBuf2 = (byte *)malloc(kTracePts * 4);
		_coords2 = _coordsBuf2;
	}
}

void PrinceEngine::freeCoords2() {
	if (_coordsBuf2 != nullptr) {
		free(_coordsBuf2);
		_coordsBuf2 = nullptr;
		_coords2 = nullptr;
	}
}

void PrinceEngine::freeCoords3() {
	if (_coordsBuf3 != nullptr) {
		free(_coordsBuf3);
		_coordsBuf3 = nullptr;
		_coords3 = nullptr;
	}
}

void PrinceEngine::openInventoryCheck() {
	if (!_optionsFlag) {
		if (_mouseFlag == 1 || _mouseFlag == 2) {
			if (_mainHero->_visible) {
				if (!_flags->getFlagValue(Flags::INVALLOWED)) {
					// 29 - Basement, 50 - Map
					if (_locationNr != 29 && _locationNr != 50) {
						Common::Point mousePos = _system->getEventManager()->getMousePos();
						if (mousePos.y < 4 && !_showInventoryFlag) {
							_invCounter++;
						} else {
							_invCounter = 0;
						}
						if (_invCounter >= _invMaxCount) {
							inventoryFlagChange(true);
						}
					}
				}
			}
		}
	}
}

void PrinceEngine::mainLoop() {
	changeCursor(0);
	_currentTime = _system->getMillis();

	while (!shouldQuit()) {
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				keyHandler(event);
				break;
			case Common::EVENT_LBUTTONDOWN:
				leftMouseButton();
				break;
			case Common::EVENT_RBUTTONDOWN:
				rightMouseButton();
				break;
			default:
				break;
			}
		}

		if (shouldQuit()) {
			return;
		}

		// for "throw a rock" mini-game
		mouseWeirdo();

		_interpreter->stepBg();
		_interpreter->stepFg();

		drawScreen();

		_graph->update(_graph->_frontScreen);

		openInventoryCheck();

		pausePrinceEngine();
	}
}

} // End of namespace Prince
